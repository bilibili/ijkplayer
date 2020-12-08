/**
 * @file
 * LAS demuxer
 * https://github.com/KwaiVideoTeam/las
 */

#include <errno.h>
#include <stdlib.h>
#ifdef __ANDROID__
#include <sys/prctl.h>
#endif
#ifdef __APPLE__
#include "TargetConditionals.h"
#endif
#include "libavutil/avstring.h"
#include "libavutil/avassert.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/dict.h"
#include "libavutil/time.h"
#include "libavformat/avformat.h"
#include "libavformat/url.h"
#include "libavformat/avio_internal.h"
#include "libavformat/id3v2.h"
#include "libavformat/flv.h"

#include "ijksdl/ijksdl_thread.h"
#include "ijksdl/ijksdl_mutex.h"
#include "cJSON.h"

#include "ijklas.h"

#define LAS_ERROR_BASE                        (-30000)
#define LAS_ERROR_MUTEX_CREATE                (-1 + LAS_ERROR_BASE)
#define LAS_ERROR_THREAD_CREATE               (-2 + LAS_ERROR_BASE)
#define LAS_ERROR_MANIFEST_JSON               (-3 + LAS_ERROR_BASE)
#define LAS_ERROR_ADAPT_CONFIG_JSON           (-4 + LAS_ERROR_BASE)
#define LAS_ERROR_INVALID_REP_INDEX           (-6 + LAS_ERROR_BASE)
#define LAS_ERROR_SOCKET_CLOSED_BY_PEER       (-11 + LAS_ERROR_BASE)
#define LAS_ERROR_GET_WHOLE_TAG               (-12 + LAS_ERROR_BASE)
#define LAS_ERROR_ABORT_BY_USER               (-14 + LAS_ERROR_BASE)
#define LAS_ERROR_ABR_HISTORY_DATA_JSON       (-15 + LAS_ERROR_BASE)
#define LAS_ERROR_COND_CREATE                 (-16 + LAS_ERROR_BASE)

#define INITIAL_BUFFER_SIZE             32768

#define FLV_HEADER_LEN 9
#define TAG_HEADER_LEN 11
#define PRE_TAG_SIZE_LEN 4

//las 2.0 Tag based
#define AV_TAG_HEADER_LEN 16 // 11+1+1+3, 1 bytes for av parameters, 1 bytes for AVCPacketType, 3 bytes for CompositionTime
#define TIME_ALGO_UPDATE_INTERVAL_MS (500)
#define INIT_BUFFER_THRESHOLD_MAX_MS (8*1000)
#define MAX_BUFFER_TIME 10000
#define MAX_STATE_CNT 30
#define NALU_HEAD_LEN 4
#define H264_NAL_SPS 7
#define H264_NAL_PPS 8

typedef struct AdaptiveConfig {
    int32_t buffer_init;
    double stable_buffer_diff_threshold_second;
    int32_t stable_buffer_interval_ms;
    int32_t generate_speed_gap_ms;
    int32_t buffer_check_interval_ms;
    double smoothed_speed_utilization_ratio;
    double small_speed_to_bitrate_ratio;
    double enough_speed_to_bitrate_ratio;
    double buffer_lower_limit_second;
    int32_t recent_buffered_size;
    double smoothed_speed_ratio;
} AdaptiveConfig;

typedef struct MultiRateAdaption {
    int32_t n_bitrates;
    int32_t bitrate_table_origin_order[MAX_STREAM_NUM];
    int32_t disable_adaptive_table[MAX_STREAM_NUM];
    int32_t next_expected_rep_index;
    struct PlayList* playlist;
    unsigned session_id;

    // algorithm related
    AdaptiveConfig conf;
    double past_buffer[MAX_STATE_CNT];
    int64_t buffer_index;
    int32_t levels[MAX_STREAM_NUM];
    int32_t current;
    int64_t stable_buffer_start_time;
    double generated_speed;
    double last_check_buffer;
    int64_t last_speed;
    int32_t buffer_init;
} MultiRateAdaption;

typedef struct FlvTag {
    uint8_t* buf;
    uint32_t tag_size;
    uint32_t buf_write_offset;
    uint32_t buf_read_offset;
    uint32_t av_tag_ts;
    enum FlvTagType type;
    int rep_index;
    int audio_only;
    int switch_index;
} FlvTag;

typedef struct Representation {
    char url[MAX_URL_SIZE];
    int id;
    int bitrate;
    int disabled_from_adaptive;
    int default_selected;
    int index;
} Representation;

typedef struct AdaptationSet {
    int duration;
    Representation* representations[MAX_STREAM_NUM];
    int n_representation;
} AdaptationSet;

typedef struct TagListNode {
    FlvTag tag;
    struct TagListNode* next;
} TagListNode;

typedef struct TagQueue {
    TagListNode* first_tag, *last_tag;
    int nb_tags;
    uint32_t last_video_ts;
    int64_t total_tag_bytes;
    int abort_request;
    SDL_mutex* mutex;
    SDL_cond* cond;
} TagQueue;

typedef struct GopReader {
    // real read context
    char realtime_url[MAX_URL_SIZE];
    URLContext* input;
    int abort_request;
    int64_t last_gop_start_ts;

    int rep_index;
    int is_audio_only;
    int switch_index;
    AVFormatContext* parent;
} GopReader;

typedef struct PlayList {
    struct AdaptationSet adaptation_set;
    AVFormatContext* outermost_ctx;

    // demuxer related
    AVFormatContext* parent;
    uint8_t* read_buffer;
    AVIOContext pb;
    AVFormatContext* ctx;
    AVPacket pkt;

    int cur_rep_index;
    int cur_switch_index;
    int stream_index_map[MAX_STREAM_NUM];
    int error_code;
    int read_abort_request;
    SDL_Thread _read_thread;
    SDL_Thread* read_thread;

    SDL_Thread _algo_thread;
    SDL_Thread* algo_thread;
    SDL_cond* algo_cond;

    SDL_mutex* rw_mutex;
    SDL_mutex* reading_tag_mutex;
    // las_mutex is privately used inside of #pragma PlayListLock's setters and getters
    SDL_mutex* las_mutex;

    MultiRateAdaption multi_rate_adaption;
    GopReader gop_reader;
    FlvTag reading_tag;
    TagQueue tag_queue;

    // cur playlist Qos
    LasStatistic* las_statistic;
    LasPlayerStatistic* las_player_statistic;
    bool is_stream_ever_opened;
    int64_t bytes_read;
    unsigned session_id;
} PlayList;

typedef struct LasContext {
    AVClass* class;
    // the outermost context
    AVFormatContext* ctx;

    AVIOInterruptCB* interrupt_callback;
    char* user_agent;                    ///< holds HTTP user agent set as an AVOption to the HTTP protocol context
    char* cookies;                       ///< holds HTTP cookie values set in either the initial response or as an AVOption to the HTTP protocol context
    char* headers;                       ///< holds HTTP headers set as an AVOption to the HTTP protocol context
    char* http_proxy;                    ///< holds the address of the HTTP proxy server
    char* server_ip;                     ///< holds the HTTP server ip
    char* manifest_string;
    int64_t network;
    char* abr_history_data;
    int64_t las_player_statistic;
    char* live_adapt_config;
    AVDictionary* avio_opts;

    // all info of las is in it
    PlayList playlist;
    unsigned session_id;
} LasContext;

#pragma mark common util
inline static int64_t get_current_time_ms() {
    return av_gettime_relative() / 1000;
}

#pragma mark log util
#define LOG_THREAD 1
// ------------------------------------ log util start------------------------------------
static inline void _log(unsigned session_id, const char* func_name, int av_log_level, ...) {
    va_list args;
    va_start(args, av_log_level);
    const char* fmt = va_arg(args, const char*);
    char tmp[1024] = {0};
    vsprintf(tmp, fmt, args);
    va_end(args);
#if LOG_THREAD
    av_log(NULL, av_log_level, "[%u][las][%s] %s\n", session_id, func_name, tmp);
#endif
}

#define log_debug_tag(session_id, av_log_level, ...) _log(session_id, __func__, av_log_level, __VA_ARGS__)
#define log_debug(...) log_debug_tag(playlist->session_id, AV_LOG_DEBUG, __VA_ARGS__)
#define log_info(...) log_debug_tag(playlist->session_id, AV_LOG_INFO, __VA_ARGS__)
#define log_error(...) log_debug_tag(playlist->session_id, AV_LOG_ERROR, __VA_ARGS__)
#define algo_debug(...) log_debug_tag(thiz->session_id, AV_LOG_DEBUG, __VA_ARGS__)
#define algo_info(...) log_debug_tag(thiz->session_id, AV_LOG_INFO, __VA_ARGS__)
#define algo_error(...) log_debug_tag(thiz->session_id, AV_LOG_ERROR, __VA_ARGS__)

#pragma mark PlayerControl
int las_stat_init(LasPlayerStatistic* stat) {
    memset(stat, 0, sizeof(LasPlayerStatistic));
    stat->las_switch_mode = LAS_AUTO_MODE;
    return pthread_mutex_init(&stat->control_mutex, NULL);
}

void las_stat_destroy(LasPlayerStatistic* stat) {
    pthread_mutex_destroy(&stat->control_mutex);
}

int las_get_switch_mode(LasPlayerStatistic* stat) {
    pthread_mutex_lock(&stat->control_mutex);
    int switch_mode = stat->las_switch_mode;
    pthread_mutex_unlock(&stat->control_mutex);
    return switch_mode;
}

void las_set_switch_mode(LasPlayerStatistic* stat, int switch_mode) {
    pthread_mutex_lock(&stat->control_mutex);
    stat->las_switch_mode = switch_mode;
    pthread_mutex_unlock(&stat->control_mutex);
}

int64_t las_get_first_audio_packet_pts(LasPlayerStatistic* stat) {
    pthread_mutex_lock(&stat->control_mutex);
    int64_t ret = stat->first_audio_packet_pts;
    pthread_mutex_unlock(&stat->control_mutex);
    return ret;
}

void las_set_first_audio_packet_pts(LasPlayerStatistic* stat, int64_t first_audio_packet_pts) {
    pthread_mutex_lock(&stat->control_mutex);
    stat->first_audio_packet_pts = first_audio_packet_pts;
    pthread_mutex_unlock(&stat->control_mutex);
}

int las_get_audio_only_request(LasPlayerStatistic* stat) {
    pthread_mutex_lock(&stat->control_mutex);
    int ret = stat->audio_only_request;
    pthread_mutex_unlock(&stat->control_mutex);
    return ret;
}

void las_set_audio_only_request(LasPlayerStatistic* stat, int audio_only) {
    pthread_mutex_lock(&stat->control_mutex);
    stat->audio_only_request = audio_only;
    pthread_mutex_unlock(&stat->control_mutex);
}

int las_get_audio_only_response(LasPlayerStatistic* stat) {
    pthread_mutex_lock(&stat->control_mutex);
    int ret = stat->audio_only_response;
    pthread_mutex_unlock(&stat->control_mutex);
    return ret;
}

void las_set_audio_only_response(LasPlayerStatistic* stat, int audio_only) {
    pthread_mutex_lock(&stat->control_mutex);
    stat->audio_only_response = audio_only;
    pthread_mutex_unlock(&stat->control_mutex);
}

int64_t las_get_audio_cached_duration_ms(LasPlayerStatistic* stat) {
    pthread_mutex_lock(&stat->control_mutex);
    int64_t ret = stat->audio_cached_duration_ms;
    pthread_mutex_unlock(&stat->control_mutex);
    return ret;
}

void las_set_audio_cached_duration_ms(LasPlayerStatistic* stat, int64_t audio_cached_duration_ms) {
    pthread_mutex_lock(&stat->control_mutex);
    stat->audio_cached_duration_ms = audio_cached_duration_ms;
    pthread_mutex_unlock(&stat->control_mutex);
}

int64_t las_get_video_cached_duration_ms(LasPlayerStatistic* stat) {
    pthread_mutex_lock(&stat->control_mutex);
    int64_t ret = stat->video_cached_duration_ms;
    pthread_mutex_unlock(&stat->control_mutex);
    return ret;
}

void las_set_video_cached_duration_ms(LasPlayerStatistic* stat, int64_t video_cached_duration_ms) {
    pthread_mutex_lock(&stat->control_mutex);
    stat->video_cached_duration_ms = video_cached_duration_ms;
    pthread_mutex_unlock(&stat->control_mutex);
}

bool las_get_stream_reopened(LasPlayerStatistic* stat) {
    pthread_mutex_lock(&stat->control_mutex);
    bool ret = stat->stream_reopened;
    pthread_mutex_unlock(&stat->control_mutex);
    return ret;
}

void las_set_stream_reopened(LasPlayerStatistic* stat, bool stream_reopened) {
    pthread_mutex_lock(&stat->control_mutex);
    stat->stream_reopened = stream_reopened;
    pthread_mutex_unlock(&stat->control_mutex);
}

#pragma mark PlayerStat
int32_t LasPlayerStatistic_get_downloading_bitrate(LasPlayerStatistic* stat) {
    if (stat->las_stat.bitrate_downloading > 0) {
        return stat->las_stat.bitrate_downloading;
    }
    return 0;
}

char* LasPlayerStatistic_get_downloading_url(LasPlayerStatistic* stat) {
    return stat->las_stat.cur_playing_url;
}

int LasPlayerStatistic_get_http_reading_error(LasPlayerStatistic* stat) {
    if (stat->las_stat.cur_rep_http_reading_error != 0) {
        return stat->las_stat.cur_rep_http_reading_error;
    } else {
        return 0;
    }
}


#pragma mark PlayListLock
static int64_t get_bytes_read(PlayList* p) {
    SDL_LockMutex(p->las_mutex);
    int64_t ret = p->bytes_read;
    SDL_UnlockMutex(p->las_mutex);
    return ret;
}

static void add_bytes_read(PlayList* p, int64_t bytes_read) {
    SDL_LockMutex(p->las_mutex);
    p->bytes_read += bytes_read;
    SDL_UnlockMutex(p->las_mutex);
}

static void algo_cond_wait(PlayList* p) {
    SDL_LockMutex(p->las_mutex);
    SDL_CondWaitTimeout(p->algo_cond, p->las_mutex, TIME_ALGO_UPDATE_INTERVAL_MS);
    SDL_UnlockMutex(p->las_mutex);
}

static void algo_cond_signal(PlayList* p) {
    SDL_LockMutex(p->las_mutex);
    SDL_CondSignal(p->algo_cond);
    SDL_UnlockMutex(p->las_mutex);
}

#pragma mark TagQueue
static int FlvTag_has_consume_all_data_l(struct FlvTag* tag) {
    if (tag->tag_size <= 0) {
        return 1;
    }

    // if reader->buf_size = 0 or reader->gop_size = 0, means that the reader is going to download data ,but not stared yet
    int ret = (tag->tag_size == tag->buf_read_offset);
    return ret;
}

int FlvTag_get_data_from_buffer(PlayList* playlist, struct FlvTag* tag, uint8_t* buf, uint32_t buf_size) {
    if (FlvTag_has_consume_all_data_l(tag)) {
        log_error("FlvTag_has_consume_all_data_l, illegal state");
        return -1;
    }

    int to_read = FFMIN(buf_size, tag->buf_write_offset - tag->buf_read_offset);

    memcpy(buf, tag->buf + tag->buf_read_offset, to_read);
    tag->buf_read_offset += to_read;

    return to_read;
}

int FlvTag_alloc_buffer(PlayList* playlist, struct FlvTag* tag, int32_t tag_size) {
    tag->buf = av_malloc(tag_size);
    if (!tag->buf) {
        log_error("alloc tag->buf fail");
        return AVERROR(ENOMEM);
    }

    tag->tag_size = tag_size;
    tag->buf_read_offset = tag->buf_write_offset = 0;
    return 0;
}

void FlvTag_dealloc(struct FlvTag* tag) {
    if (!tag) {
        return;
    }

    if (tag->buf) {
        av_freep(&tag->buf);
    }
    tag->tag_size = tag->buf_read_offset = tag->buf_write_offset = 0;
}

static int TagQueue_init(PlayList* playlist, TagQueue* q) {
    memset(q, 0, sizeof(TagQueue));
    q->mutex = SDL_CreateMutex();
    if (!q->mutex) {
        log_error("SDL_CreateMutex():fail");
        return AVERROR(ENOMEM);
    }
    q->cond = SDL_CreateCond();
    if (!q->cond) {
        log_error("SDL_CreateCond():fail");
        return AVERROR(ENOMEM);
    }
    q->abort_request = 1;
    return 0;
}

static void TagQueue_start(TagQueue* q) {
    SDL_LockMutex(q->mutex);
    q->abort_request = 0;
    SDL_UnlockMutex(q->mutex);
}

static int TagQueue_put_private(TagQueue* q, FlvTag* tag) {
    TagListNode* tag1;

    if (q->abort_request)
        return -1;

    tag1 = av_malloc(sizeof(TagListNode));
    if (!tag1)
        return -1;

    tag1->tag = *tag;
    tag1->next = NULL;

    if (!q->last_tag)
        q->first_tag = tag1;
    else
        q->last_tag->next = tag1;

    q->last_tag = tag1;
    q->nb_tags++;
    if (tag->type == FLV_TAG_TYPE_VIDEO)
        q->last_video_ts = tag->av_tag_ts;
    q->total_tag_bytes += tag->tag_size;
    SDL_CondSignal(q->cond);
    return 0;
}

static int TagQueue_put(TagQueue* q, FlvTag* tag) {
    int ret;

    SDL_LockMutex(q->mutex);
    ret = TagQueue_put_private(q, tag);
    SDL_UnlockMutex(q->mutex);

    if (ret < 0) {
        FlvTag_dealloc(tag);
    }

    return ret;
}

static int TagQueue_peek_first_video_ts(TagQueue* q) {
    TagListNode* tag_node, *tag_node1;
    int ret = -1;

    SDL_LockMutex(q->mutex);
    for (tag_node = q->first_tag; tag_node; tag_node = tag_node1) {
        tag_node1 = tag_node->next;
        if (tag_node->tag.type == FLV_TAG_TYPE_VIDEO) {
            ret = tag_node->tag.av_tag_ts;
            break;
        }
    }
    SDL_UnlockMutex(q->mutex);

    return ret;
}


/* return < 0 if aborted, 0 if no tag and > 0 if has tag.  */
static int TagQueue_get(TagQueue* q, FlvTag* tag, int block) {
    TagListNode* tag_node;
    int ret;

    SDL_LockMutex(q->mutex);

    for (;;) {
        if (q->abort_request) {
            ret = -1;
            break;
        }

        tag_node = q->first_tag;
        if (tag_node) {
            q->first_tag = tag_node->next;
            if (!q->first_tag)
                q->last_tag = NULL;
            q->nb_tags--;
            *tag = tag_node->tag;
            // q->total_tag_bytes -= tag->tag_size;
            av_free(tag_node);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            SDL_CondWait(q->cond, q->mutex);
        }
    }

    SDL_UnlockMutex(q->mutex);
    return ret;
}


static void TagQueue_flush(TagQueue* q) {
    TagListNode* tag_node, *tag_node_next;

    SDL_LockMutex(q->mutex);
    for (tag_node = q->first_tag; tag_node; tag_node = tag_node_next) {
        tag_node_next = tag_node->next;
        FlvTag_dealloc(&tag_node->tag);
        av_freep(&tag_node);
    }
    q->last_tag = NULL;
    q->first_tag = NULL;
    q->nb_tags = 0;
    q->last_video_ts = 0;
    SDL_UnlockMutex(q->mutex);
}

static void TagQueue_destroy(TagQueue* q) {
    TagQueue_flush(q);
    SDL_DestroyMutex(q->mutex);
    SDL_DestroyCond(q->cond);
}

static void TagQueue_abort(TagQueue* q) {
    SDL_LockMutex(q->mutex);
    q->abort_request = 1;
    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);
}

static int32_t TagQueue_get_duration_ms(TagQueue* q) {
    int32_t ret = 0;

    int first_ts = TagQueue_peek_first_video_ts(q);

    if (first_ts >= 0) {
        SDL_LockMutex(q->mutex);
        ret = q->last_video_ts - first_ts;
        SDL_UnlockMutex(q->mutex);
    }

    return ret > 0 ? ret : 0;
}

static int64_t TagQueue_get_total_bytes(TagQueue* q) {
    int64_t ret = 0;

    SDL_LockMutex(q->mutex);
    ret = q->total_tag_bytes;
    SDL_UnlockMutex(q->mutex);

    return ret > 0 ? ret : 0;
}


#pragma mark LasStatistic
int32_t get_video_bitrate(MultiRateAdaption* thiz) {
    return thiz->levels[thiz->current];
}

int32_t get_buffer_current(MultiRateAdaption* thiz) {
    return thiz->last_check_buffer;
}

int32_t get_bw_fragment(MultiRateAdaption* thiz) {
    return thiz->last_speed;
}

void LasStatistic_reset(LasStatistic* stat) {
    if (stat) {
        memset(stat, 0, sizeof(LasStatistic));
    }
}

void LasStatistic_init(LasStatistic* stat, PlayList* playlist) {
    LasStatistic_reset(stat);

    stat->flv_nb = playlist->adaptation_set.n_representation;
    for (int i = 0; i < playlist->adaptation_set.n_representation; i++) {
        stat->flvs[i].total_bandwidth_kbps = playlist->adaptation_set.representations[i]->bitrate;
        strncpy(stat->flvs[i].url, playlist->adaptation_set.representations[i]->url, MAX_URL_SIZE - 1);
    }
}

void LasStatistic_on_rep_http_url(LasStatistic* stat, char* request_url) {
    if (stat) {
        strncpy(stat->cur_playing_url, request_url, MAX_URL_SIZE - 1);
    }
}

void LasStatistic_on_rep_http_start(LasStatistic* stat, int64_t start_time) {
    if (stat) {
        stat->cur_rep_read_start_time = start_time;
    }
}

void LasStatistic_on_rep_http_open(LasStatistic* stat, int64_t open_time) {
    if (stat) {
        stat->cur_rep_http_open_time = open_time;
    }
}

void LasStatistic_on_rep_flv_header(LasStatistic* stat, int64_t header_time) {
    if (stat) {
        stat->cur_rep_read_header_time = header_time;
    }
}

void LasStatistic_on_rep_http_first_data(LasStatistic* stat, int64_t first_data_time) {
    if (stat) {
        stat->cur_rep_first_data_time = first_data_time;
    }
}

void LasStatistic_on_rep_start_timestamp(PlayList* playlist, int64_t start_time, int64_t request_time) {
    LasStatistic* stat = playlist->las_statistic;
    if (stat) {
        stat->cur_rep_start_time = start_time;
        stat->rep_switch_gap_time = request_time <= 0 ? 0 : start_time - request_time;
        log_info("rep_switch_gap_time=%lld", stat->rep_switch_gap_time);
    }
}

void LasStatistic_on_rep_read_error(LasStatistic* stat, int error) {
    if (stat) {
        stat->cur_rep_http_reading_error = error;
    }
}

void LasStatistic_on_read_packet(LasStatistic* stat, PlayList* playlist) {
    if (stat && playlist) {
        stat->cur_decoding_flv_index = playlist->cur_rep_index;
    }
}

void LasStatistic_on_buffer_time(LasStatistic* stat, PlayList* playlist) {
    if (stat && playlist) {
        stat->cached_a_dur_ms = las_get_audio_cached_duration_ms(playlist->las_player_statistic);
        stat->cached_v_dur_ms = las_get_video_cached_duration_ms(playlist->las_player_statistic);
        stat->cached_tag_dur_ms = TagQueue_get_duration_ms(&playlist->tag_queue);
        log_info("a_buffer_time_ms=%lld, v_buffer_time_ms=%lld, CachedTagQueue_ms=%lld",
                 stat->cached_a_dur_ms, stat->cached_v_dur_ms, stat->cached_tag_dur_ms);
    }
}

void LasStatistic_on_adaption_adapted(PlayList* playlist, MultiRateAdaption* adaption) {
    LasStatistic* stat = playlist->las_statistic;
    if (stat && adaption) {
        stat->bitrate_downloading = get_video_bitrate(adaption);
    }
}

void LasStatistic_on_bytes_downloaded(LasStatistic* stat, int64_t bytes) {
    if (stat) {
        stat->total_bytes_read += bytes;
    }
}

void LasStatistic_on_bandwidth_update(PlayList* playlist, MultiRateAdaption* adaption) {
    LasStatistic* stat = playlist->las_statistic;
    if (stat && adaption) {
        stat->bandwidth_fragment = get_bw_fragment(adaption);
        stat->current_buffer_ms = get_buffer_current(adaption);
    }
}

void LasStatistic_on_rep_switch_count(LasStatistic* stat, PlayList* playlist) {
    if (stat) {
        stat->rep_switch_cnt++;
        stat->switch_point_a_buffer_ms = las_get_audio_cached_duration_ms(playlist->las_player_statistic);
        stat->switch_point_v_buffer_ms = las_get_video_cached_duration_ms(playlist->las_player_statistic);
    }
}

#pragma mark MultiRateAdaption
int32_t local_index_2_rep_index(MultiRateAdaption* thiz, int32_t local_index) {
    int32_t rep_index = 0;
    for (int i = 0; i < thiz->n_bitrates; i++) {
        if (thiz->levels[local_index] == thiz->bitrate_table_origin_order[i]) {
            rep_index = i;
            break;
        }
    }
    return rep_index;
}

int32_t rep_index_2_local_index(MultiRateAdaption* thiz, int32_t rep_index) {
    int32_t local_index = 0;
    for (int i = 0; i < thiz->n_bitrates; i++) {
        if (thiz->levels[i] == thiz->bitrate_table_origin_order[rep_index]) {
            local_index = i;
            break;
        }
    }
    return local_index;

}

int get_local_index_from_bitrate(MultiRateAdaption* thiz, int64_t bitrate) {
    for (int32_t i = thiz->n_bitrates - 1; i > 0; --i) {
        if (thiz->levels[i] <= bitrate) {
            return i;
        }
    }
    return 0;
}

int compare(const void* a, const void* b) {
    return (*(int32_t*)a - * (int32_t*)b);
}

void RateAdaptConfig_default_init(AdaptiveConfig* config, PlayList* playlist) {
    config->buffer_init = 2000;
    config->stable_buffer_diff_threshold_second = 0.15;
    config->stable_buffer_interval_ms = 2000;
    config->generate_speed_gap_ms = 3000;
    config->buffer_check_interval_ms = 500;
    config->smoothed_speed_utilization_ratio = 0.8;
    config->small_speed_to_bitrate_ratio = 0.4;
    config->enough_speed_to_bitrate_ratio = 0.9;
    config->buffer_lower_limit_second = 0.6;
    config->recent_buffered_size = 16;
    config->smoothed_speed_ratio = 0.9;
}


// return index of optimized Representation
void MultiRateAdaption_init(MultiRateAdaption* thiz, AdaptiveConfig config,
                            struct PlayList* playlist) {
    if (!thiz || !playlist || playlist->adaptation_set.n_representation <= 0) {
        log_error("thiz:%p, p:%p", thiz, playlist);
        return;
    }
    thiz->conf = config;
    thiz->n_bitrates = 0;
    thiz->playlist = playlist;
    thiz->session_id = playlist->session_id;
    int64_t default_select_bitrate = -1;
    for (int i = 0; i < playlist->adaptation_set.n_representation; i++) {
        Representation* rep = playlist->adaptation_set.representations[i];
        thiz->bitrate_table_origin_order[i] = rep->bitrate;
        thiz->levels[i] = rep->bitrate;
        if (rep->default_selected) {
            default_select_bitrate = rep->bitrate;
        }
        thiz->disable_adaptive_table[i] = rep->disabled_from_adaptive;
        thiz->n_bitrates++;
    }
    qsort(thiz->levels, thiz->n_bitrates, sizeof(int32_t), compare);

    thiz->buffer_init = config.buffer_init;
    if (thiz->buffer_init > INIT_BUFFER_THRESHOLD_MAX_MS) {
        thiz->buffer_init = INIT_BUFFER_THRESHOLD_MAX_MS;
    }

    if (default_select_bitrate >= 0) {
        thiz->current = get_local_index_from_bitrate(thiz, default_select_bitrate);
    } else {
        thiz->current = (thiz->n_bitrates - 1) / 2;
    }
    while (thiz->current >= thiz->n_bitrates) {
        thiz->current -= 1;
    }

    int switch_mode = las_get_switch_mode(playlist->las_player_statistic);
    if (switch_mode >= 0 && switch_mode < thiz->n_bitrates) {
        thiz->current = rep_index_2_local_index(thiz, switch_mode);
    }

    LasStatistic_on_adaption_adapted(thiz->playlist, thiz);
    thiz->next_expected_rep_index = local_index_2_rep_index(thiz, thiz->current);
    thiz->past_buffer[0] = 0.1;
    thiz->buffer_index = 1;
    thiz->stable_buffer_start_time = get_current_time_ms();
    thiz->generated_speed = 0;
    thiz->last_check_buffer = 0;
    thiz->last_speed = 0;
}

bool update_stable_buffer(MultiRateAdaption* thiz, double buffered) {
    double diff = buffered - thiz->last_check_buffer;
    double diff_ratio = diff / buffered;
    double now = get_current_time_ms();
    if (diff < -thiz->conf.stable_buffer_diff_threshold_second || diff_ratio < -0.2) {
        algo_info("buffer_diff_down: %.2fs, diff_ratio: %.2f", diff, diff_ratio);
        thiz->stable_buffer_start_time = FFMAX(now, thiz->stable_buffer_start_time);
    }
    if (diff > thiz->conf.stable_buffer_diff_threshold_second
        && now - thiz->stable_buffer_start_time + thiz->conf.buffer_check_interval_ms > thiz->conf.stable_buffer_interval_ms) {
        thiz->stable_buffer_start_time = FFMAX(
            now - thiz->conf.buffer_check_interval_ms * 2,
            thiz->stable_buffer_start_time + thiz->conf.buffer_check_interval_ms * 2
        );
        algo_info("buffer_diff_up: %.2fs", diff);
    }
    thiz->last_check_buffer = buffered;
    return now - thiz->stable_buffer_start_time > thiz->conf.stable_buffer_interval_ms;
}

/**
 * check buffer periodically
 */
void check_buffer(MultiRateAdaption* thiz, PlayList* playlist) {
    double buffered = las_get_audio_cached_duration_ms(playlist->las_player_statistic) / 1000.0;
    bool is_buffer_stable = update_stable_buffer(thiz, buffered);
    if (is_buffer_stable && thiz->current + 1 < thiz->n_bitrates) {
        thiz->generated_speed = thiz->levels[thiz->current + 1];
    } else {
        thiz->generated_speed = 0;
    }

    thiz->past_buffer[thiz->buffer_index % thiz->conf.recent_buffered_size] = buffered;
    thiz->buffer_index += 1;
}

int32_t quantization(MultiRateAdaption* thiz, double speed) {
    int32_t index = 0;
    for (int i = thiz->n_bitrates - 1; i >= 0; i--) {
        if (speed >= thiz->levels[i]) {
            index = i;
            break;
        }
    }
    return index;
}

double get_past_buffer(MultiRateAdaption* thiz) {
    double max_buffer = 0.1;
    for (int i = 0; i < thiz->conf.recent_buffered_size && i < thiz->buffer_index; ++i) {
        double buffered = thiz->past_buffer[(thiz->buffer_index - 1 - i) % thiz->conf.recent_buffered_size];
        if (buffered > max_buffer) {
            max_buffer = buffered;
        }
    }
    return max_buffer;
}

double get_smoothed_speed(MultiRateAdaption* thiz, double speed) {
    if (thiz->last_speed > 0) {
        return speed * (1 - thiz->conf.smoothed_speed_ratio) + thiz->last_speed * thiz->conf.smoothed_speed_ratio;
    }
    return speed;
}

double get_predicted_buffer(MultiRateAdaption* thiz, double buffered) {
    double past_buffer = get_past_buffer(thiz);
    return buffered + (buffered - past_buffer);
}

double get_buffer_speed(MultiRateAdaption* thiz, double buffered) {
    double past_buffer = get_past_buffer(thiz);
    double buffer_speed_ratio = 1 + (buffered - past_buffer) / FFMAX(past_buffer, 0.1);
    return buffer_speed_ratio * thiz->levels[thiz->current];
}

bool is_speed_too_small(MultiRateAdaption* thiz, double speed) {
    return speed / thiz->levels[thiz->current] < thiz->conf.small_speed_to_bitrate_ratio;
}

bool is_speed_enough(MultiRateAdaption* thiz, double speed) {
    return speed / thiz->levels[thiz->current] > thiz->conf.enough_speed_to_bitrate_ratio;
}

int32_t next_local_rate_index(MultiRateAdaption* thiz, double speed, double buffered) {
    double buffer_speed = get_buffer_speed(thiz, buffered);
    double smoothed_speed = get_smoothed_speed(thiz, speed);
    algo_info("gop_speed: %.0f, smoothed_speed: %.0f", speed, smoothed_speed);

    double predicted_buffered = get_predicted_buffer(thiz, buffered);
    algo_info("s: %.0f, predicted_buffered: %.1f", buffer_speed, predicted_buffered);

    int32_t next_index = thiz->current;
    if (predicted_buffered < thiz->conf.buffer_lower_limit_second
        || is_speed_too_small(thiz, buffer_speed)) {
        next_index = FFMIN(thiz->current, quantization(thiz, buffer_speed));
    } else if (is_speed_enough(thiz, buffer_speed)) {
        if (thiz->generated_speed > 0) {
            algo_info("generated_speed used");
            next_index = quantization(thiz, thiz->generated_speed);
            thiz->generated_speed = 0;
        } else {
            next_index = quantization(thiz, smoothed_speed * thiz->conf.smoothed_speed_utilization_ratio);
        }
        next_index = FFMIN(thiz->current + 1, FFMAX(next_index, thiz->current));
    }
    algo_info("target_index = %u", next_index);
    return next_index;
}

int32_t next_representation_id(MultiRateAdaption* thiz, int switch_mode, double speed, double buffered) {
    if (switch_mode >= 0 && switch_mode < thiz->n_bitrates) {
        thiz->current = rep_index_2_local_index(thiz, switch_mode);
        return switch_mode;
    }

    int local_index = next_local_rate_index(thiz, speed, buffered);
    int rep_index = local_index_2_rep_index(thiz, local_index);
    while (local_index > 0 && thiz->disable_adaptive_table[rep_index]) {
        local_index -= 1;
        rep_index = local_index_2_rep_index(thiz, local_index);
    }

    if (local_index != thiz->current) {
        thiz->stable_buffer_start_time = get_current_time_ms() + thiz->conf.generate_speed_gap_ms;
    }
    if (local_index < thiz->current) {
        thiz->generated_speed = 0;
        thiz->last_speed = speed;
        thiz->buffer_index = 1;
        thiz->past_buffer[0] = buffered;
    } else {
        thiz->last_speed = get_smoothed_speed(thiz, speed);
    }

    thiz->current = local_index;
    return rep_index;
}

#pragma mark Download
static void update_options(char** dest, const char* name, void* src) {
    av_freep(dest);
    av_opt_get(src, name, 0, (uint8_t**)dest);
    if (*dest && !strlen(*dest))
        av_freep(dest);
}

static int open_url(LasContext* c, URLContext** uc, const char* url,
                    AVDictionary* opts, AVDictionary* opts2, PlayList* playlist) {
    AVDictionary* tmp = NULL;
    const char* proto_name = NULL;
    int ret;

    av_dict_copy(&tmp, opts, 0);
    av_dict_copy(&tmp, opts2, 0);

    if (!proto_name) {
        proto_name = avio_find_protocol_name(url);
    }

    if (!proto_name)
        return AVERROR_INVALIDDATA;

    ret = ffurl_open_whitelist(uc, url,  AVIO_FLAG_READ, c->interrupt_callback, &tmp, c->ctx->protocol_whitelist, c->ctx->protocol_blacklist, c->ctx);
    if (ret >= 0) {
        log_info("ffurl_open_whitelist succeeds");
        // update cookies on http response with setcookies.
        char* new_cookies = NULL;
        if (!(c->ctx->flags & AVFMT_FLAG_CUSTOM_IO))
            av_opt_get(*uc, "cookies", AV_OPT_SEARCH_CHILDREN, (uint8_t**)&new_cookies);
        if (new_cookies) {
            if (c->cookies) {
                av_free(c->cookies);
            }
            c->cookies = new_cookies;
        }

        // update cookies on http response with setcookies.
        URLContext* u = *uc;
        update_options(&c->cookies, "cookies", u->priv_data);
        av_dict_set(&opts, "cookies", c->cookies, 0);
    } else {
        log_error("ffurl_open_whitelist fails: %s(0x%x)", av_err2str(ret), ret);
    }

    av_dict_copy(&c->ctx->metadata, tmp, 0);
    av_dict_free(&tmp);
    return ret;
}

enum ReadFromURLMode {
    READ_NORMAL,
    READ_COMPLETE,
};

/*
 * This is actually read from the network, there is no buffer.
 * There are 12 bytes of redundancy in a 4096-byte tcp packet at most,
 * and other non-4096-byte packets are not redundant.
*/
static int read_from_url(URLContext* url_ctx,
                         uint8_t* buf, int buf_size,
                         enum ReadFromURLMode mode,
                         PlayList* playlist) {
    int ret;

    if (mode == READ_COMPLETE)
        ret = ffurl_read_complete(url_ctx, buf, buf_size);
    else
        ret = ffurl_read(url_ctx, buf, buf_size);

    if (ret > 0) {
        add_bytes_read(playlist, ret);
        LasStatistic_on_bytes_downloaded(playlist->las_statistic, ret);
    }

    return ret;
}

static int url_block_read(URLContext* url_ctx, uint8_t* buf, int want_len, PlayList* playlist) {
    int offset = 0;
    int ret;

    int remain = want_len;
    while (remain > 0) {
        ret = read_from_url(url_ctx, buf + offset, remain, READ_NORMAL, playlist);
        if (ret <= 0) {
            if (ret < 0) {
                log_error("read_from_url fails: %s(0x%x)", av_err2str(ret), ret);
            } else {
                //ffurl_read means socket was closed by peer
                ret = LAS_ERROR_SOCKET_CLOSED_BY_PEER;
                log_error("read_from_url socket closed by peer");
            }
            return ret;
        }
        offset += ret;
        remain -= ret;
    }

    if (remain != 0) {
        log_error("block_read fail, remain:%d", remain);
        return -1;
    } else {
        return want_len;
    }
}

#pragma mark Gop
void GopReader_init(GopReader* reader, Representation* rep, AVFormatContext* s, PlayList* playlist) {
    memset(reader->realtime_url, 0, sizeof(reader->realtime_url));
    strcat(reader->realtime_url, rep->url);

    if (strstr(reader->realtime_url, "?") != NULL) {
        strcat(reader->realtime_url, "&");
    } else {
        strcat(reader->realtime_url, "?");
    }
    // Tag based
    char str_starttime[256] = "\0";
    sprintf(str_starttime, "startPts=%" PRId64, reader->last_gop_start_ts);
    strcat(reader->realtime_url, str_starttime);

    if (reader->is_audio_only) {
        strcat(reader->realtime_url, "&audioOnly=true");
    }
    reader->rep_index = rep->index;
    reader->parent = s;
    log_error("rep->index:%d, realtime_url:%s", rep->index, reader->realtime_url);
}

int GopReader_open_input(GopReader* reader, LasContext* c, PlayList* playlist) {
    AVDictionary* opts = NULL;
    int ret = 0;

    // broker prior HTTP options that should be consistent across requests
    av_dict_set(&opts, "user_agent", c->user_agent, 0);
    av_dict_set(&opts, "cookies", c->cookies, 0);
    av_dict_set(&opts, "headers", c->headers, 0);
    av_dict_set(&opts, "http_proxy", c->http_proxy, 0);
    av_dict_set(&opts, "seekable", "0", 0);

    LasStatistic_on_rep_http_url(c->playlist.las_statistic, reader->realtime_url);
    ret = open_url(c, &reader->input, reader->realtime_url, c->avio_opts, opts, playlist);

    av_dict_free(&opts);
    return ret;
}

void GopReader_close(GopReader* reader, PlayList* playlist) {
    if (reader->rep_index < 0) {
        return;
    }
    ffurl_closep(&reader->input);
    log_info("ffurl_closep(rep_index: %d)", reader->rep_index);
    reader->switch_index += 1;
}

static int PlayList_algo_statistic_thread(void* data);
int64_t GopReader_download_gop(GopReader* reader, MultiRateAdaption* adaption, PlayList* playlist) {
    LasContext* c = reader->parent->priv_data;

    uint8_t flv_header[FLV_HEADER_LEN + PRE_TAG_SIZE_LEN];

    int ret = -1;
    bool rep_changed = true;
    bool first_video_tag = true;
    int to_read;

    int64_t start_time = 0;
    int64_t http_open_time = 0;
    int64_t first_data_time = 0;  // got first tag time

    start_time = get_current_time_ms();
    LasStatistic_on_rep_http_start(playlist->las_statistic, start_time);
    LasStatistic_on_rep_http_first_data(playlist->las_statistic, 0);

    if (!reader->input) {
        ret = GopReader_open_input(reader, c, playlist);
        if (ret < 0) {
            return ret;
        }

        http_open_time = get_current_time_ms();
        LasStatistic_on_rep_http_open(playlist->las_statistic, http_open_time - start_time);

        memset(flv_header, 0, FLV_HEADER_LEN + PRE_TAG_SIZE_LEN);
        ret = url_block_read(reader->input, flv_header, FLV_HEADER_LEN + PRE_TAG_SIZE_LEN, playlist);
        LasStatistic_on_rep_flv_header(playlist->las_statistic, get_current_time_ms() - http_open_time);
        if (ret < 0) {
            return ret;
        }
    }

    // start algo update thread
    if (!playlist->algo_thread) {
        playlist->algo_thread = SDL_CreateThreadEx(&playlist->_algo_thread, PlayList_algo_statistic_thread, playlist, "playlist-algo-statistic-thread");
    }

    //las 2.0 Tag based reading
    uint8_t av_tag_header[AV_TAG_HEADER_LEN];
    int gop_duration = playlist->adaptation_set.duration;
    int64_t bytes_last = get_bytes_read(playlist);
    int64_t time_last = get_current_time_ms();

    while (1) {
        if (playlist->read_abort_request || playlist->tag_queue.abort_request) {
            return LAS_ERROR_ABORT_BY_USER;
        }
        ret = playlist->error_code;
        if (ret < 0) {
            playlist->error_code = 0;
            return ret;
        }

        int request = las_get_audio_only_request(playlist->las_player_statistic);
        if (reader->is_audio_only != request) {
            reader->is_audio_only = request;
            int64_t current_playing_audio_ts = las_get_first_audio_packet_pts(playlist->las_player_statistic);
            log_info("current_playing_audio_ts: %lld", current_playing_audio_ts);
            int64_t request_ts = current_playing_audio_ts - playlist->adaptation_set.duration / 2;
            reader->last_gop_start_ts = request_ts > 0 ? request_ts : 0;
            return 0;
        }

        memset(av_tag_header, 0, AV_TAG_HEADER_LEN);
        to_read = AV_TAG_HEADER_LEN;
        ret = url_block_read(reader->input, av_tag_header, to_read, playlist);
        if (ret < 0) {
            return ret;
        }

        if (av_tag_header[0] == FLV_TAG_TYPE_VIDEO && ((av_tag_header[AV_TAG_HEADER_LEN - 5] >> 4) & 0x0F) == 1
            && av_tag_header[AV_TAG_HEADER_LEN - 4] == 1) {
            //IDR, switch edge
            uint32_t new_rep_start_pts = AV_RB24(av_tag_header + 4);
            new_rep_start_pts |= (unsigned)AV_RB8(av_tag_header + 7) << 24;
            uint32_t cts = (AV_RB24(av_tag_header + 13) + 0xff800000) ^ 0xff800000;
            new_rep_start_pts += cts;
            log_info("video key Frame, pts=%lld, parameters=0x%x, video packet_type=0x%x",
                     new_rep_start_pts, av_tag_header[AV_TAG_HEADER_LEN - 5], av_tag_header[AV_TAG_HEADER_LEN - 4]);
            if (!first_video_tag) {
                reader->last_gop_start_ts = new_rep_start_pts;
                int64_t bytes_now = get_bytes_read(playlist);
                int64_t time_now = get_current_time_ms();
                int64_t speed = (bytes_now - bytes_last) * 8 / FFMAX(time_now - time_last, 50);
                bytes_last = bytes_now;
                time_last = time_now;

                adaption->next_expected_rep_index = next_representation_id(
                        adaption,
                        las_get_switch_mode(playlist->las_player_statistic),
                        speed,
                        las_get_audio_cached_duration_ms(playlist->las_player_statistic) / 1000.0);
                if (reader->rep_index != adaption->next_expected_rep_index) {
                    LasStatistic_on_adaption_adapted(playlist, adaption);
                    LasStatistic_on_rep_switch_count(playlist->las_statistic, playlist);
                    return 0; // switch url
                }
            } else {
                first_video_tag = false;
                LasStatistic_on_rep_start_timestamp(playlist, new_rep_start_pts, reader->last_gop_start_ts);
            }
        }

        //av_malloc tag size, -1(av parameter bytes) + -1(video packet_type) + -3(CompositionTime) + 4(pre tag size)
        to_read = AV_RB24(av_tag_header + 1) - 1;

        FlvTag tag;
        memset(&tag, 0, sizeof(FlvTag));

        if (av_tag_header[0] == FLV_TAG_TYPE_VIDEO || av_tag_header[0] == FLV_TAG_TYPE_AUDIO) {
            tag.av_tag_ts = AV_RB24(av_tag_header + 4);
            tag.av_tag_ts |= (unsigned)AV_RB8(av_tag_header + 7) << 24;
            tag.type = av_tag_header[0];
        }

        int tag_size = 0;
        if (rep_changed)
            tag_size = to_read + AV_TAG_HEADER_LEN + FLV_HEADER_LEN + PRE_TAG_SIZE_LEN;
        else
            tag_size = to_read + AV_TAG_HEADER_LEN;

        ret = FlvTag_alloc_buffer(playlist, &tag, tag_size);
        if (ret) {
            return ret;
        }

        if (rep_changed) {
            memcpy(tag.buf + tag.buf_write_offset, flv_header, FLV_HEADER_LEN + PRE_TAG_SIZE_LEN);
            tag.buf_write_offset += FLV_HEADER_LEN + PRE_TAG_SIZE_LEN;
        }

        memcpy(tag.buf + tag.buf_write_offset, av_tag_header, AV_TAG_HEADER_LEN);
        tag.buf_write_offset += AV_TAG_HEADER_LEN;

        if (playlist->read_abort_request || playlist->tag_queue.abort_request) {
            FlvTag_dealloc(&tag);
            return LAS_ERROR_ABORT_BY_USER;
        }
        ret = url_block_read(reader->input, tag.buf + tag.buf_write_offset, to_read, playlist);
        if (ret < 0) {
            FlvTag_dealloc(&tag);
            return ret;
        } else {
            tag.buf_write_offset += to_read;
        }

        if (tag.buf_write_offset != tag.tag_size) {
            log_error("ERROR! tag.buf_write_offset(%d) != tag.tag_size(%d)", tag.buf_write_offset, tag.tag_size);
            FlvTag_dealloc(&tag);
            return LAS_ERROR_GET_WHOLE_TAG;
        }

        if (first_data_time == 0) {
            first_data_time = get_current_time_ms();
            LasStatistic_on_rep_http_first_data(playlist->las_statistic, first_data_time);
        }

        tag.rep_index = playlist->gop_reader.rep_index;
        tag.audio_only = playlist->gop_reader.is_audio_only;
        tag.switch_index = playlist->gop_reader.switch_index;

        TagQueue_put(&playlist->tag_queue, &tag);

        if (rep_changed) {
            rep_changed = false;
        }
    }
}


#pragma mark PlayList
static int PlayList_prepare_reading_tag(PlayList* playlist) {
    SDL_LockMutex(playlist->reading_tag_mutex);
    int ret = 0;
    if (FlvTag_has_consume_all_data_l(&playlist->reading_tag)) {
        FlvTag_dealloc(&playlist->reading_tag);
        SDL_UnlockMutex(playlist->reading_tag_mutex);

        FlvTag tag;
        ret = TagQueue_get(&playlist->tag_queue, &tag, 1);
        if (ret < 0) {
            log_error("TagQueue_get fail");
        } else {
            SDL_LockMutex(playlist->reading_tag_mutex);
            playlist->reading_tag = tag;
            SDL_UnlockMutex(playlist->reading_tag_mutex);
        }
        return ret;
    } else {
        SDL_UnlockMutex(playlist->reading_tag_mutex);
        return 0;
    }
}

static int PlayList_read_from_reading_tag(PlayList* playlist, uint8_t* buf, uint32_t buf_size) {
    SDL_LockMutex(playlist->reading_tag_mutex);
    int ret = FlvTag_get_data_from_buffer(playlist, &playlist->reading_tag, buf, buf_size);
    SDL_UnlockMutex(playlist->reading_tag_mutex);
    return ret;
}

static int PlayList_read_data(void* opaque, uint8_t* buf, int buf_size) {
    PlayList* playlist = opaque;
    int ret;

    ret = PlayList_prepare_reading_tag(playlist);
    if (ret < 0) {
        return ret;
    }

    if (playlist->reading_tag.switch_index != playlist->cur_switch_index) {
        return AVERROR_EOF;
    }

    ret = PlayList_read_from_reading_tag(playlist, buf, buf_size);
    if (ret < 0) {
        return ret;
    }

    return ret;
}

static void PlayList_reset_state(PlayList* p) {
    p->parent = NULL;
    p->read_buffer = NULL;
    p->cur_switch_index = 0;
    p->cur_rep_index = p->multi_rate_adaption.next_expected_rep_index;
}

static int PlayList_algo_statistic_thread(void* data) {
    PlayList* playlist = (PlayList*)data;
    TagQueue* tag_queue = &playlist->tag_queue;
    while (!tag_queue->abort_request) {
        algo_cond_wait(playlist);
        if (tag_queue->abort_request || playlist->read_abort_request) {
            break;
        }
        check_buffer(&playlist->multi_rate_adaption, playlist);
        LasStatistic_on_bandwidth_update(playlist, &playlist->multi_rate_adaption);
    }
    return 0;
}

int PlayList_is_valid_index_l(PlayList* playlist, int index) {
    if (!playlist)
        return 0;
    return index >= 0 && index < playlist->adaptation_set.n_representation;
}

static int PlayList_read_thread(void* data) {
    PlayList* playlist = (PlayList*)data;
    log_info("Start las reading");

    AVFormatContext* s = playlist->outermost_ctx;
    GopReader* gop_reader = &playlist->gop_reader;
    TagQueue* tag_queue = &playlist->tag_queue;

    int64_t ret = 0;

    while (!tag_queue->abort_request) {
        // change GopReader if needed
        int new_index = playlist->multi_rate_adaption.next_expected_rep_index;
        if (!PlayList_is_valid_index_l(playlist, new_index)) {
            log_error("invalid rep index:%d, IGNORE!!!", new_index);
            break;
        }
        GopReader_close(gop_reader, playlist);
        Representation* rep = playlist->adaptation_set.representations[new_index];
        if (ff_check_interrupt(&s->interrupt_callback)) {
            break;
        }
        GopReader_init(gop_reader, rep, s, playlist);
        ret = GopReader_download_gop(gop_reader, &playlist->multi_rate_adaption, playlist);
        if (ret < 0) {
            LasStatistic_on_rep_read_error(playlist->las_statistic, ret);
            break;
        }
    }

    TagQueue_abort(&playlist->tag_queue);
    if (playlist->algo_thread) {
        log_info("Signals algo_thread");
        algo_cond_signal(playlist);
    }

    if (gop_reader->input) {
        log_info("Calls GopReader_close");
        GopReader_close(gop_reader, playlist);
    }

    log_error("Thread is over, playlist->read_abort_request=%d, ret:%s(0x%x)", playlist->read_abort_request, av_err2str(ret), ret);
    return playlist->read_abort_request ? 0 : ret;
}

int PlayList_open_rep(PlayList* playlist, FlvTag* tag, AVFormatContext* s) {
    int ret = 0;
    Representation* rep = NULL;

    if (!PlayList_is_valid_index_l(playlist, tag->rep_index)) {
        ret = LAS_ERROR_INVALID_REP_INDEX;
        goto fail;
    }

    rep = playlist->adaptation_set.representations[tag->rep_index];
    if (!(playlist->ctx = avformat_alloc_context())) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    playlist->read_buffer = av_malloc(INITIAL_BUFFER_SIZE);
    if (!playlist->read_buffer) {
        ret = AVERROR(ENOMEM);
        avformat_free_context(playlist->ctx);
        playlist->ctx = NULL;
        goto fail;
    }

    ffio_init_context(&playlist->pb, playlist->read_buffer, INITIAL_BUFFER_SIZE, 0, playlist,
                      PlayList_read_data, NULL, NULL);

    playlist->ctx->pb = &playlist->pb;
    playlist->ctx->flags |= s->flags & ~AVFMT_FLAG_CUSTOM_IO;

    SDL_LockMutex(playlist->rw_mutex);
    playlist->cur_switch_index = tag->switch_index;
    SDL_UnlockMutex(playlist->rw_mutex);

    playlist->ctx->fps_probe_size = 0;

    // fix me ,url should be read reading_gop 's url
    ret = avformat_open_input(&playlist->ctx, playlist->gop_reader.realtime_url, NULL, NULL);
    if (ret < 0) {
        if (playlist->read_thread && playlist->read_thread->retval) {
            log_error("PlayList_read_thread() already Fails!");
            ret = playlist->read_thread->retval;
        }
        log_error("avformat_open_input() ret:%s(0x%x)", av_err2str(ret), ret);
        goto fail;
    }

    ret = avformat_find_stream_info(playlist->ctx, NULL);
    if (ret < 0) {
        goto fail;
    }

    // transcoder_group used in current flv stream
    AVDictionaryEntry* tsc_group = av_dict_get(playlist->ctx->metadata, "tsc_group", NULL, 0);
    if (tsc_group && tsc_group->value) {
        av_dict_set(&playlist->outermost_ctx->metadata, "tsc_group", tsc_group->value, 0);
    }

    if (!playlist->is_stream_ever_opened) {
        // first inited, add streams to outermost AVFormatContext
        /* Create new AVStreams for each stream in this playlist */
        for (int j = 0; j < playlist->ctx->nb_streams; j++) {
            AVStream* st = avformat_new_stream(s, NULL);
            AVStream* ist = playlist->ctx->streams[j];
            if (!st) {
                ret = AVERROR(ENOMEM);
                goto fail;
            }

            st->id = 0;

            avcodec_parameters_copy(st->codecpar, playlist->ctx->streams[j]->codecpar);
            avpriv_set_pts_info(st, ist->pts_wrap_bits, ist->time_base.num, ist->time_base.den);
        }
        playlist->is_stream_ever_opened = true;
    } else {
        las_set_stream_reopened(playlist->las_player_statistic, true);
    }

    for (int j = 0; j < playlist->ctx->nb_streams && j < MAX_STREAM_NUM; j++) {
        AVCodecParameters* new_codec = playlist->ctx->streams[j]->codecpar;
        for (int k = 0; k < s->nb_streams; k++) {
            AVCodecParameters* old_codec = s->streams[k]->codecpar;
            if (new_codec->codec_type == old_codec->codec_type) {
                playlist->stream_index_map[j] = k;
                break;
            }
        }
    }

    playlist->cur_rep_index = tag->rep_index;
    las_set_audio_only_response(playlist->las_player_statistic, tag->audio_only);
    log_info("open_index:%d, audio_only:%d finished", tag->rep_index, tag->audio_only);
    return 0;

fail:
    return ret;
}

int PlayList_open_read_thread(PlayList* playlist) {
    int ret;
    AVFormatContext* s = playlist->outermost_ctx;
    playlist->read_abort_request = 0;

    playlist->rw_mutex = SDL_CreateMutex();
    if (!playlist->rw_mutex) {
        log_error("SDL_CreateMutex playlist->rw_mutex fail");
        return LAS_ERROR_MUTEX_CREATE;
    }

    playlist->reading_tag_mutex = SDL_CreateMutex();
    if (!playlist->reading_tag_mutex) {
        log_error("SDL_CreateMutex playlist->reading_tag_mutex fail");
        return LAS_ERROR_MUTEX_CREATE;
    }

    playlist->las_mutex = SDL_CreateMutex();
    if (!playlist->las_mutex) {
        log_error("SDL_CreateMutex playlist->las_mutex fail");
        return LAS_ERROR_MUTEX_CREATE;
    }

    playlist->algo_cond = SDL_CreateCond();
    if (!playlist->algo_cond) {
        log_error("SDL_CreateCond playlist->algo_cond fail");
        return LAS_ERROR_COND_CREATE;
    }

    // init and start TagQueue
    TagQueue_init(playlist, &playlist->tag_queue);
    TagQueue_start(&playlist->tag_queue);

    // init GopReader
    playlist->gop_reader.switch_index = 0;
    playlist->gop_reader.rep_index = -1;
    playlist->gop_reader.last_gop_start_ts = (int)(playlist->multi_rate_adaption.buffer_init) * (-1);

    // start downloading thread
    playlist->read_thread = SDL_CreateThreadEx(&playlist->_read_thread, PlayList_read_thread, playlist, "playlist-read-thread");
    if (!playlist->read_thread) {
        log_error("SDL_CreateThreadEx fail");
        return LAS_ERROR_THREAD_CREATE;
    }

    if (playlist->read_thread->retval) {
        log_error("PlayList_read_thread() fails: %s(0x%x)", av_err2str(playlist->read_thread->retval), playlist->read_thread->retval);
        return playlist->read_thread->retval;
    }

    ret = PlayList_prepare_reading_tag(playlist);
    if (ret < 0) {
        return ret;
    }
    ret = PlayList_open_rep(playlist, &playlist->reading_tag, s);
    if (ret) {
        log_error("PlayList_open_rep() fails: %s(0x%x)", av_err2str(ret), ret);
        return ret;
    }


    return 0;
}
static void PlayList_abort(PlayList* playlist) {
    TagQueue_abort(&playlist->tag_queue);

    SDL_LockMutex(playlist->rw_mutex);
    playlist->read_abort_request = 1;
    SDL_UnlockMutex(playlist->rw_mutex);
}

void PlayList_close_rep(PlayList* playlist) {
    SDL_LockMutex(playlist->rw_mutex);
    avformat_close_input(&playlist->ctx);
    av_freep(&playlist->pb.buffer);
    log_info("close_index:%d finished", playlist->cur_rep_index);
    SDL_UnlockMutex(playlist->rw_mutex);
}

void PlayList_close_read_thread(PlayList* playlist) {
    // abort request
    if (playlist->rw_mutex) {
        PlayList_abort(playlist);
        PlayList_close_rep(playlist);
    }

    SDL_WaitThread(playlist->read_thread, NULL);
    playlist->read_thread = NULL;

    SDL_WaitThread(playlist->algo_thread, NULL);
    playlist->algo_thread = NULL;

    SDL_DestroyMutexP(&playlist->rw_mutex);
    SDL_DestroyMutexP(&playlist->reading_tag_mutex);
    SDL_DestroyMutexP(&playlist->las_mutex);
    TagQueue_destroy(&playlist->tag_queue);
    SDL_DestroyCondP(&playlist->algo_cond);
}


#pragma mark parser
void free_multi_rate_flv_context(PlayList* c) {
    if (!c)
        return;
    AdaptationSet* adaptation_set_item = &c->adaptation_set;
    for (int j = 0; j < adaptation_set_item->n_representation; j++) {
        if (adaptation_set_item->representations[j]) {
            av_freep(&adaptation_set_item->representations[j]);
        }
    }
}

static void dump_multi_rate_flv_context(PlayList* c) {
    if (!c)
        return;
    AdaptationSet* adaptation_set_item = &c->adaptation_set;
    for (int j = 0; j < adaptation_set_item->n_representation; j++) {
        Representation* representation_item = adaptation_set_item->representations[j];
        av_log(NULL, AV_LOG_DEBUG, "{\n");
        av_log(NULL, AV_LOG_DEBUG, "    id: %d \n", representation_item->id);
        av_log(NULL, AV_LOG_DEBUG, "    bitrate: %d \n", representation_item->bitrate);
        av_log(NULL, AV_LOG_DEBUG, "    url: \"%s\" \n", representation_item->url);
        av_log(NULL, AV_LOG_DEBUG, "}\n");
    }
}

static int parse_representation_set(Representation* c, cJSON* root) {
    int len = cJSON_GetArraySize(root);
    for (int i = 0; i < len; i++) {
        cJSON* child_json = cJSON_GetArrayItem(root, i);
        switch (child_json->type) {
            case cJSON_Number:
                if (!strcmp(child_json->string, "id")) {
                    c->id = (int)child_json->valuedouble;
                } else if (!strcmp(child_json->string, "maxBitrate")) {
                    c->bitrate = (int)child_json->valuedouble;
                }
                break;
            case cJSON_String:
                if (!strcmp(child_json->string, "url")) {
                    strcpy(c->url, child_json->valuestring);
                }
                break;
            case cJSON_NULL:
            case cJSON_True:
                if (!strcmp(child_json->string, "defaultSelected")) {
                    c->default_selected = 1;
                } else if (!strcmp(child_json->string, "disabledFromAdaptive")) {
                    c->disabled_from_adaptive = 1;
                }
                break;
            case cJSON_Array:
            case cJSON_Object:
            case cJSON_False:
                break;
        }
    }
    return 0;
}

static int parse_adaptation_set(AdaptationSet* c, cJSON* root) {
    int len = cJSON_GetArraySize(root);
    for (int i = 0; i < len; i++) {
        cJSON* child_json = cJSON_GetArrayItem(root, i);
        switch (child_json->type) {
            case cJSON_Number:
                if (!strcmp(child_json->string, "duration")) {
                    c->duration = (int)child_json->valuedouble;
                }
                break;
            case cJSON_Array:
                if (child_json->string && !strcmp(child_json->string, "representation")) {
                    int len = cJSON_GetArraySize(child_json);
                    for (int i = 0; i < len; i++) {
                        Representation* representation_item = NULL;
                        representation_item = av_mallocz(sizeof(Representation));
                        if (!representation_item) {
                            return -1;
                        }
                        c->representations[c->n_representation] = representation_item;
                        representation_item->index = c->n_representation;
                        representation_item->disabled_from_adaptive = 0;
                        representation_item->default_selected = 0;
                        c->n_representation++;

                        cJSON* root_json = cJSON_GetArrayItem(child_json, i);
                        parse_representation_set(representation_item, root_json);
                    }
                }
                break;
            case cJSON_False:
            case cJSON_True:
            case cJSON_String:
            case cJSON_NULL:
            case cJSON_Object:
                break;
        }
    }
    return 0;
}

int parse_root(char* file_name, PlayList* c) {
    cJSON* root = cJSON_Parse(file_name);
    if (!root)
        return LAS_ERROR_MANIFEST_JSON;
    if (cJSON_Object == root->type) {
        int len = cJSON_GetArraySize(root);
        for (int i = 0; i < len; i++) {
            cJSON* child_json = cJSON_GetArrayItem(root, i);
            switch (child_json->type) {
                case cJSON_Array:
                    if (child_json->string && !strcmp(child_json->string, "adaptationSet")) {
                        cJSON* adaptation_set = cJSON_GetArrayItem(child_json, 0);
                        if (adaptation_set) {
                            parse_adaptation_set(&c->adaptation_set, adaptation_set);
                        }
                    }
                    break;
                case cJSON_Number:
                case cJSON_String:
                case cJSON_NULL:
                case cJSON_False:
                case cJSON_True:
                case cJSON_Object:
                    break;
            }
            printf("\n");
        }
    }
    cJSON_Delete(root);
    dump_multi_rate_flv_context(c);
    return 0;
}

int parse_adapt_config(char* config_string, AdaptiveConfig* config, PlayList* playlist) {
    LasPlayerStatistic* player_stat = playlist->las_player_statistic;
    cJSON* root = cJSON_Parse(config_string);
    if (!root)
        return LAS_ERROR_ADAPT_CONFIG_JSON;
    if (cJSON_Object == root->type) {
        int len = cJSON_GetArraySize(root);
        for (int i = 0; i < len; i++) {
            cJSON* child_json = cJSON_GetArrayItem(root, i);
            switch (child_json->type) {
                case cJSON_Number:
                    if (!strcmp(child_json->string, "bufferInit")) {
                        config->buffer_init = child_json->valueint;
                    } else if (!strcmp(child_json->string, "stableBufferDiffThresholdSecond")) {
                        config->stable_buffer_diff_threshold_second = child_json->valuedouble;
                    } else if (!strcmp(child_json->string, "stableBufferIntervalMs")) {
                        config->stable_buffer_interval_ms = child_json->valuedouble;
                    } else if (!strcmp(child_json->string, "generateSpeedGapMs")) {
                        config->generate_speed_gap_ms = child_json->valuedouble;
                    } else if (!strcmp(child_json->string, "bufferCheckIntervalMs")) {
                        config->buffer_check_interval_ms = child_json->valuedouble;
                    } else if (!strcmp(child_json->string, "smoothedSpeedUtilizationRatio")) {
                        config->smoothed_speed_utilization_ratio = child_json->valuedouble;
                    } else if (!strcmp(child_json->string, "smallSpeedToBitrateRatio")) {
                        config->small_speed_to_bitrate_ratio = child_json->valuedouble;
                    } else if (!strcmp(child_json->string, "enoughSpeedToBitrateRatio")) {
                        config->enough_speed_to_bitrate_ratio = child_json->valuedouble;
                    } else if (!strcmp(child_json->string, "bufferLowerLimitSecond")) {
                        config->buffer_lower_limit_second = child_json->valuedouble;
                    } else if (!strcmp(child_json->string, "recentBufferedSize")) {
                        config->recent_buffered_size = child_json->valuedouble;
                    } else if (!strcmp(child_json->string, "smoothedSpeedRatio")) {
                        config->smoothed_speed_ratio = child_json->valuedouble;
                    }
                    break;
                case cJSON_Object:
                case cJSON_False:
                case cJSON_NULL:
                case cJSON_Array:
                case cJSON_True:
                    break;
            }
        }
    }
    cJSON_Delete(root);
    return 0;
}

static int parse_int_from(cJSON* json, const char* key) {
    cJSON* entry = cJSON_GetObjectItemCaseSensitive(json, key);
    if (cJSON_IsNumber(entry)) {
        return entry->valueint;
    }
    return 0;
}

#pragma mark las
static int las_close(AVFormatContext* s) {
    LasContext* c = s->priv_data;
    PlayList* playlist = &c->playlist;
    PlayList_close_read_thread(playlist);
    free_multi_rate_flv_context(playlist);
    av_freep(&c->user_agent);
    av_freep(&c->cookies);
    av_freep(&c->headers);
    av_freep(&c->server_ip);
    av_freep(&c->manifest_string);
    av_freep(&c->live_adapt_config);
    av_dict_free(&c->avio_opts);
    return 0;
}

static int las_probe(AVProbeData* p) {
    if (p->filename && strstr(p->filename, ".las"))
        return AVPROBE_SCORE_MAX;

    return 0;
}

static int las_read_header(AVFormatContext* s) {
    // for debug
#ifdef __ANDROID__
    pthread_setname_np(pthread_self(), "ffplay_read_thread");
#elif defined(__APPLE__)
    pthread_setname_np("ffplay_read_thread");
#endif

    LasContext* c = s->priv_data;
    PlayList* playlist = &c->playlist;
    AdaptiveConfig config;
    AVDictionaryEntry* entry;
    int ret = 0;

    c->ctx = s;
    c->interrupt_callback = &s->interrupt_callback;
    c->server_ip = NULL;
    playlist->session_id = c->session_id;

    av_dict_set(&c->avio_opts, "timeout", "10000000", 0);
    
    if ((ret = parse_root(c->manifest_string, playlist)) < 0) {
        log_error("Illegal manifest Json String");
        goto fail;
    }
    log_info("Finish parsing las manifest (player)");
    playlist->las_player_statistic = (LasPlayerStatistic*)c->las_player_statistic;
    if (playlist->las_player_statistic) {
        playlist->las_statistic = &playlist->las_player_statistic->las_stat;
        log_info("playlist->stat: %p, las_statistic: %p", playlist->las_player_statistic, playlist->las_statistic);
    } else {
        log_error("las_player_statistic is null");
        goto fail;
    }
    LasStatistic_init(playlist->las_statistic, playlist);

    RateAdaptConfig_default_init(&config, playlist);
    if (parse_adapt_config(c->live_adapt_config, &config, playlist) < 0) {
        log_error("Illegal adaptation Configure Json String");
    }
    playlist->outermost_ctx = s;
    MultiRateAdaption_init(&playlist->multi_rate_adaption, config, playlist);
    PlayList_reset_state(playlist);
    ret = PlayList_open_read_thread(playlist);
    if (ret  != 0) {
        goto fail;
    }
    return ret;

fail:
    las_close(s);
    return ret == 0 ? 0 : AVERROR_EXIT;
}

/*
 * Used to reset a statically allocated AVPacket to a clean slate,
 * containing no data.
 */
static void reset_packet(AVPacket* pkt) {
    if (pkt) {
        av_init_packet(pkt);
        pkt->data = NULL;
    }
}

static bool h264_check_sps_pps(const AVPacket* pkt) {
    if (pkt && pkt->data && pkt->size >= 5) {
        int offset = 0;

        while (offset >= 0 && offset + 5 <= pkt->size) {
            uint8_t* nal_start = pkt->data + offset;
            int nal_size = AV_RB32(nal_start);
            int nal_type = nal_start[4] & 0x1f;

            if (nal_type == H264_NAL_SPS || nal_type == H264_NAL_PPS) {
                return true;
            }

            offset += nal_size + 4;
        }
    }
    return false;
}

static bool read_sps_pps_by_avcc(uint8_t* extradata, uint32_t extrasize,
                                 uint8_t** sps, int* sps_len,
                                 uint8_t** pps, int* pps_len) {
    uint8_t* spc = extradata + 6;
    uint32_t sps_size = AV_RB16(spc);
    if (sps_size) {
        *sps_len = sps_size;
        *sps = (uint8_t*)av_mallocz(sps_size);
        if (!*sps) {
            return false;
        }
        spc += 2;
        memcpy(*sps, spc, sps_size);
        spc += sps_size;
    }
    spc += 1;
    uint32_t  pps_size = AV_RB16(spc);
    if (pps_size) {
        *pps_len = pps_size;
        *pps = (uint8_t*)av_mallocz(pps_size);
        if (!*pps) {
            if (*sps) {
                free(*sps);
            }
            return false;
        }
        spc += 2;
        memcpy(*pps, spc, pps_size);
    }
    return true;
}

static void insert_sps_pps_into_avpacket(AVPacket* packet, uint8_t* new_extradata, int new_extradata_size, PlayList* playlist) {
    uint8_t* sps = NULL, *pps = NULL;
    int sps_len = 0, pps_len = 0;

    if (read_sps_pps_by_avcc(new_extradata, new_extradata_size, &sps, &sps_len, &pps, &pps_len)) {
        int size = packet->size;
        if (sps) {
            size += NALU_HEAD_LEN + sps_len;
        }
        if (pps) {
            size += NALU_HEAD_LEN + pps_len;
        }
        if (size == packet->size) {
            return;
        }
        AVPacket new_pack;
        int res = av_new_packet(&new_pack, size);
        if (res < 0) {
            log_error("Failed memory allocation");
        } else {
            av_packet_copy_props(&new_pack, packet);
            uint8_t* data = new_pack.data;

            if (sps) {
                AV_WB32(data, sps_len);
                data += NALU_HEAD_LEN;
                memcpy(data, sps, sps_len);
                data += sps_len;
                log_info("insert sps, size:%d", sps_len);
            } else {
                log_info("sps is null");
            }
            if (pps) {
                AV_WB32(data, pps_len);
                data += NALU_HEAD_LEN;
                memcpy(data, pps, pps_len);
                data += pps_len;
                log_info("insert pps, size:%d", pps_len);
            } else {
                log_info("pps is null");
            }
            memcpy(data, packet->data, packet->size);
            av_packet_unref(packet);
            *packet = new_pack;
        }
    }
}

static int las_read_packet(AVFormatContext* s, AVPacket* pkt) {
    LasContext* c = s->priv_data;
    PlayList* playlist = &c->playlist;
    int ret = 0;

    while (1) {
        if (!playlist->ctx) {
            log_error("playlist->ctx is null");
            ret = AVERROR_EOF;
            goto fail;
        }
        ret = av_read_frame(playlist->ctx, &playlist->pkt);
        if (ret < 0) {
            reset_packet(&playlist->pkt);
            if (avio_feof(&playlist->pb) || ret == AVERROR_EOF) {
                // change rep if needed
                if (playlist->cur_switch_index != playlist->reading_tag.switch_index) {
                    PlayList_close_rep(playlist);
                    PlayList_open_rep(playlist, &playlist->reading_tag, s);
                    continue;
                }
            }
            break;
        } else if (!playlist->pkt.data) {
            // success, but no packet data yet
            continue;
        } else {
            // go packet
            *pkt = playlist->pkt;
            if (pkt->stream_index >= 0 && pkt->stream_index < MAX_STREAM_NUM) {
                pkt->stream_index = playlist->stream_index_map[pkt->stream_index];
            }
            AVCodecParameters* codec = playlist->ctx->streams[pkt->stream_index]->codecpar;
            if (codec->extradata) {
                if (codec->codec_id == AV_CODEC_ID_H264
                    && !h264_check_sps_pps(pkt)) {
                    insert_sps_pps_into_avpacket(pkt, codec->extradata, codec->extradata_size, playlist);
                }
                uint8_t *side = av_packet_new_side_data(pkt, AV_PKT_DATA_NEW_EXTRADATA, codec->extradata_size);
                if (side) {
                    memcpy(side, codec->extradata, codec->extradata_size);
                    av_freep(&codec->extradata);
                    codec->extradata_size = 0;
                }
            }
            break;
        }
    }
    reset_packet(&playlist->pkt);
    LasStatistic_on_read_packet(playlist->las_statistic, playlist);

fail:
    return ret == 0 ? 0 : AVERROR_EXIT;
}

static int las_read_seek(AVFormatContext* s, int stream_index,
                          int64_t timestamp, int flags) {
//    LasContext *c = s->priv_data;
    return 0;
}

#define OFFSET(x) offsetof(LasContext, x)
#define FLAGS AV_OPT_FLAG_DECODING_PARAM
static const AVOption las_options[] = {
    {
        "user-agent", "user agent",
        OFFSET(user_agent), AV_OPT_TYPE_STRING, { .str = NULL }, CHAR_MIN, CHAR_MAX, FLAGS
    },
    {
        "headers", "headers",
        OFFSET(headers), AV_OPT_TYPE_STRING, { .str = NULL }, CHAR_MIN, CHAR_MAX, FLAGS
    },
    {
        "manifest_string", "manifest_string",
        OFFSET(manifest_string), AV_OPT_TYPE_STRING, { .str = NULL }, CHAR_MIN, CHAR_MAX, FLAGS
    },
    {
        "abr_history_data", "abr_history_data",
        OFFSET(abr_history_data), AV_OPT_TYPE_STRING, { .str = NULL }, CHAR_MIN, CHAR_MAX, FLAGS
    },
    {
        "device-network-type", "device-network-type",
        OFFSET(network), AV_OPT_TYPE_INT64, {.i64 = 0}, 0, INT64_MAX, FLAGS
    },
    {
        "las_player_statistic", "las_player_statistic",
        OFFSET(las_player_statistic), AV_OPT_TYPE_INT64, {.i64 = 0}, INT64_MIN, INT64_MAX, FLAGS
    },
    {
        "liveAdaptConfig", "liveAdaptConfig",
        OFFSET(live_adapt_config), AV_OPT_TYPE_STRING, { .str = NULL }, CHAR_MIN, CHAR_MAX, FLAGS
    },
    {
        "session_id", "session_id",
        OFFSET(session_id), AV_OPT_TYPE_INT, {.i64 = 0}, INT32_MIN, INT32_MAX, FLAGS
    },
    {NULL}
};

static const AVClass las_class = {
    .class_name = "las",
    .item_name  = av_default_item_name,
    .option     = las_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVInputFormat ijkff_las_demuxer = {
    .name           = "las",
    .long_name      = "Live Adaptive Streaming",
    .priv_class     = &las_class,
    .priv_data_size = sizeof(LasContext),
    .read_probe     = las_probe,
    .read_header    = las_read_header,
    .read_packet    = las_read_packet,
    .read_close     = las_close,
    .read_seek      = las_read_seek,
    .extensions     = "las",
    .flags          = AVFMT_NOFILE
};



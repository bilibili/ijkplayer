#ifndef LAS_DEMUX
#define LAS_DEMUX

#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_URL_SIZE 4096
#define MAX_STREAM_NUM 10
#define LAS_AUTO_MODE -1

typedef struct FlvInfo {
    int total_bandwidth_kbps;
    char url[MAX_URL_SIZE];
} FlvInfo;

typedef struct LasStatistic {
    // info from manifest.json
    FlvInfo flvs[MAX_STREAM_NUM];
    int flv_nb;

    // algo specific
    int64_t bandwidth_fragment;
    int64_t bitrate_downloading;
    int64_t current_buffer_ms;

    // status
    int cur_decoding_flv_index;
    int64_t switch_point_a_buffer_ms;
    int64_t switch_point_v_buffer_ms;
    char cur_playing_url[MAX_URL_SIZE];

    // rep switch speed
    int64_t cur_rep_read_start_time;
    int64_t cur_rep_http_open_time;
    int64_t cur_rep_read_header_time;
    int64_t cur_rep_first_data_time;  // first tag time
    int64_t cur_rep_start_time;
    int64_t rep_switch_gap_time;  //new_rep_start_pts - request_pts
    int64_t rep_switch_cnt;

    int cur_rep_http_reading_error;  // errors during gop reading
    int64_t cached_tag_dur_ms;
    int64_t cached_a_dur_ms; // 上层播放器packet_queue长度
    int64_t cached_v_dur_ms;
    int64_t total_bytes_read;
} LasStatistic;

typedef struct LasPlayerStatistic {
    pthread_mutex_t control_mutex;

    int las_switch_mode;
    int64_t first_audio_packet_pts;
    int block_duration;
    int audio_only_request;
    int audio_only_response;
    int64_t audio_cached_duration_ms;
    int64_t video_cached_duration_ms;

    LasStatistic las_stat;
    bool stream_reopened;
} LasPlayerStatistic;

int las_stat_init(LasPlayerStatistic* stat);
void las_stat_destroy(LasPlayerStatistic* stat);

int las_get_switch_mode(LasPlayerStatistic* stat);
void las_set_switch_mode(LasPlayerStatistic* stat, int switch_mode);

int64_t las_get_first_audio_packet_pts(LasPlayerStatistic* stat);
void las_set_first_audio_packet_pts(LasPlayerStatistic* stat, int64_t first_audio_packet_pts);

int las_get_audio_only_request(LasPlayerStatistic* stat);
void las_set_audio_only_request(LasPlayerStatistic* stat, int audio_only);

int las_get_audio_only_response(LasPlayerStatistic* stat);
void las_set_audio_only_response(LasPlayerStatistic* stat, int audio_only);

int64_t las_get_audio_cached_duration_ms(LasPlayerStatistic* stat);
void las_set_audio_cached_duration_ms(LasPlayerStatistic* stat, int64_t audio_cached_duration_ms);

int64_t las_get_video_cached_duration_ms(LasPlayerStatistic* stat);
void las_set_video_cached_duration_ms(LasPlayerStatistic* stat, int64_t video_cached_duration_ms);

bool las_get_stream_reopened(LasPlayerStatistic* stat);
void las_set_stream_reopened(LasPlayerStatistic* stat, bool stream_reopened);

int32_t LasPlayerStatistic_get_downloading_bitrate(LasPlayerStatistic* stat);
char* LasPlayerStatistic_get_downloading_url(LasPlayerStatistic* stat);
int LasPlayerStatistic_get_http_reading_error(LasPlayerStatistic* stat);

#endif

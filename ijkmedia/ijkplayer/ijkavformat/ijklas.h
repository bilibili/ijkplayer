#ifndef LAS_DEMUX
#define LAS_DEMUX

#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_URL_SIZE 4096
#define MAX_STREAM_NUM 10
#define LAS_AUTO_MODE (-1)

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

#endif

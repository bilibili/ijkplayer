//
//  IJKFFCMediaPlayer.h
//  IJKPlayer
//
//  Created by befovy on 2019/11/1.
//  Copyright © 2019 befovy. All rights reserved.
//

#ifndef IJKFF_MEDIA_PLAYER_C_API_H
#define IJKFF_MEDIA_PLAYER_C_API_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct IJKFFOverlay {
    int w;
    int h;
    uint32_t format;
    int planes;
    uint16_t *pitches;
    uint8_t **pixels;
    int sar_num;
    int sar_dem;
} IJKFFOverlay;

typedef void (*IJKFFOverlayCb) (void *userdata, IJKFFOverlay* overlay);

typedef struct IJKFFCMediaPlayer IJKFFCMediaPlayer;

IJKFFCMediaPlayer *ijkcmp_create(void);

int ijkcmp_set_data_source(IJKFFCMediaPlayer *mp, const char * url);
int ijkcmp_prepareAsync(IJKFFCMediaPlayer *mp);
int ijkcmp_start(IJKFFCMediaPlayer *mp);
int ijkcmp_stop(IJKFFCMediaPlayer *mp);
int ijkcmp_pause(IJKFFCMediaPlayer *mp);
int ijkcmp_reset(IJKFFCMediaPlayer *mp);
bool ijkcmp_is_playing(IJKFFCMediaPlayer *mp);
void ijkcmp_shutdown(IJKFFCMediaPlayer *mp);


int64_t ijkcmp_get_current_position(IJKFFCMediaPlayer *mp);
int64_t ijkcmp_get_duration(IJKFFCMediaPlayer *mp);
int ijkcmp_seek_to(IJKFFCMediaPlayer *mp, int64_t msec);

void ijkcmp_set_loop(IJKFFCMediaPlayer *mp, int loop);
int  ijkcmp_get_loop(IJKFFCMediaPlayer *mp);

void ijkcmp_set_speed(IJKFFCMediaPlayer *mp, float speed);

void ijkcmp_set_playback_volume(IJKFFCMediaPlayer *mp, float volume);
float ijkcmp_get_playback_volume(IJKFFCMediaPlayer *mp);

void ijkcmp_set_stream_selected(IJKFFCMediaPlayer *mp, int strean, bool selected);

float ijkcmp_get_float_property(IJKFFCMediaPlayer *mp, int property, float dfault);
int64_t ijkcmp_get_long_property(IJKFFCMediaPlayer *mp, int property, int64_t dfault);


void ijkcmp_set_option(IJKFFCMediaPlayer *mp, const char *value, const char *key, int category);
void ijkcmp_set_int_option(IJKFFCMediaPlayer *mp, int64_t value, const char *key, int category);


typedef void (*IJKFFEventCb) (void *userdata, int what, int arg1, int arg2, void *extra);

void ijkcmp_add_event_listener(IJKFFCMediaPlayer *mp, void *userdata, IJKFFEventCb cb);
void ijkcmp_remove_event_listener(IJKFFCMediaPlayer *mp, void *userdata, IJKFFEventCb cb);

void ijkcmp_set_overlay_cb(IJKFFCMediaPlayer *mp, void *userdata, IJKFFOverlayCb cb);

#endif //IJKFF_MEDIA_PLAYER_C_API_H

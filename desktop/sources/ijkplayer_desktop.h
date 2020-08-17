/*****************************************************************************
* ijkplayer_desktop.h
*****************************************************************************
*
* copyright (c) 2019 befovy <befovy@gmail.com>
*
* This file is part of ijkPlayer.
*
* ijkPlayer is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* ijkPlayer is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with ijkPlayer; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef IJKPLAYER_DESKTOP_IJKPLAYER_DESKTOP_H
#define IJKPLAYER_DESKTOP_IJKPLAYER_DESKTOP_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_IJK
    #ifdef __GNUC__
      #define IJK_API __attribute__ ((dllexport))
    #else
      #define IJK_API __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define IJK_API __attribute__ ((dllimport))
    #else
      #define IJK_API __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define IJK_API __attribute__ ((visibility ("default")))
  #else
    #define IJK_API
  #endif
#endif

#define IJK_LOG_UNKNOWN     0
#define IJK_LOG_DEFAULT     1
#define IJK_LOG_VERBOSE     2
#define IJK_LOG_DEBUG       3
#define IJK_LOG_INFO        4
#define IJK_LOG_WARN        5
#define IJK_LOG_ERROR       6
#define IJK_LOG_FATAL       7
#define IJK_LOG_SILENT      8

#define IJK_OPT_CATEGORY_FORMAT 1
#define IJK_OPT_CATEGORY_CODEC  2
#define IJK_OPT_CATEGORY_SWS    3
#define IJK_OPT_CATEGORY_PLAYER 4
#define IJK_OPT_CATEGORY_SWR    5

#define IJK_VOUT_SDL2           0
#define IJK_VOUT_GLFW           1
#define IJK_VOUT_CALLBACK       2
#define IJK_VOUT_DUMMY          3

#define IJK_STATE_IDLE               0
#define IJK_STATE_INITIALIZED        1
#define IJK_STATE_ASYNC_PREPARING    2
#define IJK_STATE_PREPARED           3
#define IJK_STATE_STARTED            4
#define IJK_STATE_PAUSED             5
#define IJK_STATE_COMPLETED          6
#define IJK_STATE_STOPPED            7
#define IJK_STATE_ERROR              8
#define IJK_STATE_END                9


#define IJK_MSG_FLUSH                       0
#define IJK_MSG_ERROR                       100     /* arg1 = error */
#define IJK_MSG_PREPARED                    200
#define IJK_MSG_COMPLETED                   300
#define IJK_MSG_VIDEO_SIZE_CHANGED          400     /* arg1 = width, arg2 = height */
#define IJK_MSG_SAR_CHANGED                 401     /* arg1 = sar.num, arg2 = sar.den */
#define IJK_MSG_VIDEO_RENDERING_START       402
#define IJK_MSG_AUDIO_RENDERING_START       403
#define IJK_MSG_VIDEO_ROTATION_CHANGED      404     /* arg1 = degree */
#define IJK_MSG_AUDIO_DECODED_START         405
#define IJK_MSG_VIDEO_DECODED_START         406
#define IJK_MSG_OPEN_INPUT                  407
#define IJK_MSG_FIND_STREAM_INFO            408
#define IJK_MSG_COMPONENT_OPEN              409
#define IJK_MSG_VIDEO_SEEK_RENDERING_START  410
#define IJK_MSG_AUDIO_SEEK_RENDERING_START  411

#define IJK_MSG_BUFFERING_START             500
#define IJK_MSG_BUFFERING_END               501
#define IJK_MSG_BUFFERING_UPDATE            502     /* arg1 = buffering head position in time, arg2 = minimum percent in time or bytes */
#define IJK_MSG_BUFFERING_BYTES_UPDATE      503     /* arg1 = cached data in bytes,            arg2 = high water mark */
#define IJK_MSG_BUFFERING_TIME_UPDATE       504     /* arg1 = cached duration in milliseconds, arg2 = high water mark */
#define IJK_MSG_CURRENT_POSITION_UPDATE     510     /* arg1 = current position in milliseconds */
#define IJK_MSG_SEEK_COMPLETE               600     /* arg1 = seek position,                   arg2 = error */
#define IJK_MSG_PLAYBACK_STATE_CHANGED      700
#define IJK_MSG_TIMED_TEXT                  800
#define IJK_MSG_ACCURATE_SEEK_COMPLETE      900     /* arg1 = current position*/
#define IJK_MSG_GET_IMG_STATE               1000    /* arg1 = timestamp, arg2 = result code, obj = file name*/


#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct IjkFFMediaPlayer IjkFFMediaPlayer;

typedef struct IjkFFOverlay {
    int w;
    int h;
    uint32_t format;
    int planes;
    uint16_t *pitches;
    uint8_t **pixels;
    int sar_num;
    int sar_den;
} IjkFFOverlay;

typedef void(*ijkff_overlay_cb)(void *userdata, IjkFFOverlay *overlay);

typedef void(*ijkff_event_cb)(void *userdata, int what, int arg1, int arg2, void *extra);

/**
 * Create player with different vout
 * @param vout_type 0 @IJK_VOUT_SDL2 SDL vout
 *                  1 @IJK_VOUT_GLFW  glfw render
 *                  2 @IJK_VOUT_CALLBACK  callback vout
 *                  3 @IJK_VOUT_DUMMY dummy vout, do nothing
 * @return
 */
IJK_API IjkFFMediaPlayer *ijkff_create(int vout_type);

IJK_API int ijkff_set_data_source(IjkFFMediaPlayer *fp, const char *url);

IJK_API int ijkff_prepare_async(IjkFFMediaPlayer *fp);

IJK_API int ijkff_start(IjkFFMediaPlayer *fp);

IJK_API int ijkff_stop(IjkFFMediaPlayer *fp);

IJK_API int ijkff_pause(IjkFFMediaPlayer *fp);

IJK_API int ijkff_reset(IjkFFMediaPlayer *fp);

IJK_API bool ijkff_is_playing(IjkFFMediaPlayer *fp);

IJK_API void ijkff_shutdown(IjkFFMediaPlayer *fp);


IJK_API int64_t ijkff_get_current_position(IjkFFMediaPlayer *fp);

IJK_API int64_t ijkff_get_duration(IjkFFMediaPlayer *fp);

IJK_API int ijkff_seek_to(IjkFFMediaPlayer *fp, int64_t msec);

IJK_API void ijkff_set_loop(IjkFFMediaPlayer *fp, int loop);

IJK_API int ijkff_get_loop(IjkFFMediaPlayer *fp);

IJK_API void ijkff_set_speed(IjkFFMediaPlayer *fp, float speed);

IJK_API void ijkff_set_playback_volume(IjkFFMediaPlayer *fp, float volume);

IJK_API float ijkff_get_playback_volume(IjkFFMediaPlayer *fp);

IJK_API void ijkff_set_stream_selected(IjkFFMediaPlayer *fp, int strean, bool selected);

IJK_API float ijkff_get_float_property(IjkFFMediaPlayer *fp, int property, float dfault);

IJK_API int64_t ijkff_get_long_property(IjkFFMediaPlayer *fp, int property, int64_t dfault);


IJK_API void ijkff_set_option(IjkFFMediaPlayer *fp, const char *value, const char *key, int category);

IJK_API void ijkff_set_int_option(IjkFFMediaPlayer *fp, int64_t value, const char *key, int category);

IJK_API void ijkff_set_window(IjkFFMediaPlayer *fp, void *window);

IJK_API void ijkff_set_event_cb(IjkFFMediaPlayer *fp, void *userdata, ijkff_event_cb cb);

IJK_API void ijkff_set_overlay_cb(IjkFFMediaPlayer *fp, void *userdata, ijkff_overlay_cb cb);


IJK_API void ijkff_log_level(int level);

IJK_API void ijkff_global_init();

IJK_API const char *ijkff_version();

#ifdef __cplusplus
}
#endif

#endif // IJKPLAYER_DESKTOP_IJKPLAYER_DESKTOP_H

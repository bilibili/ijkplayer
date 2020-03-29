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

IJK_API IjkFFMediaPlayer *ijkff_create();

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

IJK_API const char *ijkff_version();

#ifdef __cplusplus
}
#endif

#endif // IJKPLAYER_DESKTOP_IJKPLAYER_DESKTOP_H

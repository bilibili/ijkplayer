/*
 * ff_cmdutils.c
 *      based on ffmpeg/cmdutils.c
 *
 * Copyright (c) 2000-2003 Fabrice Bellard
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#include "ff_cmdutils.h"

#include "libavutil/display.h"
#include "libavutil/eval.h"

// MERGE: sws_opts
// MERGE: swr_opts
// MERGE: format_opts, codec_opts, resample_opts
// MERGE: this_year
// MERGE: report_file;
// MERGE: init_opts
// MERGE: uninit_opts
// MERGE: log_callback_help
// MERGE: log_callback_report(void *ptr, int level, const char *fmt, va_list vl)
// MERGE: parse_number_or_die
// MERGE: parse_time_or_die
// MERGE: show_help_options
// MERGE: show_help_children
// MERGE: find_option
// MERGE: #include <windows.h>
// MERGE: #include <shellapi.h>
// MERGE: win32_argv_utf8
// MERGE: win32_argc
// MERGE: prepare_app_arguments
// MERGE: write_option
// MERGE: parse_option
// MERGE: parse_options
// MERGE: parse_optgroup
// MERGE: locate_option
// MERGE: dump_argument
// MERGE: parse_loglevel
// MERGE: #define FLAGS
// MERGE: opt_default
// MERGE: match_group_separator
// MERGE: finish_group
// MERGE: add_opt
// MERGE: init_parse_context
// MERGE: uninit_parse_context
// MERGE: split_commandline
// MERGE: opt_loglevel
// MERGE: expand_filename_template
// MERGE: init_report
// MERGE: opt_report
// MERGE: opt_max_alloc
// MERGE: opt_cpuflags
// MERGE: opt_timelimit

void print_error(const char *filename, int err)
{
    char errbuf[128];
    const char *errbuf_ptr = errbuf;

    if (av_strerror(err, errbuf, sizeof(errbuf)) < 0)
        errbuf_ptr = strerror(AVUNERROR(err));
    av_log(NULL, AV_LOG_ERROR, "%s: %s\n", filename, errbuf_ptr);
}

// MERGE: warned_cfg
// MERGE: INDENT
// MERGE: SHOW_VERSION
// MERGE: SHOW_CONFIG
// MERGE: SHOW_COPYRIGHT
// MERGE: PRINT_LIB_INFO
// MERGE: print_all_libs_info
// MERGE: print_program_info
// MERGE: show_banner
// MERGE: show_version
// MERGE: show_license
// MERGE: show_formats
// MERGE: PRINT_CODEC_SUPPORTED
// MERGE: print_codec
// MERGE: get_media_type_char
// MERGE: next_codec_for_id
// MERGE: compare_codec_desc
// MERGE: get_codecs_sorted
// MERGE: print_codecs_for_id
// MERGE: show_codecs
// MERGE: print_codecs
// MERGE: show_decoders
// MERGE: show_encoders
// MERGE: show_bsfs
// MERGE: show_protocols
// MERGE: show_filters
// MERGE: show_pix_fmts
// MERGE: show_layouts
// MERGE: show_sample_fmts
// MERGE: show_help_codec
// MERGE: show_help_demuxer
// MERGE: show_help_muxer
// MERGE: show_help
// MERGE: read_yesno
// MERGE: cmdutils_read_file
// MERGE: get_preset_file

static int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec)
{
    int ret = avformat_match_stream_specifier(s, st, spec);
    if (ret < 0)
        av_log(s, AV_LOG_ERROR, "Invalid stream specifier: %s.\n", spec);
    return ret;
}

AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
                                AVFormatContext *s, AVStream *st, AVCodec *codec)
{
    AVDictionary    *ret = NULL;
    AVDictionaryEntry *t = NULL;
    int            flags = s->oformat ? AV_OPT_FLAG_ENCODING_PARAM
                                      : AV_OPT_FLAG_DECODING_PARAM;
    char          prefix = 0;
    const AVClass    *cc = avcodec_get_class();

    if (!codec)
        codec            = s->oformat ? avcodec_find_encoder(codec_id)
                                      : avcodec_find_decoder(codec_id);

    switch (st->codec->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        prefix  = 'v';
        flags  |= AV_OPT_FLAG_VIDEO_PARAM;
        break;
    case AVMEDIA_TYPE_AUDIO:
        prefix  = 'a';
        flags  |= AV_OPT_FLAG_AUDIO_PARAM;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        prefix  = 's';
        flags  |= AV_OPT_FLAG_SUBTITLE_PARAM;
        break;
    default:
        break;
    }

    while ((t = av_dict_get(opts, "", t, AV_DICT_IGNORE_SUFFIX))) {
        char *p = strchr(t->key, ':');

        /* check stream specification in opt name */
        if (p)
            switch (check_stream_specifier(s, st, p + 1)) {
            case  1: *p = 0; break;
            case  0:         continue;
            default:         return NULL;
            }

        if (av_opt_find(&cc, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ) ||
            (codec && codec->priv_class &&
             av_opt_find(&codec->priv_class, t->key, NULL, flags,
                         AV_OPT_SEARCH_FAKE_OBJ)))
            av_dict_set(&ret, t->key, t->value, 0);
        else if (t->key[0] == prefix &&
                 av_opt_find(&cc, t->key + 1, NULL, flags,
                             AV_OPT_SEARCH_FAKE_OBJ))
            av_dict_set(&ret, t->key + 1, t->value, 0);

        if (p)
            *p = ':';
    }
    return ret;
}

AVDictionary **setup_find_stream_info_opts(AVFormatContext *s,
                                           AVDictionary *codec_opts)
{
    int i;
    AVDictionary **opts;

    if (!s->nb_streams)
        return NULL;
    opts = av_mallocz(s->nb_streams * sizeof(*opts));
    if (!opts) {
        av_log(NULL, AV_LOG_ERROR,
               "Could not alloc memory for stream options.\n");
        return NULL;
    }
    for (i = 0; i < s->nb_streams; i++)
        opts[i] = filter_codec_opts(codec_opts, s->streams[i]->codec->codec_id,
                                    s, s->streams[i], NULL);
    return opts;
}

// MERGE: grow_array
// MERGE: print_device_sources
// MERGE: print_device_sinks
// MERGE: show_sinks_sources_parse_arg
// MERGE: show_sources
// MERGE: show_sinks

double get_rotation(AVStream *st)
{
    AVDictionaryEntry *rotate_tag = av_dict_get(st->metadata, "rotate", NULL, 0);
    uint8_t* displaymatrix = av_stream_get_side_data(st,
                                                     AV_PKT_DATA_DISPLAYMATRIX, NULL);
    double theta = 0;

    if (rotate_tag && *rotate_tag->value && strcmp(rotate_tag->value, "0")) {
        char *tail;
        theta = av_strtod(rotate_tag->value, &tail);
        if (*tail)
            theta = 0;
    }
    if (displaymatrix && !theta)
        theta = -av_display_rotation_get((int32_t*) displaymatrix);

    theta -= 360*floor(theta/360 + 0.9/360);

    if (fabs(theta - 90*round(theta/90)) > 2)
        av_log(NULL, AV_LOG_WARNING, "Odd rotation angle.\n"
               "If you want to help, upload a sample "
               "of this file to ftp://upload.ffmpeg.org/incoming/ "
               "and contact the ffmpeg-devel mailing list. (ffmpeg-devel@ffmpeg.org)");

    return theta;
}

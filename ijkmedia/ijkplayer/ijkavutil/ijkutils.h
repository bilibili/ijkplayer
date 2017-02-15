/*
 * Copyright (c) 2016 Bilibili
 * Copyright (c) 2016 Raymond Zheng <raymondzheng1412@gmail.com>
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

#ifndef IJKAVUTIL_IJKUTILS_H
#define IJKAVUTIL_IJKUTILS_H

#include "ijktree.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

typedef struct IjkAVIOInterruptCB {
    int (*callback)(void*);
    void *opaque;
} IjkAVIOInterruptCB;

typedef struct IjkCacheTreeInfo {
    struct IjkAVTreeNode *root;
    int64_t physical_init_pos;
    int64_t physical_size;
} IjkCacheTreeInfo;

#define FFDIFFSIGN(x,y) (((x)>(y)) - ((x)<(y)))

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMAX3(a,b,c) FFMAX(FFMAX(a,b),c)
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMIN3(a,b,c) FFMIN(FFMIN(a,b),c)

#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))

/* error handling */
#if EDOM > 0
#define IJKAVERROR(e) (-(e))   ///< Returns a negative error code from a POSIX error code, to return from library functions.
#define IJKAVUNERROR(e) (-(e)) ///< Returns a POSIX error code from a library function error return value.
#else
/* Some platforms have E* and errno already negated. */
#define IJKAVERROR(e) (e)
#define IJKAVUNERROR(e) (e)
#endif

#define FFERRTAG(a, b, c, d) (-(int)MKTAG(a, b, c, d))
#define IJKAVERROR_BSF_NOT_FOUND      FFERRTAG(0xF8,'B','S','F') ///< Bitstream filter not found
#define IJKAVERROR_BUG                FFERRTAG( 'B','U','G','!') ///< Internal bug, also see AVERROR_BUG2
#define IJKAVERROR_BUFFER_TOO_SMALL   FFERRTAG( 'B','U','F','S') ///< Buffer too small
#define IJKAVERROR_DECODER_NOT_FOUND  FFERRTAG(0xF8,'D','E','C') ///< Decoder not found
#define IJKAVERROR_DEMUXER_NOT_FOUND  FFERRTAG(0xF8,'D','E','M') ///< Demuxer not found
#define IJKAVERROR_ENCODER_NOT_FOUND  FFERRTAG(0xF8,'E','N','C') ///< Encoder not found
#define IJKAVERROR_EOF                FFERRTAG( 'E','O','F',' ') ///< End of file
#define IJKAVERROR_EXIT               FFERRTAG( 'E','X','I','T') ///< Immediate exit was requested; the called function should not be restarted
#define IJKAVERROR_EXTERNAL           FFERRTAG( 'E','X','T',' ') ///< Generic error in an external library
#define IJKAVERROR_FILTER_NOT_FOUND   FFERRTAG(0xF8,'F','I','L') ///< Filter not found
#define IJKAVERROR_INVALIDDATA        FFERRTAG( 'I','N','D','A') ///< Invalid data found when processing input
#define IJKAVERROR_MUXER_NOT_FOUND    FFERRTAG(0xF8,'M','U','X') ///< Muxer not found
#define IJKAVERROR_OPTION_NOT_FOUND   FFERRTAG(0xF8,'O','P','T') ///< Option not found
#define IJKAVERROR_PATCHWELCOME       FFERRTAG( 'P','A','W','E') ///< Not yet implemented in FFmpeg, patches welcome
#define IJKAVERROR_PROTOCOL_NOT_FOUND FFERRTAG(0xF8,'P','R','O') ///< Protocol not found
#define IJKAVERROR_STREAM_NOT_FOUND   FFERRTAG(0xF8,'S','T','R') ///< Stream not found

/**
 * ORing this as the "whence" parameter to a seek function causes it to
 * return the filesize without seeking anywhere. Supporting this is optional.
 * If it is not supported then the seek function will return <0.
 */
#define IJKAVSEEK_SIZE 0x10000

/**
 * Passing this flag as the "whence" parameter to a seek function causes it to
 * seek by any means (like reopening and linear reading) or other normally unreasonable
 * means that can be extremely slow.
 * This may be ignored by the seek code.
 */
#define IJKAVSEEK_FORCE 0x20000

void ijk_av_freep(void *arg);

int ijk_av_strstart(const char *str, const char *pfx, const char **ptr);
#endif  // IJKAVUTIL_IJKUTILS_H

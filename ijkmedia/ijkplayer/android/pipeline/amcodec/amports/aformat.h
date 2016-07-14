/**
* @file aformat.h
* @brief  Porting from decoder driver for audio format
* @author Tim Yao <timyao@amlogic.com>
* @version 1.0.0
* @date 2011-02-24
*/
/* Copyright (C) 2007-2011, Amlogic Inc.
* All right reserved
* 
*/

/*
 * AMLOGIC Audio/Video streaming port driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the named License,
 * or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Author:  Tim Yao <timyao@amlogic.com>
 *
 */

#ifndef AFORMAT_H
#define AFORMAT_H

typedef enum {
    AFORMAT_UNKNOWN = -1,
    AFORMAT_MPEG   = 0,
    AFORMAT_PCM_S16LE = 1,
    AFORMAT_AAC   = 2,
    AFORMAT_AC3   = 3,
    AFORMAT_ALAW = 4,
    AFORMAT_MULAW = 5,
    AFORMAT_DTS = 6,
    AFORMAT_PCM_S16BE = 7,
    AFORMAT_FLAC = 8,
    AFORMAT_COOK = 9,
    AFORMAT_PCM_U8 = 10,
    AFORMAT_ADPCM = 11,
    AFORMAT_AMR  = 12,
    AFORMAT_RAAC  = 13,
    AFORMAT_WMA  = 14,
    AFORMAT_WMAPRO   = 15,
    AFORMAT_PCM_BLURAY  = 16,
    AFORMAT_ALAC  = 17,
    AFORMAT_VORBIS    = 18,
    AFORMAT_AAC_LATM   = 19,
    AFORMAT_APE   = 20,
    AFORMAT_UNSUPPORT ,
    AFORMAT_MAX    

} aformat_t;

#define AUDIO_EXTRA_DATA_SIZE   (4096)
#define IS_AFMT_VALID(afmt)	((afmt > AFORMAT_UNKNOWN) && (afmt < AFORMAT_MAX))
    
#define IS_AUIDO_NEED_EXT_INFO(afmt) ((afmt == AFORMAT_ADPCM) \
								 ||(afmt == AFORMAT_WMA) \
								 ||(afmt == AFORMAT_WMAPRO) \
								 ||(afmt == AFORMAT_PCM_S16BE) \
								 ||(afmt == AFORMAT_PCM_S16LE) \
								 ||(afmt == AFORMAT_PCM_U8) \
								 ||(afmt == AFORMAT_PCM_BLURAY) \
								 ||(afmt == AFORMAT_AMR)\
								 ||(afmt == AFORMAT_ALAC)\
								 ||(afmt == AFORMAT_AC3) \
								 ||(afmt == AFORMAT_APE) \
								 ||(afmt == AFORMAT_FLAC) )


#define IS_AUDIO_NOT_SUPPORT_EXCEED_2CH(afmt) ((afmt == AFORMAT_RAAC) \
										||(afmt == AFORMAT_COOK) \
										||(afmt == AFORMAT_FLAC))

#define IS_AUIDO_NEED_PREFEED_HEADER(afmt) ((afmt == AFORMAT_VORBIS) )

#endif /* AFORMAT_H */


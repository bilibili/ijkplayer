/*
 * mpeg4_esds.h
 * 
 * Copyright (c) 2015 yuazhen <zhengyuan10503@gmail.com>
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

#include <limits.h>
#include <stdint.h>
#include "libavcodec/avcodec.h"

static void restore_mpeg4_esds(AVCodecParameters *codecpar,
                                 uint8_t *p_buf, size_t i_buf_size,
                                 size_t i_es_dscr_length, size_t i_dec_dscr_length,
                                 uint8_t *p_esds_buf)
{
    p_esds_buf[0] = 0x03;
    p_esds_buf[1] = 0x80;
    p_esds_buf[2] = 0x80;
    p_esds_buf[3] = 0x80;
    p_esds_buf[4] = i_es_dscr_length;
    uint16_t *es_id = (uint16_t *)&p_esds_buf[5];
    *es_id = htobe16(1);
    
    p_esds_buf[8] = 0x04;
    p_esds_buf[9] = 0x80;
    p_esds_buf[10] = 0x80;
    p_esds_buf[11] = 0x80;
    p_esds_buf[12] = i_dec_dscr_length;
    p_esds_buf[13] = 0x20;
    p_esds_buf[14] = 0x11;
    uint32_t *max_br = (uint32_t *)&p_esds_buf[18];
    uint32_t *avg_br = (uint32_t *)&p_esds_buf[22];
    *max_br = *avg_br = htobe32(codecpar->bit_rate);

    p_esds_buf[26] = 0x05;
    p_esds_buf[27] = 0x80;
    p_esds_buf[28] = 0x80;
    p_esds_buf[29] = 0x80;
    p_esds_buf[30] = i_buf_size;
    memcpy(&p_esds_buf[31], p_buf, i_buf_size);

    p_esds_buf[31+i_buf_size] = 0x06;
    p_esds_buf[31+i_buf_size+1] = 0x80;
    p_esds_buf[31+i_buf_size+2] = 0x80;
    p_esds_buf[31+i_buf_size+3] = 0x80;
    p_esds_buf[31+i_buf_size+4] = 0x01;
    p_esds_buf[31+i_buf_size+5] = 0x02;
}

/*****************************************************************************
 * chroma_neon.h
 *****************************************************************************
 * Copyright (C) 2011 Rémi Denis-Courmont
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/* Planes must start on a 16-bytes boundary. Pitches must be multiples of 16
 * bytes even for subsampled components. */

/* Planar picture buffer.
 * Pitch corresponds to luminance component in bytes. Chrominance pitches are
 * inferred from the color subsampling ratio. */
struct yuv_planes
{
    void *y, *u, *v;
    size_t pitch;
};

struct yuv_planes_in
{
    const void *y, *u, *v;
    size_t pitch;
};

/* Packed picture buffer. Pitch is in bytes (_not_ pixels). */
struct yuv_pack
{
    void *yuv;
    size_t pitch;
};

/* I420 to RGBA conversion. */
void i420_rgb_neon (struct yuv_pack *const out,
                    const struct yuv_planes_in *const in,
                    int width, int height) __asm__("i420_rgb_neon");

/* I420 to RV16 conversion. */
void i420_rv16_neon (struct yuv_pack *const out,
                     const struct yuv_planes_in *const in,
                     int width, int height) __asm__("i420_rv16_neon");

/* NV21 to RGBA conversion. */
void nv21_rgb_neon (struct yuv_pack *const out,
                    const struct yuv_planes_in *const in,
                    int width, int height) __asm__("nv21_rgb_neon");

/* NV12 to RGBA conversion. */
void nv12_rgb_neon (struct yuv_pack *const out,
                    const struct yuv_planes_in *const in,
                    int width, int height) __asm__("nv12_rgb_neon");

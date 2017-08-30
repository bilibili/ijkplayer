/*
 * This file is part of Ijkplayer.
 *
 * Ijkplayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Ijkplayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Ijkplayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * a very simple circular buffer FIFO implementation
 */

#ifndef IJKUTIL_IJKFIFO_H
#define IJKUTIL_IJKFIFO_H

#include <stdint.h>
#include <stddef.h>

typedef struct IjkFifoBuffer {
    uint8_t *buffer;
    uint8_t *rptr, *wptr, *end;
    uint64_t rndx, wndx;
} IjkFifoBuffer;

/**
 * Initialize an IjkFifoBuffer.
 * @param size of FIFO
 * @return IjkFifoBuffer or NULL in case of memory allocation failure
 */
IjkFifoBuffer *ijk_av_fifo_alloc(unsigned int size);

/**
 * Initialize an IjkFifoBuffer.
 * @param nmemb number of elements
 * @param size  size of the single element
 * @return IjkFifoBuffer or NULL in case of memory allocation failure
 */
IjkFifoBuffer *ijk_av_fifo_alloc_array(size_t nmemb, size_t size);

/**
 * Free an IjkFifoBuffer.
 * @param f IjkFifoBuffer to free
 */
void ijk_av_fifo_free(IjkFifoBuffer *f);

/**
 * Free an IjkFifoBuffer and reset pointer to NULL.
 * @param f IjkFifoBuffer to free
 */
void ijk_av_fifo_freep(IjkFifoBuffer **f);

/**
 * Reset the IjkFifoBuffer to the state right after ijk_av_fifo_alloc, in particular it is emptied.
 * @param f IjkFifoBuffer to reset
 */
void ijk_av_fifo_reset(IjkFifoBuffer *f);

/**
 * Return the amount of data in bytes in the IjkFifoBuffer, that is the
 * amount of data you can read from it.
 * @param f IjkFifoBuffer to read from
 * @return size
 */
int ijk_av_fifo_size(const IjkFifoBuffer *f);

/**
 * Return the amount of space in bytes in the IjkFifoBuffer, that is the
 * amount of data you can write into it.
 * @param f IjkFifoBuffer to write into
 * @return size
 */
int ijk_av_fifo_space(const IjkFifoBuffer *f);

/**
 * Feed data at specific position from an IjkFifoBuffer to a user-supplied callback.
 * Similar as ijk_av_fifo_gereric_read but without discarding data.
 * @param f IjkFifoBuffer to read from
 * @param offset offset from current read position
 * @param buf_size number of bytes to read
 * @param func generic read function
 * @param dest data destination
 */
int ijk_av_fifo_generic_peek_at(IjkFifoBuffer *f, void *dest, int offset, int buf_size, void (*func)(void*, void*, int));

/**
 * Feed data from an IjkFifoBuffer to a user-supplied callback.
 * Similar as ijk_av_fifo_gereric_read but without discarding data.
 * @param f IjkFifoBuffer to read from
 * @param buf_size number of bytes to read
 * @param func generic read function
 * @param dest data destination
 */
int ijk_av_fifo_generic_peek(IjkFifoBuffer *f, void *dest, int buf_size, void (*func)(void*, void*, int));

/**
 * Feed data from an IjkFifoBuffer to a user-supplied callback.
 * @param f IjkFifoBuffer to read from
 * @param buf_size number of bytes to read
 * @param func generic read function
 * @param dest data destination
 */
int ijk_av_fifo_generic_read(IjkFifoBuffer *f, void *dest, int buf_size, void (*func)(void*, void*, int));

/**
 * Feed data from a user-supplied callback to an IjkFifoBuffer.
 * @param f IjkFifoBuffer to write to
 * @param src data source; non-const since it may be used as a
 * modifiable context by the function defined in func
 * @param size number of bytes to write
 * @param func generic write function; the first parameter is src,
 * the second is dest_buf, the third is dest_buf_size.
 * func must return the number of bytes written to dest_buf, or <= 0 to
 * indicate no more data available to write.
 * If func is NULL, src is interpreted as a simple byte array for source data.
 * @return the number of bytes written to the FIFO
 */
int ijk_av_fifo_generic_write(IjkFifoBuffer *f, void *src, int size, int (*func)(void*, void*, int));

/**
 * Resize an IjkFifoBuffer.
 * In case of reallocation failure, the old FIFO is kept unchanged.
 *
 * @param f IjkFifoBuffer to resize
 * @param size new IjkFifoBuffer size in bytes
 * @return <0 for failure, >=0 otherwise
 */
int ijk_av_fifo_realloc2(IjkFifoBuffer *f, unsigned int size);

/**
 * Enlarge an IjkFifoBuffer.
 * In case of reallocation failure, the old FIFO is kept unchanged.
 * The new fifo size may be larger than the requested size.
 *
 * @param f IjkFifoBuffer to resize
 * @param additional_space the amount of space in bytes to allocate in addition to ijk_av_fifo_size()
 * @return <0 for failure, >=0 otherwise
 */
int ijk_av_fifo_grow(IjkFifoBuffer *f, unsigned int additional_space);

/**
 * Read and discard the specified amount of data from an IjkFifoBuffer.
 * @param f IjkFifoBuffer to read from
 * @param size amount of data to read in bytes
 */
void ijk_av_fifo_drain(IjkFifoBuffer *f, int size);

/**
 * Return a pointer to the data stored in a FIFO buffer at a certain offset.
 * The FIFO buffer is not modified.
 *
 * @param f    IjkFifoBuffer to peek at, f must be non-NULL
 * @param offs an offset in bytes, its absolute value must be less
 *             than the used buffer size or the returned pointer will
 *             point outside to the buffer data.
 *             The used buffer size can be checked with ijk_av_fifo_size().
 */
static inline uint8_t *ijk_av_fifo_peek2(const IjkFifoBuffer *f, int offs)
{
    uint8_t *ptr = f->rptr + offs;
    if (ptr >= f->end)
        ptr = f->buffer + (ptr - f->end);
    else if (ptr < f->buffer)
        ptr = f->end - (f->buffer - ptr);
    return ptr;
}

#endif /* IJKUTIL_IJKFIFO_H */

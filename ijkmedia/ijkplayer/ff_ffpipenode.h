/*
 * ff_ffpipenode.h
 *
 * Copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

#ifndef FFPLAY__FF_FFPIPENODE_H
#define FFPLAY__FF_FFPIPENODE_H

#include "ijksdl/ijksdl_mutex.h"

typedef struct IJKFF_Pipenode_Opaque IJKFF_Pipenode_Opaque;
typedef struct IJKFF_Pipenode IJKFF_Pipenode;
struct IJKFF_Pipenode {
    SDL_mutex *mutex;
    void *opaque;

    void (*func_destroy) (IJKFF_Pipenode *node);
    int  (*func_run_sync)(IJKFF_Pipenode *node);
    int  (*func_flush)   (IJKFF_Pipenode *node); // optional
};

IJKFF_Pipenode *ffpipenode_alloc(size_t opaque_size);
void ffpipenode_free(IJKFF_Pipenode *node);
void ffpipenode_free_p(IJKFF_Pipenode **node);

int  ffpipenode_run_sync(IJKFF_Pipenode *node);
int  ffpipenode_flush(IJKFF_Pipenode *node);

#endif

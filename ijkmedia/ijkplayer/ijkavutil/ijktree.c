/*
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "ijktree.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

typedef struct IjkAVTreeNode {
    struct IjkAVTreeNode *child[2];
    void *elem;
    int state;
} IjkAVTreeNode;

const int ijk_av_tree_node_size = sizeof(IjkAVTreeNode);

struct IjkAVTreeNode *ijk_av_tree_node_alloc(void)
{
    return calloc(1, sizeof(struct IjkAVTreeNode));
}

void *ijk_av_tree_find(const IjkAVTreeNode *t, void *key,
                   int (*cmp)(const void *key, const void *b), void *next[2])
{
    if (t) {
        unsigned int v = cmp(key, t->elem);
        if (v) {
            if (next)
                next[v >> 31] = t->elem;
            return ijk_av_tree_find(t->child[(v >> 31) ^ 1], key, cmp, next);
        } else {
            if (next) {
                ijk_av_tree_find(t->child[0], key, cmp, next);
                ijk_av_tree_find(t->child[1], key, cmp, next);
            }
            return t->elem;
        }
    }
    return NULL;
}

void *ijk_av_tree_insert(IjkAVTreeNode **tp, void *key,
                     int (*cmp)(const void *key, const void *b), IjkAVTreeNode **next)
{
    IjkAVTreeNode *t = *tp;
    if (t) {
        unsigned int v = cmp(t->elem, key);
        void *ret;
        if (!v) {
            if (*next)
                return t->elem;
            else if (t->child[0] || t->child[1]) {
                int i = !t->child[0];
                void *next_elem[2];
                ijk_av_tree_find(t->child[i], key, cmp, next_elem);
                key = t->elem = next_elem[i];
                v   = -i;
            } else {
                *next = t;
                *tp   = NULL;
                return NULL;
            }
        }
        ret = ijk_av_tree_insert(&t->child[v >> 31], key, cmp, next);
        if (!ret) {
            int i              = (v >> 31) ^ !!*next;
            IjkAVTreeNode **child = &t->child[i];
            t->state += 2 * i - 1;

            if (!(t->state & 1)) {
                if (t->state) {
                    /* The following code is equivalent to
                     * if ((*child)->state * 2 == -t->state)
                     *     rotate(child, i ^ 1);
                     * rotate(tp, i);
                     *
                     * with rotate():
                     * static void rotate(AVTreeNode **tp, int i)
                     * {
                     *     AVTreeNode *t= *tp;
                     *
                     *     *tp = t->child[i];
                     *     t->child[i] = t->child[i]->child[i ^ 1];
                     *     (*tp)->child[i ^ 1] = t;
                     *     i = 4 * t->state + 2 * (*tp)->state + 12;
                     *     t->state     = ((0x614586 >> i) & 3) - 1;
                     *     (*tp)->state = ((0x400EEA >> i) & 3) - 1 +
                     *                    ((*tp)->state >> 1);
                     * }
                     * but such a rotate function is both bigger and slower
                     */
                    if ((*child)->state * 2 == -t->state) {
                        *tp                    = (*child)->child[i ^ 1];
                        (*child)->child[i ^ 1] = (*tp)->child[i];
                        (*tp)->child[i]        = *child;
                        *child                 = (*tp)->child[i ^ 1];
                        (*tp)->child[i ^ 1]    = t;

                        (*tp)->child[0]->state = -((*tp)->state > 0);
                        (*tp)->child[1]->state = (*tp)->state < 0;
                        (*tp)->state           = 0;
                    } else {
                        *tp                 = *child;
                        *child              = (*child)->child[i ^ 1];
                        (*tp)->child[i ^ 1] = t;
                        if ((*tp)->state)
                            t->state = 0;
                        else
                            t->state >>= 1;
                        (*tp)->state = -t->state;
                    }
                }
            }
            if (!(*tp)->state ^ !!*next)
                return key;
        }
        return ret;
    } else {
        *tp   = *next;
        *next = NULL;
        if (*tp) {
            (*tp)->elem = key;
            return NULL;
        } else
            return key;
    }
}

void ijk_av_tree_destroy(IjkAVTreeNode *t)
{
    if (t) {
        ijk_av_tree_destroy(t->child[0]);
        ijk_av_tree_destroy(t->child[1]);
        free(t);
    }
}

void ijk_av_tree_enumerate(IjkAVTreeNode *t, void *opaque,
                       int (*cmp)(void *opaque, void *elem),
                       int (*enu)(void *opaque, void *elem))
{
    if (t) {
        int v = cmp ? cmp(opaque, t->elem) : 0;
        if (v >= 0)
            ijk_av_tree_enumerate(t->child[0], opaque, cmp, enu);
        if (v == 0)
            enu(opaque, t->elem);
        if (v <= 0)
            ijk_av_tree_enumerate(t->child[1], opaque, cmp, enu);
    }
}

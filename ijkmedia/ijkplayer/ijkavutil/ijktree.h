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

#ifndef IJKAVUTIL_IJKTREE_H
#define IJKAVUTIL_IJKTREE_H

/**
 * @addtogroup lavu_tree IjkAVTree
 * @ingroup lavu_data
 *
 * Low-complexity tree container
 *
 * Insertion, removal, finding equal, largest which is smaller than and
 * smallest which is larger than, all have O(log n) worst-case complexity.
 * @{
 */


struct IjkAVTreeNode;
extern const int ijk_av_tree_node_size;

/**
 * Allocate an IjkAVTreeNode.
 */
struct IjkAVTreeNode *ijk_av_tree_node_alloc(void);

/**
 * Find an element.
 * @param root a pointer to the root node of the tree
 * @param next If next is not NULL, then next[0] will contain the previous
 *             element and next[1] the next element. If either does not exist,
 *             then the corresponding entry in next is unchanged.
 * @param cmp compare function used to compare elements in the tree,
 *            API identical to that of Standard C's qsort
 *            It is guranteed that the first and only the first argument to cmp()
 *            will be the key parameter to av_tree_find(), thus it could if the
 *            user wants, be a different type (like an opaque context).
 * @return An element with cmp(key, elem) == 0 or NULL if no such element
 *         exists in the tree.
 */
void *ijk_av_tree_find(const struct IjkAVTreeNode *root, void *key,
                   int (*cmp)(const void *key, const void *b), void *next[2]);

/**
 * Insert or remove an element.
 *
 * If *next is NULL, then the supplied element will be removed if it exists.
 * If *next is non-NULL, then the supplied element will be inserted, unless
 * it already exists in the tree.
 *
 * @param rootp A pointer to a pointer to the root node of the tree; note that
 *              the root node can change during insertions, this is required
 *              to keep the tree balanced.
 * @param key  pointer to the element key to insert in the tree
 * @param next Used to allocate and free AVTreeNodes. For insertion the user
 *             must set it to an allocated and zeroed object of at least
 *             av_tree_node_size bytes size. av_tree_insert() will set it to
 *             NULL if it has been consumed.
 *             For deleting elements *next is set to NULL by the user and
 *             av_tree_insert() will set it to the AVTreeNode which was
 *             used for the removed element.
 *             This allows the use of flat arrays, which have
 *             lower overhead compared to many malloced elements.
 *             You might want to define a function like:
 *             @code
 *             void *tree_insert(struct AVTreeNode **rootp, void *key,
 *                               int (*cmp)(void *key, const void *b),
 *                               AVTreeNode **next)
 *             {
 *                 if (!*next)
 *                     *next = av_mallocz(av_tree_node_size);
 *                 return av_tree_insert(rootp, key, cmp, next);
 *             }
 *             void *tree_remove(struct AVTreeNode **rootp, void *key,
 *                               int (*cmp)(void *key, const void *b, AVTreeNode **next))
 *             {
 *                 av_freep(next);
 *                 return av_tree_insert(rootp, key, cmp, next);
 *             }
 *             @endcode
 * @param cmp compare function used to compare elements in the tree, API identical
 *            to that of Standard C's qsort
 * @return If no insertion happened, the found element; if an insertion or
 *         removal happened, then either key or NULL will be returned.
 *         Which one it is depends on the tree state and the implementation. You
 *         should make no assumptions that it's one or the other in the code.
 */
void *ijk_av_tree_insert(struct IjkAVTreeNode **rootp, void *key,
                     int (*cmp)(const void *key, const void *b),
                     struct IjkAVTreeNode **next);

void ijk_av_tree_destroy(struct IjkAVTreeNode *t);

/**
 * Apply enu(opaque, &elem) to all the elements in the tree in a given range.
 *
 * @param cmp a comparison function that returns < 0 for an element below the
 *            range, > 0 for an element above the range and == 0 for an
 *            element inside the range
 *
 * @note The cmp function should use the same ordering used to construct the
 *       tree.
 */
void ijk_av_tree_enumerate(struct IjkAVTreeNode *t, void *opaque,
                       int (*cmp)(void *opaque, void *elem),
                       int (*enu)(void *opaque, void *elem));

/**
 * @}
 */

#endif /* IJKAVUTIL_IJKTREE_H */

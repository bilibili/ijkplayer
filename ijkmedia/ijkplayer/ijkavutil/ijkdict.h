/*
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
 * You should have ijkPlayer a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Public dictionary API.
 * @deprecated
 *  AVDictionary is provided for compatibility with libav. It is both in
 *  implementation as well as API inefficient. It does not scale and is
 *  extremely slow with large dictionaries.
 *  It is recommended that new code uses our tree container from tree.c/h
 *  where applicable, which uses AVL trees to achieve O(log n) performance.
 */

#ifndef IJKAVUTIL_IJKDICT_H
#define IJKAVUTIL_IJKDICT_H

#include <stdint.h>

#define IJK_AV_DICT_MATCH_CASE      1   /**< Only get an entry with exact-case key match. Only relevant in av_dict_get(). */
#define IJK_AV_DICT_IGNORE_SUFFIX   2   /**< Return first entry in a dictionary whose first part corresponds to the search key,
                                         ignoring the suffix of the found key string. Only relevant in av_dict_get(). */
#define IJK_AV_DICT_DONT_STRDUP_KEY 4   /**< Take ownership of a key that's been
                                         allocated with av_malloc() or another memory allocation function. */
#define IJK_AV_DICT_DONT_STRDUP_VAL 8   /**< Take ownership of a value that's been
                                         allocated with av_malloc() or another memory allocation function. */
#define IJK_AV_DICT_DONT_OVERWRITE 16   ///< Don't overwrite existing entries.
#define IJK_AV_DICT_APPEND         32   /**< If the entry already exists, append to it.  Note that no
                                      delimiter is added, the strings are simply concatenated. */
#define IJK_AV_DICT_MULTIKEY       64   /**< Allow to store several equal keys in the dictionary */

typedef struct IjkAVDictionaryEntry {
    char *key;
    char *value;
} IjkAVDictionaryEntry;

typedef struct IjkAVDictionary IjkAVDictionary;

/**
 * Get a dictionary entry with matching key.
 *
 * The returned entry key or value must not be changed, or it will
 * cause undefined behavior.
 *
 * To iterate through all the dictionary entries, you can set the matching key
 * to the null string "" and set the AV_DICT_IGNORE_SUFFIX flag.
 *
 * @param prev Set to the previous matching element to find the next.
 *             If set to NULL the first matching element is returned.
 * @param key matching key
 * @param flags a collection of AV_DICT_* flags controlling how the entry is retrieved
 * @return found entry or NULL in case no matching entry was found in the dictionary
 */
IjkAVDictionaryEntry *ijk_av_dict_get(const IjkAVDictionary *m, const char *key,
                               const IjkAVDictionaryEntry *prev, int flags);

/**
 * Get number of entries in dictionary.
 *
 * @param m dictionary
 * @return  number of entries in dictionary
 */
int ijk_av_dict_count(const IjkAVDictionary *m);

/**
 * Set the given entry in *pm, overwriting an existing entry.
 *
 * Note: If AV_DICT_DONT_STRDUP_KEY or AV_DICT_DONT_STRDUP_VAL is set,
 * these arguments will be freed on error.
 *
 * Warning: Adding a new entry to a dictionary invalidates all existing entries
 * previously returned with av_dict_get.
 *
 * @param pm pointer to a pointer to a dictionary struct. If *pm is NULL
 * a dictionary struct is allocated and put in *pm.
 * @param key entry key to add to *pm (will either be av_strduped or added as a new key depending on flags)
 * @param value entry value to add to *pm (will be av_strduped or added as a new key depending on flags).
 *        Passing a NULL value will cause an existing entry to be deleted.
 * @return >= 0 on success otherwise an error code <0
 */
int ijk_av_dict_set(IjkAVDictionary **pm, const char *key, const char *value, int flags);

/**
 * Convenience wrapper for av_dict_set that converts the value to a string
 * and stores it.
 *
 * Note: If AV_DICT_DONT_STRDUP_KEY is set, key will be freed on error.
 */
int ijk_av_dict_set_int(IjkAVDictionary **pm, const char *key, int64_t value, int flags);

/**
 * Copy entries from one AVDictionary struct into another.
 * @param dst pointer to a pointer to a AVDictionary struct. If *dst is NULL,
 *            this function will allocate a struct for you and put it in *dst
 * @param src pointer to source AVDictionary struct
 * @param flags flags to use when setting entries in *dst
 * @note metadata is read using the AV_DICT_IGNORE_SUFFIX flag
 * @return 0 on success, negative AVERROR code on failure. If dst was allocated
 *           by this function, callers should free the associated memory.
 */
int ijk_av_dict_copy(IjkAVDictionary **dst, const IjkAVDictionary *src, int flags);

/**
 * Free all the memory allocated for an AVDictionary struct
 * and all keys and values.
 */
void ijk_av_dict_free(IjkAVDictionary **m);


/**
 * @}
 */

#endif /* IJKAVUTIL_IJKDICT_H */

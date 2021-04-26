/*
 * ijkaudiocallback.h
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

#ifndef IJKPLAYER__IJKAUDIOCALLBACK_H
#define IJKPLAYER__IJKAUDIOCALLBACK_H

/* Include support for DSP callback function in library build */
#define NOMIT_AUDIO_DSP_CALLBACK_FN (0)

/* Callback function prototype */
typedef int (*AUDIO_DSP_CBFN) ( void **inData,           unsigned int  numInSamples,
                                void **outData,          unsigned int *numOutSamples,
                                unsigned int sampleRate, unsigned int  numChans,
                                unsigned int format);

#endif//IJKPLAYER__IJKAUDIOCALLBACK_H

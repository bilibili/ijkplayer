/*****************************************************************************
* ijksdl_vout_callback.h
*****************************************************************************
*
* copyright (c) 2019 befovy <befovy@gmail.com>
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

#include "../ijksdl_vout.h"

SDL_Vout *SDL_Vout_Callback_Create();


typedef int(*ijkmp_vout_callback)(void *userdata,
    int w, int h, int sar_num, int sar_den,
    int planes, uint16_t *linesize, uint8_t **pixels);


int SDL_Vout_Set_Callback(SDL_Vout *vout, void *userdata, ijkmp_vout_callback callback);
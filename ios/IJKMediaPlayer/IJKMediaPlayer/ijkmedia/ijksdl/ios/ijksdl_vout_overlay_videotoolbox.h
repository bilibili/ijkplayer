#ifndef __IJKMediaPlayer__ijksdl_vout_overlay_videotoolbox__
#define __IJKMediaPlayer__ijksdl_vout_overlay_videotoolbox__

#include "ijksdl_stdinc.h"
#include "ijksdl_vout.h"
#include "IJKVideoToolBox.h"
#include "ijksdl_inc_ffmpeg.h"

SDL_VoutOverlay *SDL_VoutVideoToolBox_CreateOverlay(int width, int height, Uint32 format, SDL_Vout *vout);
int SDL_VoutOverlayVideoToolBox_FillFrame(SDL_VoutOverlay *overlay, VTBPicture* picture);

#endif

#ifndef FFPLAY__FF_FFPIPENODE_IOS_VIDEOTOOLBOX_DEC_H
#define FFPLAY__FF_FFPIPENODE_IOS_VIDEOTOOLBOX_DEC_H

#include "ijkplayer/ff_ffpipenode.h"

typedef struct FFPlayer FFPlayer;

IJKFF_Pipenode *ffpipenode_create_video_decoder_from_ios_videotoolbox(FFPlayer *ffp);

#endif

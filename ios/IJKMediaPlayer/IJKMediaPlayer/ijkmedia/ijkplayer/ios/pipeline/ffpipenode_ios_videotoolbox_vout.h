#ifndef FFPLAY__FF_FFPIPENODE_IOS_VIDEOTOOLBOX_VOUT_H
#define FFPLAY__FF_FFPIPENODE_IOS_VIDEOTOOLBOX_VOUT_H

#include "ijkplayer/ff_ffpipenode.h"

typedef struct FFPlayer FFPlayer;

IJKFF_Pipenode *ffpipenode_create_video_output_from_ios_videotoolbox(FFPlayer *ffp);

#endif

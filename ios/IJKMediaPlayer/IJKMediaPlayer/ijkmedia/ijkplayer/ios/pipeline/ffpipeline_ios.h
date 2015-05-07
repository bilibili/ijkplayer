#ifndef FFPLAY__FF_FFPIPELINE_IOS_H
#define FFPLAY__FF_FFPIPELINE_IOS_H

#include "ijkplayer/ff_ffpipeline.h"

typedef struct FFPlayer       FFPlayer;
typedef struct IJKFF_Pipeline IJKFF_Pipeline;

IJKFF_Pipeline *ffpipeline_create_from_ios(FFPlayer *ffp);
void ffpipeline_ios_set_frame_max_width(IJKFF_Pipeline *pipeline, int width);
int  ffpipeline_ios_get_frame_max_width(IJKFF_Pipeline *pipeline);
void ffpipeline_ios_set_videotoolbox_enabled(IJKFF_Pipeline *pipeline, int enabled);

#endif

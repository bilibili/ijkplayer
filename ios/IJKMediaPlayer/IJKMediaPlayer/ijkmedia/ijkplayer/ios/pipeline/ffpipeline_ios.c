#include "ffpipeline_ios.h"
#include "ffpipenode_ios_videotoolbox_vdec.h"
#include "ffpipenode_ios_videotoolbox_vout.h"
#include "ffpipenode_ffplay_vdec.h"
#include "ff_ffplay.h"

typedef struct IJKFF_Pipeline_Opaque {
    FFPlayer    *ffp;
    int          m_max_frame_width;
    int          m_videotoolbox_enable;
    bool         is_videotoolbox_open;
} IJKFF_Pipeline_Opaque;

static void func_destroy(IJKFF_Pipeline *pipeline)
{
}

void ffpipeline_ios_set_frame_max_width(IJKFF_Pipeline *pipeline, int max_frame_width)
{
    IJKFF_Pipeline_Opaque *opaque = pipeline->opaque;
    opaque->m_max_frame_width = max_frame_width;
}


int ffpipeline_ios_get_frame_max_width(IJKFF_Pipeline *pipeline)
{
    IJKFF_Pipeline_Opaque *opaque = pipeline->opaque;
    return opaque->m_max_frame_width;
}


void ffpipeline_ios_set_videotoolbox_enabled(IJKFF_Pipeline *pipeline, int enabled)
{
    IJKFF_Pipeline_Opaque *opaque = pipeline->opaque;
    opaque->m_videotoolbox_enable = enabled;
}

static IJKFF_Pipenode *func_open_video_decoder(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    IJKFF_Pipenode* node = NULL;
    IJKFF_Pipeline_Opaque *opaque = pipeline->opaque;
    if (opaque->m_videotoolbox_enable) {
        node = ffpipenode_create_video_decoder_from_ios_videotoolbox(ffp);
    }
    if (node == NULL) {
        ALOGE("vtb fail!!! switch to ffmpeg decode!!!! \n");
        node = ffpipenode_create_video_decoder_from_ffplay(ffp);
        opaque->is_videotoolbox_open = false;
    } else {
        opaque->is_videotoolbox_open = true;
    }
    ffp_notify_msg2(ffp, FFP_MSG_VIDEO_DECODER_OPEN, opaque->is_videotoolbox_open);
    return node;
}

static IJKFF_Pipenode *func_open_video_output(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    return ffpipenode_create_video_output_from_ios_videotoolbox(ffp);
}

static SDL_Class g_pipeline_class = {
    .name = "ffpipeline_ios",
};

IJKFF_Pipeline *ffpipeline_create_from_ios(FFPlayer *ffp)
{
    IJKFF_Pipeline *pipeline = ffpipeline_alloc(&g_pipeline_class, sizeof(IJKFF_Pipeline_Opaque));
    if (!pipeline)
        return pipeline;

    IJKFF_Pipeline_Opaque *opaque     = pipeline->opaque;
    opaque->ffp                       = ffp;
    opaque->m_max_frame_width         = 0;
    opaque->m_videotoolbox_enable     = 0;
    pipeline->func_destroy            = func_destroy;
    pipeline->func_open_video_decoder = func_open_video_decoder;
    pipeline->func_open_video_output  = func_open_video_output;

    return pipeline;
fail:
    ffpipeline_free_p(&pipeline);
    return NULL;
}

#include "ffpipenode_ios_videotoolbox_vout.h"
#include "ff_ffpipeline.h"
#include "IJKVideoToolBox.h"
#include "ijkplayer/ff_ffpipenode.h"
#include "ijkplayer/ff_ffplay.h"

typedef struct IJKFF_Pipenode_Opaque {
    FFPlayer *ffp;
} IJKFF_Pipenode_Opaque;

static void func_destroy(IJKFF_Pipenode *node)
{
    // do nothing
}

static int func_run_sync(IJKFF_Pipenode *node)
{
    IJKFF_Pipenode_Opaque *opaque = node->opaque;
    int ret = ffp_video_refresh_thread(opaque->ffp);
    return ret;
}

IJKFF_Pipenode *ffpipenode_create_video_output_from_ios_videotoolbox(FFPlayer *ffp)
{
    IJKFF_Pipenode *node = ffpipenode_alloc(sizeof(IJKFF_Pipenode_Opaque));
    if (!node)
        return node;
    IJKFF_Pipenode_Opaque *opaque = node->opaque;
    opaque->ffp         = ffp;
    node->func_destroy  = func_destroy;
    node->func_run_sync = func_run_sync;
    return node;
}

/* Compile the production adapter against minimal pipeline dependency types. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
enum { AV_CODEC_ID_H264, AV_CODEC_ID_HEVC, AV_CODEC_ID_MPEG4 };
typedef struct { int codec_id; } AVCodecParameters;
typedef struct {
    AVCodecParameters *codecpar;
    char acodec_name[128];
    int frame_width, frame_height, frame_rotate_degrees;
} IJKFF_Pipenode_Opaque;
#include "../ffamc_output_size.h"

/* Simulate ad -> content -> ad while allocation remains 1920x1080. */
static void test_crop_only_transition(void)
{
    AVCodecParameters par = { AV_CODEC_ID_H264 };
    IJKFF_Pipenode_Opaque o = { .codecpar = &par, .frame_width = 1920, .frame_height = 1080 };
    strcpy(o.acodec_name, "OMX.amlogic.avc.decoder.awesome");
    update_output_frame_size(&o, 1920, 1080, 15, 0, 0, 1919, 1079);
    assert(o.frame_width == 1920 && o.frame_height == 1080);
    update_output_frame_size(&o, 1920, 1080, 15, 0, 0, 1279, 535);
    assert(o.frame_width == 1280 && o.frame_height == 536);
    update_output_frame_size(&o, 1920, 1080, 15, 0, 0, 1279, 535);
    assert(o.frame_width == 1280 && o.frame_height == 536);
    update_output_frame_size(&o, 1920, 1080, 15, 0, 0, 1919, 1079);
    assert(o.frame_width == 1920 && o.frame_height == 1080);
    update_output_frame_size(&o, 1920, 1080, 15, 100, 0, 1379, 1079);
    assert(o.frame_width == 1280 && o.frame_height == 1080);
    update_output_frame_size(&o, 1920, 1080, 15, 0, 100, 1919, 635);
    assert(o.frame_width == 1920 && o.frame_height == 536);
}

/* Missing crop uses the format size; inclusive bounds and rotation are kept. */
static void test_missing_crop_rotation_and_boundaries(void)
{
    AVCodecParameters par = { AV_CODEC_ID_H264 };
    IJKFF_Pipenode_Opaque o = { .codecpar = &par };
    int rotations[] = { 0, 90, 180, 270 };
    for (unsigned i = 0; i < sizeof(rotations) / sizeof(rotations[0]); ++i) {
        o.frame_rotate_degrees = rotations[i];
        int rotated = rotations[i] == 90 || rotations[i] == 270;
        update_output_frame_size(&o, 1280, 536, 0, -1, -1, -1, -1);
        assert(o.frame_width == (rotated ? 536 : 1280));
        assert(o.frame_height == (rotated ? 1280 : 536));
        update_output_frame_size(&o, 1920, 1080, 15, 0, 0, 1279, 535);
        assert(o.frame_width == (rotated ? 536 : 1280));
        assert(o.frame_height == (rotated ? 1280 : 536));
    }
    o.frame_rotate_degrees = 0;
    update_output_frame_size(&o, 1, 1, 15, 0, 0, 0, 0);
    assert(o.frame_width == 1 && o.frame_height == 1);
    update_output_frame_size(&o, INT_MAX, INT_MAX, 15, 0, 0, INT_MAX-1, INT_MAX-1);
    assert(o.frame_width == INT_MAX && o.frame_height == INT_MAX);
}

/* Reject partial, inverted and out-of-bounds geometry without altering state. */
static void test_invalid_geometry(void)
{
    int invalid[][7] = {
        {0,1080,0,0,0,0,0}, {-1,1080,0,0,0,0,0},
        {1920,0,0,0,0,0,0}, {1920,-1,0,0,0,0,0},
        {1920,1080,15,-1,0,1279,535}, {1920,1080,15,0,-1,1279,535},
        {1920,1080,15,1280,0,1279,535}, {1920,1080,15,0,536,1279,535},
        {1920,1080,15,0,0,1920,535}, {1920,1080,15,0,0,1279,1080},
        {1920,1080,15,INT_MIN,0,INT_MAX,535},
        {1920,1080,16,0,0,1279,535}, {1920,1080,-1,0,0,1279,535}
    };
    AVCodecParameters par = { AV_CODEC_ID_H264 };
    IJKFF_Pipenode_Opaque o = { .codecpar = &par, .frame_width = 1280, .frame_height = 536 };
    for (int mask = 1; mask < 15; ++mask) {
        update_output_frame_size(&o, 1920, 1080, mask, 0, 0, 1919, 1079);
        assert(o.frame_width == 1280 && o.frame_height == 536);
    }
    for (unsigned i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        int *v = invalid[i];
        update_output_frame_size(&o, v[0], v[1], v[2], v[3], v[4], v[5], v[6]);
        assert(o.frame_width == 1280 && o.frame_height == 536);
    }
}

/* This patch does not enable the new sizing for HEVC, MPEG4 or legacy TI. */
static void test_unchanged_codec_policy(void)
{
    AVCodecParameters par = { AV_CODEC_ID_HEVC };
    IJKFF_Pipenode_Opaque o = { .codecpar = &par, .frame_width = 1920, .frame_height = 1080 };
    update_output_frame_size(&o, 1920, 1080, 15, 0, 0, 1279, 535);
    assert(o.frame_width == 1920 && o.frame_height == 1080);
    par.codec_id = AV_CODEC_ID_MPEG4;
    update_output_frame_size(&o, 1920, 1080, 15, 0, 0, 1279, 535);
    assert(o.frame_width == 1920 && o.frame_height == 1080);
    par.codec_id = AV_CODEC_ID_H264;
    strcpy(o.acodec_name, "OMX.TI.DUCATI1.VIDEO.DECODER");
    update_output_frame_size(&o, 1920, 1080, 15, 0, 0, 1279, 535);
    assert(o.frame_width == 1920 && o.frame_height == 1080);
    strcpy(o.acodec_name, "omx.ti.ducati1.video.decoder");
    update_output_frame_size(&o, 1920, 1080, 15, 0, 0, 1279, 535);
    assert(o.frame_width == 1920 && o.frame_height == 1080);
    strcpy(o.acodec_name, "OMX.TI.other.decoder");
    update_output_frame_size(&o, 1920, 1080, 15, 0, 0, 1279, 535);
    assert(o.frame_width == 1280 && o.frame_height == 536);
}

/* No FFmpeg, JNI or hardware decoder is required for these host regressions. */
int main(void)
{
    test_crop_only_transition();
    test_missing_crop_rotation_and_boundaries();
    test_invalid_geometry();
    test_unchanged_codec_policy();
    puts("PASS H.264 output crop sizing: ad/content/ad, repeat, offset, one-axis, rotation, invalid geometry and unchanged HEVC/TI policy");
    return 0;
}

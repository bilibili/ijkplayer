#include "ffpipenode_android_mediacodec_vdec.h"
#include "ijksdl/android/ijksdl_android_jni.h"
#include "ijksdl/android/ijksdl_vout_android_nativewindow.h"
#include "ijkplayer/ff_ffpipenode.h"
#include "ijkplayer/ff_ffplay.h"
#include "ijkplayer/ff_ffplay_debug.h"
#include "ffpipeline_android.h"
#include "h264_nal.h"
#include "hevc_nal.h"

#include "amcodec/codec.h"
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

// libamplayer.so entry points
static int (*am_codec_init)(codec_para_t *pcodec);
static int (*am_codec_close)(codec_para_t *pcodec);
static int (*am_codec_reset)(codec_para_t *pcodec);
static int (*am_codec_pause)(codec_para_t *pcodec);
static int (*am_codec_resume)(codec_para_t *pcodec);
static int (*am_codec_write)(codec_para_t *pcodec, void *buffer, int len);
static int (*am_codec_checkin_pts)(codec_para_t *pcodec, unsigned long pts);
static int (*am_codec_get_vbuf_state)(codec_para_t *pcodec, struct buf_status *buf);
static int (*am_codec_get_vdec_state)(codec_para_t *pcodec, struct vdec_status *vdec);
static int (*am_codec_init_cntl)(codec_para_t *pcodec);
static int (*am_codec_poll_cntl)(codec_para_t *pcodec);
static int (*am_codec_set_cntl_mode)(codec_para_t *pcodec, unsigned int mode);
static int (*am_codec_set_cntl_avthresh)(codec_para_t *pcodec, unsigned int avthresh);
static int (*am_codec_set_cntl_syncthresh)(codec_para_t *pcodec, unsigned int syncthresh);

#define HDR_BUF_SIZE 1024
typedef struct hdr_buf {
    char *data;
    int size;
} hdr_buf_t;

typedef struct am_packet {
    AVPacket      avpkt;
    int64_t       avpts;
    int64_t       avdts;
    int           avduration;
    int           isvalid;
    int           newflag;
    int64_t       lastpts;
    unsigned char *data;
    unsigned char *buf;
    int           data_size;
    int           buf_size;
    hdr_buf_t     *hdr;
    codec_para_t  *codec;
} am_packet_t;


typedef enum {
    AM_STREAM_UNKNOWN = 0,
    AM_STREAM_TS,
    AM_STREAM_PS,
    AM_STREAM_ES,
    AM_STREAM_RM,
    AM_STREAM_AUDIO,
    AM_STREAM_VIDEO,
} pstream_type;

typedef struct IJKAM_Pipenode_Opaque {
    FFPlayer                 *ffp;
    //IJKFF_Pipeline           *pipeline;
    Decoder                  *decoder;
    //SDL_Vout                 *weak_vout;

    SDL_Thread               _enqueue_thread;
    SDL_Thread               *enqueue_thread;


    pstream_type      stream_type;
    vformat_t         video_format;
    unsigned int      video_codec_id;
    unsigned int      video_codec_tag;
    unsigned int      video_codec_type;
    int               extrasize;
    uint8_t           *extradata;
    int               video_width;
    int               video_height;
    codec_para_t      vcodec;

    am_packet_t am_pkt;

    int             input_packet_count;
} IJKAM_Pipenode_Opaque;

typedef IJKAM_Pipenode_Opaque am_private_t;


static int func_run_sync(IJKFF_Pipenode *node);
int pre_header_feeding(am_private_t *para, am_packet_t *pkt);
int set_header_info(am_private_t *para);
int write_av_packet(am_private_t *para, am_packet_t *pkt);

static void loadLibrary() {
  // We use Java to call System.loadLibrary as dlopen is weird on Android.
  //void * lib = dlopen("libamplayer.so", RTLD_NOW);

  am_codec_init = dlsym(RTLD_DEFAULT, "codec_init");
  am_codec_close = dlsym(RTLD_DEFAULT, "codec_close");
  am_codec_reset = dlsym(RTLD_DEFAULT, "codec_reset");
  am_codec_pause = dlsym(RTLD_DEFAULT, "codec_pause");
  am_codec_resume = dlsym(RTLD_DEFAULT, "codec_resume");
  am_codec_write = dlsym(RTLD_DEFAULT, "codec_write");
  am_codec_checkin_pts = dlsym(RTLD_DEFAULT, "codec_checkin_pts");
  am_codec_get_vbuf_state = dlsym(RTLD_DEFAULT, "codec_get_vbuf_state");
  am_codec_get_vdec_state = dlsym(RTLD_DEFAULT, "codec_get_vdec_state");
  am_codec_init_cntl = dlsym(RTLD_DEFAULT, "codec_init_cntl");
  am_codec_poll_cntl = dlsym(RTLD_DEFAULT, "codec_poll_cntl");
  am_codec_set_cntl_mode = dlsym(RTLD_DEFAULT, "codec_set_cntl_mode");
  am_codec_set_cntl_avthresh = dlsym(RTLD_DEFAULT, "codec_set_cntl_avthresh");
  am_codec_set_cntl_syncthresh = dlsym(RTLD_DEFAULT, "codec_set_cntl_syncthresh");
}


#define PTS_FREQ        90000
#define UNIT_FREQ       96000
#define AV_SYNC_THRESH  PTS_FREQ*30

#define TRICKMODE_NONE  0x00
#define TRICKMODE_I     0x01
#define TRICKMODE_FFFB  0x02

// same as AV_NOPTS_VALUE
#define INT64_0         INT64_C(0x8000000000000000)

#define EXTERNAL_PTS    (1)
#define SYNC_OUTSIDE    (2)

// missing tags
#define CODEC_TAG_VC_1  (0x312D4356)
#define CODEC_TAG_RV30  (0x30335652)
#define CODEC_TAG_RV40  (0x30345652)
#define CODEC_TAG_MJPEG (0x47504a4d)
#define CODEC_TAG_mjpeg (0x47504a4c)
#define CODEC_TAG_jpeg  (0x6765706a)
#define CODEC_TAG_mjpa  (0x61706a6d)

#define RW_WAIT_TIME    (20 * 1000) // 20ms

#define P_PRE           (0x02000000)
#define F_PRE           (0x03000000)
#define PLAYER_SUCCESS          (0)
#define PLAYER_FAILED           (-(P_PRE|0x01))
#define PLAYER_NOMEM            (-(P_PRE|0x02))
#define PLAYER_EMPTY_P          (-(P_PRE|0x03))

#define PLAYER_WR_FAILED        (-(P_PRE|0x21))
#define PLAYER_WR_EMPTYP        (-(P_PRE|0x22))
#define PLAYER_WR_FINISH        (P_PRE|0x1)

#define PLAYER_PTS_ERROR        (-(P_PRE|0x31))
#define PLAYER_UNSUPPORT        (-(P_PRE|0x35))
#define PLAYER_CHECK_CODEC_ERROR  (-(P_PRE|0x39))



#define LOGDEBUG  0
#define LOGERROR  1

static void Log(int level, const char * fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    __android_log_print(ANDROID_LOG_INFO, "AMCODEC", fmt, args);
    va_end(args);
}


int sysfs_setint(const char * path, const int val)
{
  int fd = open(path, O_RDWR, 0644);
  int ret = 0;
  if (fd >= 0)
  {
    char bcmd[16];
    sprintf(bcmd, "%d", val);
    if (write(fd, bcmd, strlen(bcmd)) < 0)
      ret = -1;
    close(fd);
  }
  if (ret)
    Log(LOGERROR, "%s: error writing %s",__FUNCTION__, path);

  return ret;
}

int sysfs_getint(const char * path, int* val)
{
  int fd = open(path, O_RDONLY);
  int ret = 0;
  if (fd >= 0)
  {
    char bcmd[16];
    if (read(fd, bcmd, sizeof(bcmd)) < 0)
      ret = -1;
    else
      *val = strtol(bcmd, NULL, 16);

    close(fd);
  }
  if (ret)
    Log(LOGERROR, "%s: error reading %s",__FUNCTION__, path);

  return ret;
}

static void am_packet_init(am_packet_t *pkt)
{
  memset(&pkt->avpkt, 0, sizeof(AVPacket));
  pkt->avpts      = 0;
  pkt->avdts      = 0;
  pkt->avduration = 0;
  pkt->isvalid    = 0;
  pkt->newflag    = 0;
  pkt->lastpts    = 0;
  pkt->data       = NULL;
  pkt->buf        = NULL;
  pkt->data_size  = 0;
  pkt->buf_size   = 0;
  pkt->hdr        = NULL;
  //pkt->codec      = NULL;
}

void am_packet_release(am_packet_t *pkt)
{
  if (pkt->buf != NULL)
    free(pkt->buf), pkt->buf= NULL;
  if (pkt->hdr != NULL)
  {
    if (pkt->hdr->data != NULL)
      free(pkt->hdr->data), pkt->hdr->data = NULL;
    free(pkt->hdr), pkt->hdr = NULL;
  }

  //pkt->codec = NULL;
}


static vformat_t codecid_to_vformat(enum AVCodecID id)
{
  vformat_t format;
  switch (id)
  {
    case AV_CODEC_ID_MPEG1VIDEO:
    case AV_CODEC_ID_MPEG2VIDEO:
    case AV_CODEC_ID_MPEG2VIDEO_XVMC:
      format = VFORMAT_MPEG12;
      break;
    case AV_CODEC_ID_H263:
    case AV_CODEC_ID_MPEG4:
    case AV_CODEC_ID_H263P:
    case AV_CODEC_ID_H263I:
    case AV_CODEC_ID_MSMPEG4V2:
    case AV_CODEC_ID_MSMPEG4V3:
    case AV_CODEC_ID_FLV1:
      format = VFORMAT_MPEG4;
      break;
    case AV_CODEC_ID_RV10:
    case AV_CODEC_ID_RV20:
    case AV_CODEC_ID_RV30:
    case AV_CODEC_ID_RV40:
      format = VFORMAT_REAL;
      break;
    case AV_CODEC_ID_H264:
      format = VFORMAT_H264;
      break;
    /*
    case AV_CODEC_ID_H264MVC:
      // H264 Multiview Video Coding (3d blurays)
      format = VFORMAT_H264MVC;
      break;
    */
    case AV_CODEC_ID_MJPEG:
      format = VFORMAT_MJPEG;
      break;
    case AV_CODEC_ID_VC1:
    case AV_CODEC_ID_WMV3:
      format = VFORMAT_VC1;
      break;
    case AV_CODEC_ID_AVS:
    case AV_CODEC_ID_CAVS:
      format = VFORMAT_AVS;
      break;
    case AV_CODEC_ID_HEVC:
      format = VFORMAT_HEVC;
      break;

    default:
      format = VFORMAT_UNSUPPORT;
      break;
  }

  Log(LOGDEBUG, "codecid_to_vformat, id(%d) -> vformat(%d)", (int)id, format);
  return format;
}

static vdec_type_t codec_tag_to_vdec_type(unsigned int codec_tag)
{
  vdec_type_t dec_type;
  switch (codec_tag)
  {
    case CODEC_TAG_MJPEG:
    case CODEC_TAG_mjpeg:
    case CODEC_TAG_jpeg:
    case CODEC_TAG_mjpa:
      // mjpeg
      dec_type = VIDEO_DEC_FORMAT_MJPEG;
      break;
    case CODEC_TAG_XVID:
    case CODEC_TAG_xvid:
    case CODEC_TAG_XVIX:
      // xvid
      dec_type = VIDEO_DEC_FORMAT_MPEG4_5;
      break;
    case CODEC_TAG_COL1:
    case CODEC_TAG_DIV3:
    case CODEC_TAG_MP43:
      // divx3.11
      dec_type = VIDEO_DEC_FORMAT_MPEG4_3;
      break;
    case CODEC_TAG_DIV4:
    case CODEC_TAG_DIVX:
      // divx4
      dec_type = VIDEO_DEC_FORMAT_MPEG4_4;
      break;
    case CODEC_TAG_DIV5:
    case CODEC_TAG_DX50:
    case CODEC_TAG_M4S2:
    case CODEC_TAG_FMP4:
      // divx5
      dec_type = VIDEO_DEC_FORMAT_MPEG4_5;
      break;
    case CODEC_TAG_DIV6:
      // divx6
      dec_type = VIDEO_DEC_FORMAT_MPEG4_5;
      break;
    case CODEC_TAG_MP4V:
    case CODEC_TAG_RMP4:
    case CODEC_TAG_MPG4:
    case CODEC_TAG_mp4v:
    case AV_CODEC_ID_MPEG4:
      // mp4
      dec_type = VIDEO_DEC_FORMAT_MPEG4_5;
      break;
    case AV_CODEC_ID_H263:
    case CODEC_TAG_H263:
    case CODEC_TAG_h263:
    case CODEC_TAG_s263:
    case CODEC_TAG_F263:
      // h263
      dec_type = VIDEO_DEC_FORMAT_H263;
      break;
    case CODEC_TAG_AVC1:
    case CODEC_TAG_avc1:
    case CODEC_TAG_H264:
    case CODEC_TAG_h264:
    case AV_CODEC_ID_H264:
      // h264
      dec_type = VIDEO_DEC_FORMAT_H264;
      break;
    /*
    case AV_CODEC_ID_H264MVC:
      dec_type = VIDEO_DEC_FORMAT_H264;
      break;
    */
    case AV_CODEC_ID_RV30:
    case CODEC_TAG_RV30:
      // realmedia 3
      dec_type = VIDEO_DEC_FORMAT_REAL_8;
      break;
    case AV_CODEC_ID_RV40:
    case CODEC_TAG_RV40:
      // realmedia 4
      dec_type = VIDEO_DEC_FORMAT_REAL_9;
      break;
    case CODEC_TAG_WMV3:
      // wmv3
      dec_type = VIDEO_DEC_FORMAT_WMV3;
      break;
    case AV_CODEC_ID_VC1:
    case CODEC_TAG_VC_1:
    case CODEC_TAG_WVC1:
    case CODEC_TAG_WMVA:
      // vc1
      dec_type = VIDEO_DEC_FORMAT_WVC1;
      break;
    case AV_CODEC_ID_VP6F:
      // vp6
      dec_type = VIDEO_DEC_FORMAT_SW;
      break;
    case AV_CODEC_ID_CAVS:
    case AV_CODEC_ID_AVS:
      // avs
      dec_type = VIDEO_DEC_FORMAT_AVS;
      break;
    case AV_CODEC_ID_HEVC:
      // h265
      dec_type = VIDEO_DEC_FORMAT_HEVC;
      break;
    default:
      dec_type = VIDEO_DEC_FORMAT_UNKNOW;
      break;
  }

  Log(LOGDEBUG, "codec_tag_to_vdec_type, codec_tag(%d) -> vdec_type(%d)", codec_tag, dec_type);
  return dec_type;
}
static void func_destroy(IJKFF_Pipenode *node)
{
    if (!node || !node->opaque)
        return;

    //IJKAM_Pipenode_Opaque *opaque = node->opaque;
}


static int func_flush(IJKFF_Pipenode *node)
{
    return 0;
}

IJKFF_Pipenode *ffpipenode_create_video_decoder_from_android_amcodec(FFPlayer *ffp)
{
    if (!ffp || !ffp->is)
        return NULL;

    IJKFF_Pipenode *node = ffpipenode_alloc(sizeof(IJKAM_Pipenode_Opaque));
    if (!node)
        return node;

    VideoState            *is     = ffp->is;

    IJKAM_Pipenode_Opaque *opaque = node->opaque;
    node->func_destroy  = func_destroy;
    node->func_run_sync = func_run_sync;
    node->func_flush    = func_flush;

    //opaque->pipeline    = pipeline;
    opaque->ffp         = ffp;
    opaque->decoder     = &is->viddec;
    //opaque->weak_vout   = vout;

    AVCodecContext * avctx = opaque->decoder->avctx;

    Log(LOGDEBUG, "codec initializing.");

    loadLibrary();

    opaque->stream_type = AM_STREAM_ES;

    opaque->video_codec_id = avctx->codec_id; //AV_CODEC_ID_H264;
    opaque->video_codec_tag = avctx->codec_tag; //CODEC_TAG_AVC1;

    opaque->video_format = codecid_to_vformat(avctx->codec_id); //VFORMAT_H264
    if(opaque->video_format == VFORMAT_UNSUPPORT)
      return NULL;

    opaque->video_codec_type = codec_tag_to_vdec_type(avctx->codec_tag);  //VIDEO_DEC_FORMAT_H264;
    //if(opaque->video_codec_type == VIDEO_DEC_FORMAT_UNKNOW)
    //  return NULL;

    opaque->extrasize = avctx->extradata_size;
    opaque->extradata = (uint8_t*)malloc(opaque->extrasize);
    memcpy(opaque->extradata, avctx->extradata, opaque->extrasize);
    opaque->video_width = avctx->width;
    opaque->video_height = avctx->height;

    codec_para_t * vcodec = &opaque->vcodec;
    memset(vcodec, 0, sizeof(codec_para_t));

    vcodec->handle = -1;        ///< codec device handler
    vcodec->cntl_handle = -1;   ///< video control device handler
    vcodec->sub_handle = -1;    ///< subtile device handler
    vcodec->stream_type = STREAM_TYPE_ES_VIDEO;  ///< stream type(es, ps, rm, ts)
    vcodec->has_video = 1;   ///< stream has video(1) or not(0)
    vcodec->has_audio = 0;   ///< stream has audio(1) or not(0)
    vcodec->has_sub = 0;     ///< stream has subtitle(1) or not(0)
    vcodec->noblock = 0;     ///< codec device is NONBLOCK(1) or not(0)
    vcodec->audio_type = 0;             ///< stream audio type(PCM, WMA...)
    vcodec->sub_type = 0;               ///< stream subtitle type(TXT, SSA...)
    vcodec->video_pid = -1;              ///< stream video pid
    vcodec->audio_pid = 0;              ///< stream audio pid
    vcodec->sub_pid = 0;                ///< stream subtitle pid
    vcodec->audio_channels = 0;         ///< stream audio channel number
    vcodec->audio_samplerate = 0;       ///< steram audio sample rate
    vcodec->vbuf_size = 0;              ///< video buffer size of codec device
    vcodec->abuf_size= 0;              ///< audio buffer size of codec device
    vcodec->am_sysinfo.width = avctx->width;         // am_sysinfo, unsigned int width
    vcodec->am_sysinfo.height = avctx->height;        // am_sysinfo, unsigned int height
    vcodec->am_sysinfo.rate = UNIT_FREQ / 30;          // am_sysinfo, unsigned int rate
    vcodec->am_sysinfo.extra = 0;         // am_sysinfo, unsigned int extra
    vcodec->am_sysinfo.status = 0;        // am_sysinfo, unsigned int status
    vcodec->am_sysinfo.param = NULL; // am_sysinfo, unsigned int param
    vcodec->am_sysinfo.ratio = 0x00010001;         // am_sysinfo, unsigned int ratio
    vcodec->am_sysinfo.ratio64 = 0x0000000100000001ULL;         // am_sysinfo, unsigned int ratio


    vcodec->video_type = opaque->video_format;             //VFORMAT_H264 ///< stream video type(H264, VC1...)
    vcodec->am_sysinfo.format = opaque->video_codec_type; //VIDEO_DEC_FORMAT_H264;   ///< system information for video
    vcodec->am_sysinfo.param = (void *)(EXTERNAL_PTS | SYNC_OUTSIDE);

    int ret = am_codec_init(vcodec);
    if (ret != CODEC_ERROR_NONE)
    {
      ALOGD("CAMLCodec::OpenDecoder codec init failed, ret=0x%x", -ret);
      return false;
    }

    // make sure we are not stuck in pause (amcodec bug)
    ret = am_codec_resume(vcodec);
    ret = am_codec_set_cntl_mode(vcodec, TRICKMODE_NONE);

    ret = am_codec_set_cntl_avthresh(vcodec, AV_SYNC_THRESH);
    ret = am_codec_set_cntl_syncthresh(vcodec, 0);

    am_packet_init(&opaque->am_pkt);

    opaque->am_pkt.codec = vcodec;
    ret = pre_header_feeding(opaque, &opaque->am_pkt);

    sysfs_setint("/sys/class/video/screen_mode", 1);
    sysfs_setint("/sys/class/video/disable_video", 0);

    return node;
}


static int feed_input_buffer(JNIEnv *env, IJKFF_Pipenode *node, int *enqueue_count)
{
    IJKAM_Pipenode_Opaque *opaque   = node->opaque;
    FFPlayer              *ffp      = opaque->ffp;
    //IJKFF_Pipeline        *pipeline = opaque->ffp->pipeline;
    VideoState            *is       = ffp->is;
    Decoder               *d        = &is->viddec;
    //PacketQueue           *q        = d->queue;
    int                    ret      = 0;
    //int64_t  time_stamp         = 0;
    AVCodecContext * avctx = opaque->decoder->avctx;
    am_packet_t * am_pkt = &opaque->am_pkt;

    int nal_size = 0; //TODO: we need to get this from somewhere...

    if (enqueue_count)
        *enqueue_count = 0;

    if (d->queue->abort_request) {
        ret = 0;
        goto fail;
    }

    if (!d->packet_pending || d->queue->serial != d->pkt_serial) {
        H264ConvertState convert_state = {0, 0};
        AVPacket pkt;
        do {
            if (d->queue->nb_packets == 0)
                SDL_CondSignal(d->empty_queue_cond);
            if (ffp_packet_queue_get_or_buffering(ffp, d->queue, &pkt, &d->pkt_serial, &d->finished) < 0) {
                ret = -1;
                goto fail;
            }
            if (ffp_is_flush_packet(&pkt)) {
                // request flush before lock, or never get mutex
                d->finished = 0;
                d->next_pts = d->start_pts;
                d->next_pts_tb = d->start_pts_tb;
            }
        } while (ffp_is_flush_packet(&pkt) || d->queue->serial != d->pkt_serial);
        av_free_packet(&d->pkt);
        d->pkt_temp = d->pkt = pkt;
        d->packet_pending = 1;

        if (nal_size > 0 && (avctx->codec_id == AV_CODEC_ID_H264 || avctx->codec_id == AV_CODEC_ID_HEVC)) {
            convert_h264_to_annexb(d->pkt_temp.data, d->pkt_temp.size, nal_size, &convert_state);
/*            int64_t time_stamp = d->pkt_temp.pts;
            if (!time_stamp && d->pkt_temp.dts)
                time_stamp = d->pkt_temp.dts;
            if (time_stamp > 0) {
                time_stamp = av_rescale_q(time_stamp, is->video_st->time_base, AV_TIME_BASE_Q);
            } else {
                time_stamp = 0;
            }*/
        }

        int64_t time_stamp = d->pkt_temp.pts;

        time_stamp = av_rescale_q(time_stamp, is->video_st->time_base, AV_TIME_BASE_Q);

        am_pkt->data = d->pkt_temp.data;
        am_pkt->data_size = d->pkt_temp.size;

        am_pkt->newflag    = 1;
        am_pkt->isvalid    = 1;
        am_pkt->avduration = 0;
        am_pkt->avdts = time_stamp;
        am_pkt->avpts = time_stamp;

        // some formats need header/data tweaks.
        // the actual write occurs once in write_av_packet
        // and is controlled by am_pkt.newflag.
        set_header_info(opaque);

        // loop until we write all into codec, am_pkt.isvalid
        // will get set to zero once everything is consumed.
        // PLAYER_SUCCESS means all is ok, not all bytes were written.
        int loop = 0;
        while (opaque->am_pkt.isvalid && loop < 100)
        {
          // abort on any errors.
          if (write_av_packet(opaque, &opaque->am_pkt) != PLAYER_SUCCESS)
            break;

          if (opaque->am_pkt.isvalid)
            Log(LOGDEBUG, "CAMLCodec::Decode: write_av_packet looping");
          loop++;
        }
        assert(loop < 100);


        d->packet_pending = 0;

        d->pkt_temp.dts =
        d->pkt_temp.pts = AV_NOPTS_VALUE;
    }


fail:
    return ret;
}

static int64_t get_pts_video()
{
  int fd = open("/sys/class/tsync/pts_video", O_RDONLY);
  if (fd >= 0)
  {
    char pts_str[16];
    int size = read(fd, pts_str, sizeof(pts_str));
    close(fd);
    if (size > 0)
    {
      unsigned long pts = strtoul(pts_str, NULL, 16);
      return pts;
    }
  }

  Log(LOGERROR, "get_pts_video: open /tsync/event error");
  return -1;
}


//
// Feed packets to decoder
//
static int enqueue_thread_func(void *arg)
{
    JNIEnv                *env      = NULL;
    IJKFF_Pipenode        *node     = arg;
    IJKAM_Pipenode_Opaque *opaque   = node->opaque;
    FFPlayer              *ffp      = opaque->ffp;
    VideoState            *is       = ffp->is;
    Decoder               *d        = &is->viddec;
    PacketQueue           *q        = d->queue;
    int                    ret      = -1;
    int                    dequeue_count = 0;

    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SetupThreadEnv failed\n", __func__);
        goto fail;
    }

    while (!q->abort_request) {
        ret = feed_input_buffer(env, node, &dequeue_count);
        if (ret != 0) {
            goto fail;
        }
    }

    ret = 0;

fail:
    return ret;
}


static int func_run_sync(IJKFF_Pipenode *node)
{
    JNIEnv                *env      = NULL;
    IJKAM_Pipenode_Opaque *opaque   = node->opaque;
    FFPlayer              *ffp      = opaque->ffp;
    VideoState            *is       = ffp->is;
    Decoder               *d        = &is->viddec;
    PacketQueue           *q        = d->queue;
    int                    ret      = 0;
    int                    dequeue_count = 0;
    /*AVFrame               *frame    = NULL;
    int                    got_frame = 0;
    AVRational             tb         = is->video_st->time_base;
    AVRational             frame_rate = av_guess_frame_rate(is->ic, is->video_st, NULL);
    double                 duration;
    double                 pts;*/

    //if (!opaque->acodec) {
    //    return ffp_video_thread(ffp);
    //}

    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SetupThreadEnv failed\n", __func__);
        return -1;
    }

/*
    opaque->enqueue_thread = SDL_CreateThreadEx(&opaque->_enqueue_thread, enqueue_thread_func, node, "amediacodec_input_thread");
    if (!opaque->enqueue_thread) {
        ALOGE("%s: SDL_CreateThreadEx failed\n", __func__);
        return -1;
    }*/

    while (!q->abort_request) {
        ret = feed_input_buffer(env, node, &dequeue_count);
        if (ret != 0) {
            goto fail;
        }

        struct vdec_status vdec = {0};
        ret = am_codec_get_vdec_state(&opaque->vcodec, &vdec);

        struct buf_status vbuf ={0};
        ret = am_codec_get_vbuf_state(&opaque->vcodec, &vbuf);

        int64_t pts_video = get_pts_video();
        //Log(LOGDEBUG, "pts:%lld", pts_video);
    }

fail:
    return -1;
}


static int write_header(am_private_t *para, am_packet_t *pkt)
{
    int write_bytes = 0, len = 0;

    if (pkt->hdr && pkt->hdr->size > 0) {
        if ((NULL == pkt->codec) || (NULL == pkt->hdr->data)) {
            Log(LOGDEBUG, "[write_header]codec null!");
            return PLAYER_EMPTY_P;
        }
        //some wvc1 es data not need to add header
        if (para->video_format == VFORMAT_VC1 && para->video_codec_type == VIDEO_DEC_FORMAT_WVC1) {
            if ((pkt->data) && (pkt->data_size >= 4)
              && (pkt->data[0] == 0) && (pkt->data[1] == 0)
              && (pkt->data[2] == 1) && (pkt->data[3] == 0xd || pkt->data[3] == 0xf)) {
                return PLAYER_SUCCESS;
            }
        }
        while (1) {
            write_bytes = am_codec_write(pkt->codec, pkt->hdr->data + len, pkt->hdr->size - len);
            if (write_bytes < 0 || write_bytes > (pkt->hdr->size - len)) {
                if (-errno != AVERROR(EAGAIN)) {
                    Log(LOGDEBUG, "ERROR:write header failed!");
                    return PLAYER_WR_FAILED;
                } else {
                    continue;
                }
            } else {
                //dumpfile_write(para, pkt->hdr->data, write_bytes);
                len += write_bytes;
                if (len == pkt->hdr->size) {
                    break;
                }
            }
        }
    }
    return PLAYER_SUCCESS;
}



int write_av_packet(am_private_t *para, am_packet_t *pkt)
{
  //Log(LOGDEBUG, "write_av_packet, pkt->isvalid(%d), pkt->data(%p), pkt->data_size(%d)",
  //  pkt->isvalid, pkt->data, pkt->data_size);

    int write_bytes = 0, len = 0;
    unsigned char *buf;
    int size;

    // do we need to check in pts or write the header ?
    if (pkt->newflag) {
        if (pkt->isvalid) {
            //ret = check_in_pts(para, pkt);
            //if (ret != PLAYER_SUCCESS) {
            //    Log(LOGDEBUG, "check in pts failed");
            //    return PLAYER_WR_FAILED;
            //}
        }
        if (write_header(para, pkt) == PLAYER_WR_FAILED) {
            Log(LOGDEBUG, "[%s]write header failed!", __FUNCTION__);
            return PLAYER_WR_FAILED;
        }
        pkt->newflag = 0;
    }
  
    buf = pkt->data;
    size = pkt->data_size ;
    if (size == 0 && pkt->isvalid) {
        pkt->isvalid = 0;
        pkt->data_size = 0;
    }

    while (size > 0 && pkt->isvalid) {
        write_bytes = am_codec_write(pkt->codec, buf, size);
        if (write_bytes < 0 || write_bytes > size) {
            Log(LOGDEBUG, "write codec data failed, write_bytes(%d), errno(%d), size(%d)", write_bytes, errno, size);
            if (-errno != AVERROR(EAGAIN)) {
                Log(LOGDEBUG, "write codec data failed!");
                return PLAYER_WR_FAILED;
            } else {
                // adjust for any data we already wrote into codec.
                // we sleep a bit then exit as we will get called again
                // with the same pkt because pkt->isvalid has not been cleared.
                pkt->data += len;
                pkt->data_size -= len;
                usleep(RW_WAIT_TIME);
                Log(LOGDEBUG, "usleep(RW_WAIT_TIME), len(%d)", len);
                return PLAYER_SUCCESS;
            }
        } else {
            //dumpfile_write(para, buf, write_bytes);
            // keep track of what we write into codec from this pkt
            // in case we get hit with EAGAIN.
            len += write_bytes;
            if (len == pkt->data_size) {
                pkt->isvalid = 0;
                pkt->data_size = 0;
                break;
            } else if (len < pkt->data_size) {
                buf += write_bytes;
                size -= write_bytes;
            } else {
                // writing more that we should is a failure.
                return PLAYER_WR_FAILED;
            }
        }
    }

    return PLAYER_SUCCESS;
}

/*************************************************************************/
static int m4s2_dx50_mp4v_add_header(unsigned char *buf, int size,  am_packet_t *pkt)
{
    if (size > pkt->hdr->size) {
        free(pkt->hdr->data), pkt->hdr->data = NULL;
        pkt->hdr->size = 0;

        pkt->hdr->data = (char*)malloc(size);
        if (!pkt->hdr->data) {
            Log(LOGDEBUG, "[m4s2_dx50_add_header] NOMEM!");
            return PLAYER_FAILED;
        }
    }

    pkt->hdr->size = size;
    memcpy(pkt->hdr->data, buf, size);

    return PLAYER_SUCCESS;
}

static int m4s2_dx50_mp4v_write_header(am_private_t *para, am_packet_t *pkt)
{
    Log(LOGDEBUG, "m4s2_dx50_mp4v_write_header");
    int ret = m4s2_dx50_mp4v_add_header(para->extradata, para->extrasize, pkt);
    if (ret == PLAYER_SUCCESS) {
        if (1) {
            pkt->codec = &para->vcodec;
        } else {
            Log(LOGDEBUG, "[m4s2_dx50_mp4v_add_header]invalid video codec!");
            return PLAYER_EMPTY_P;
        }
        pkt->newflag = 1;
        ret = write_av_packet(para, pkt);
    }
    return ret;
}

static int mjpeg_data_prefeeding(am_packet_t *pkt)
{
    const unsigned char mjpeg_addon_data[] = {
        0xff, 0xd8, 0xff, 0xc4, 0x01, 0xa2, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
        0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x01, 0x00, 0x03, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x10,
        0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00,
        0x00, 0x01, 0x7d, 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31,
        0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1,
        0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72,
        0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28, 0x29,
        0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64,
        0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
        0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95,
        0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
        0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4,
        0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8,
        0xd9, 0xda, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1,
        0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0x11, 0x00, 0x02, 0x01,
        0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
        0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51,
        0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1,
        0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24,
        0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a,
        0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66,
        0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82,
        0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
        0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,
        0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
        0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
        0xda, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4,
        0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa
    };

    if (pkt->hdr->data) {
        memcpy(pkt->hdr->data, &mjpeg_addon_data, sizeof(mjpeg_addon_data));
        pkt->hdr->size = sizeof(mjpeg_addon_data);
    } else {
        Log(LOGDEBUG, "[mjpeg_data_prefeeding]No enough memory!");
        return PLAYER_FAILED;
    }
    return PLAYER_SUCCESS;
}

static int mjpeg_write_header(am_private_t *para, am_packet_t *pkt)
{
    mjpeg_data_prefeeding(pkt);
    if (1) {
        pkt->codec = &para->vcodec;
    } else {
        Log(LOGDEBUG, "[mjpeg_write_header]invalid codec!");
        return PLAYER_EMPTY_P;
    }
    pkt->newflag = 1;
    write_av_packet(para, pkt);
    return PLAYER_SUCCESS;
}

static int divx3_data_prefeeding(am_packet_t *pkt, unsigned w, unsigned h)
{
    unsigned i = (w << 12) | (h & 0xfff);
    unsigned char divx311_add[10] = {
        0x00, 0x00, 0x00, 0x01,
        0x20, 0x00, 0x00, 0x00,
        0x00, 0x00
    };
    divx311_add[5] = (i >> 16) & 0xff;
    divx311_add[6] = (i >> 8) & 0xff;
    divx311_add[7] = i & 0xff;

    if (pkt->hdr->data) {
        memcpy(pkt->hdr->data, divx311_add, sizeof(divx311_add));
        pkt->hdr->size = sizeof(divx311_add);
    } else {
        Log(LOGDEBUG, "[divx3_data_prefeeding]No enough memory!");
        return PLAYER_FAILED;
    }
    return PLAYER_SUCCESS;
}

static int divx3_write_header(am_private_t *para, am_packet_t *pkt)
{
    Log(LOGDEBUG, "divx3_write_header");
    divx3_data_prefeeding(pkt, para->video_width, para->video_height);
    if (1) {
        pkt->codec = &para->vcodec;
    } else {
        Log(LOGDEBUG, "[divx3_write_header]invalid codec!");
        return PLAYER_EMPTY_P;
    }
    pkt->newflag = 1;
    write_av_packet(para, pkt);
    return PLAYER_SUCCESS;
}

static int h264_add_header(unsigned char *buf, int size, am_packet_t *pkt)
{
    if (size > HDR_BUF_SIZE)
    {
        free(pkt->hdr->data);
        pkt->hdr->data = (char *)malloc(size);
        if (!pkt->hdr->data)
            return PLAYER_NOMEM;
    }

    memcpy(pkt->hdr->data, buf, size);
    pkt->hdr->size = size;
    return PLAYER_SUCCESS;
}

static int h264_write_header(am_private_t *para, am_packet_t *pkt)
{
    // Log(LOGDEBUG, "h264_write_header");
    int ret = h264_add_header(para->extradata, para->extrasize, pkt);
    if (ret == PLAYER_SUCCESS) {
        //if (ctx->vcodec) {
        if (1) {
            pkt->codec = &para->vcodec;
        } else {
            //Log(LOGDEBUG, "[pre_header_feeding]invalid video codec!");
            return PLAYER_EMPTY_P;
        }

        pkt->newflag = 1;
        ret = write_av_packet(para, pkt);
    }
    return ret;
}

static int hevc_add_header(unsigned char *buf, int size,  am_packet_t *pkt)
{
    if (size > HDR_BUF_SIZE)
    {
        free(pkt->hdr->data);
        pkt->hdr->data = (char *)malloc(size);
        if (!pkt->hdr->data)
            return PLAYER_NOMEM;
    }

    memcpy(pkt->hdr->data, buf, size);
    pkt->hdr->size = size;
    return PLAYER_SUCCESS;
}

static int hevc_write_header(am_private_t *para, am_packet_t *pkt)
{
    int ret = -1;

    if (para->extradata) {
      ret = hevc_add_header(para->extradata, para->extrasize, pkt);
    }
    if (ret == PLAYER_SUCCESS) {
      pkt->codec = &para->vcodec;
      pkt->newflag = 1;
      ret = write_av_packet(para, pkt);
    }
    return ret;
}

static int wmv3_write_header(am_private_t *para, am_packet_t *pkt)
{
    Log(LOGDEBUG, "wmv3_write_header");
    unsigned i, check_sum = 0;
    unsigned data_len = para->extrasize + 4;

    pkt->hdr->data[0] = 0;
    pkt->hdr->data[1] = 0;
    pkt->hdr->data[2] = 1;
    pkt->hdr->data[3] = 0x10;

    pkt->hdr->data[4] = 0;
    pkt->hdr->data[5] = (data_len >> 16) & 0xff;
    pkt->hdr->data[6] = 0x88;
    pkt->hdr->data[7] = (data_len >> 8) & 0xff;
    pkt->hdr->data[8] = data_len & 0xff;
    pkt->hdr->data[9] = 0x88;

    pkt->hdr->data[10] = 0xff;
    pkt->hdr->data[11] = 0xff;
    pkt->hdr->data[12] = 0x88;
    pkt->hdr->data[13] = 0xff;
    pkt->hdr->data[14] = 0xff;
    pkt->hdr->data[15] = 0x88;

    for (i = 4 ; i < 16 ; i++) {
        check_sum += pkt->hdr->data[i];
    }

    pkt->hdr->data[16] = (check_sum >> 8) & 0xff;
    pkt->hdr->data[17] =  check_sum & 0xff;
    pkt->hdr->data[18] = 0x88;
    pkt->hdr->data[19] = (check_sum >> 8) & 0xff;
    pkt->hdr->data[20] =  check_sum & 0xff;
    pkt->hdr->data[21] = 0x88;

    pkt->hdr->data[22] = (para->video_width >> 8) & 0xff;
    pkt->hdr->data[23] =  para->video_width & 0xff;
    pkt->hdr->data[24] = (para->video_height >> 8) & 0xff;
    pkt->hdr->data[25] =  para->video_height & 0xff;

    memcpy(pkt->hdr->data + 26, para->extradata, para->extrasize);
    pkt->hdr->size = para->extrasize + 26;
    if (1) {
        pkt->codec = &para->vcodec;
    } else {
        Log(LOGDEBUG, "[wmv3_write_header]invalid codec!");
        return PLAYER_EMPTY_P;
    }
    pkt->newflag = 1;
    return write_av_packet(para, pkt);
}

static int wvc1_write_header(am_private_t *para, am_packet_t *pkt)
{
    Log(LOGDEBUG, "wvc1_write_header");
    memcpy(pkt->hdr->data, para->extradata + 1, para->extrasize - 1);
    pkt->hdr->size = para->extrasize - 1;
    if (1) {
        pkt->codec = &para->vcodec;
    } else {
        Log(LOGDEBUG, "[wvc1_write_header]invalid codec!");
        return PLAYER_EMPTY_P;
    }
    pkt->newflag = 1;
    return write_av_packet(para, pkt);
}

static int mpeg_add_header(am_private_t *para, am_packet_t *pkt)
{
    Log(LOGDEBUG, "mpeg_add_header");
#define STUFF_BYTES_LENGTH     (256)
    int size;
    unsigned char packet_wrapper[] = {
        0x00, 0x00, 0x01, 0xe0,
        0x00, 0x00,                                /* pes packet length */
        0x81, 0xc0, 0x0d,
        0x20, 0x00, 0x00, 0x00, 0x00, /* PTS */
        0x1f, 0xff, 0xff, 0xff, 0xff, /* DTS */
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };

    size = para->extrasize + sizeof(packet_wrapper);
    packet_wrapper[4] = size >> 8 ;
    packet_wrapper[5] = size & 0xff ;
    memcpy(pkt->hdr->data, packet_wrapper, sizeof(packet_wrapper));
    size = sizeof(packet_wrapper);
    //Log(LOGDEBUG, "[mpeg_add_header:%d]wrapper size=%d\n",__LINE__,size);
    memcpy(pkt->hdr->data + size, para->extradata, para->extrasize);
    size += para->extrasize;
    //Log(LOGDEBUG, "[mpeg_add_header:%d]wrapper+seq size=%d\n",__LINE__,size);
    memset(pkt->hdr->data + size, 0xff, STUFF_BYTES_LENGTH);
    size += STUFF_BYTES_LENGTH;
    pkt->hdr->size = size;
    //Log(LOGDEBUG, "[mpeg_add_header:%d]hdr_size=%d\n",__LINE__,size);
    if (1) {
        pkt->codec = &para->vcodec;
    } else {
        Log(LOGDEBUG, "[mpeg_add_header]invalid codec!");
        return PLAYER_EMPTY_P;
    }

    pkt->newflag = 1;
    return write_av_packet(para, pkt);
}

int pre_header_feeding(am_private_t *para, am_packet_t *pkt)
{
    int ret;
    if (para->stream_type == AM_STREAM_ES) {
        if (pkt->hdr == NULL) {
            pkt->hdr = (hdr_buf_t*)malloc(sizeof(hdr_buf_t));
            pkt->hdr->data = (char *)malloc(HDR_BUF_SIZE);
            if (!pkt->hdr->data) {
                //Log(LOGDEBUG, "[pre_header_feeding] NOMEM!");
                return PLAYER_NOMEM;
            }
        }

        if (VFORMAT_H264 == para->video_format || VFORMAT_H264_4K2K == para->video_format) {
            ret = h264_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        } else if ((VFORMAT_MPEG4 == para->video_format) && (VIDEO_DEC_FORMAT_MPEG4_3 == para->video_codec_type)) {
            ret = divx3_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        } else if ((CODEC_TAG_M4S2 == para->video_codec_tag)
                || (CODEC_TAG_DX50 == para->video_codec_tag)
                || (CODEC_TAG_mp4v == para->video_codec_tag)) {
            ret = m4s2_dx50_mp4v_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        /*
        } else if ((AVI_FILE == para->file_type)
                && (VIDEO_DEC_FORMAT_MPEG4_3 != para->vstream_info.video_codec_type)
                && (VFORMAT_H264 != para->vstream_info.video_format)
                && (VFORMAT_VC1 != para->vstream_info.video_format)) {
            ret = avi_write_header(para);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        */
        } else if (CODEC_TAG_WMV3 == para->video_codec_tag) {
            Log(LOGDEBUG, "CODEC_TAG_WMV3 == para->video_codec_tag");
            ret = wmv3_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        } else if ((CODEC_TAG_WVC1 == para->video_codec_tag)
                || (CODEC_TAG_VC_1 == para->video_codec_tag)
                || (CODEC_TAG_WMVA == para->video_codec_tag)) {
            Log(LOGDEBUG, "CODEC_TAG_WVC1 == para->video_codec_tag");
            ret = wvc1_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        /*
        } else if ((MKV_FILE == para->file_type) &&
                  ((VFORMAT_MPEG4 == para->vstream_info.video_format)
                || (VFORMAT_MPEG12 == para->vstream_info.video_format))) {
            ret = mkv_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        */
        } else if (VFORMAT_MJPEG == para->video_format) {
            ret = mjpeg_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        } else if (VFORMAT_HEVC == para->video_format) {
            ret = hevc_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        }

        if (pkt->hdr) {
            if (pkt->hdr->data) {
                free(pkt->hdr->data);
                pkt->hdr->data = NULL;
            }
            free(pkt->hdr);
            pkt->hdr = NULL;
        }
    }
    else if (para->stream_type == AM_STREAM_PS) {
        if (pkt->hdr == NULL) {
            pkt->hdr = (hdr_buf_t*)malloc(sizeof(hdr_buf_t));
            pkt->hdr->data = (char*)malloc(HDR_BUF_SIZE);
            if (!pkt->hdr->data) {
                Log(LOGDEBUG, "[pre_header_feeding] NOMEM!");
                return PLAYER_NOMEM;
            }
        }
        if (( AV_CODEC_ID_MPEG1VIDEO == para->video_codec_id)
          || (AV_CODEC_ID_MPEG2VIDEO == para->video_codec_id)
          || (AV_CODEC_ID_MPEG2VIDEO_XVMC == para->video_codec_id)) {
            ret = mpeg_add_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        }
        if (pkt->hdr) {
            if (pkt->hdr->data) {
                free(pkt->hdr->data);
                pkt->hdr->data = NULL;
            }
            free(pkt->hdr);
            pkt->hdr = NULL;
        }
    }
    return PLAYER_SUCCESS;
}



int divx3_prefix(am_packet_t *pkt)
{
#define DIVX311_CHUNK_HEAD_SIZE 13
    const unsigned char divx311_chunk_prefix[DIVX311_CHUNK_HEAD_SIZE] = {
        0x00, 0x00, 0x00, 0x01, 0xb6, 'D', 'I', 'V', 'X', '3', '.', '1', '1'
    };
    if ((pkt->hdr != NULL) && (pkt->hdr->data != NULL)) {
        free(pkt->hdr->data);
        pkt->hdr->data = NULL;
    }

    if (pkt->hdr == NULL) {
        pkt->hdr = (hdr_buf_t*)malloc(sizeof(hdr_buf_t));
        if (!pkt->hdr) {
            Log(LOGDEBUG, "[divx3_prefix] NOMEM!");
            return PLAYER_FAILED;
        }

        pkt->hdr->data = NULL;
        pkt->hdr->size = 0;
    }

    pkt->hdr->data = (char*)malloc(DIVX311_CHUNK_HEAD_SIZE + 4);
    if (pkt->hdr->data == NULL) {
        Log(LOGDEBUG, "[divx3_prefix] NOMEM!");
        return PLAYER_FAILED;
    }

    memcpy(pkt->hdr->data, divx311_chunk_prefix, DIVX311_CHUNK_HEAD_SIZE);

    pkt->hdr->data[DIVX311_CHUNK_HEAD_SIZE + 0] = (pkt->data_size >> 24) & 0xff;
    pkt->hdr->data[DIVX311_CHUNK_HEAD_SIZE + 1] = (pkt->data_size >> 16) & 0xff;
    pkt->hdr->data[DIVX311_CHUNK_HEAD_SIZE + 2] = (pkt->data_size >>  8) & 0xff;
    pkt->hdr->data[DIVX311_CHUNK_HEAD_SIZE + 3] = pkt->data_size & 0xff;

    pkt->hdr->size = DIVX311_CHUNK_HEAD_SIZE + 4;
    pkt->newflag = 1;

    return PLAYER_SUCCESS;
}


int set_header_info(am_private_t *para)
{
  am_packet_t *pkt = &para->am_pkt;

  //if (pkt->newflag)
  {
    //if (pkt->hdr)
    //  pkt->hdr->size = 0;

    if (para->video_format == VFORMAT_MPEG4)
    {
      if (para->video_codec_type == VIDEO_DEC_FORMAT_MPEG4_3)
      {
        return divx3_prefix(pkt);
      }
      else if (para->video_codec_type == VIDEO_DEC_FORMAT_H263)
      {
        return PLAYER_UNSUPPORT;
      }
    } else if (para->video_format == VFORMAT_VC1) {
        if (para->video_codec_type == VIDEO_DEC_FORMAT_WMV3) {
            unsigned i, check_sum = 0, data_len = 0;

            if ((pkt->hdr != NULL) && (pkt->hdr->data != NULL)) {
                free(pkt->hdr->data);
                pkt->hdr->data = NULL;
            }

            if (pkt->hdr == NULL) {
                pkt->hdr = (hdr_buf_t*)malloc(sizeof(hdr_buf_t));
                if (!pkt->hdr) {
                    return PLAYER_FAILED;
                }

                pkt->hdr->data = NULL;
                pkt->hdr->size = 0;
            }

            if (pkt->avpkt.flags) {
                pkt->hdr->data = (char*)malloc(para->extrasize + 26 + 22);
                if (pkt->hdr->data == NULL) {
                    return PLAYER_FAILED;
                }

                pkt->hdr->data[0] = 0;
                pkt->hdr->data[1] = 0;
                pkt->hdr->data[2] = 1;
                pkt->hdr->data[3] = 0x10;

                data_len = para->extrasize + 4;
                pkt->hdr->data[4] = 0;
                pkt->hdr->data[5] = (data_len >> 16) & 0xff;
                pkt->hdr->data[6] = 0x88;
                pkt->hdr->data[7] = (data_len >> 8) & 0xff;
                pkt->hdr->data[8] =  data_len & 0xff;
                pkt->hdr->data[9] = 0x88;

                pkt->hdr->data[10] = 0xff;
                pkt->hdr->data[11] = 0xff;
                pkt->hdr->data[12] = 0x88;
                pkt->hdr->data[13] = 0xff;
                pkt->hdr->data[14] = 0xff;
                pkt->hdr->data[15] = 0x88;

                for (i = 4 ; i < 16 ; i++) {
                    check_sum += pkt->hdr->data[i];
                }

                pkt->hdr->data[16] = (check_sum >> 8) & 0xff;
                pkt->hdr->data[17] =  check_sum & 0xff;
                pkt->hdr->data[18] = 0x88;
                pkt->hdr->data[19] = (check_sum >> 8) & 0xff;
                pkt->hdr->data[20] =  check_sum & 0xff;
                pkt->hdr->data[21] = 0x88;

                pkt->hdr->data[22] = (para->video_width  >> 8) & 0xff;
                pkt->hdr->data[23] =  para->video_width  & 0xff;
                pkt->hdr->data[24] = (para->video_height >> 8) & 0xff;
                pkt->hdr->data[25] =  para->video_height & 0xff;

                memcpy(pkt->hdr->data + 26, para->extradata, para->extrasize);

                check_sum = 0;
                data_len = para->extrasize + 26;
            } else {
                pkt->hdr->data = (char*)malloc(22);
                if (pkt->hdr->data == NULL) {
                    return PLAYER_FAILED;
                }
            }

            pkt->hdr->data[data_len + 0]  = 0;
            pkt->hdr->data[data_len + 1]  = 0;
            pkt->hdr->data[data_len + 2]  = 1;
            pkt->hdr->data[data_len + 3]  = 0xd;

            pkt->hdr->data[data_len + 4]  = 0;
            pkt->hdr->data[data_len + 5]  = (pkt->data_size >> 16) & 0xff;
            pkt->hdr->data[data_len + 6]  = 0x88;
            pkt->hdr->data[data_len + 7]  = (pkt->data_size >> 8) & 0xff;
            pkt->hdr->data[data_len + 8]  =  pkt->data_size & 0xff;
            pkt->hdr->data[data_len + 9]  = 0x88;

            pkt->hdr->data[data_len + 10] = 0xff;
            pkt->hdr->data[data_len + 11] = 0xff;
            pkt->hdr->data[data_len + 12] = 0x88;
            pkt->hdr->data[data_len + 13] = 0xff;
            pkt->hdr->data[data_len + 14] = 0xff;
            pkt->hdr->data[data_len + 15] = 0x88;

            for (i = data_len + 4 ; i < data_len + 16 ; i++) {
                check_sum += pkt->hdr->data[i];
            }

            pkt->hdr->data[data_len + 16] = (check_sum >> 8) & 0xff;
            pkt->hdr->data[data_len + 17] =  check_sum & 0xff;
            pkt->hdr->data[data_len + 18] = 0x88;
            pkt->hdr->data[data_len + 19] = (check_sum >> 8) & 0xff;
            pkt->hdr->data[data_len + 20] =  check_sum & 0xff;
            pkt->hdr->data[data_len + 21] = 0x88;

            pkt->hdr->size = data_len + 22;
            pkt->newflag = 1;
        } else if (para->video_codec_type == VIDEO_DEC_FORMAT_WVC1) {
            if ((pkt->hdr != NULL) && (pkt->hdr->data != NULL)) {
                free(pkt->hdr->data);
                pkt->hdr->data = NULL;
            }

            if (pkt->hdr == NULL) {
                pkt->hdr = (hdr_buf_t*)malloc(sizeof(hdr_buf_t));
                if (!pkt->hdr) {
                    Log(LOGDEBUG, "[wvc1_prefix] NOMEM!");
                    return PLAYER_FAILED;
                }

                pkt->hdr->data = NULL;
                pkt->hdr->size = 0;
            }

            pkt->hdr->data = (char*)malloc(4);
            if (pkt->hdr->data == NULL) {
                Log(LOGDEBUG, "[wvc1_prefix] NOMEM!");
                return PLAYER_FAILED;
            }

            pkt->hdr->data[0] = 0;
            pkt->hdr->data[1] = 0;
            pkt->hdr->data[2] = 1;
            pkt->hdr->data[3] = 0xd;
            pkt->hdr->size = 4;
            pkt->newflag = 1;
        }
    }
  }
  return PLAYER_SUCCESS;
}

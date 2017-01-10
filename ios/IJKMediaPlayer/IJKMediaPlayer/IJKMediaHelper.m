//
//  IJKMediaHelper.m
//  IJKMediaPlayer
//
//  Created by lsc on 09/01/2017.
//  Copyright © 2017 bilibili. All rights reserved.
//

#import "IJKMediaHelper.h"

#import <libavcodec/avcodec.h>
#import <libavformat/avformat.h>
#import <libavutil/imgutils.h>
#import <libswscale/swscale.h>

@implementation IJKMediaHelper

+ (void)createScreenshotOfVideoAtPath:(NSString*)path atTime:(NSTimeInterval)time size:(CGSize)size completion:(CreateScreenshotCompletionHandler)completion {
    AVFormatContext *pFormatCtx;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    AVFrame         *pFrame;
    AVPacket        *packet;
    int             frameFinished;
    int             ret = 0;
    uint8_t         *buffer;
    int videoStream;
    
    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    
    if (avformat_open_input(&pFormatCtx, [path UTF8String], NULL, NULL) != 0) {
        NSLog(@"IJKMediaHelper::Couldn't open input stream");
        return;
    }
    
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        NSLog(@"IJKMediaHelper::Couldn't find stream information");
        return;
    }
    
    // TODO: SEEK
    
    // Find the first video stream
    videoStream = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    
    if (videoStream == -1) {
        NSLog(@"IJKMediaHelper::Couldn't find a video stream");
        return;
    }
    
    // Find the decoder for the video streams
    pCodec = avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
    if (pCodec == NULL) {
        return;
    }

    // Alloc Codec Context
    pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar);

    // Open Codec
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        return;
    }
    
    pFrame = av_frame_alloc();
    buffer = av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
    av_image_fill_arrays(pFrame->data, pFrame->linesize, buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    
    int i = 0;
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == videoStream) {
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, packet);
        }
        if (ret < 0) {
            NSLog(@"Decode Error");
            return;
        }
        if (frameFinished) {
            UIImage *imaage = [self imageFromeAVFrame:pFrame];
//            [UIImageJPEGRepresentation(imaage, 0.8) writeToFile:@"/Users/lsc/Desktop/test.jpg" atomically:YES];
            i++;
            if (i > 1000) {
                return;
            }
        }
//        av_free_packet(packet);
        av_packet_unref(packet);
    }
    
    free(buffer);
    av_free(pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
}

+ (NSTimeInterval)durationOfVideoAtPath:(NSString *)path {
    AVFormatContext *pFormatCtx;
    
    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    
    if (avformat_open_input(&pFormatCtx, [path UTF8String], NULL, NULL) != 0) {
        NSLog(@"IJKMediaHelper::Couldn't open input stream");
        return 0;
    }
    
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        NSLog(@"IJKMediaHelper::Couldn't find stream information");
        return 0;
    }
    
    NSInteger duration = pFormatCtx->duration * 1.0 / AV_TIME_BASE;
    
    avformat_close_input(&pFormatCtx);
    
    return duration;
}

+ (UIImage *)imageFromeAVFrame:(AVFrame *)frame {
    int width = frame->width;
    int height = frame->height;
    AVPicture picture;
    
    struct SwsContext *imgConvertCtx = sws_getContext(frame->width,
                                                      frame->height,
                                                      AV_PIX_FMT_YUV420P,
                                                      frame->width,
                                                      frame->height,
                                                      AV_PIX_FMT_RGB24,
                                                      SWS_FAST_BILINEAR,
                                                      NULL,
                                                      NULL,
                                                      NULL);
    
    if (!imgConvertCtx) {
        return nil;
    }
    
    
    avpicture_alloc(&picture, AV_PIX_FMT_RGB24, width, height);
    sws_scale(imgConvertCtx,
              frame->data,
              frame->linesize,
              0,
              frame->height,
              picture.data, picture.linesize);
    sws_freeContext(imgConvertCtx);
    
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderMask;
    CFDataRef data = CFDataCreate(kCFAllocatorDefault, picture.data[0], picture.linesize[0] * height);
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGImageRef cgImage = CGImageCreate(width,
                                       height,
                                       8,
                                       24,
                                       picture.linesize[0],
                                       colorSpace,
                                       bitmapInfo,
                                       provider,
                                       NULL,
                                       NO,
                                       kCGRenderingIntentDefault);
    
    UIImage *image = [UIImage imageWithCGImage:cgImage];
    CGImageRelease(cgImage);
    CGColorSpaceRelease(colorSpace);
    CGDataProviderRelease(provider);
    CFRelease(data);
    
    avpicture_free(&picture);
    
    return image;
}

@end

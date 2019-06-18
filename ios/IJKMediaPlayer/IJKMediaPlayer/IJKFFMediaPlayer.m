/*
 * IJKFFMoviePlayerDef.m
 *
 * Copyright (c) 2019 Befovy <befovy@gmail.com>
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

#import "IJKFFMediaPlayer.h"

#import "IJKFFMoviePlayerDef.h"
#import "IJKAudioKit.h"
#import "IJKFFOptions.h"
#import "ijkplayer/ijkplayer.h"


@interface IJKFFWeakHolder : NSObject
@property (nonatomic, weak) id object;
@end

@implementation IJKFFWeakHolder
@end


@implementation IJKFFMediaPlayer {
    IjkMediaPlayer* _nativeMediaPlayer;
    IJKFFMoviePlayerMessagePool *_msgPool;
    
    NSString *_dataSource;
    int _videoWidth;
    int _videoHeight;
    int _videoSarNum;
    int _videoSarDen;
}

- (IJKFFMoviePlayerMessage *) obtainMessage {
    return [_msgPool obtain];
}

inline static IJKFFMediaPlayer *ffplayerRetain(void *arg) {
    return (__bridge_transfer IJKFFMediaPlayer *) arg;
}

int ff_media_player_msg_loop(void* arg)
{
    @autoreleasepool {
        IjkMediaPlayer *mp = (IjkMediaPlayer*)arg;
        __weak IJKFFMediaPlayer *ffPlayer = ffplayerRetain(ijkmp_set_weak_thiz(mp, NULL));
        while (ffPlayer) {
            @autoreleasepool {
                IJKFFMoviePlayerMessage *msg = [ffPlayer obtainMessage];
                if (!msg)
                    break;
                
                int retval = ijkmp_get_msg(mp, &msg->_msg, 1);
                if (retval < 0)
                    break;
                
                // block-get should never return 0
                assert(retval > 0);
                [ffPlayer performSelectorOnMainThread:@selector(postEvent:) withObject:msg waitUntilDone:NO];
            }
        }
        
        // retained in prepare_async, before SDL_CreateThreadEx
        ijkmp_dec_ref_p(&mp);
        return 0;
    }
}

- (IJKFFMediaPlayer *)init
{
    self = [super init];
    if (self) {
        ijkmp_global_init();
        _msgPool = [[IJKFFMoviePlayerMessagePool alloc] init];

        [self nativeSetup];
        
        ijkmp_set_option(_nativeMediaPlayer, IJKMP_OPT_CATEGORY_PLAYER, "overlay-format", "fcc-_es2");
        
        [[IJKAudioKit sharedInstance] setupAudioSession];
    }
    return self;
}

- (void) nativeSetup
{
    _nativeMediaPlayer = ijkmp_ios_create(ff_media_player_msg_loop);
    
    IJKFFWeakHolder *weakHolder = [[IJKFFWeakHolder alloc] init];
    weakHolder.object = self;
    
    ijkmp_set_weak_thiz(_nativeMediaPlayer, (__bridge_retained void *) self);
    ijkmp_set_inject_opaque(_nativeMediaPlayer, (__bridge_retained void *) weakHolder);
    ijkmp_set_ijkio_inject_opaque(_nativeMediaPlayer, (__bridge_retained void *) weakHolder);
}

- (void)postEvent: (IJKFFMoviePlayerMessage *)msg
{
    if (!msg)
        return;
    
    AVMessage *avmsg = &msg->_msg;
    switch (avmsg->what) {
        case FFP_MSG_FLUSH:
            break;
        case FFP_MSG_ERROR:
            break;
        case FFP_MSG_PREPARED:
            break;
            
        default:
            break;
    }
    [_msgPool recycle:msg];
}


- (void) setSurface
{
    
}

- (void) prepareAsync
{
    ijkmp_prepare_async(_nativeMediaPlayer);
}

- (void) setDataSource:(NSString *)url
{
    _dataSource = url;
    ijkmp_set_data_source(_nativeMediaPlayer, [url UTF8String]);
}


- (void) start
{
    ijkmp_start(_nativeMediaPlayer);
}

- (void) stop
{
    ijkmp_stop(_nativeMediaPlayer);
}

- (void) pause
{
    ijkmp_pause(_nativeMediaPlayer);
}


- (BOOL) isPlaying
{
    return ijkmp_is_playing(_nativeMediaPlayer);
}

- (long) getCurrentPosition
{
    return ijkmp_get_current_position(_nativeMediaPlayer);
}

- (long) getDuration
{
    return ijkmp_get_duration(_nativeMediaPlayer);
}

- (void) seekTo:(long) msec
{
    ijkmp_seek_to(_nativeMediaPlayer, msec);
}

- (void) shutdown
{
    ijkmp_shutdown(_nativeMediaPlayer);
    
    __unused id weakPlayer = (__bridge_transfer IJKFFMediaPlayer*)ijkmp_set_weak_thiz(_nativeMediaPlayer, NULL);
    __unused id weakHolder = (__bridge_transfer IJKFFWeakHolder*)ijkmp_set_inject_opaque(_nativeMediaPlayer, NULL);
    __unused id weakijkHolder = (__bridge_transfer IJKFFWeakHolder*)ijkmp_set_ijkio_inject_opaque(_nativeMediaPlayer, NULL);
    ijkmp_dec_ref_p(&_nativeMediaPlayer);
}

- (void) reset
{
    [self shutdown];
    [self nativeSetup];
}


- (void)setOptionValue:(NSString *)value
                forKey:(NSString *)key
            ofCategory:(IJKFFOptionCategory)category
{
    ijkmp_set_option(_nativeMediaPlayer, category, [key UTF8String], [value UTF8String]);
}

- (void)setOptionIntValue:(int64_t)value
                   forKey:(NSString *)key
               ofCategory:(IJKFFOptionCategory)category
{
    ijkmp_set_option_int(_nativeMediaPlayer, category, [key UTF8String], value);
}


@end

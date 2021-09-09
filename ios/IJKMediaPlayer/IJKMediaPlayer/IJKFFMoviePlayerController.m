/*
 * IJKFFMoviePlayerController.m
 *
 * Copyright (c) 2013 Bilibili
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#import "IJKFFMoviePlayerController.h"

#import <UIKit/UIKit.h>
#import "IJKSDLHudViewController.h"
#import "IJKFFMoviePlayerDef.h"
#import "IJKMediaPlayback.h"
#import "IJKMediaModule.h"
#import "IJKAudioKit.h"
#import "IJKNotificationManager.h"
#import "NSString+IJKMedia.h"
#import "ijkioapplication.h"
#include "string.h"

static const char *kIJKFFRequiredFFmpegVersion = "ff4.0--ijk0.8.8--20210426--001";

// It means you didn't call shutdown if you found this object leaked.
@interface IJKWeakHolder : NSObject
@property (nonatomic, weak) id object;
@end

@implementation IJKWeakHolder
@end

@interface IJKFFMoviePlayerController()

@end

@implementation IJKFFMoviePlayerController {
    IjkMediaPlayer *_mediaPlayer;
    IJKSDLGLView *_glView;
    IJKFFMoviePlayerMessagePool *_msgPool;
    NSString *_urlString;

    NSInteger _videoWidth;
    NSInteger _videoHeight;
    NSInteger _sampleAspectRatioNumerator;
    NSInteger _sampleAspectRatioDenominator;

    BOOL      _seeking;
    NSInteger _bufferingTime;
    NSInteger _bufferingPosition;

    BOOL _keepScreenOnWhilePlaying;
    BOOL _pauseInBackground;
    BOOL _isVideoToolboxOpen;
    BOOL _playingBeforeInterruption;

    IJKNotificationManager *_notificationManager;

    AVAppAsyncStatistic _asyncStat;
    IjkIOAppCacheStatistic _cacheStat;
    NSTimer *_hudTimer;
    IJKSDLHudViewController *_hudViewController;
}

@synthesize view = _view;
@synthesize currentPlaybackTime;
@synthesize duration;
@synthesize playableDuration;
@synthesize bufferingProgress = _bufferingProgress;

@synthesize numberOfBytesTransferred = _numberOfBytesTransferred;

@synthesize isPreparedToPlay = _isPreparedToPlay;
@synthesize playbackState = _playbackState;
@synthesize loadState = _loadState;

@synthesize naturalSize = _naturalSize;
@synthesize scalingMode = _scalingMode;
@synthesize shouldAutoplay = _shouldAutoplay;

@synthesize allowsMediaAirPlay = _allowsMediaAirPlay;
@synthesize airPlayMediaActive = _airPlayMediaActive;

@synthesize isDanmakuMediaAirPlay = _isDanmakuMediaAirPlay;

@synthesize monitor = _monitor;
@synthesize shouldShowHudView           = _shouldShowHudView;
@synthesize isSeekBuffering = _isSeekBuffering;
@synthesize isAudioSync = _isAudioSync;
@synthesize isVideoSync = _isVideoSync;

#define FFP_IO_STAT_STEP (50 * 1024)

// as an example
void IJKFFIOStatDebugCallback(const char *url, int type, int bytes)
{
    static int64_t s_ff_io_stat_check_points = 0;
    static int64_t s_ff_io_stat_bytes = 0;
    if (!url)
        return;

    if (type != IJKMP_IO_STAT_READ)
        return;

    if (!av_strstart(url, "http:", NULL))
        return;

    s_ff_io_stat_bytes += bytes;
    if (s_ff_io_stat_bytes < s_ff_io_stat_check_points ||
        s_ff_io_stat_bytes > s_ff_io_stat_check_points + FFP_IO_STAT_STEP) {
        s_ff_io_stat_check_points = s_ff_io_stat_bytes;
        NSLog(@"io-stat: %s, +%d = %"PRId64"\n", url, bytes, s_ff_io_stat_bytes);
    }
}

void IJKFFIOStatRegister(void (*cb)(const char *url, int type, int bytes))
{
    ijkmp_io_stat_register(cb);
}

void IJKFFIOStatCompleteDebugCallback(const char *url,
                                      int64_t read_bytes, int64_t total_size,
                                      int64_t elpased_time, int64_t total_duration)
{
    if (!url)
        return;

    if (!av_strstart(url, "http:", NULL))
        return;

    NSLog(@"io-stat-complete: %s, %"PRId64"/%"PRId64", %"PRId64"/%"PRId64"\n",
          url, read_bytes, total_size, elpased_time, total_duration);
}

void IJKFFIOStatCompleteRegister(void (*cb)(const char *url,
                                            int64_t read_bytes, int64_t total_size,
                                            int64_t elpased_time, int64_t total_duration))
{
    ijkmp_io_stat_complete_register(cb);
}

- (id)initWithContentURL:(NSURL *)aUrl
             withOptions:(IJKFFOptions *)options
{
    if (aUrl == nil)
        return nil;

    // Detect if URL is file path and return proper string for it
    NSString *aUrlString = [aUrl isFileURL] ? [aUrl path] : [aUrl absoluteString];

    return [self initWithContentURLString:aUrlString
                              withOptions:options];
}

- (void)setScreenOn: (BOOL)on
{
    [IJKMediaModule sharedModule].mediaModuleIdleTimerDisabled = on;
    // [UIApplication sharedApplication].idleTimerDisabled = on;
}

- (id)initWithContentURLString:(NSString *)aUrlString
                   withOptions:(IJKFFOptions *)options
{
    if (aUrlString == nil)
        return nil;

    self = [super init];
    if (self) {
        ijkmp_global_init();
        ijkmp_global_set_inject_callback(ijkff_inject_callback);

        [IJKFFMoviePlayerController checkIfFFmpegVersionMatch:NO];

        if (options == nil)
            options = [IJKFFOptions optionsByDefault];

        // IJKFFIOStatRegister(IJKFFIOStatDebugCallback);
        // IJKFFIOStatCompleteRegister(IJKFFIOStatCompleteDebugCallback);

        // init fields
        _scalingMode = IJKMPMovieScalingModeAspectFit;
        _shouldAutoplay = YES;
        memset(&_asyncStat, 0, sizeof(_asyncStat));
        memset(&_cacheStat, 0, sizeof(_cacheStat));
        _monitor = [[IJKFFMonitor alloc] init];

        // init media resource
        _urlString = aUrlString;

        // init player
        _mediaPlayer = ijkmp_ios_create(media_player_msg_loop);
        _msgPool = [[IJKFFMoviePlayerMessagePool alloc] init];
        IJKWeakHolder *weakHolder = [IJKWeakHolder new];
        weakHolder.object = self;

        ijkmp_set_weak_thiz(_mediaPlayer, (__bridge_retained void *) self);
        ijkmp_set_inject_opaque(_mediaPlayer, (__bridge_retained void *) weakHolder);
        ijkmp_set_ijkio_inject_opaque(_mediaPlayer, (__bridge_retained void *)weakHolder);
        ijkmp_set_option_int(_mediaPlayer, IJKMP_OPT_CATEGORY_PLAYER, "start-on-prepared", _shouldAutoplay ? 1 : 0);

        // init video sink
        _glView = [[IJKSDLGLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
        _glView.isThirdGLView = NO;
        _view = _glView;
        _hudViewController = [[IJKSDLHudViewController alloc] init];
        [_hudViewController setRect:_glView.frame];
        _shouldShowHudView = NO;
        _hudViewController.tableView.hidden = YES;
        [_view addSubview:_hudViewController.tableView];

        [self setHudValue:nil forKey:@"scheme"];
        [self setHudValue:nil forKey:@"host"];
        [self setHudValue:nil forKey:@"path"];
        [self setHudValue:nil forKey:@"ip"];
        [self setHudValue:nil forKey:@"tcp-info"];
        [self setHudValue:nil forKey:@"http"];
        [self setHudValue:nil forKey:@"tcp-spd"];
        [self setHudValue:nil forKey:@"t-prepared"];
        [self setHudValue:nil forKey:@"t-render"];
        [self setHudValue:nil forKey:@"t-preroll"];
        [self setHudValue:nil forKey:@"t-http-open"];
        [self setHudValue:nil forKey:@"t-http-seek"];
        
        self.shouldShowHudView = options.showHudView;

        ijkmp_ios_set_glview(_mediaPlayer, _glView);
        ijkmp_set_option(_mediaPlayer, IJKMP_OPT_CATEGORY_PLAYER, "overlay-format", "fcc-_es2");
#ifdef DEBUG
        [IJKFFMoviePlayerController setLogLevel:k_IJK_LOG_DEBUG];
#else
        [IJKFFMoviePlayerController setLogLevel:k_IJK_LOG_SILENT];
#endif
        // init audio sink
        [[IJKAudioKit sharedInstance] setupAudioSession];

        [options applyTo:_mediaPlayer];
        _pauseInBackground = NO;

        // init extra
        _keepScreenOnWhilePlaying = YES;
        [self setScreenOn:YES];

        _notificationManager = [[IJKNotificationManager alloc] init];
        [self registerApplicationObservers];
    }
    return self;
}

- (id)initWithMoreContent:(NSURL *)aUrl
              withOptions:(IJKFFOptions *)options
               withGLView:(UIView<IJKSDLGLViewProtocol> *)glView
{
    if (aUrl == nil)
        return nil;

    // Detect if URL is file path and return proper string for it
    NSString *aUrlString = [aUrl isFileURL] ? [aUrl path] : [aUrl absoluteString];

    return [self initWithMoreContentString:aUrlString
                              withOptions:options
                               withGLView:glView];
}

- (id)initWithMoreContentString:(NSString *)aUrlString
                 withOptions:(IJKFFOptions *)options
                  withGLView:(UIView <IJKSDLGLViewProtocol> *)glView
{
    if (aUrlString == nil || glView == nil)
        return nil;

    self = [super init];
    if (self) {
        ijkmp_global_init();
        ijkmp_global_set_inject_callback(ijkff_inject_callback);

        [IJKFFMoviePlayerController checkIfFFmpegVersionMatch:NO];

        if (options == nil)
            options = [IJKFFOptions optionsByDefault];

        // IJKFFIOStatRegister(IJKFFIOStatDebugCallback);
        // IJKFFIOStatCompleteRegister(IJKFFIOStatCompleteDebugCallback);

        // init fields
        _scalingMode = IJKMPMovieScalingModeAspectFit;
        _shouldAutoplay = YES;
        memset(&_asyncStat, 0, sizeof(_asyncStat));
        memset(&_cacheStat, 0, sizeof(_cacheStat));
        _monitor = [[IJKFFMonitor alloc] init];

        // init media resource
        _urlString = aUrlString;

        // init player
        _mediaPlayer = ijkmp_ios_create(media_player_msg_loop);
        _msgPool = [[IJKFFMoviePlayerMessagePool alloc] init];
        IJKWeakHolder *weakHolder = [IJKWeakHolder new];
        weakHolder.object = self;

        ijkmp_set_weak_thiz(_mediaPlayer, (__bridge_retained void *) self);
        ijkmp_set_inject_opaque(_mediaPlayer, (__bridge_retained void *) weakHolder);
        ijkmp_set_ijkio_inject_opaque(_mediaPlayer, (__bridge_retained void *)weakHolder);
        ijkmp_set_option_int(_mediaPlayer, IJKMP_OPT_CATEGORY_PLAYER, "start-on-prepared", _shouldAutoplay ? 1 : 0);

        self.shouldShowHudView = options.showHudView;
        glView.isThirdGLView = YES;
        _view = _glView = (IJKSDLGLView *)glView;
        _hudViewController = [[IJKSDLHudViewController alloc] init];
        [_hudViewController setRect:_glView.frame];
        _shouldShowHudView = NO;
        _hudViewController.tableView.hidden = YES;
        [_view addSubview:_hudViewController.tableView];

        [self setHudValue:nil forKey:@"scheme"];
        [self setHudValue:nil forKey:@"host"];
        [self setHudValue:nil forKey:@"path"];
        [self setHudValue:nil forKey:@"ip"];
        [self setHudValue:nil forKey:@"tcp-info"];
        [self setHudValue:nil forKey:@"http"];
        [self setHudValue:nil forKey:@"tcp-spd"];
        [self setHudValue:nil forKey:@"t-prepared"];
        [self setHudValue:nil forKey:@"t-render"];
        [self setHudValue:nil forKey:@"t-preroll"];
        [self setHudValue:nil forKey:@"t-http-open"];
        [self setHudValue:nil forKey:@"t-http-seek"];
        self.shouldShowHudView = options.showHudView;

        ijkmp_ios_set_glview(_mediaPlayer, _glView);

        ijkmp_set_option(_mediaPlayer, IJKMP_OPT_CATEGORY_PLAYER, "overlay-format", "fcc-_es2");
#ifdef DEBUG
        [IJKFFMoviePlayerController setLogLevel:k_IJK_LOG_DEBUG];
#else
        [IJKFFMoviePlayerController setLogLevel:k_IJK_LOG_SILENT];
#endif
        // init audio sink
        [[IJKAudioKit sharedInstance] setupAudioSession];

        [options applyTo:_mediaPlayer];
        _pauseInBackground = NO;

        // init extra
        _keepScreenOnWhilePlaying = YES;
        [self setScreenOn:YES];

        _notificationManager = [[IJKNotificationManager alloc] init];
        [self registerApplicationObservers];
    }
    return self;
}

- (void)dealloc
{
//    [self unregisterApplicationObservers];
}

- (void)setShouldAutoplay:(BOOL)shouldAutoplay
{
    _shouldAutoplay = shouldAutoplay;

    if (!_mediaPlayer)
        return;

    ijkmp_set_option_int(_mediaPlayer, IJKMP_OPT_CATEGORY_PLAYER, "start-on-prepared", _shouldAutoplay ? 1 : 0);
}

- (BOOL)shouldAutoplay
{
    return _shouldAutoplay;
}

- (void)prepareToPlay
{
    if (!_mediaPlayer)
        return;

    [self setScreenOn:_keepScreenOnWhilePlaying];

    ijkmp_set_data_source(_mediaPlayer, [_urlString UTF8String]);
    ijkmp_set_option(_mediaPlayer, IJKMP_OPT_CATEGORY_FORMAT, "safe", "0"); // for concat demuxer

    _monitor.prepareStartTick = (int64_t)SDL_GetTickHR();
    ijkmp_prepare_async(_mediaPlayer);
}

- (void)setHudUrl:(NSString *)urlString
{
    if ([[NSThread currentThread] isMainThread]) {
        NSURL *url = [NSURL URLWithString:urlString];
        [self setHudValue:url.scheme forKey:@"scheme"];
        [self setHudValue:url.host   forKey:@"host"];
        [self setHudValue:url.path   forKey:@"path"];
    } else {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self setHudUrl:urlString];
        });
    }
}

- (void)play
{
    if (!_mediaPlayer)
        return;

    [self setScreenOn:_keepScreenOnWhilePlaying];

    [self startHudTimer];
    ijkmp_start(_mediaPlayer);
}

- (void)pause
{
    if (!_mediaPlayer)
        return;

//    [self stopHudTimer];
    ijkmp_pause(_mediaPlayer);
}

- (void)stop
{
    if (!_mediaPlayer)
        return;

    [self setScreenOn:NO];

    [self stopHudTimer];
    ijkmp_stop(_mediaPlayer);
}

- (BOOL)isPlaying
{
    if (!_mediaPlayer)
        return NO;

    return ijkmp_is_playing(_mediaPlayer);
}

- (void)setPauseInBackground:(BOOL)pause
{
    _pauseInBackground = pause;
}

- (BOOL)isVideoToolboxOpen
{
    if (!_mediaPlayer)
        return NO;

    return _isVideoToolboxOpen;
}

inline static int getPlayerOption(IJKFFOptionCategory category)
{
    int mp_category = -1;
    switch (category) {
        case kIJKFFOptionCategoryFormat:
            mp_category = IJKMP_OPT_CATEGORY_FORMAT;
            break;
        case kIJKFFOptionCategoryCodec:
            mp_category = IJKMP_OPT_CATEGORY_CODEC;
            break;
        case kIJKFFOptionCategorySws:
            mp_category = IJKMP_OPT_CATEGORY_SWS;
            break;
        case kIJKFFOptionCategoryPlayer:
            mp_category = IJKMP_OPT_CATEGORY_PLAYER;
            break;
        default:
            NSLog(@"unknown option category: %d\n", category);
    }
    return mp_category;
}

- (void)setOptionValue:(NSString *)value
                forKey:(NSString *)key
            ofCategory:(IJKFFOptionCategory)category
{
    assert(_mediaPlayer);
    if (!_mediaPlayer)
        return;

    ijkmp_set_option(_mediaPlayer, getPlayerOption(category), [key UTF8String], [value UTF8String]);
}

- (void)setOptionIntValue:(int64_t)value
                   forKey:(NSString *)key
               ofCategory:(IJKFFOptionCategory)category
{
    assert(_mediaPlayer);
    if (!_mediaPlayer)
        return;

    ijkmp_set_option_int(_mediaPlayer, getPlayerOption(category), [key UTF8String], value);
}

+ (void)setLogReport:(BOOL)preferLogReport
{
    ijkmp_global_set_log_report(preferLogReport ? 1 : 0);
}

+ (void)setLogLevel:(IJKLogLevel)logLevel
{
    ijkmp_global_set_log_level(logLevel);
}

+ (BOOL)checkIfFFmpegVersionMatch:(BOOL)showAlert;
{
    const char *actualVersion = av_version_info();
    const char *expectVersion = kIJKFFRequiredFFmpegVersion;
    if (0 == strcmp(actualVersion, expectVersion)) {
        return YES;
    } else {
        NSString *message = [NSString stringWithFormat:@"actual: %s\n expect: %s\n", actualVersion, expectVersion];
        NSLog(@"\n!!!!!!!!!!\n%@\n!!!!!!!!!!\n", message);
        if (showAlert) {
            UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"Unexpected FFmpeg version"
                                                                message:message
                                                               delegate:nil
                                                      cancelButtonTitle:@"OK"
                                                      otherButtonTitles:nil];
            [alertView show];
        }
        return NO;
    }
}

+ (BOOL)checkIfPlayerVersionMatch:(BOOL)showAlert
                          version:(NSString *)version
{
    const char *actualVersion = ijkmp_version();
    const char *expectVersion = version.UTF8String;
    if (0 == strcmp(actualVersion, expectVersion)) {
        return YES;
    } else {
        if (showAlert) {
            NSString *message = [NSString stringWithFormat:@"actual: %s\n expect: %s\n",
                                 actualVersion, expectVersion];
            UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"Unexpected ijkplayer version"
                                                                message:message
                                                               delegate:nil
                                                      cancelButtonTitle:@"OK"
                                                      otherButtonTitles:nil];
            [alertView show];
        }
        return NO;
    }
}

- (void)shutdown
{
    if (!_mediaPlayer)
        return;

    [self stopHudTimer];
    [self unregisterApplicationObservers];
    [self setScreenOn:NO];

    [self performSelectorInBackground:@selector(shutdownWaitStop:) withObject:self];
}

- (void)shutdownWaitStop:(IJKFFMoviePlayerController *) mySelf
{
    if (!_mediaPlayer)
        return;

    ijkmp_stop(_mediaPlayer);
    ijkmp_shutdown(_mediaPlayer);

    [self performSelectorOnMainThread:@selector(shutdownClose:) withObject:self waitUntilDone:YES];
}

- (void)shutdownClose:(IJKFFMoviePlayerController *) mySelf
{
    if (!_mediaPlayer)
        return;

    _segmentOpenDelegate    = nil;
    _tcpOpenDelegate        = nil;
    _httpOpenDelegate       = nil;
    _liveOpenDelegate       = nil;
    _nativeInvokeDelegate   = nil;

    __unused id weakPlayer = (__bridge_transfer IJKFFMoviePlayerController*)ijkmp_set_weak_thiz(_mediaPlayer, NULL);
    __unused id weakHolder = (__bridge_transfer IJKWeakHolder*)ijkmp_set_inject_opaque(_mediaPlayer, NULL);
    __unused id weakijkHolder = (__bridge_transfer IJKWeakHolder*)ijkmp_set_ijkio_inject_opaque(_mediaPlayer, NULL);
    ijkmp_dec_ref_p(&_mediaPlayer);

    [self didShutdown];
}

- (void)didShutdown
{
}

- (IJKMPMoviePlaybackState)playbackState
{
    if (!_mediaPlayer)
        return NO;

    IJKMPMoviePlaybackState mpState = IJKMPMoviePlaybackStateStopped;
    int state = ijkmp_get_state(_mediaPlayer);
    switch (state) {
        case MP_STATE_STOPPED:
        case MP_STATE_COMPLETED:
        case MP_STATE_ERROR:
        case MP_STATE_END:
            mpState = IJKMPMoviePlaybackStateStopped;
            break;
        case MP_STATE_IDLE:
        case MP_STATE_INITIALIZED:
        case MP_STATE_ASYNC_PREPARING:
        case MP_STATE_PAUSED:
            mpState = IJKMPMoviePlaybackStatePaused;
            break;
        case MP_STATE_PREPARED:
        case MP_STATE_STARTED: {
            if (_seeking)
                mpState = IJKMPMoviePlaybackStateSeekingForward;
            else
                mpState = IJKMPMoviePlaybackStatePlaying;
            break;
        }
    }
    // IJKMPMoviePlaybackStatePlaying,
    // IJKMPMoviePlaybackStatePaused,
    // IJKMPMoviePlaybackStateStopped,
    // IJKMPMoviePlaybackStateInterrupted,
    // IJKMPMoviePlaybackStateSeekingForward,
    // IJKMPMoviePlaybackStateSeekingBackward
    return mpState;
}

- (void)setCurrentPlaybackTime:(NSTimeInterval)aCurrentPlaybackTime
{
    if (!_mediaPlayer)
        return;

    _seeking = YES;
    [[NSNotificationCenter defaultCenter]
     postNotificationName:IJKMPMoviePlayerPlaybackStateDidChangeNotification
     object:self];

    _bufferingPosition = 0;
    ijkmp_seek_to(_mediaPlayer, aCurrentPlaybackTime * 1000);
}

- (NSTimeInterval)currentPlaybackTime
{
    if (!_mediaPlayer)
        return 0.0f;

    NSTimeInterval ret = ijkmp_get_current_position(_mediaPlayer);
    if (isnan(ret) || isinf(ret))
        return -1;

    return ret / 1000;
}

- (NSTimeInterval)duration
{
    if (!_mediaPlayer)
        return 0.0f;

    NSTimeInterval ret = ijkmp_get_duration(_mediaPlayer);
    if (isnan(ret) || isinf(ret))
        return -1;

    return ret / 1000;
}

- (NSTimeInterval)playableDuration
{
    if (!_mediaPlayer)
        return 0.0f;

    NSTimeInterval demux_cache = ((NSTimeInterval)ijkmp_get_playable_duration(_mediaPlayer)) / 1000;

    int64_t buf_forwards = _asyncStat.buf_forwards;
    int64_t bit_rate = ijkmp_get_property_int64(_mediaPlayer, FFP_PROP_INT64_BIT_RATE, 0);

    if (buf_forwards > 0 && bit_rate > 0) {
        NSTimeInterval io_cache = ((float)buf_forwards) * 8 / bit_rate;
        demux_cache += io_cache;
    }

    return demux_cache;
}

- (CGSize)naturalSize
{
    return _naturalSize;
}

- (void)changeNaturalSize
{
    [self willChangeValueForKey:@"naturalSize"];
    if (_sampleAspectRatioNumerator > 0 && _sampleAspectRatioDenominator > 0) {
        self->_naturalSize = CGSizeMake(1.0f * _videoWidth * _sampleAspectRatioNumerator / _sampleAspectRatioDenominator, _videoHeight);
    } else {
        self->_naturalSize = CGSizeMake(_videoWidth, _videoHeight);
    }
    [self didChangeValueForKey:@"naturalSize"];

    if (self->_naturalSize.width > 0 && self->_naturalSize.height > 0) {
        [[NSNotificationCenter defaultCenter]
         postNotificationName:IJKMPMovieNaturalSizeAvailableNotification
         object:self];
    }
}

- (void)setScalingMode: (IJKMPMovieScalingMode) aScalingMode
{
    IJKMPMovieScalingMode newScalingMode = aScalingMode;
    switch (aScalingMode) {
        case IJKMPMovieScalingModeNone:
            [_view setContentMode:UIViewContentModeCenter];
            break;
        case IJKMPMovieScalingModeAspectFit:
            [_view setContentMode:UIViewContentModeScaleAspectFit];
            break;
        case IJKMPMovieScalingModeAspectFill:
            [_view setContentMode:UIViewContentModeScaleAspectFill];
            break;
        case IJKMPMovieScalingModeFill:
            [_view setContentMode:UIViewContentModeScaleToFill];
            break;
        default:
            newScalingMode = _scalingMode;
    }

    _scalingMode = newScalingMode;
}

// deprecated, for MPMoviePlayerController compatiable
- (UIImage *)thumbnailImageAtTime:(NSTimeInterval)playbackTime timeOption:(IJKMPMovieTimeOption)option
{
    return nil;
}

- (UIImage *)thumbnailImageAtCurrentTime
{
    if ([_view conformsToProtocol:@protocol(IJKSDLGLViewProtocol)]) {
        id<IJKSDLGLViewProtocol> glView = (id<IJKSDLGLViewProtocol>)_view;
        return [glView snapshot];
    }

    return nil;
}

- (CGFloat)fpsAtOutput
{
    return _glView.fps;
}

inline static NSString *formatedDurationMilli(int64_t duration) {
    if (duration >=  1000) {
        return [NSString stringWithFormat:@"%.2f sec", ((float)duration) / 1000];
    } else {
        return [NSString stringWithFormat:@"%ld msec", (long)duration];
    }
}

inline static NSString *formatedDurationBytesAndBitrate(int64_t bytes, int64_t bitRate) {
    if (bitRate <= 0) {
        return @"inf";
    }
    return formatedDurationMilli(((float)bytes) * 8 * 1000 / bitRate);
}

inline static NSString *formatedSize(int64_t bytes) {
    if (bytes >= 100 * 1024) {
        return [NSString stringWithFormat:@"%.2f MB", ((float)bytes) / 1000 / 1024];
    } else if (bytes >= 100) {
        return [NSString stringWithFormat:@"%.1f KB", ((float)bytes) / 1000];
    } else {
        return [NSString stringWithFormat:@"%ld B", (long)bytes];
    }
}

inline static NSString *formatedSpeed(int64_t bytes, int64_t elapsed_milli) {
    if (elapsed_milli <= 0) {
        return @"N/A";
    }

    if (bytes <= 0) {
        return @"0";
    }

    float bytes_per_sec = ((float)bytes) * 1000.f /  elapsed_milli;
    if (bytes_per_sec >= 1000 * 1000) {
        return [NSString stringWithFormat:@"%.2f MB/s", ((float)bytes_per_sec) / 1000 / 1000];
    } else if (bytes_per_sec >= 1000) {
        return [NSString stringWithFormat:@"%.1f KB/s", ((float)bytes_per_sec) / 1000];
    } else {
        return [NSString stringWithFormat:@"%ld B/s", (long)bytes_per_sec];
    }
}

- (void)refreshHudView
{
    if (_mediaPlayer == nil)
        return;

    int64_t vdec = ijkmp_get_property_int64(_mediaPlayer, FFP_PROP_INT64_VIDEO_DECODER, FFP_PROPV_DECODER_UNKNOWN);
    float   vdps = ijkmp_get_property_float(_mediaPlayer, FFP_PROP_FLOAT_VIDEO_DECODE_FRAMES_PER_SECOND, .0f);
    float   vfps = ijkmp_get_property_float(_mediaPlayer, FFP_PROP_FLOAT_VIDEO_OUTPUT_FRAMES_PER_SECOND, .0f);

    switch (vdec) {
        case FFP_PROPV_DECODER_VIDEOTOOLBOX:
            [self setHudValue:@"VideoToolbox" forKey:@"vdec"];
            break;
        case FFP_PROPV_DECODER_AVCODEC:
            [self setHudValue:[NSString stringWithFormat:@"avcodec %d.%d.%d",
                                  LIBAVCODEC_VERSION_MAJOR,
                                  LIBAVCODEC_VERSION_MINOR,
                                  LIBAVCODEC_VERSION_MICRO]
                          forKey:@"vdec"];
            break;
        default:
            [self setHudValue:@"N/A" forKey:@"vdec"];
            break;
    }

    [self setHudValue:[NSString stringWithFormat:@"%.2f / %.2f", vdps, vfps] forKey:@"fps"];

    int64_t vcacheb = ijkmp_get_property_int64(_mediaPlayer, FFP_PROP_INT64_VIDEO_CACHED_BYTES, 0);
    int64_t acacheb = ijkmp_get_property_int64(_mediaPlayer, FFP_PROP_INT64_AUDIO_CACHED_BYTES, 0);
    int64_t vcached = ijkmp_get_property_int64(_mediaPlayer, FFP_PROP_INT64_VIDEO_CACHED_DURATION, 0);
    int64_t acached = ijkmp_get_property_int64(_mediaPlayer, FFP_PROP_INT64_AUDIO_CACHED_DURATION, 0);
    int64_t vcachep = ijkmp_get_property_int64(_mediaPlayer, FFP_PROP_INT64_VIDEO_CACHED_PACKETS, 0);
    int64_t acachep = ijkmp_get_property_int64(_mediaPlayer, FFP_PROP_INT64_AUDIO_CACHED_PACKETS, 0);
    [self setHudValue:[NSString stringWithFormat:@"%@, %@, %"PRId64" packets",
                          formatedDurationMilli(vcached),
                          formatedSize(vcacheb),
                          vcachep]
                  forKey:@"v-cache"];
    [self setHudValue:[NSString stringWithFormat:@"%@, %@, %"PRId64" packets",
                          formatedDurationMilli(acached),
                          formatedSize(acacheb),
                          acachep]
                  forKey:@"a-cache"];

    float avdelay = ijkmp_get_property_float(_mediaPlayer, FFP_PROP_FLOAT_AVDELAY, .0f);
    float avdiff  = ijkmp_get_property_float(_mediaPlayer, FFP_PROP_FLOAT_AVDIFF, .0f);
    [self setHudValue:[NSString stringWithFormat:@"%.3f %.3f", avdelay, -avdiff] forKey:@"delay"];

    int64_t bitRate = ijkmp_get_property_int64(_mediaPlayer, FFP_PROP_INT64_BIT_RATE, 0);
    [self setHudValue:[NSString stringWithFormat:@"-%@, %@",
                         formatedSize(_cacheStat.cache_file_forwards),
                          formatedDurationBytesAndBitrate(_cacheStat.cache_file_forwards, bitRate)] forKey:@"cache-forwards"];
    [self setHudValue:formatedSize(_cacheStat.cache_physical_pos) forKey:@"cache-physical-pos"];
    [self setHudValue:formatedSize(_cacheStat.cache_file_pos) forKey:@"cache-file-pos"];
    [self setHudValue:formatedSize(_cacheStat.cache_count_bytes) forKey:@"cache-bytes"];
    [self setHudValue:[NSString stringWithFormat:@"-%@, %@",
                          formatedSize(_asyncStat.buf_backwards),
                          formatedDurationBytesAndBitrate(_asyncStat.buf_backwards, bitRate)]
                  forKey:@"async-backward"];
    [self setHudValue:[NSString stringWithFormat:@"+%@, %@",
                          formatedSize(_asyncStat.buf_forwards),
                          formatedDurationBytesAndBitrate(_asyncStat.buf_forwards, bitRate)]
                  forKey:@"async-forward"];

    int64_t tcpSpeed = ijkmp_get_property_int64(_mediaPlayer, FFP_PROP_INT64_TCP_SPEED, 0);
    [self setHudValue:[NSString stringWithFormat:@"%@", formatedSpeed(tcpSpeed, 1000)]
                  forKey:@"tcp-spd"];

    [self setHudValue:formatedDurationMilli(_monitor.prepareDuration) forKey:@"t-prepared"];
    [self setHudValue:formatedDurationMilli(_monitor.firstVideoFrameLatency) forKey:@"t-render"];
    [self setHudValue:formatedDurationMilli(_monitor.lastPrerollDuration) forKey:@"t-preroll"];
    [self setHudValue:[NSString stringWithFormat:@"%@ / %d",
                          formatedDurationMilli(_monitor.lastHttpOpenDuration),
                          _monitor.httpOpenCount]
                  forKey:@"t-http-open"];
    [self setHudValue:[NSString stringWithFormat:@"%@ / %d",
                          formatedDurationMilli(_monitor.lastHttpSeekDuration),
                          _monitor.httpSeekCount]
                  forKey:@"t-http-seek"];
}

- (void)startHudTimer
{
    if (!_shouldShowHudView)
        return;

    if (_hudTimer != nil)
        return;

    if ([[NSThread currentThread] isMainThread]) {
        _hudViewController.tableView.hidden = NO;
        _hudTimer = [NSTimer scheduledTimerWithTimeInterval:.5f
                                                     target:self
                                                   selector:@selector(refreshHudView)
                                                   userInfo:nil
                                                    repeats:YES];
    } else {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self startHudTimer];
        });
    }
}

- (void)stopHudTimer
{
    if (_hudTimer == nil)
        return;

    if ([[NSThread currentThread] isMainThread]) {
        _hudViewController.tableView.hidden = YES;
        [_hudTimer invalidate];
        _hudTimer = nil;
    } else {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self stopHudTimer];
        });
    }
}

- (void)setShouldShowHudView:(BOOL)shouldShowHudView
{
    if (shouldShowHudView == _shouldShowHudView) {
        return;
    }
    _shouldShowHudView = shouldShowHudView;
    if (shouldShowHudView)
        [self startHudTimer];
    else
        [self stopHudTimer];
}

- (BOOL)shouldShowHudView
{
    return _shouldShowHudView;
}

- (void)setPlaybackRate:(float)playbackRate
{
    if (!_mediaPlayer)
        return;

    return ijkmp_set_playback_rate(_mediaPlayer, playbackRate);
}

- (float)playbackRate
{
    if (!_mediaPlayer)
        return 0.0f;

    return ijkmp_get_property_float(_mediaPlayer, FFP_PROP_FLOAT_PLAYBACK_RATE, 0.0f);
}

- (void)setPlaybackVolume:(float)volume
{
    if (!_mediaPlayer)
        return;
    return ijkmp_set_playback_volume(_mediaPlayer, volume);
}

- (float)playbackVolume
{
    if (!_mediaPlayer)
        return 0.0f;
    return ijkmp_get_property_float(_mediaPlayer, FFP_PROP_FLOAT_PLAYBACK_VOLUME, 1.0f);
}

- (int64_t)getFileSize
{
    if (!_mediaPlayer)
        return 0;
    return ijkmp_get_property_int64(_mediaPlayer, FFP_PROP_INT64_LOGICAL_FILE_SIZE, 0);
}

- (int64_t)trafficStatistic
{
    if (!_mediaPlayer)
        return 0;
    return ijkmp_get_property_int64(_mediaPlayer, FFP_PROP_INT64_TRAFFIC_STATISTIC_BYTE_COUNT, 0);
}

- (float)dropFrameRate
{
    if (!_mediaPlayer)
        return 0;
    return ijkmp_get_property_float(_mediaPlayer, FFP_PROP_FLOAT_DROP_FRAME_RATE, 0.0f);
}

inline static void fillMetaInternal(NSMutableDictionary *meta, IjkMediaMeta *rawMeta, const char *name, NSString *defaultValue)
{
    if (!meta || !rawMeta || !name)
        return;

    NSString *key = [NSString stringWithUTF8String:name];
    const char *value = ijkmeta_get_string_l(rawMeta, name);
    if (value) {
        [meta setObject:[NSString stringWithUTF8String:value] forKey:key];
    } else if (defaultValue) {
        [meta setObject:defaultValue forKey:key];
    } else {
        [meta removeObjectForKey:key];
    }
}

- (void)postEvent: (IJKFFMoviePlayerMessage *)msg
{
    if (!msg)
        return;

    AVMessage *avmsg = &msg->_msg;
    switch (avmsg->what) {
        case FFP_MSG_FLUSH:
            break;
        case FFP_MSG_ERROR: {
            NSLog(@"FFP_MSG_ERROR: %d\n", avmsg->arg1);

            [self setScreenOn:NO];

            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerPlaybackStateDidChangeNotification
             object:self];

            [[NSNotificationCenter defaultCenter]
                postNotificationName:IJKMPMoviePlayerPlaybackDidFinishNotification
                object:self
                userInfo:@{
                    IJKMPMoviePlayerPlaybackDidFinishReasonUserInfoKey: @(IJKMPMovieFinishReasonPlaybackError),
                    @"error": @(avmsg->arg1)}];
            break;
        }
        case FFP_MSG_PREPARED: {
            NSLog(@"FFP_MSG_PREPARED:\n");

            _monitor.prepareDuration = (int64_t)SDL_GetTickHR() - _monitor.prepareStartTick;
            int64_t vdec = ijkmp_get_property_int64(_mediaPlayer, FFP_PROP_INT64_VIDEO_DECODER, FFP_PROPV_DECODER_UNKNOWN);
            switch (vdec) {
                case FFP_PROPV_DECODER_VIDEOTOOLBOX:
                    _monitor.vdecoder = @"VideoToolbox";
                    break;
                case FFP_PROPV_DECODER_AVCODEC:
                    _monitor.vdecoder = [NSString stringWithFormat:@"avcodec %d.%d.%d",
                                         LIBAVCODEC_VERSION_MAJOR,
                                         LIBAVCODEC_VERSION_MINOR,
                                         LIBAVCODEC_VERSION_MICRO];
                    break;
                default:
                    _monitor.vdecoder = @"Unknown";
                    break;
            }

            IjkMediaMeta *rawMeta = ijkmp_get_meta_l(_mediaPlayer);
            if (rawMeta) {
                ijkmeta_lock(rawMeta);

                NSMutableDictionary *newMediaMeta = [[NSMutableDictionary alloc] init];

                fillMetaInternal(newMediaMeta, rawMeta, IJKM_KEY_FORMAT, nil);
                fillMetaInternal(newMediaMeta, rawMeta, IJKM_KEY_DURATION_US, nil);
                fillMetaInternal(newMediaMeta, rawMeta, IJKM_KEY_START_US, nil);
                fillMetaInternal(newMediaMeta, rawMeta, IJKM_KEY_BITRATE, nil);

                fillMetaInternal(newMediaMeta, rawMeta, IJKM_KEY_VIDEO_STREAM, nil);
                fillMetaInternal(newMediaMeta, rawMeta, IJKM_KEY_AUDIO_STREAM, nil);

                int64_t video_stream = ijkmeta_get_int64_l(rawMeta, IJKM_KEY_VIDEO_STREAM, -1);
                int64_t audio_stream = ijkmeta_get_int64_l(rawMeta, IJKM_KEY_AUDIO_STREAM, -1);

                NSMutableArray *streams = [[NSMutableArray alloc] init];

                size_t count = ijkmeta_get_children_count_l(rawMeta);
                for(size_t i = 0; i < count; ++i) {
                    IjkMediaMeta *streamRawMeta = ijkmeta_get_child_l(rawMeta, i);
                    NSMutableDictionary *streamMeta = [[NSMutableDictionary alloc] init];

                    if (streamRawMeta) {
                        fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_TYPE, k_IJKM_VAL_TYPE__UNKNOWN);
                        const char *type = ijkmeta_get_string_l(streamRawMeta, IJKM_KEY_TYPE);
                        if (type) {
                            fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_CODEC_NAME, nil);
                            fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_CODEC_PROFILE, nil);
                            fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_CODEC_LONG_NAME, nil);
                            fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_BITRATE, nil);

                            if (0 == strcmp(type, IJKM_VAL_TYPE__VIDEO)) {
                                fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_WIDTH, nil);
                                fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_HEIGHT, nil);
                                fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_FPS_NUM, nil);
                                fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_FPS_DEN, nil);
                                fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_TBR_NUM, nil);
                                fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_TBR_DEN, nil);
                                fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_SAR_NUM, nil);
                                fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_SAR_DEN, nil);

                                if (video_stream == i) {
                                    _monitor.videoMeta = streamMeta;

                                    int64_t fps_num = ijkmeta_get_int64_l(streamRawMeta, IJKM_KEY_FPS_NUM, 0);
                                    int64_t fps_den = ijkmeta_get_int64_l(streamRawMeta, IJKM_KEY_FPS_DEN, 0);
                                    if (fps_num > 0 && fps_den > 0) {
                                        _fpsInMeta = ((CGFloat)(fps_num)) / fps_den;
                                        NSLog(@"fps in meta %f\n", _fpsInMeta);
                                    }
                                }

                            } else if (0 == strcmp(type, IJKM_VAL_TYPE__AUDIO)) {
                                fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_SAMPLE_RATE, nil);
                                fillMetaInternal(streamMeta, streamRawMeta, IJKM_KEY_CHANNEL_LAYOUT, nil);

                                if (audio_stream == i) {
                                    _monitor.audioMeta = streamMeta;
                                }
                            }
                        }
                    }

                    [streams addObject:streamMeta];
                }

                [newMediaMeta setObject:streams forKey:kk_IJKM_KEY_STREAMS];

                ijkmeta_unlock(rawMeta);
                _monitor.mediaMeta = newMediaMeta;
            }
            ijkmp_set_playback_rate(_mediaPlayer, [self playbackRate]);
            ijkmp_set_playback_volume(_mediaPlayer, [self playbackVolume]);

            [self startHudTimer];
            _isPreparedToPlay = YES;

            [[NSNotificationCenter defaultCenter] postNotificationName:IJKMPMediaPlaybackIsPreparedToPlayDidChangeNotification object:self];
            _loadState = IJKMPMovieLoadStatePlayable | IJKMPMovieLoadStatePlaythroughOK;

            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerLoadStateDidChangeNotification
             object:self];

            break;
        }
        case FFP_MSG_COMPLETED: {

            [self setScreenOn:NO];

            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerPlaybackStateDidChangeNotification
             object:self];

            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerPlaybackDidFinishNotification
             object:self
             userInfo:@{IJKMPMoviePlayerPlaybackDidFinishReasonUserInfoKey: @(IJKMPMovieFinishReasonPlaybackEnded)}];
            break;
        }
        case FFP_MSG_VIDEO_SIZE_CHANGED:
            NSLog(@"FFP_MSG_VIDEO_SIZE_CHANGED: %d, %d\n", avmsg->arg1, avmsg->arg2);
            if (avmsg->arg1 > 0)
                _videoWidth = avmsg->arg1;
            if (avmsg->arg2 > 0)
                _videoHeight = avmsg->arg2;
            [self changeNaturalSize];
            break;
        case FFP_MSG_SAR_CHANGED:
            NSLog(@"FFP_MSG_SAR_CHANGED: %d, %d\n", avmsg->arg1, avmsg->arg2);
            if (avmsg->arg1 > 0)
                _sampleAspectRatioNumerator = avmsg->arg1;
            if (avmsg->arg2 > 0)
                _sampleAspectRatioDenominator = avmsg->arg2;
            [self changeNaturalSize];
            break;
        case FFP_MSG_BUFFERING_START: {
            NSLog(@"FFP_MSG_BUFFERING_START:\n");

            _monitor.lastPrerollStartTick = (int64_t)SDL_GetTickHR();

            _loadState = IJKMPMovieLoadStateStalled;
            _isSeekBuffering = avmsg->arg1;

            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerLoadStateDidChangeNotification
             object:self];
            _isSeekBuffering = 0;
            break;
        }
        case FFP_MSG_BUFFERING_END: {
            NSLog(@"FFP_MSG_BUFFERING_END:\n");

            _monitor.lastPrerollDuration = (int64_t)SDL_GetTickHR() - _monitor.lastPrerollStartTick;

            _loadState = IJKMPMovieLoadStatePlayable | IJKMPMovieLoadStatePlaythroughOK;
            _isSeekBuffering = avmsg->arg1;

            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerLoadStateDidChangeNotification
             object:self];
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerPlaybackStateDidChangeNotification
             object:self];
            _isSeekBuffering = 0;
            break;
        }
        case FFP_MSG_BUFFERING_UPDATE:
            _bufferingPosition = avmsg->arg1;
            _bufferingProgress = avmsg->arg2;
            // NSLog(@"FFP_MSG_BUFFERING_UPDATE: %d, %%%d\n", _bufferingPosition, _bufferingProgress);
            break;
        case FFP_MSG_BUFFERING_BYTES_UPDATE:
            // NSLog(@"FFP_MSG_BUFFERING_BYTES_UPDATE: %d\n", avmsg->arg1);
            break;
        case FFP_MSG_BUFFERING_TIME_UPDATE:
            _bufferingTime       = avmsg->arg1;
            // NSLog(@"FFP_MSG_BUFFERING_TIME_UPDATE: %d\n", avmsg->arg1);
            break;
        case FFP_MSG_PLAYBACK_STATE_CHANGED:
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerPlaybackStateDidChangeNotification
             object:self];
            break;
        case FFP_MSG_SEEK_COMPLETE: {
            NSLog(@"FFP_MSG_SEEK_COMPLETE:\n");
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerDidSeekCompleteNotification
             object:self
             userInfo:@{IJKMPMoviePlayerDidSeekCompleteTargetKey: @(avmsg->arg1),
                        IJKMPMoviePlayerDidSeekCompleteErrorKey: @(avmsg->arg2)}];
            _seeking = NO;
            break;
        }
        case FFP_MSG_VIDEO_DECODER_OPEN: {
            _isVideoToolboxOpen = avmsg->arg1;
            NSLog(@"FFP_MSG_VIDEO_DECODER_OPEN: %@\n", _isVideoToolboxOpen ? @"true" : @"false");
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerVideoDecoderOpenNotification
             object:self];
            break;
        }
        case FFP_MSG_VIDEO_RENDERING_START: {
            NSLog(@"FFP_MSG_VIDEO_RENDERING_START:\n");
            _monitor.firstVideoFrameLatency = (int64_t)SDL_GetTickHR() - _monitor.prepareStartTick;
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerFirstVideoFrameRenderedNotification
             object:self];
            break;
        }
        case FFP_MSG_AUDIO_RENDERING_START: {
            NSLog(@"FFP_MSG_AUDIO_RENDERING_START:\n");
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerFirstAudioFrameRenderedNotification
             object:self];
            break;
        }
        case FFP_MSG_AUDIO_DECODED_START: {
            NSLog(@"FFP_MSG_AUDIO_DECODED_START:\n");
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerFirstAudioFrameDecodedNotification
             object:self];
            break;
        }
        case FFP_MSG_VIDEO_DECODED_START: {
            NSLog(@"FFP_MSG_VIDEO_DECODED_START:\n");
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerFirstVideoFrameDecodedNotification
             object:self];
            break;
        }
        case FFP_MSG_OPEN_INPUT: {
            NSLog(@"FFP_MSG_OPEN_INPUT:\n");
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerOpenInputNotification
             object:self];
            break;
        }
        case FFP_MSG_FIND_STREAM_INFO: {
            NSLog(@"FFP_MSG_FIND_STREAM_INFO:\n");
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerFindStreamInfoNotification
             object:self];
            break;
        }
        case FFP_MSG_COMPONENT_OPEN: {
            NSLog(@"FFP_MSG_COMPONENT_OPEN:\n");
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerComponentOpenNotification
             object:self];
            break;
        }
        case FFP_MSG_ACCURATE_SEEK_COMPLETE: {
            NSLog(@"FFP_MSG_ACCURATE_SEEK_COMPLETE:\n");
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerAccurateSeekCompleteNotification
             object:self
             userInfo:@{IJKMPMoviePlayerDidAccurateSeekCompleteCurPos: @(avmsg->arg1)}];
            break;
        }
        case FFP_MSG_VIDEO_SEEK_RENDERING_START: {
            NSLog(@"FFP_MSG_VIDEO_SEEK_RENDERING_START:\n");
            _isVideoSync = avmsg->arg1;
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerSeekVideoStartNotification
             object:self];
            _isVideoSync = 0;
            break;
        }
        case FFP_MSG_AUDIO_SEEK_RENDERING_START: {
            NSLog(@"FFP_MSG_AUDIO_SEEK_RENDERING_START:\n");
            _isAudioSync = avmsg->arg1;
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMPMoviePlayerSeekAudioStartNotification
             object:self];
            _isAudioSync = 0;
            break;
        }
        default:
            // NSLog(@"unknown FFP_MSG_xxx(%d)\n", avmsg->what);
            break;
    }

    [_msgPool recycle:msg];
}

- (IJKFFMoviePlayerMessage *) obtainMessage {
    return [_msgPool obtain];
}

inline static IJKFFMoviePlayerController *ffplayerRetain(void *arg) {
    return (__bridge_transfer IJKFFMoviePlayerController *) arg;
}

int media_player_msg_loop(void* arg)
{
    @autoreleasepool {
        IjkMediaPlayer *mp = (IjkMediaPlayer*)arg;
        __weak IJKFFMoviePlayerController *ffpController = ffplayerRetain(ijkmp_set_weak_thiz(mp, NULL));
        while (ffpController) {
            @autoreleasepool {
                IJKFFMoviePlayerMessage *msg = [ffpController obtainMessage];
                if (!msg)
                    break;

                int retval = ijkmp_get_msg(mp, &msg->_msg, 1);
                if (retval < 0)
                    break;

                // block-get should never return 0
                assert(retval > 0);
                [ffpController performSelectorOnMainThread:@selector(postEvent:) withObject:msg waitUntilDone:NO];
            }
        }

        // retained in prepare_async, before SDL_CreateThreadEx
        ijkmp_dec_ref_p(&mp);
        return 0;
    }
}

#pragma mark av_format_control_message

static int onInjectIOControl(IJKFFMoviePlayerController *mpc, id<IJKMediaUrlOpenDelegate> delegate, int type, void *data, size_t data_size)
{
    AVAppIOControl *realData = data;
    assert(realData);
    assert(sizeof(AVAppIOControl) == data_size);
    realData->is_handled     = NO;
    realData->is_url_changed = NO;

    if (delegate == nil)
        return 0;

    NSString *urlString = [NSString stringWithUTF8String:realData->url];

    IJKMediaUrlOpenData *openData =
    [[IJKMediaUrlOpenData alloc] initWithUrl:urlString
                                       event:(IJKMediaEvent)type
                                segmentIndex:realData->segment_index
                                retryCounter:realData->retry_counter];

    [delegate willOpenUrl:openData];
    if (openData.error < 0)
        return -1;

    if (openData.isHandled) {
        realData->is_handled = YES;
        if (openData.isUrlChanged && openData.url != nil) {
            realData->is_url_changed = YES;
            const char *newUrlUTF8 = [openData.url UTF8String];
            strlcpy(realData->url, newUrlUTF8, sizeof(realData->url));
            realData->url[sizeof(realData->url) - 1] = 0;
        }
    }
    
    return 0;
}

static int onInjectTcpIOControl(IJKFFMoviePlayerController *mpc, id<IJKMediaUrlOpenDelegate> delegate, int type, void *data, size_t data_size)
{
    AVAppTcpIOControl *realData = data;
    assert(realData);
    assert(sizeof(AVAppTcpIOControl) == data_size);

    switch (type) {
        case IJKMediaCtrl_WillTcpOpen:

            break;
        case IJKMediaCtrl_DidTcpOpen:
            mpc->_monitor.tcpError = realData->error;
            mpc->_monitor.remoteIp = [NSString stringWithUTF8String:realData->ip];
            [mpc setHudValue: mpc->_monitor.remoteIp forKey:@"ip"];
            break;
        default:
            assert(!"unexcepted type for tcp io control");
            break;
    }

    if (delegate == nil)
        return 0;

    NSString *urlString = [NSString stringWithUTF8String:realData->ip];

    IJKMediaUrlOpenData *openData =
    [[IJKMediaUrlOpenData alloc] initWithUrl:urlString
                                       event:(IJKMediaEvent)type
                                segmentIndex:0
                                retryCounter:0];
    openData.fd = realData->fd;

    [delegate willOpenUrl:openData];
    if (openData.error < 0)
        return -1;
    [mpc setHudValue: [NSString stringWithFormat:@"fd:%d %@", openData.fd, openData.msg?:@"unknown"] forKey:@"tcp-info"];
    return 0;
}

static int onInjectAsyncStatistic(IJKFFMoviePlayerController *mpc, int type, void *data, size_t data_size)
{
    AVAppAsyncStatistic *realData = data;
    assert(realData);
    assert(sizeof(AVAppAsyncStatistic) == data_size);

    mpc->_asyncStat = *realData;
    return 0;
}

static int onInectIJKIOStatistic(IJKFFMoviePlayerController *mpc, int type, void *data, size_t data_size)
{
    IjkIOAppCacheStatistic *realData = data;
    assert(realData);
    assert(sizeof(IjkIOAppCacheStatistic) == data_size);

    mpc->_cacheStat = *realData;
    return 0;
}

static int64_t calculateElapsed(int64_t begin, int64_t end)
{
    if (begin <= 0)
        return -1;

    if (end < begin)
        return -1;

    return end - begin;
}

static int onInjectOnHttpEvent(IJKFFMoviePlayerController *mpc, int type, void *data, size_t data_size)
{
    AVAppHttpEvent *realData = data;
    assert(realData);
    assert(sizeof(AVAppHttpEvent) == data_size);

    NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
    NSURL        *nsurl   = nil;
    IJKFFMonitor *monitor = mpc->_monitor;
    NSString     *url  = monitor.httpUrl;
    NSString     *host = monitor.httpHost;
    int64_t       elapsed = 0;

    id<IJKMediaNativeInvokeDelegate> delegate = mpc.nativeInvokeDelegate;

    switch (type) {
        case AVAPP_EVENT_WILL_HTTP_OPEN:
            url   = [NSString stringWithUTF8String:realData->url];
            nsurl = [NSURL URLWithString:url];
            host  = nsurl.host;

            monitor.httpUrl      = url;
            monitor.httpHost     = host;
            monitor.httpOpenTick = SDL_GetTickHR();
            [mpc setHudUrl:url];

            if (delegate != nil) {
                dict[IJKMediaEventAttrKey_host]         = [NSString ijk_stringBeEmptyIfNil:host];
                dict[IJKMediaEventAttrKey_url]          = [NSString ijk_stringBeEmptyIfNil:monitor.httpUrl];
                [delegate invoke:type attributes:dict];
            }
            break;
        case AVAPP_EVENT_DID_HTTP_OPEN:
            elapsed = calculateElapsed(monitor.httpOpenTick, SDL_GetTickHR());
            monitor.httpError = realData->error;
            monitor.httpCode  = realData->http_code;
            monitor.filesize  = realData->filesize;
            monitor.httpOpenCount++;
            monitor.httpOpenTick = 0;
            monitor.lastHttpOpenDuration = elapsed;
            [mpc setHudValue:@(realData->http_code).stringValue forKey:@"http"];

            if (delegate != nil) {
                dict[IJKMediaEventAttrKey_time_of_event]    = @(elapsed).stringValue;
                dict[IJKMediaEventAttrKey_url]              = [NSString ijk_stringBeEmptyIfNil:monitor.httpUrl];
                dict[IJKMediaEventAttrKey_host]             = [NSString ijk_stringBeEmptyIfNil:host];
                dict[IJKMediaEventAttrKey_error]            = @(realData->error).stringValue;
                dict[IJKMediaEventAttrKey_http_code]        = @(realData->http_code).stringValue;
                dict[IJKMediaEventAttrKey_file_size]        = @(realData->filesize).stringValue;
                [delegate invoke:type attributes:dict];
            }
            break;
        case AVAPP_EVENT_WILL_HTTP_SEEK:
            monitor.httpSeekTick = SDL_GetTickHR();

            if (delegate != nil) {
                dict[IJKMediaEventAttrKey_host]         = [NSString ijk_stringBeEmptyIfNil:host];
                dict[IJKMediaEventAttrKey_offset]       = @(realData->offset).stringValue;
                [delegate invoke:type attributes:dict];
            }
            break;
        case AVAPP_EVENT_DID_HTTP_SEEK:
            elapsed = calculateElapsed(monitor.httpSeekTick, SDL_GetTickHR());
            monitor.httpError = realData->error;
            monitor.httpCode  = realData->http_code;
            monitor.httpSeekCount++;
            monitor.httpSeekTick = 0;
            monitor.lastHttpSeekDuration = elapsed;
            [mpc setHudValue:@(realData->http_code).stringValue forKey:@"http"];

            if (delegate != nil) {
                dict[IJKMediaEventAttrKey_time_of_event]    = @(elapsed).stringValue;
                dict[IJKMediaEventAttrKey_url]              = [NSString ijk_stringBeEmptyIfNil:monitor.httpUrl];
                dict[IJKMediaEventAttrKey_host]             = [NSString ijk_stringBeEmptyIfNil:host];
                dict[IJKMediaEventAttrKey_offset]           = @(realData->offset).stringValue;
                dict[IJKMediaEventAttrKey_error]            = @(realData->error).stringValue;
                dict[IJKMediaEventAttrKey_http_code]        = @(realData->http_code).stringValue;
                [delegate invoke:type attributes:dict];
            }
            break;
    }

    return 0;
}

// NOTE: could be called from multiple thread
static int ijkff_inject_callback(void *opaque, int message, void *data, size_t data_size)
{
    IJKWeakHolder *weakHolder = (__bridge IJKWeakHolder*)opaque;
    IJKFFMoviePlayerController *mpc = weakHolder.object;
    if (!mpc)
        return 0;

    switch (message) {
        case AVAPP_CTRL_WILL_CONCAT_SEGMENT_OPEN:
            return onInjectIOControl(mpc, mpc.segmentOpenDelegate, message, data, data_size);
        case AVAPP_CTRL_WILL_TCP_OPEN:
            return onInjectTcpIOControl(mpc, mpc.tcpOpenDelegate, message, data, data_size);
        case AVAPP_CTRL_WILL_HTTP_OPEN:
            return onInjectIOControl(mpc, mpc.httpOpenDelegate, message, data, data_size);
        case AVAPP_CTRL_WILL_LIVE_OPEN:
            return onInjectIOControl(mpc, mpc.liveOpenDelegate, message, data, data_size);
        case AVAPP_EVENT_ASYNC_STATISTIC:
            return onInjectAsyncStatistic(mpc, message, data, data_size);
        case IJKIOAPP_EVENT_CACHE_STATISTIC:
            return onInectIJKIOStatistic(mpc, message, data, data_size);
        case AVAPP_CTRL_DID_TCP_OPEN:
            return onInjectTcpIOControl(mpc, mpc.tcpOpenDelegate, message, data, data_size);
        case AVAPP_EVENT_WILL_HTTP_OPEN:
        case AVAPP_EVENT_DID_HTTP_OPEN:
        case AVAPP_EVENT_WILL_HTTP_SEEK:
        case AVAPP_EVENT_DID_HTTP_SEEK:
            return onInjectOnHttpEvent(mpc, message, data, data_size);
        default: {
            return 0;
        }
    }
}

#pragma mark Airplay

-(BOOL)allowsMediaAirPlay
{
    if (!self)
        return NO;
    return _allowsMediaAirPlay;
}

-(void)setAllowsMediaAirPlay:(BOOL)b
{
    if (!self)
        return;
    _allowsMediaAirPlay = b;
}

-(BOOL)airPlayMediaActive
{
    if (!self)
        return NO;
    if (_isDanmakuMediaAirPlay) {
        return YES;
    }
    return NO;
}

-(BOOL)isDanmakuMediaAirPlay
{
    return _isDanmakuMediaAirPlay;
}

-(void)setIsDanmakuMediaAirPlay:(BOOL)isDanmakuMediaAirPlay
{
    _isDanmakuMediaAirPlay = isDanmakuMediaAirPlay;
    if (_isDanmakuMediaAirPlay) {
        _glView.scaleFactor = 1.0f;
    }
    else {
        CGFloat scale = [[UIScreen mainScreen] scale];
        if (scale < 0.1f)
            scale = 1.0f;
        _glView.scaleFactor = scale;
    }
     [[NSNotificationCenter defaultCenter] postNotificationName:IJKMPMoviePlayerIsAirPlayVideoActiveDidChangeNotification object:nil userInfo:nil];
}


#pragma mark Option Conventionce

- (void)setFormatOptionValue:(NSString *)value forKey:(NSString *)key
{
    [self setOptionValue:value forKey:key ofCategory:kIJKFFOptionCategoryFormat];
}

- (void)setCodecOptionValue:(NSString *)value forKey:(NSString *)key
{
    [self setOptionValue:value forKey:key ofCategory:kIJKFFOptionCategoryCodec];
}

- (void)setSwsOptionValue:(NSString *)value forKey:(NSString *)key
{
    [self setOptionValue:value forKey:key ofCategory:kIJKFFOptionCategorySws];
}

- (void)setPlayerOptionValue:(NSString *)value forKey:(NSString *)key
{
    [self setOptionValue:value forKey:key ofCategory:kIJKFFOptionCategoryPlayer];
}

- (void)setFormatOptionIntValue:(int64_t)value forKey:(NSString *)key
{
    [self setOptionIntValue:value forKey:key ofCategory:kIJKFFOptionCategoryFormat];
}

- (void)setCodecOptionIntValue:(int64_t)value forKey:(NSString *)key
{
    [self setOptionIntValue:value forKey:key ofCategory:kIJKFFOptionCategoryCodec];
}

- (void)setSwsOptionIntValue:(int64_t)value forKey:(NSString *)key
{
    [self setOptionIntValue:value forKey:key ofCategory:kIJKFFOptionCategorySws];
}

- (void)setPlayerOptionIntValue:(int64_t)value forKey:(NSString *)key
{
    [self setOptionIntValue:value forKey:key ofCategory:kIJKFFOptionCategoryPlayer];
}

- (void)setMaxBufferSize:(int)maxBufferSize
{
    [self setPlayerOptionIntValue:maxBufferSize forKey:@"max-buffer-size"];
}

#pragma mark app state changed

- (void)registerApplicationObservers
{
    [_notificationManager addObserver:self
                             selector:@selector(audioSessionInterrupt:)
                                 name:AVAudioSessionInterruptionNotification
                               object:nil];

    [_notificationManager addObserver:self
                             selector:@selector(applicationWillEnterForeground)
                                 name:UIApplicationWillEnterForegroundNotification
                               object:nil];

    [_notificationManager addObserver:self
                             selector:@selector(applicationDidBecomeActive)
                                 name:UIApplicationDidBecomeActiveNotification
                               object:nil];

    [_notificationManager addObserver:self
                             selector:@selector(applicationWillResignActive)
                                 name:UIApplicationWillResignActiveNotification
                               object:nil];

    [_notificationManager addObserver:self
                             selector:@selector(applicationDidEnterBackground)
                                 name:UIApplicationDidEnterBackgroundNotification
                               object:nil];

    [_notificationManager addObserver:self
                             selector:@selector(applicationWillTerminate)
                                 name:UIApplicationWillTerminateNotification
                               object:nil];
}

- (void)unregisterApplicationObservers
{
    [_notificationManager removeAllObservers:self];
}

- (void)audioSessionInterrupt:(NSNotification *)notification
{
    int reason = [[[notification userInfo] valueForKey:AVAudioSessionInterruptionTypeKey] intValue];
    switch (reason) {
        case AVAudioSessionInterruptionTypeBegan: {
            NSLog(@"IJKFFMoviePlayerController:audioSessionInterrupt: begin\n");
            switch (self.playbackState) {
                case IJKMPMoviePlaybackStatePaused:
                case IJKMPMoviePlaybackStateStopped:
                    _playingBeforeInterruption = NO;
                    break;
                default:
                    _playingBeforeInterruption = YES;
                    break;
            }
            [self pause];
            [[IJKAudioKit sharedInstance] setActive:NO];
            break;
        }
        case AVAudioSessionInterruptionTypeEnded: {
            NSLog(@"IJKFFMoviePlayerController:audioSessionInterrupt: end\n");
            [[IJKAudioKit sharedInstance] setActive:YES];
            if (_playingBeforeInterruption) {
                [self play];
            }
            break;
        }
    }
}

- (void)applicationWillEnterForeground
{
    NSLog(@"IJKFFMoviePlayerController:applicationWillEnterForeground: %d", (int)[UIApplication sharedApplication].applicationState);
}

- (void)applicationDidBecomeActive
{
    NSLog(@"IJKFFMoviePlayerController:applicationDidBecomeActive: %d", (int)[UIApplication sharedApplication].applicationState);
}

- (void)applicationWillResignActive
{
    NSLog(@"IJKFFMoviePlayerController:applicationWillResignActive: %d", (int)[UIApplication sharedApplication].applicationState);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_pauseInBackground) {
            [self pause];
        }
    });
}

- (void)applicationDidEnterBackground
{
    NSLog(@"IJKFFMoviePlayerController:applicationDidEnterBackground: %d", (int)[UIApplication sharedApplication].applicationState);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_pauseInBackground) {
            [self pause];
        }
    });
}

- (void)applicationWillTerminate
{
    NSLog(@"IJKFFMoviePlayerController:applicationWillTerminate: %d", (int)[UIApplication sharedApplication].applicationState);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_pauseInBackground) {
            [self pause];
        }
    });
}

#pragma mark IJKFFHudController
- (void)setHudValue:(NSString *)value forKey:(NSString *)key
{
    if ([[NSThread currentThread] isMainThread]) {
        [_hudViewController setHudValue:value forKey:key];
    } else {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self setHudValue:value forKey:key];
        });
    }
}

@end


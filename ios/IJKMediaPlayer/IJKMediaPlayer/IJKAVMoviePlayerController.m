/*
 * IJKAVMoviePlayerController.m
 *
 * Copyright (c) 2014 Bilibili
 * Copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

/*
 File: AVPlayerDemoPlaybackViewController.m
 Abstract: UIViewController managing a playback view, thumbnail view, and associated playback UI.
 Version: 1.3
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 
 */

#import "IJKAVMoviePlayerController.h"
#import "IJKAVPlayerLayerView.h"
#import "IJKAudioKit.h"
#import "IJKMediaModule.h"
#import "IJKMediaUtils.h"
#import "IJKKVOController.h"
#import "IJKNotificationManager.h"
#include "ijksdl/ios/ijksdl_ios.h"

// avoid float equal compare
inline static bool isFloatZero(float value)
{
    return fabsf(value) <= 0.00001f;
}

// resume play after stall
static const float kMaxHighWaterMarkMilli   = 15 * 1000;

static NSString *kErrorDomain = @"IJKAVMoviePlayer";
static const NSInteger kEC_CurrentPlayerItemIsNil   = 5001;
static const NSInteger kEC_PlayerItemCancelled      = 5002;

static void *KVO_AVPlayer_rate          = &KVO_AVPlayer_rate;
static void *KVO_AVPlayer_currentItem   = &KVO_AVPlayer_currentItem;
static void *KVO_AVPlayer_airplay   = &KVO_AVPlayer_airplay;

static void *KVO_AVPlayerItem_state                     = &KVO_AVPlayerItem_state;
static void *KVO_AVPlayerItem_loadedTimeRanges          = &KVO_AVPlayerItem_loadedTimeRanges;
static void *KVO_AVPlayerItem_playbackLikelyToKeepUp    = &KVO_AVPlayerItem_playbackLikelyToKeepUp;
static void *KVO_AVPlayerItem_playbackBufferFull        = &KVO_AVPlayerItem_playbackBufferFull;
static void *KVO_AVPlayerItem_playbackBufferEmpty       = &KVO_AVPlayerItem_playbackBufferEmpty;

@interface IJKAVMoviePlayerController()

// Redeclare property
@property(nonatomic, readwrite) UIView *view;

@property(nonatomic, readwrite) NSTimeInterval duration;
@property(nonatomic, readwrite) NSTimeInterval playableDuration;
@property(nonatomic, readwrite) NSInteger bufferingProgress;

@property(nonatomic, readwrite)  BOOL isPreparedToPlay;

@end

@implementation IJKAVMoviePlayerController {
    NSURL           *_playUrl;
    AVURLAsset      *_playAsset;
    AVPlayerItem    *_playerItem;
    AVPlayer        *_player;
    IJKAVPlayerLayerView * _avView;
    
    IJKKVOController *_playerKVO;
    IJKKVOController *_playerItemKVO;
    
    id _timeObserver;
    dispatch_once_t _readyToPlayToken;
    // while AVPlayer is prerolling, it could resume itself.
    // foring start could
    BOOL _isPrerolling;

    NSTimeInterval _seekingTime;
    BOOL _isSeeking;
    BOOL _isError;
    BOOL _isCompleted;
    BOOL _isShutdown;
    
    BOOL _pauseInBackground;
    
    BOOL _playbackLikelyToKeeyUp;
    BOOL _playbackBufferEmpty;
    BOOL _playbackBufferFull;
    
    BOOL _playingBeforeInterruption;

    IJKNotificationManager *_notificationManager;

    float _playbackRate;
    float _playbackVolume;
}

@synthesize view                        = _view;
@synthesize currentPlaybackTime         = _currentPlaybackTime;
@synthesize duration                    = _duration;
@synthesize playableDuration            = _playableDuration;
@synthesize bufferingProgress           = _bufferingProgress;

@synthesize numberOfBytesTransferred    = _numberOfBytesTransferred;

@synthesize playbackState               = _playbackState;
@synthesize loadState                   = _loadState;

@synthesize scalingMode                 = _scalingMode;
@synthesize shouldAutoplay              = _shouldAutoplay;
@synthesize isDanmakuMediaAirPlay       = _isDanmakuMediaAirPlay;

static IJKAVMoviePlayerController* instance;

- (id)initWithContentURL:(NSURL *)aUrl
{
    self = [super init];
    if (self != nil) {
        self.scalingMode = IJKMPMovieScalingModeAspectFit;
        self.shouldAutoplay = NO;

        _playUrl = aUrl;

        _avView = [[IJKAVPlayerLayerView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
        self.view = _avView;

        // TODO:
        [[IJKAudioKit sharedInstance] setupAudioSession];

        _isPrerolling           = NO;

        _isSeeking              = NO;
        _isError                = NO;
        _isCompleted            = NO;
        self.bufferingProgress  = 0;

        _playbackLikelyToKeeyUp = NO;
        _playbackBufferEmpty    = YES;
        _playbackBufferFull     = NO;

        _playbackRate           = 1.0f;
        _playbackVolume         = 1.0f;
        // init extra
        [self setScreenOn:YES];

        _notificationManager = [[IJKNotificationManager alloc] init];
    }
    return self;
}

+ (id)getInstance:(NSString *)aUrl
{
    if (instance == nil) {
        instance = [[IJKAVMoviePlayerController alloc] initWithContentURLString:aUrl];
    } else {
        instance = [instance initWithContentURLString:aUrl];
    }
    return instance;
}

- (id)initWithContentURLString:(NSString *)aUrl
{
    NSURL *url;
    if (aUrl == nil) {
        aUrl = @"";
    }
    if ([aUrl rangeOfString:@"/"].location == 0) {
        //本地
        url = [NSURL fileURLWithPath:aUrl];
    }
    else {
        url = [NSURL URLWithString:aUrl];
    }
    self = [self initWithContentURL:url];
    if (self != nil) {
        
    }
    return self;
}

- (void)setScreenOn: (BOOL)on
{
    [IJKMediaModule sharedModule].mediaModuleIdleTimerDisabled = on;
}

- (void)dealloc
{
    [self shutdown];
}

- (void)prepareToPlay
{
    AVURLAsset *asset = [AVURLAsset URLAssetWithURL:_playUrl options:nil];
    NSArray *requestedKeys = @[@"playable"];
    
    _playAsset = asset;
    [asset loadValuesAsynchronouslyForKeys:requestedKeys
                         completionHandler:^{
                             dispatch_async( dispatch_get_main_queue(), ^{
                                 [self didPrepareToPlayAsset:asset withKeys:requestedKeys];
                                 [[NSNotificationCenter defaultCenter]
                                  postNotificationName:IJKMPMovieNaturalSizeAvailableNotification
                                  object:self];

                                 [self setPlaybackVolume:_playbackVolume];
                             });
                         }];
}

- (void)play
{
    if (_isCompleted)
    {
        _isCompleted = NO;
        [_player seekToTime:kCMTimeZero];
    }
    
    [_player play];
}

- (void)pause
{
    _isPrerolling = NO;
    [_player pause];
}

- (void)stop
{
    [_player pause];
    [self setScreenOn:NO];
    _isCompleted = YES;
}

- (BOOL)isPlaying
{
    if (!isFloatZero(_player.rate)) {
        return YES;
    } else {
        if (_isPrerolling) {
            return YES;
        } else {
            return NO;
        }
    }
}

- (void)shutdown
{
    _isShutdown = YES;
    [self stop];
    
    if (_playerItem != nil) {
        [_playerItem cancelPendingSeeks];
    }
    
    [_playerItemKVO safelyRemoveAllObservers];
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:nil
                                                  object:_playerItem];
    
    [_playerKVO safelyRemoveAllObservers];
    
    [self unregisterApplicationObservers];
    
    if (_avView != nil) {
        [_avView setPlayer:nil];
    }
    
    self.view = nil;
}

- (UIImage *)thumbnailImageAtCurrentTime
{
    AVAssetImageGenerator *imageGenerator = [AVAssetImageGenerator assetImageGeneratorWithAsset:_playAsset];
    CMTime expectedTime = _playerItem.currentTime;
    CGImageRef cgImage = NULL;
    
    imageGenerator.requestedTimeToleranceBefore = kCMTimeZero;
    imageGenerator.requestedTimeToleranceAfter = kCMTimeZero;
    cgImage = [imageGenerator copyCGImageAtTime:expectedTime actualTime:NULL error:NULL];
    
    if (!cgImage) {
        imageGenerator.requestedTimeToleranceBefore = kCMTimePositiveInfinity;
        imageGenerator.requestedTimeToleranceAfter = kCMTimePositiveInfinity;
        cgImage = [imageGenerator copyCGImageAtTime:expectedTime actualTime:NULL error:NULL];
    }
    
    UIImage *image = [UIImage imageWithCGImage:cgImage];
    return image;
}

- (void)setCurrentPlaybackTime:(NSTimeInterval)aCurrentPlaybackTime
{
    if (!_player)
        return;

    _seekingTime = aCurrentPlaybackTime;
    _isSeeking = YES;
    _bufferingProgress = 0;
    [self didPlaybackStateChange];
    [self didLoadStateChange];
    if (_isPrerolling) {
        [_player pause];
    }

    [_player seekToTime:CMTimeMakeWithSeconds(aCurrentPlaybackTime, NSEC_PER_SEC)
      completionHandler:^(BOOL finished) {
          dispatch_async(dispatch_get_main_queue(), ^{
              _isSeeking = NO;
              if (_isPrerolling) {
                  [_player play];
              }
              [self didPlaybackStateChange];
              [self didLoadStateChange];
          });
      }];
}

- (NSTimeInterval)currentPlaybackTime
{
    if (!_player)
        return 0.0f;

    if (_isSeeking)
        return _seekingTime;

    return CMTimeGetSeconds([_player currentTime]);
}

-(int64_t)numberOfBytesTransferred
{
#if 0
    if (_player == nil)
        return 0;

    AVPlayerItem *playerItem = [_player currentItem];
    if (playerItem == nil)
        return 0;

    NSArray *events = playerItem.accessLog.events;
    if (events != nil && events.count > 0) {
        MPMovieAccessLogEvent *currentEvent = [events objectAtIndex:events.count -1];
        return currentEvent.numberOfBytesTransferred;
    }
#endif
    return 0;
}

- (IJKMPMoviePlaybackState)playbackState
{
    if (!_player)
        return IJKMPMoviePlaybackStateStopped;
    
    IJKMPMoviePlaybackState mpState = IJKMPMoviePlaybackStateStopped;
    if (_isCompleted) {
        mpState = IJKMPMoviePlaybackStateStopped;
    } else if (_isSeeking) {
        mpState = IJKMPMoviePlaybackStateSeekingForward;
    } else if ([self isPlaying]) {
        mpState = IJKMPMoviePlaybackStatePlaying;
    } else {
        mpState = IJKMPMoviePlaybackStatePaused;
    }
    return mpState;
}

- (IJKMPMovieLoadState)loadState
{
    if (_player == nil)
        return IJKMPMovieLoadStateUnknown;
    
    if (_isSeeking)
        return IJKMPMovieLoadStateStalled;
    
    AVPlayerItem *playerItem = [_player currentItem];
    if (playerItem == nil)
        return IJKMPMovieLoadStateUnknown;
    
    if (_player != nil && !isFloatZero(_player.rate)) {
        // NSLog(@"loadState: playing");
        return IJKMPMovieLoadStatePlayable | IJKMPMovieLoadStatePlaythroughOK;
    } else if ([playerItem isPlaybackBufferFull]) {
        // NSLog(@"loadState: isPlaybackBufferFull");
        return IJKMPMovieLoadStatePlayable | IJKMPMovieLoadStatePlaythroughOK;
    } else if ([playerItem isPlaybackLikelyToKeepUp]) {
        // NSLog(@"loadState: isPlaybackLikelyToKeepUp");
        return IJKMPMovieLoadStatePlayable | IJKMPMovieLoadStatePlaythroughOK;
    } else if ([playerItem isPlaybackBufferEmpty]) {
        // NSLog(@"loadState: isPlaybackBufferEmpty");
        return IJKMPMovieLoadStateStalled;
    } else {
        NSLog(@"loadState: unknown");
        return IJKMPMovieLoadStateUnknown;
    }
}

-(void)setPlaybackRate:(float)playbackRate
{
    _playbackRate = playbackRate;
    if (_player != nil && !isFloatZero(_player.rate)) {
        _player.rate = _playbackRate;
    }
}

-(float)playbackRate
{
    return _playbackRate;
}

-(void)setPlaybackVolume:(float)playbackVolume
{
    _playbackVolume = playbackVolume;
    if (_player != nil && _player.volume != playbackVolume) {
        _player.volume = playbackVolume;
    }
    BOOL muted = fabs(playbackVolume) < 1e-6;
    if (_player != nil && _player.muted != muted) {
        _player.muted = muted;
    }
}

-(float)playbackVolume
{
    return _playbackVolume;
}

- (void)didPrepareToPlayAsset:(AVURLAsset *)asset withKeys:(NSArray *)requestedKeys
{
    if (_isShutdown)
        return;
    
    /* Make sure that the value of each key has loaded successfully. */
    for (NSString *thisKey in requestedKeys)
    {
        NSError *error = nil;
        AVKeyValueStatus keyStatus = [asset statusOfValueForKey:thisKey error:&error];
        if (keyStatus == AVKeyValueStatusFailed)
        {
            [self assetFailedToPrepareForPlayback:error];
            return;
        } else if (keyStatus == AVKeyValueStatusCancelled) {
            // TODO [AVAsset cancelLoading]
            error = [self createErrorWithCode:kEC_PlayerItemCancelled
                                  description:@"player item cancelled"
                                       reason:nil];
            [self assetFailedToPrepareForPlayback:error];
            return;
        }
    }
    
    /* Use the AVAsset playable property to detect whether the asset can be played. */
    if (!asset.playable)
    {
        NSError *assetCannotBePlayedError = [NSError errorWithDomain:@"AVMoviePlayer"
                                                                code:0
                                                            userInfo:nil];
        
        [self assetFailedToPrepareForPlayback:assetCannotBePlayedError];
        return;
    }
    
    /* At this point we're ready to set up for playback of the asset. */
    
    /* Stop observing our prior AVPlayerItem, if we have one. */
    [_playerItemKVO safelyRemoveAllObservers];
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:nil
                                                  object:_playerItem];
    
    /* Create a new instance of AVPlayerItem from the now successfully loaded AVAsset. */
    _playerItem = [AVPlayerItem playerItemWithAsset:asset];
    _playerItemKVO = [[IJKKVOController alloc] initWithTarget:_playerItem];
    [self registerApplicationObservers];
    /* Observe the player item "status" key to determine when it is ready to play. */
    [_playerItemKVO safelyAddObserver:self
                           forKeyPath:@"status"
                              options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                              context:KVO_AVPlayerItem_state];
    
    [_playerItemKVO safelyAddObserver:self
                           forKeyPath:@"loadedTimeRanges"
                              options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                              context:KVO_AVPlayerItem_loadedTimeRanges];
    
    [_playerItemKVO safelyAddObserver:self
                           forKeyPath:@"playbackLikelyToKeepUp"
                              options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                              context:KVO_AVPlayerItem_playbackLikelyToKeepUp];
    
    [_playerItemKVO safelyAddObserver:self
                           forKeyPath:@"playbackBufferEmpty"
                              options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                              context:KVO_AVPlayerItem_playbackBufferEmpty];
    
    [_playerItemKVO safelyAddObserver:self
                           forKeyPath:@"playbackBufferFull"
                              options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                              context:KVO_AVPlayerItem_playbackBufferFull];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(playerItemDidReachEnd:)
                                                 name:AVPlayerItemDidPlayToEndTimeNotification
                                               object:_playerItem];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(playerItemFailedToPlayToEndTime:)
                                                 name:AVPlayerItemFailedToPlayToEndTimeNotification
                                               object:_playerItem];
    
    _isCompleted = NO;
    
    /* Create new player, if we don't already have one. */
    if (!_player)
    {
        /* Get a new AVPlayer initialized to play the specified player item. */
        _player = [AVPlayer playerWithPlayerItem:_playerItem];
        _playerKVO = [[IJKKVOController alloc] initWithTarget:_player];
        
        /* Observe the AVPlayer "currentItem" property to find out when any
         AVPlayer replaceCurrentItemWithPlayerItem: replacement will/did
         occur.*/
        [_playerKVO safelyAddObserver:self
                           forKeyPath:@"currentItem"
                              options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                              context:KVO_AVPlayer_currentItem];
        
        /* Observe the AVPlayer "rate" property to update the scrubber control. */
        [_playerKVO safelyAddObserver:self
                           forKeyPath:@"rate"
                              options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                              context:KVO_AVPlayer_rate];
        
        [_playerKVO safelyAddObserver:self
                           forKeyPath:@"airPlayVideoActive"
                              options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                              context:KVO_AVPlayer_airplay];
    }
    
    /* Make our new AVPlayerItem the AVPlayer's current item. */
    if (_player.currentItem != _playerItem)
    {
        /* Replace the player item with a new player item. The item replacement occurs
         asynchronously; observe the currentItem property to find out when the
         replacement will/did occur
         
         If needed, configure player item here (example: adding outputs, setting text style rules,
         selecting media options) before associating it with a player
         */
        [_player replaceCurrentItemWithPlayerItem:_playerItem];
        
        // TODO: notify state change
    }
    
    // TODO: set time to 0;
}

- (void)didPlaybackStateChange
{
    if (_playbackState != self.playbackState) {
        _playbackState = self.playbackState;
        [[NSNotificationCenter defaultCenter]
         postNotificationName:IJKMPMoviePlayerPlaybackStateDidChangeNotification
         object:self];
    }
    
}

- (void)fetchLoadStateFromItem:(AVPlayerItem*)playerItem
{
    if (playerItem == nil)
        return;
    
    _playbackLikelyToKeeyUp = playerItem.isPlaybackLikelyToKeepUp;
    _playbackBufferEmpty    = playerItem.isPlaybackBufferEmpty;
    _playbackBufferFull     = playerItem.isPlaybackBufferFull;
}

- (void)didLoadStateChange
{
    // NOTE: do not force play after stall,
    // which may cause AVPlayer get into wrong state
    //
    // Rely on AVPlayer's auto resume.
    
    [[NSNotificationCenter defaultCenter]
     postNotificationName:IJKMPMoviePlayerLoadStateDidChangeNotification
     object:self];
}

- (void)didPlayableDurationUpdate
{
    NSTimeInterval currentPlaybackTime = self.currentPlaybackTime;
    int playableDurationMilli    = (int)(self.playableDuration * 1000);
    int currentPlaybackTimeMilli = (int)(currentPlaybackTime * 1000);
    
    int bufferedDurationMilli = playableDurationMilli - currentPlaybackTimeMilli;
    if (bufferedDurationMilli > 0) {
        self.bufferingProgress = bufferedDurationMilli * 100 / kMaxHighWaterMarkMilli;
        
        if (self.bufferingProgress > 100) {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (self.bufferingProgress > 100) {
                    if ([self isPlaying]) {
                        _player.rate = _playbackRate;
                    }
                }
            });
        }
        
    }
    
    NSLog(@"KVO_AVPlayerItem_loadedTimeRanges: %d / %d\n",
          bufferedDurationMilli,
          (int)kMaxHighWaterMarkMilli);
}

- (void)onError:(NSError *)error
{
    _isError = YES;
    
    __block NSError *blockError = error;
    
    NSLog(@"AVPlayer: onError\n");
    dispatch_async(dispatch_get_main_queue(), ^{
        [self didPlaybackStateChange];
        [self didLoadStateChange];
        [self setScreenOn:NO];
 
        if (blockError == nil) {
            blockError = [[NSError alloc] init];
        }
        
        [[NSNotificationCenter defaultCenter]
         postNotificationName:IJKMPMoviePlayerPlaybackDidFinishNotification
         object:self
         userInfo:@{
                    IJKMPMoviePlayerPlaybackDidFinishReasonUserInfoKey: @(IJKMPMovieFinishReasonPlaybackError),
                    @"error": blockError
                    }];
    });
}

- (void)assetFailedToPrepareForPlayback:(NSError *)error
{
    if (_isShutdown)
        return;
    
    [self onError:error];
}

- (void)playerItemFailedToPlayToEndTime:(NSNotification *)notification
{
    if (_isShutdown)
        return;
    
    [self onError:[notification.userInfo objectForKey:@"error"]];
}

- (void)playerItemDidReachEnd:(NSNotification *)notification
{
    if (_isShutdown)
        return;
    
    _isCompleted = YES;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self didPlaybackStateChange];
        [self didLoadStateChange];
        [self setScreenOn:NO];
 
        [[NSNotificationCenter defaultCenter]
         postNotificationName:IJKMPMoviePlayerPlaybackDidFinishNotification
         object:self
         userInfo:@{
                    IJKMPMoviePlayerPlaybackDidFinishReasonUserInfoKey: @(IJKMPMovieFinishReasonPlaybackEnded)
                    }];
    });
}


#pragma mark KVO

- (void)observeValueForKeyPath:(NSString*)path
                      ofObject:(id)object
                        change:(NSDictionary*)change
                       context:(void*)context
{
    if (_isShutdown)
        return;
    
    if (context == KVO_AVPlayerItem_state)
    {
        /* AVPlayerItem "status" property value observer. */
        AVPlayerItemStatus status = [[change objectForKey:NSKeyValueChangeNewKey] integerValue];
        switch (status)
        {
            case AVPlayerItemStatusUnknown:
            {
                /* Indicates that the status of the player is not yet known because
                 it has not tried to load new media resources for playback */
            }
                break;
                
            case AVPlayerItemStatusReadyToPlay:
            {
                /* Once the AVPlayerItem becomes ready to play, i.e.
                 [playerItem status] == AVPlayerItemStatusReadyToPlay,
                 its duration can be fetched from the item. */
                dispatch_once(&_readyToPlayToken, ^{
                    [_avView setPlayer:_player];
                    
                    self.isPreparedToPlay = YES;
                    AVPlayerItem *playerItem = (AVPlayerItem *)object;
                    NSTimeInterval duration = CMTimeGetSeconds(playerItem.duration);
                    if (duration <= 0)
                        self.duration = 0.0f;
                    else
                        self.duration = duration;
                    
                    [[NSNotificationCenter defaultCenter]
                     postNotificationName:IJKMPMediaPlaybackIsPreparedToPlayDidChangeNotification
                     object:self];
                    
                    if (_shouldAutoplay && (!_pauseInBackground || [UIApplication sharedApplication].applicationState == UIApplicationStateActive))
                        [_player play];
                });
            }
                break;
                
            case AVPlayerItemStatusFailed:
            {
                AVPlayerItem *playerItem = (AVPlayerItem *)object;
                [self assetFailedToPrepareForPlayback:playerItem.error];
            }
                break;
        }
        
        [self didPlaybackStateChange];
        [self didLoadStateChange];
    }
    else if (context == KVO_AVPlayerItem_loadedTimeRanges)
    {
        AVPlayerItem *playerItem = (AVPlayerItem *)object;
        if (_player != nil && playerItem.status == AVPlayerItemStatusReadyToPlay) {
            NSArray *timeRangeArray = playerItem.loadedTimeRanges;
            CMTime currentTime = [_player currentTime];
            
            BOOL foundRange = NO;
            CMTimeRange aTimeRange = {0};
            
            if (timeRangeArray.count) {
                aTimeRange = [[timeRangeArray objectAtIndex:0] CMTimeRangeValue];
                if(CMTimeRangeContainsTime(aTimeRange, currentTime)) {
                    foundRange = YES;
                }
            }
            
            if (foundRange) {
                CMTime maxTime = CMTimeRangeGetEnd(aTimeRange);
                NSTimeInterval playableDuration = CMTimeGetSeconds(maxTime);
                if (playableDuration > 0) {
                    self.playableDuration = playableDuration;
                    [self didPlayableDurationUpdate];
                }
            }
        }
        else
        {
            self.playableDuration = 0;
        }
    }
    else if (context == KVO_AVPlayerItem_playbackLikelyToKeepUp) {
        AVPlayerItem *playerItem = (AVPlayerItem *)object;
        NSLog(@"KVO_AVPlayerItem_playbackLikelyToKeepUp: %@\n", playerItem.isPlaybackLikelyToKeepUp ? @"YES" : @"NO");
        [self fetchLoadStateFromItem:playerItem];
        [self didLoadStateChange];
    }
    else if (context == KVO_AVPlayerItem_playbackBufferEmpty) {
        AVPlayerItem *playerItem = (AVPlayerItem *)object;
        BOOL isPlaybackBufferEmpty = playerItem.isPlaybackBufferEmpty;
        NSLog(@"KVO_AVPlayerItem_playbackBufferEmpty: %@\n", isPlaybackBufferEmpty ? @"YES" : @"NO");
        if (isPlaybackBufferEmpty)
            _isPrerolling = YES;
        [self fetchLoadStateFromItem:playerItem];
        [self didLoadStateChange];
    }
    else if (context == KVO_AVPlayerItem_playbackBufferFull) {
        AVPlayerItem *playerItem = (AVPlayerItem *)object;
        NSLog(@"KVO_AVPlayerItem_playbackBufferFull: %@\n", playerItem.isPlaybackBufferFull ? @"YES" : @"NO");
        [self fetchLoadStateFromItem:playerItem];
        [self didLoadStateChange];
    }
    else if (context == KVO_AVPlayer_rate)
    {
        if (_player != nil && !isFloatZero(_player.rate))
            _isPrerolling = NO;
        /* AVPlayer "rate" property value observer. */
        [self didPlaybackStateChange];
        [self didLoadStateChange];
    }
    else if (context == KVO_AVPlayer_currentItem)
    {
        _isPrerolling = NO;
        /* AVPlayer "currentItem" property observer.
         Called when the AVPlayer replaceCurrentItemWithPlayerItem:
         replacement will/did occur. */
        AVPlayerItem *newPlayerItem = [change objectForKey:NSKeyValueChangeNewKey];
        
        /* Is the new player item null? */
        if (newPlayerItem == (id)[NSNull null])
        {
            NSError *error = [self createErrorWithCode:kEC_CurrentPlayerItemIsNil
                                           description:@"current player item is nil"
                                                reason:nil];
            [self assetFailedToPrepareForPlayback:error];
        }
        else /* Replacement of player currentItem has occurred */
        {
            [_avView setPlayer:_player];
            
            [self didPlaybackStateChange];
            [self didLoadStateChange];
        }
    }
    else if (context == KVO_AVPlayer_airplay)
    {
        [[NSNotificationCenter defaultCenter] postNotificationName:IJKMPMoviePlayerIsAirPlayVideoActiveDidChangeNotification object:nil userInfo:nil];
    }
    else
    {
        [super observeValueForKeyPath:path ofObject:object change:change context:context];
    }
}


- (NSError*)createErrorWithCode: (NSInteger)code
                    description: (NSString*)description
                         reason: (NSString*)reason
{
    NSError *error = [IJKMediaUtils createErrorWithDomain:kErrorDomain
                                                     code:code
                                              description:description
                                                   reason:reason];
    return error;
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

-(BOOL)allowsMediaAirPlay
{
    if (!_player)
        return NO;
    return _player.allowsExternalPlayback;
}

-(void)setAllowsMediaAirPlay:(BOOL)b
{
    if (!_player)
        return;
    _player.allowsExternalPlayback = b;
}

-(BOOL)airPlayMediaActive
{
    if (!_player)
        return NO;
    return _player.externalPlaybackActive || self.isDanmakuMediaAirPlay;
}

- (CGSize)naturalSize
{
    if (_playAsset == nil)
        return CGSizeZero;

    NSArray<AVAssetTrack *> *videoTracks = [_playAsset tracksWithMediaType:AVMediaTypeVideo];
    if (videoTracks == nil || videoTracks.count <= 0)
        return CGSizeZero;

    return [videoTracks objectAtIndex:0].naturalSize;
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

- (void)setPauseInBackground:(BOOL)pause
{
    _pauseInBackground = pause;
}

-(BOOL)isDanmakuMediaAirPlay
{
    return _isDanmakuMediaAirPlay;
}

-(void)setIsDanmakuMediaAirPlay:(BOOL)isDanmakuMediaAirPlay
{
    _isDanmakuMediaAirPlay = isDanmakuMediaAirPlay;
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMPMoviePlayerIsAirPlayVideoActiveDidChangeNotification object:nil userInfo:nil];
}

- (void)audioSessionInterrupt:(NSNotification *)notification
{
    int reason = [[[notification userInfo] valueForKey:AVAudioSessionInterruptionTypeKey] intValue];
    switch (reason) {
        case AVAudioSessionInterruptionTypeBegan: {
            NSLog(@"IJKAVMoviePlayerController:audioSessionInterrupt: begin\n");
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
            NSLog(@"IJKAVMoviePlayerController:audioSessionInterrupt: end\n");
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
    NSLog(@"IJKAVMoviePlayerController:applicationWillEnterForeground: %d\n", (int)[UIApplication sharedApplication].applicationState);
}

- (void)applicationDidBecomeActive
{
    NSLog(@"IJKAVMoviePlayerController:applicationDidBecomeActive: %d\n", (int)[UIApplication sharedApplication].applicationState);
    [_avView setPlayer:_player];
}

- (void)applicationWillResignActive
{
    NSLog(@"IJKAVMoviePlayerController:applicationWillResignActive: %d\n", (int)[UIApplication sharedApplication].applicationState);
}

- (void)applicationDidEnterBackground
{
    NSLog(@"IJKAVMoviePlayerController:applicationDidEnterBackground: %d\n", (int)[UIApplication sharedApplication].applicationState);
    if (_pauseInBackground && ![self airPlayMediaActive]) {
        [self pause];
    } else {
        if (![self airPlayMediaActive]) {
            [_avView setPlayer:nil];
            if (isIOS9OrLater()) {
                if ([self isPlaying]) {
                    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                        [self play];
                    });
                }
            }
        }
    }
}

- (void)applicationWillTerminate
{
    NSLog(@"IJKAVMoviePlayerController:applicationWillTerminate: %d\n", (int)[UIApplication sharedApplication].applicationState);
}

@end

/*
 * IJKAVMoviePlayerController.m
 *
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
#import <AVFoundation/AVFoundation.h>

static NSString *kErrorDomain = @"IJKAVMoviePlayer";
static const NSInteger kEC_CurrentPlayerItemIsNil   = 5001;
static const NSInteger kEC_PlayerItemCancelled      = 5002;

static void *KVO_AVPlayer_rate          = &KVO_AVPlayer_rate;
static void *KVO_AVPlayer_currentItem   = &KVO_AVPlayer_currentItem;

static void *KVO_AVPlayerItem_state                     = &KVO_AVPlayerItem_state;
static void *KVO_AVPlayerItem_loadedTimeRanges          = &KVO_AVPlayerItem_loadedTimeRanges;
static void *KVO_AVPlayerItem_playbackLikelyToKeepUp    = &KVO_AVPlayerItem_playbackLikelyToKeepUp;
static void *KVO_AVPlayerItem_playbackBufferFull        = &KVO_AVPlayerItem_playbackBufferFull;
static void *KVO_AVPlayerItem_playbackBufferEmpty       = &KVO_AVPlayerItem_playbackBufferEmpty;

@interface IJKAVMoviePlayerController() <IJKAudioSessionDelegate>

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
    
	id _timeObserver;

    BOOL _isSeeking;
    BOOL _isError;
    BOOL _isCompleted;

    BOOL _pauseInBackground;

    BOOL _playbackLikelyToKeeyUp;
    BOOL _playbackBufferEmpty;
    BOOL _playbackBufferFull;

    NSMutableArray *_registeredNotifications;
}

@synthesize view                        = _view;
@synthesize currentPlaybackTime         = _currentPlaybackTime;
@synthesize duration                    = _duration;
@synthesize playableDuration            = _playableDuration;
@synthesize bufferingProgress           = _bufferingProgress;

@synthesize numberOfBytesTransferred    = _numberOfBytesTransferred;

@synthesize playbackState               = _playbackState;
@synthesize loadState                   = _loadState;

@synthesize controlStyle                = _controlStyle;
@synthesize scalingMode                 = _scalingMode;
@synthesize shouldAutoplay              = _shouldAutoplay;

- (id)initWithContentURL:(NSURL *)aUrl
{
    self = [super init];
    if (self != nil) {
        self.controlStyle = MPMovieControlStyleNone;
        self.scalingMode = MPMovieScalingModeAspectFit;
        self.shouldAutoplay = NO;

        _playUrl = aUrl;

        _avView = [[IJKAVPlayerLayerView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
        [_avView setVideoFillMode:AVLayerVideoGravityResizeAspect];
        self.view = _avView;

        // TODO:
        [[IJKAudioKit sharedInstance] setupAudioSession:self];

        _isSeeking      = NO;
        _isError        = NO;
        _isCompleted    = NO;
        self.bufferingProgress  = -1;

        _playbackLikelyToKeeyUp = NO;
        _playbackBufferEmpty    = YES;
        _playbackBufferFull     = NO;

        // init extra
        [self setScreenOn:YES];

        _registeredNotifications = [[NSMutableArray alloc] init];
        [self registerApplicationObservers];
    }
    return self;
}

- (void)setScreenOn: (BOOL)on
{
    [IJKMediaModule sharedModule].mediaModuleIdleTimerDisabled = on;
    // [UIApplication sharedApplication].idleTimerDisabled = on;
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
	[_player pause];
}

- (void)stop
{
	[_player pause];
    _isCompleted = YES;
}

- (BOOL)isPlaying
{
    return _player.rate >= 0.00001f;
}

- (void)removeObserversFromPlayerItem:(AVPlayerItem*)playerItem
{
    if (playerItem == nil)
        return;

    [IJKMediaUtils kvoQuietlyRemoveObserver:self
                                 forKeyPath:@"status"
                                 fromObject:playerItem];
    [IJKMediaUtils kvoQuietlyRemoveObserver:self
                                 forKeyPath:@"loadedTimeRanges"
                                 fromObject:playerItem];
    [IJKMediaUtils kvoQuietlyRemoveObserver:self
                                 forKeyPath:@"playbackLikelyToKeepUp"
                                 fromObject:playerItem];
    [IJKMediaUtils kvoQuietlyRemoveObserver:self
                                 forKeyPath:@"playbackBufferFull"
                                 fromObject:playerItem];
    [IJKMediaUtils kvoQuietlyRemoveObserver:self
                                 forKeyPath:@"playbackBufferEmpty"
                                 fromObject:playerItem];

    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:nil
                                                  object:playerItem];
}

- (void)removeObserversFromPlayer:(AVPlayer*)player
{
    if (player == nil)
        return;

    [player removeObserver:self forKeyPath:@"currentItem"];
    [player removeObserver:self forKeyPath:@"rate"];
}


- (void)shutdown
{
    [self stop];

    if (_playerItem != nil) {
        [_playerItem cancelPendingSeeks];
    }

    [self removeObserversFromPlayerItem:_playerItem];
    [self unregisterApplicationObservers];

    if (_avView != nil) {
        [_avView setPlayer:nil];
    }

    self.view = nil;
}

- (UIImage *)thumbnailImageAtCurrentTime
{
    return nil;
}

- (void)setCurrentPlaybackTime:(NSTimeInterval)aCurrentPlaybackTime
{
    if (!_player)
        return;

    _isSeeking = YES;

    [self didPlaybackStateChange];
    [self didLoadStateChange];

    [_player seekToTime:CMTimeMakeWithSeconds(aCurrentPlaybackTime, NSEC_PER_SEC)
      completionHandler:^(BOOL finished) {
          dispatch_async(dispatch_get_main_queue(), ^{
              _isSeeking = NO;

              [self didPlaybackStateChange];
              [self didLoadStateChange];
          });
    }];
}

- (NSTimeInterval)currentPlaybackTime
{
    if (!_player)
        return 0.0f;

    return CMTimeGetSeconds([_player currentTime]);
}

-(int64_t)numberOfBytesTransferred
{
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
    return 0;
}

- (MPMoviePlaybackState)playbackState
{
    if (!_player)
        return NO;

    MPMoviePlaybackState mpState = MPMoviePlaybackStateStopped;
    if (_isCompleted) {
        mpState = MPMoviePlaybackStateStopped;
    } else if (_isSeeking) {
        mpState = MPMoviePlaybackStateSeekingForward;
    } else if ([self isPlaying]) {
        mpState = MPMoviePlaybackStatePlaying;
    } else {
        mpState = MPMoviePlaybackStatePaused;
    }

    return mpState;
}

- (MPMovieLoadState)loadState
{
    if (_player == nil)
        return MPMovieLoadStateUnknown;

    AVPlayerItem *playerItem = [_player currentItem];
    if (playerItem == nil)
        return MPMovieLoadStateUnknown;

    if ([playerItem isPlaybackBufferFull]) {
        return MPMovieLoadStatePlayable;
    } else if ([playerItem isPlaybackLikelyToKeepUp]) {
        return MPMovieLoadStatePlaythroughOK;
    } else if ([playerItem isPlaybackBufferEmpty]) {
        return MPMovieLoadStateStalled;
    } else {
        return MPMovieLoadStateUnknown;
    }
}




- (void)didPrepareToPlayAsset:(AVURLAsset *)asset withKeys:(NSArray *)requestedKeys
{
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
    [self removeObserversFromPlayerItem:_playerItem];

    /* Create a new instance of AVPlayerItem from the now successfully loaded AVAsset. */
    _playerItem = [AVPlayerItem playerItemWithAsset:asset];

    /* Observe the player item "status" key to determine when it is ready to play. */
    [_playerItem addObserver:self
                  forKeyPath:@"status"
                     options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                     context:KVO_AVPlayerItem_state];

    [_playerItem addObserver:self
                  forKeyPath:@"loadedTimeRanges"
                     options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                     context:KVO_AVPlayerItem_loadedTimeRanges];

    [_playerItem addObserver:self
                  forKeyPath:@"playbackLikelyToKeepUp"
                     options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                     context:KVO_AVPlayerItem_playbackLikelyToKeepUp];

    [_playerItem addObserver:self
                  forKeyPath:@"playbackBufferEmpty"
                     options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                     context:KVO_AVPlayerItem_playbackBufferEmpty];

    [_playerItem addObserver:self
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

        /* Observe the AVPlayer "currentItem" property to find out when any
         AVPlayer replaceCurrentItemWithPlayerItem: replacement will/did
         occur.*/
        [_player addObserver:self
                      forKeyPath:@"currentItem"
                         options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                         context:KVO_AVPlayer_currentItem];

        /* Observe the AVPlayer "rate" property to update the scrubber control. */
        [_player addObserver:self
                  forKeyPath:@"rate"
                     options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                     context:KVO_AVPlayer_rate];
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
    [[NSNotificationCenter defaultCenter]
     postNotificationName:IJKMoviePlayerPlaybackStateDidChangeNotification
     object:self];
}

- (void)didLoadStateChange
{
    [[NSNotificationCenter defaultCenter]
     postNotificationName:IJKMoviePlayerLoadStateDidChangeNotification
     object:self];
}

- (void)onError:(NSError *)error
{
    _isError = YES;

    __block NSError *blockError = error;

    dispatch_async(dispatch_get_main_queue(), ^{
        [self didPlaybackStateChange];
        [self didLoadStateChange];

        [[NSNotificationCenter defaultCenter]
         postNotificationName:IJKMoviePlayerPlaybackDidFinishNotification
         object:self
         userInfo:@{
                    MPMoviePlayerPlaybackDidFinishReasonUserInfoKey: @(MPMovieFinishReasonPlaybackError),
                    @"error": blockError
                    }];
    });
}

- (void)assetFailedToPrepareForPlayback:(NSError *)error
{
    [self onError:error];
}

- (void)playerItemFailedToPlayToEndTime:(NSNotification *)notification
{
    [self onError:[notification.userInfo objectForKey:@"error"]];
}

- (void)playerItemDidReachEnd:(NSNotification *)notification
{
    _isCompleted = YES;

    dispatch_async(dispatch_get_main_queue(), ^{
        [self didPlaybackStateChange];
        [self didLoadStateChange];

        [[NSNotificationCenter defaultCenter]
         postNotificationName:IJKMoviePlayerPlaybackDidFinishNotification
         object:self
         userInfo:@{
                    MPMoviePlayerPlaybackDidFinishReasonUserInfoKey: @(MPMovieFinishReasonPlaybackEnded)
                    }];
    });
}


#pragma mark KVO

- (void)observeValueForKeyPath:(NSString*)path
                      ofObject:(id)object
                        change:(NSDictionary*)change
                       context:(void*)context
{
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
                [_avView setPlayer:_player];

                self.isPreparedToPlay = YES;
                AVPlayerItem *playerItem = (AVPlayerItem *)object;
                NSTimeInterval duration = CMTimeGetSeconds(playerItem.duration);
                if (duration <= 0)
                    self.duration = 0.0f;
                else
                    self.duration = duration;

                [[NSNotificationCenter defaultCenter]
                 postNotificationName:IJKMediaPlaybackIsPreparedToPlayDidChangeNotification
                 object:self];
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

            __block BOOL foundRange = NO;
            __block CMTimeRange aTimeRange;

            [timeRangeArray enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                aTimeRange = [[timeRangeArray objectAtIndex:0] CMTimeRangeValue];
                if(CMTimeRangeContainsTime(aTimeRange, currentTime)) {
                    *stop = YES;
                    foundRange = YES;
                }
            }];

            if (foundRange) {
                CMTime maxTime = CMTimeRangeGetEnd(aTimeRange);
                NSTimeInterval playableDuration = CMTimeGetSeconds(maxTime);
                if (playableDuration > 0)
                    self.playableDuration = playableDuration;
            }
        }
        else
        {
            self.playableDuration = 0;
        }
    }
    else if (context == KVO_AVPlayerItem_playbackLikelyToKeepUp) {
        AVPlayerItem *playerItem = (AVPlayerItem *)object;
        _playbackLikelyToKeeyUp = playerItem.isPlaybackLikelyToKeepUp;
        [self didLoadStateChange];
    }
    else if (context == KVO_AVPlayerItem_playbackBufferEmpty) {
        AVPlayerItem *playerItem = (AVPlayerItem *)object;
        _playbackBufferEmpty = playerItem.isPlaybackBufferEmpty;
        [self didLoadStateChange];
    }
    else if (context == KVO_AVPlayerItem_playbackBufferFull) {
        AVPlayerItem *playerItem = (AVPlayerItem *)object;
        _playbackBufferFull = playerItem.isPlaybackBufferFull;
        [self didLoadStateChange];
    }
	else if (context == KVO_AVPlayer_rate)
	{
        /* AVPlayer "rate" property value observer. */
        [self didPlaybackStateChange];
        [self didLoadStateChange];
	}
	else if (context == KVO_AVPlayer_currentItem)
	{
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
            [self didPlaybackStateChange];
            [self didLoadStateChange];
        }
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



#pragma mark IJKAudioSessionDelegate

- (void)ijkAudioBeginInterruption
{
    [self pause];
}

- (void)ijkAudioEndInterruption
{
    [self pause];
}

#pragma mark app state changed

- (void)registerApplicationObservers
{
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillEnterForeground)
                                                 name:UIApplicationWillEnterForegroundNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationWillEnterForegroundNotification];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidBecomeActive)
                                                 name:UIApplicationDidBecomeActiveNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationDidBecomeActiveNotification];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillResignActive)
                                                 name:UIApplicationWillResignActiveNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationWillResignActiveNotification];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidEnterBackground)
                                                 name:UIApplicationDidEnterBackgroundNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationDidEnterBackgroundNotification];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillTerminate)
                                                 name:UIApplicationWillTerminateNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationWillTerminateNotification];
}

- (void)unregisterApplicationObservers
{
    for (NSString *name in _registeredNotifications) {
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                                        name:name
                                                      object:nil];
    }
}

- (void)applicationWillEnterForeground
{
    NSLog(@"IJKAVMoviePlayerController:applicationWillEnterForeground: %d", (int)[UIApplication sharedApplication].applicationState);
}

- (void)applicationDidBecomeActive
{
    NSLog(@"IJKAVMoviePlayerController:applicationDidBecomeActive: %d", (int)[UIApplication sharedApplication].applicationState);
}

- (void)applicationWillResignActive
{
    NSLog(@"IJKAVMoviePlayerController:applicationWillResignActive: %d", (int)[UIApplication sharedApplication].applicationState);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_pauseInBackground) {
            [self pause];
        }
    });
}

- (void)applicationDidEnterBackground
{
    NSLog(@"IJKAVMoviePlayerController:applicationDidEnterBackground: %d", (int)[UIApplication sharedApplication].applicationState);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_pauseInBackground) {
            [self pause];
        }
    });
}

- (void)applicationWillTerminate
{
    NSLog(@"IJKAVMoviePlayerController:applicationWillTerminate: %d", (int)[UIApplication sharedApplication].applicationState);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_pauseInBackground) {
            [self pause];
        }
    });
}
                                        
@end

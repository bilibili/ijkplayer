//
//  IJKMoviePlayerController.m
//  IJKMediaDemo
//
//  Created by ZhangRui on 13-9-21.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKMoviePlayerViewController.h"
#import "IJKMediaControl.h"
#import "IJKCommon.h"
#import "IJKMediaPlayer/IJKMediaPlayer.h"

@implementation IJKVideoViewController
{
    IJKMPMovieSourceType mUrlSourceType;
	int mPlaySpeedMode;
    
    NSTimer *timer;
}

@synthesize urlString = _urlString;

- (void)setUrlString:(NSString *)URLString UrlSourceType:(IJKMPMovieSourceType) urlSourceType
{
    _urlString = URLString;
    mUrlSourceType = urlSourceType;
}

- (id)initView
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return [self initWithNibName:@"IJKMoviePlayerViewController" bundle:nil];
    } else {
        return [self initWithNibName:@"IJKMoviePlayerViewController" bundle:nil];
    }
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.

    [[UIApplication sharedApplication] setStatusBarHidden:YES];
    [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationLandscapeLeft animated:NO];
    
    [self.view addSubview:self.mediaControl];
    
    IJKFFOptions * options = [IJKFFOptions optionsByDefault];
    options.reportPlayInfo = YES;
    options.sourceType = mUrlSourceType;
    options.cache = 10000;
    
    [IJKFFMoviePlayerController setLogReport:YES];
//    self.player = [[IJKFFMoviePlayerController alloc] initWithContentToken:self.urlString withOptions:options];
    self.player = [[IJKFFMoviePlayerController alloc] initWithContentURLString:self.urlString withOptions:options withSegmentResolver:nil];
    [self installMovieNotificationObservers];
}

- (void)dealloc
{
    [self removeMovieNotificationObservers];
    if (self.player) {
        [self.player shutdown];
    }
    
    if(timer!=nil)
    {
        [timer invalidate];
        timer = nil;
    }
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation{
    return UIInterfaceOrientationIsLandscape(toInterfaceOrientation);
}

- (NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscape;
}

- (UIInterfaceOrientation)preferredInterfaceOrientationForPresentation{
    return UIInterfaceOrientationLandscapeLeft;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark IBAction

- (IBAction)onClickMediaControl:(id)sender
{
    [self.mediaControl showAndFade];
}

- (IBAction)onClickOverlay:(id)sender
{
    [self.mediaControl hide];
	[self.volume resignFirstResponder];
}

- (IBAction)onClickBack:(id)sender
{
//    exit(0);
    [self dismissModalViewControllerAnimated:YES];
}

- (IBAction)onClickPlay:(id)sender
{
    [self.player play];
    [self.mediaControl refreshMediaControl];
}

- (IBAction)onClickPause:(id)sender
{
    [self.player pause];
    [self.mediaControl refreshMediaControl];
}

- (IBAction)onClickBackPlayREL:(id)sender
{
    [[[UIApplication sharedApplication] keyWindow] endEditing:YES];
    
    [self.player backPlayWithREL:[self.relTimeTextField.text doubleValue]];
}

- (IBAction)onClickBackPlayABS:(id)sender
{
    [[[UIApplication sharedApplication] keyWindow] endEditing:YES];
    
    [self.player backPlayWithABS:[self.startTimeTextField.text longLongValue]];
}

- (IBAction)onClickBackLive:(id)sender
{
    [self.player backLivePlay];
}

- (IBAction)onClickSlower:(id)sender {
	
	if(mPlaySpeedMode <=-2)
		return;
	
	mPlaySpeedMode--;
	[self.player setPlaySpeedMode:mPlaySpeedMode];
}

- (IBAction)onClickFaster:(id)sender {
	if(mPlaySpeedMode >= 2)
		return;
	
	mPlaySpeedMode++;
	[self.player setPlaySpeedMode:mPlaySpeedMode];
}

- (IBAction)onClickNormal:(id)sender {
	mPlaySpeedMode = 0;
	[self.player setPlaySpeedMode:mPlaySpeedMode];
}

- (IBAction)onClickSetVolume:(id)sender {
	int volume;
	volume = [[self.volume text] intValue];
	[self.player setPlayVolume:volume];
}

- (void)loadStateDidChange:(NSNotification*)notification
{
    //    MPMovieLoadStateUnknown        = 0,
    //    MPMovieLoadStatePlayable       = 1 << 0,
    //    MPMovieLoadStatePlaythroughOK  = 1 << 1, // Playback will be automatically started in this state when shouldAutoplay is YES
    //    MPMovieLoadStateStalled        = 1 << 2, // Playback will be automatically paused in this state, if started

    MPMovieLoadState loadState = _player.loadState;

    if ((loadState & MPMovieLoadStatePlaythroughOK) != 0) {
        NSLog(@"loadStateDidChange: MPMovieLoadStatePlaythroughOK: %d\n", (int)loadState);
    } else if ((loadState & MPMovieLoadStateStalled) != 0) {
        NSLog(@"loadStateDidChange: MPMovieLoadStateStalled: %d\n", (int)loadState);
    } else {
        NSLog(@"loadStateDidChange: ???: %d\n", (int)loadState);
    }
}

- (void)moviePlayBackDidFinish:(NSNotification*)notification
{
    //    MPMovieFinishReasonPlaybackEnded,
    //    MPMovieFinishReasonPlaybackError,
    //    MPMovieFinishReasonUserExited
    int reason = [[[notification userInfo] valueForKey:MPMoviePlayerPlaybackDidFinishReasonUserInfoKey] intValue];

    switch (reason)
    {
        case MPMovieFinishReasonPlaybackEnded:
            NSLog(@"playbackStateDidChange: MPMovieFinishReasonPlaybackEnded: %d\n", reason);
            break;

        case MPMovieFinishReasonUserExited:
            NSLog(@"playbackStateDidChange: MPMovieFinishReasonUserExited: %d\n", reason);
            break;

        case MPMovieFinishReasonPlaybackError:
            NSLog(@"playbackStateDidChange: MPMovieFinishReasonPlaybackError: %d\n", reason);
            break;

        default:
            NSLog(@"playbackPlayBackDidFinish: ???: %d\n", reason);
            break;
    }
    
//    [self.player shutdown];
}

- (void)mediaIsPreparedToPlayDidChange:(NSNotification*)notification
{
    NSLog(@"mediaIsPreparedToPlayDidChange\n");
    
//    self.player.view.autoresizingMask = UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
//    self.player.view.frame = self.view.bounds;
    
//    self.view.autoresizesSubviews = YES;
    [self.view insertSubview:self.player.view belowSubview:self.mediaControl];
    
    self.mediaControl.delegatePlayer = self.player;
    
    [self.player play];
    
    timer =  [NSTimer scheduledTimerWithTimeInterval:3.0 target:self selector:@selector(doPrintABTMOnce:) userInfo:nil repeats:YES];
}

- (void)printABTM
{
    NSLog(@"absolute timestamp : %@", [_player absoluteTimeStamp]);
}

- (void)doPrintABTMOnce:(NSTimer*)theTimer
{
    [self performSelectorInBackground:@selector(printABTM) withObject:self];
}

- (void)moviePlayBackStateDidChange:(NSNotification*)notification
{
    //    MPMoviePlaybackStateStopped,
    //    MPMoviePlaybackStatePlaying,
    //    MPMoviePlaybackStatePaused,
    //    MPMoviePlaybackStateInterrupted,
    //    MPMoviePlaybackStateSeekingForward,
    //    MPMoviePlaybackStateSeekingBackward

    switch (_player.playbackState)
    {
        case MPMoviePlaybackStateStopped: {
            NSLog(@"moviePlayBackStateDidChange %d: stoped", (int)_player.playbackState);
            break;
        }
        case MPMoviePlaybackStatePlaying: {
            NSLog(@"moviePlayBackStateDidChange %d: playing", (int)_player.playbackState);
            break;
        }
        case MPMoviePlaybackStatePaused: {
            NSLog(@"moviePlayBackStateDidChange %d: paused", (int)_player.playbackState);
            break;
        }
        case MPMoviePlaybackStateInterrupted: {
            NSLog(@"moviePlayBackStateDidChange %d: interrupted", (int)_player.playbackState);
            break;
        }
        case MPMoviePlaybackStateSeekingForward:
        case MPMoviePlaybackStateSeekingBackward: {
            NSLog(@"moviePlayBackStateDidChange %d: seeking", (int)_player.playbackState);
            break;
        }
        default: {
            NSLog(@"moviePlayBackStateDidChange %d: unknown", (int)_player.playbackState);
            break;
        }
    }
}

#pragma mark Install Movie Notifications

/* Register observers for the various movie object notifications. */
-(void)installMovieNotificationObservers
{
	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(loadStateDidChange:)
                                                 name:IJKMoviePlayerLoadStateDidChangeNotification
                                               object:_player];

	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(moviePlayBackDidFinish:)
                                                 name:IJKMoviePlayerPlaybackDidFinishNotification
                                               object:_player];

	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(mediaIsPreparedToPlayDidChange:)
                                                 name:IJKMediaPlaybackIsPreparedToPlayDidChangeNotification
                                               object:_player];

	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(moviePlayBackStateDidChange:)
                                                 name:IJKMoviePlayerPlaybackStateDidChangeNotification
                                               object:_player];
}

#pragma mark Remove Movie Notification Handlers

/* Remove the movie notification observers from the movie object. */
-(void)removeMovieNotificationObservers
{
    [[NSNotificationCenter defaultCenter]removeObserver:self name:IJKMoviePlayerLoadStateDidChangeNotification object:_player];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:IJKMoviePlayerPlaybackDidFinishNotification object:_player];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:IJKMediaPlaybackIsPreparedToPlayDidChangeNotification object:_player];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:IJKMoviePlayerPlaybackStateDidChangeNotification object:_player];
}

@end

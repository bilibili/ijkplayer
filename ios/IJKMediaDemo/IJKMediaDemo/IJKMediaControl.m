//
//  IJKMediaControl.m
//  IJKMediaDemo
//
//  Created by ZhangRui on 13-9-22.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKMediaControl.h"
#import "IJKMediaPlayer/IJKMediaPlayer.h"

@implementation IJKMediaControl

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
    }
    return self;
}

-(void)sliderValueChanged:(id)sender
{
    if (self.mediaProgressSlider.value<=self.mediaProgressSlider.maximumValue && self.mediaProgressSlider.value>=0) {
        self.delegatePlayer.currentPlaybackTime = self.mediaProgressSlider.value*1.0;
    }
    
    [self refreshMediaControl];
}

-(void)sliderTouchDown:(id)sender
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(refreshMediaControl) object:nil];
}

- (void)awakeFromNib
{
    //    [self.mediaProgressSlider addTarget:self action:@selector(sliderValueChanged:) forControlEvents:UIControlEventValueChanged];
    
    [self.mediaProgressSlider addTarget:self action:@selector(sliderTouchDown:) forControlEvents:UIControlEventTouchDown];
    
    [self.mediaProgressSlider addTarget:self action:@selector(sliderValueChanged:) forControlEvents:UIControlEventTouchUpInside];
    
        [self.mediaProgressSlider addTarget:self action:@selector(sliderValueChanged:) forControlEvents:UIControlEventTouchUpOutside];
    
    [self refreshMediaControl];
}

- (void)showNoFade
{
    self.overlayPanel.hidden = NO;
    [self cancelDelayedHide];
    [self refreshMediaControl];
}

- (void)showAndFade
{
    [self showNoFade];
    [self performSelector:@selector(hide) withObject:nil afterDelay:5];
}

- (void)hide
{
    self.overlayPanel.hidden = YES;
    [self cancelDelayedHide];
}

- (void)cancelDelayedHide
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(hide) object:nil];
}

- (void)refreshMediaControl
{
    NSTimeInterval duration = self.delegatePlayer.duration;
    NSTimeInterval position = self.delegatePlayer.currentPlaybackTime;

    NSInteger intDuration = duration + 0.5;
    NSInteger intPosition = position + 0.5;

    self.currentTimeLabel.text = [NSString stringWithFormat:@"%02d:%02d", (int)(intPosition / 60), (int)(intPosition % 60)];

    if (intDuration > 0) {
        self.totalDurationLabel.text = [NSString stringWithFormat:@"%02d:%02d", (int)(intDuration / 60), (int)(intDuration % 60)];
        self.mediaProgressSlider.value = position;
        self.mediaProgressSlider.maximumValue = duration;
    } else {
        self.totalDurationLabel.text = @"--:--";
        self.mediaProgressSlider.value = 0.0f;
        self.mediaProgressSlider.maximumValue = 1.0f;
    }


    BOOL isPlaying = [self.delegatePlayer isPlaying];
    self.playButton.hidden = isPlaying;
    self.pauseButton.hidden = !isPlaying;


    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(refreshMediaControl) object:nil];
    if (!self.overlayPanel.hidden) {
        [self performSelector:@selector(refreshMediaControl) withObject:nil afterDelay:0.5];
    }
}

#pragma mark IBAction

@end

//
//  IJKMediaControl.h
//  IJKMediaDemo
//
//  Created by ZhangRui on 13-9-22.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface IJKMediaControl : UIControl

- (void)showNoFade;
- (void)showAndFade;
- (void)hide;
- (void)refreshMediaControl;

@property(nonatomic,weak) id<IJKMediaPlayback> delegatePlayer;

@property(nonatomic,strong) IBOutlet UIView *overlayPanel;
@property(nonatomic,strong) IBOutlet UIView *topPanel;
@property(nonatomic,strong) IBOutlet UIView *bottomPanel;

@property(nonatomic,strong) IBOutlet UIButton *playButton;
@property(nonatomic,strong) IBOutlet UIButton *pauseButton;

@property(nonatomic,strong) IBOutlet UILabel *currentTimeLabel;
@property(nonatomic,strong) IBOutlet UILabel *totalDurationLabel;
@property(nonatomic,strong) IBOutlet UISlider *mediaProgressSlider;


@end

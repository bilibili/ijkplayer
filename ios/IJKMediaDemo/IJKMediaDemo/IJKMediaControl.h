//
//  IJKMediaControl.h
//  IJKMediaDemo
//
//  Created by ZhangRui on 13-9-22.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface IJKMediaControl : UIControl

@property(nonatomic,strong) IBOutlet UILabel *currentTime;
@property(nonatomic,strong) IBOutlet UILabel *totalDuration;
@property(nonatomic,strong) IBOutlet UISlider *mediaProgress;

@property(nonatomic,strong) IBOutlet UIView *topPanel;
@property(nonatomic,strong) IBOutlet UIView *bottomPanel;

@end

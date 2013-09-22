//
//  IJKVideoViewController.h
//  IJKMediaDemo
//
//  Created by ZhangRui on 13-9-21.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <UIKit/UIKit.h>

@class MPMoviePlayerViewController;
@class MPMoviePlayerController;

@interface IJKVideoViewController : UIViewController

@property (nonatomic, retain) MPMoviePlayerViewController *videoView;
@property (atomic, retain) MPMoviePlayerController *player;

- (id)initView;

@end

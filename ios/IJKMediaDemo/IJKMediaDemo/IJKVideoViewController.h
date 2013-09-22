//
//  IJKVideoViewController.h
//  IJKMediaDemo
//
//  Created by ZhangRui on 13-9-21.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <IJKMediaPlayer/IJKMediaPlayer.h>

@interface IJKVideoViewController : UIViewController

@property(atomic, retain) id<IJKMediaPlayback> player;

- (id)initView;

@end

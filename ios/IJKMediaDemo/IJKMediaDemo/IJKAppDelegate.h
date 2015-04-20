//
//  IJKAppDelegate.h
//  IJKMediaDemo
//
//  Created by ZhangRui on 13-9-19.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <UIKit/UIKit.h>

//@class IJKVideoViewController;
@class MainViewController;

@interface IJKAppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;
//@property (strong, nonatomic) IJKVideoViewController *viewController;
@property (strong, nonatomic) MainViewController *viewController;

@end

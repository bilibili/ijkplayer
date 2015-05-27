//
//  IJKMoviePlayerController.h
//  IJKMediaDemo
//
//  Created by ZhangRui on 13-9-21.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <IJKMediaPlayer/IJKMediaPlayer.h>
@class IJKMediaControl;

@interface IJKVideoViewController : UIViewController

@property(atomic,strong) NSURL *url;
@property(atomic, retain) id<IJKMediaPlayback> player;

- (id)initWithURL:(NSURL *)url;

+ (void)presentFromViewController:(UIViewController *)viewController withTitle:(NSString *)title URL:(NSURL *)url completion:(void(^)())completion;

- (IBAction)onClickMediaControl:(id)sender;
- (IBAction)onClickOverlay:(id)sender;
- (IBAction)onClickBack:(id)sender;
- (IBAction)onClickPlay:(id)sender;
- (IBAction)onClickPause:(id)sender;

@property(nonatomic,strong) IBOutlet IJKMediaControl *mediaControl;

@end

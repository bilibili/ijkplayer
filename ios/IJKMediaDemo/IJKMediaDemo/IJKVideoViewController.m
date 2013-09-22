//
//  IJKVideoViewController.m
//  IJKMediaDemo
//
//  Created by ZhangRui on 13-9-21.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKVideoViewController.h"
#import "IJKMediaPlayer/IJKMediaPlayer.h"

@interface IJKVideoViewController ()

@end

@implementation IJKVideoViewController

- (id)initView
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return [self initWithNibName:@"IJKVideoViewController" bundle:nil];
    } else {
        return [self initWithNibName:@"IJKVideoViewController" bundle:nil];
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

    NSURL *theMovieURL = [NSURL URLWithString:@"http://devimages.apple.com/iphone/samples/bipbop/gear1/prog_index.m3u8"];

    self.player = [[IJKMPMoviePlayerController alloc] initWithContentURL:theMovieURL];
    [self.player prepareToPlay];

    [self.player.view setFrame: self.view.bounds];
    [self.view addSubview: self.player.view];

    [self.player play];
}

- (NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscape;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end

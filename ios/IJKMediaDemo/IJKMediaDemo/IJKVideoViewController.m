//
//  IJKVideoViewController.m
//  IJKMediaDemo
//
//  Created by ZhangRui on 13-9-21.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKVideoViewController.h"
#import "IJKMediaControl.h"
#import "IJKCommon.h"

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
    [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationLandscapeLeft animated:NO];

    NSURL *theMovieURL = [NSURL URLWithString:@"http://devimages.apple.com/iphone/samples/bipbop/gear1/prog_index.m3u8"];
    self.player = [[IJKMPMoviePlayerController alloc] initWithContentURL:theMovieURL];

    [self.view addSubview:self.player.view];
    [self.view addSubview:self.mediaControl];

    self.mediaControl.delegatePlayer = self.player;

    [self.player prepareToPlay];
    [self.player.view setFrame: self.view.bounds];
    [self.player play];
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
}

- (IBAction)onClickBack:(id)sender
{
    exit(0);
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

@end

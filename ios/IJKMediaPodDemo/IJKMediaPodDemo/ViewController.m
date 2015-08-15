//
//  ViewController.m
//  IJKMediaPodDemo
//
//  Created by Zhang Rui on 15/7/23.
//  Copyright (c) 2015年 Zhang Rui. All rights reserved.
//

#import "ViewController.h"
#import <IJKMediaPlayer/IJKMediaPlayer.h>

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.

    id<IJKMediaPlayback> playback = [[IJKFFMoviePlayerController alloc] initWithContentURL:nil  withOptions:nil];

    [playback shutdown];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end

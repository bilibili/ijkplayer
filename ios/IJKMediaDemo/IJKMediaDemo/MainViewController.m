//
//  MainViewController.m
//  IJKMediaDemo
//
//  Created by 施灵凯 on 14/12/22.
//  Copyright (c) 2014年 bilibili. All rights reserved.
//

#import "MainViewController.h"
#import "iJKMoviePlayerViewController.h"

@interface MainViewController ()

@end

@implementation MainViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

#pragma mark - Buttons Action

-(IBAction)playWSButtonAction:(id)sender
{
    IJKVideoViewController *viewController;
    viewController = [[IJKVideoViewController alloc] initView];
    [viewController setUrlString:self.wsUrlTextField.text UrlSourceType:IJKMPMovieSourceTypeLowDelayLiveStreaming];
    [self presentModalViewController:viewController animated:YES];
}

-(IBAction)playLXButtonAction:(id)sender
{
    IJKVideoViewController *viewController;
    viewController = [[IJKVideoViewController alloc] initView];
    [viewController setUrlString:self.lxUrlTextField.text UrlSourceType:IJKMPMovieSourceTypeOnDemandStreaming];
    [self presentModalViewController:viewController animated:YES];
}

-(IBAction)playDLButtonAction:(id)sender
{
    IJKVideoViewController *viewController;
    viewController = [[IJKVideoViewController alloc] initView];
    [viewController setUrlString:self.dlUrlTextField.text UrlSourceType:IJKMPMovieSourceTypeHighDelayLiveStreaming];
    [self presentModalViewController:viewController animated:YES];
}

-(IBAction)playAVPLayerButtonAction:(id)sender
{
    IJKVideoViewController *viewController;
    viewController = [[IJKVideoViewController alloc] initView];
    [viewController setUrlString:self.avplayerUrlTextField.text UrlSourceType:IJKMPMovieSourceTypeUnknown];
    [viewController setLocalAVPlayer:YES];
    [self presentModalViewController:viewController animated:YES];
}

@end

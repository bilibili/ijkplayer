//
//  MainViewController.h
//  IJKMediaDemo
//
//  Created by 施灵凯 on 14/12/22.
//  Copyright (c) 2014年 bilibili. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface MainViewController : UIViewController

@property (nonatomic, strong) IBOutlet UITextField *wsUrlTextField;
@property (nonatomic, strong) IBOutlet UITextField *lxUrlTextField;
@property (nonatomic, strong) IBOutlet UITextField *dlUrlTextField;

-(IBAction)playWSButtonAction:(id)sender;
-(IBAction)playLXButtonAction:(id)sender;
-(IBAction)playDLButtonAction:(id)sender;

@end

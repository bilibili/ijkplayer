//
//  SampleViewController.h
//  Part of the ASIHTTPRequest sample project - see http://allseeing-i.com/ASIHTTPRequest for details
//
//  Created by Ben Copsey on 17/06/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface SampleViewController : UIViewController <UITableViewDelegate> {
	UINavigationBar *navigationBar;
	UITableView *tableView;
}

- (void)showNavigationButton:(UIBarButtonItem *)button;
- (void)hideNavigationButton:(UIBarButtonItem *)button;

@property (retain, nonatomic) IBOutlet UINavigationBar *navigationBar;
@property (retain, nonatomic) IBOutlet UITableView *tableView;
@end

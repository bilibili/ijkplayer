//
//  SampleViewController.m
//  Part of the ASIHTTPRequest sample project - see http://allseeing-i.com/ASIHTTPRequest for details
//
//  Created by Ben Copsey on 17/06/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "SampleViewController.h"

// Private stuff
@interface SampleViewController ()
- (void)keyboardWillShow:(NSNotification *)notification;
- (void)keyboardWillHide:(NSNotification *)notification;
@end



@implementation SampleViewController

- (void)showNavigationButton:(UIBarButtonItem *)button
{
    [[[self navigationBar] topItem] setLeftBarButtonItem:button animated:NO];	
}

- (void)hideNavigationButton:(UIBarButtonItem *)button
{
    [[[self navigationBar] topItem] setLeftBarButtonItem:nil animated:NO];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation 
{
	return YES;
}

- (NSIndexPath *)tableView:(UITableView *)theTableView willSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	return nil;
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
	[[self tableView] reloadData];
}

- (void)viewDidLoad
{
	[[self view] setAutoresizingMask:UIViewAutoresizingFlexibleHeight|UIViewAutoresizingFlexibleWidth];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];
}

- (void)viewDidUnload {
    [super viewDidUnload];
    [self setNavigationBar:nil];
	[self setTableView:nil];
}

- (void)keyboardWillShow:(NSNotification *)notification
{
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_3_2
	NSValue *keyboardBoundsValue = [[notification userInfo] objectForKey:UIKeyboardFrameEndUserInfoKey];
#else
	NSValue *keyboardBoundsValue = [[notification userInfo] objectForKey:UIKeyboardBoundsUserInfoKey];
#endif
	CGRect keyboardBounds;
	[keyboardBoundsValue getValue:&keyboardBounds];
	UIEdgeInsets e = UIEdgeInsetsMake(0, 0, keyboardBounds.size.height-42, 0);
	[[self tableView] setScrollIndicatorInsets:e];
	[[self tableView] setContentInset:e];
}

- (void)keyboardWillHide:(NSNotification *)notification
{
	UIEdgeInsets e = UIEdgeInsetsMake(0, 0, 0, 0);
	[[self tableView] setScrollIndicatorInsets:e];
	[[self tableView] setContentInset:e];	
}

- (void)dealloc {
	[navigationBar release];
	[tableView release];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:UIKeyboardWillShowNotification object:nil];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:UIKeyboardWillHideNotification object:nil];
    [super dealloc];
}

@synthesize navigationBar;
@synthesize tableView;
@end

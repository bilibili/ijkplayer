//
//  RootViewController.m
//  Part of the ASIHTTPRequest sample project - see http://allseeing-i.com/ASIHTTPRequest for details
//
//  Created by Ben Copsey on 16/06/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "RootViewController.h"
#import "SynchronousViewController.h"
#import "QueueViewController.h"
#import "AuthenticationViewController.h"
#import "UploadViewController.h"
#import "WebPageViewController.h"

@implementation RootViewController


- (void)viewDidLoad
{
    [super viewDidLoad];
    self.contentSizeForViewInPopover = CGSizeMake(310.0f, self.tableView.rowHeight*5.0f);
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"MyCell"];
	if (!cell) {
		cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"MyCell"] autorelease];
	}
	switch ([indexPath row]) {
		case 0:
			[[cell textLabel] setText:@"Synchronous"];
			break;
		case 1:
			[[cell textLabel] setText:@"Queue"];
			break;
		case 2:
			[[cell textLabel] setText:@"Authentication"];
			break;
		case 3:
			[[cell textLabel] setText:@"Upload"];
			break;
		case 4:
			[[cell textLabel] setText:@"Web Page Download"];
			break;
	}
	return cell;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	return 5;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	UIViewController *viewController = nil;
	switch ([indexPath row]) {
		case 0:
			viewController = [[[SynchronousViewController alloc] initWithNibName:@"Sample" bundle:nil] autorelease];
			break;
		case 1:
			viewController = [[[QueueViewController alloc] initWithNibName:@"Sample" bundle:nil] autorelease];
			break;
		case 2:
			viewController = [[[AuthenticationViewController alloc] initWithNibName:@"Sample" bundle:nil] autorelease];
			break;
		case 3:
			viewController = [[[UploadViewController alloc] initWithNibName:@"Sample" bundle:nil] autorelease];
			break;
		case 4:
			viewController = [[[WebPageViewController alloc] initWithNibName:@"Sample" bundle:nil] autorelease];
			break;
	}	
	[splitViewController setViewControllers:[NSArray arrayWithObjects:[self navigationController],viewController,nil]];
	
	// Dismiss the popover if it's present.
    if ([self popoverController]) {
        [[self popoverController] dismissPopoverAnimated:YES];
    }
	
    // Configure the new view controller's popover button (after the view has been displayed and its toolbar/navigation bar has been created).
    if ([self rootPopoverButtonItem]) {
        [[[(id)viewController navigationBar] topItem] setLeftBarButtonItem:[self rootPopoverButtonItem] animated:NO];
    }
	
}

- (void)splitViewController:(UISplitViewController*)svc willHideViewController:(UIViewController *)aViewController withBarButtonItem:(UIBarButtonItem*)barButtonItem forPopoverController:(UIPopoverController*)pc
{
	[barButtonItem setTitle:@"More Examples"];
	[self setPopoverController:pc];
	[self setRootPopoverButtonItem:barButtonItem];
	SampleViewController *detailViewController = [[splitViewController viewControllers] objectAtIndex:1];
    [detailViewController showNavigationButton:barButtonItem];
}

- (void)splitViewController:(UISplitViewController*)svc willShowViewController:(UIViewController *)aViewController invalidatingBarButtonItem:(UIBarButtonItem *)barButtonItem
{
    [self setPopoverController:nil];
	[self setRootPopoverButtonItem:nil];
	SampleViewController *detailViewController = [[splitViewController viewControllers] objectAtIndex:1];
    [detailViewController hideNavigationButton:barButtonItem];
}

- (void)splitViewController:(UISplitViewController *)svc popoverController: (UIPopoverController *)pc willPresentViewController: (UIViewController *)aViewController
{
    if (pc != nil) {
        [pc dismissPopoverAnimated:YES];
    }
}

- (void)dealloc
{
    [popoverController release];
    [rootPopoverButtonItem release];
    [super dealloc];
}

@synthesize splitViewController;
@synthesize popoverController;
@synthesize rootPopoverButtonItem;
@end

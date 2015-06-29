//
//  SynchronousViewController.m
//  Part of the ASIHTTPRequest sample project - see http://allseeing-i.com/ASIHTTPRequest for details
//
//  Created by Ben Copsey on 07/11/2008.
//  Copyright 2008 All-Seeing Interactive. All rights reserved.
//

#import "SynchronousViewController.h"
#import "ASIHTTPRequest.h"
#import "DetailCell.h"
#import "InfoCell.h"

@implementation SynchronousViewController


// Runs a request synchronously
- (IBAction)simpleURLFetch:(id)sender
{
	NSURL *url = [NSURL URLWithString:[urlField text]];

	// Create a request
	// You don't normally need to retain a synchronous request, but we need to in this case because we'll need it later if we reload the table data
	[self setRequest:[ASIHTTPRequest requestWithURL:url]];
	
	//Customise our user agent, for no real reason
	[request addRequestHeader:@"User-Agent" value:@"ASIHTTPRequest"];

	// Start the request
	[request startSynchronous];
	
	// Request has now finished
	[[self tableView] reloadData];

}

/*
Most of the code below here relates to the table view, and isn't that interesting
*/

- (void)viewDidLoad
{
	[super viewDidLoad];
	[[[self navigationBar] topItem] setTitle:@"Synchronous Requests"];

}

- (void)dealloc
{
	[request cancel];
	[request release];
	[super dealloc];
}



static NSString *intro = @"Demonstrates fetching a web page synchronously, the HTML source will appear in the box below when the download is complete.  The interface will lock up when you press this button until the operation times out or succeeds. You should avoid using synchronous requests on the main thread, even for the simplest operations.";

- (UITableViewCell *)tableView:(UITableView *)theTableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	int tablePadding = 40;
	int tableWidth = [tableView frame].size.width;
	if (tableWidth > 480) { // iPad
		tablePadding = 110;
	}
	
	UITableViewCell *cell;
	if ([indexPath section] == 0) {
		cell = [tableView dequeueReusableCellWithIdentifier:@"InfoCell"];
		if (!cell) {
			cell = [InfoCell cell];	
		}
		[[cell textLabel] setText:intro];
		[cell layoutSubviews];
		
	} else if ([indexPath section] == 1) {
		cell = [tableView dequeueReusableCellWithIdentifier:@"URLCell"];
		if (!cell) {
			cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"URLCell"] autorelease];
			urlField = [[[UITextField alloc] initWithFrame:CGRectZero] autorelease];
			[[cell contentView] addSubview:urlField];	
			goButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
			[goButton setTitle:@"Go!" forState:UIControlStateNormal];
			[goButton addTarget:self action:@selector(simpleURLFetch:) forControlEvents:UIControlEventTouchUpInside];
			[[cell contentView] addSubview:goButton];
		}
		[goButton setFrame:CGRectMake(tableWidth-tablePadding-38,7,20,20)];
		[goButton sizeToFit];
		[urlField setFrame:CGRectMake(10,12,tableWidth-tablePadding-50,20)];
		if ([self request]) {
			[urlField setText:[[[self request] url] absoluteString]];
		} else {
			[urlField setText:@"http://allseeing-i.com"];
		}
		
		
	} else if ([indexPath section] == 2) {
		cell = [tableView dequeueReusableCellWithIdentifier:@"ResponseCell"];
		if (!cell) {
			cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"ResponseCell"] autorelease];
			responseField = [[[UITextView alloc] initWithFrame:CGRectZero] autorelease];
			[responseField setBackgroundColor:[UIColor clearColor]];
			[[cell contentView] addSubview:responseField];
		}
		[responseField setFrame:CGRectMake(5,5,tableWidth-tablePadding,150)];
		if (request) {
			if ([request error]) {
				[responseField setText:[[request error] localizedDescription]];
			} else if ([request responseString]) {
				[responseField setText:[request responseString]];
			}
		}
		
	} else {
		cell = [tableView dequeueReusableCellWithIdentifier:@"HeaderCell"];
		if (!cell) {
			cell = [DetailCell cell];
		}
		NSString *key = [[[request responseHeaders] allKeys] objectAtIndex:[indexPath row]];
		[[cell textLabel] setText:key];
		[[cell detailTextLabel] setText:[[request responseHeaders] objectForKey:key]];
	}
	[cell setSelectionStyle:UITableViewCellSelectionStyleNone];
	return cell;
}

- (NSInteger)tableView:(UITableView *)theTableView numberOfRowsInSection:(NSInteger)section
{
	if (section == 3) {
		return [[request responseHeaders] count];
	} else {
		return 1;
	}
}

- (CGFloat)tableView:(UITableView *)theTableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	if ([indexPath section] == 0) {
		return [InfoCell neededHeightForDescription:intro withTableWidth:[tableView frame].size.width]+20;
	} else if ([indexPath section] == 1) {
		return 48;
	} else if ([indexPath section] == 2) {
		return 160;
	} else {
		return 34;
	}
}

- (NSString *)tableView:(UITableView *)theTableView titleForHeaderInSection:(NSInteger)section
{
	switch (section) {
		case 0:
			return nil;
		case 1:
			return @"Enter a URL";
		case 2:
			return @"Response";
		case 3:
			return @"Response Headers";
	}
	return nil;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
	if ([self request]) {
		return 4;
	} else {
		return 2;
	}
}


@synthesize request;

@end

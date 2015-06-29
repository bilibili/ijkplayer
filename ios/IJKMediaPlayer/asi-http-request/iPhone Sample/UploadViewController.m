//
//  UploadViewController.m
//  Part of the ASIHTTPRequest sample project - see http://allseeing-i.com/ASIHTTPRequest for details
//
//  Created by Ben Copsey on 31/12/2008.
//  Copyright 2008 All-Seeing Interactive. All rights reserved.
//

#import "UploadViewController.h"
#import "ASIFormDataRequest.h"
#import "InfoCell.h"

// Private stuff
@interface UploadViewController ()
- (void)uploadFailed:(ASIHTTPRequest *)theRequest;
- (void)uploadFinished:(ASIHTTPRequest *)theRequest;
@end

@implementation UploadViewController

- (IBAction)performLargeUpload:(id)sender
{
	[request cancel];
	[self setRequest:[ASIFormDataRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ignore"]]];
	[request setPostValue:@"test" forKey:@"value1"];
	[request setPostValue:@"test" forKey:@"value2"];
	[request setPostValue:@"test" forKey:@"value3"];
	[request setTimeOutSeconds:20];

	#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_4_0
	[request setShouldContinueWhenAppEntersBackground:YES];
	#endif
	[request setUploadProgressDelegate:progressIndicator];
	[request setDelegate:self];
	[request setDidFailSelector:@selector(uploadFailed:)];
	[request setDidFinishSelector:@selector(uploadFinished:)];
	
	//Create a 256KB file
	NSData *data = [[[NSMutableData alloc] initWithLength:256*1024] autorelease];
	NSString *path = [[NSHomeDirectory() stringByAppendingPathComponent:@"Documents"] stringByAppendingPathComponent:@"file"];
	[data writeToFile:path atomically:NO];
	
	//Add the file 8 times to the request, for a total request size around 2MB
	int i;
	for (i=0; i<8; i++) {
		[request setFile:path forKey:[NSString stringWithFormat:@"file-%i",i]];
	}
	
	[request startAsynchronous];
	[resultView setText:@"Uploading data..."];
}

- (IBAction)toggleThrottling:(id)sender
{
	[ASIHTTPRequest setShouldThrottleBandwidthForWWAN:[(UISwitch *)sender isOn]];
}

- (void)uploadFailed:(ASIHTTPRequest *)theRequest
{
	[resultView setText:[NSString stringWithFormat:@"Request failed:\r\n%@",[[theRequest error] localizedDescription]]];
}

- (void)uploadFinished:(ASIHTTPRequest *)theRequest
{
	[resultView setText:[NSString stringWithFormat:@"Finished uploading %llu bytes of data",[theRequest postLength]]];

	#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_4_0
    // Clear out the old notification before scheduling a new one.
    if ([[[UIApplication sharedApplication] scheduledLocalNotifications] count] > 0)
        [[UIApplication sharedApplication] cancelAllLocalNotifications];

    // Create a new notification
    UILocalNotification *notification = [[[UILocalNotification alloc] init] autorelease];
    if (notification) {
		[notification setFireDate:[NSDate date]];
		[notification setTimeZone:[NSTimeZone defaultTimeZone]];
		[notification setRepeatInterval:0];
		[notification setSoundName:@"alarmsound.caf"];
		[notification setAlertBody:@"Boom!\r\n\r\nUpload is finished!"];
        [[UIApplication sharedApplication] scheduleLocalNotification:notification];
    }
	#endif
}

- (void)dealloc
{
	[request setDelegate:nil];
	[request setUploadProgressDelegate:nil];
	[request cancel];
	[request release];
	[progressIndicator release];
	[resultView release];
    [super dealloc];
}

/*
 Most of the code below here relates to the table view, and isn't that interesting
 */

- (void)viewDidLoad
{
	[super viewDidLoad];
	[[[self navigationBar] topItem] setTitle:@"Tracking Upload Progress"];
	resultView = [[UITextView alloc] initWithFrame:CGRectZero];
	[resultView setBackgroundColor:[UIColor clearColor]];
	progressIndicator = [[UIProgressView alloc] initWithFrame:CGRectZero];
}

static NSString *intro = @"Demonstrates POSTing content to a URL, showing upload progress.\nYou'll only see accurate progress for uploads when the request body is larger than 128KB (in 2.2.1 SDK), or when the request body is larger than 32KB (in 3.0 SDK)\n\nThis request is also setup to run when the app enters the background on devices running on iOS4. In the delegate method that is called when the request finishes, we show a local notification to let the user know the upload is finished.";

- (UIView *)tableView:(UITableView *)theTableView viewForHeaderInSection:(NSInteger)section
{
	if (section == 1) {
		int tablePadding = 40;
		int tableWidth = [tableView frame].size.width;
		if (tableWidth > 480) { // iPad
			tablePadding = 110;
		}
		
		UIView *view = [[[UIView alloc] initWithFrame:CGRectMake(0,0,tableWidth-(tablePadding/2),30)] autorelease];
		UIButton *goButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
		[goButton setTitle:@"Go!" forState:UIControlStateNormal];
		[goButton sizeToFit];
		[goButton setFrame:CGRectMake([view frame].size.width-[goButton frame].size.width+10,7,[goButton frame].size.width,[goButton frame].size.height)];
		
		[goButton addTarget:self action:@selector(performLargeUpload:) forControlEvents:UIControlEventTouchUpInside];
		[view addSubview:goButton];
		[progressIndicator setFrame:CGRectMake((tablePadding/2)-10,20,200,10)];
		[view addSubview:progressIndicator];
		
		return view;
	}
	return nil;
}

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
		return nil;
	} else {
		cell = [tableView dequeueReusableCellWithIdentifier:@"Response"];
		if (!cell) {
			cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"Response"] autorelease];
			[[cell contentView] addSubview:resultView];
		}	
		[resultView setFrame:CGRectMake(5,5,tableWidth-tablePadding,60)];
	}
	[cell setSelectionStyle:UITableViewCellSelectionStyleNone];

	return cell;
}

- (NSInteger)tableView:(UITableView *)theTableView numberOfRowsInSection:(NSInteger)section
{
	if (section == 1) {
		return 0;
	}
	return 1;
}

- (CGFloat)tableView:(UITableView *)theTableView heightForHeaderInSection:(NSInteger)section
{
	if (section == 1) {
		return 50;
	}
	return 34;
}

- (CGFloat)tableView:(UITableView *)theTableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	if ([indexPath section] == 0) {
		return [InfoCell neededHeightForDescription:intro withTableWidth:[tableView frame].size.width]+20;
	} else if ([indexPath section] == 2) {
		return 80;
	} else {
		return 42;
	}
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
	return 3;
}

@synthesize request;
@end

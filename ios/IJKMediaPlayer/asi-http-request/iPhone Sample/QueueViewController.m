//
//  QueueViewController.m
//  Part of the ASIHTTPRequest sample project - see http://allseeing-i.com/ASIHTTPRequest for details
//
//  Created by Ben Copsey on 07/11/2008.
//  Copyright 2008 All-Seeing Interactive. All rights reserved.
//

#import "QueueViewController.h"
#import "ASIHTTPRequest.h"
#import "ASINetworkQueue.h"
#import "InfoCell.h"
#import "ToggleCell.h"

// Private stuff
@interface QueueViewController ()
- (void)imageFetchComplete:(ASIHTTPRequest *)request;
- (void)imageFetchFailed:(ASIHTTPRequest *)request;
@end

@implementation QueueViewController

- (IBAction)fetchThreeImages:(id)sender
{
	[imageView1 setImage:nil];
	[imageView2 setImage:nil];
	[imageView3 setImage:nil];
	
	if (!networkQueue) {
		networkQueue = [[ASINetworkQueue alloc] init];	
	}
	failed = NO;
	[networkQueue reset];
	[networkQueue setDownloadProgressDelegate:progressIndicator];
	[networkQueue setRequestDidFinishSelector:@selector(imageFetchComplete:)];
	[networkQueue setRequestDidFailSelector:@selector(imageFetchFailed:)];
	[networkQueue setShowAccurateProgress:[accurateProgress isOn]];
	[networkQueue setDelegate:self];
	
	ASIHTTPRequest *request;
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/images/small-image.jpg"]];
	[request setDownloadDestinationPath:[[NSHomeDirectory() stringByAppendingPathComponent:@"Documents"] stringByAppendingPathComponent:@"1.png"]];
	[request setDownloadProgressDelegate:imageProgressIndicator1];
    [request setUserInfo:[NSDictionary dictionaryWithObject:@"request1" forKey:@"name"]];
	[networkQueue addOperation:request];
	
	request = [[[ASIHTTPRequest alloc] initWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/images/medium-image.jpg"]] autorelease];
	[request setDownloadDestinationPath:[[NSHomeDirectory() stringByAppendingPathComponent:@"Documents"] stringByAppendingPathComponent:@"2.png"]];
	[request setDownloadProgressDelegate:imageProgressIndicator2];
    [request setUserInfo:[NSDictionary dictionaryWithObject:@"request2" forKey:@"name"]];
	[networkQueue addOperation:request];
	
	request = [[[ASIHTTPRequest alloc] initWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/images/large-image.jpg"]] autorelease];
	[request setDownloadDestinationPath:[[NSHomeDirectory() stringByAppendingPathComponent:@"Documents"] stringByAppendingPathComponent:@"3.png"]];
	[request setDownloadProgressDelegate:imageProgressIndicator3];
    [request setUserInfo:[NSDictionary dictionaryWithObject:@"request3" forKey:@"name"]];
	[networkQueue addOperation:request];
	
	[networkQueue go];
}


- (void)imageFetchComplete:(ASIHTTPRequest *)request
{
	UIImage *img = [UIImage imageWithContentsOfFile:[request downloadDestinationPath]];
	if (img) {
		if ([imageView1 image]) {
			if ([imageView2 image]) {
				[imageView3 setImage:img];
			} else {
				[imageView2 setImage:img];
			}
		} else {
			[imageView1 setImage:img];
		}
	}
}

- (void)imageFetchFailed:(ASIHTTPRequest *)request
{
	if (!failed) {
		if ([[request error] domain] != NetworkRequestErrorDomain || [[request error] code] != ASIRequestCancelledErrorType) {
			UIAlertView *alertView = [[[UIAlertView alloc] initWithTitle:@"Download failed" message:@"Failed to download images" delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil] autorelease];
			[alertView show];
		}
		failed = YES;
	}
}

- (void)dealloc {
	[progressIndicator release];
	[networkQueue reset];
	[networkQueue release];
    [super dealloc];
}


/*
 Most of the code below here relates to the table view, and isn't that interesting
 */

- (void)viewDidLoad
{
	[super viewDidLoad];
	[[[self navigationBar] topItem] setTitle:@"Using a Queue"];
}

static NSString *intro = @"Demonstrates a fetching 3 items at once, using an ASINetworkQueue to track progress.\r\nEach request has its own downloadProgressDelegate, and the queue has an additional downloadProgressDelegate to track overall progress.";

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
		
		[goButton addTarget:self action:@selector(fetchThreeImages:) forControlEvents:UIControlEventTouchUpInside];
		[view addSubview:goButton];
		
		if (!progressIndicator) {
			progressIndicator = [[UIProgressView alloc] initWithFrame:CGRectMake((tablePadding/2)-10,20,200,10)];
		} else {
			[progressIndicator setFrame:CGRectMake((tablePadding/2)-10,20,200,10)];
		}
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
		cell = [tableView dequeueReusableCellWithIdentifier:@"ToggleCell"];
		if (!cell) {
			cell = [ToggleCell cell];
		}	
		[[cell textLabel] setText:@"Show Accurate Progress"];
		accurateProgress = [(ToggleCell *)cell toggle];
		
	} else {
		
		cell = [tableView dequeueReusableCellWithIdentifier:@"ImagesCell"];
		
		
		if (!cell) {
			
			cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"ImagesCell"] autorelease];

			imageView1 = [[[UIImageView alloc] initWithFrame:CGRectZero] autorelease];
			[imageView1 setBackgroundColor:[UIColor grayColor]];
			[cell addSubview:imageView1];
			
			imageProgressIndicator1 = [[[UIProgressView alloc] initWithFrame:CGRectZero] autorelease];
			[cell addSubview:imageProgressIndicator1];
			
			imageLabel1 = [[[UILabel alloc] initWithFrame:CGRectZero] autorelease];
			if (tableWidth > 480) {
				[imageLabel1 setText:@"This image is 15KB in size"];
			} else {
				[imageLabel1 setText:@"Img size: 15KB"];
			}
			[imageLabel1 setTextAlignment:UITextAlignmentCenter];
			[imageLabel1 setFont:[UIFont systemFontOfSize:11]];
			[imageLabel1 setBackgroundColor:[UIColor clearColor]];
			[cell addSubview:imageLabel1];
			
			imageView2 = [[[UIImageView alloc] initWithFrame:CGRectZero] autorelease];
			[imageView2 setBackgroundColor:[UIColor grayColor]];
			[cell addSubview:imageView2];
			
			imageProgressIndicator2 = [[[UIProgressView alloc] initWithFrame:CGRectZero] autorelease];
			[cell addSubview:imageProgressIndicator2];
			
			imageLabel2 = [[[UILabel alloc] initWithFrame:CGRectZero] autorelease];
			if (tableWidth > 480) {
				[imageLabel2 setText:@"This image is 176KB in size"];
			} else {
				[imageLabel2 setText:@"Img size: 176KB"];
			}
			[imageLabel2 setTextAlignment:UITextAlignmentCenter];
			[imageLabel2 setFont:[UIFont systemFontOfSize:11]];
			[imageLabel2 setBackgroundColor:[UIColor clearColor]];
			[cell addSubview:imageLabel2];
			
			imageView3 = [[[UIImageView alloc] initWithFrame:CGRectZero] autorelease];
			[imageView3 setBackgroundColor:[UIColor grayColor]];
			[cell addSubview:imageView3];
			
			imageProgressIndicator3 = [[[UIProgressView alloc] initWithFrame:CGRectZero] autorelease];
			[cell addSubview:imageProgressIndicator3];
			
			imageLabel3 = [[[UILabel alloc] initWithFrame:CGRectZero] autorelease];
			if (tableWidth > 480) {
				[imageLabel3 setText:@"This image is 1.4MB in size"];
			} else {
				[imageLabel3 setText:@"Img size: 1.4MB"];
			}
			[imageLabel3 setTextAlignment:UITextAlignmentCenter];
			[imageLabel3 setFont:[UIFont systemFontOfSize:11]];
			[imageLabel3 setBackgroundColor:[UIColor clearColor]];
			[cell addSubview:imageLabel3];
			
		}
		NSUInteger imageWidth = (tableWidth-tablePadding-20)/3;
		NSUInteger imageHeight = imageWidth*0.66f;
		
		
		[imageView1 setFrame:CGRectMake(tablePadding/2,10,imageWidth,imageHeight)];
		[imageProgressIndicator1 setFrame:CGRectMake(tablePadding/2,15+imageHeight,imageWidth,20)];
		[imageLabel1 setFrame:CGRectMake(tablePadding/2,25+imageHeight,imageWidth,20)];
		
		[imageView2 setFrame:CGRectMake((tablePadding/2)+imageWidth+10,10,imageWidth,imageHeight)];
		[imageProgressIndicator2 setFrame:CGRectMake((tablePadding/2)+imageWidth+10,15+imageHeight,imageWidth,20)];
		[imageLabel2 setFrame:CGRectMake(tablePadding/2+imageWidth+10,25+imageHeight,imageWidth,20)];
		
		[imageView3 setFrame:CGRectMake((tablePadding/2)+(imageWidth*2)+20,10,imageWidth,imageHeight)];
		[imageProgressIndicator3 setFrame:CGRectMake((tablePadding/2)+(imageWidth*2)+20,15+imageHeight,imageWidth,20)];
		[imageLabel3 setFrame:CGRectMake(tablePadding/2+(imageWidth*2)+20,25+imageHeight,imageWidth,20)];
	}
	[cell setSelectionStyle:UITableViewCellSelectionStyleNone];
	return cell;
}

- (NSInteger)tableView:(UITableView *)theTableView numberOfRowsInSection:(NSInteger)section
{
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
		int tablePadding = 40;
		int tableWidth = [tableView frame].size.width;
		if (tableWidth > 480) { // iPad
			tablePadding = 110;
		}
		NSUInteger imageWidth = (tableWidth-tablePadding-20)/3;
		NSUInteger imageHeight = imageWidth*0.66f;
		return imageHeight+50;
	} else {
		return 42;
	}
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
	return 3;
}

@end

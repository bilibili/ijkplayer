//
//  WebPageViewController.m
//  iPhone
//
//  Created by Ben Copsey on 03/10/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "WebPageViewController.h"
#import "InfoCell.h"
#import "ASIWebPageRequest.h"
#import "ASIDownloadCache.h"
#import "ToggleCell.h"
#import "RequestProgressCell.h"

// Private stuff
@interface WebPageViewController ()
- (void)webPageFetchFailed:(ASIHTTPRequest *)theRequest;
- (void)webPageFetchSucceeded:(ASIHTTPRequest *)theRequest;
@end

@implementation WebPageViewController

- (void)fetchWebPage:(id)sender
{
	[self fetchURL:[NSURL URLWithString:[urlField text]]];
}

- (void)clearCache:(id)sender
{
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICachePermanentlyCacheStoragePolicy];
}

- (void)fetchURL:(NSURL *)url
{
	[urlField resignFirstResponder];

	[self setRequestsInProgress:[NSMutableArray array]];
	[[self tableView] reloadData];

	[request setDelegate:nil];
	[request cancel];
	[self setRequest:[ASIWebPageRequest requestWithURL:url]];

	[request setDidFailSelector:@selector(webPageFetchFailed:)];
	[request setDidFinishSelector:@selector(webPageFetchSucceeded:)];
	[request setDelegate:self];
	[request setDownloadProgressDelegate:self];
	[request setUrlReplacementMode:([replaceURLsSwitch isOn] ? ASIReplaceExternalResourcesWithData : ASIReplaceExternalResourcesWithLocalURLs)];
	
	// It is strongly recommended that you set both a downloadCache and a downloadDestinationPath for all ASIWebPageRequests
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request setCachePolicy:ASIOnlyLoadIfNotCachedCachePolicy];

	// This is actually the most efficient way to set a download path for ASIWebPageRequest, as it writes to the cache directly
	[request setDownloadDestinationPath:[[ASIDownloadCache sharedCache] pathToStoreCachedResponseDataForRequest:request]];
	
	[[ASIDownloadCache sharedCache] setShouldRespectCacheControlHeaders:NO];
	[request startAsynchronous];
}

- (void)webPageFetchFailed:(ASIHTTPRequest *)theRequest
{
	[responseField setText:[NSString stringWithFormat:@"Something went wrong: %@",[theRequest error]]];
}

- (void)webPageFetchSucceeded:(ASIHTTPRequest *)theRequest
{
	NSURL *baseURL;
	if ([replaceURLsSwitch isOn]) {
		baseURL = [theRequest url];

	// If we're using ASIReplaceExternalResourcesWithLocalURLs, we must set the baseURL to point to our locally cached file
	} else {
		baseURL = [NSURL fileURLWithPath:[request downloadDestinationPath]];
	}

	if ([theRequest downloadDestinationPath]) {
		NSString *response = [NSString stringWithContentsOfFile:[theRequest downloadDestinationPath] encoding:[theRequest responseEncoding] error:nil];
		[responseField setText:response];
		[webView loadHTMLString:response baseURL:baseURL];
	} else {
		[responseField setText:[theRequest responseString]];
		[webView loadHTMLString:[theRequest responseString] baseURL:baseURL];
	}
	
	[urlField setText:[[theRequest url] absoluteString]];
}

// We'll take over the page load when the user clicks on a link
- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)theRequest navigationType:(UIWebViewNavigationType)navigationType
{
	if (navigationType == UIWebViewNavigationTypeLinkClicked) {
		[self fetchURL:[theRequest URL]];
		return NO;
	}
	return YES;
}


// At time of writing ASIWebPageRequests do not support automatic progress tracking across all requests needed for a page
// The code below shows one approach you could use for tracking progress - it creates a new row with a progress indicator for each resource request
// However, you could use the same approach and keep track of an overal total to show progress
- (void)requestStarted:(ASIWebPageRequest *)theRequest
{
	[[self requestsInProgress] addObject:theRequest];
	[[self tableView] reloadData];
}

- (void)requestFinished:(ASIWebPageRequest *)theRequest
{
	if (![[self requestsInProgress] containsObject:theRequest]) {
		[[self requestsInProgress] addObject:theRequest];
		[[self tableView] reloadData];	
	}
}

- (void)request:(ASIHTTPRequest *)theRequest didReceiveBytes:(long long)newLength
{
	NSInteger requestNumber = [[self requestsInProgress] indexOfObject:theRequest];
	if (requestNumber != NSNotFound) {
		RequestProgressCell *cell = (RequestProgressCell *)[[self tableView] cellForRowAtIndexPath:[NSIndexPath indexPathForRow:requestNumber inSection:2]];
		if ([theRequest contentLength]+[theRequest partialDownloadSize] > 0) {
			float progressAmount = (float)(([theRequest totalBytesRead]*1.0)/(([theRequest contentLength]+[theRequest partialDownloadSize])*1.0));
			[[cell progressView] setProgress:progressAmount];
		}
	}
}

- (void)request:(ASIHTTPRequest *)theRequest incrementDownloadSizeBy:(long long)newLength
{
	[self request:theRequest didReceiveBytes:0];
}

/*
 Most of the code below here relates to the table view, and isn't that interesting
 */

- (void)dealloc
{
	[request setDelegate:nil];
	[request setDownloadProgressDelegate:nil];
	[request cancel];
	[request release];
	[webView setDelegate:nil];
	[webView release];
	[responseField release];
	[urlField release];
	[requestsInProgress release];
	[super dealloc];
}

- (void)viewDidLoad
{
	[super viewDidLoad];
	[[[self navigationBar] topItem] setTitle:@"Downloading a Web Page"];
	webView = [[UIWebView alloc] initWithFrame:CGRectZero];
	[webView setDelegate:self];
	responseField = [[UITextView alloc] initWithFrame:CGRectZero];
	[responseField setBackgroundColor:[UIColor clearColor]];
	[responseField setEditable:NO];
	[responseField setText:@"HTML source will appear here"];
	urlField = [[UITextField alloc] initWithFrame:CGRectZero];
	[urlField setBorderStyle:UITextBorderStyleRoundedRect];
	[urlField setText:@"http://allseeing-i.com"];
}

static NSString *intro = @"ASIWebPageRequest lets you download complete webpages, including most of their external resources. ASIWebPageRequest can download stylesheets, javascript files, images (including those referenced in CSS), frames, iframes, and HTML 5 audio and video.\r\n\r\nYou can set ASIWebPageRequest to modify HTML and CSS content to point at locally cached external resources, or you can have it replace the urls of external files with their actual data. This lets you save a complete web page as a single file.\r\n\r\nASIWebPageRequest is NOT intended to be a drop-in replacement for UIWebView's regular loading mechanism. It is best used for getting more control over caching web pages you control, or for displaying web page content that requires more complex authentication (eg NTLM).\r\n\r\nIt is strongly recommended that you use ASIWebPageRequest in conjunction with a downloadCache, and you should always set a downloadDestinationPath for all ASIWebPageRequests. ASIWebPageRequests cannot be used with startSynchronous.\r\n\r\nTo use ASIWebPage request, you must link with libxml, and add '${SDK_DIR}/usr/include/libxml2' to your Header Search Paths.";

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
		if ([indexPath row] == 0) {
			cell = [tableView dequeueReusableCellWithIdentifier:@"WebPageCell"];
			if (!cell) {
				cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"WebPageCell"] autorelease];
				[[cell contentView] addSubview:webView];
			}
			[webView setFrame:CGRectMake(10,10,tableWidth-tablePadding,280)];
		} else {
			cell = [tableView dequeueReusableCellWithIdentifier:@"ToggleCell"];
			if (!cell) {
				cell = [ToggleCell cell];
			}
			[[cell textLabel] setText:@"Replace URLs with Data"];
			replaceURLsSwitch = [(ToggleCell *)cell toggle];

			// We set our switch to clear the cache when the value is changed
			// This is because data cached with ASIReplaceExternalResourcesWithLocalURLs is not suitable for use by other urlReplacementModes
			// In your own app, you should basically choose one of these modes, and always use that
			[replaceURLsSwitch addTarget:self action:@selector(clearCache:) forControlEvents:UIControlEventValueChanged];
		}
	} else if ([indexPath section] == 2) {

		cell = [tableView dequeueReusableCellWithIdentifier:@"RequestProgressCell"];
		if (!cell) {
			cell = [RequestProgressCell cell];
		}
		ASIHTTPRequest *theRequest = [[self requestsInProgress] objectAtIndex:[indexPath row]];
		if ([theRequest didUseCachedResponse]) {
			[[cell textLabel] setText:[NSString stringWithFormat:@"Cached: %@",[[theRequest url] absoluteString]]];
			[[cell accessoryView] setHidden:YES];
			
		} else {
			[[cell textLabel] setText:[[theRequest url] absoluteString]];
			if ([theRequest contentLength] > 0) {
				[[(RequestProgressCell *)cell progressView] setProgress:(float)(([theRequest totalBytesRead]*1.0)/(([theRequest contentLength]+[theRequest partialDownloadSize])*1.0))];
			}
			[[cell accessoryView] setHidden:NO];
		}

	} else {
		
		cell = [tableView dequeueReusableCellWithIdentifier:@"Response"];
		if (!cell) {
			cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"Response"] autorelease];
			[[cell contentView] addSubview:responseField];
			
		}	
		[responseField setFrame:CGRectMake(5,5,tableWidth-tablePadding,180)];
		
	}
	[cell setSelectionStyle:UITableViewCellSelectionStyleNone];
	return cell;
}


- (UIView *)tableView:(UITableView *)theTableView viewForHeaderInSection:(NSInteger)section
{
	if (section == 1) {
		int tablePadding = 40;
		int tableWidth = [tableView frame].size.width;
		if (tableWidth > 480) { // iPad
			tablePadding = 110;
		}
		
		UIView *view = [[[UIView alloc] initWithFrame:CGRectMake(0,0,tableWidth-(tablePadding/2),30)] autorelease];

		UIButton *clearCacheButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
		[clearCacheButton setTitle:@"Clear Cache" forState:UIControlStateNormal];
		[clearCacheButton sizeToFit];
		[clearCacheButton setFrame:CGRectMake([view frame].size.width-[clearCacheButton frame].size.width+10,7,[clearCacheButton frame].size.width,[clearCacheButton frame].size.height)];
		
		[clearCacheButton addTarget:self action:@selector(clearCache:) forControlEvents:UIControlEventTouchDown];
		[view addSubview:clearCacheButton];
		
		UIButton *goButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
		[goButton setTitle:@"Go!" forState:UIControlStateNormal];
		[goButton sizeToFit];
		[goButton setFrame:CGRectMake([clearCacheButton frame].origin.x-[goButton frame].size.width-10,7,[goButton frame].size.width,[goButton frame].size.height)];
		
		
		[goButton addTarget:self action:@selector(fetchWebPage:) forControlEvents:UIControlEventTouchDown];
		[view addSubview:goButton];
		
		[urlField setFrame:CGRectMake((tablePadding/2)-10,8,tableWidth-tablePadding-160,34)];
		[view addSubview:urlField];
		
		
		return view;
	}
	return nil;
}

- (NSInteger)tableView:(UITableView *)theTableView numberOfRowsInSection:(NSInteger)section
{
	if (section == 1) {
		return 2;
	} else if (section == 2) {
		return [requestsInProgress count];
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
	} else if ([indexPath section] == 1) {
		if ([indexPath row] == 0) {
			return 300;
		} else {
			return 50;
		}
	} else if ([indexPath section] == 3) {
		return 200;
	} else {
		return 34;
	}
}

- (NSString *)tableView:(UITableView *)theTableView titleForHeaderInSection:(NSInteger)section
{
	return nil;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
	return 4;
}

@synthesize request;
@synthesize requestsInProgress;
@end

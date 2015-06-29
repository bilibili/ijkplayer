//
//  AppDelegate.m
//
//  Created by Ben Copsey on 09/07/2008.
//  Copyright 2008 All-Seeing Interactive Ltd. All rights reserved.
//

#import "AppDelegate.h"
#import "ASIHTTPRequest.h"
#import "ASIFormDataRequest.h"
#import "ASINetworkQueue.h"
#import "ASIDownloadCache.h"
#import "ASIWebPageRequest.h"

@interface AppDelegate ()
- (void)updateBandwidthUsageIndicator;
- (void)URLFetchWithProgressComplete:(ASIHTTPRequest *)request;
- (void)URLFetchWithProgressFailed:(ASIHTTPRequest *)request;
- (void)imageFetch1Complete:(ASIHTTPRequest *)request;
- (void)imageFetch2Complete:(ASIHTTPRequest *)request;
- (void)imageFetch3Complete:(ASIHTTPRequest *)request;
- (void)topSecretFetchComplete:(ASIHTTPRequest *)request;
- (void)authSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
- (void)postFinished:(ASIHTTPRequest *)request;
- (void)postFailed:(ASIHTTPRequest *)request;
- (void)fetchURL:(NSURL *)url;
- (void)tableViewDataFetchFinished:(ASIHTTPRequest *)request;
- (void)rowImageDownloadFinished:(ASIHTTPRequest *)request;
- (void)webPageFetchFailed:(ASIHTTPRequest *)request;
- (void)webPageFetchSucceeded:(ASIHTTPRequest *)request;
@end

@implementation AppDelegate


- (id)init
{
	[super init];
	networkQueue = [[ASINetworkQueue alloc] init];
	[NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(updateBandwidthUsageIndicator) userInfo:nil repeats:YES];
	return self;
}

- (void)dealloc
{
	[networkQueue release];
	[super dealloc];
}


- (IBAction)simpleURLFetch:(id)sender
{
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]] autorelease];
	
	//Customise our user agent, for no real reason
	[request addRequestHeader:@"User-Agent" value:@"ASIHTTPRequest"];
	[request setDelegate:self];
	[request startSynchronous];
	if ([request error]) {
		[htmlSource setString:[[request error] localizedDescription]];
	} else if ([request responseString]) {
		[htmlSource setString:[request responseString]];
	}
}



- (IBAction)URLFetchWithProgress:(id)sender
{
	[startButton setTitle:@"Stop"];
	[startButton setAction:@selector(stopURLFetchWithProgress:)];
	
	NSString *tempFile = [[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"The Great American Novel.txt.download"];
	if ([[NSFileManager defaultManager] fileExistsAtPath:tempFile]) {
		[[NSFileManager defaultManager] removeItemAtPath:tempFile error:nil];
	}
	
	[self resumeURLFetchWithProgress:self];
}


- (IBAction)stopURLFetchWithProgress:(id)sender
{
	[startButton setTitle:@"Start"];
	[startButton setAction:@selector(URLFetchWithProgress:)];
	[[self bigFetchRequest] cancel];
	[self setBigFetchRequest:nil];
	[resumeButton setEnabled:YES];
}

- (IBAction)resumeURLFetchWithProgress:(id)sender
{
	[fileLocation setStringValue:@"(Request running)"];
	[resumeButton setEnabled:NO];
	[startButton setTitle:@"Stop"];
	[startButton setAction:@selector(stopURLFetchWithProgress:)];
	
	// Stop any other requests
	[networkQueue reset];
	
	[self setBigFetchRequest:[ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect_resume"]]];
	[[self bigFetchRequest] setDownloadDestinationPath:[[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"The Great American Novel.txt"]];
	[[self bigFetchRequest] setTemporaryFileDownloadPath:[[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"The Great American Novel.txt.download"]];
	[[self bigFetchRequest] setAllowResumeForFileDownloads:YES];
	[[self bigFetchRequest] setDelegate:self];
	[[self bigFetchRequest] setDidFinishSelector:@selector(URLFetchWithProgressComplete:)];
	[[self bigFetchRequest] setDidFailSelector:@selector(URLFetchWithProgressFailed:)];
	[[self bigFetchRequest] setDownloadProgressDelegate:progressIndicator];
	[[self bigFetchRequest] startAsynchronous];
}

- (void)URLFetchWithProgressComplete:(ASIHTTPRequest *)request
{
	[fileLocation setStringValue:[NSString stringWithFormat:@"File downloaded to %@",[request downloadDestinationPath]]];
	[startButton setTitle:@"Start"];
	[startButton setAction:@selector(URLFetchWithProgress:)];
}

- (void)URLFetchWithProgressFailed:(ASIHTTPRequest *)request
{
	if ([[request error] domain] == NetworkRequestErrorDomain && [[request error] code] == ASIRequestCancelledErrorType) {
		[fileLocation setStringValue:@"(Request paused)"];
	} else {
		[fileLocation setStringValue:[NSString stringWithFormat:@"An error occurred: %@",[[request error] localizedDescription]]];
		[startButton setTitle:@"Start"];
		[startButton setAction:@selector(URLFetchWithProgress:)];
	}
}

- (IBAction)fetchThreeImages:(id)sender
{
	[imageView1 setImage:nil];
	[imageView2 setImage:nil];
	[imageView3 setImage:nil];
	
	[networkQueue reset];
	[networkQueue setDownloadProgressDelegate:progressIndicator];
	[networkQueue setDelegate:self];
	[networkQueue setShowAccurateProgress:([showAccurateProgress state] == NSOnState)];
	
	ASIHTTPRequest *request;
	
	request = [[[ASIHTTPRequest alloc] initWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/images/small-image.jpg"]] autorelease];
	[request setDownloadDestinationPath:[[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"1.png"]];
	[request setDownloadProgressDelegate:imageProgress1];
	[request setDidFinishSelector:@selector(imageFetch1Complete:)];
	[request setDelegate:self];
	[networkQueue addOperation:request];
	
	request = [[[ASIHTTPRequest alloc] initWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/images/medium-image.jpg"]] autorelease];
	[request setDownloadDestinationPath:[[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"2.png"]];
	[request setDownloadProgressDelegate:imageProgress2];
	[request setDidFinishSelector:@selector(imageFetch2Complete:)];
	[request setDelegate:self];
	[networkQueue addOperation:request];
	
	request = [[[ASIHTTPRequest alloc] initWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/images/large-image.jpg"]] autorelease];
	[request setDownloadDestinationPath:[[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"3.png"]];
	[request setDownloadProgressDelegate:imageProgress3];
	[request setDidFinishSelector:@selector(imageFetch3Complete:)];
	[request setDelegate:self];
	[networkQueue addOperation:request];
	
	
	[networkQueue go];
}

- (void)updateBandwidthUsageIndicator
{
	[bandwidthUsed setStringValue:[NSString stringWithFormat:@"%luKB / second",[ASIHTTPRequest averageBandwidthUsedPerSecond]/1024]];
}

- (IBAction)throttleBandwidth:(id)sender
{
	if ([(NSButton *)sender state] == NSOnState) {
		[ASIHTTPRequest setMaxBandwidthPerSecond:ASIWWANBandwidthThrottleAmount];
	} else {
		[ASIHTTPRequest setMaxBandwidthPerSecond:0];
	}
}


- (void)imageFetch1Complete:(ASIHTTPRequest *)request
{
	NSImage *img = [[[NSImage alloc] initWithContentsOfFile:[request downloadDestinationPath]] autorelease];
	if (img) {
		[imageView1 setImage:img];
	}
}

- (void)imageFetch2Complete:(ASIHTTPRequest *)request
{
	NSImage *img = [[[NSImage alloc] initWithContentsOfFile:[request downloadDestinationPath]] autorelease];
	if (img) {
		[imageView2 setImage:img];
	}
}


- (void)imageFetch3Complete:(ASIHTTPRequest *)request
{
	NSImage *img = [[[NSImage alloc] initWithContentsOfFile:[request downloadDestinationPath]] autorelease];
	if (img) {
		[imageView3 setImage:img];
	}
}


- (IBAction)fetchTopSecretInformation:(id)sender
{
	[networkQueue reset];
	
	[progressIndicator setDoubleValue:0];
	
	ASIHTTPRequest *request;
	request = [[[ASIHTTPRequest alloc] initWithURL:[NSURL URLWithString:@"http://allseeing-i.com/top_secret/"]] autorelease];
	[request setDidFinishSelector:@selector(topSecretFetchComplete:)];
	[request setDelegate:self];
	[request setUseKeychainPersistence:[keychainCheckbox state]];
	[request startAsynchronous];

}

- (void)topSecretFetchComplete:(ASIHTTPRequest *)request
{
	if (![request error]) {
		[topSecretInfo setStringValue:[request responseString]];
		[topSecretInfo setFont:[NSFont boldSystemFontOfSize:13]];
	}
}

- (void)authenticationNeededForRequest:(ASIHTTPRequest *)request
{
	[realm setStringValue:[request authenticationRealm]];
	[host setStringValue:[[request url] host]];

	[NSApp beginSheet: loginWindow
		modalForWindow: window
		modalDelegate: self
		didEndSelector: @selector(authSheetDidEnd:returnCode:contextInfo:)
		contextInfo: request];
}

- (void)proxyAuthenticationNeededForRequest:(ASIHTTPRequest *)request
{
	[realm setStringValue:[request proxyAuthenticationRealm]];
	[host setStringValue:[request proxyHost]];
	
	[NSApp beginSheet: loginWindow
	   modalForWindow: window
		modalDelegate: self
	   didEndSelector: @selector(authSheetDidEnd:returnCode:contextInfo:)
		  contextInfo: request];
}


- (IBAction)dismissAuthSheet:(id)sender {
    [[NSApplication sharedApplication] endSheet: loginWindow returnCode: [(NSControl*)sender tag]];
}

- (void)authSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	ASIHTTPRequest *request = (ASIHTTPRequest *)contextInfo;
    if (returnCode == NSOKButton) {
		if ([request authenticationNeeded] == ASIProxyAuthenticationNeeded) {
			[request setProxyUsername:[[[username stringValue] copy] autorelease]];
			[request setProxyPassword:[[[password stringValue] copy] autorelease]];			
		} else {
			[request setUsername:[[[username stringValue] copy] autorelease]];
			[request setPassword:[[[password stringValue] copy] autorelease]];
		}
		[request retryUsingSuppliedCredentials];
    } else {
		[request cancelAuthentication];
	}
    [loginWindow orderOut: self];
}

- (IBAction)postWithProgress:(id)sender
{	
	//Create a 1MB file
	NSMutableData *data = [NSMutableData dataWithLength:1024*1024];
	NSString *path = [[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"bigfile"];
	[data writeToFile:path atomically:NO];
	
	
	[networkQueue reset];
	[networkQueue setShowAccurateProgress:YES];
	[networkQueue setUploadProgressDelegate:progressIndicator];
	[networkQueue setRequestDidFailSelector:@selector(postFailed:)];
	[networkQueue setRequestDidFinishSelector:@selector(postFinished:)];
	[networkQueue setDelegate:self];
	
	ASIFormDataRequest *request = [[[ASIFormDataRequest alloc] initWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ignore"]] autorelease];
	[request setPostValue:@"test" forKey:@"value1"];
	[request setPostValue:@"test" forKey:@"value2"];
	[request setPostValue:@"test" forKey:@"value3"];
	[request setFile:path forKey:@"file"];
	

	[networkQueue addOperation:request];
	[networkQueue go];
}

- (void)postFinished:(ASIHTTPRequest *)request
{
	[postStatus setStringValue:@"Post Finished"];
}
- (void)postFailed:(ASIHTTPRequest *)request
{
	[postStatus setStringValue:[NSString stringWithFormat:@"Post Failed: %@",[[request error] localizedDescription]]];
}


- (IBAction)reloadTableData:(id)sender
{
	[[self tableQueue] cancelAllOperations];
	[self setRowData:[NSMutableArray array]];
	[tableView reloadData];

	[self setTableQueue:[ASINetworkQueue queue]];
	
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/table-row-data.xml"]];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request setDidFinishSelector:@selector(tableViewDataFetchFinished:)];
	[request setDelegate:self];
	[[self tableQueue] addOperation:request];
	[[self tableQueue] setDownloadProgressDelegate:progressIndicator];
	[[self tableQueue] go];
}

- (void)tableViewDataFetchFailed:(ASIHTTPRequest *)request
{
	if ([[request error] domain] != NetworkRequestErrorDomain || ![[request error] code] == ASIRequestCancelledErrorType) {
		[tableLoadStatus setStringValue:@"Loading data failed"];
	}
}

- (void)tableViewDataFetchFinished:(ASIHTTPRequest *)request
{
	NSXMLDocument *xml = [[[NSXMLDocument alloc] initWithData:[request responseData] options:NSXMLDocumentValidate error:nil] autorelease];
	for (NSXMLElement *row in [[xml rootElement] elementsForName:@"row"]) {
		NSMutableDictionary *rowInfo = [NSMutableDictionary dictionary];
		NSString *description = [[[row elementsForName:@"description"] objectAtIndex:0] stringValue];
		[rowInfo setValue:description forKey:@"description"];
		NSString *imageURL = [[[row elementsForName:@"image"] objectAtIndex:0] stringValue];
		ASIHTTPRequest *imageRequest = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:imageURL]];
		[imageRequest setDownloadCache:[ASIDownloadCache sharedCache]];
		[imageRequest setDidFinishSelector:@selector(rowImageDownloadFinished:)];
		[imageRequest setDidFailSelector:@selector(tableViewDataFetchFailed:)];
		[imageRequest setDelegate:self];
		[imageRequest setUserInfo:rowInfo];
		[[self tableQueue] addOperation:imageRequest];
		[[self rowData] addObject:rowInfo];
	}
	[tableView reloadData];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    return [[self rowData] count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	if ([[aTableColumn identifier] isEqualToString:@"image"]) {
		return [[[self rowData] objectAtIndex:rowIndex] objectForKey:@"image"];
	} else {
		return [[[self rowData] objectAtIndex:rowIndex] objectForKey:@"description"];
	}
}


- (void)rowImageDownloadFinished:(ASIHTTPRequest *)request
{
	NSImage *image = [[[NSImage alloc] initWithData:[request responseData]] autorelease];
	[(NSMutableDictionary *)[request userInfo] setObject:image forKey:@"image"];
	[tableView reloadData]; // Not efficient, but I hate table view programming :)
}

- (IBAction)clearCache:(id)sender
{
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICachePermanentlyCacheStoragePolicy];
}

- (IBAction)fetchWebPage:(id)sender
{
	[self fetchURL:[NSURL URLWithString:[urlField stringValue]]];

}

- (void)webView:(WebView *)sender decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id)listener
{
	// If this is a web page we've requested ourselves, let it load
	if ([[actionInformation objectForKey:WebActionNavigationTypeKey] intValue] == WebNavigationTypeOther) {
		[listener use];
		return;
	}

	// If the user clicked on a link, let's tell the webview to ignore it, and we'll load it ourselves
	[self fetchURL:[NSURL URLWithString:[[request URL] absoluteString] relativeToURL:[NSURL URLWithString:[urlField stringValue]]]];
	[listener ignore];
}

- (void)fetchURL:(NSURL *)url
{
	ASIWebPageRequest *request = [ASIWebPageRequest requestWithURL:url];
	[request setDidFailSelector:@selector(webPageFetchFailed:)];
	[request setDidFinishSelector:@selector(webPageFetchSucceeded:)];
	[request setDelegate:self];
	[request setShowAccurateProgress:NO];
	[request setDownloadProgressDelegate:progressIndicator];
	[request setUrlReplacementMode:([dataURICheckbox state] == NSOnState ? ASIReplaceExternalResourcesWithData : ASIReplaceExternalResourcesWithLocalURLs)];

	// It is strongly recommended that you set both a downloadCache and a downloadDestinationPath for all ASIWebPageRequests
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request setDownloadDestinationPath:[[ASIDownloadCache sharedCache] pathToStoreCachedResponseDataForRequest:request]];

	[[ASIDownloadCache sharedCache] setShouldRespectCacheControlHeaders:NO];
	[request startAsynchronous];
}

- (void)webPageFetchFailed:(ASIHTTPRequest *)request
{
	[[NSAlert alertWithError:[request error]] runModal];
}

- (void)webPageFetchSucceeded:(ASIHTTPRequest *)request
{
	NSURL *baseURL;
	if ([dataURICheckbox state] == NSOnState) {
		baseURL = [request url];

		// If we're using ASIReplaceExternalResourcesWithLocalURLs, we must set the baseURL to point to our locally cached file
	} else {
		baseURL = [NSURL fileURLWithPath:[request downloadDestinationPath]];
	}

	if ([request downloadDestinationPath]) {
		NSString *response = [NSString stringWithContentsOfFile:[request downloadDestinationPath] encoding:[request responseEncoding] error:nil];
		[webPageSource setString:response];
		[[webView mainFrame] loadHTMLString:response baseURL:baseURL];
	} else if ([request responseString]) {
		[webPageSource setString:[request responseString]];
		[[webView mainFrame] loadHTMLString:[request responseString] baseURL:baseURL];
	}

	[urlField setStringValue:[[request url] absoluteString]];
}


@synthesize bigFetchRequest;
@synthesize rowData;
@synthesize tableQueue;
@end

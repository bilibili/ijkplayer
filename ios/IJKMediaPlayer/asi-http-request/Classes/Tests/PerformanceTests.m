//
//  PerformanceTests.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 17/12/2009.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//

#import "PerformanceTests.h"
#import "ASIHTTPRequest.h"

// IMPORTANT - these tests need to be run one at a time!

@interface NSURLConnectionSubclass : NSURLConnection {
	int tag;
}
@property (assign) int tag;
@end
@implementation NSURLConnectionSubclass
@synthesize tag;
@end

// Stop clang complaining about undeclared selectors
@interface PerformanceTests ()
- (void)runSynchronousASIHTTPRequests;
- (void)runSynchronousNSURLConnections;
- (void)startASIHTTPRequests;
- (void)startASIHTTPRequestsWithQueue;
- (void)startNSURLConnections;
@end


@implementation PerformanceTests

- (void)setUp
{
	[self setTestURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_%28abridged%29.txt"]];
	//[self setTestURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
}

- (void)testASIHTTPRequestSynchronousPerformance
{
	[self performSelectorOnMainThread:@selector(runSynchronousASIHTTPRequests) withObject:nil waitUntilDone:YES];
}


- (void)runSynchronousASIHTTPRequests
{
	int runTimes = 10;
	NSTimeInterval times[runTimes];
	int i;
	for (i=0; i<runTimes; i++) {
		NSDate *startTime = [NSDate date];
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:testURL];
		//Send the same headers as NSURLRequest
		[request addRequestHeader:@"Pragma" value:@"no-cache"];
		[request addRequestHeader:@"Accept" value:@"*/*"];
		[request addRequestHeader:@"Accept-Language" value:@"en/us"];
		[request startSynchronous];
		if ([request error]) {
			NSLog(@"Request failed - cannot proceed with test");
			return;
		}
		times[i] = [[NSDate date] timeIntervalSinceDate:startTime];
	}
	NSTimeInterval bestTime = 1000;
	NSTimeInterval worstTime = 0;
	NSTimeInterval totalTime = 0;
	for (i=0; i<runTimes; i++) {
		if (times[i] < bestTime) {
			bestTime = times[i];
		}
		if (times[i] > worstTime) {
			worstTime = times[i];
		}
		totalTime += times[i];
	}
	NSLog(@"Ran %i requests in %f seconds (average time: %f secs / best time: %f secs / worst time: %f secs)",runTimes,totalTime,totalTime/runTimes,bestTime,worstTime);
}


- (void)testNSURLConnectionSynchronousPerformance
{
	[self performSelectorOnMainThread:@selector(runSynchronousNSURLConnections) withObject:nil waitUntilDone:YES];
}


- (void)runSynchronousNSURLConnections
{
	int runTimes = 10;
	NSTimeInterval times[runTimes];
	int i;
	for (i=0; i<runTimes; i++) {
		NSDate *startTime = [NSDate date];
		
		NSURLResponse *response = nil;
		NSError *error = nil;
		NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:testURL cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:10];
		[NSURLConnection sendSynchronousRequest:request returningResponse:&response error:&error];
		if (error) {
			NSLog(@"Request failed - cannot proceed with test");
			return;
		}
		times[i] = [[NSDate date] timeIntervalSinceDate:startTime];
	}
	NSTimeInterval bestTime = 1000;
	NSTimeInterval worstTime = 0;
	NSTimeInterval totalTime = 0;
	for (i=0; i<runTimes; i++) {
		if (times[i] < bestTime) {
			bestTime = times[i];
		}
		if (times[i] > worstTime) {
			worstTime = times[i];
		}
		totalTime += times[i];
	}
	NSLog(@"Ran %i requests in %f seconds (average time: %f secs / best time: %f secs / worst time: %f secs)",runTimes,totalTime,totalTime/runTimes,bestTime,worstTime);
}


- (void)testASIHTTPRequestAsyncPerformance
{
	[self performSelectorOnMainThread:@selector(startASIHTTPRequests) withObject:nil waitUntilDone:NO];
}

- (void)testQueuedASIHTTPRequestAsyncPerformance
{
	[self performSelectorOnMainThread:@selector(startASIHTTPRequestsWithQueue) withObject:nil waitUntilDone:NO];
}


- (void)startASIHTTPRequests
{
	bytesDownloaded = 0;
	[self setRequestsComplete:0];
	[self setTestStartDate:[NSDate date]];
	int i;
	for (i=0; i<10; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:testURL];
		//Send the same headers as NSURLRequest
		[request addRequestHeader:@"Pragma" value:@"no-cache"];
		[request addRequestHeader:@"Accept" value:@"*/*"];
		[request addRequestHeader:@"Accept-Language" value:@"en/us"];
		[request setDelegate:self];
		[request startAsynchronous];
	}
}

- (void)startASIHTTPRequestsWithQueue
{
	bytesDownloaded = 0;
	[self setRequestsComplete:0];
	[self setTestStartDate:[NSDate date]];
	int i;
	NSOperationQueue *queue = [[[NSOperationQueue alloc] init] autorelease];
	[queue setMaxConcurrentOperationCount:4];
	for (i=0; i<10; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:testURL];
		//Send the same headers as NSURLRequest
		[request addRequestHeader:@"Pragma" value:@"no-cache"];
		[request addRequestHeader:@"Accept" value:@"*/*"];
		[request addRequestHeader:@"Accept-Language" value:@"en/us"];
		[request setUseCookiePersistence:NO];
		[request setUseSessionPersistence:NO];
		[request setDelegate:self];
		[queue addOperation:request];
	}
}

- (void)requestFailed:(ASIHTTPRequest *)request
{
	GHFail(@"Cannot proceed with ASIHTTPRequest test - a request failed");
}

- (void)requestFinished:(ASIHTTPRequest *)request
{
	bytesDownloaded += [[request responseData] length];
	requestsComplete++;
	if (requestsComplete == 10) {
		NSLog(@"ASIHTTPRequest: Completed 10 (downloaded %lu bytes) requests in %f seconds",bytesDownloaded,[[NSDate date] timeIntervalSinceDate:[self testStartDate]]);
	}
}

- (void)testNSURLConnectionAsyncPerformance
{
	[self performSelectorOnMainThread:@selector(startNSURLConnections) withObject:nil waitUntilDone:NO];
}

- (void)startNSURLConnections
{
	bytesDownloaded = 0;
	[self setRequestsComplete:0];
	[self setTestStartDate:[NSDate date]];
	[self setResponseData:[NSMutableArray arrayWithCapacity:5]]; 
	
	int i;
	for (i=0; i<10; i++) {
		NSURLRequest *request = [NSURLRequest requestWithURL:testURL cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:10];
		[[self responseData] addObject:[NSMutableData data]];
		NSURLConnectionSubclass *connection = [[[NSURLConnectionSubclass alloc] initWithRequest:request delegate:self startImmediately:YES] autorelease];
		[connection setTag:i];		
	}
}

- (void)connection:(NSURLConnectionSubclass *)connection didReceiveResponse:(NSURLResponse *)response
{
}

- (void)connection:(NSURLConnectionSubclass *)connection didFailWithError:(NSError *)error
{
	GHFail(@"Cannot proceed with NSURLConnection test - a request failed");
}

- (void)connection:(NSURLConnectionSubclass *)connection didReceiveData:(NSData *)data
{
	[[[self responseData] objectAtIndex:[connection tag]] appendData:data];	

}

- (void)connectionDidFinishLoading:(NSURLConnectionSubclass *)connection
{
	bytesDownloaded += [[responseData objectAtIndex:[connection tag]] length];
	requestsComplete++;
	if (requestsComplete == 10) {
		NSLog(@"NSURLConnection: Completed 10 (downloaded %lu bytes) requests in %f seconds",bytesDownloaded,[[NSDate date] timeIntervalSinceDate:[self testStartDate]]);
	}		
}

@synthesize testURL;
@synthesize requestsComplete;
@synthesize testStartDate;
@synthesize responseData;
@end

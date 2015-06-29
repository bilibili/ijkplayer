//
//  StressTests.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 30/10/2009.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//

/*
IMPORTANT
All these tests depend on you running a local webserver on port 80
These tests create 1000s of requests in a very short space of time - DO NOT RUN THESE TESTS ON A REMOTE WEBSERVER
IMPORTANT
*/

#import "StressTests.h"
#import "ASIHTTPRequest.h"
#import "ASINetworkQueue.h"



@implementation MyDelegate;
- (void)dealloc
{
	[request setDelegate:nil];
	[request release];
	[super dealloc];
}
@synthesize request;
@end

// Stop clang complaining about undeclared selectors
@interface StressTests ()
- (void)cancelRedirectRequest;
- (void)cancelSetDelegateRequest;
@end



@implementation StressTests

// A test for a potential crasher that used to exist when requests were cancelled
// We aren't testing a specific condition here, but rather attempting to trigger a crash
- (void)testCancelQueue
{
	ASINetworkQueue *queue = [ASINetworkQueue queue];
	
	// Increase the risk of this crash
	[queue setMaxConcurrentOperationCount:25];
	int i;
	for (i=0; i<100; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://127.0.0.1"]];
		[queue addOperation:request];
	}
	[queue go];
	[queue cancelAllOperations];
	
	// Run the test again with requests running on a background thread
	queue = [ASINetworkQueue queue];

	[queue setMaxConcurrentOperationCount:25];
	for (i=0; i<100; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://127.0.0.1"]];
		[queue addOperation:request];
	}
	[queue go];
	[NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow:2]];
	[queue cancelAllOperations];
	
}

// This test looks for thread-safety problems with cancelling requests
// It will run for 30 seconds, creating a request, then cancelling it and creating another as soon as it gets some indication of progress

- (void)testCancelStressTest
{
	[self setCancelStartDate:[NSDate dateWithTimeIntervalSinceNow:30]];
	[self performCancelRequest];
	while ([[self cancelStartDate] timeIntervalSinceNow] > 0) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	NSLog(@"Stress test: DONE");
}

- (void)performCancelRequest
{
	[self setCancelRequest:[ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://127.0.0.1/ASIHTTPRequest/tests/the_great_american_novel.txt"]]];
	if ([[self cancelStartDate] timeIntervalSinceNow] > 0) {
		[[self cancelRequest] setDownloadProgressDelegate:self];
		[[self cancelRequest] setShowAccurateProgress:YES];
		NSLog(@"Stress test: Start request %@",[self cancelRequest]);
		[[self cancelRequest] startAsynchronous];
	}
}


// Another stress test that looks from problems when redirecting

- (void)testRedirectStressTest
{
	[self setCancelStartDate:[NSDate dateWithTimeIntervalSinceNow:30]];
	[self performRedirectRequest];
	while ([[self cancelStartDate] timeIntervalSinceNow] > 0) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	NSLog(@"Redirect stress test: DONE");
}

- (void)performRedirectRequest
{
	[self setCancelRequest:[ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://127.0.0.1/ASIHTTPRequest/tests/one_infinite_loop"]]];
	if ([[self cancelStartDate] timeIntervalSinceNow] > 0) {
		NSLog(@"Redirect stress test: Start request %@",[self cancelRequest]);
		[[self cancelRequest] startAsynchronous];
		[self performSelector:@selector(cancelRedirectRequest) withObject:nil afterDelay:0.2];
	}
}

- (void)cancelRedirectRequest
{
	NSLog(@"Redirect stress test: Cancel request %@",[self cancelRequest]);
	[[self cancelRequest] cancel];
	[self performRedirectRequest];
}

// Ensures we can set the delegate while the request is running without problems
- (void)testSetDelegate
{
	[self setCreateRequestLock:[[[NSLock alloc] init] autorelease]];
	[self setCancelStartDate:[NSDate dateWithTimeIntervalSinceNow:30]];
	[self performSetDelegateRequest];
	while ([[self cancelStartDate] timeIntervalSinceNow] > 0) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	NSLog(@"Set delegate stress test: DONE");
}

- (void)performSetDelegateRequest
{
	[self setDelegate:nil];
	
	[createRequestLock lock];
	[self setCancelRequest:[ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://127.0.0.1/ASIHTTPRequest/tests/the_great_american_novel.txt"]]];
	if ([[self cancelStartDate] timeIntervalSinceNow] > 0) {
		[self setDelegate:[[[MyDelegate alloc] init] autorelease]];
		[[self delegate] setRequest:[self cancelRequest]];
		[[self cancelRequest] setDelegate:delegate];
		[[self cancelRequest] setShowAccurateProgress:YES];
		NSLog(@"Set delegate stress test: Start request %@",[self cancelRequest]);
		[[self cancelRequest] startAsynchronous];
		[self performSelectorInBackground:@selector(cancelSetDelegateRequest) withObject:nil];
	}
	[createRequestLock unlock];
}

- (void)cancelSetDelegateRequest
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[self performSetDelegateRequest];
	[pool release];
}


- (void)setDoubleValue:(double)newProgress
{
	[self setProgress:(float)newProgress];
}

- (void)setProgress:(float)newProgress
{
	progress = newProgress;
	
	// For cancel test
	if (newProgress > 0 && [self cancelRequest]) {
		
		NSLog(@"Stress test: Cancel request %@",[self cancelRequest]);
		[[self cancelRequest] cancel];
		
		[self performSelector:@selector(performCancelRequest) withObject:nil afterDelay:0.2];
		[self setCancelRequest:nil];
	}
}





@synthesize cancelRequest;
@synthesize cancelStartDate;
@synthesize delegate;
@synthesize createRequestLock;
@end

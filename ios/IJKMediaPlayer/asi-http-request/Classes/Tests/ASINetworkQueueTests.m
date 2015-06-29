//
//  ASINetworkQueueTests.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 08/11/2008.
//  Copyright 2008 All-Seeing Interactive. All rights reserved.
//

#import "ASINetworkQueueTests.h"
#import "ASIHTTPRequest.h"
#import "ASINetworkQueue.h"
#import "ASIFormDataRequest.h"
#import <SystemConfiguration/SystemConfiguration.h>
#import <unistd.h>
/*
IMPORTANT
Code that appears in these tests is not for general purpose use. 
You should not use [networkQueue waitUntilAllOperationsAreFinished] or [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]] in your own software.
They are used here to force a queue to operate synchronously to simplify writing the tests.
IMPORTANT
*/

// Used for subclass test
@interface ASINetworkQueueSubclass : ASINetworkQueue {}
@end
@implementation ASINetworkQueueSubclass
@end

// Stop clang complaining about undeclared selectors
@interface ASINetworkQueueTests ()
- (void)queueFinished:(ASINetworkQueue *)request;
- (void)addedRequestComplete:(ASIHTTPRequest *)request;
- (void)addAnotherRequest;
- (void)immediateCancelFail:(ASIHTTPRequest *)request;
- (void)immediateCancelFinish:(ASIHTTPRequest *)request;
- (void)finish:(ASIHTTPRequest *)request;
- (void)throttleFail:(ASIHTTPRequest *)request;
- (void)postDone:(ASIHTTPRequest *)request;
- (void)ntlmDone:(ASIHTTPRequest *)request;
- (void)ntlmFailed:(ASIHTTPRequest *)request;
- (void)runHEADFailureTest;
- (void)queueFailureFinish:(ASINetworkQueue *)request;
- (void)queueFailureFinishCallOnce:(ASINetworkQueue *)request;
- (void)request:(ASIHTTPRequest *)request isGoingToRedirectToURL:(NSURL *)url;
- (void)redirectURLTestFailed:(ASIHTTPRequest *)request;
- (void)redirectURLTestSucceeded:(ASIHTTPRequest *)request;
- (void)runDelegateMethodsTest;
- (void)delegateTestStarted:(ASIHTTPRequest *)request;
- (void)delegateTestFinished:(ASIHTTPRequest *)request;
- (void)delegateTestFailed:(ASIHTTPRequest *)request;
- (void)delegateTestRequest:(ASIHTTPRequest *)request receivedResponseHeaders:(NSDictionary *)headers;
- (void)addMoreRequestsQueueFinished:(ASINetworkQueue *)request;
- (void)requestFailedCancellingOthers:(ASINetworkQueue *)request;
- (void)fail:(ASIHTTPRequest *)request;
- (void)HEADFail:(ASIHTTPRequest *)request;
- (void)runTestQueueFinishedCalledOnFailureTest;
@end

@implementation ASINetworkQueueTests

- (void)testDelegateAuthenticationCredentialsReuse
{
	complete = NO;
	authenticationPromptCount = 0;

	ASINetworkQueue *networkQueue = [ASINetworkQueue queue];
	[networkQueue setDelegate:self];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	NSDictionary *userInfo = [NSDictionary dictionaryWithObject:@"reuse" forKey:@"test"];
	
	int i;
	for (i=0; i<5; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/basic-authentication"]];
		[request setUserInfo:userInfo];
		[networkQueue addOperation:request];
	}
	[networkQueue go];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
}


- (void)testDelegateMethods
{
	[self performSelectorOnMainThread:@selector(runDelegateMethodsTest) withObject:nil waitUntilDone:YES];
}

- (void)runDelegateMethodsTest
{
	started = NO;
	finished = NO;
	failed = NO;

	ASINetworkQueue *networkQueue = [ASINetworkQueue queue];
	[networkQueue setDelegate:self];
	[networkQueue setRequestDidStartSelector:@selector(delegateTestStarted:)];
	[networkQueue setRequestDidFinishSelector:@selector(delegateTestFinished:)];
	[networkQueue setRequestDidReceiveResponseHeadersSelector:@selector(delegateTestRequest:receivedResponseHeaders:)];
	
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[networkQueue addOperation:request];
	[networkQueue go];
	
	[networkQueue waitUntilAllOperationsAreFinished];
	
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:2]];

	GHAssertTrue(started,@"Failed to call the delegate method when the request started");
	GHAssertTrue(receivedResponseHeaders,@"Failed to call the delegate method when the request received response headers");
	GHAssertTrue(finished,@"Failed to call the delegate method when the request finished");
	
	networkQueue = [ASINetworkQueue queue];
	[networkQueue setDelegate:self];
	[networkQueue setRequestDidFailSelector:@selector(delegateTestFailed:)];
	
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_%28abridged%29.txt"]];
	[request setTimeOutSeconds:0.01];
	[networkQueue addOperation:request];
	[networkQueue go];
	
	[networkQueue waitUntilAllOperationsAreFinished];

	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:2]];
	
	GHAssertTrue(failed,@"Failed to call the delegate method when the request failed");
	
}

- (void)delegateTestStarted:(ASIHTTPRequest *)request
{
	started = YES;
}

- (void)delegateTestRequest:(ASIHTTPRequest *)request receivedResponseHeaders:(NSDictionary *)responseHeaders
{
	GHAssertNotNil(responseHeaders,@"Called delegateTestResponseHeaders: when we have no headers");
	receivedResponseHeaders = YES;
}

- (void)delegateTestFinished:(ASIHTTPRequest *)request
{
	finished = YES;
}

- (void)delegateTestFailed:(ASIHTTPRequest *)request
{
	failed = YES;
}


- (void)testDownloadProgress
{
	complete = NO;
	progress = 0;
	
	ASINetworkQueue *networkQueue = [ASINetworkQueue queue];
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setDelegate:self];
	[networkQueue setShowAccurateProgress:NO];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
		
	int i;
	for (i=0; i<5; i++) {
		NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/i/logo.png"] autorelease];
		ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
		[networkQueue addOperation:request];
	}
	[networkQueue go];
		
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
	BOOL success = (progress == 1.0);
	GHAssertTrue(success,@"Failed to increment progress properly");
	

	//Now test again with accurate progress
	complete = NO;
	progress = 0;
	[networkQueue cancelAllOperations];
	[networkQueue setShowAccurateProgress:YES];
	
	for (i=0; i<5; i++) {
		NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/i/logo.png"] autorelease];
		ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
		[networkQueue addOperation:request];
	}	
	[networkQueue go];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	
	success = (progress == 1.0);
	GHAssertTrue(success,@"Failed to increment progress properly");
	
}

- (void)testAccurateProgressFallsBackToSimpleProgress
{
	
	ASINetworkQueue *networkQueue = [ASINetworkQueue queue];
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setDelegate:self];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	// Test accurate progress falls back to simpler progress when responses have no content-length header
	complete = NO;
	progress = 0;
	[networkQueue setShowAccurateProgress:YES];
	
	int i;
	for (i=0; i<5; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
		[request setAllowCompressedResponse:NO]; // A bit hacky - my server will send a chunked response (without content length) when we don't specify that we accept gzip
		[networkQueue addOperation:request];
	}	
	[networkQueue go];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	
	BOOL success = (progress == 1.0);
	GHAssertTrue(success,@"Failed to increment progress properly");
	
	[networkQueue reset];
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setDelegate:self];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	// This test will request gzipped content, but the content-length header we get on the HEAD request will be wrong, ASIHTTPRequest should fall back to simple progress
	// This is to workaround an issue Apache has with HEAD requests for dynamically generated content when accepting gzip - it returns the content-length of a gzipped empty body
	complete = NO;
	progress = 0;
	[networkQueue setShowAccurateProgress:YES];
	
	for (i=0; i<5; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
		[networkQueue addOperation:request];
	}	
	[networkQueue go];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	
	success = (progress == 1.0);
	GHAssertTrue(success,@"Failed to increment progress properly");
}

- (void)testAddingRequestsToQueueWhileInProgress
{
	[[self addMoreRequestsQueue] reset];
	[self setAddMoreRequestsQueue:[ASINetworkQueue queue]];
	[[self addMoreRequestsQueue] setDownloadProgressDelegate:self];
	[[self addMoreRequestsQueue] setDelegate:self];
	[[self addMoreRequestsQueue] setShowAccurateProgress:YES];
	[[self addMoreRequestsQueue]setQueueDidFinishSelector:@selector(addMoreRequestsQueueFinished:)];	
	
	requestsFinishedCount = 0;
	
	complete = NO;
	progress = 0;
	
	int i;
	for (i=0; i<5; i++) {
		NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_(abridged).txt"] autorelease];
		ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
		[request setDelegate:self];
		[request setDidFinishSelector:@selector(addedRequestComplete:)];
		[[self addMoreRequestsQueue] addOperation:request];
	}	
	[[self addMoreRequestsQueue] go];
	
	// Add another request to the queue each second for 5 seconds
	addedRequests = 0;
	for (i=0; i<5; i++) {
		[self performSelector:@selector(addAnotherRequest) withObject:nil afterDelay:i];
	}
	
	while (addedRequests < 5) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
	}
	
	// Must wait or subsequent tests will reset our progress
	[[self addMoreRequestsQueue] waitUntilAllOperationsAreFinished];
}

- (void)addAnotherRequest
{
	addedRequests++;
	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_(abridged).txt"] autorelease];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setDelegate:self];
	[request setDidFinishSelector:@selector(addedRequestComplete:)];
	[[self addMoreRequestsQueue] addOperation:request];
}

- (void)addedRequestComplete:(ASIHTTPRequest *)request
{
	requestsFinishedCount++;
}

- (void)addMoreRequestsQueueFinished:(ASINetworkQueue *)queue
{
	// This might get called multiple times if the queue finishes before all the requests can be added
	// So we'll make sure they're all done first
	while (requestsFinishedCount < 10) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1.0]];
	}

	BOOL success = (progress == 1.0);
	GHAssertTrue(success,@"Failed to increment progress properly");	
	
	// The file we downloaded 10 times is 130050 bytes long
	success = ([queue totalBytesToDownload] == 130050*10);
	GHAssertTrue(success,@"Failed to increment total download size properly");	
}


- (void)uploadFailed:(ASIHTTPRequest *)request
{
	GHFail(@"Failed to upload some data, cannot continue with this test");
}

- (void)testUploadProgress
{
	complete = NO;
	progress = 0;
	
	ASINetworkQueue *networkQueue = [[[ASINetworkQueue alloc] init] autorelease];
	[networkQueue setUploadProgressDelegate:self];
	[networkQueue setDelegate:self];
	[networkQueue setShowAccurateProgress:NO];
	[networkQueue setRequestDidFailSelector:@selector(uploadFailed:)];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ignore"];
	
	int fileSizes[3] = {16,64,257};
	int i;
	for (i=0; i<3; i++) {
		NSData *data = [[[NSMutableData alloc] initWithLength:fileSizes[i]*1024] autorelease];
		NSString *path = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:[NSString stringWithFormat:@"file%hi",i]];
		[data writeToFile:path atomically:NO];
		ASIFormDataRequest *request = [[[ASIFormDataRequest alloc] initWithURL:url] autorelease];
		[request setFile:path forKey:@"file"];
		[networkQueue addOperation:request];	
	}
	
	[networkQueue go];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
	BOOL success = (progress == 1.0f);
	GHAssertTrue(success,@"Failed to increment progress properly");
	
	//Now test again with accurate progress
	complete = NO;
	progress = 0;
	[networkQueue reset];
	[networkQueue setUploadProgressDelegate:self];
	[networkQueue setDelegate:self];
	[networkQueue setShowAccurateProgress:NO];
	[networkQueue setRequestDidFailSelector:@selector(uploadFailed:)];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	[networkQueue setShowAccurateProgress:YES];
	
	for (i=0; i<3; i++) {
		NSData *data = [[[NSMutableData alloc] initWithLength:fileSizes[i]*1024] autorelease];
		NSString *path = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:[NSString stringWithFormat:@"file%hi",i]];
		[data writeToFile:path atomically:NO];
		ASIFormDataRequest *request = [[[ASIFormDataRequest alloc] initWithURL:url] autorelease];
		[request setFile:path forKey:@"file"];
		[networkQueue addOperation:request];	
	}
	
	[networkQueue go];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
	success = (progress == 1.0f);
	GHAssertTrue(success,@"Failed to increment progress properly");
	
}

// Will be called on Mac OS
- (void)setDoubleValue:(double)newProgress;
{
	progress = (float)newProgress;
}

// Will be called on iPhone OS
- (void)setProgress:(float)newProgress;
{
	progress = newProgress;
}


- (void)testFailure
{
	complete = NO;
	
	ASINetworkQueue *networkQueue = [ASINetworkQueue queue];
	[networkQueue setDelegate:self];
	[networkQueue setRequestDidFailSelector:@selector(requestFailed:)];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];
	[networkQueue setShouldCancelAllRequestsOnFailure:NO];
	
	NSURL *url;	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/first"] autorelease];
	ASIHTTPRequest *request1 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request1];
	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/second"] autorelease];
	ASIHTTPRequest *request2 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request2];
	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/third"] autorelease];
	ASIHTTPRequest *request3 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request3];
	
	url = [[[NSURL alloc] initWithString:@""] autorelease];
	requestThatShouldFail = [[ASIHTTPRequest alloc] initWithURL:url];
	[networkQueue addOperation:requestThatShouldFail];

	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/broken"] autorelease];
	ASIHTTPRequest *request5 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request5];

	[networkQueue go];
	
	[networkQueue waitUntilAllOperationsAreFinished];
	
	
	BOOL success;
	success = ([request1 error] == nil);
	GHAssertTrue(success,@"Request 1 failed");
	
	success = [[request1 responseString] isEqualToString:@"This is the expected content for the first string"];
	GHAssertTrue(success,@"Failed to download the correct data for request 1");
	
	success = ([request2 error] == nil);
	GHAssertTrue(success,@"Request 2 failed");
	
	success = [[request2 responseString] isEqualToString:@"This is the expected content for the second string"];
	GHAssertTrue(success,@"Failed to download the correct data for request 2");
	
	success = ([request3 error] == nil);
	GHAssertTrue(success,@"Request 3 failed");
	
	success = [[request3 responseString] isEqualToString:@"This is the expected content for the third string"];
	GHAssertTrue(success,@"Failed to download the correct data for request 3");
	
	success = ([requestThatShouldFail error] != nil);
	GHAssertTrue(success,@"Request 4 succeed when it should have failed");
	
	success = ([request5 error] == nil);
	GHAssertTrue(success,@"Request 5 failed");
	
	success = ([request5 responseStatusCode] == 404);
	GHAssertTrue(success,@"Failed to obtain the correct status code for request 5");

	[requestThatShouldFail release];
	
}


- (void)testFailureCancelsOtherRequests
{
	complete = NO;
	
	ASINetworkQueue *networkQueue = [ASINetworkQueue queue];
	[networkQueue setDelegate:self];
	[networkQueue setRequestDidFailSelector:@selector(requestFailedCancellingOthers:)];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	NSURL *url;	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/first"] autorelease];
	ASIHTTPRequest *request1 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request1];
	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/second"] autorelease];
	ASIHTTPRequest *request2 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request2];
	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/third"] autorelease];
	ASIHTTPRequest *request3 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request3];
	
	url = [[[NSURL alloc] initWithString:@""] autorelease];
	requestThatShouldFail = [[ASIHTTPRequest alloc] initWithURL:url];
	[networkQueue addOperation:requestThatShouldFail];

	[networkQueue go];
	
	[networkQueue waitUntilAllOperationsAreFinished];
	
	
	[requestThatShouldFail release];	
}

 
- (void)requestFailedCancellingOthers:(ASIHTTPRequest *)request
{
	complete = YES;
}

- (void)requestFailed:(ASIHTTPRequest *)request
{
	BOOL success = (request == requestThatShouldFail);
	GHAssertTrue(success,@"Wrong request failed");
}

- (void)queueFinished:(ASINetworkQueue *)queue
{
	complete = YES;
}

- (void)testProgressWithAuthentication
{
	complete = NO;
	progress = 0;
	
	// Make sure we don't re-use credentials from previous tests
	[ASIHTTPRequest clearSession];
	
	ASINetworkQueue *networkQueue = [ASINetworkQueue queue];
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setDelegate:self];
	[networkQueue setShowAccurateProgress:YES];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	[networkQueue setRequestDidFailSelector:@selector(requestFailedCancellingOthers:)];
	
	NSURL *url;	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/basic-authentication"] autorelease];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUserInfo:[NSDictionary dictionaryWithObject:@"Don't bother" forKey:@"Shall I return any credentials?"]];
	[networkQueue addOperation:request];
	
	[networkQueue go];
	

	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}

	NSError *error = [request error];
	GHAssertNotNil(error,@"The HEAD request failed, but it didn't tell the main request to fail");	
	
	complete = NO;
	progress = 0;	
	networkQueue = [ASINetworkQueue queue];
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setDelegate:self];
	[networkQueue setShowAccurateProgress:YES];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	[networkQueue setRequestDidFailSelector:@selector(requestFailed:)];
	
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUserInfo:[NSDictionary dictionaryWithObject:@"Don't bother" forKey:@"Shall I return any credentials?"]];
	[request setUsername:@"secret_username"];
	[request setPassword:@"secret_password"];
	[networkQueue addOperation:request];
	
	[networkQueue go];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	
	error = [request error];
	GHAssertNil(error,@"Failed to use authentication in a queue");	
	
}

- (void)testDelegateAuthentication
{
	complete = NO;
	ASINetworkQueue *networkQueue = [ASINetworkQueue queue];
	[networkQueue setDelegate:self];
	[networkQueue setRequestDidFinishSelector:@selector(queueFinished:)];
	
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/basic-authentication"]];
	[networkQueue addOperation:request];
	
	[networkQueue go];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	
	NSError *error = [request error];
	GHAssertNil(error,@"Request failed");	
}


- (void)authenticationNeededForRequest:(ASIHTTPRequest *)request
{
	// We're using this method in multiple tests, so the code here is to act appropriatly for each one
	
	if ([[[request userInfo] objectForKey:@"test"] isEqualToString:@"ntlm"]) {
		authenticationPromptCount++;
		if (authenticationPromptCount == 5) {
			[request setUsername:@"king"];
			[request setPassword:@"crown"];
			[request setDomain:@"CASTLE.KINGDOM"];
		}
		[request retryUsingSuppliedCredentials];
	
	} else if ([[[request userInfo] objectForKey:@"test"] isEqualToString:@"reuse"]) {
		authenticationPromptCount++;
		BOOL success = (authenticationPromptCount == 1);
		GHAssertTrue(success,@"Delegate was asked for credentials more than once");
		
		[request setUsername:@"secret_username"];
		[request setPassword:@"secret_password"];
		[request retryUsingSuppliedCredentials];
		
	} else if ([[[request userInfo] objectForKey:@"test"] isEqualToString:@"delegate-auth-failure"]) {
		authenticationPromptCount++;
		if (authenticationPromptCount == 5) {
			[request setUsername:@"secret_username"];
			[request setPassword:@"secret_password"];
		} else {
			[request setUsername:@"wrong_username"];
			[request setPassword:@"wrong_password"];
		}
		[request retryUsingSuppliedCredentials];
			

	// testProgressWithAuthentication will set a userInfo dictionary on the main request, to tell us not to supply credentials
	} else if (![request mainRequest] || ![[request mainRequest] userInfo]) {
		[request setUsername:@"secret_username"];
		[request setPassword:@"secret_password"];
		[request retryUsingSuppliedCredentials];
	} else {
		[request cancelAuthentication];
	}
}

- (void)requestFailedExpectedly:(ASIHTTPRequest *)request
{
    request_didfail = YES;
    BOOL success = (request == requestThatShouldFail);
    GHAssertTrue(success,@"Wrong request failed");
}

- (void)requestSucceededUnexpectedly:(ASIHTTPRequest *)request
{
    request_succeeded = YES;
}

//Connect to a port the server isn't listening on, and the read stream won't be created (Test + Fix contributed by Michael Krause)
- (void)testWithNoListener
{
    request_succeeded = NO;
    request_didfail = NO;
	ASINetworkQueue *networkQueue = [ASINetworkQueue queue];
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setDelegate:self];
	[networkQueue setShowAccurateProgress:YES];
    [networkQueue setRequestDidFailSelector:@selector(requestFailedExpectedly:)];
    [networkQueue setRequestDidFinishSelector:@selector(requestSucceededUnexpectedly:)];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	NSURL *url;	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com:9999/i/logo.png"] autorelease];
	requestThatShouldFail = [[ASIHTTPRequest alloc] initWithURL:url];
	[networkQueue addOperation:requestThatShouldFail];
	
	[networkQueue go];
	[networkQueue waitUntilAllOperationsAreFinished];
    
	// Give the queue time to notify us
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
	
	// This test may fail if you are using a proxy and it returns a page when you try to connect to a bad port.
	GHAssertTrue(!request_succeeded && request_didfail,@"Request to resource without listener succeeded but should have failed (May fail with proxy!)");
    
}

- (void)testPartialResume
{
	complete = NO;
	progress = 0;
	
	NSString *temporaryPath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"the_great_american_novel_%28young_readers_edition%29.txt.download"];
	if ([[NSFileManager defaultManager] fileExistsAtPath:temporaryPath]) {
		[[NSFileManager defaultManager] removeItemAtPath:temporaryPath error:nil];
	}
	
	NSString *downloadPath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"the_great_american_novel_%28young_readers_edition%29.txt"];
	if ([[NSFileManager defaultManager] fileExistsAtPath:downloadPath]) {
		[[NSFileManager defaultManager] removeItemAtPath:downloadPath error:nil];
	}
	
	NSURL *downloadURL = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_%28young_readers_edition%29.txt"];
	ASINetworkQueue *networkQueue = [ASINetworkQueue queue];
	[networkQueue setShouldCancelAllRequestsOnFailure:NO];

	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:downloadURL] autorelease];
	[request setDownloadDestinationPath:downloadPath];
	[request setTemporaryFileDownloadPath:temporaryPath];
	[request setAllowResumeForFileDownloads:YES];
	[networkQueue addOperation:request];
	[networkQueue go];
	 
	// Run until we have received a bit of data
	while (1) {
		usleep(250000);
		if ([request error] || [request contentLength]) {
			break;
		}
	}
	
	// Ok, let's tell the queue to stop
	[networkQueue reset];
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setShowAccurateProgress:YES];
	[networkQueue setDelegate:self];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	complete = NO;
	progress = 0;
	
	NSError *err = nil;
	unsigned long long downloadedSoFar = [[[NSFileManager defaultManager] attributesOfItemAtPath:temporaryPath error:&err] fileSize];
	GHAssertNil(err,@"Got an error obtaining attributes on the file, this shouldn't happen");
	
	BOOL success = (downloadedSoFar > 0);
	GHAssertTrue(success,@"Failed to download part of the file, so we can't proceed with this test");
	
	request = [[[ASIHTTPRequest alloc] initWithURL:downloadURL] autorelease];
	[request setDownloadDestinationPath:downloadPath];
	[request setTemporaryFileDownloadPath:temporaryPath];
	[request setAllowResumeForFileDownloads:YES];
	
	[networkQueue addOperation:request];

	[networkQueue go];

	[networkQueue waitUntilAllOperationsAreFinished];
	
	unsigned long long amountDownloaded = [[[NSFileManager defaultManager] attributesOfItemAtPath:downloadPath error:&err] fileSize];
	GHAssertNil(err,@"Got an error obtaining attributes on the file, this shouldn't happen");
	success = (amountDownloaded == 1036935);
	GHAssertTrue(success,@"Failed to complete the download");
	
	success = (progress == 1.0f);
	GHAssertTrue(success,@"Failed to increment progress properly");
	

	
	//Test the temporary file cleanup
	downloadURL = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel.txt"];
	complete = NO;
	progress = 0;
	networkQueue = [ASINetworkQueue queue];
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setShowAccurateProgress:YES];
	[networkQueue setDelegate:self];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	request = [[[ASIHTTPRequest alloc] initWithURL:downloadURL] autorelease];
	[request setDownloadDestinationPath:downloadPath];
	[request setTemporaryFileDownloadPath:temporaryPath];
	[request setAllowResumeForFileDownloads:YES];
	[networkQueue addOperation:request];
	[networkQueue go];
	
	// Run until we have received a bit of data
	while (1) {
		usleep(1000000);
		if ([request error] || [request contentLength]) {
			break;
		}
	}
	[networkQueue cancelAllOperations];
	
	success = ([[NSFileManager defaultManager] fileExistsAtPath:temporaryPath]);
	GHAssertTrue(success,@"Temporary download file doesn't exist");	
	
	[request removeTemporaryDownloadFile];
	
	success = (![[NSFileManager defaultManager] fileExistsAtPath:temporaryPath]);
	GHAssertTrue(success,@"Temporary download file should have been deleted");		
	
	
}

- (void)stopQueue:(id)sender
{
	complete = YES;
}



// Not strictly an ASINetworkQueue test, but queue related
// As soon as one request finishes or fails, we'll cancel the others and ensure that no requests are both finished and failed
- (void)testImmediateCancel
{
	[self setFailedRequests:[NSMutableArray array]];
	[self setFinishedRequests:[NSMutableArray array]];
	[self setImmediateCancelQueue:[[[NSOperationQueue alloc] init] autorelease]];
	int i;
	for (i=0; i<10; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
		[request setDelegate:self];
		[request setDidFailSelector:@selector(immediateCancelFail:)];
		[request setDidFinishSelector:@selector(immediateCancelFinish:)];
		[[self immediateCancelQueue] addOperation:request];
	}
	
}

- (void)immediateCancelFail:(ASIHTTPRequest *)request
{
	if ([[self failedRequests] containsObject:request]) {
		GHFail(@"A request called its fail delegate method twice");
	}
	if ([[self finishedRequests] containsObject:request]) {
		GHFail(@"A request that had already finished called its fail delegate method");
	}
	[[self failedRequests] addObject:request];
	if ([[self failedRequests] count]+[[self finishedRequests] count] > 25) {
		GHFail(@"We got more than 25 delegate fail/finish calls - this shouldn't happen!");
	}
	[[self immediateCancelQueue] cancelAllOperations];

}

- (void)immediateCancelFinish:(ASIHTTPRequest *)request
{
	if ([[self finishedRequests] containsObject:request]) {
		GHFail(@"A request called its finish delegate method twice");
	}
	if ([[self failedRequests] containsObject:request]) {
		GHFail(@"A request that had already failed called its finish delegate method");
	}
	[[self finishedRequests] addObject:request];
	if ([[self failedRequests] count]+[[self finishedRequests] count] > 25) {
		GHFail(@"We got more than 25 delegate fail/finish calls - this shouldn't happen!");
	}
	[[self immediateCancelQueue] cancelAllOperations];
}

// Ensure class convenience constructor returns an instance of our subclass
- (void)testSubclass
{
	ASINetworkQueueSubclass *instance = [ASINetworkQueueSubclass queue];
	BOOL success = [instance isKindOfClass:[ASINetworkQueueSubclass class]];
	GHAssertTrue(success,@"Convenience constructor failed to return an instance of the correct class");	
}


// Test releasing the queue in a couple of ways - the purpose of these tests is really just to ensure we don't crash
- (void)testQueueReleaseOnRequestComplete
{
	[[self releaseTestQueue] cancelAllOperations];
	[self setReleaseTestQueue:[ASINetworkQueue queue]];
	int i;
	for (i=0; i<5; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
		[request setDelegate:self];
		[request setDidFailSelector:@selector(fail:)];
		[request setDidFinishSelector:@selector(finish:)];
		[[self releaseTestQueue] addOperation:request];
	}
}

- (void)fail:(ASIHTTPRequest *)request
{
	if ([[self releaseTestQueue] requestsCount] == 0) {
		[self setReleaseTestQueue:nil];
	}
}
 
- (void)finish:(ASIHTTPRequest *)request
{
	if ([[self releaseTestQueue] requestsCount] == 0) {
		[self setReleaseTestQueue:nil];
	}	
}

- (void)testQueueReleaseOnQueueComplete
{
	[[self releaseTestQueue] cancelAllOperations];
	[self setReleaseTestQueue:[ASINetworkQueue queue]];
	[[self releaseTestQueue] setDelegate:self];
	int i;
	for (i=0; i<5; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
		[[self releaseTestQueue] addOperation:request];
	}
}

- (void)queueComplete:(ASINetworkQueue *)queue
{
	[self setReleaseTestQueue:nil];
}

- (void)testMultipleDownloadsThrottlingBandwidth
{
	complete = NO;
	
	[ASIHTTPRequest setMaxBandwidthPerSecond:0];
	
	ASINetworkQueue *networkQueue = [ASINetworkQueue queue];
	[networkQueue setDelegate:self];
	[networkQueue setRequestDidFailSelector:@selector(throttleFail:)];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];
	
	// We'll test first without throttling
	int i;
	for (i=0; i<5; i++) {
		// This image is around 18KB in size, for 90KB total download size
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/i/logo.png"]];
		[networkQueue addOperation:request];
	}
	
	NSDate *date = [NSDate date];
	[networkQueue go];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	
	
	NSTimeInterval interval =[date timeIntervalSinceNow];
	BOOL success = (interval > -6);
	GHAssertTrue(success,@"Downloaded the data too slowly - either this is a bug, or your internet connection is too slow to run this test (must be able to download 90KB in less than 6 seconds, without throttling)");

	
	// Reset the queue
	[networkQueue reset];
	[networkQueue setDelegate:self];
	[networkQueue setRequestDidFailSelector:@selector(throttleFail:)];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];
	complete = NO;
	
	// Now we'll test with throttling
	[ASIHTTPRequest setMaxBandwidthPerSecond:ASIWWANBandwidthThrottleAmount];
	
	for (i=0; i<5; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/i/logo.png"]];
		[networkQueue addOperation:request];
	}
	
	date = [NSDate date];
	[networkQueue go];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	
	[ASIHTTPRequest setMaxBandwidthPerSecond:0];
	
	interval =[date timeIntervalSinceNow];
	success = (interval < -6);
	GHAssertTrue(success,@"Failed to throttle upload");		
	
}

- (void)testMultipleUploadsThrottlingBandwidth
{
	complete = NO;
	
	[ASIHTTPRequest setMaxBandwidthPerSecond:0];
	
	ASINetworkQueue *networkQueue = [ASINetworkQueue queue];
	[networkQueue setDelegate:self];
	[networkQueue setRequestDidFailSelector:@selector(throttleFail:)];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];

	// Create a 16KB request body
	NSData *data = [[[NSMutableData alloc] initWithLength:16*1024] autorelease];

	// We'll test first without throttling
	int i;
	for (i=0; i<10; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ignore"]];
		[request appendPostData:data];
		[networkQueue addOperation:request];
	}
	
	NSDate *date = [NSDate date];
	[networkQueue go];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	
		
	NSTimeInterval interval =[date timeIntervalSinceNow];
	BOOL success = (interval > -10);
	GHAssertTrue(success,@"Uploaded the data too slowly - either this is a bug, or your internet connection is too slow to run this test (must be able to upload 160KB in less than 10 seconds, without throttling)");
	
	// Reset the queue
	[networkQueue reset];
	[networkQueue setDelegate:self];
	[networkQueue setRequestDidFailSelector:@selector(throttleFail:)];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];
	complete = NO;
	
	// Now we'll test with throttling
	[ASIHTTPRequest setMaxBandwidthPerSecond:ASIWWANBandwidthThrottleAmount];

	for (i=0; i<10; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ignore"]];
		[request appendPostData:data];
		[networkQueue addOperation:request];
	}
	
	date = [NSDate date];
	[networkQueue go];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
	
	[ASIHTTPRequest setMaxBandwidthPerSecond:0];
	
	interval =[date timeIntervalSinceNow];
	success = (interval < -10);
	GHAssertTrue(success,@"Failed to throttle upload");		
	
}
	 
- (void)throttleFail:(ASIHTTPRequest *)request
{
	GHAssertTrue(NO,@"Request failed, cannot continue with this test");
	[[request queue] cancelAllOperations];
}

// Test for a bug that used to exist where the temporary file used to store the request body would be removed when authentication failed
- (void)testPOSTWithAuthentication
{
	[[self postQueue] cancelAllOperations];
	[self setPostQueue:[ASINetworkQueue queue]];
	[[self postQueue] setRequestDidFinishSelector:@selector(postDone:)];
	[[self postQueue] setDelegate:self];
	
	ASIFormDataRequest *request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/Tests/post_with_authentication"]];
	[request setPostValue:@"This is the first item" forKey:@"first"];
	[request setData:[@"This is the second item" dataUsingEncoding:NSUTF8StringEncoding] forKey:@"second"];
	[[self postQueue] addOperation:request];
	[[self postQueue] go];
}

- (void)postDone:(ASIHTTPRequest *)request
{
	BOOL success = [[request responseString] isEqualToString:@"This is the first item\r\nThis is the second item"];
	GHAssertTrue(success,@"Didn't post correct data");	
}

- (void)testDelegateAuthenticationFailure
{
	[[self postQueue] cancelAllOperations];
	[self setPostQueue:[ASINetworkQueue queue]];
	[[self postQueue] setRequestDidFinishSelector:@selector(postDone:)];
	[[self postQueue] setDelegate:self];
	
	ASIFormDataRequest *request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/Tests/post_with_authentication"]];
	[request setPostValue:@"This is the first item" forKey:@"first"];
	[request setData:[@"This is the second item" dataUsingEncoding:NSUTF8StringEncoding] forKey:@"second"];
	[request setUserInfo:[NSDictionary dictionaryWithObject:@"delegate-auth-failure" forKey:@"test"]];
	[[self postQueue] addOperation:request];
	[[self postQueue] go];
}

- (void)testNTLMMultipleFailure
{
	authenticationPromptCount = 0;
	[ASIHTTPRequest clearSession];
	[[self testNTLMQueue] cancelAllOperations];
	[self setTestNTLMQueue:[ASINetworkQueue queue]];
	
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/pretend-ntlm-handshake"]];
	[request setUseKeychainPersistence:NO];
	[request setUseSessionPersistence:NO];
	[request setUserInfo:[NSDictionary dictionaryWithObject:@"ntlm" forKey:@"test"]];
	
	[[self testNTLMQueue] setRequestDidFinishSelector:@selector(ntlmDone:)];
	[[self testNTLMQueue] setRequestDidFailSelector:@selector(ntlmFailed:)];
	[[self testNTLMQueue] setDelegate:self];	
	[[self testNTLMQueue] addOperation:request];
	[[self testNTLMQueue] go];	
}

- (void)ntlmFailed:(ASIHTTPRequest *)request
{
	GHFail(@"Failed to provide NTLM credentials (error was :%@)",[request error]);	
}

- (void)ntlmDone:(ASIHTTPRequest *)request
{
	GHAssertNil([request error],@"Got an error when credentials were supplied");
	
	// NSProcessInfo returns a lower case string for host name, while CFNetwork will send a mixed case string for host name, so we'll compare by lowercasing everything
	NSString *hostName = [[NSProcessInfo processInfo] hostName];
	NSString *expectedResponse = [[NSString stringWithFormat:@"You are %@ from %@/%@",@"king",@"Castle.Kingdom",hostName] lowercaseString];
	BOOL success = [[[request responseString] lowercaseString] isEqualToString:expectedResponse];
	GHAssertTrue(success,@"Failed to send credentials correctly? (Expected: '%@', got '%@')",expectedResponse,[[request responseString] lowercaseString]);
}

- (void)testHEADFailure
{
	[self performSelectorOnMainThread:@selector(runHEADFailureTest) withObject:nil waitUntilDone:YES];
}

// Test for a bug where failing head requests would not notify the original request's delegate of the failure
- (void)runHEADFailureTest
{
	headFailed = NO;
	ASINetworkQueue *queue = [ASINetworkQueue queue];
	[queue setShowAccurateProgress:YES];
	
	[queue setDelegate:self];
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://"]];
	[request setDelegate:self];
	[request setDidFailSelector:@selector(HEADFail:)];
	[queue addOperation:request];
	[queue go];
	
	[queue waitUntilAllOperationsAreFinished];
	
	// Hope the request gets around to notifying the delegate in time
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:2]];
	
	GHAssertTrue(headFailed,@"Failed to notify the request's delegate");
}

- (void)HEADFail:(ASIHTTPRequest *)request
{
	headFailed = YES;	
}

- (void)testCopy
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	ASINetworkQueue *queue = [ASINetworkQueue queue];
	ASINetworkQueue *queue2 = [queue copy];
	GHAssertNotNil(queue2,@"Failed to create a copy");
	
	[pool release];
	
	BOOL success = ([queue2 retainCount] > 0);
	GHAssertTrue(success,@"Failed to create a retained copy");
	
	[queue2 release];
}

// Test for: http://allseeing-i.lighthouseapp.com/projects/27881/tickets/43-asinetworkqueue-and-setshouldcancelallrequestsonfailure-yes-will-not-trigger-queuefinished-selector-with-multiple-requests

- (void)testQueueFinishedCalledOnFailure
{
	[self performSelectorOnMainThread:@selector(runTestQueueFinishedCalledOnFailureTest) withObject:nil waitUntilDone:YES];
}
- (void)runTestQueueFinishedCalledOnFailureTest
{
	complete = NO;
	ASINetworkQueue *networkQueue = [[ASINetworkQueue queue] retain];
	[networkQueue setDelegate:self];
	[networkQueue setQueueDidFinishSelector:@selector(queueFailureFinish:)];
	
	NSUInteger i;
	for (i=0; i<10; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://there-is-no-spoon.allseeing-i.com"]];
		[networkQueue addOperation:request];
	}
	
	[networkQueue go];
	
	NSDate *dateStarted = [NSDate date];
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.5]];
		if ([dateStarted timeIntervalSinceNow] < -10) {
			break;
		}
	}
	GHAssertTrue(complete,@"Failed to call queue finished delegate method");
	
	queueFinishedCallCount = 0;
	complete = NO;

	[networkQueue release];
	networkQueue = [[ASINetworkQueue queue] retain];
	[networkQueue setDelegate:self];
	[networkQueue setQueueDidFinishSelector:@selector(queueFailureFinishCallOnce:)];
	[networkQueue setMaxConcurrentOperationCount:1];
	
	for (i=0; i<10; i++) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://there-is-no-spoon.allseeing-i.com"]];
		[networkQueue addOperation:request];
	}
	
	[networkQueue go];
	
	dateStarted = [NSDate date];
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:2.0f]];
		if ([dateStarted timeIntervalSinceNow] < -10) {
			break;
		}
	}
	BOOL success = (queueFinishedCallCount == 1);
	GHAssertTrue(success,@"Called the queue finish method more/less than once");
	
}

- (void)queueFailureFinishCallOnce:(ASINetworkQueue *)queue
{
	queueFinishedCallCount++;
	complete = YES;
}

- (void)queueFailureFinish:(ASINetworkQueue *)queue
{
	complete = YES;
}


- (void)testDelegateRedirectHandling
{
	ASINetworkQueue *networkQueue = [ASINetworkQueue queue];
	[networkQueue setDelegate:self];

	[networkQueue setRequestWillRedirectSelector:@selector(request:isGoingToRedirectToURL:)];
	[networkQueue setRequestDidFailSelector:@selector(redirectURLTestFailed:)];
	[networkQueue setRequestDidFinishSelector:@selector(redirectURLTestSucceeded:)];

	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect_to_ssl"]];

	[networkQueue addOperation:request];
	[networkQueue go];
}

- (void)redirectURLTestSucceeded:(ASIHTTPRequest *)request
{
	BOOL success = [[request url] isEqual:[NSURL URLWithString:@"http://allseeing-i.com"]];
	GHAssertTrue(success,@"Request failed to redirect to url specified by delegate");
}

- (void)redirectURLTestFailed:(ASIHTTPRequest *)request
{
	GHFail(@"Request failed, cannot proceed with test");
}

- (void)request:(ASIHTTPRequest *)request isGoingToRedirectToURL:(NSURL *)url
{
	[request redirectToURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
}

@synthesize immediateCancelQueue;
@synthesize failedRequests;
@synthesize finishedRequests;
@synthesize releaseTestQueue;
@synthesize cancelQueue;
@synthesize postQueue;
@synthesize testNTLMQueue;
@synthesize addMoreRequestsQueue;
@end

//
//  ASIHTTPRequestTests.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 01/08/2008.
//  Copyright 2008 All-Seeing Interactive. All rights reserved.
//

#import "ASIHTTPRequestTests.h"
#import "ASIHTTPRequest.h"
#import "ASINetworkQueue.h"
#import "ASIFormDataRequest.h"
#import <SystemConfiguration/SystemConfiguration.h>
#import <unistd.h>

// Used for subclass test
@interface ASIHTTPRequestSubclass : ASIHTTPRequest {}
@end
@implementation ASIHTTPRequestSubclass;

// For testing exceptions are caught
- (void)startRequest
{
	[[NSException exceptionWithName:@"Test Exception" reason:@"Test Reason" userInfo:nil] raise];
}
@end


// Stop clang complaining about undeclared selectors
@interface ASIHTTPRequestTests ()
- (void)runCancelTest;
- (void)performDelegateMethodsTest;
- (void)requestStarted:(ASIHTTPRequest *)request;
- (void)requestFinished:(ASIHTTPRequest *)request;
- (void)requestFailed:(ASIHTTPRequest *)request;
- (void)delegateTestStarted:(ASIHTTPRequest *)request;
- (void)delegateTestResponseHeaders:(ASIHTTPRequest *)request;
- (void)delegateTestFinished:(ASIHTTPRequest *)request;
- (void)delegateTestFailed:(ASIHTTPRequest *)request;
- (void)runRemoveUploadProgressTest;
- (void)runRedirectedResume;
- (void)performDownloadProgressTest;
- (void)theTestRequest:(ASIHTTPRequest *)request didReceiveData:(NSData *)data;
- (void)theTestRequestFinished:(ASIHTTPRequest *)request;
- (void)performUploadProgressTest;
- (void)performPostBodyStreamedFromDiskTest;
- (void)performPartialFetchTest;
- (void)asyncFail:(ASIHTTPRequest *)request;
- (void)asyncSuccess:(ASIHTTPRequest *)request;
- (void)request:(ASIHTTPRequest *)request isGoingToRedirectToURL:(NSURL *)url;
- (void)redirectURLTestFailed:(ASIHTTPRequest *)request;
- (void)redirectURLTestSucceeded:(ASIHTTPRequest *)request;
@end

@implementation ASIHTTPRequestTests

- (void)testBasicDownload
{
	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com"];
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:url];
	[request startSynchronous];
	NSString *html = [request responseString];
	GHAssertNotNil(html,@"Basic synchronous request failed");

	// Check we're getting the correct response headers
	NSString *pingBackHeader = [[request responseHeaders] objectForKey:@"X-Pingback"];
	BOOL success = [pingBackHeader isEqualToString:@"http://allseeing-i.com/Ping-Back"];
	GHAssertTrue(success,@"Failed to populate response headers");
	
	// Check we're getting back the correct status code
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/a-page-that-does-not-exist"] autorelease];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	success = ([request responseStatusCode] == 404);
	GHAssertTrue(success,@"Didn't get correct status code");	
	
	// Check data is as expected
	NSRange notFound = NSMakeRange(NSNotFound, 0);
	success = !NSEqualRanges([html rangeOfString:@"All-Seeing Interactive"],notFound);
	GHAssertTrue(success,@"Failed to download the correct data");
	
	// Attempt to grab from bad url
	url = [[[NSURL alloc] initWithString:@""] autorelease];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	success = [[request error] code] == ASIInternalErrorWhileBuildingRequestType;
	GHAssertTrue(success,@"Failed to generate an error for a bad host");
	
	request = [[[ASIHTTPRequest alloc] initWithURL:nil] autorelease];
	[request startSynchronous];
	success = [[request error] code] == ASIUnableToCreateRequestErrorType;
	GHAssertTrue(success,@"Failed to generate an error for a bad host");
}

- (void)testBase64Encode
{
	NSData *data = [@"Hello, world" dataUsingEncoding:NSUTF8StringEncoding];
	NSString *base64 = [ASIHTTPRequest base64forData:data];
	BOOL success = [base64 isEqualToString:@"SGVsbG8sIHdvcmxk"];
	GHAssertTrue(success,@"Failed to encode data using base64 data correctly");
}

- (void)testCancel
{
	// We run this test on the main thread because otherwise we can't depend on the  delegate being notified before we need to test it's working
	[self performSelectorOnMainThread:@selector(runCancelTest) withObject:nil waitUntilDone:YES];

}

- (void)runCancelTest
{
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_%28abridged%29.txt"]];
	[request startAsynchronous];
	[request cancel];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:2.0]];
	GHAssertNotNil([request error],@"Failed to cancel the request");
	
	// Test cancelling a redirected request works
	// This test is probably unreliable on very slow or very fast connections, as it depends on being able to complete the first request (but not the second) in under 2 seconds
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cancel_redirect"]];
	[request startAsynchronous];
	
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:2.0]];
	[request cancel];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:2.0]];
	
	BOOL success = ([[[request url] absoluteString] isEqualToString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel.txt"]);

	GHAssertTrue(success, @"Request did not redirect quickly enough, cannot proceed with test");
	
	GHAssertNotNil([request error],@"Failed to cancel the request");	 
	
	success = [request totalBytesRead] < 7900198;
	GHAssertTrue(success, @"Downloaded the whole of the response even though we should have cancelled by now");
	

}



- (void)testDelegateMethods
{
	// We run this test on the main thread because otherwise we can't depend on the  delegate being notified before we need to test it's working
	[self performSelectorOnMainThread:@selector(performDelegateMethodsTest) withObject:nil waitUntilDone:YES];
}

- (void)performDelegateMethodsTest
{
	started = NO;
	finished = NO;
	failed = NO;
	
	// Test default delegate methods
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request setDelegate:self];
	[request startSynchronous];

	
	GHAssertTrue(started,@"Failed to call the delegate method when the request started");	
	GHAssertTrue(receivedResponseHeaders,@"Failed to call the delegate method when the request started");	
	GHAssertTrue(finished,@"Failed to call the delegate method when the request finished");
	
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel.txt"]];
	[request setDelegate:self];
	[request setTimeOutSeconds:0.01];
	[request startSynchronous];
	
	GHAssertTrue(failed,@"Failed to call the delegate method when the request failed");
	
	started = NO;
	finished = NO;
	failed = NO;
	receivedResponseHeaders = NO;
	
	// Test custom delegate methods
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request setDelegate:self];
	[request setDidStartSelector:@selector(delegateTestStarted:)];
	[request setDidReceiveResponseHeadersSelector:@selector(delegateTestResponseHeaders:)];
	[request setDidFinishSelector:@selector(delegateTestFinished:)];
	[request startSynchronous];
	

	GHAssertTrue(started,@"Failed to call the delegate method when the request started");	
	GHAssertTrue(finished,@"Failed to call the delegate method when the request finished");
	
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel.txt"]];
	[request setDidFailSelector:@selector(delegateTestFailed:)];
	[request setDelegate:self];
	[request setTimeOutSeconds:0.01];
	[request startSynchronous];
	
	GHAssertTrue(failed,@"Failed to call the delegate method when the request failed");
	
}

- (void)requestStarted:(ASIHTTPRequest *)request
{
	started = YES;
}

- (void)request:(ASIHTTPRequest *)request didReceiveResponseHeaders:(NSDictionary *)newResponseHeaders
{
	GHAssertNotNil(newResponseHeaders,@"Called request:didReceiveResponseHeaders: when we have no headers");
	receivedResponseHeaders = YES;
}


- (void)requestFinished:(ASIHTTPRequest *)request
{
	finished = YES;
}

- (void)requestFailed:(ASIHTTPRequest *)request
{
	failed = YES;
}

- (void)delegateTestStarted:(ASIHTTPRequest *)request
{
	started = YES;
}

- (void)delegateTestResponseHeaders:(ASIHTTPRequest *)request
{
	GHAssertNotNil([request responseHeaders],@"Called delegateTestResponseHeaders: when we have no headers");
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

- (void)testConditionalGET
{
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/i/logo.png"]];
	[request startSynchronous];
	BOOL success = ([request responseStatusCode] == 200);
	GHAssertTrue(success, @"Failed to download file, cannot proceed with this test");
	success = ([[request responseData] length] > 0);
	GHAssertTrue(success, @"Response length is 0, this shouldn't happen");
	
	NSString *etag = [[request responseHeaders] objectForKey:@"Etag"];
	NSString *lastModified = [[request responseHeaders] objectForKey:@"Last-Modified"];
	
	GHAssertNotNil(etag, @"Response didn't return an etag");
	GHAssertNotNil(lastModified, @"Response didn't return a last modified date");
	
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/i/logo.png"]];
	[request addRequestHeader:@"If-Modified-Since" value:lastModified];
	[request addRequestHeader:@"If-None-Match" value:etag];
	[request startSynchronous];
	success = ([request responseStatusCode] == 304);
	GHAssertTrue(success, @"Got wrong status code");
	success = ([[request responseData] length] == 0);
	GHAssertTrue(success, @"Response length is not 0, this shouldn't happen");
	
}

- (void)testException
{
	ASIHTTPRequestSubclass *request = [ASIHTTPRequestSubclass requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request startSynchronous];
	NSError *error = [request error];
	GHAssertNotNil(error,@"Failed to generate an error for an exception");
	
	BOOL success = [[[error userInfo] objectForKey:NSLocalizedDescriptionKey] isEqualToString:@"Test Exception"];
	GHAssertTrue(success, @"Generated wrong error for exception");
	
}

- (void)testCharacterEncoding
{
	
	NSArray *IANAEncodings = [NSArray arrayWithObjects:@"UTF-8",@"US-ASCII",@"ISO-8859-1",@"UTF-16",nil];
	NSUInteger NSStringEncodings[] = {NSUTF8StringEncoding,NSASCIIStringEncoding,NSISOLatin1StringEncoding,NSUnicodeStringEncoding};
	
	NSUInteger i;
	for (i=0; i<[IANAEncodings count]; i++) {
		NSURL *url = [[[NSURL alloc] initWithString:[NSString stringWithFormat:@"http://allseeing-i.com/ASIHTTPRequest/tests/Character-Encoding/%@",[IANAEncodings objectAtIndex:i]]] autorelease];
		ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
		[request startSynchronous];
		BOOL success = [request responseEncoding] == NSStringEncodings[i];
		GHAssertTrue(success,[NSString stringWithFormat:@"Failed to use the correct text encoding for %@i",[IANAEncodings objectAtIndex:i]]);
	}
					 
	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/Character-Encoding/Something-else"] autorelease];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setDefaultResponseEncoding:NSWindowsCP1251StringEncoding];
	[request startSynchronous];
	BOOL success = [request responseEncoding] == [request defaultResponseEncoding];
	GHAssertTrue(success,[NSString stringWithFormat:@"Failed to use the default string encoding"]);
	
	// Will return a Content-Type header with charset in the middle of the value (Fix contributed by Roman Busyghin)
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/Character-Encoding/utf-16-with-type-header"] autorelease];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	success = [request responseEncoding] == NSUnicodeStringEncoding;
	GHAssertTrue(success,[NSString stringWithFormat:@"Failed to parse the content type header correctly"]);
}

- (void)testTimeOut
{
	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_%28abridged%29.txt"] autorelease];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setTimeOutSeconds:0.0001]; //It's pretty unlikely we will be able to grab the data this quickly, so the request should timeout
	[request startSynchronous];
	
	BOOL success = [[request error] code] == ASIRequestTimedOutErrorType;
	GHAssertTrue(success,@"Timeout didn't generate the correct error");
	
	[ASIHTTPRequest setDefaultTimeOutSeconds:0.0001];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	
	success = [[request error] code] == ASIRequestTimedOutErrorType;
	GHAssertTrue(success,@"Failed to change the default timeout");	
	
	[ASIHTTPRequest setDefaultTimeOutSeconds:10];
}


// Test fix for a bug that might have caused timeouts when posting data
- (void)testTimeOutWithoutDownloadDelegate
{
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_%28young_readers_edition%29.txt"]];
	[request setTimeOutSeconds:5];
	[request setShowAccurateProgress:NO];
	[request setPostBody:[NSMutableData dataWithData:[@"Small Body" dataUsingEncoding:NSUTF8StringEncoding]]];
	[request startSynchronous];
	
	GHAssertNil([request error],@"Generated an error (most likely a timeout) - this test might fail on high latency connections");	
}


- (void)testRequestMethod
{
	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/request-method"] autorelease];
	NSArray *methods = [[[NSArray alloc] initWithObjects:@"GET",@"POST",@"PUT",@"DELETE", nil] autorelease];
	for (NSString *method in methods) {
		ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
		[request setRequestMethod:method];
		[request startSynchronous];
		BOOL success = [[request responseString] isEqualToString:method];
		GHAssertTrue(success,@"Failed to set the request method correctly");	
	}
	
	// Test to ensure we don't change the request method when we have an unrecognised method already set
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setRequestMethod:@"FINK"];
	[request appendPostData:[@"King" dataUsingEncoding:NSUTF8StringEncoding]];
	[request buildPostBody];
	BOOL success = [[request requestMethod] isEqualToString:@"FINK"];
	GHAssertTrue(success,@"Erroneously changed request method");	
}

- (void)testHTTPVersion
{
	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/http-version"] autorelease];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	
	BOOL success = [[request responseString] isEqualToString:@"HTTP/1.1"];
	GHAssertTrue(success,@"Wrong HTTP version used (May fail when using a proxy that changes the HTTP version!)");
	
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseHTTPVersionOne:YES];
	[request startSynchronous];
	
	success = [[request responseString] isEqualToString:@"HTTP/1.0"];
	GHAssertTrue(success,@"Wrong HTTP version used (May fail when using a proxy that changes the HTTP version!)");	
}

- (void)testUserAgent
{
	// defaultUserAgentString will be nil if we haven't set a Bundle Name or Bundle Display Name
	if ([ASIHTTPRequest defaultUserAgentString]) {
		
		// Returns the user agent it received in the response body
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/user-agent"]];
		[request startSynchronous];
		BOOL success = [[request responseString] isEqualToString:[ASIHTTPRequest defaultUserAgentString]];
		GHAssertTrue(success,@"Failed to set the correct user agent");
	}

	NSString *customUserAgent = @"Ferdinand Fuzzworth's Magic Tent of Mystery";

	// Test specifying a custom user-agent for a single request
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/user-agent"]];
	[request addRequestHeader:@"User-Agent" value:customUserAgent];
	[request startSynchronous];
	BOOL success = [[request responseString] isEqualToString:customUserAgent];
	GHAssertTrue(success,@"Failed to set the correct user-agent for a single request");

	// Test again using userAgent
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/user-agent"]];
	[request setUserAgentString:customUserAgent];
	[request startSynchronous];
	success = [[request responseString] isEqualToString:customUserAgent];
	GHAssertTrue(success,@"Failed to set the correct user-agent for a single request");

	// Test again to ensure user-agent not reused
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/user-agent"]];
	[request startSynchronous];
	success = ![[request responseString] isEqualToString:customUserAgent];
	GHAssertTrue(success,@"Re-used a user agent when we shouldn't have done so");

	// Test setting a custom default user-agent string
	[ASIHTTPRequest setDefaultUserAgentString:customUserAgent];
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/user-agent"]];
	[request startSynchronous];
	success = [[request responseString] isEqualToString:customUserAgent];
	GHAssertTrue(success,@"Failed to set the correct user-agent when using a custom default");

	[ASIHTTPRequest setDefaultUserAgentString:nil];
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/user-agent"]];
	[request startSynchronous];
	success = ![[request responseString] isEqualToString:customUserAgent];
	GHAssertTrue(success,@"Failed to clear a custom default user-agent");
}

- (void)testAutomaticRedirection
{
	ASIHTTPRequest *request;
	ASIFormDataRequest *request2;
	BOOL success;
	unsigned int i;
	for (i=301; i<308; i++) {
		
		if (i > 304 && i < 307) {
			continue;
		}
		NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect/%hi",i]];
		request = [ASIHTTPRequest requestWithURL:url];
		[request setShouldRedirect:NO];
		[request startSynchronous];
		if (i == 304) { // 304s will not contain a body, as per rfc2616. Will test 304 handling in a future test when we have etag support
			continue;
		}
		success = [[request responseString] isEqualToString:[NSString stringWithFormat:@"Non-redirected content with %hi status code",i]];
		GHAssertTrue(success,[NSString stringWithFormat:@"Got the wrong content when not redirecting after a %hi",i]);
	
		request2 = [ASIFormDataRequest requestWithURL:url];
		[request2 setPostValue:@"Giant Monkey" forKey:@"lookbehindyou"];
		[request2 startSynchronous];
		
		NSString *method = @"GET";
		if (i>304) {
			method = @"POST";	
		}
		NSString *expectedString = [NSString stringWithFormat:@"Redirected as %@ after a %hi status code",method,i];
		if (i>304) {
			expectedString = [NSString stringWithFormat:@"%@\r\nWatch out for the Giant Monkey!",expectedString];
		}

		success = [[request2 responseString] isEqualToString:expectedString];
		GHAssertTrue(success,[NSString stringWithFormat:@"Got the wrong content when redirecting after a %hi",i]);
	
		success = ([request2 responseStatusCode] == 200);
		GHAssertTrue(success,@"Got the wrong status code (expected 200)");

	}
	
	// Test RFC 2616 behaviour
	for (i=301; i<303; i++) {
		
		NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect/%hi",i]];
		request2 = [ASIFormDataRequest requestWithURL:url];
		[request2 setPostValue:@"Giant Monkey" forKey:@"lookbehindyou"];
		[request2 setShouldUseRFC2616RedirectBehaviour:YES];
		[request2 startSynchronous];
		
		success = ([request2 responseStatusCode] == 200);
		GHAssertTrue(success,@"Got the wrong status code (expected 200)");	

		if (i == 303) {
			success = ([request2 postLength] == 0 && ![request2 postBody] && [[request2 requestMethod] isEqualToString:@"GET"]);
			GHAssertTrue(success,@"Failed to reset request to GET on 303 redirect");
			
			success = [[request2 responseString] isEqualToString:[NSString stringWithFormat:@"Redirected as GET after a %hi status code",i]];
			GHAssertTrue(success,@"Failed to dump the post body on 303 redirect");
			
		} else {
			success = ([request2 postLength] > 0 || ![request2 postBody] || ![[request2 requestMethod] isEqualToString:@"POST"]);
			GHAssertTrue(success,@"Failed to use the same request method and body for a redirect when using rfc2616 behaviour");
		
			success = ([[request2 responseString] isEqualToString:[NSString stringWithFormat:@"Redirected as POST after a %hi status code\r\nWatch out for the Giant Monkey!",i]]);
			GHAssertTrue(success,@"Failed to send the correct post body on redirect");
		}
	}
	
	// Ensure the file contains only the body of the last request (after redirects) when downloading to a file
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect/301"]];
	NSString *path = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"test.txt"];
	[request setDownloadDestinationPath:path];
	[request startSynchronous];
	NSString *result = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
	success = [result isEqualToString:@"Redirected as GET after a 301 status code"];
	GHAssertTrue(success,@"Failed to store just the body of the file request on redirect");
	
	success = ([request originalURL] != [request url]);
	GHAssertTrue(success,@"Failed to update request url on redirection");
	
	success = ([[[request originalURL] absoluteString] isEqualToString:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect/301"]);
	GHAssertTrue(success,@"Failed to preserve original url");	

	// Ensure user agent is preserved
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect/301"]];
	[request addRequestHeader:@"User-Agent" value:@"test"];
	[request startSynchronous];
	success = ([[[request requestHeaders] objectForKey:@"User-Agent"] isEqualToString:@"test"]);
	GHAssertTrue(success,@"Failed to preserve original user agent on redirect");
}

// Using a persistent connection for HTTP 305-307 would cause crashes on the redirect, not really sure why
// Since 305 (use proxy) wasn't properly supported anyway, 306 is unused, and clients are supposed to confirm redirects for 307, I've simply removed automatic redirect for these codes
- (void)test30xCrash
{
	int i;
	for (i=305; i<308; i++) {
		ASIFormDataRequest *request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:[NSString stringWithFormat:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect/%hi",i]]];
		[request setPostValue:@"foo" forKey:@"eep"];
		[request setShouldRedirect:NO];
		[request startSynchronous];
		request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:[NSString stringWithFormat:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect/%hi",i]]];
		[request setPostValue:@"foo" forKey:@"eep"];
		[request startSynchronous];
	}
}

- (void)testResumeChecksContentRangeHeader
{
	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/no_resume"];
	NSString *temporaryPath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"foo.temp"];

	[@"" writeToFile:temporaryPath atomically:NO encoding:NSUTF8StringEncoding error:NULL];

	NSString *downloadPath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"foo.txt"];

	// Download part of a large file that is returned after a redirect
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:url];
	[request setTemporaryFileDownloadPath:temporaryPath];
	[request setDownloadDestinationPath:downloadPath];
	[request setAllowResumeForFileDownloads:YES];
	[request setAllowCompressedResponse:NO];
	[request setShouldAttemptPersistentConnection:NO];
	[request startAsynchronous];

	// Cancel the request as soon as it has downloaded 64KB
	while (1) {
		sleep(0.5);
		if ([request totalBytesRead] > 32*1024) {
			[request cancel];
			break;
		}
	}
	NSNumber *fileSize =  [[[NSFileManager defaultManager] attributesOfItemAtPath:temporaryPath error:NULL] objectForKey:NSFileSize];
	unsigned long long partialFileSize = [fileSize unsignedLongLongValue];
	BOOL success = (partialFileSize < 1036935);
	GHAssertTrue(success,@"Downloaded whole file too quickly, cannot proceed with this test");


	// Resume the download
	request = [ASIHTTPRequest requestWithURL:url];
	[request setTemporaryFileDownloadPath:temporaryPath];
	[request setDownloadDestinationPath:downloadPath];
	[request setAllowResumeForFileDownloads:YES];
	[request setAllowCompressedResponse:NO];

	[request buildRequestHeaders];
	success = ([request partialDownloadSize] == partialFileSize);
	GHAssertTrue(success,@"Failed to obtain correct partial dowload size");
	[request startAsynchronous];

	while (1) {
		sleep(0.5);
		if ([request isFinished]) {
			break;
		}
	}

	GHAssertNil([request error],@"Request failed, cannot proceed with this test");

	success = (![[request responseHeaders] objectForKey:@"Content-Range"]);
	GHAssertTrue(success,@"Got range header back, cannot proceed with this test");

	NSDictionary *attributes = [[NSFileManager defaultManager] attributesOfItemAtPath:downloadPath error:NULL];
	fileSize =  [attributes objectForKey:NSFileSize];
	success = ([fileSize intValue] == 1036935);
	GHAssertTrue(success,@"Downloaded file has wrong length");

	success = ([request partialDownloadSize] == 0);
	GHAssertTrue(success,@"Failed to reset download size");
}


- (void)testRedirectedResume
{
	[self performSelectorOnMainThread:@selector(runRedirectedResume) withObject:nil waitUntilDone:YES];
}

- (void)runRedirectedResume
{
	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect_resume"];
	NSString *temporaryPath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"foo.temp"];
	
	[@"" writeToFile:temporaryPath atomically:NO encoding:NSUTF8StringEncoding error:NULL];
	
	NSString *downloadPath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"foo.txt"];
	
	// Download part of a large file that is returned after a redirect
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:url];
	[request setTemporaryFileDownloadPath:temporaryPath];
	[request setDownloadDestinationPath:downloadPath];
	[request setAllowResumeForFileDownloads:YES];
	[request setAllowCompressedResponse:NO];
	[request startAsynchronous];
	
	// Cancel the request as soon as it has downloaded 64KB
	while (1) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.5]];
		if ([request totalBytesRead] > 32*1024) {
			[request cancel];
			break;
		}
	}
	NSNumber *fileSize =  [[[NSFileManager defaultManager] attributesOfItemAtPath:temporaryPath error:NULL] objectForKey:NSFileSize];
	unsigned long long partialFileSize = [fileSize unsignedLongLongValue];
	BOOL success = (partialFileSize < 1036935);
	GHAssertTrue(success,@"Downloaded whole file too quickly, cannot proceed with this test");


	// Resume the download synchronously
	request = [ASIHTTPRequest requestWithURL:url];
	[request setTemporaryFileDownloadPath:temporaryPath];
	[request setDownloadDestinationPath:downloadPath];
	[request setAllowResumeForFileDownloads:YES];
	[request setAllowCompressedResponse:NO];
	[request startSynchronous];
	
	fileSize =  [[[NSFileManager defaultManager] attributesOfItemAtPath:downloadPath error:NULL] objectForKey:NSFileSize];
	success = ([fileSize intValue] == 1036935);
	GHAssertTrue(success,@"Downloaded file has wrong length");
	
	success = [[[request requestHeaders] objectForKey:@"Range"] isEqualToString:[NSString stringWithFormat:@"bytes=%llu-",partialFileSize]];
	GHAssertTrue(success,@"Restarted download when we should have resumed, or asked for the wrong segment of the file");
	
}

- (void)testUploadContentLength
{
	//This url will return the contents of the Content-Length request header
	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/content-length"];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setPostBody:[NSMutableData dataWithLength:1024*32]];
	[request startSynchronous];
	
	BOOL success = ([[request responseString] isEqualToString:[NSString stringWithFormat:@"%hu",(1024*32)]]);
	GHAssertTrue(success,@"Sent wrong content length");
}

- (void)testDownloadContentLength
{
	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/i/logo.png"] autorelease];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	
	BOOL success = ([request contentLength] == 27872);
	GHAssertTrue(success,@"Got wrong content length");
}

- (void)testFileDownload
{
	NSString *path = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"testimage.png"];
	
	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/i/logo.png"] autorelease];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setDownloadDestinationPath:path];
	[request startSynchronous];

#if TARGET_OS_IPHONE
	UIImage *image = [[[UIImage alloc] initWithContentsOfFile:path] autorelease];
#else
	NSImage *image = [[[NSImage alloc] initWithContentsOfFile:path] autorelease];	
#endif
	
	GHAssertNotNil(image,@"Failed to download data to a file");
}


- (void)testCompressedResponseDownloadToFile
{
	NSString *path = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"testfile"];

	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/first"] autorelease];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setDownloadDestinationPath:path];
	[request startSynchronous];

	NSString *tempPath = [request temporaryFileDownloadPath];
	GHAssertNil(tempPath,@"Failed to clean up temporary download file");		

	BOOL success = [[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:NULL] isEqualToString:@"This is the expected content for the first string"];
	GHAssertTrue(success,@"Failed to download data to a file");

	// Now test with inflating the response on the fly
	NSError *error = nil;
	[ASIHTTPRequest removeFileAtPath:path error:&error];
	if (error) {
		GHFail(@"Failed to remove file, cannot proceed with test");
	}

	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setDownloadDestinationPath:path];
	[request setShouldWaitToInflateCompressedResponses:NO];
	[request startSynchronous];

	tempPath = [request temporaryFileDownloadPath];
	GHAssertNil(tempPath,@"Failed to clean up temporary download file");

	success = [[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:NULL] isEqualToString:@"This is the expected content for the first string"];
	GHAssertTrue(success,@"Failed to download data to a file");
}

- (void)request:(ASIHTTPRequest *)request didGetMoreData:(NSData *)data
{
	[[self responseData] appendData:data];
}

- (void)downloadFinished:(ASIHTTPRequest *)request
{
	finished = YES;
}

- (void)testCompressedResponseDelegateDataHandling
{
	finished = NO;
	[self setResponseData:[NSMutableData data]];

	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_hound_of_the_baskervilles.text"] autorelease];

	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];

	NSString *response = [request responseString];

	// Now download again, using the delegate to handle the data
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setDelegate:self];
	[request setDidReceiveDataSelector:@selector(request:didGetMoreData:)];
	[request setDidFinishSelector:@selector(downloadFinished:)];
	[request setShouldWaitToInflateCompressedResponses:NO];
	[request startSynchronous];

	while (!finished) {
		sleep(1);
	}

	NSString *delegateResponse = [[[NSString alloc] initWithBytes:[responseData bytes] length:[responseData length] encoding:[request responseEncoding]] autorelease];
	BOOL success = [delegateResponse isEqualToString:response];
	GHAssertTrue(success,@"Failed to correctly download the response using a delegate");

	// Test again without compression
	finished = NO;
	[self setResponseData:[NSMutableData data]];

	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setAllowCompressedResponse:NO];
	[request startSynchronous];

	response = [request responseString];

	// Now download again, using the delegate to handle the data
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setDelegate:self];
	[request setDidReceiveDataSelector:@selector(request:didGetMoreData:)];
	[request setDidFinishSelector:@selector(downloadFinished:)];
	[request setAllowCompressedResponse:NO];
	[request startSynchronous];

	while (!finished) {
		sleep(1);
	}

	delegateResponse = [[[NSString alloc] initWithBytes:[responseData bytes] length:[responseData length] encoding:[request responseEncoding]] autorelease];
	success = [delegateResponse isEqualToString:response];
	GHAssertTrue(success,@"Failed to correctly download the response using a delegate");
}


- (void)testDownloadProgress
{
	// We run tests that measure progress on the main thread because otherwise we can't depend on the progress delegate being notified before we need to test it's working
	[self performSelectorOnMainThread:@selector(performDownloadProgressTest) withObject:nil waitUntilDone:YES];
}

- (void)performDownloadProgressTest
{
	progress = 0;
	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/i/logo.png"] autorelease];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setDownloadProgressDelegate:self];
	[request startSynchronous];

	BOOL success = (progress == 1.0);
	GHAssertTrue(success,@"Failed to properly increment download progress %f != 1.0",progress);	
	
	progress = 0;
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel.txt"]];
	[request setDownloadProgressDelegate:self];
	[request startAsynchronous];
	
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:2]];
	
	success = (progress != 1.0);
	GHAssertTrue(success,@"Downloaded too quickly, cannot proceed with test");	
	 
	success = (progress > 0);
	GHAssertTrue(success,@"Either downloaded too slowly, or progress is not being correctly updated");		 
	
	
}

- (void)testUploadProgress
{
	// We run tests that measure progress on the main thread because otherwise we can't depend on the progress delegate being notified before we need to test it's working
	[self performSelectorOnMainThread:@selector(performUploadProgressTest) withObject:nil waitUntilDone:YES];	
}

- (void)performUploadProgressTest
{
	progress = 0;
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ignore"]] autorelease];
	[request setPostBody:(NSMutableData *)[@"This is the request body" dataUsingEncoding:NSUTF8StringEncoding]];
	[request setUploadProgressDelegate:self];
	[request startSynchronous];
	
	
	BOOL success = (progress == 1.0);
	GHAssertTrue(success,@"Failed to properly increment upload progress %f != 1.0",progress);	
}

- (void)testPostBodyStreamedFromDisk
{
	// We run tests that measure progress on the main thread because otherwise we can't depend on the progress delegate being notified before we need to test it's working
	[self performSelectorOnMainThread:@selector(performPostBodyStreamedFromDiskTest) withObject:nil waitUntilDone:YES];
	
}

- (void)performPostBodyStreamedFromDiskTest
{
	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/print_request_body"];
	NSString *requestBody = @"This is the request body";
	NSString *requestContentPath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"testfile.txt"];
	[[requestBody dataUsingEncoding:NSUTF8StringEncoding] writeToFile:requestContentPath atomically:NO];
	
	
	// Test using a user-specified file as the request body (useful for PUT)
	progress = 0;
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setRequestMethod:@"PUT"];
	[request setShouldStreamPostDataFromDisk:YES];
	[request setUploadProgressDelegate:self];
	[request setPostBodyFilePath:requestContentPath];
	[request startSynchronous];
	
	BOOL success = (progress == 1.0);
	GHAssertTrue(success,@"Failed to properly increment upload progress %f != 1.0",progress);
	
	success = [[request responseString] isEqualToString:requestBody];
	GHAssertTrue(success,@"Failed upload the correct request body");
	
	
	// Test building a request body by appending data
	progress = 0;
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setShouldStreamPostDataFromDisk:YES];
	[request setRequestMethod:@"PUT"];
	[request setUploadProgressDelegate:self];
	[request appendPostDataFromFile:requestContentPath];
	[request startSynchronous];
	
	success = (progress == 1.0);
	GHAssertTrue(success,@"Failed to properly increment upload progress %f != 1.0",progress);
	
	success = [[request responseString] isEqualToString:requestBody];
	GHAssertTrue(success,@"Failed upload the correct request body");		
}

- (void)testCookies
{
	BOOL success;
	
	// Set setting a cookie
	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/set_cookie"] autorelease];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseCookiePersistence:YES];
	[request startSynchronous];
	NSString *html = [request responseString];
	success = [html isEqualToString:@"I have set a cookie"];
	GHAssertTrue(success,@"Failed to set a cookie");
	
	// Test a cookie is stored in responseCookies
	NSArray *cookies = [request responseCookies];
	GHAssertNotNil(cookies,@"Failed to store cookie data in responseCookies");
	

	// Test the cookie contains the correct data
	NSHTTPCookie *cookie = nil;
	BOOL foundCookie = NO;
	for (cookie in cookies) {
		if ([[cookie name] isEqualToString:@"ASIHTTPRequestTestCookie"]) {
			foundCookie = YES;
			success = [[cookie value] isEqualToString:@"This+is+the+value"];
			GHAssertTrue(success,@"Failed to store the correct value for a cookie");
			success = [[cookie domain] isEqualToString:@"allseeing-i.com"];
			GHAssertTrue(success,@"Failed to store the correct domain for a cookie");
			success = [[cookie path] isEqualToString:@"/ASIHTTPRequest/tests"];
			GHAssertTrue(success,@"Failed to store the correct path for a cookie");
			break;
		}
	}
	GHAssertTrue(foundCookie,@"Failed store a particular cookie - can't continue with the rest of the tests");
	
	if (!foundCookie) {
		return;
	}
	// Test a cookie is presented when manually added to the request
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/read_cookie"] autorelease];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseCookiePersistence:NO];
	[request setRequestCookies:[NSMutableArray arrayWithObject:cookie]];
	[request startSynchronous];
	html = [request responseString];
	success = [html isEqualToString:@"I have 'This is the value' as the value of 'ASIHTTPRequestTestCookie'"];
	GHAssertTrue(success,@"Cookie not presented to the server with cookie persistence OFF");

	// Test a cookie is presented from the persistent store
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/read_cookie"] autorelease];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseCookiePersistence:YES];
	[request startSynchronous];
	html = [request responseString];
	success = [html isEqualToString:@"I have 'This is the value' as the value of 'ASIHTTPRequestTestCookie'"];
	GHAssertTrue(success,@"Cookie not presented to the server with cookie persistence ON");
	
	// Test removing a cookie
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/remove_cookie"] autorelease];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	html = [request responseString];
	success = [html isEqualToString:@"I have removed a cookie"];
	GHAssertTrue(success,@"Failed to remove a cookie");

	// Test making sure cookie was properly removed
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/read_cookie"] autorelease];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	html = [request responseString];
	success = [html isEqualToString:@"No cookie exists"];
	GHAssertTrue(success,@"Cookie presented to the server when it should have been removed");
	
	// Test setting a custom cookie works
	NSDictionary *cookieProperties = [[[NSMutableDictionary alloc] init] autorelease];
	
	// We'll add a line break to our cookie value to test it gets correctly encoded
	[cookieProperties setValue:@"Test%0D%0AValue" forKey:NSHTTPCookieValue];
	[cookieProperties setValue:@"ASIHTTPRequestTestCookie" forKey:NSHTTPCookieName];
	[cookieProperties setValue:@"allseeing-i.com" forKey:NSHTTPCookieDomain];
	[cookieProperties setValue:[NSDate dateWithTimeIntervalSinceNow:60*60*4] forKey:NSHTTPCookieExpires];
	[cookieProperties setValue:@"/ASIHTTPRequest/tests" forKey:NSHTTPCookiePath];
	cookie = [[[NSHTTPCookie alloc] initWithProperties:cookieProperties] autorelease];
	
	GHAssertNotNil(cookie,@"Failed to create a cookie - cookie value was not correctly encoded?");

	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/read_cookie"] autorelease];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseCookiePersistence:NO];
	[request setRequestCookies:[NSMutableArray arrayWithObject:cookie]];
	[request startSynchronous];
	html = [request responseString];
	success = [html isEqualToString:@"I have 'Test\r\nValue' as the value of 'ASIHTTPRequestTestCookie'"];
	GHAssertTrue(success,@"Custom cookie not presented to the server with cookie persistence OFF");
	

	// Test removing all cookies works
	[ASIHTTPRequest clearSession];
	NSArray *sessionCookies = [ASIHTTPRequest sessionCookies];
	success = ([sessionCookies count] == 0);
	GHAssertTrue(success,@"Cookies not removed");

	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/read_cookie"] autorelease];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseCookiePersistence:YES];
	[request startSynchronous];
	html = [request responseString];
	success = [html isEqualToString:@"No cookie exists"];
	GHAssertTrue(success,@"Cookie presented to the server when it should have been removed");
	
	// Test fetching cookies for a relative url - fixes a problem where urls created with URLWithString:relativeToURL: wouldn't always read cookies from the persistent store
	[ASIHTTPRequest clearSession];
	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/set_cookie"] autorelease];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseCookiePersistence:YES];
	[request setUseSessionPersistence:NO];
	[request startSynchronous];
	
	NSURL *originalURL = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/"];
	url = [NSURL URLWithString:@"read_cookie" relativeToURL:originalURL];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseCookiePersistence:YES];
	[request setUseSessionPersistence:NO];
	[request startSynchronous];
	html = [request responseString];
	NSLog(@"%@",html);
	success = [html isEqualToString:@"I have 'This is the value' as the value of 'ASIHTTPRequestTestCookie'"];
	GHAssertTrue(success,@"Custom cookie not presented to the server with cookie persistence OFF");
	
}

// Test fix for a crash if you tried to remove credentials that didn't exist
- (void)testRemoveCredentialsFromKeychain
{
	[ASIHTTPRequest removeCredentialsForHost:@"apple.com" port:0 protocol:@"http" realm:@"Nothing to see here"];
	[ASIHTTPRequest removeCredentialsForProxy:@"apple.com" port:0 realm:@"Nothing to see here"];
	
}

- (void)testPreserveResponseWhenDownloadComplete
{
	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/basic-authentication"];
	ASIHTTPRequest *request;
	BOOL success;

	request = [ASIHTTPRequest requestWithURL:url];
	[request startSynchronous];

	success = ([[request responseString] length]);
	GHAssertTrue(success,@"Request removed the response body when we encountered an error, even though the download was complete");

	NSString *downloadPath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"test.txt"];
	if ([[NSFileManager defaultManager] fileExistsAtPath:downloadPath]) {
		[[NSFileManager defaultManager] removeItemAtPath:downloadPath error:NULL];
	}

	request = [ASIHTTPRequest requestWithURL:url];
	[request setDownloadDestinationPath:downloadPath];
	[request startSynchronous];

	success = ([[[NSFileManager defaultManager] attributesOfItemAtPath:downloadPath error:NULL] fileSize]);
	GHAssertTrue(success,@"Request removed or failed to copy the response to downloadDestinationPath");
}

- (void)testBasicAuthentication
{
	[ASIHTTPRequest removeCredentialsForHost:@"allseeing-i.com" port:0 protocol:@"http" realm:@"SECRET_STUFF"];
	[ASIHTTPRequest clearSession];

	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/basic-authentication"];
	ASIHTTPRequest *request;
	BOOL success;
	NSError *err;
	
	// Test authentication needed when no credentials supplied
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseKeychainPersistence:NO];
	[request startSynchronous];
	success = [[request error] code] == ASIAuthenticationErrorType;
	GHAssertTrue(success,@"Failed to generate permission denied error with no credentials");

	// Test wrong credentials supplied
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseKeychainPersistence:NO];
	[request setUsername:@"wrong"];
	[request setPassword:@"wrong"];
	[request startSynchronous];
	success = [[request error] code] == ASIAuthenticationErrorType;
	GHAssertTrue(success,@"Failed to generate permission denied error with wrong credentials");
	
	// Test correct credentials supplied
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseSessionPersistence:YES];
	[request setUseKeychainPersistence:YES];
	[request setShouldPresentCredentialsBeforeChallenge:NO];
	[request setUsername:@"secret_username"];
	[request setPassword:@"secret_password"];
	[request startSynchronous];
	err = [request error];
	GHAssertNil(err,@"Failed to supply correct username and password");
	
	// Ensure credentials are not reused
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseSessionPersistence:NO];
	[request setUseKeychainPersistence:NO];
	[request startSynchronous];
	success = [[request error] code] == ASIAuthenticationErrorType;
	GHAssertTrue(success,@"Reused credentials when we shouldn't have");

	// Ensure credentials stored in the session are reused
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseSessionPersistence:YES];
	[request setUseKeychainPersistence:NO];
	[request startSynchronous];
	err = [request error];
	GHAssertNil(err,@"Failed to reuse credentials");
	
	// Ensure new credentials are used in place of those in the session
	request = [[[ASIHTTPRequest alloc] initWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/basic-authentication-new-credentials"]] autorelease];
	[request setUsername:@"secret_username_2"];
	[request setPassword:@"secret_password_2"];
	[request setUseSessionPersistence:YES];
	[request setUseKeychainPersistence:NO];
	[request startSynchronous];
	err = [request error];
	GHAssertNil(err,@"Failed to reuse credentials");
	
	[ASIHTTPRequest clearSession];
	
	// Ensure credentials stored in the session were wiped
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseKeychainPersistence:NO];
	[request startSynchronous];
	success = [[request error] code] == ASIAuthenticationErrorType;
	GHAssertTrue(success,@"Failed to clear credentials");
	
	// Ensure credentials stored in the keychain are reused
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseKeychainPersistence:YES];
	[request startSynchronous];
	err = [request error];
	GHAssertNil(err,@"Failed to use stored credentials");
	
	[ASIHTTPRequest removeCredentialsForHost:@"allseeing-i.com" port:0 protocol:@"http" realm:@"SECRET_STUFF"];
	
	// Ensure credentials stored in the keychain were wiped
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseKeychainPersistence:YES];
	[request setUseSessionPersistence:NO];
	[request startSynchronous];
	success = [[request error] code] == ASIAuthenticationErrorType;
	GHAssertTrue(success,@"Failed to clear credentials");
	
	// Tests shouldPresentCredentialsBeforeChallenge with credentials stored in the session
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseSessionPersistence:YES];
	[request startSynchronous];
	success = [request authenticationRetryCount] == 0;
	GHAssertTrue(success,@"Didn't supply credentials before being asked for them when talking to the same server with shouldPresentCredentialsBeforeChallenge == YES");
	
	// Ensure credentials stored in the session were not presented to the server before it asked for them
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseSessionPersistence:YES];
	[request setShouldPresentCredentialsBeforeChallenge:NO];
	[request startSynchronous];
	success = [request authenticationRetryCount] == 1;
	GHAssertTrue(success,@"Supplied session credentials before being asked for them");	
	
	[ASIHTTPRequest clearSession];
	
	// Test credentials set on the request are not sent before the server asks for them unless they are cached credentials from a previous request to this server
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseSessionPersistence:NO];
	[request setUsername:@"secret_username"];
	[request setPassword:@"secret_password"];
	[request setShouldPresentCredentialsBeforeChallenge:YES];
	[request startSynchronous];
	BOOL fail = [request authenticationRetryCount] == 0;
	GHAssertFalse(fail,@"Sent Basic credentials even though request did not have kCFHTTPAuthenticationSchemeBasic set as authenticationScheme.");

	// Test basic credentials set on the request are sent before the server asks for them if we've explictly set the authentication scheme to basic
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseSessionPersistence:NO];
	[request setUsername:@"secret_username"];
	[request setPassword:@"secret_password"];
	[request setShouldPresentCredentialsBeforeChallenge:YES];
	[request setAuthenticationScheme:(NSString *)kCFHTTPAuthenticationSchemeBasic];
	[request startSynchronous];
	success = [request authenticationRetryCount] == 0;
	GHAssertTrue(success,@"Didn't supply credentials before being asked for them, even though they were set on the request and shouldPresentCredentialsBeforeChallenge == YES");
	
	// Test credentials set on the request aren't sent before the server asks for them
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseSessionPersistence:NO];
	[request setUsername:@"secret_username"];
	[request setPassword:@"secret_password"];
	[request setShouldPresentCredentialsBeforeChallenge:NO];
	[request startSynchronous];
	success = [request authenticationRetryCount] == 1;
	GHAssertTrue(success,@"Supplied request credentials before being asked for them");	
	
	
	// Test credentials presented before a challenge are stored in the session store
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUsername:@"secret_username"];
	[request setPassword:@"secret_password"];
	[request setShouldPresentCredentialsBeforeChallenge:YES];
	[request startSynchronous];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	err = [request error];
	GHAssertNil(err,@"Failed to use stored credentials");	
	
	
	// Ok, now let's test on a different server to sanity check that the credentials from out previous requests are not being used
	url = [NSURL URLWithString:@"https://selfsigned.allseeing-i.com/ASIHTTPRequest/tests/basic-authentication"];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseSessionPersistence:YES];
	[request setUseKeychainPersistence:NO];
	[request setValidatesSecureCertificate:NO];
	[request startSynchronous];
	success = [[request error] code] == ASIAuthenticationErrorType;
	GHAssertTrue(success,@"Reused credentials when we shouldn't have");	
	
}



- (void)testDigestAuthentication
{
	[ASIHTTPRequest removeCredentialsForHost:@"allseeing-i.com" port:0 protocol:@"http" realm:@"Keep out"];
	[ASIHTTPRequest clearSession];
	
	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/digest-authentication"] autorelease];
	ASIHTTPRequest *request;
	BOOL success;
	NSError *err;
	
	// Test authentication needed when no credentials supplied
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseKeychainPersistence:NO];
	[request startSynchronous];
	success = [[request error] code] == ASIAuthenticationErrorType;
	GHAssertTrue(success,@"Failed to generate permission denied error with no credentials");
	
	// Test wrong credentials supplied
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseKeychainPersistence:NO];
	[request setUsername:@"wrong"];
	[request setPassword:@"wrong"];
	[request startSynchronous];
	success = [[request error] code] == ASIAuthenticationErrorType;
	GHAssertTrue(success,@"Failed to generate permission denied error with wrong credentials");
	
	// Test correct credentials supplied
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseSessionPersistence:YES];
	[request setUseKeychainPersistence:YES];
	[request setUsername:@"secret_username"];
	[request setPassword:@"secret_password"];
	[request startSynchronous];
	err = [request error];
	GHAssertNil(err,@"Failed to supply correct username and password");
	
	// Ensure credentials are not reused
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseSessionPersistence:NO];
	[request setUseKeychainPersistence:NO];
	[request startSynchronous];
	success = [[request error] code] == ASIAuthenticationErrorType;
	GHAssertTrue(success,@"Reused credentials when we shouldn't have");
	
	// Ensure credentials stored in the session are reused
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseSessionPersistence:YES];
	[request setUseKeychainPersistence:NO];
	[request startSynchronous];
	err = [request error];
	GHAssertNil(err,@"Failed to reuse credentials");
	
	[ASIHTTPRequest clearSession];
	
	// Ensure credentials stored in the session were wiped
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseKeychainPersistence:NO];
	[request startSynchronous];
	success = [[request error] code] == ASIAuthenticationErrorType;
	GHAssertTrue(success,@"Failed to clear credentials");
	
	// Ensure credentials stored in the keychain are reused
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseKeychainPersistence:YES];
	[request startSynchronous];
	err = [request error];
	GHAssertNil(err,@"Failed to reuse credentials");
	
	[ASIHTTPRequest removeCredentialsForHost:@"allseeing-i.com" port:0 protocol:@"http" realm:@"Keep out"];
	
	// Ensure credentials stored in the keychain were wiped
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseKeychainPersistence:YES];
	[request setUseSessionPersistence:NO];
	[request startSynchronous];
	success = [[request error] code] == ASIAuthenticationErrorType;
	GHAssertTrue(success,@"Failed to clear credentials");	
	
	
	// Test credentials set on the request are sent before the server asks for them
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUseSessionPersistence:YES];
	[request setShouldPresentCredentialsBeforeChallenge:YES];
	[request startSynchronous];
	success = [request authenticationRetryCount] == 0;
	GHAssertTrue(success,@"Didn't supply credentials before being asked for them, even though they were set in the session and shouldPresentCredentialsBeforeChallenge == YES");	
	
}

- (void)testNTLMHandshake
{
	// This test connects to a script that masquerades as an NTLM server
	// It tests that the handshake seems sane, but doesn't actually authenticate
	
	[ASIHTTPRequest clearSession];
	
	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/pretend-ntlm-handshake"];
	
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:url];
	[request setUseKeychainPersistence:NO];
	[request setUseSessionPersistence:NO];
	[request startSynchronous];
	BOOL success = [[request error] code] == ASIAuthenticationErrorType;
	GHAssertTrue(success,@"Failed to generate permission denied error with no credentials");


	request = [ASIHTTPRequest requestWithURL:url];
	[request setUseSessionPersistence:YES];
	[request setUseKeychainPersistence:NO];
	[request setUsername:@"king"];
	[request setPassword:@"fink"];
	[request setDomain:@"Castle.Kingdom"];
	[request startSynchronous];

	GHAssertNil([request error],@"Got an error when credentials were supplied");
	
	// NSProcessInfo returns a lower case string for host name, while CFNetwork will send a mixed case string for host name, so we'll compare by lowercasing everything
	NSString *hostName = [[NSProcessInfo processInfo] hostName];
	NSString *expectedResponse = [[NSString stringWithFormat:@"You are %@ from %@/%@",@"king",@"Castle.Kingdom",hostName] lowercaseString];
	success = [[[request responseString] lowercaseString] isEqualToString:expectedResponse];
	GHAssertTrue(success,@"Failed to send credentials correctly? (Expected: '%@', got '%@')",expectedResponse,[[request responseString] lowercaseString]);
}

- (void)testCompressedResponse
{
	// allseeing-i.com does not gzip png images
	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/i/logo.png"] autorelease];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	NSString *encoding = [[request responseHeaders] objectForKey:@"Content-Encoding"];
	BOOL success = (!encoding || [encoding rangeOfString:@"gzip"].location != NSNotFound);
	GHAssertTrue(success,@"Got incorrect request headers from server");

	success = ([request rawResponseData] == [request responseData]);
	GHAssertTrue(success,@"Attempted to uncompress data that was not compressed");	

	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/first"] autorelease];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	success = ([request rawResponseData] != [request responseData]);
	GHAssertTrue(success,@"Uncompressed data is the same as compressed data");	

	success = [[request responseString] isEqualToString:@"This is the expected content for the first string"];
	GHAssertTrue(success,@"Failed to decompress data correctly?");

	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/first"] autorelease];
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setShouldWaitToInflateCompressedResponses:NO];
	[request startSynchronous];
	success = ([request rawResponseData] == [request responseData]);
	GHAssertTrue(success,@"Failed to populate rawResponseData with the inflated data");
}


- (void)testPartialFetch
{
	// We run tests that measure progress on the main thread because otherwise we can't depend on the progress delegate being notified before we need to test it's working
	[self performSelectorOnMainThread:@selector(performPartialFetchTest) withObject:nil waitUntilDone:YES];	

}
					
- (void)performPartialFetchTest
{
	NSString *downloadPath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"testfile.txt"];
	NSString *tempPath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"tempfile.txt"];
	NSString *partialContent = @"This file should be exactly 163 bytes long when encoded as UTF8, Unix line breaks with no BOM.\n";
	[partialContent writeToFile:tempPath atomically:NO encoding:NSASCIIStringEncoding error:nil];
	
	progress = 0;
	NSURL *url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/test_partial_download.txt"] autorelease];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setDownloadDestinationPath:downloadPath];
	[request setTemporaryFileDownloadPath:tempPath];
	[request setAllowResumeForFileDownloads:YES];
	[request setAllowCompressedResponse:NO];
	[request setDownloadProgressDelegate:self];
	[request startSynchronous];
	
	BOOL success = ([request contentLength] == 163-95);
	GHAssertTrue(success,@"Failed to download a segment of the data");
	
	NSString *content = [NSString stringWithContentsOfFile:downloadPath encoding:NSUTF8StringEncoding error:NULL];
	
	NSString *newPartialContent = [content substringFromIndex:95];
	success = ([newPartialContent isEqualToString:@"This is the content we ought to be getting if we start from byte 95."]);
	GHAssertTrue(success,@"Failed to append the correct data to the end of the file?");
	
	success = (progress == 1.0);
	GHAssertTrue(success,@"Failed to correctly display increment progress for a partial download");
}

// The '000' is to ensure this test runs first, as another test may connect to https://selfsigned.allseeing-i.com and accept the certificate
- (void)test000SSL
{
	NSURL *url = [NSURL URLWithString:@"https://selfsigned.allseeing-i.com"];
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:url];
	[request startSynchronous];
	
	GHAssertNotNil([request error],@"Failed to generate an error for a self-signed certificate (Will fail on the second run in the same session!)");		
	
	// Just for testing the request generated a custom error description - don't do this! You should look at the domain / code of the underlyingError in your own programs.
	BOOL success = ([[[request error] localizedDescription] rangeOfString:@"SSL problem"].location != NSNotFound);
	GHAssertTrue(success,@"Generated the wrong error for a self signed cert");
	
	// Turn off certificate validation, and try again
	request = [ASIHTTPRequest requestWithURL:url];
	[request setValidatesSecureCertificate:NO];
	[request startSynchronous];
	
	GHAssertNil([request error],@"Failed to accept a self-signed certificate");	
}

- (void)testRedirectPreservesSession
{
	// Remove any old session cookies
	[ASIHTTPRequest clearSession];
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/session_redirect"]];
	[request startSynchronous];
	BOOL success = [[request responseString] isEqualToString:@"Take me to your leader"];
	GHAssertTrue(success,@"Failed to redirect preserving session cookies");	
}

- (void)testTooMuchRedirection
{
	// This url will simply send a 302 redirect back to itself
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/one_infinite_loop"]];
	[request startSynchronous];
	GHAssertNotNil([request error],@"Failed to generate an error when redirection occurs too many times");
	BOOL success = ([[request error] code] == ASITooMuchRedirectionErrorType);
	GHAssertTrue(success,@"Generated the wrong error for a redirection loop");		
}

- (void)testRedirectToNewDomain
{
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect_to_new_domain"]];
	[request startSynchronous];
	BOOL success = [[[[request url] absoluteString] stringByTrimmingCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@"/"]] isEqualToString:@"http://www.apple.com"];
	GHAssertTrue(success,@"Failed to redirect to a different domain");		
}

// Ensure request method changes to get
- (void)test303Redirect
{
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect_303"]];
	[request setRequestMethod:@"PUT"];
	[request appendPostData:[@"Fuzzy" dataUsingEncoding:NSUTF8StringEncoding]];
	[request startSynchronous];
	BOOL success = [[[request url] absoluteString] isEqualToString:@"http://allseeing-i.com/ASIHTTPRequest/tests/request-method"];
	GHAssertTrue(success,@"Failed to redirect to correct location");
	success = [[request responseString] isEqualToString:@"GET"];
	GHAssertTrue(success,@"Failed to use GET on new URL");
}

- (void)testCompressedBody
{
	
	NSString *content = @"This is the test content. This is the test content. This is the test content. This is the test content.";
	
	// Test in memory compression / decompression
	NSData *data = [content dataUsingEncoding:NSUTF8StringEncoding];

	
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/compressed_post_body"]];
	[request setRequestMethod:@"PUT"];
	[request setShouldCompressRequestBody:YES];
	[request setShouldStreamPostDataFromDisk:YES];
	[request setUploadProgressDelegate:self];
	[request appendPostData:data];
	[request startSynchronous];

	BOOL success = ([[request responseString] isEqualToString:content]);
	GHAssertTrue(success,@"Failed to compress the body, or server failed to decompress it");

}


// Ensure class convenience constructor returns an instance of our subclass
- (void)testSubclass
{
	ASIHTTPRequestSubclass *instance = [ASIHTTPRequestSubclass requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	BOOL success = [instance isKindOfClass:[ASIHTTPRequestSubclass class]];
	GHAssertTrue(success,@"Convenience constructor failed to return an instance of the correct class");	
}


- (void)testThrottlingDownloadBandwidth
{
	[ASIHTTPRequest setMaxBandwidthPerSecond:0];
	
	// This content is around 128KB in size, and it won't be gzipped, so it should take more than 8 seconds to download at 14.5KB / second
	// We'll test first without throttling
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_%28abridged%29.txt"]];
	NSDate *date = [NSDate date];
	[request startSynchronous];	
	
	NSTimeInterval interval =[date timeIntervalSinceNow];
	BOOL success = (interval > -7);
	GHAssertTrue(success,@"Downloaded the file too slowly - either this is a bug, or your internet connection is too slow to run this test (must be able to download 128KB in less than 7 seconds, without throttling)");
	
	// Now we'll test with throttling
	[ASIHTTPRequest setMaxBandwidthPerSecond:ASIWWANBandwidthThrottleAmount];
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_%28abridged%29.txt"]];
	date = [NSDate date];
	[request startSynchronous];	
	
	[ASIHTTPRequest setMaxBandwidthPerSecond:0];
	
	interval =[date timeIntervalSinceNow];
	success = (interval < -7);
	GHAssertTrue(success,@"Failed to throttle download");		
	GHAssertNil([request error],@"Request generated an error - timeout?");	
	
}

- (void)testThrottlingUploadBandwidth
{
	[ASIHTTPRequest setMaxBandwidthPerSecond:0];
	
	// Create a 64KB request body
	NSData *data = [[[NSMutableData alloc] initWithLength:64*1024] autorelease];
	
	// We'll test first without throttling
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ignore"]];
	[request appendPostData:data];
	NSDate *date = [NSDate date];
	[request startSynchronous];	
	
	NSTimeInterval interval =[date timeIntervalSinceNow];
	BOOL success = (interval > -3);
	GHAssertTrue(success,@"Uploaded the data too slowly - either this is a bug, or your internet connection is too slow to run this test (must be able to upload 64KB in less than 3 seconds, without throttling)");
	
	// Now we'll test with throttling
	[ASIHTTPRequest setMaxBandwidthPerSecond:ASIWWANBandwidthThrottleAmount];
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ignore"]];
	[request appendPostData:data];
	date = [NSDate date];
	[request startSynchronous];	
	
	[ASIHTTPRequest setMaxBandwidthPerSecond:0];
	
	interval =[date timeIntervalSinceNow];
	success = (interval < -3);
	GHAssertTrue(success,@"Failed to throttle upload");		
	GHAssertNil([request error],@"Request generated an error - timeout?");	
}


- (void)testFetchToInvalidPath
{
	// Test gzipped content
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL	URLWithString:@"http://allseeing-i.com"]];
	[request setDownloadDestinationPath:@"/an/invalid/location.html"];
	[request startSynchronous];
	GHAssertNotNil([request error],@"Failed to generate an authentication when attempting to write to an invalid location");	
	
	//Test non-gzipped content
	request = [ASIHTTPRequest requestWithURL:[NSURL	URLWithString:@"http://allseeing-i.com/i/logo.png"]];
	[request setDownloadDestinationPath:@"/an/invalid/location.png"];
	[request startSynchronous];
	GHAssertNotNil([request error],@"Failed to generate an authentication when attempting to write to an invalid location");		
}

- (void)testResponseStatusMessage
{
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL	URLWithString:@"http://allseeing-i.com/the-meaning-of-life"]];
	[request startSynchronous];	
	BOOL success = [[request responseStatusMessage] isEqualToString:@"HTTP/1.0 404 Not Found"];
	GHAssertTrue(success,@"Got wrong response status message");
}

- (void)testAsynchronous
{
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/first"]];
	[request setTag:1];
	[request setDidFailSelector:@selector(asyncFail:)];
	[request setDidFinishSelector:@selector(asyncSuccess:)];
	[request setDelegate:self];
	[request startAsynchronous];
	
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/second"]];
	[request setTag:2];
	[request setDidFailSelector:@selector(asyncFail:)];
	[request setDidFinishSelector:@selector(asyncSuccess:)];
	[request setDelegate:self];
	[request startAsynchronous];
	
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/third"]];
	[request setTag:3];
	[request setDidFailSelector:@selector(asyncFail:)];
	[request setDidFinishSelector:@selector(asyncSuccess:)];
	[request setDelegate:self];
	[request startAsynchronous];	
	
	request = [ASIHTTPRequest requestWithURL:nil];
	[request setTag:4];
	[request setDidFailSelector:@selector(asyncFail:)];
	[request setDidFinishSelector:@selector(asyncSuccess:)];
	[request setDelegate:self];
	[request startAsynchronous];	
}


- (void)asyncFail:(ASIHTTPRequest *)request
{
	BOOL success = ([request tag] == 4);
	GHAssertTrue(success,@"Wrong request failed");
}

- (void)asyncSuccess:(ASIHTTPRequest *)request
{
	BOOL success = ([request tag] != 4);
	GHAssertTrue(success,@"Request succeeded when it should have failed");
	
	switch ([request tag]) {
		case 1:
			success = [[request responseString] isEqualToString:@"This is the expected content for the first string"];
			break;
		case 2:
			success = [[request responseString] isEqualToString:@"This is the expected content for the second string"];
			break;
		case 3:
			success = [[request responseString] isEqualToString:@"This is the expected content for the third string"];
			break;
	}
	GHAssertTrue(success,@"Got wrong request content - very bad!");
	
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

#if TARGET_OS_IPHONE
- (void)testReachability
{
#if REACHABILITY_20_API
	NSLog(@"Using Reachability 2.0 API");
#else
	NSLog(@"Using Reachability 1.5 API");
#endif
	if ([ASIHTTPRequest isNetworkReachableViaWWAN]) {
		NSLog(@"Connected via WWAN");
	} else {
		NSLog(@"Not connected via WWAN");
	}
}
#endif

- (void)testAutomaticRetry
{
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request setTimeOutSeconds:0.001];
	[request setNumberOfTimesToRetryOnTimeout:5];
	[request startSynchronous];
	GHAssertNotNil([request error],@"Request failed to timeout, cannot proceed with test");
	BOOL success = ([request retryCount] == 5);
	GHAssertTrue(success,@"Request failed to retry on timeout");
}

- (void)testCopy
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	ASIHTTPRequest *request2 = [request copy];
	
	[pool release];
	
	GHAssertNotNil(request2,@"Failed to create a copy");
	BOOL success = ([request2 retainCount] == 1);
	GHAssertTrue(success,@"Failed to create a retained copy");
	
	[request2 release];
}

- (void)testCloseConnection
{
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/close-connection"]];
	[request startSynchronous];
	
	BOOL success = ![request connectionCanBeReused];
	GHAssertTrue(success,@"Should not be able to re-use a request sent with Connection:close");
	
	// Ensure we close the connection when authentication is needed
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/close-connection-auth-needed"]];
	[request startSynchronous];
	
	success = ![request connectionCanBeReused];
	GHAssertTrue(success,@"Should not be able to re-use a request sent with Connection:close");
	
}

- (void)testPersistentConnections
{
	// allseeing-i.com is configured to keep persistent connections alive for 2 seconds

	// Ensure we parse a keep-alive header
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];

	BOOL success = ([request persistentConnectionTimeoutSeconds] == 60);
	GHAssertTrue(success,@"Request failed to default to 60 seconds for connection timeout");

	[request startSynchronous];

	NSNumber *connectionId = [request connectionID];

	success = ([request persistentConnectionTimeoutSeconds] == 2);
	GHAssertTrue(success,@"Request failed to use time out set by server");

	// Wait 3 seconds - connection should have timed out
	sleep(3);

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request startSynchronous];

	success = ([[request connectionID] intValue] != [connectionId intValue]);
	GHAssertTrue(success,@"Reused a connection that should have timed out");

	// Ensure persistent connections are turned off by default with POST/PUT and/or a request body
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request appendPostData:[@"Foo" dataUsingEncoding:NSUTF8StringEncoding]];
	[request startSynchronous];

	success = ![request shouldAttemptPersistentConnection];
	GHAssertTrue(success,@"Used a persistent connection with a body");

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request setRequestMethod:@"PUT"];
	[request startSynchronous];

	success = ![request shouldAttemptPersistentConnection];
	GHAssertTrue(success,@"Used a persistent connection with PUT");

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request setRequestMethod:@"POST"];
	[request startSynchronous];

	success = ![request shouldAttemptPersistentConnection];
	GHAssertTrue(success,@"Used a persistent connection with POST");

	// Ensure we can force a persistent connection
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request setRequestMethod:@"POST"];
	[request setShouldAttemptPersistentConnection:YES];
	[request startSynchronous];

	success = [request shouldAttemptPersistentConnection];
	GHAssertTrue(success,@"Failed to use a persistent connection");
}

- (void)testRemoveUploadProgress
{
	[self performSelectorOnMainThread:@selector(runRemoveUploadProgressTest) withObject:nil waitUntilDone:YES];
}

- (void)runRemoveUploadProgressTest
{
	progress = 0;
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	NSData *data = [[[NSMutableData alloc] initWithLength:64*1024] autorelease];
	[request appendPostData:data];
	[request setRequestMethod:@"POST"];
	[request setUploadProgressDelegate:self];
	[request startSynchronous];
	
	BOOL success = (progress == 1.0);
	GHAssertTrue(success,@"Failed to set upload progress, cannot proceed with test");
	
	[request removeUploadProgressSoFar];
	success = (progress == 0);
	GHAssertTrue(success,@"Failed to set upload progress, cannot proceed with test");
}

- (void)testMimeType
{
	NSString *text = @"This is my content";
	NSString *filePath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"testfile.txt"];
	[[text dataUsingEncoding:NSUTF8StringEncoding] writeToFile:filePath atomically:NO];
	
	BOOL success = ([[ASIHTTPRequest mimeTypeForFileAtPath:filePath] isEqualToString:@"text/plain"]);
	GHAssertTrue(success,@"Failed to detect the mime type for a file");
	
	filePath = @"/nowhere";
	success = (![ASIHTTPRequest mimeTypeForFileAtPath:filePath]);
	GHAssertTrue(success,@"Returned a mime type for a non-existent file");
	
	filePath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"testfile"];
	[[text dataUsingEncoding:NSUTF8StringEncoding] writeToFile:filePath atomically:NO];
	success = ([[ASIHTTPRequest mimeTypeForFileAtPath:filePath] isEqualToString:@"application/octet-stream"]);
	GHAssertTrue(success,@"Failed to return the default mime type when a file has no extension");
}

- (void)testDelegateResponseDataHandling
{
	[self setResponseData:[NSMutableData dataWithLength:0]];
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_%28young_readers_edition%29.txt"]];
	[request setDelegate:self];
	[request setDidReceiveDataSelector:@selector(theTestRequest:didReceiveData:)];
	[request setDidFinishSelector:@selector(theTestRequestFinished:)];
	[request startAsynchronous];
}

- (void)theTestRequestFinished:(ASIHTTPRequest *)request
{
	ASIHTTPRequest *request2 = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_%28young_readers_edition%29.txt"]];
	[request2 startSynchronous];
	NSString *firstResponse = [[[NSString alloc] initWithBytes:[[self responseData] bytes] length:[[self responseData] length] encoding:[request responseEncoding]] autorelease];
	BOOL success = [[request2 responseString] isEqualToString:firstResponse];
	GHAssertTrue(success,@"Failed to correctly download and store the response using a delegate");
}

- (void)theTestRequest:(ASIHTTPRequest *)request didReceiveData:(NSData *)data
{
	[[self responseData] appendData:data];
}


- (void)testNilPortCredentialsMatching
{
	// Test for http://github.com/pokeb/asi-http-request/issues#issue/39
	[ASIHTTPRequest clearSession];
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com:80/ASIHTTPRequest/tests/basic-authentication"]];
	[request setUsername:@"secret_username"];
	[request setPassword:@"secret_password"];
	[request startSynchronous];

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/basic-authentication"]];
	[request startSynchronous];

	// Now let's test the other way around
	[ASIHTTPRequest clearSession];

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com:/ASIHTTPRequest/tests/basic-authentication"]];
	[request setUsername:@"secret_username"];
	[request setPassword:@"secret_password"];
	[request startSynchronous];

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com:80/ASIHTTPRequest/tests/basic-authentication"]];
	[request startSynchronous];
}


- (void)testRFC1123DateParsing
{
	unsigned dateUnits = NSYearCalendarUnit | NSMonthCalendarUnit |  NSDayCalendarUnit | NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit | NSWeekdayCalendarUnit;
	NSCalendar *calendar = [[[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar] autorelease];
	[calendar setTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0]];
	NSString *dateString = @"Thu, 19 Nov 1981 08:52:01 GMT";
	NSDate *date = [ASIHTTPRequest dateFromRFC1123String:dateString];
	NSDateComponents *components = [calendar components:dateUnits fromDate:date];
	BOOL success = ([components year] == 1981 && [components month] == 11 && [components day] == 19 && [components weekday] == 5 && [components hour] == 8 && [components minute] == 52 && [components second] == 1);
	GHAssertTrue(success,@"Failed to parse an RFC1123 date correctly");

	dateString = @"4 May 2010 00:59 CET";
	date = [ASIHTTPRequest dateFromRFC1123String:dateString];
	components = [calendar components:dateUnits fromDate:date];
	success = ([components year] == 2010 && [components month] == 5 && [components day] == 3 && [components hour] == 23 && [components minute] == 59);
	GHAssertTrue(success,@"Failed to parse an RFC1123 date correctly");

}

- (void)testAccurateProgressFallback
{
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request setAllowCompressedResponse:NO]; // A bit hacky - my server will send a chunked response (without content length) when we don't specify that we accept gzip
	[request startSynchronous];

	BOOL success = ([request showAccurateProgress] == NO);
	GHAssertTrue(success,@"Request failed to fall back to simple progress");

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect_resume"]];
	[request startSynchronous];

	success = ([request showAccurateProgress] == YES);
	GHAssertTrue(success,@"Request fell back to simple progress when redirecting");
}

// Because of they way I implemented the server part of this test, I'm afraid you won't be able to run it yourself

- (void)testResumeWithAutomaticTimeoutRetry
{
	printf("\nSkipping testResumeWithAutomaticTimeoutRetry - ");
	return;
	// Get the first part of the response
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/resume-with-timeout"]];
	[request setAllowCompressedResponse:NO];
	[request startSynchronous];

	NSString *partialPath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"partial.txt"];
	[[request responseString] writeToFile:partialPath atomically:NO encoding:NSUTF8StringEncoding error:NULL];

	NSString *completePath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"complete.txt"];

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/resume-with-timeout-finish"]];
	[request setAllowCompressedResponse:NO];
	[request setAllowResumeForFileDownloads:YES];
	[request setTemporaryFileDownloadPath:partialPath];
	[request setDownloadDestinationPath:completePath];
	[request setNumberOfTimesToRetryOnTimeout:1];
	[request startSynchronous];

	NSString *expectedOutput = @"";

	char i;
	for (i=0; i<3; i++) {
		char *s;
		s = (char *)malloc(1024*128);
		memset(s, i+49, 1024*128);
		expectedOutput = [expectedOutput stringByAppendingString:[[[NSString alloc] initWithBytes:s length:1024*128 encoding:NSUTF8StringEncoding] autorelease]];
		expectedOutput = [expectedOutput stringByAppendingString:@"\r\n"];
		free(s);
	}

	BOOL success = [expectedOutput isEqualToString:[NSString stringWithContentsOfFile:completePath encoding:NSUTF8StringEncoding error:NULL]];
	GHAssertTrue(success, @"Failed to send the correct Range headers to the server when resuming after a timeout");
}

- (void)testChangeURLOnAuthenticationRetry
{
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/basic-authentication"]];
	[request setDelegate:self];
	[request setValidatesSecureCertificate:NO];
	[request startAsynchronous];
}

- (void)changeURLFailed:(ASIHTTPRequest *)request
{
	GHFail(@"Request failed when changing url");
}

- (void)authenticationNeededForRequest:(ASIHTTPRequest *)request
{
	[request setUsername:@"foo"];
	[request setPassword:@"foo"];
	[request setDidFailSelector:@selector(changeURLFailed:)];
	[request setURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request retryUsingSuppliedCredentials];
}

- (void)testDelegateRedirectHandling
{
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/redirect_to_ssl"]];
	[request setDelegate:self];
	[request setWillRedirectSelector:@selector(request:isGoingToRedirectToURL:)];
	[request setDidFailSelector:@selector(redirectURLTestFailed:)];
	[request setDidFinishSelector:@selector(redirectURLTestSucceeded:)];
	[request startAsynchronous];
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

@synthesize responseData;
@end
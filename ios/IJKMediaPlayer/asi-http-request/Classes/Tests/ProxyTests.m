//
//  ProxyTests.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 02/08/2009.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//

#import "ProxyTests.h"
#import "ASIHTTPRequest.h"
#import "ASINetworkQueue.h"

// Fill in these to run the proxy tests
static NSString *proxyHost = @"";
static int proxyPort = 0;
static NSString *proxyUsername = @"";
static NSString *proxyPassword = @"";

// Stop clang complaining about undeclared selectors
@interface ProxyTests ()
- (void)requestDone:(ASIHTTPRequest *)request;
- (void)requestFailed:(ASIHTTPRequest *)request;
@end


@implementation ProxyTests

- (void)testProxyForHTTPS
{
	// Also test we are case-insensitive comparing our scheme
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"HTTPS://selfsigned.allseeing-i.com/"]];
	[request setValidatesSecureCertificate:NO];
	[request startSynchronous];
	BOOL success = ([[request responseString] rangeOfString:@"All-Seeing Interactive"].location != NSNotFound);
	GHAssertTrue(success,@"Failed to connect to an HTTPS URL using a proxy");
}

- (void)testAutoConfigureWithPAC
{

	NSString *pacurl = @"file:///non-existent.pac";
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request setPACurl:[NSURL URLWithString:pacurl]];
	[request startSynchronous];
	GHAssertNil([request proxyHost],@"Shouldn't use a proxy here");
	GHAssertNil([request error],@"Request failed when unable to fetch PAC (should assume no proxy instead)");
	
	// To run this test, specify the location of the pac script that is available at http://developer.apple.com/samplecode/CFProxySupportTool/listing1.html
	pacurl = @"file:///Users/ben/Desktop/test.pac";
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request setPACurl:[NSURL URLWithString:pacurl]];
	[request startSynchronous];

	BOOL success = [[request proxyHost] isEqualToString:@"proxy1.apple.com"];
	GHAssertTrue(success,@"Failed to use the correct proxy");
	
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://www.apple.com"]];
	[request setPACurl:[NSURL URLWithString:pacurl]];
	[request startSynchronous];
	GHAssertNil([request proxyHost],@"Used a proxy when the script told us to go direct");
}

- (void)testAutoConfigureWithSystemPAC
{
	// To run this test, specify the pac script above in your network settings
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request startSynchronous];

	BOOL success = [[request proxyHost] isEqualToString:@"proxy1.apple.com"];
	GHAssertTrue(success,@"Failed to use the correct proxy");
	
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://www.apple.com"]];
	[request startSynchronous];
	GHAssertNil([request proxyHost],@"Used a proxy when the script told us to go direct");
}

- (void)testProxy
{
	BOOL success = (![proxyHost isEqualToString:@""] && proxyPort > 0);
	GHAssertTrue(success,@"You need to supply the details of your proxy to run the proxy autodetect test");	
	
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request setProxyHost:proxyHost];
	[request setProxyPort:proxyPort];
	[request startSynchronous];
	
	// Check data is as expected
	NSRange notFound = NSMakeRange(NSNotFound, 0);
	success = !NSEqualRanges([[request responseString] rangeOfString:@"All-Seeing Interactive"],notFound);
	GHAssertTrue(success,@"Failed to download the correct data, navigating the proxy");
}

- (void)testProxyAutodetect
{	
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request startSynchronous];
	
	BOOL success = ([request proxyHost] && [request proxyPort]);
	GHAssertTrue(success,@"Failed to detect the proxy");		
}


- (void)testProxyWithSuppliedAuthenticationCredentials
{
	BOOL success = (![proxyHost isEqualToString:@""] && proxyPort > 0 && ![proxyUsername isEqualToString:@""] && ![proxyPassword isEqualToString:@""]);
	GHAssertTrue(success,@"You need to supply the details of your authenticating proxy to run the proxy authentication test");	
	
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request setProxyHost:proxyHost];
	[request setProxyPort:proxyPort];
	[request setProxyUsername:proxyUsername];
	[request setProxyPassword:proxyPassword];
	[request startSynchronous];
	
	// Check data is as expected
	NSRange notFound = NSMakeRange(NSNotFound, 0);
	success = !NSEqualRanges([[request responseString] rangeOfString:@"All-Seeing Interactive"],notFound);
	GHAssertTrue(success,@"Failed to download the correct data, navigating the proxy");
}

- (void)testProxyWithDelegateSupplyingCredentials
{
	[self setComplete:NO];
	BOOL success = (![proxyHost isEqualToString:@""] && proxyPort > 0 && ![proxyUsername isEqualToString:@""] && ![proxyPassword isEqualToString:@""]);
	GHAssertTrue(success,@"You need to supply the details of your authenticating proxy to run the proxy authentication test");	
	
	[[self queue] cancelAllOperations];
	[self setQueue:[ASINetworkQueue queue]];
	[[self queue] setDelegate:self];
	[[self queue] setRequestDidFinishSelector:@selector(requestFinished:)];
	[[self queue] setRequestDidFailSelector:@selector(requestFailed:)];
	
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[[self queue] addOperation:request];
	
	[queue go];
	
	while (![self complete]) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
}

- (void)requestFinished:(ASIHTTPRequest *)request
{
	[self setComplete:YES];
	// Check data is as expected
	NSRange notFound = NSMakeRange(NSNotFound, 0);
	BOOL success = !NSEqualRanges([[request responseString] rangeOfString:@"All-Seeing Interactive"],notFound);
	GHAssertTrue(success,@"Failed to download the correct data, navigating the proxy");	
}

- (void)requestFailed:(ASIHTTPRequest *)request
{
	[self setComplete:YES];
	GHAssertTrue(0,@"Request failed when it shouldn't have done so");	
}

- (void)proxyAuthenticationNeededForRequest:(ASIHTTPRequest *)request
{
	[request setProxyUsername:proxyUsername];
	[request setProxyPassword:proxyPassword];
	[request retryUsingSuppliedCredentials];
}


- (void)testDoubleAuthentication
{
	[self setComplete:NO];
	BOOL success = (![proxyHost isEqualToString:@""] && proxyPort > 0 && ![proxyUsername isEqualToString:@""] && ![proxyPassword isEqualToString:@""]);
	GHAssertTrue(success,@"You need to supply the details of your authenticating proxy to run the proxy authentication test");	
	
	[[self queue] cancelAllOperations];
	[self setQueue:[ASINetworkQueue queue]];
	[[self queue] setDelegate:self];
	[[self queue] setRequestDidFinishSelector:@selector(requestDone:)];
	[[self queue] setRequestDidFailSelector:@selector(requestFailed:)];
	
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/basic-authentication"]];
	[[self queue] addOperation:request];
	
	[queue go];
	
	while (![self complete]) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
	}
}

- (void)requestDone:(ASIHTTPRequest *)request
{
	[self setComplete:YES];
}

- (void)authenticationNeededForRequest:(ASIHTTPRequest *)request
{
	[request setUsername:@"secret_username"];
	[request setPassword:@"secret_password"];
	[request retryUsingSuppliedCredentials];
}


@synthesize queue;
@synthesize complete;
@end

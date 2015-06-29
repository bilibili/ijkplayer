//
//  ASIDownloadCacheTests.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 03/05/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "ASIDownloadCacheTests.h"
#import "ASIDownloadCache.h"
#import "ASIHTTPRequest.h"

// Stop clang complaining about undeclared selectors
@interface ASIDownloadCacheTests ()
- (void)runCacheOnlyCallsRequestFinishedOnceTest;
- (void)finishCached:(ASIHTTPRequest *)request;
- (void)runRedirectTest;
@end


@implementation ASIDownloadCacheTests

- (void)testDownloadCache
{
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICachePermanentlyCacheStoragePolicy];
	[[ASIDownloadCache sharedCache] setDefaultCachePolicy:ASIUseDefaultCachePolicy];
	[ASIHTTPRequest setDefaultCache:nil];

	// Ensure a request without a download cache does not pull from the cache
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-away"]];
	[request startSynchronous];
	BOOL success = ![request didUseCachedResponse];
	GHAssertTrue(success,@"Used cached response when we shouldn't have");

	// Make all requests use the cache
	[ASIHTTPRequest setDefaultCache:[ASIDownloadCache sharedCache]];

	// Check a request isn't setting didUseCachedResponse when the data is not in the cache
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-away"]];
	[request startSynchronous];
	success = ![request didUseCachedResponse];
	GHAssertTrue(success,@"Cached response should not have been available");	

	// Test read from the cache
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-away"]];
	[request startSynchronous];
	success = [request didUseCachedResponse];
	GHAssertTrue(success,@"Failed to use cached response");

	// Test preventing reads from the cache
	[[ASIDownloadCache sharedCache] setShouldRespectCacheControlHeaders:YES];
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-away"]];
	[request setCachePolicy:ASIAskServerIfModifiedWhenStaleCachePolicy|ASIDoNotReadFromCacheCachePolicy];
	[request startSynchronous];
	success = ![request didUseCachedResponse];
	GHAssertTrue(success,@"Used the cache when reads were not enabled");

	//Empty the cache
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];

	// Test preventing writes to the cache
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-away"]];
	[request setCachePolicy:ASIAskServerIfModifiedWhenStaleCachePolicy|ASIDoNotWriteToCacheCachePolicy];
	[request startSynchronous];

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-away"]];
	[request setCachePolicy:ASIAskServerIfModifiedWhenStaleCachePolicy];
	[request startSynchronous];
	success = ![request didUseCachedResponse];
	GHAssertTrue(success,@"Used cached response when the cache should have been empty");

	// Test respecting etag
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/content-always-new"]];
	[request startSynchronous];
	success = ![request didUseCachedResponse];
	GHAssertTrue(success,@"Used cached response when we shouldn't have");

	// Etag will be different on the second request
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/content-always-new"]];

	// Note: we are forcing it to perform a conditional GET
	[request setCachePolicy:ASIDoNotReadFromCacheCachePolicy|ASIAskServerIfModifiedCachePolicy];
	[request startSynchronous];
	success = ![request didUseCachedResponse];
	GHAssertTrue(success,@"Used cached response when we shouldn't have");

	// Test ignoring server headers
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/no-cache"]];
	[[ASIDownloadCache sharedCache] setShouldRespectCacheControlHeaders:NO];
	[request startSynchronous];

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/no-cache"]];
	[[ASIDownloadCache sharedCache] setShouldRespectCacheControlHeaders:NO];
	[request startSynchronous];

	success = [request didUseCachedResponse];
	GHAssertTrue(success,@"Failed to use cached response");

	// Test ASIOnlyLoadIfNotCachedCachePolicy
	[[ASIDownloadCache sharedCache] setShouldRespectCacheControlHeaders:YES];
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/content-always-new"]];
	[[ASIDownloadCache sharedCache] setDefaultCachePolicy:ASIOnlyLoadIfNotCachedCachePolicy];
	[request startSynchronous];
	success = [request didUseCachedResponse];
	GHAssertTrue(success,@"Failed to use cached response");

	// Test clearing the cache
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-away"]];
	[request startSynchronous];
	success = ![request didUseCachedResponse];
	GHAssertTrue(success,@"Cached response should not have been available");

	// Test ASIAskServerIfModifiedWhenStaleCachePolicy
	[[ASIDownloadCache sharedCache] setDefaultCachePolicy:ASIUseDefaultCachePolicy];
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/content-always-new"]];
	[request setSecondsToCache:2];
	[request startSynchronous];

	// This request should not go to the network
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/content-always-new"]];
	[request startSynchronous];
	success = [request didUseCachedResponse];
	GHAssertTrue(success,@"Failed to use cached response");

	[NSThread sleepForTimeInterval:2];

	// This request will perform a conditional GET
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/content-always-new"]];
	[request setSecondsToCache:2];
	[request startSynchronous];
	success = ![request didUseCachedResponse];
	GHAssertTrue(success,@"Failed to use cached response");

	// Test ASIFallbackToCacheIfLoadFailsCachePolicy
	// Store something in the cache
	[request setURL:[NSURL URLWithString:@"http://"]];
	[request setResponseHeaders:[NSDictionary dictionaryWithObject:@"test" forKey:@"test"]];
	[request setRawResponseData:(NSMutableData *)[@"test" dataUsingEncoding:NSUTF8StringEncoding]];
	[[ASIDownloadCache sharedCache] storeResponseForRequest:request maxAge:0];

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://"]];
	[request setCachePolicy:ASIFallbackToCacheIfLoadFailsCachePolicy];
	[request startSynchronous];

	success = [request didUseCachedResponse];
	GHAssertTrue(success,@"Failed to use cached response");

	success = [[request responseString] isEqualToString:@"test"];
	GHAssertTrue(success,@"Failed to read cached response");

	success = [[[request responseHeaders] valueForKey:@"test"] isEqualToString:@"test"];
	GHAssertTrue(success,@"Failed to read cached response headers");

	// Remove the stuff from the cache, and try again
	[request setURL:[NSURL URLWithString:@"http://"]];
	[[ASIDownloadCache sharedCache] removeCachedDataForRequest:request];

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://"]];
	[request setCachePolicy:ASIFallbackToCacheIfLoadFailsCachePolicy];
	[request startSynchronous];

	success = ![request didUseCachedResponse];
	GHAssertTrue(success,@"Request says it used a cached response, but there wasn't one to use");

	success = ([request error] != nil);
	GHAssertTrue(success,@"Request had no error set");

	// Cache some data
	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-away"];
	request = [ASIHTTPRequest requestWithURL:url];
	[request startSynchronous];

	NSString *path = [[ASIDownloadCache sharedCache] pathToStoreCachedResponseDataForRequest:request];
	success = (path != nil);
	GHAssertTrue(success,@"Cache failed to store data");

	path = [[ASIDownloadCache sharedCache] pathToStoreCachedResponseHeadersForRequest:request];
	success = (path != nil);
	GHAssertTrue(success,@"Cache failed to store data");

	// Make sure data gets removed
	[[ASIDownloadCache sharedCache] removeCachedDataForURL:url];

	path = [[ASIDownloadCache sharedCache] pathToCachedResponseDataForURL:url];
	success = (path == nil);
	GHAssertTrue(success,@"Cache failed to remove data");

	path = [[ASIDownloadCache sharedCache] pathToCachedResponseHeadersForURL:url];
	success = (path == nil);
	GHAssertTrue(success,@"Cache failed to remove data");

	// Test ASIDontLoadCachePolicy
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/content-always-new"]];
	[request setCachePolicy:ASIDontLoadCachePolicy];
	[request startSynchronous];
	success = ![request error];
	GHAssertTrue(success,@"Request had an error");
	success = ![request contentLength];
	GHAssertTrue(success,@"Request had a response");
}

- (void)testDefaultPolicy
{
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_(abridged).txt"]];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request startSynchronous];
	BOOL success = ([request cachePolicy] == [[ASIDownloadCache sharedCache] defaultCachePolicy]);
	GHAssertTrue(success,@"Failed to use the cache policy from the cache");
	
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_(abridged).txt"]];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request setCachePolicy:ASIOnlyLoadIfNotCachedCachePolicy];
	[request startSynchronous];
	success = ([request cachePolicy] == ASIOnlyLoadIfNotCachedCachePolicy);
	GHAssertTrue(success,@"Failed to use the cache policy from the cache");
}

- (void)testNoCache
{

	// Test server no-cache headers
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	NSArray *cacheHeaders = [NSArray arrayWithObjects:@"cache-control/no-cache",@"cache-control/no-store",@"pragma/no-cache",nil];
	for (NSString *cacheType in cacheHeaders) {
		NSString *url = [NSString stringWithFormat:@"http://allseeing-i.com/ASIHTTPRequest/tests/%@",cacheType];
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:url]];
		[request setDownloadCache:[ASIDownloadCache sharedCache]];
		[request startSynchronous];
		
		request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:url]];
		[request setDownloadCache:[ASIDownloadCache sharedCache]];
		[request startSynchronous];
		BOOL success = ![request didUseCachedResponse];
		GHAssertTrue(success,@"Data should not have been stored in the cache");
	}
}

- (void)testSharedCache
{
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];

	// Make using the cache automatic
	[ASIHTTPRequest setDefaultCache:[ASIDownloadCache sharedCache]];
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_(abridged).txt"]];
	[request startSynchronous];
	
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_(abridged).txt"]];
	[request startSynchronous];
	BOOL success = [request didUseCachedResponse];
	GHAssertTrue(success,@"Failed to use data cached in default cache");
	
	[ASIHTTPRequest setDefaultCache:nil];
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_(abridged).txt"]];
	[request startSynchronous];
	success = ![request didUseCachedResponse];
	GHAssertTrue(success,@"Should not have used data cached in default cache");
}

- (void)testExpiry
{
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	[[ASIDownloadCache sharedCache] setDefaultCachePolicy:ASIAskServerIfModifiedCachePolicy];

	NSArray *headers = [NSArray arrayWithObjects:@"last-modified",@"etag",@"expires",@"max-age",nil];
	for (NSString *header in headers) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:[NSString stringWithFormat:@"http://allseeing-i.com/ASIHTTPRequest/tests/content-always-new/%@",header]]];
		[request setDownloadCache:[ASIDownloadCache sharedCache]];
		[request startSynchronous];

		if ([header isEqualToString:@"last-modified"]) {
			[NSThread sleepForTimeInterval:2];
		}

		request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:[NSString stringWithFormat:@"http://allseeing-i.com/ASIHTTPRequest/tests/content-always-new/%@",header]]];
		[request setDownloadCache:[ASIDownloadCache sharedCache]];
		[request startSynchronous];
		BOOL success = ![request didUseCachedResponse];
		GHAssertTrue(success,@"Cached data should have expired");
	}
}

- (void)testMaxAgeParsing
{
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	[[ASIDownloadCache sharedCache] setDefaultCachePolicy:ASIUseDefaultCachePolicy];
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-control-max-age-parsing"]];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request startSynchronous];

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-control-max-age-parsing"]];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request startSynchronous];
	BOOL success = [request didUseCachedResponse];
	GHAssertTrue(success,@"Failed to use cached response");
}

- (void)testExtensionHandling
{
	NSArray *extensions = [ASIDownloadCache fileExtensionsToHandleAsHTML];
	for (NSString *extension in extensions) {
		NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"http://allseeing-i.com/file.%@",extension]];
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:url];
		NSString *path = [[ASIDownloadCache sharedCache] pathToStoreCachedResponseDataForRequest:request];
		BOOL success = [[path pathExtension] isEqualToString:@"html"];
		GHAssertTrue(success, @"Failed to use html extension on cached path for a resource we know a webview won't be able to open locally");
	}

	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/"];
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:url];
	NSString *path = [[ASIDownloadCache sharedCache] pathToStoreCachedResponseDataForRequest:request];
	BOOL success = [[path pathExtension] isEqualToString:@"html"];
	GHAssertTrue(success, @"Failed to use html extension on cached path for a url without an extension");

	url = [NSURL URLWithString:@"http://allseeing-i.com/i/logo.png"];
	request = [ASIHTTPRequest requestWithURL:url];
	path = [[ASIDownloadCache sharedCache] pathToStoreCachedResponseDataForRequest:request];
	success = [[path pathExtension] isEqualToString:@"png"];
	GHAssertTrue(success, @"Failed to preserve file extension on cached path");
}

- (void)testCustomExpiry
{
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	[[ASIDownloadCache sharedCache] setDefaultCachePolicy:ASIUseDefaultCachePolicy];
	[[ASIDownloadCache sharedCache] setShouldRespectCacheControlHeaders:YES];

	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-away"]];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request setSecondsToCache:-2];
	[request startSynchronous];

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-away"]];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request startSynchronous];

	BOOL success = ![request didUseCachedResponse];
	GHAssertTrue(success,@"Cached data should have expired");

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-away"]];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request setSecondsToCache:20];
	[request startSynchronous];

	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cache-away"]];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request startSynchronous];

	success = [request didUseCachedResponse];
	GHAssertTrue(success,@"Cached data should have been used");
}

- (void)test304
{
	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_great_american_novel_(abridged).txt"];

	// Test default cache policy
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	[[ASIDownloadCache sharedCache] setDefaultCachePolicy:ASIUseDefaultCachePolicy];
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:url];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request startSynchronous];

	request = [ASIHTTPRequest requestWithURL:url];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request startSynchronous];
	BOOL success = ([request responseStatusCode] == 200);
	GHAssertTrue(success,@"Failed to perform a conditional get");

	success = [request didUseCachedResponse];
	GHAssertTrue(success,@"Cached data should have been used");

	success = ([[request responseData] length]);
	GHAssertTrue(success,@"Response was empty");

	// Test 304 updates expiry date
	url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/content_not_modified_but_expires_tomorrow"];
	request = [ASIHTTPRequest requestWithURL:url];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request startSynchronous];

	NSTimeInterval expiryTimestamp = [[[[ASIDownloadCache sharedCache] cachedResponseHeadersForURL:url] objectForKey:@"X-ASIHTTPRequest-Expires"] doubleValue];

	// Wait to give the expiry date a chance to change
	sleep(2);

	request = [ASIHTTPRequest requestWithURL:url];
	[request setCachePolicy:ASIAskServerIfModifiedCachePolicy];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request startSynchronous];

	success = [request didUseCachedResponse];
	GHAssertTrue(success, @"Cached data should have been used");

	NSTimeInterval newExpiryTimestamp = [[[[ASIDownloadCache sharedCache] cachedResponseHeadersForURL:url] objectForKey:@"X-ASIHTTPRequest-Expires"] doubleValue];
	NSLog(@"%@",[request responseString]);
	success = (newExpiryTimestamp > expiryTimestamp);
	GHAssertTrue(success, @"Failed to update expiry timestamp on 304");
}

- (void)testStringEncoding
{
	[ASIHTTPRequest setDefaultCache:[ASIDownloadCache sharedCache]];
	[[ASIDownloadCache sharedCache] setShouldRespectCacheControlHeaders:NO];
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	[[ASIDownloadCache sharedCache] setDefaultCachePolicy:ASIOnlyLoadIfNotCachedCachePolicy];

	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/Character-Encoding/UTF-16"];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	BOOL success = ([request responseEncoding] == NSUnicodeStringEncoding);
	GHAssertTrue(success,@"Got the wrong encoding back, cannot proceed with test");

	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	success = [request responseEncoding] == NSUnicodeStringEncoding;
	GHAssertTrue(success,@"Failed to set the correct encoding on the cached response");

	[ASIHTTPRequest setDefaultCache:nil];
}

- (void)testCookies
{
	[ASIHTTPRequest setDefaultCache:[ASIDownloadCache sharedCache]];
	[[ASIDownloadCache sharedCache] setShouldRespectCacheControlHeaders:NO];
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	[[ASIDownloadCache sharedCache] setDefaultCachePolicy:ASIOnlyLoadIfNotCachedCachePolicy];

	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/set_cookie"];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];
	NSArray *cookies = [request responseCookies];

	BOOL success = ([cookies count]);
	GHAssertTrue(success,@"Got no cookies back, cannot proceed with test");

	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request startSynchronous];

	NSUInteger i;
	for (i=0; i<[cookies count]; i++) {
		if (![[[cookies objectAtIndex:i] name] isEqualToString:[[[request responseCookies] objectAtIndex:i] name]]) {
			GHAssertTrue(success,@"Failed to set response cookies correctly");
			return;
		}
	}

	[ASIHTTPRequest setDefaultCache:nil];
}

// Text fix for a bug where the didFinishSelector would be called twice for a cached response using ASIReloadIfDifferentCachePolicy
- (void)testCacheOnlyCallsRequestFinishedOnce
{
	// Run this request on the main thread to force delegate calls to happen synchronously
	[self performSelectorOnMainThread:@selector(runCacheOnlyCallsRequestFinishedOnceTest) withObject:nil waitUntilDone:YES];
}

- (void)runCacheOnlyCallsRequestFinishedOnceTest
{
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/i/logo.png"]];
	[request setCachePolicy:ASIUseDefaultCachePolicy];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request setDelegate:self];
	[request startSynchronous];

	requestsFinishedCount = 0;
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/i/logo.png"]];
	[request setCachePolicy:ASIUseDefaultCachePolicy];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request setDidFinishSelector:@selector(finishCached:)];
	[request setDelegate:self];
	[request startSynchronous];

	BOOL success = (requestsFinishedCount == 1);
	GHAssertTrue(success,@"didFinishSelector called more than once");
}

- (void)finishCached:(ASIHTTPRequest *)request
{
	requestsFinishedCount++;
}

- (void)testRedirect
{
	// Run this request on the main thread to force delegate calls to happen synchronously
	[self performSelectorOnMainThread:@selector(runRedirectTest) withObject:nil waitUntilDone:YES];
}

- (void)runRedirectTest
{
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	[[ASIDownloadCache sharedCache] clearCachedResponsesForStoragePolicy:ASICachePermanentlyCacheStoragePolicy];
	[[ASIDownloadCache sharedCache] setDefaultCachePolicy:ASIUseDefaultCachePolicy];
	[ASIHTTPRequest setDefaultCache:[ASIDownloadCache sharedCache]];

	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cached-redirect"]];
	[request startSynchronous];

	BOOL success = ([[[request url] absoluteString] isEqualToString:@"http://allseeing-i.com/i/logo.png"]);
	GHAssertTrue(success,@"Request did not redirect correctly, cannot proceed with test");

	requestRedirectedWasCalled = NO;
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/cached-redirect"]];
	[request setDelegate:self];
	[request startSynchronous];

	success = ([request didUseCachedResponse]);
	GHAssertTrue(success,@"Failed to cache final response");

	GHAssertTrue(requestRedirectedWasCalled,@"Failed to call requestRedirected");
}

- (void)requestRedirected:(ASIHTTPRequest *)redirected
{
	requestRedirectedWasCalled = YES;
}

- (void)request:(ASIHTTPRequest *)request willRedirectToURL:(NSURL *)newURL
{
	BOOL success = ([[newURL absoluteString] isEqualToString:@"http://allseeing-i.com/i/logo.png"]);
	GHAssertTrue(success,@"Request did not redirect correctly, cannot proceed with test");

	success = ([request didUseCachedResponse]);
	GHAssertTrue(success,@"Failed to cache redirect response");

	[request redirectToURL:newURL];
}

- (void)testCachedFileOverwritten
{
	// Test for https://github.com/pokeb/asi-http-request/pull/211
	// This test ensures that items in the cache are correctly overwritten when a downloadDestinationPath is set,
	// and they need to be copied to the cache at the end of the request

	// This url returns different content every time
	NSURL *url = [NSURL URLWithString:@"http://asi/ASIHTTPRequest/tests/random-content"];
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:url];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request setSecondsToCache:0.5f];
	[request startSynchronous];

	NSString *path = [[ASIDownloadCache sharedCache] pathToCachedResponseDataForURL:url];
	NSString *content = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:NULL];

	sleep(1);

	request = [ASIHTTPRequest requestWithURL:url];
	[request setDownloadCache:[ASIDownloadCache sharedCache]];
	[request setDownloadDestinationPath:[[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"test.html"]];
	[request startSynchronous];

	NSString *content2 = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:NULL];

	BOOL success = ![content isEqualToString:content2];
	GHAssertTrue(success, @"Failed to overwrite response in cache");
}

@end

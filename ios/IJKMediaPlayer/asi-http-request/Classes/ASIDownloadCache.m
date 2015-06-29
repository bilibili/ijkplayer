//
//  ASIDownloadCache.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 01/05/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "ASIDownloadCache.h"
#import "ASIHTTPRequest.h"
#import <CommonCrypto/CommonHMAC.h>

static ASIDownloadCache *sharedCache = nil;

static NSString *sessionCacheFolder = @"SessionStore";
static NSString *permanentCacheFolder = @"PermanentStore";
static NSArray *fileExtensionsToHandleAsHTML = nil;

@interface ASIDownloadCache ()
+ (NSString *)keyForURL:(NSURL *)url;
- (NSString *)pathToFile:(NSString *)file;
@end

@implementation ASIDownloadCache

+ (void)initialize
{
	if (self == [ASIDownloadCache class]) {
		// Obviously this is not an exhaustive list, but hopefully these are the most commonly used and this will 'just work' for the widest range of people
		// I imagine many web developers probably use url rewriting anyway
		fileExtensionsToHandleAsHTML = [[NSArray alloc] initWithObjects:@"asp",@"aspx",@"jsp",@"php",@"rb",@"py",@"pl",@"cgi", nil];
	}
}

- (id)init
{
	self = [super init];
	[self setShouldRespectCacheControlHeaders:YES];
	[self setDefaultCachePolicy:ASIUseDefaultCachePolicy];
	[self setAccessLock:[[[NSRecursiveLock alloc] init] autorelease]];
	return self;
}

+ (id)sharedCache
{
	if (!sharedCache) {
		@synchronized(self) {
			if (!sharedCache) {
				sharedCache = [[self alloc] init];
				[sharedCache setStoragePath:[[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0] stringByAppendingPathComponent:@"ASIHTTPRequestCache"]];
			}
		}
	}
	return sharedCache;
}

- (void)dealloc
{
	[storagePath release];
	[accessLock release];
	[super dealloc];
}

- (NSString *)storagePath
{
	[[self accessLock] lock];
	NSString *p = [[storagePath retain] autorelease];
	[[self accessLock] unlock];
	return p;
}


- (void)setStoragePath:(NSString *)path
{
	[[self accessLock] lock];
	[self clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	[storagePath release];
	storagePath = [path retain];

	NSFileManager *fileManager = [[[NSFileManager alloc] init] autorelease];

	BOOL isDirectory = NO;
	NSArray *directories = [NSArray arrayWithObjects:path,[path stringByAppendingPathComponent:sessionCacheFolder],[path stringByAppendingPathComponent:permanentCacheFolder],nil];
	for (NSString *directory in directories) {
		BOOL exists = [fileManager fileExistsAtPath:directory isDirectory:&isDirectory];
		if (exists && !isDirectory) {
			[[self accessLock] unlock];
			[NSException raise:@"FileExistsAtCachePath" format:@"Cannot create a directory for the cache at '%@', because a file already exists",directory];
		} else if (!exists) {
			[fileManager createDirectoryAtPath:directory withIntermediateDirectories:NO attributes:nil error:nil];
			if (![fileManager fileExistsAtPath:directory]) {
				[[self accessLock] unlock];
				[NSException raise:@"FailedToCreateCacheDirectory" format:@"Failed to create a directory for the cache at '%@'",directory];
			}
		}
	}
	[self clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
	[[self accessLock] unlock];
}

- (void)updateExpiryForRequest:(ASIHTTPRequest *)request maxAge:(NSTimeInterval)maxAge
{
	NSString *headerPath = [self pathToStoreCachedResponseHeadersForRequest:request];
	NSMutableDictionary *cachedHeaders = [NSMutableDictionary dictionaryWithContentsOfFile:headerPath];
	if (!cachedHeaders) {
		return;
	}
	NSDate *expires = [self expiryDateForRequest:request maxAge:maxAge];
	if (!expires) {
		return;
	}
	[cachedHeaders setObject:[NSNumber numberWithDouble:[expires timeIntervalSince1970]] forKey:@"X-ASIHTTPRequest-Expires"];
	[cachedHeaders writeToFile:headerPath atomically:NO];
}

- (NSDate *)expiryDateForRequest:(ASIHTTPRequest *)request maxAge:(NSTimeInterval)maxAge
{
  return [ASIHTTPRequest expiryDateForRequest:request maxAge:maxAge];
}

- (void)storeResponseForRequest:(ASIHTTPRequest *)request maxAge:(NSTimeInterval)maxAge
{
	[[self accessLock] lock];

	if ([request error] || ![request responseHeaders] || ([request cachePolicy] & ASIDoNotWriteToCacheCachePolicy)) {
		[[self accessLock] unlock];
		return;
	}

	// We only cache 200/OK or redirect reponses (redirect responses are cached so the cache works better with no internet connection)
	int responseCode = [request responseStatusCode];
	if (responseCode != 200 && responseCode != 301 && responseCode != 302 && responseCode != 303 && responseCode != 307) {
		[[self accessLock] unlock];
		return;
	}

	if ([self shouldRespectCacheControlHeaders] && ![[self class] serverAllowsResponseCachingForRequest:request]) {
		[[self accessLock] unlock];
		return;
	}

	NSString *headerPath = [self pathToStoreCachedResponseHeadersForRequest:request];
	NSString *dataPath = [self pathToStoreCachedResponseDataForRequest:request];

	NSMutableDictionary *responseHeaders = [NSMutableDictionary dictionaryWithDictionary:[request responseHeaders]];
	if ([request isResponseCompressed]) {
		[responseHeaders removeObjectForKey:@"Content-Encoding"];
	}

	// Create a special 'X-ASIHTTPRequest-Expires' header
	// This is what we use for deciding if cached data is current, rather than parsing the expires / max-age headers individually each time
	// We store this as a timestamp to make reading it easier as NSDateFormatter is quite expensive

	NSDate *expires = [self expiryDateForRequest:request maxAge:maxAge];
	if (expires) {
		[responseHeaders setObject:[NSNumber numberWithDouble:[expires timeIntervalSince1970]] forKey:@"X-ASIHTTPRequest-Expires"];
	}

	// Store the response code in a custom header so we can reuse it later

	// We'll change 304/Not Modified to 200/OK because this is likely to be us updating the cached headers with a conditional GET
	int statusCode = [request responseStatusCode];
	if (statusCode == 304) {
		statusCode = 200;
	}
	[responseHeaders setObject:[NSNumber numberWithInt:statusCode] forKey:@"X-ASIHTTPRequest-Response-Status-Code"];

	[responseHeaders writeToFile:headerPath atomically:NO];

	if ([request responseData]) {
		[[request responseData] writeToFile:dataPath atomically:NO];
	} else if ([request downloadDestinationPath] && ![[request downloadDestinationPath] isEqualToString:dataPath]) {        
		NSError *error = nil;
        NSFileManager* manager = [[NSFileManager alloc] init];
        if ([manager fileExistsAtPath:dataPath]) {
            [manager removeItemAtPath:dataPath error:&error];
        }
        [manager copyItemAtPath:[request downloadDestinationPath] toPath:dataPath error:&error];
        [manager release];
	}
	[[self accessLock] unlock];
}

- (NSDictionary *)cachedResponseHeadersForURL:(NSURL *)url
{
	NSString *path = [self pathToCachedResponseHeadersForURL:url];
	if (path) {
		return [NSDictionary dictionaryWithContentsOfFile:path];
	}
	return nil;
}

- (NSData *)cachedResponseDataForURL:(NSURL *)url
{
	NSString *path = [self pathToCachedResponseDataForURL:url];
	if (path) {
		return [NSData dataWithContentsOfFile:path];
	}
	return nil;
}

- (NSString *)pathToCachedResponseDataForURL:(NSURL *)url
{
	// Grab the file extension, if there is one. We do this so we can save the cached response with the same file extension - this is important if you want to display locally cached data in a web view 
	NSString *extension = [[url path] pathExtension];

	// If the url doesn't have an extension, we'll add one so a webview can read it when locally cached
	// If the url has the extension of a common web scripting language, we'll change the extension on the cached path to html for the same reason
	if (![extension length] || [[[self class] fileExtensionsToHandleAsHTML] containsObject:[extension lowercaseString]]) {
		extension = @"html";
	}
	return [self pathToFile:[[[self class] keyForURL:url] stringByAppendingPathExtension:extension]];
}

+ (NSArray *)fileExtensionsToHandleAsHTML
{
	return fileExtensionsToHandleAsHTML;
}


- (NSString *)pathToCachedResponseHeadersForURL:(NSURL *)url
{
	return [self pathToFile:[[[self class] keyForURL:url] stringByAppendingPathExtension:@"cachedheaders"]];
}

- (NSString *)pathToFile:(NSString *)file
{
	[[self accessLock] lock];
	if (![self storagePath]) {
		[[self accessLock] unlock];
		return nil;
	}

	NSFileManager *fileManager = [[[NSFileManager alloc] init] autorelease];

	// Look in the session store
	NSString *dataPath = [[[self storagePath] stringByAppendingPathComponent:sessionCacheFolder] stringByAppendingPathComponent:file];
	if ([fileManager fileExistsAtPath:dataPath]) {
		[[self accessLock] unlock];
		return dataPath;
	}
	// Look in the permanent store
	dataPath = [[[self storagePath] stringByAppendingPathComponent:permanentCacheFolder] stringByAppendingPathComponent:file];
	if ([fileManager fileExistsAtPath:dataPath]) {
		[[self accessLock] unlock];
		return dataPath;
	}
	[[self accessLock] unlock];
	return nil;
}


- (NSString *)pathToStoreCachedResponseDataForRequest:(ASIHTTPRequest *)request
{
	[[self accessLock] lock];
	if (![self storagePath]) {
		[[self accessLock] unlock];
		return nil;
	}

	NSString *path = [[self storagePath] stringByAppendingPathComponent:([request cacheStoragePolicy] == ASICacheForSessionDurationCacheStoragePolicy ? sessionCacheFolder : permanentCacheFolder)];

	// Grab the file extension, if there is one. We do this so we can save the cached response with the same file extension - this is important if you want to display locally cached data in a web view 
	NSString *extension = [[[request url] path] pathExtension];

	// If the url doesn't have an extension, we'll add one so a webview can read it when locally cached
	// If the url has the extension of a common web scripting language, we'll change the extension on the cached path to html for the same reason
	if (![extension length] || [[[self class] fileExtensionsToHandleAsHTML] containsObject:[extension lowercaseString]]) {
		extension = @"html";
	}
	path =  [path stringByAppendingPathComponent:[[[self class] keyForURL:[request url]] stringByAppendingPathExtension:extension]];
	[[self accessLock] unlock];
	return path;
}

- (NSString *)pathToStoreCachedResponseHeadersForRequest:(ASIHTTPRequest *)request
{
	[[self accessLock] lock];
	if (![self storagePath]) {
		[[self accessLock] unlock];
		return nil;
	}
	NSString *path = [[self storagePath] stringByAppendingPathComponent:([request cacheStoragePolicy] == ASICacheForSessionDurationCacheStoragePolicy ? sessionCacheFolder : permanentCacheFolder)];
	path =  [path stringByAppendingPathComponent:[[[self class] keyForURL:[request url]] stringByAppendingPathExtension:@"cachedheaders"]];
	[[self accessLock] unlock];
	return path;
}

- (void)removeCachedDataForURL:(NSURL *)url
{
	[[self accessLock] lock];
	if (![self storagePath]) {
		[[self accessLock] unlock];
		return;
	}
	NSFileManager *fileManager = [[[NSFileManager alloc] init] autorelease];

	NSString *path = [self pathToCachedResponseHeadersForURL:url];
	if (path) {
		[fileManager removeItemAtPath:path error:NULL];
	}

	path = [self pathToCachedResponseDataForURL:url];
	if (path) {
		[fileManager removeItemAtPath:path error:NULL];
	}
	[[self accessLock] unlock];
}

- (void)removeCachedDataForRequest:(ASIHTTPRequest *)request
{
	[self removeCachedDataForURL:[request url]];
}

- (BOOL)isCachedDataCurrentForRequest:(ASIHTTPRequest *)request
{
	[[self accessLock] lock];
	if (![self storagePath]) {
		[[self accessLock] unlock];
		return NO;
	}
	NSDictionary *cachedHeaders = [self cachedResponseHeadersForURL:[request url]];
	if (!cachedHeaders) {
		[[self accessLock] unlock];
		return NO;
	}
	NSString *dataPath = [self pathToCachedResponseDataForURL:[request url]];
	if (!dataPath) {
		[[self accessLock] unlock];
		return NO;
	}

	// New content is not different
	if ([request responseStatusCode] == 304) {
		[[self accessLock] unlock];
		return YES;
	}

	// If we already have response headers for this request, check to see if the new content is different
	// We check [request complete] so that we don't end up comparing response headers from a redirection with these
	if ([request responseHeaders] && [request complete]) {

		// If the Etag or Last-Modified date are different from the one we have, we'll have to fetch this resource again
		NSArray *headersToCompare = [NSArray arrayWithObjects:@"Etag",@"Last-Modified",nil];
		for (NSString *header in headersToCompare) {
			if (![[[request responseHeaders] objectForKey:header] isEqualToString:[cachedHeaders objectForKey:header]]) {
				[[self accessLock] unlock];
				return NO;
			}
		}
	}

	if ([self shouldRespectCacheControlHeaders]) {

		// Look for X-ASIHTTPRequest-Expires header to see if the content is out of date
		NSNumber *expires = [cachedHeaders objectForKey:@"X-ASIHTTPRequest-Expires"];
		if (expires) {
			if ([[NSDate dateWithTimeIntervalSince1970:[expires doubleValue]] timeIntervalSinceNow] >= 0) {
				[[self accessLock] unlock];
				return YES;
			}
		}

		// No explicit expiration time sent by the server
		[[self accessLock] unlock];
		return NO;
	}
	

	[[self accessLock] unlock];
	return YES;
}

- (ASICachePolicy)defaultCachePolicy
{
	[[self accessLock] lock];
	ASICachePolicy cp = defaultCachePolicy;
	[[self accessLock] unlock];
	return cp;
}


- (void)setDefaultCachePolicy:(ASICachePolicy)cachePolicy
{
	[[self accessLock] lock];
	if (!cachePolicy) {
		defaultCachePolicy = ASIAskServerIfModifiedWhenStaleCachePolicy;
	}  else {
		defaultCachePolicy = cachePolicy;	
	}
	[[self accessLock] unlock];
}

- (void)clearCachedResponsesForStoragePolicy:(ASICacheStoragePolicy)storagePolicy
{
	[[self accessLock] lock];
	if (![self storagePath]) {
		[[self accessLock] unlock];
		return;
	}
	NSString *path = [[self storagePath] stringByAppendingPathComponent:(storagePolicy == ASICacheForSessionDurationCacheStoragePolicy ? sessionCacheFolder : permanentCacheFolder)];

	NSFileManager *fileManager = [[[NSFileManager alloc] init] autorelease];

	BOOL isDirectory = NO;
	BOOL exists = [fileManager fileExistsAtPath:path isDirectory:&isDirectory];
	if (!exists || !isDirectory) {
		[[self accessLock] unlock];
		return;
	}
	NSError *error = nil;
	NSArray *cacheFiles = [fileManager contentsOfDirectoryAtPath:path error:&error];
	if (error) {
		[[self accessLock] unlock];
		[NSException raise:@"FailedToTraverseCacheDirectory" format:@"Listing cache directory failed at path '%@'",path];	
	}
	for (NSString *file in cacheFiles) {
		[fileManager removeItemAtPath:[path stringByAppendingPathComponent:file] error:&error];
		if (error) {
			[[self accessLock] unlock];
			[NSException raise:@"FailedToRemoveCacheFile" format:@"Failed to remove cached data at path '%@'",path];
		}
	}
	[[self accessLock] unlock];
}

+ (BOOL)serverAllowsResponseCachingForRequest:(ASIHTTPRequest *)request
{
	NSString *cacheControl = [[[request responseHeaders] objectForKey:@"Cache-Control"] lowercaseString];
	if (cacheControl) {
		if ([cacheControl isEqualToString:@"no-cache"] || [cacheControl isEqualToString:@"no-store"]) {
			return NO;
		}
	}
	NSString *pragma = [[[request responseHeaders] objectForKey:@"Pragma"] lowercaseString];
	if (pragma) {
		if ([pragma isEqualToString:@"no-cache"]) {
			return NO;
		}
	}
	return YES;
}

+ (NSString *)keyForURL:(NSURL *)url
{
	NSString *urlString = [url absoluteString];
	if ([urlString length] == 0) {
		return nil;
	}

	// Strip trailing slashes so http://allseeing-i.com/ASIHTTPRequest/ is cached the same as http://allseeing-i.com/ASIHTTPRequest
	if ([[urlString substringFromIndex:[urlString length]-1] isEqualToString:@"/"]) {
		urlString = [urlString substringToIndex:[urlString length]-1];
	}

	// Borrowed from: http://stackoverflow.com/questions/652300/using-md5-hash-on-a-string-in-cocoa
	const char *cStr = [urlString UTF8String];
	unsigned char result[16];
	CC_MD5(cStr, (CC_LONG)strlen(cStr), result);
	return [NSString stringWithFormat:@"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",result[0], result[1], result[2], result[3], result[4], result[5], result[6], result[7],result[8], result[9], result[10], result[11],result[12], result[13], result[14], result[15]]; 	
}

- (BOOL)canUseCachedDataForRequest:(ASIHTTPRequest *)request
{
	// Ensure the request is allowed to read from the cache
	if ([request cachePolicy] & ASIDoNotReadFromCacheCachePolicy) {
		return NO;

	// If we don't want to load the request whatever happens, always pretend we have cached data even if we don't
	} else if ([request cachePolicy] & ASIDontLoadCachePolicy) {
		return YES;
	}

	NSDictionary *headers = [self cachedResponseHeadersForURL:[request url]];
	if (!headers) {
		return NO;
	}
	NSString *dataPath = [self pathToCachedResponseDataForURL:[request url]];
	if (!dataPath) {
		return NO;
	}

	// If we get here, we have cached data

	// If we have cached data, we can use it
	if ([request cachePolicy] & ASIOnlyLoadIfNotCachedCachePolicy) {
		return YES;

	// If we want to fallback to the cache after an error
	} else if ([request complete] && [request cachePolicy] & ASIFallbackToCacheIfLoadFailsCachePolicy) {
		return YES;

	// If we have cached data that is current, we can use it
	} else if ([request cachePolicy] & ASIAskServerIfModifiedWhenStaleCachePolicy) {
		if ([self isCachedDataCurrentForRequest:request]) {
			return YES;
		}

	// If we've got headers from a conditional GET and the cached data is still current, we can use it
	} else if ([request cachePolicy] & ASIAskServerIfModifiedCachePolicy) {
		if (![request responseHeaders]) {
			return NO;
		} else if ([self isCachedDataCurrentForRequest:request]) {
			return YES;
		}
	}
	return NO;
}

@synthesize storagePath;
@synthesize defaultCachePolicy;
@synthesize accessLock;
@synthesize shouldRespectCacheControlHeaders;
@end

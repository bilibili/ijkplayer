//
//  ASICloudFilesRequest.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Michael Mayo on 22/12/09.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//
// A class for accessing data stored on Rackspace's Cloud Files Service
// http://www.rackspacecloud.com/cloud_hosting_products/files
// 
// Cloud Files Developer Guide:
// http://docs.rackspacecloud.com/servers/api/cs-devguide-latest.pdf

#import "ASICloudFilesRequest.h"

static NSString *username = nil;
static NSString *apiKey = nil;
static NSString *authToken = nil;
static NSString *storageURL = nil;
static NSString *cdnManagementURL = nil;
static NSString *rackspaceCloudAuthURL = @"https://auth.api.rackspacecloud.com/v1.0";

static NSRecursiveLock *accessDetailsLock = nil;

@implementation ASICloudFilesRequest

+ (void)initialize
{
	if (self == [ASICloudFilesRequest class]) {
		accessDetailsLock = [[NSRecursiveLock alloc] init];
	}
}

#pragma mark -
#pragma mark Attributes and Service URLs

+ (NSString *)authToken {
	return authToken;
}

+ (NSString *)storageURL {
	return storageURL;
}

+ (NSString *)cdnManagementURL {
	return cdnManagementURL;
}

#pragma mark -
#pragma mark Authentication

+ (id)authenticationRequest
{
	[accessDetailsLock lock];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:[NSURL URLWithString:rackspaceCloudAuthURL]] autorelease];
	[request addRequestHeader:@"X-Auth-User" value:username];
	[request addRequestHeader:@"X-Auth-Key" value:apiKey];
	[accessDetailsLock unlock];
	return request;
}

+ (NSError *)authenticate
{
	[accessDetailsLock lock];
	ASIHTTPRequest *request = [ASICloudFilesRequest authenticationRequest];
	[request startSynchronous];
	
	if (![request error]) {
		NSDictionary *responseHeaders = [request responseHeaders];
		authToken = [responseHeaders objectForKey:@"X-Auth-Token"];
		storageURL = [responseHeaders objectForKey:@"X-Storage-Url"];
		cdnManagementURL = [responseHeaders objectForKey:@"X-CDN-Management-Url"];
        
        // there is a bug in the Cloud Files API for some older accounts that causes
        // the CDN URL to come back in a slightly different header
        if (!cdnManagementURL) {
            cdnManagementURL = [responseHeaders objectForKey:@"X-Cdn-Management-Url"];
        }
	}
	[accessDetailsLock unlock];
	return [request error];
}

+ (NSString *)username
{
	return username;
}

+ (void)setUsername:(NSString *)newUsername
{
	[accessDetailsLock lock];
	[username release];
	username = [newUsername retain];
	[accessDetailsLock unlock];
}

+ (NSString *)apiKey {
	return apiKey;
}

+ (void)setApiKey:(NSString *)newApiKey
{
	[accessDetailsLock lock];
	[apiKey release];
	apiKey = [newApiKey retain];
	[accessDetailsLock unlock];
}

#pragma mark -
#pragma mark Date Parser

-(NSDate *)dateFromString:(NSString *)dateString
{
	// We store our date formatter in the calling thread's dictionary
	// NSDateFormatter is not thread-safe, this approach ensures each formatter is only used on a single thread
	// This formatter can be reused many times in parsing a single response, so it would be expensive to keep creating new date formatters
	NSMutableDictionary *threadDict = [[NSThread currentThread] threadDictionary];
	NSDateFormatter *dateFormatter = [threadDict objectForKey:@"ASICloudFilesResponseDateFormatter"];
	if (dateFormatter == nil) {
		dateFormatter = [[[NSDateFormatter alloc] init] autorelease];
		[dateFormatter setLocale:[[[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"] autorelease]];
		// example: 2009-11-04T19:46:20.192723
		[dateFormatter setDateFormat:@"yyyy-MM-dd'T'H:mm:ss.SSSSSS"];
		[threadDict setObject:dateFormatter forKey:@"ASICloudFilesResponseDateFormatter"];
	}
	return [dateFormatter dateFromString:dateString];
}

@end

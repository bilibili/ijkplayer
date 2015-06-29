//
//  ASIS3BucketObject.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 13/07/2009.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//

#import "ASIS3BucketObject.h"
#import "ASIS3ObjectRequest.h"

@implementation ASIS3BucketObject

+ (id)objectWithBucket:(NSString *)theBucket
{
	ASIS3BucketObject *object = [[[self alloc] init] autorelease];
	[object setBucket:theBucket];
	return object;
}

- (void)dealloc
{
	[bucket release];
	[key release];
	[lastModified release];
	[ETag release];
	[ownerID release];
	[ownerName release];
	[super dealloc];
}

- (ASIS3ObjectRequest *)GETRequest
{
	return [ASIS3ObjectRequest requestWithBucket:[self bucket] key:[self key]];
}

- (ASIS3ObjectRequest *)PUTRequestWithFile:(NSString *)filePath
{
	return [ASIS3ObjectRequest PUTRequestForFile:filePath withBucket:[self bucket] key:[self key]];
}

- (ASIS3ObjectRequest *)DELETERequest
{
	ASIS3ObjectRequest *request = [ASIS3ObjectRequest requestWithBucket:[self bucket] key:[self key]];
	[request setRequestMethod:@"DELETE"];
	return request;
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"Key: %@ lastModified: %@ ETag: %@ size: %llu ownerID: %@ ownerName: %@",[self key],[self lastModified],[self ETag],[self size],[self ownerID],[self ownerName]];
}

- (id)copyWithZone:(NSZone *)zone
{
	ASIS3BucketObject *newBucketObject = [[[self class] alloc] init];
	[newBucketObject setBucket:[self bucket]];
	[newBucketObject setKey:[self key]];
	[newBucketObject setLastModified:[self lastModified]];
	[newBucketObject setETag:[self ETag]];
	[newBucketObject setSize:[self size]];
	[newBucketObject setOwnerID:[self ownerID]];
	[newBucketObject setOwnerName:[self ownerName]];
	return newBucketObject;
}

@synthesize bucket;
@synthesize key;
@synthesize lastModified;
@synthesize ETag;
@synthesize size;
@synthesize ownerID;
@synthesize ownerName;
@end

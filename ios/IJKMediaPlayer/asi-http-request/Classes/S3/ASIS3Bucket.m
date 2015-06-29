//
//  ASIS3Bucket.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 16/03/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "ASIS3Bucket.h"


@implementation ASIS3Bucket

+ (id)bucketWithOwnerID:(NSString *)anOwnerID ownerName:(NSString *)anOwnerName
{
	ASIS3Bucket *bucket = [[[self alloc] init] autorelease];
	[bucket setOwnerID:anOwnerID];
	[bucket setOwnerName:anOwnerName];
	return bucket;
}

- (void)dealloc
{
	[name release];
	[creationDate release];
	[ownerID release];
	[ownerName release];
	[super dealloc];
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"Name: %@ creationDate: %@ ownerID: %@ ownerName: %@",[self name],[self creationDate],[self ownerID],[self ownerName]];
}

@synthesize name;
@synthesize creationDate;
@synthesize ownerID;
@synthesize ownerName;
@end

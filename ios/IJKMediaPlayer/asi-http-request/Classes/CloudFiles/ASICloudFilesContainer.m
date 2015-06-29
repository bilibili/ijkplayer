//
//  ASICloudFilesContainer.m
//
//  Created by Michael Mayo on 1/7/10.
//

#import "ASICloudFilesContainer.h"


@implementation ASICloudFilesContainer

// regular container attributes
@synthesize name, count, bytes;

// CDN container attributes
@synthesize cdnEnabled, ttl, cdnURL, logRetention, referrerACL, useragentACL;

+ (id)container {
	ASICloudFilesContainer *container = [[[self alloc] init] autorelease];
	return container;
}

-(void) dealloc {
	[name release];
	[cdnURL release];
	[referrerACL release];
	[useragentACL release];
	[super dealloc];
}

@end

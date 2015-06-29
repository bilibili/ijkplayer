//
//  ASITestCase.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 26/07/2009.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//

#import "ASITestCase.h"


@implementation ASITestCase

- (NSString *)filePathForTemporaryTestFiles
{
	NSString *path = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0] stringByAppendingPathComponent:@"ASIHTTPRequest Test Files"];
	if (![[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:NULL]) {
		[[NSFileManager defaultManager] createDirectoryAtPath:path withIntermediateDirectories:NO attributes:nil error:NULL];
	}
	return path;
}

@end

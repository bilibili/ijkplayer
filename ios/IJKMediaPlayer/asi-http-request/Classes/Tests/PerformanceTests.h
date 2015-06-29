//
//  PerformanceTests.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 17/12/2009.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ASITestCase.h"

@interface PerformanceTests : ASITestCase {
	NSURL *testURL;

	NSDate *testStartDate;
	int requestsComplete;
	NSMutableArray *responseData;
	unsigned long bytesDownloaded;
}

- (void)testASIHTTPRequestAsyncPerformance;
- (void)testNSURLConnectionAsyncPerformance;

@property (retain,nonatomic) NSURL *testURL;
@property (retain,nonatomic) NSDate *testStartDate;
@property (assign,nonatomic) int requestsComplete;
@property (retain,nonatomic) NSMutableArray *responseData;
@end

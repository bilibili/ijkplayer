//
//  ASIDownloadCacheTests.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 03/05/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "ASITestCase.h"


@interface ASIDownloadCacheTests : ASITestCase {
	NSUInteger requestsFinishedCount;
	BOOL requestRedirectedWasCalled;
}

@end

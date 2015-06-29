//
//  ASIFormDataRequestTests.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 08/11/2008.
//  Copyright 2008 All-Seeing Interactive. All rights reserved.
//

#import "ASITestCase.h"

@interface ASIFormDataRequestTests : ASITestCase {
	float progress;
}

- (void)testDefaultMethod;
- (void)testPostWithFileUpload;
- (void)testEmptyData;
- (void)testSubclass;
- (void)testURLEncodedPost;
- (void)testCharset;
- (void)testPUT;
- (void)testCopy;

@end

//
//  ASIHTTPRequestTests.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 01/08/2008.
//  Copyright 2008 All-Seeing Interactive. All rights reserved.
//

#import "ASITestCase.h"

@class ASIHTTPRequest;

@interface ASIHTTPRequestTests : ASITestCase {
	float progress;
	BOOL started;
	BOOL finished;
	BOOL failed;
	BOOL receivedResponseHeaders;
	NSMutableData *responseData;
}

- (void)testBasicDownload;
- (void)testBase64Encode;
- (void)testDelegateMethods;
- (void)testConditionalGET;
- (void)testException;
- (void)testTimeOut;
- (void)testRequestMethod;
- (void)testHTTPVersion;
- (void)testUserAgent;
- (void)testAutomaticRedirection;
- (void)test30xCrash;
- (void)testUploadContentLength;
- (void)testDownloadContentLength;
- (void)testFileDownload;
- (void)testDownloadProgress;
- (void)testUploadProgress;
- (void)testCookies;
- (void)testRemoveCredentialsFromKeychain;
- (void)testBasicAuthentication;
- (void)testDigestAuthentication;
- (void)testNTLMHandshake;
- (void)testCharacterEncoding;
- (void)testCompressedResponse;
- (void)testCompressedResponseDownloadToFile;
- (void)test000SSL;
- (void)testRedirectPreservesSession;
- (void)testTooMuchRedirection;
- (void)testRedirectToNewDomain;
- (void)test303Redirect;
- (void)testSubclass;
- (void)testTimeOutWithoutDownloadDelegate;
- (void)testThrottlingDownloadBandwidth;
- (void)testThrottlingUploadBandwidth;
#if TARGET_OS_IPHONE
- (void)testReachability;
#endif
- (void)testAutomaticRetry;
- (void)testCloseConnection;
- (void)testPersistentConnections;
- (void)testNilPortCredentialsMatching;

@property (retain, nonatomic) NSMutableData *responseData;
@end

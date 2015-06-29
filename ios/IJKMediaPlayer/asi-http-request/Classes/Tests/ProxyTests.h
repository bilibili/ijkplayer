//
//  ProxyTests.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 02/08/2009.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ASITestCase.h"
@class ASINetworkQueue;

// Proxy tests must be run separately from other tests, using the proxy you specify
// Some tests require an authenticating proxy to function

@interface ProxyTests : ASITestCase {
	ASINetworkQueue *queue;
	BOOL complete;
}
- (void)testProxy;
- (void)testProxyAutodetect;
- (void)testProxyWithSuppliedAuthenticationCredentials;
- (void)testDoubleAuthentication;
- (void)testProxyForHTTPS;

@property (retain) ASINetworkQueue *queue;
@property (assign) BOOL complete;

@end

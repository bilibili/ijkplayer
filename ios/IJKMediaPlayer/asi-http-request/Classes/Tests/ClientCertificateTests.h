//
//  ClientCertificateTests.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 18/08/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

// Currently, these tests only work on iOS - it looks like the method for parsing the PKCS12 file would need to be ported

#import <Foundation/Foundation.h>
#import <Security/Security.h>
#import "ASITestCase.h"

@interface ClientCertificateTests : ASITestCase {

}
- (void)testClientCertificate;
+ (BOOL)extractIdentity:(SecIdentityRef *)outIdentity andTrust:(SecTrustRef*)outTrust fromPKCS12Data:(NSData *)inPKCS12Data;

@end

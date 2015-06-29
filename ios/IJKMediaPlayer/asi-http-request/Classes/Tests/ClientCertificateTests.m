//
//  ClientCertificateTests.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 18/08/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "ClientCertificateTests.h"
#import "ASIHTTPRequest.h"

@implementation ClientCertificateTests

- (void)testClientCertificate
{
	// This test will fail the second time it is run, I presume the certificate is being cached somewhere
	
	// This url requires we present a client certificate to connect to it
	NSURL *url = [NSURL URLWithString:@"https://clientcertificate.allseeing-i.com:8080/ASIHTTPRequest/tests/first"];
	
	// First, let's attempt to connect to the url without supplying a certificate
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:url];

	// We have to turn off validation for these tests, as the server has a self-signed certificate
	[request setValidatesSecureCertificate:NO];
	[request startSynchronous];
		
	GHAssertNotNil([request error],@"Request succeeded even though we presented no certificate, cannot proceed with test");
	
	// Now, let's grab the certificate (included in the resources of the test app)
	SecIdentityRef identity = NULL;
	SecTrustRef trust = NULL;
	NSData *PKCS12Data = [NSData dataWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"client" ofType:@"p12"]];
	[ClientCertificateTests extractIdentity:&identity andTrust:&trust fromPKCS12Data:PKCS12Data];
	
	request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"https://clientcertificate.allseeing-i.com:8080/ASIHTTPRequest/tests/first"]];
	
	// In this case, we have no need to add extra certificates, just the one inside the indentity will be used
	[request setClientCertificateIdentity:identity];
	[request setValidatesSecureCertificate:NO];
	[request startSynchronous];
	
	// Make sure the request got the correct content
	GHAssertNil([request error],@"Request failed with error %@",[request error]);
	BOOL success = [[request responseString] isEqualToString:@"This is the expected content for the first string"];
	GHAssertTrue(success,@"Request failed to download the correct content");
}

// Based on code from http://developer.apple.com/mac/library/documentation/Security/Conceptual/CertKeyTrustProgGuide/iPhone_Tasks/iPhone_Tasks.html

+ (BOOL)extractIdentity:(SecIdentityRef *)outIdentity andTrust:(SecTrustRef*)outTrust fromPKCS12Data:(NSData *)inPKCS12Data
{
	OSStatus securityError = errSecSuccess;
	
	NSDictionary *optionsDictionary = [NSDictionary dictionaryWithObject:@"" forKey:(id)kSecImportExportPassphrase];
	
	CFArrayRef items = CFArrayCreate(NULL, 0, 0, NULL);
	securityError = SecPKCS12Import((CFDataRef)inPKCS12Data,(CFDictionaryRef)optionsDictionary,&items);
	
	if (securityError == 0) { 
		CFDictionaryRef myIdentityAndTrust = CFArrayGetValueAtIndex (items, 0);
		const void *tempIdentity = NULL;
		tempIdentity = CFDictionaryGetValue (myIdentityAndTrust, kSecImportItemIdentity);
		*outIdentity = (SecIdentityRef)tempIdentity;
		const void *tempTrust = NULL;
		tempTrust = CFDictionaryGetValue (myIdentityAndTrust, kSecImportItemTrust);
		*outTrust = (SecTrustRef)tempTrust;
	} else {
		NSLog(@"Failed with error code %d",(int)securityError);
		return NO;
	}
	return YES;
}


@end

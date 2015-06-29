//
//  ASIWebPageRequestTests.m
//  Mac
//
//  Created by Ben Copsey on 06/01/2011.
//  Copyright 2011 All-Seeing Interactive. All rights reserved.
//

#import "ASIWebPageRequestTests.h"
#import "ASIWebPageRequest.h"

@implementation ASIWebPageRequestTests

- (void)testEncoding
{
	NSArray *encodings = [NSArray arrayWithObjects:@"us-ascii",@"iso-8859-1",@"utf-16",@"utf-8",nil];
	NSArray *expectedResponses = [NSArray arrayWithObjects:@"Hi there",@"Olá",@"你好",@"今日は",nil];
	NSUInteger i;
	for (i=0; i<[encodings count]; i++) {
		ASIWebPageRequest *request = [ASIWebPageRequest requestWithURL:[NSURL URLWithString:[NSString stringWithFormat:@"http://allseeing-i.com/ASIHTTPRequest/tests/asiwebpagerequest/character-encoding/%@",[encodings objectAtIndex:i]]]];
		[request setUserInfo:[NSDictionary dictionaryWithObject:[expectedResponses objectAtIndex:i] forKey:@"expected-response"]];
		[request setDelegate:self];
		[request setUrlReplacementMode:ASIReplaceExternalResourcesWithLocalURLs];
		[request startAsynchronous];
	}
}

- (void)requestFinished:(ASIHTTPRequest *)request
{
	if ([[request userInfo] objectForKey:@"expected-response"]) {
		BOOL success = ([[request responseString] rangeOfString:[[request userInfo] objectForKey:@"expected-response"]].location != NSNotFound);
		GHAssertTrue(success,@"Response HTML used wrong encoding");
	}
}

- (void)requestFailed:(ASIHTTPRequest *)request
{
	GHAssertNil([request error],@"Request failed, cannot proceed with test");		
}

@end

//
//  ASIFormDataRequestTests.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 08/11/2008.
//  Copyright 2008 All-Seeing Interactive. All rights reserved.
//

#import "ASIFormDataRequestTests.h"
#import "ASIFormDataRequest.h"

// Used for subclass test
@interface ASIFormDataRequestSubclass : ASIFormDataRequest {}
@end
@implementation ASIFormDataRequestSubclass;
@end

@implementation ASIFormDataRequestTests


-(void)testDefaultMethod
{
    ASIFormDataRequest *request = [[[ASIFormDataRequest alloc] initWithURL:[NSURL URLWithString:@"http://wedontcare.com"]] autorelease];
    
    GHAssertTrue([[request requestMethod] isEqualToString:@"POST"], @"Default request method should be POST");
}

- (void)testAddNilKeysAndValues
{
	ASIFormDataRequest *request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/empty-post-value"]];
	[request setPostValue:nil forKey:@"key1"];
	[request setPostValue:@"value2" forKey:@"key2"];
	[request setData:nil forKey:@"file1"];
	[request setData:[@"hello" dataUsingEncoding:NSUTF8StringEncoding] forKey:@"file2"];
	[request startSynchronous];
	BOOL success = ([[request responseString] isEqualToString:@"key1: \r\nkey2: value2\r\nfile1: \r\nfile2: hello"]);
	GHAssertTrue(success, @"Sent wrong data");

	// Test nil key (no key or value should be sent to the server)
	request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	[request addPostValue:@"value1" forKey:nil];
	[request addPostValue:@"value2" forKey:@"key2"];
	[request buildPostBody];
	NSString *postBody = [[[NSString alloc] initWithData:[request postBody] encoding:NSUTF8StringEncoding] autorelease];
	success = ([postBody isEqualToString:@"key2=value2"]);
	GHAssertTrue(success, @"Sent wrong data");
}

- (void)testPostWithFileUpload
{
	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/post"];
	
	//Create a 32kb file
	unsigned int size = 1024*32;
	NSMutableData *data = [NSMutableData dataWithLength:size];
	NSString *path = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"bigfile"];
	[data writeToFile:path atomically:NO];
	
	ASIFormDataRequest *request = [ASIFormDataRequest requestWithURL:url];
	
	NSDate *d = [NSDate date];
#if TARGET_OS_IPHONE
	NSValue *v = [NSValue valueWithCGRect:CGRectMake(0, 0, 200, 200)];
#else
	NSValue *v = [NSValue valueWithRect:NSMakeRect(0, 0, 200, 200)];	
#endif
	[request setPostValue:@"foo" forKey:@"post_var"];
	[request setPostValue:d forKey:@"post_var2"];
	[request setPostValue:v forKey:@"post_var3"];
	[request setFile:path forKey:@"file"];
	[request startSynchronous];

	BOOL success = ([[request responseString] isEqualToString:[NSString stringWithFormat:@"post_var: %@\r\npost_var2: %@\r\npost_var3: %@\r\nfile_name: %@\r\nfile_size: %hu\r\ncontent_type: %@",@"foo",d,v,@"bigfile",size,@"application/octet-stream"]]);
	GHAssertTrue(success,@"Failed to upload the correct data (using local file)");	
	
	//Try the same with the raw data
	request = [[[ASIFormDataRequest alloc] initWithURL:url] autorelease];
	[request setPostValue:@"foo" forKey:@"post_var"];
	[request setPostValue:d forKey:@"post_var2"];
	[request setPostValue:v forKey:@"post_var3"];
	[request setData:data forKey:@"file"];
	[request startSynchronous];

	success = ([[request responseString] isEqualToString:[NSString stringWithFormat:@"post_var: %@\r\npost_var2: %@\r\npost_var3: %@\r\nfile_name: %@\r\nfile_size: %hu\r\ncontent_type: %@",@"foo",d,v,@"file",size,@"application/octet-stream"]]);
	GHAssertTrue(success,@"Failed to upload the correct data (using NSData)");	

	//Post with custom content-type and file name
	request = [[[ASIFormDataRequest alloc] initWithURL:url] autorelease];
	[request setPostValue:@"foo" forKey:@"post_var"];
	[request setPostValue:d forKey:@"post_var2"];
	[request setPostValue:v forKey:@"post_var3"];	
	[request setFile:path withFileName:@"myfile" andContentType:@"text/plain" forKey:@"file"];
	[request startSynchronous];
	
	success = ([[request responseString] isEqualToString:[NSString stringWithFormat:@"post_var: %@\r\npost_var2: %@\r\npost_var3: %@\r\nfile_name: %@\r\nfile_size: %hu\r\ncontent_type: %@",@"foo",d,v,@"myfile",size,@"text/plain"]]);
	GHAssertTrue(success,@"Failed to send the correct content-type / file name");	
	
	//Post raw data with custom content-type and file name
	request = [[[ASIFormDataRequest alloc] initWithURL:url] autorelease];
	[request setPostValue:@"foo" forKey:@"post_var"];
	[request setPostValue:d forKey:@"post_var2"];
	[request setPostValue:v forKey:@"post_var3"];	
	[request setData:data withFileName:@"myfile" andContentType:@"text/plain" forKey:@"file"];
	[request startSynchronous];
	
	success = ([[request responseString] isEqualToString:[NSString stringWithFormat:@"post_var: %@\r\npost_var2: %@\r\npost_var3: %@\r\nfile_name: %@\r\nfile_size: %hu\r\ncontent_type: %@",@"foo",d,v,@"myfile",size,@"text/plain"]]);
	GHAssertTrue(success,@"Failed to send the correct content-type / file name");	
	
}

// Test fix for bug where setting an empty string for a form post value would cause the rest of the post body to be ignored (because an NSOutputStream won't like it if you try to write 0 bytes)
- (void)testEmptyData
{
	ASIFormDataRequest *request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/post-empty"]];
	[request setPostValue:@"hello" forKey:@"a_non_empty_string"];
	[request setPostValue:@"" forKey:@"zzz_empty_string"];
	[request setPostValue:@"there" forKey:@"xxx_non_empty_string"];
	[request setShouldStreamPostDataFromDisk:YES];
	[request buildPostBody];
	[request startSynchronous];
	
	BOOL success = ([[request responseString] isEqualToString:@"a_non_empty_string: hello\r\nzzz_empty_string: \r\nxxx_non_empty_string: there"]);
	GHAssertTrue(success,@"Failed to send the correct post data");		
	
}

// Ensure class convenience constructor returns an instance of our subclass
- (void)testSubclass
{
	ASIFormDataRequestSubclass *instance = [ASIFormDataRequestSubclass requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	BOOL success = [instance isKindOfClass:[ASIFormDataRequestSubclass class]];
	GHAssertTrue(success,@"Convenience constructor failed to return an instance of the correct class");	
}

- (void)testURLEncodedPost
{
	ASIFormDataRequest *request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/url-encoded-post"]];
	[request setPostValue:@"value1" forKey:@"value1"];
	[request setPostValue:@"(%20 ? =)" forKey:@"value2"];
	[request setPostValue:@"£100.00" forKey:@"value3"];
	[request setPostValue:@"" forKey:@"value4"];
	[request setPostValue:@"&??aaa=//ciaoèèè" forKey:@"teskey&aa"]; 
	
	[request setShouldStreamPostDataFromDisk:YES];
	[request setPostFormat:ASIURLEncodedPostFormat];
	[request startSynchronous];

	
	BOOL success = ([[request responseString] isEqualToString:@"value1: value1\r\nvalue2: (%20 ? =)\r\nvalue3: £100.00\r\nvalue4: \r\nteskey&aa: &??aaa=//ciaoèèè"]);
	GHAssertTrue(success,@"Failed to send the correct post data");			
}

- (void)testCharset
{
	NSURL *url = [NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/formdata-charset"];
	NSString *testString = @"£££s don't seem to buy me many €€€s these days";
	
	// Test the default (UTF-8) with a url-encoded request
	NSString *charset = @"utf-8";
	ASIFormDataRequest *request = [ASIFormDataRequest requestWithURL:url];
	[request setPostValue:testString forKey:@"value"];
	[request startSynchronous];
	BOOL success = ([[request responseString] isEqualToString:[NSString stringWithFormat:@"Got data in %@: %@",charset,testString]]);
	GHAssertTrue(success,@"Failed to correctly encode the data");	
	
	// Test the default (UTF-8) with a multipart/form-data request
	request = [ASIFormDataRequest requestWithURL:url];
	[request setPostValue:testString forKey:@"value"];
	[request setPostFormat:ASIMultipartFormDataPostFormat];
	[request startSynchronous];
	success = ([[request responseString] isEqualToString:[NSString stringWithFormat:@"Got data in %@: %@",charset,testString]]);
	GHAssertTrue(success,@"Failed to correctly encode the data");
	
	// Test a different charset
	testString = @"£££s don't seem to buy me many $$$s these days";
	charset = @"iso-8859-1";
	request = [ASIFormDataRequest requestWithURL:url];
	[request setPostValue:testString forKey:@"value"];
	[request setStringEncoding:NSISOLatin1StringEncoding];
	[request startSynchronous];
	success = ([[request responseString] isEqualToString:[NSString stringWithFormat:@"Got data in %@: %@",charset,testString]]);
	GHAssertTrue(success,@"Failed to correctly encode the data");	
	
	// And again with multipart/form-data request
	request = [ASIFormDataRequest requestWithURL:url];
	[request setPostValue:testString forKey:@"value"];
	[request setPostFormat:ASIMultipartFormDataPostFormat];
	[request setStringEncoding:NSISOLatin1StringEncoding];
	[request startSynchronous];
	success = ([[request responseString] isEqualToString:[NSString stringWithFormat:@"Got data in %@: %@",charset,testString]]);
	GHAssertTrue(success,@"Failed to correctly encode the data");	
	
	// Once more for luck
	charset = @"windows-1252";
	request = [ASIFormDataRequest requestWithURL:url];
	[request setPostValue:testString forKey:@"value"];
	[request setStringEncoding:NSWindowsCP1252StringEncoding];
	[request startSynchronous];
	success = ([[request responseString] isEqualToString:[NSString stringWithFormat:@"Got data in %@: %@",charset,testString]]);
	GHAssertTrue(success,@"Failed to correctly encode the data");

	request = [ASIFormDataRequest requestWithURL:url];
	[request setPostValue:testString forKey:@"value"];
	[request setPostFormat:ASIMultipartFormDataPostFormat];
	[request setStringEncoding:NSWindowsCP1252StringEncoding];
	[request startSynchronous];
	success = ([[request responseString] isEqualToString:[NSString stringWithFormat:@"Got data in %@: %@",charset,testString]]);
	GHAssertTrue(success,@"Failed to correctly encode the data");
	
	// Ensure charset isn't added to file post (GH issue 36)
	request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/Tests/return-raw-request"]];
	[request setData:[@"test 123" dataUsingEncoding:NSUTF8StringEncoding] forKey:@"file"];
	[request setRequestMethod:@"PUT"];
	[request startSynchronous];	
	success = ([[request responseString] rangeOfString:@"charset=utf-8"].location == NSNotFound);
	GHAssertTrue(success,@"Sent a charset header for an uploaded file");


}

- (void)testPUT
{
	ASIFormDataRequest *request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/Tests/put_form_data"]];
	[request setRequestMethod:@"PUT"];
	[request setPostValue:@"cheep cheep" forKey:@"hello"];
	[request startSynchronous];
	
	NSString *expectedResponse = [[[NSString alloc] initWithBytes:[[request postBody] bytes] length:[[request postBody] length] encoding:[request stringEncoding]] autorelease];
	BOOL success = ([[request responseString] isEqualToString:expectedResponse]);
	GHAssertTrue(success,@"Failed to send form data using PUT");
	
	// Ensure that other methods still default to POST
	request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/Tests/put_form_data"]];
	[request setRequestMethod:@"DELETE"];
	[request setPostValue:@"cheep cheep" forKey:@"hello"];
	[request startSynchronous];
	
	success = ([[request responseString] isEqualToString:@"Got POST instead"]);
	GHAssertTrue(success,@"Failed to send form data using PUT");		
}

- (void)testCopy
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	ASIFormDataRequest *request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com"]];
	ASIFormDataRequest *request2 = [request copy];
	GHAssertNotNil(request2,@"Failed to create a copy");
	
	[pool release];
	BOOL success = ([request2 retainCount] == 1);
	GHAssertTrue(success,@"Failed to create a retained copy");
	success = ([request2 isKindOfClass:[ASIFormDataRequest class]]);
	GHAssertTrue(success,@"Copy is of wrong class");

	[request2 release];
}

- (void)testMultipleValuesForASingleKey
{
	ASIFormDataRequest *request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/multiple-values"]];
	[request addPostValue:@"here" forKey:@"test_value[]"];
	[request addPostValue:@"are" forKey:@"test_value[]"];
	[request addPostValue:@"some" forKey:@"test_value[]"];
	[request addPostValue:@"values" forKey:@"test_value[]"];

	NSString *path1 = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"file1.txt"];
	NSString *path2 = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"file2.txt"];
	[@"hello" writeToFile:path1 atomically:NO encoding:NSUTF8StringEncoding error:nil];
	[@"there" writeToFile:path2 atomically:NO encoding:NSUTF8StringEncoding error:nil];
	[request addFile:path1 forKey:@"test_file[]"];
	[request addFile:path2 forKey:@"test_file[]"];

	[request startSynchronous];
	NSString *expectedOutput = @"here\r\nare\r\nsome\r\nvalues\r\nfile1.txt\r\nfile2.txt\r\n";
	BOOL success = [[request responseString] isEqualToString:expectedOutput];
	GHAssertTrue(success,@"Failed to send the correct data");

	// Check data replaces older data
	request = [ASIFormDataRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/single-values"]];
	[request addPostValue:@"here" forKey:@"test_value[]"];
	[request addPostValue:@"are" forKey:@"test_value[]"];
	[request addPostValue:@"some" forKey:@"test_value[]"];
	[request addPostValue:@"values" forKey:@"test_value[]"];

	[request setPostValue:@"this is new data" forKey:@"test_value[]"];

	[request addFile:path1 forKey:@"test_file[]"];
	[request addFile:path2 forKey:@"test_file[]"];

	[request setData:[@"this is new data" dataUsingEncoding:NSUTF8StringEncoding] forKey:@"test_file[]"];

	[request startSynchronous];
	expectedOutput = @"this is new data\r\nfile\r\n";
	success = [[request responseString] isEqualToString:expectedOutput];
	GHAssertTrue(success,@"Failed to send the correct data");
}

@end

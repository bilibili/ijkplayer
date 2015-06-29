//
//  ASICloudFilesRequestTests.m
//
//  Created by Michael Mayo on 1/6/10.
//

#import "ASICloudFilesRequestTests.h"

// models
#import "ASICloudFilesContainer.h"
#import "ASICloudFilesObject.h"

// requests
#import "ASICloudFilesRequest.h"
#import "ASICloudFilesContainerRequest.h"
#import "ASICloudFilesObjectRequest.h"
#import "ASICloudFilesCDNRequest.h"

// Fill in these to run the tests that actually connect and manipulate objects on Cloud Files
static NSString *username = @"";
static NSString *apiKey = @"";

@implementation ASICloudFilesRequestTests

@synthesize networkQueue;

// Authenticate before any test if there's no auth token present
- (void)authenticate {
	if (![ASICloudFilesRequest authToken]) {
		[ASICloudFilesRequest setUsername:username];
		[ASICloudFilesRequest setApiKey:apiKey];
		[ASICloudFilesRequest authenticate];		
	}
}

// ASICloudFilesRequest
- (void)testAuthentication {
	[self authenticate];
	GHAssertNotNil([ASICloudFilesRequest authToken], @"Failed to authenticate and obtain authentication token");
	GHAssertNotNil([ASICloudFilesRequest storageURL], @"Failed to authenticate and obtain storage URL");
	GHAssertNotNil([ASICloudFilesRequest cdnManagementURL], @"Failed to authenticate and obtain CDN URL");
}

- (void)testDateParser {
	ASICloudFilesRequest *request = [[[ASICloudFilesRequest alloc] init] autorelease];
	
	NSDate *date = [request dateFromString:@"invalid date string"];
	GHAssertNil(date, @"Should have failed to parse an invalid date string");
	
	date = [request dateFromString:@"2009-11-04T19:46:20.192723"];
	GHAssertNotNil(date, @"Failed to parse date string");		
	
	NSDateComponents *components = [[[NSDateComponents alloc] init] autorelease];
	[components setYear:2009];
	[components setMonth:11];
	[components setDay:4];
	[components setHour:19];
	[components setMinute:46];
	[components setSecond:20];
	NSCalendar *calendar = [[[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar] autorelease];
	NSDate *referenceDate = [calendar dateFromComponents:components];
	
	// NSDateComponents has seconds as the smallest value, so we'll just check the created date is less than 1 second different from what we expect
	NSTimeInterval timeDifference = [date timeIntervalSinceDate:referenceDate];
	BOOL success = (timeDifference < 1.0);
	GHAssertTrue(success, @"Parsed date incorrectly");	
}

// ASICloudFilesContainerRequest
- (void)testAccountInfo {
	[self authenticate];
	
	ASICloudFilesContainerRequest *request = [ASICloudFilesContainerRequest accountInfoRequest];
	[request startSynchronous];
	
	GHAssertTrue([request containerCount] > 0, @"Failed to retrieve account info");
	GHAssertTrue([request bytesUsed] > 0, @"Failed to retrieve account info");
}

- (void)testContainerList {
	[self authenticate];
	
	NSArray *containers = nil;
	
	ASICloudFilesContainerRequest *containerListRequest = [ASICloudFilesContainerRequest listRequest];
	[containerListRequest startSynchronous];
	
	containers = [containerListRequest containers];
	GHAssertTrue([containers count] > 0, @"Failed to list containers");
	NSUInteger i;
	for (i = 0; i < [containers count]; i++) {
		ASICloudFilesContainer *container = [containers objectAtIndex:i];
		GHAssertNotNil(container.name, @"Failed to parse container");
	}
	
	ASICloudFilesContainerRequest *limitContainerListRequest = [ASICloudFilesContainerRequest listRequestWithLimit:2 marker:nil];
	[limitContainerListRequest startSynchronous];	
	containers = [limitContainerListRequest containers];
	GHAssertTrue([containers count] == 2, @"Failed to limit container list");
}

- (void)testContainerCreate {
	[self authenticate];
	
	ASICloudFilesContainerRequest *createContainerRequest = [ASICloudFilesContainerRequest createContainerRequest:@"ASICloudFilesContainerTest"];
	[createContainerRequest startSynchronous];
	GHAssertTrue([createContainerRequest error] == nil, @"Failed to create container");
}

- (void)testContainerDelete {
	[self authenticate];

	ASICloudFilesContainerRequest *deleteContainerRequest = [ASICloudFilesContainerRequest deleteContainerRequest:@"ASICloudFilesContainerTest"];
	[deleteContainerRequest startSynchronous];
	GHAssertTrue([deleteContainerRequest error] == nil, @"Failed to delete container");	
}

// ASICloudFilesObjectRequest
- (void)testContainerInfo {
	[self authenticate];

	// create a file first
	ASICloudFilesContainerRequest *createContainerRequest = [ASICloudFilesContainerRequest createContainerRequest:@"ASICloudFilesTest"];
	[createContainerRequest startSynchronous];
	NSData *data = [@"this is a test" dataUsingEncoding:NSUTF8StringEncoding];
	ASICloudFilesObjectRequest *putRequest 
		= [ASICloudFilesObjectRequest putObjectRequestWithContainer:@"ASICloudFilesTest" 
													 objectPath:@"infotestfile.txt" contentType:@"text/plain" 
													 objectData:data metadata:nil etag:nil];
	
	[putRequest startSynchronous];
	
	ASICloudFilesObjectRequest *request = [ASICloudFilesObjectRequest containerInfoRequest:@"ASICloudFilesTest"];
	[request startSynchronous];	
	GHAssertTrue([request containerObjectCount] > 0, @"Failed to retrieve container info");
	GHAssertTrue([request containerBytesUsed] > 0, @"Failed to retrieve container info");
}

- (void)testObjectInfo {
	[self authenticate];
	
	ASICloudFilesObjectRequest *request = [ASICloudFilesObjectRequest objectInfoRequest:@"ASICloudFilesTest" objectPath:@"infotestfile.txt"];
	[request startSynchronous];
	
	ASICloudFilesObject *object = [request object];
	GHAssertNotNil(object, @"Failed to retrieve object");
	GHAssertTrue([object.metadata count] > 0, @"Failed to parse metadata");
	
	GHAssertTrue([object.metadata objectForKey:@"Test"] != nil, @"Failed to parse metadata");
	
}

- (void)testObjectList {
	[self authenticate];
	
	ASICloudFilesObjectRequest *objectListRequest = [ASICloudFilesObjectRequest listRequestWithContainer:@"ASICloudFilesTest"];
	[objectListRequest startSynchronous];
	
	NSArray *containers = [objectListRequest objects];
	GHAssertTrue([containers count] > 0, @"Failed to list objects");
	NSUInteger i;
	for (i = 0; i < [containers count]; i++) {
		ASICloudFilesObject *object = [containers objectAtIndex:i];
		GHAssertNotNil(object.name, @"Failed to parse object");
	}
	
}

- (void)testGetObject {
	[self authenticate];
	
	ASICloudFilesObjectRequest *request = [ASICloudFilesObjectRequest getObjectRequestWithContainer:@"ASICloudFilesTest" objectPath:@"infotestfile.txt"];
	[request startSynchronous];
	
	ASICloudFilesObject *object = [request object];
	GHAssertNotNil(object, @"Failed to retrieve object");
	
	GHAssertNotNil(object.name, @"Failed to parse object name");
	GHAssertTrue(object.bytes > 0, @"Failed to parse object bytes");
	GHAssertNotNil(object.contentType, @"Failed to parse object content type");
	GHAssertNotNil(object.lastModified, @"Failed to parse object last modified");
	GHAssertNotNil(object.data, @"Failed to parse object data");
}

- (void)testPutObject {
	[self authenticate];
	
	ASICloudFilesContainerRequest *createContainerRequest 
			= [ASICloudFilesContainerRequest createContainerRequest:@"ASICloudFilesTest"];
	[createContainerRequest startSynchronous];

	NSData *data = [@"this is a test" dataUsingEncoding:NSUTF8StringEncoding];
	
	ASICloudFilesObjectRequest *putRequest 
			= [ASICloudFilesObjectRequest putObjectRequestWithContainer:@"ASICloudFilesTest" 
											objectPath:@"puttestfile.txt" contentType:@"text/plain" 
											objectData:data metadata:nil etag:nil];
	
	[putRequest startSynchronous];
	
	GHAssertNil([putRequest error], @"Failed to PUT object");

	ASICloudFilesObjectRequest *getRequest = [ASICloudFilesObjectRequest getObjectRequestWithContainer:@"ASICloudFilesTest" objectPath:@"puttestfile.txt"];
	[getRequest startSynchronous];
	
	ASICloudFilesObject *object = [getRequest object];
	NSString *string = [[NSString alloc] initWithData:object.data encoding:NSASCIIStringEncoding];

	GHAssertNotNil(object, @"Failed to retrieve new object");
	GHAssertNotNil(object.name, @"Failed to parse object name");
	GHAssertEqualStrings(object.name, @"puttestfile.txt", @"Failed to parse object name", @"Failed to parse object name");
	GHAssertNotNil(object.data, @"Failed to parse object data");
	GHAssertEqualStrings(string, @"this is a test", @"Failed to parse object data", @"Failed to parse object data");

	
	ASICloudFilesContainerRequest *deleteContainerRequest = [ASICloudFilesContainerRequest deleteContainerRequest:@"ASICloudFilesTest"];
	[deleteContainerRequest startSynchronous];
	
	// Now put the object from a file

	createContainerRequest = [ASICloudFilesContainerRequest createContainerRequest:@"ASICloudFilesTest"];
	[createContainerRequest startSynchronous];
	
	NSString *filePath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"cloudfile"];
	[data writeToFile:filePath atomically:NO];
	
	putRequest = [ASICloudFilesObjectRequest putObjectRequestWithContainer:@"ASICloudFilesTest" objectPath:@"puttestfile.txt" contentType:@"text/plain" file:filePath metadata:nil etag:nil];
	
	[putRequest startSynchronous];
	
	GHAssertNil([putRequest error], @"Failed to PUT object");
	
	getRequest = [ASICloudFilesObjectRequest getObjectRequestWithContainer:@"ASICloudFilesTest" objectPath:@"puttestfile.txt"];
	[getRequest startSynchronous];
	
	object = [getRequest object];
	
	GHAssertNotNil(object, @"Failed to retrieve new object");
	GHAssertNotNil(object.name, @"Failed to parse object name");
	GHAssertEqualStrings(object.name, @"puttestfile.txt", @"Failed to parse object name", @"Failed to parse object name");
	GHAssertNotNil(object.data, @"Failed to parse object data");
	GHAssertEqualStrings(string, @"this is a test", @"Failed to parse object data", @"Failed to parse object data");
	
	[string release];
	
	deleteContainerRequest = [ASICloudFilesContainerRequest deleteContainerRequest:@"ASICloudFilesTest"];
	[deleteContainerRequest startSynchronous];
}

- (void)testPostObject {
	[self authenticate];
	
	NSMutableDictionary *metadata = [[NSMutableDictionary alloc] initWithCapacity:2];
	[metadata setObject:@"test" forKey:@"Test"];
	[metadata setObject:@"test" forKey:@"ASITest"];
	
	ASICloudFilesObject *object = [ASICloudFilesObject object];
	object.name = @"infotestfile.txt";
	object.metadata = metadata;
	
	ASICloudFilesObjectRequest *request = [ASICloudFilesObjectRequest postObjectRequestWithContainer:@"ASICloudFilesTest" object:object];
	[request startSynchronous];
	
	GHAssertTrue([request responseStatusCode] == 202, @"Failed to post object metadata");
	
	[metadata release];
	
}

- (void)testDeleteObject {
	[self authenticate];
	
	ASICloudFilesObjectRequest *deleteRequest = [ASICloudFilesObjectRequest deleteObjectRequestWithContainer:@"ASICloudFilesTest" objectPath:@"puttestfile.txt"];
	[deleteRequest startSynchronous];
	GHAssertTrue([deleteRequest responseStatusCode] == 204, @"Failed to delete object");
}

#pragma mark -
#pragma mark CDN Tests

- (void)testCDNContainerInfo {
	[self authenticate];
	
	ASICloudFilesCDNRequest *request = [ASICloudFilesCDNRequest containerInfoRequest:@"ASICloudFilesTest"];
	[request startSynchronous];
	
	GHAssertTrue([request responseStatusCode] == 204, @"Failed to retrieve CDN container info");
	GHAssertTrue([request cdnEnabled], @"Failed to retrieve CDN container info");
	GHAssertNotNil([request cdnURI], @"Failed to retrieve CDN container info");
	GHAssertTrue([request cdnTTL] > 0, @"Failed to retrieve CDN container info");	
}

- (void)testCDNContainerList {
	[self authenticate];
	
	ASICloudFilesCDNRequest *request = [ASICloudFilesCDNRequest listRequest];
	[request startSynchronous];
	
	GHAssertNotNil([request containers], @"Failed to retrieve CDN container list");
}

- (void)testCDNContainerListWithParams {
	[self authenticate];
	
	ASICloudFilesCDNRequest *request = [ASICloudFilesCDNRequest listRequestWithLimit:2 marker:nil enabledOnly:YES];
	[request startSynchronous];
	
	GHAssertNotNil([request containers], @"Failed to retrieve CDN container list");
	GHAssertTrue([[request containers] count] == 2, @"Failed to retrieve limited CDN container list");
}

- (void)testCDNPut {
	[self authenticate];
	
	ASICloudFilesCDNRequest *request = [ASICloudFilesCDNRequest putRequestWithContainer:@"ASICloudFilesTest"];
	[request startSynchronous];
	
	GHAssertNotNil([request cdnURI], @"Failed to PUT to CDN container");
}

- (void)testCDNPost {
	[self authenticate];
	
	ASICloudFilesCDNRequest *request = [ASICloudFilesCDNRequest postRequestWithContainer:@"ASICloudFilesTest" cdnEnabled:YES ttl:86600];
	[request startSynchronous];
	
	GHAssertNotNil([request cdnURI], @"Failed to POST to CDN container");
}

#pragma mark -
#pragma mark Memory Management

-(void)dealloc {
	[networkQueue release];
	[super dealloc];
}

@end

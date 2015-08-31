//
//  ASIS3BucketRequest.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 16/03/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//
//  Use this class to create buckets, fetch a list of their contents, and delete buckets

#import <Foundation/Foundation.h>
#import "ASIS3Request.h"

@class ASIS3BucketObject;

@interface ASIS3BucketRequest : ASIS3Request {
	
	// Name of the bucket to talk to
	NSString *bucket;
	
	// A parameter passed to S3 in the query string to tell it to return specialised information
	// Consult the S3 REST API documentation for more info
	NSString *subResource;
	
	// Options for filtering GET requests
	// See http://docs.amazonwebservices.com/AmazonS3/2006-03-01/index.html?RESTBucketGET.html
	NSString *prefix;
	NSString *marker;
	int maxResultCount;
	NSString *delimiter;
	
	// Internally used while parsing the response
	ASIS3BucketObject *currentObject;
	
	// Returns an array of ASIS3BucketObjects created from the XML response
	NSMutableArray *objects;	
	
	// Will be populated with a list of 'folders' when a delimiter is set
	NSMutableArray *commonPrefixes;
	
	// Will be true if this request did not return all the results matching the query (use maxResultCount to configure the number of results to return)
	BOOL isTruncated;
}

// Fetch a bucket
+ (id)requestWithBucket:(NSString *)bucket;

// Create a bucket request, passing a parameter in the query string
// You'll need to parse the response XML yourself
// Examples:
// Fetch ACL:
// ASIS3BucketRequest *request = [ASIS3BucketRequest requestWithBucket:@"mybucket" parameter:@"acl"];
// Fetch Location:
// ASIS3BucketRequest *request = [ASIS3BucketRequest requestWithBucket:@"mybucket" parameter:@"location"];
// See the S3 REST API docs for more information about the parameters you can pass
+ (id)requestWithBucket:(NSString *)bucket subResource:(NSString *)subResource;

// Use for creating new buckets
+ (id)PUTRequestWithBucket:(NSString *)bucket;

// Use for deleting buckets - they must be empty for this to succeed
+ (id)DELETERequestWithBucket:(NSString *)bucket;

@property (retain, nonatomic) NSString *bucket;
@property (retain, nonatomic) NSString *subResource;
@property (retain, nonatomic) NSString *prefix;
@property (retain, nonatomic) NSString *marker;
@property (assign, nonatomic) int maxResultCount;
@property (retain, nonatomic) NSString *delimiter;
@property (retain, readonly) NSMutableArray *objects;
@property (retain, readonly) NSMutableArray *commonPrefixes;
@property (assign, readonly) BOOL isTruncated;
@end

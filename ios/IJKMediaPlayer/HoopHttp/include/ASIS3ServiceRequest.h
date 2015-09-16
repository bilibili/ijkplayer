//
//  ASIS3ServiceRequest.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 16/03/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//
//  Create an ASIS3ServiceRequest to obtain a list of your buckets

#import <Foundation/Foundation.h>
#import "ASIS3Request.h"

@class ASIS3Bucket;

@interface ASIS3ServiceRequest : ASIS3Request {
	
	// Internally used while parsing the response
	ASIS3Bucket *currentBucket;
	NSString *ownerName;
	NSString *ownerID;
	
	// A list of the buckets stored on S3 for this account
	NSMutableArray *buckets;
}

// Perform a GET request on the S3 service
// This will fetch a list of the buckets attached to the S3 account
+ (id)serviceRequest;

@property (retain, readonly) NSMutableArray *buckets;
@end

//
//  ASIS3Bucket.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 16/03/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//
//  Instances of this class represent buckets stored on S3
//  ASIS3ServiceRequests return an array of ASIS3Buckets when you perform a service GET query
//  You'll probably never need to create instances of ASIS3Bucket yourself

#import <Foundation/Foundation.h>


@interface ASIS3Bucket : NSObject {
	
	// The name of this bucket (will be unique throughout S3)
	NSString *name;
	
	// The date this bucket was created
	NSDate *creationDate;
	
	// Information about the owner of this bucket
	NSString *ownerID;
	NSString *ownerName;
}

+ (id)bucketWithOwnerID:(NSString *)ownerID ownerName:(NSString *)ownerName;

@property (retain) NSString *name;
@property (retain) NSDate *creationDate;
@property (retain) NSString *ownerID;
@property (retain) NSString *ownerName;
@end

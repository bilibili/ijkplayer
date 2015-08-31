//
//  ASIS3Request.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 30/06/2009.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//
// A class for accessing data stored on Amazon's Simple Storage Service (http://aws.amazon.com/s3/) using the REST API
// While you can use this class directly, the included subclasses make typical operations easier

#import <Foundation/Foundation.h>
#import "ASIHTTPRequest.h"

#if !TARGET_OS_IPHONE || (TARGET_OS_IPHONE && __IPHONE_OS_VERSION_MAX_ALLOWED < __IPHONE_4_0)
#import "ASINSXMLParserCompat.h"
#endif

// See http://docs.amazonwebservices.com/AmazonS3/2006-03-01/index.html?RESTAccessPolicy.html for what these mean
extern NSString *const ASIS3AccessPolicyPrivate; // This is the default in S3 when no access policy header is provided
extern NSString *const ASIS3AccessPolicyPublicRead;
extern NSString *const ASIS3AccessPolicyPublicReadWrite;
extern NSString *const ASIS3AccessPolicyAuthenticatedRead;
extern NSString *const ASIS3AccessPolicyBucketOwnerRead;
extern NSString *const ASIS3AccessPolicyBucketOwnerFullControl;

// Constants for requestScheme - defaults is ASIS3RequestSchemeHTTP
extern NSString *const ASIS3RequestSchemeHTTP;
extern NSString *const ASIS3RequestSchemeHTTPS;



typedef enum _ASIS3ErrorType {
    ASIS3ResponseParsingFailedType = 1,
    ASIS3ResponseErrorType = 2
} ASIS3ErrorType;



@interface ASIS3Request : ASIHTTPRequest <NSCopying, NSXMLParserDelegate> {
	
	// Your S3 access key. Set it on the request, or set it globally using [ASIS3Request setSharedAccessKey:]
	NSString *accessKey;
	
	// Your S3 secret access key. Set it on the request, or set it globally using [ASIS3Request setSharedSecretAccessKey:]
	NSString *secretAccessKey;
	
	// Set to ASIS3RequestSchemeHTTPS to send your requests via HTTPS (default is ASIS3RequestSchemeHTTP)
	NSString *requestScheme;

	// The string that will be used in the HTTP date header. Generally you'll want to ignore this and let the class add the current date for you, but the accessor is used by the tests
	NSString *dateString;

	// The access policy to use when PUTting a file (see the string constants at the top ASIS3Request.h for details on what the possible options are)
	NSString *accessPolicy;

	// Internally used while parsing errors
	NSString *currentXMLElementContent;
	NSMutableArray *currentXMLElementStack;
}

// Uses the supplied date to create a Date header string
- (void)setDate:(NSDate *)date;

// Will return a dictionary of the 'amz-' headers that wil be sent to S3
// Override in subclasses to add new ones	
- (NSMutableDictionary *)S3Headers;
	
// Returns the string that will used to create a signature for this request
// Is overridden in ASIS3ObjectRequest
- (NSString *)stringToSignForHeaders:(NSString *)canonicalizedAmzHeaders resource:(NSString *)canonicalizedResource;

// Parses the response to work out if S3 returned an error	
- (void)parseResponseXML;

#pragma mark shared access keys

// Get and set the global access key, this will be used for all requests the access key hasn't been set for
+ (NSString *)sharedAccessKey;
+ (void)setSharedAccessKey:(NSString *)newAccessKey;
+ (NSString *)sharedSecretAccessKey;
+ (void)setSharedSecretAccessKey:(NSString *)newAccessKey;

# pragma mark helpers
	
// Returns a date formatter than can be used to parse a date from S3
+ (NSDateFormatter*)S3ResponseDateFormatter;
	
// Returns a date formatter than can be used to send a date header to S3
+ (NSDateFormatter*)S3RequestDateFormatter;


// URL-encodes an S3 key so it can be used in a url
// You shouldn't normally need to use this yourself
+ (NSString *)stringByURLEncodingForS3Path:(NSString *)key;

// Returns a string for the hostname used for S3 requests. You shouldn't ever need to change this.
+ (NSString *)S3Host;

// This is called automatically before the request starts to build the request URL (if one has not been manually set already)
- (void)buildURL;

@property (retain) NSString *dateString;
@property (retain) NSString *accessKey;
@property (retain) NSString *secretAccessKey;
@property (retain) NSString *accessPolicy;
@property (retain) NSString *currentXMLElementContent;
@property (retain) NSMutableArray *currentXMLElementStack;
@property (retain) NSString *requestScheme;
@end

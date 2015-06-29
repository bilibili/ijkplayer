//
//  ASIS3Request.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 30/06/2009.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//

#import "ASIS3Request.h"
#import <CommonCrypto/CommonHMAC.h>

NSString *const ASIS3AccessPolicyPrivate = @"private";
NSString *const ASIS3AccessPolicyPublicRead = @"public-read";
NSString *const ASIS3AccessPolicyPublicReadWrite = @"public-read-write";
NSString *const ASIS3AccessPolicyAuthenticatedRead = @"authenticated-read";
NSString *const ASIS3AccessPolicyBucketOwnerRead = @"bucket-owner-read";
NSString *const ASIS3AccessPolicyBucketOwnerFullControl = @"bucket-owner-full-control";

NSString *const ASIS3RequestSchemeHTTP = @"http";
NSString *const ASIS3RequestSchemeHTTPS = @"https";

static NSString *sharedAccessKey = nil;
static NSString *sharedSecretAccessKey = nil;

// Private stuff
@interface ASIS3Request ()
	+ (NSData *)HMACSHA1withKey:(NSString *)key forString:(NSString *)string;
@end

@implementation ASIS3Request

- (id)initWithURL:(NSURL *)newURL
{
	self = [super initWithURL:newURL];
	// After a bit of experimentation/guesswork, this number seems to reduce the chance of a 'RequestTimeout' error
	[self setPersistentConnectionTimeoutSeconds:20];
	[self setRequestScheme:ASIS3RequestSchemeHTTP];
	return self;
}


- (void)dealloc
{
	[currentXMLElementContent release];
	[currentXMLElementStack release];
	[dateString release];
	[accessKey release];
	[secretAccessKey release];
	[accessPolicy release];
	[requestScheme release];
	[super dealloc];
}


- (void)setDate:(NSDate *)date
{
	[self setDateString:[[ASIS3Request S3RequestDateFormatter] stringFromDate:date]];	
}

- (ASIHTTPRequest *)HEADRequest
{
	ASIS3Request *headRequest = (ASIS3Request *)[super HEADRequest];
	[headRequest setAccessKey:[self accessKey]];
	[headRequest setSecretAccessKey:[self secretAccessKey]];
	return headRequest;
}

- (NSMutableDictionary *)S3Headers
{
	NSMutableDictionary *headers = [NSMutableDictionary dictionary];
	if ([self accessPolicy]) {
		[headers setObject:[self accessPolicy] forKey:@"x-amz-acl"];
	}
	return headers;
}

- (void)main
{
	if (![self url]) {
		[self buildURL];
	}
	[super main];
}

- (NSString *)canonicalizedResource
{
	return @"/";
}

- (NSString *)stringToSignForHeaders:(NSString *)canonicalizedAmzHeaders resource:(NSString *)canonicalizedResource
{
	return [NSString stringWithFormat:@"%@\n\n\n%@\n%@%@",[self requestMethod],[self dateString],canonicalizedAmzHeaders,canonicalizedResource];
}

- (void)buildRequestHeaders
{
	if (![self url]) {
		[self buildURL];
	}
	[super buildRequestHeaders];

	// If an access key / secret access key haven't been set for this request, let's use the shared keys
	if (![self accessKey]) {
		[self setAccessKey:[ASIS3Request sharedAccessKey]];
	}
	if (![self secretAccessKey]) {
		[self setSecretAccessKey:[ASIS3Request sharedSecretAccessKey]];
	}
	// If a date string hasn't been set, we'll create one from the current time
	if (![self dateString]) {
		[self setDate:[NSDate date]];
	}
	[self addRequestHeader:@"Date" value:[self dateString]];
	
	// Ensure our formatted string doesn't use '(null)' for the empty path
	NSString *canonicalizedResource = [self canonicalizedResource];
	
	// Add a header for the access policy if one was set, otherwise we won't add one (and S3 will default to private)
	NSMutableDictionary *amzHeaders = [self S3Headers];
	NSString *canonicalizedAmzHeaders = @"";
	for (NSString *header in [amzHeaders keysSortedByValueUsingSelector:@selector(compare:)]) {
		canonicalizedAmzHeaders = [NSString stringWithFormat:@"%@%@:%@\n",canonicalizedAmzHeaders,[header lowercaseString],[amzHeaders objectForKey:header]];
		[self addRequestHeader:header value:[amzHeaders objectForKey:header]];
	}
	
	// Jump through hoops while eating hot food
	NSString *stringToSign = [self stringToSignForHeaders:canonicalizedAmzHeaders resource:canonicalizedResource];
	NSString *signature = [ASIHTTPRequest base64forData:[ASIS3Request HMACSHA1withKey:[self secretAccessKey] forString:stringToSign]];
	NSString *authorizationString = [NSString stringWithFormat:@"AWS %@:%@",[self accessKey],signature];
	[self addRequestHeader:@"Authorization" value:authorizationString];
	
	
}

- (void)requestFinished
{
	if ([[[self responseHeaders] objectForKey:@"Content-Type"] isEqualToString:@"application/xml"]) {
		[self parseResponseXML];
	}
	if (![self error]) {
		[super requestFinished];
	}
}

#pragma mark Error XML parsing

- (void)parseResponseXML
{
	NSData* xmlData = [self responseData];
	if (![xmlData length]) {
		return;
	}
	NSXMLParser *parser = [[[NSXMLParser alloc] initWithData:xmlData] autorelease];
	[self setCurrentXMLElementStack:[NSMutableArray array]];
	[parser setDelegate:self];
	[parser setShouldProcessNamespaces:NO];
	[parser setShouldReportNamespacePrefixes:NO];
	[parser setShouldResolveExternalEntities:NO];
	[parser parse];

}

- (void)parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError
{
	[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIS3ResponseParsingFailedType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Parsing the resposnse failed",NSLocalizedDescriptionKey,parseError,NSUnderlyingErrorKey,nil]]];
}

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict
{
	[self setCurrentXMLElementContent:@""];
	[[self currentXMLElementStack] addObject:elementName];
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
	[[self currentXMLElementStack] removeLastObject];
	if ([elementName isEqualToString:@"Message"]) {
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIS3ResponseErrorType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[self currentXMLElementContent],NSLocalizedDescriptionKey,nil]]];
	// Handle S3 connection expiry errors
	} else if ([elementName isEqualToString:@"Code"]) {
		if ([[self currentXMLElementContent] isEqualToString:@"RequestTimeout"]) {
			if ([self retryUsingNewConnection]) {
				[parser abortParsing];
				return;
			}
		}
	}
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string
{
	[self setCurrentXMLElementContent:[[self currentXMLElementContent] stringByAppendingString:string]];
}

- (id)copyWithZone:(NSZone *)zone
{
	ASIS3Request *newRequest = [super copyWithZone:zone];
	[newRequest setAccessKey:[self accessKey]];
	[newRequest setSecretAccessKey:[self secretAccessKey]];
	[newRequest setRequestScheme:[self requestScheme]];
	[newRequest setAccessPolicy:[self accessPolicy]];
	return newRequest;
}


#pragma mark Shared access keys

+ (NSString *)sharedAccessKey
{
	return sharedAccessKey;
}

+ (void)setSharedAccessKey:(NSString *)newAccessKey
{
	[sharedAccessKey release];
	sharedAccessKey = [newAccessKey retain];
}

+ (NSString *)sharedSecretAccessKey
{
	return sharedSecretAccessKey;
}

+ (void)setSharedSecretAccessKey:(NSString *)newAccessKey
{
	[sharedSecretAccessKey release];
	sharedSecretAccessKey = [newAccessKey retain];
}


#pragma mark helpers

+ (NSString *)stringByURLEncodingForS3Path:(NSString *)key
{
	if (!key) {
		return @"/";
	}
	NSString *path = [(NSString *)CFURLCreateStringByAddingPercentEscapes(kCFAllocatorDefault, (CFStringRef)key, NULL, CFSTR(":?#[]@!$ &'()*+,;=\"<>%{}|\\^~`"), CFStringConvertNSStringEncodingToEncoding(NSUTF8StringEncoding)) autorelease];
	if (![[path substringWithRange:NSMakeRange(0, 1)] isEqualToString:@"/"]) {
		path = [@"/" stringByAppendingString:path];
	}
	return path;
}

// Thanks to Tom Andersen for pointing out the threading issues and providing this code!
+ (NSDateFormatter*)S3ResponseDateFormatter
{
	// We store our date formatter in the calling thread's dictionary
	// NSDateFormatter is not thread-safe, this approach ensures each formatter is only used on a single thread
	// This formatter can be reused 1000 times in parsing a single response, so it would be expensive to keep creating new date formatters
	NSMutableDictionary *threadDict = [[NSThread currentThread] threadDictionary];
	NSDateFormatter *dateFormatter = [threadDict objectForKey:@"ASIS3ResponseDateFormatter"];
	if (dateFormatter == nil) {
		dateFormatter = [[[NSDateFormatter alloc] init] autorelease];
		[dateFormatter setLocale:[[[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"] autorelease]];
		[dateFormatter setTimeZone:[NSTimeZone timeZoneWithAbbreviation:@"UTC"]];
		[dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ss'.000Z'"];
		[threadDict setObject:dateFormatter forKey:@"ASIS3ResponseDateFormatter"];
	}
	return dateFormatter;
}

+ (NSDateFormatter*)S3RequestDateFormatter
{
	NSMutableDictionary *threadDict = [[NSThread currentThread] threadDictionary];
	NSDateFormatter *dateFormatter = [threadDict objectForKey:@"ASIS3RequestHeaderDateFormatter"];
	if (dateFormatter == nil) {
		dateFormatter = [[[NSDateFormatter alloc] init] autorelease];
		// Prevent problems with dates generated by other locales (tip from: http://rel.me/t/date/)
		[dateFormatter setLocale:[[[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"] autorelease]];
		[dateFormatter setTimeZone:[NSTimeZone timeZoneWithAbbreviation:@"UTC"]];
		[dateFormatter setDateFormat:@"EEE, d MMM yyyy HH:mm:ss Z"];
		[threadDict setObject:dateFormatter forKey:@"ASIS3RequestHeaderDateFormatter"];
	}
	return dateFormatter;
	
}

// From: http://stackoverflow.com/questions/476455/is-there-a-library-for-iphone-to-work-with-hmac-sha-1-encoding

+ (NSData *)HMACSHA1withKey:(NSString *)key forString:(NSString *)string
{
	NSData *clearTextData = [string dataUsingEncoding:NSUTF8StringEncoding];
	NSData *keyData = [key dataUsingEncoding:NSUTF8StringEncoding];
	
	uint8_t digest[CC_SHA1_DIGEST_LENGTH] = {0};
	
	CCHmacContext hmacContext;
	CCHmacInit(&hmacContext, kCCHmacAlgSHA1, keyData.bytes, keyData.length);
	CCHmacUpdate(&hmacContext, clearTextData.bytes, clearTextData.length);
	CCHmacFinal(&hmacContext, digest);
	
	return [NSData dataWithBytes:digest length:CC_SHA1_DIGEST_LENGTH];
}

+ (NSString *)S3Host
{
	return @"s3.amazonaws.com";
}

- (void)buildURL
{
}

@synthesize dateString;
@synthesize accessKey;
@synthesize secretAccessKey;
@synthesize currentXMLElementContent;
@synthesize currentXMLElementStack;
@synthesize accessPolicy;
@synthesize requestScheme;
@end

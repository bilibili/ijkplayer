//
//  ASIS3ServiceRequest.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 16/03/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "ASIS3ServiceRequest.h"
#import "ASIS3Bucket.h"

// Private stuff
@interface ASIS3ServiceRequest ()
@property (retain) NSMutableArray *buckets;
@property (retain, nonatomic) ASIS3Bucket *currentBucket;
@property (retain, nonatomic) NSString *ownerID;
@property (retain, nonatomic) NSString *ownerName;
@end

@implementation ASIS3ServiceRequest

+ (id)serviceRequest
{
	ASIS3ServiceRequest *request = [[[self alloc] initWithURL:nil] autorelease];
	return request;
}

- (id)initWithURL:(NSURL *)newURL
{
	self = [super initWithURL:newURL];
	[self setBuckets:[[[NSMutableArray alloc] init] autorelease]];
	return self;
}

- (void)dealloc
{
	[buckets release];
	[currentBucket release];
	[ownerID release];
	[ownerName release];
	[super dealloc];
}

- (void)buildURL
{
	[self setURL:[NSURL URLWithString:[NSString stringWithFormat:@"%@://%@",[self requestScheme],[[self class] S3Host]]]];
}

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict
{
	if ([elementName isEqualToString:@"Bucket"]) {
		[self setCurrentBucket:[ASIS3Bucket bucketWithOwnerID:[self ownerID] ownerName:[self ownerName]]];
	}
	[super parser:parser didStartElement:elementName namespaceURI:namespaceURI qualifiedName:qName attributes:attributeDict];
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
	if ([elementName isEqualToString:@"Bucket"]) {
		[[self buckets] addObject:[self currentBucket]];
		[self setCurrentBucket:nil];
	} else if ([elementName isEqualToString:@"Name"]) {
		[[self currentBucket] setName:[self currentXMLElementContent]];
	} else if ([elementName isEqualToString:@"CreationDate"]) {
		[[self currentBucket] setCreationDate:[[ASIS3Request S3ResponseDateFormatter] dateFromString:[self currentXMLElementContent]]];
	} else if ([elementName isEqualToString:@"ID"]) {
		[self setOwnerID:[self currentXMLElementContent]];
	} else if ([elementName isEqualToString:@"DisplayName"]) {
		[self setOwnerName:[self currentXMLElementContent]];
	} else {
		// Let ASIS3Request look for error messages
		[super parser:parser didEndElement:elementName namespaceURI:namespaceURI qualifiedName:qName];
	}
}

@synthesize buckets;
@synthesize currentBucket;
@synthesize ownerID;
@synthesize ownerName;
@end

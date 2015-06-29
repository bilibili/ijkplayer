//
//  ASICloudFilesObjectRequest.m
//
//  Created by Michael Mayo on 1/6/10.
//

#import "ASICloudFilesObjectRequest.h"
#import "ASICloudFilesObject.h"


@implementation ASICloudFilesObjectRequest

@synthesize currentElement, currentContent, currentObject;
@synthesize accountName, containerName;

#pragma mark -
#pragma mark Constructors

+ (id)storageRequestWithMethod:(NSString *)method containerName:(NSString *)containerName {
	NSString *urlString = [NSString stringWithFormat:@"%@/%@", [ASICloudFilesRequest storageURL], containerName];
	ASICloudFilesObjectRequest *request = [[[ASICloudFilesObjectRequest alloc] initWithURL:[NSURL URLWithString:urlString]] autorelease];
	[request setRequestMethod:method];
	[request addRequestHeader:@"X-Auth-Token" value:[ASICloudFilesRequest authToken]];
	request.containerName = containerName;
	return request;
}

+ (id)storageRequestWithMethod:(NSString *)method containerName:(NSString *)containerName queryString:(NSString *)queryString {
	NSString *urlString = [NSString stringWithFormat:@"%@/%@%@", [ASICloudFilesRequest storageURL], containerName, queryString];
	ASICloudFilesObjectRequest *request = [[[ASICloudFilesObjectRequest alloc] initWithURL:[NSURL URLWithString:urlString]] autorelease];
	[request setRequestMethod:method];
	[request addRequestHeader:@"X-Auth-Token" value:[ASICloudFilesRequest authToken]];
	request.containerName = containerName;
	return request;
}

+ (id)storageRequestWithMethod:(NSString *)method containerName:(NSString *)containerName objectPath:(NSString *)objectPath {
	NSString *urlString = [NSString stringWithFormat:@"%@/%@/%@", [ASICloudFilesRequest storageURL], containerName, objectPath];
	ASICloudFilesObjectRequest *request = [[[ASICloudFilesObjectRequest alloc] initWithURL:[NSURL URLWithString:urlString]] autorelease];
	[request setRequestMethod:method];
	[request addRequestHeader:@"X-Auth-Token" value:[ASICloudFilesRequest authToken]];
	request.containerName = containerName;
	return request;
}

#pragma mark -
#pragma mark HEAD - Container Info

+ (id)containerInfoRequest:(NSString *)containerName {
	ASICloudFilesObjectRequest *request = [ASICloudFilesObjectRequest storageRequestWithMethod:@"HEAD" containerName:containerName];
	return request;
}

- (NSUInteger)containerObjectCount {
	return [[[self responseHeaders] objectForKey:@"X-Container-Object-Count"] intValue];
}

- (NSUInteger)containerBytesUsed {
	return [[[self responseHeaders] objectForKey:@"X-Container-Bytes-Used"] intValue];
}

#pragma mark -
#pragma mark HEAD - Object Info

+ (id)objectInfoRequest:(NSString *)containerName objectPath:(NSString *)objectPath {
	ASICloudFilesObjectRequest *request = [ASICloudFilesObjectRequest storageRequestWithMethod:@"HEAD" containerName:containerName objectPath:objectPath];
	return request;
}

#pragma mark -
#pragma mark GET - List Objects

+ (NSString *)queryStringWithContainer:(NSString *)container limit:(NSUInteger)limit marker:(NSString *)marker prefix:(NSString *)prefix path:(NSString *)path {
	NSString *queryString = @"?format=xml";
	
	if (limit && limit > 0) {
		queryString = [queryString stringByAppendingString:[NSString stringWithFormat:@"&limit=%i", limit]];
	}
	if (marker) {
		queryString = [queryString stringByAppendingString:[NSString stringWithFormat:@"&marker=%@", [marker stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]]];
	}
	if (path) {
		queryString = [queryString stringByAppendingString:[NSString stringWithFormat:@"&path=%@", [path stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]]];
	}
	
	return queryString;
}

+ (id)listRequestWithContainer:(NSString *)containerName limit:(NSUInteger)limit marker:(NSString *)marker prefix:(NSString *)prefix path:(NSString *)path {
	NSString *queryString = [ASICloudFilesObjectRequest queryStringWithContainer:containerName limit:limit marker:marker prefix:prefix path:path];
	ASICloudFilesObjectRequest *request = [ASICloudFilesObjectRequest storageRequestWithMethod:@"GET" containerName:containerName queryString:queryString];
	return request;
}

+ (id)listRequestWithContainer:(NSString *)containerName {
	return [ASICloudFilesObjectRequest listRequestWithContainer:containerName limit:0 marker:nil prefix:nil path:nil];
}

- (NSArray *)objects {
	if (objects) {
		return objects;
	}
	objects = [[[NSMutableArray alloc] init] autorelease];
	
	NSXMLParser *parser = [[[NSXMLParser alloc] initWithData:[self responseData]] autorelease];
	[parser setDelegate:self];
	[parser setShouldProcessNamespaces:NO];
	[parser setShouldReportNamespacePrefixes:NO];
	[parser setShouldResolveExternalEntities:NO];
	[parser parse];
	return objects;
}

#pragma mark -
#pragma mark GET - Retrieve Object

+ (id)getObjectRequestWithContainer:(NSString *)containerName objectPath:(NSString *)objectPath {
	return [ASICloudFilesObjectRequest storageRequestWithMethod:@"GET" containerName:containerName objectPath:objectPath];
}

- (ASICloudFilesObject *)object {
	ASICloudFilesObject *object = [ASICloudFilesObject object];
	
	NSString *path = [self url].path;
	NSRange range = [path rangeOfString:self.containerName];
	path = [path substringFromIndex:range.location + range.length + 1];
	
	object.name = path;
	object.hash = [[self responseHeaders] objectForKey:@"ETag"];
	object.bytes = [[[self responseHeaders] objectForKey:@"Content-Length"] intValue];
	object.contentType = [[self responseHeaders] objectForKey:@"Content-Type"];
	object.lastModified = [[self responseHeaders] objectForKey:@"Last-Modified"];
	object.metadata = [NSMutableDictionary dictionary];
	
	for (NSString *key in [[self responseHeaders] keyEnumerator]) {
		NSRange metaRange = [key rangeOfString:@"X-Object-Meta-"];
		if (metaRange.location == 0) {
			[object.metadata setObject:[[self responseHeaders] objectForKey:key] forKey:[key substringFromIndex:metaRange.length]];
		}
	}
	
	object.data = [self responseData];
	
	return object;
}

#pragma mark -
#pragma mark PUT - Upload Object

+ (id)putObjectRequestWithContainer:(NSString *)containerName object:(ASICloudFilesObject *)object {
	return [self putObjectRequestWithContainer:containerName objectPath:object.name contentType:object.contentType objectData:object.data metadata:object.metadata etag:nil];
}

+ (id)putObjectRequestWithContainer:(NSString *)containerName objectPath:(NSString *)objectPath contentType:(NSString *)contentType objectData:(NSData *)objectData metadata:(NSDictionary *)metadata etag:(NSString *)etag {
	
	ASICloudFilesObjectRequest *request = [ASICloudFilesObjectRequest storageRequestWithMethod:@"PUT" containerName:containerName objectPath:objectPath];
	[request addRequestHeader:@"Content-Type" value:contentType];

	// add metadata to headers
	if (metadata) {
		for (NSString *key in [metadata keyEnumerator]) {
			[request addRequestHeader:[NSString stringWithFormat:@"X-Object-Meta-%@", key] value:[metadata objectForKey:key]];
		}
	}	
	
	[request appendPostData:objectData];	
	return request;
}

+ (id)putObjectRequestWithContainer:(NSString *)containerName objectPath:(NSString *)objectPath contentType:(NSString *)contentType file:(NSString *)filePath metadata:(NSDictionary *)metadata etag:(NSString *)etag
{
	ASICloudFilesObjectRequest *request = [ASICloudFilesObjectRequest storageRequestWithMethod:@"PUT" containerName:containerName objectPath:objectPath];
	[request addRequestHeader:@"Content-Type" value:contentType];
	
	// add metadata to headers
	if (metadata) {
		for (NSString *key in [metadata keyEnumerator]) {
			[request addRequestHeader:[NSString stringWithFormat:@"X-Object-Meta-%@", key] value:[metadata objectForKey:key]];
		}
	}	
	
	[request setShouldStreamPostDataFromDisk:YES];
	[request setPostBodyFilePath:filePath];
	return request;	
}

#pragma mark -
#pragma mark POST - Set Object Metadata

+ (id)postObjectRequestWithContainer:(NSString *)containerName object:(ASICloudFilesObject *)object {
	return [self postObjectRequestWithContainer:containerName objectPath:object.name metadata:object.metadata];
}

+ (id)postObjectRequestWithContainer:(NSString *)containerName objectPath:(NSString *)objectPath metadata:(NSDictionary *)metadata {
	ASICloudFilesObjectRequest *request = [ASICloudFilesObjectRequest storageRequestWithMethod:@"POST" containerName:containerName objectPath:objectPath];
	
	// add metadata to headers
	if (metadata) {
		for (NSString *key in [metadata keyEnumerator]) {
			[request addRequestHeader:[NSString stringWithFormat:@"X-Object-Meta-%@", key] value:[metadata objectForKey:key]];
		}
	}	
	
	return request;
}

#pragma mark -
#pragma mark DELETE - Delete Object

+ (id)deleteObjectRequestWithContainer:(NSString *)containerName objectPath:(NSString *)objectPath {
	ASICloudFilesObjectRequest *request = [ASICloudFilesObjectRequest storageRequestWithMethod:@"DELETE" containerName:containerName objectPath:objectPath];
	return request;
}

#pragma mark -
#pragma mark XML Parser Delegate

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict {
	[self setCurrentElement:elementName];
	
	if ([elementName isEqualToString:@"object"]) {
		[self setCurrentObject:[ASICloudFilesObject object]];
	}
	[self setCurrentContent:@""];
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName {
	if ([elementName isEqualToString:@"name"]) {
		[self currentObject].name = [self currentContent];
	} else if ([elementName isEqualToString:@"hash"]) {
		[self currentObject].hash = [self currentContent];
	} else if ([elementName isEqualToString:@"bytes"]) {
		[self currentObject].bytes = [[self currentContent] intValue];
	} else if ([elementName isEqualToString:@"content_type"]) {
		[self currentObject].contentType = [self currentContent];
	} else if ([elementName isEqualToString:@"last_modified"]) {
		[self currentObject].lastModified = [self dateFromString:[self currentContent]];
	} else if ([elementName isEqualToString:@"object"]) {
		// we're done with this object.  time to move on to the next
		[objects addObject:currentObject];
		[self setCurrentObject:nil];
	}
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string {
	[self setCurrentContent:[[self currentContent] stringByAppendingString:string]];
}

#pragma mark -
#pragma mark Memory Management

- (void)dealloc {
	[currentElement release];
	[currentContent release];
	[currentObject release];
	[accountName release];
	[containerName release];
	[super dealloc];
}

@end

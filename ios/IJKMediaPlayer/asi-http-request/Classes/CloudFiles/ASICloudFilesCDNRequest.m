//
//  ASICloudFilesCDNRequest.m
//
//  Created by Michael Mayo on 1/6/10.
//

#import "ASICloudFilesCDNRequest.h"
#import "ASICloudFilesContainerXMLParserDelegate.h"


@implementation ASICloudFilesCDNRequest

@synthesize accountName, containerName, xmlParserDelegate;

+ (id)cdnRequestWithMethod:(NSString *)method query:(NSString *)query {
	NSString *urlString = [NSString stringWithFormat:@"%@%@", [ASICloudFilesRequest cdnManagementURL], query];
	ASICloudFilesCDNRequest *request = [[[ASICloudFilesCDNRequest alloc] initWithURL:[NSURL URLWithString:urlString]] autorelease];
	[request setRequestMethod:method];
	[request addRequestHeader:@"X-Auth-Token" value:[ASICloudFilesRequest authToken]];
	return request;
}

+ (id)cdnRequestWithMethod:(NSString *)method containerName:(NSString *)containerName {
	NSString *urlString = [NSString stringWithFormat:@"%@/%@", [ASICloudFilesRequest cdnManagementURL], containerName];
	ASICloudFilesCDNRequest *request = [[[ASICloudFilesCDNRequest alloc] initWithURL:[NSURL URLWithString:urlString]] autorelease];
	[request setRequestMethod:method];
	[request addRequestHeader:@"X-Auth-Token" value:[ASICloudFilesRequest authToken]];
	request.containerName = containerName;
	return request;
}

#pragma mark -
#pragma mark HEAD - Container Info

+ (id)containerInfoRequest:(NSString *)containerName {
	ASICloudFilesCDNRequest *request = [ASICloudFilesCDNRequest cdnRequestWithMethod:@"HEAD" containerName:containerName];
	return request;
}

- (BOOL)cdnEnabled {
    NSNumber *enabled = [[self responseHeaders] objectForKey:@"X-CDN-Enabled"];
    if (!enabled) {
        enabled = [[self responseHeaders] objectForKey:@"X-Cdn-Enabled"];
    }
	return [enabled boolValue];
}

- (NSString *)cdnURI {
	NSString *uri = [[self responseHeaders] objectForKey:@"X-CDN-URI"];
    if (!uri) {
        uri = [[self responseHeaders] objectForKey:@"X-Cdn-Uri"];
    }
    return uri;
}

- (NSString *)cdnSSLURI {
    NSString *uri = [[self responseHeaders] objectForKey:@"X-CDN-SSL-URI"];
    if (!uri) {
        uri = [[self responseHeaders] objectForKey:@"X-Cdn-Ssl-Uri"];
    }
	return uri;
}

- (NSUInteger)cdnTTL {
    NSNumber *ttl = [[self responseHeaders] objectForKey:@"X-TTL"];
    if (!ttl) {
        ttl = [[self responseHeaders] objectForKey:@"X-Ttl"];
    }
    return [ttl intValue];
}

#pragma mark -
#pragma mark GET - CDN Container Lists

+ (id)listRequest {
	ASICloudFilesCDNRequest *request = [ASICloudFilesCDNRequest cdnRequestWithMethod:@"GET" query:@"?format=xml"];
	return request;
}

+ (id)listRequestWithLimit:(NSUInteger)limit marker:(NSString *)marker enabledOnly:(BOOL)enabledOnly  {
	NSString *query = @"?format=xml";
	
	if (limit > 0) {
		query = [query stringByAppendingString:[NSString stringWithFormat:@"&limit=%i", limit]];
	}
	
	if (marker) {
		query = [query stringByAppendingString:[NSString stringWithFormat:@"&marker=%@", marker]];
	}
	
	if (limit > 0) {
		query = [query stringByAppendingString:[NSString stringWithFormat:@"&limit=%i", limit]];
	}
	
	ASICloudFilesCDNRequest *request = [ASICloudFilesCDNRequest cdnRequestWithMethod:@"GET" query:query];
	return request;
}

- (NSArray *)containers {
	if (xmlParserDelegate.containerObjects) {
		return xmlParserDelegate.containerObjects;
	}
	
	NSXMLParser *parser = [[[NSXMLParser alloc] initWithData:[self responseData]] autorelease];
	if (xmlParserDelegate == nil) {
		xmlParserDelegate = [[ASICloudFilesContainerXMLParserDelegate alloc] init];
	}
	
	[parser setDelegate:xmlParserDelegate];
	[parser setShouldProcessNamespaces:NO];
	[parser setShouldReportNamespacePrefixes:NO];
	[parser setShouldResolveExternalEntities:NO];
	[parser parse];
	
	return xmlParserDelegate.containerObjects;
}

#pragma mark -
#pragma mark PUT - CDN Enable Container

// PUT /<api version>/<account>/<container>
// PUT operations against a Container are used to CDN-enable that Container.
// Include an HTTP header of X-TTL to specify a custom TTL.
+ (id)putRequestWithContainer:(NSString *)containerName {
	ASICloudFilesCDNRequest *request = [ASICloudFilesCDNRequest cdnRequestWithMethod:@"PUT" containerName:containerName];
	return request;
}

+ (id)putRequestWithContainer:(NSString *)containerName ttl:(NSUInteger)ttl {
	ASICloudFilesCDNRequest *request = [ASICloudFilesCDNRequest cdnRequestWithMethod:@"PUT" containerName:containerName];	
	[request addRequestHeader:@"X-Ttl" value:[NSString stringWithFormat:@"%i", ttl]];
	return request;
}

#pragma mark -
#pragma mark POST - Adjust CDN Attributes

// POST /<api version>/<account>/<container>
// POST operations against a CDN-enabled Container are used to adjust CDN attributes.
// The POST operation can be used to set a new TTL cache expiration or to enable/disable public sharing over the CDN.
// X-TTL: 86400
// X-CDN-Enabled: True
+ (id)postRequestWithContainer:(NSString *)containerName {
	ASICloudFilesCDNRequest *request = [ASICloudFilesCDNRequest cdnRequestWithMethod:@"POST" containerName:containerName];
	return request;
}

+ (id)postRequestWithContainer:(NSString *)containerName cdnEnabled:(BOOL)cdnEnabled ttl:(NSUInteger)ttl {
	ASICloudFilesCDNRequest *request = [ASICloudFilesCDNRequest cdnRequestWithMethod:@"POST" containerName:containerName];
	if (ttl > 0) {
		[request addRequestHeader:@"X-Ttl" value:[NSString stringWithFormat:@"%i", ttl]];
	}
	[request addRequestHeader:@"X-CDN-Enabled" value:cdnEnabled ? @"True" : @"False"];
	return request;
}

#pragma mark -
#pragma mark Memory Management

-(void)dealloc {
	[accountName release];
	[containerName release];
	[xmlParserDelegate release];
	[super dealloc];
}

@end

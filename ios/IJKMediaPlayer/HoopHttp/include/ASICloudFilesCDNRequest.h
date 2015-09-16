//
//  ASICloudFilesCDNRequest.h
//
//  Created by Michael Mayo on 1/6/10.
//

#import "ASICloudFilesRequest.h"

@class ASICloudFilesContainerXMLParserDelegate;

@interface ASICloudFilesCDNRequest : ASICloudFilesRequest {
	NSString *accountName;
	NSString *containerName;
	ASICloudFilesContainerXMLParserDelegate *xmlParserDelegate;
	
}

@property (retain) NSString *accountName;
@property (retain) NSString *containerName;
@property (retain) ASICloudFilesContainerXMLParserDelegate *xmlParserDelegate;


// HEAD /<api version>/<account>/<container>
// Response:
// X-CDN-Enabled: True
// X-CDN-URI: http://cdn.cloudfiles.mosso.com/c1234
// X-CDN-SSL-URI: https://cdn.ssl.cloudfiles.mosso.com/c1234
// X-CDN-TTL: 86400
+ (id)containerInfoRequest:(NSString *)containerName;
- (BOOL)cdnEnabled;
- (NSString *)cdnURI;
- (NSString *)cdnSSLURI;
- (NSUInteger)cdnTTL;


// GET /<api version>/<account>
// limit, marker, format, enabled_only=true
+ (id)listRequest;
+ (id)listRequestWithLimit:(NSUInteger)limit marker:(NSString *)marker enabledOnly:(BOOL)enabledOnly;
- (NSArray *)containers;


// PUT /<api version>/<account>/<container>
// PUT operations against a Container are used to CDN-enable that Container.
// Include an HTTP header of X-TTL to specify a custom TTL.
+ (id)putRequestWithContainer:(NSString *)containerName;
+ (id)putRequestWithContainer:(NSString *)containerName ttl:(NSUInteger)ttl;
// returns: - (NSString *)cdnURI;

// POST /<api version>/<account>/<container>
// POST operations against a CDN-enabled Container are used to adjust CDN attributes.
// The POST operation can be used to set a new TTL cache expiration or to enable/disable public sharing over the CDN.
// X-TTL: 86400
// X-CDN-Enabled: True
+ (id)postRequestWithContainer:(NSString *)containerName;
+ (id)postRequestWithContainer:(NSString *)containerName cdnEnabled:(BOOL)cdnEnabled ttl:(NSUInteger)ttl;
// returns: - (NSString *)cdnURI;


@end

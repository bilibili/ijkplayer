//
//  ASICloudFilesObjectRequest.h
//
//  Created by Michael Mayo on 1/6/10.
//

#import "ASICloudFilesRequest.h"

#if !TARGET_OS_IPHONE || (TARGET_OS_IPHONE && __IPHONE_OS_VERSION_MAX_ALLOWED < __IPHONE_4_0)
#import "ASINSXMLParserCompat.h"
#endif

@class ASICloudFilesObject;

@interface ASICloudFilesObjectRequest : ASICloudFilesRequest <NSXMLParserDelegate> {

	
	NSString *accountName;
	NSString *containerName;
	
	// Internally used while parsing the response
	NSString *currentContent;
	NSString *currentElement;
	ASICloudFilesObject *currentObject;
	NSMutableArray *objects;
	
}

@property (retain) NSString *accountName;
@property (retain) NSString *containerName;
@property (retain) NSString *currentElement;
@property (retain) NSString *currentContent;
@property (retain) ASICloudFilesObject *currentObject;


// HEAD /<api version>/<account>/<container>
// HEAD operations against an account are performed to retrieve the number of Containers and the total bytes stored in Cloud Files for the account. This information is returned in two custom headers, X-Account-Container-Count and X-Account-Bytes-Used.
+ (id)containerInfoRequest:(NSString *)containerName;
- (NSUInteger)containerObjectCount;
- (NSUInteger)containerBytesUsed;

// HEAD /<api version>/<account>/<container>/<object>
// to get metadata
+ (id)objectInfoRequest:(NSString *)containerName objectPath:(NSString *)objectPath;
- (NSArray *)objects;

+ (id)listRequestWithContainer:(NSString *)containerName;
+ (id)listRequestWithContainer:(NSString *)containerName limit:(NSUInteger)limit marker:(NSString *)marker prefix:(NSString *)prefix path:(NSString *)path;

// Conditional GET headers: If-Match • If-None-Match • If-Modified-Since • If-Unmodified-Since
// HTTP Range header: “Range: bytes=0-5” •	“Range: bytes=-5” •	“Range: bytes=32-“
+ (id)getObjectRequestWithContainer:(NSString *)containerName objectPath:(NSString *)objectPath;
- (ASICloudFilesObject *)object;

// PUT /<api version>/<account>/<container>/<object>
// PUT operations are used to write, or overwrite, an Object's metadata and content.
// The Object can be created with custom metadata via HTTP headers identified with the “X-Object-Meta-” prefix.
+ (id)putObjectRequestWithContainer:(NSString *)containerName object:(ASICloudFilesObject *)object;
+ (id)putObjectRequestWithContainer:(NSString *)containerName objectPath:(NSString *)objectPath contentType:(NSString *)contentType objectData:(NSData *)objectData metadata:(NSDictionary *)metadata etag:(NSString *)etag;
+ (id)putObjectRequestWithContainer:(NSString *)containerName objectPath:(NSString *)objectPath contentType:(NSString *)contentType file:(NSString *)filePath metadata:(NSDictionary *)metadata etag:(NSString *)etag;

// POST /<api version>/<account>/<container>/<object>
// POST operations against an Object name are used to set and overwrite arbitrary key/value metadata. You cannot use the POST operation to change any of the Object's other headers such as Content-Type, ETag, etc. It is not used to upload storage Objects (see PUT).
// A POST request will delete all existing metadata added with a previous PUT/POST.
+ (id)postObjectRequestWithContainer:(NSString *)containerName object:(ASICloudFilesObject *)object;
+ (id)postObjectRequestWithContainer:(NSString *)containerName objectPath:(NSString *)objectPath metadata:(NSDictionary *)metadata;

// DELETE /<api version>/<account>/<container>/<object>
+ (id)deleteObjectRequestWithContainer:(NSString *)containerName objectPath:(NSString *)objectPath;

@end

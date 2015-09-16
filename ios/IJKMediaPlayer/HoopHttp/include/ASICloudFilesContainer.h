//
//  ASICloudFilesContainer.h
//
//  Created by Michael Mayo on 1/7/10.
//

#import <Foundation/Foundation.h>


@interface ASICloudFilesContainer : NSObject {
	
	// regular container attributes
	NSString *name;
	NSUInteger count;
	NSUInteger bytes;
	
	// CDN container attributes
	BOOL cdnEnabled;
	NSUInteger ttl;
	NSString *cdnURL;
	BOOL logRetention;
	NSString *referrerACL;
	NSString *useragentACL;
}

+ (id)container;

// regular container attributes
@property (retain) NSString *name;
@property (assign) NSUInteger count;
@property (assign) NSUInteger bytes;

// CDN container attributes
@property (assign) BOOL cdnEnabled;
@property (assign) NSUInteger ttl;
@property (retain) NSString *cdnURL;
@property (assign) BOOL logRetention;
@property (retain) NSString *referrerACL;
@property (retain) NSString *useragentACL;

@end

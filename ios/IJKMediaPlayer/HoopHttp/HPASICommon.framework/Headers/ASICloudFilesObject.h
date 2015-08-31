//
//  ASICloudFilesObject.h
//
//  Created by Michael Mayo on 1/7/10.
//

#import <Foundation/Foundation.h>


@interface ASICloudFilesObject : NSObject {
	NSString *name;
	NSString *hash;
	NSUInteger bytes;
	NSString *contentType;
	NSDate *lastModified;
	NSData *data;
	NSMutableDictionary *metadata;
}

@property (retain) NSString *name;
@property (retain) NSString *hash;
@property (assign) NSUInteger bytes;
@property (retain) NSString *contentType;
@property (retain) NSDate *lastModified;
@property (retain) NSData *data;	
@property (retain) NSMutableDictionary *metadata;

+ (id)object;

@end

//
//  ASIDataDecompressor.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 17/08/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

// This is a helper class used by ASIHTTPRequest to handle inflating (decompressing) data in memory and on disk
// You may also find it helpful if you need to inflate data and files yourself - see the class methods below
// Most of the zlib stuff is based on the sample code by Mark Adler available at http://zlib.net

#import <Foundation/Foundation.h>
#import <zlib.h>

@interface ASIDataDecompressor : NSObject {
	BOOL streamReady;
	z_stream zStream;
}

// Convenience constructor will call setupStream for you
+ (id)decompressor;

// Uncompress the passed chunk of data
- (NSData *)uncompressBytes:(Bytef *)bytes length:(NSUInteger)length error:(NSError **)err;

// Convenience method - pass it some deflated data, and you'll get inflated data back
+ (NSData *)uncompressData:(NSData*)compressedData error:(NSError **)err;

// Convenience method - pass it a file containing deflated data in sourcePath, and it will write inflated data to destinationPath
+ (BOOL)uncompressDataFromFile:(NSString *)sourcePath toFile:(NSString *)destinationPath error:(NSError **)err;

// Sets up zlib to handle the inflating. You only need to call this yourself if you aren't using the convenience constructor 'decompressor'
- (NSError *)setupStream;

// Tells zlib to clean up. You need to call this if you need to cancel inflating part way through
// If inflating finishes or fails, this method will be called automatically
- (NSError *)closeStream;

@property (assign, readonly) BOOL streamReady;
@end

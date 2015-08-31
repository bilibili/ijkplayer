//
//  ASIDataCompressor.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 17/08/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

// This is a helper class used by ASIHTTPRequest to handle deflating (compressing) data in memory and on disk
// You may also find it helpful if you need to deflate data and files yourself - see the class methods below
// Most of the zlib stuff is based on the sample code by Mark Adler available at http://zlib.net

#import <Foundation/Foundation.h>
#import <zlib.h>

@interface ASIDataCompressor : NSObject {
	BOOL streamReady;
	z_stream zStream;
}

// Convenience constructor will call setupStream for you
+ (id)compressor;

// Compress the passed chunk of data
// Passing YES for shouldFinish will finalize the deflated data - you must pass YES when you are on the last chunk of data
- (NSData *)compressBytes:(Bytef *)bytes length:(NSUInteger)length error:(NSError **)err shouldFinish:(BOOL)shouldFinish;

// Convenience method - pass it some data, and you'll get deflated data back
+ (NSData *)compressData:(NSData*)uncompressedData error:(NSError **)err;

// Convenience method - pass it a file containing the data to compress in sourcePath, and it will write deflated data to destinationPath
+ (BOOL)compressDataFromFile:(NSString *)sourcePath toFile:(NSString *)destinationPath error:(NSError **)err;

// Sets up zlib to handle the inflating. You only need to call this yourself if you aren't using the convenience constructor 'compressor'
- (NSError *)setupStream;

// Tells zlib to clean up. You need to call this if you need to cancel deflating part way through
// If deflating finishes or fails, this method will be called automatically
- (NSError *)closeStream;

@property (assign, readonly) BOOL streamReady;
@end

//
//  ASIDataCompressor.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 17/08/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "ASIDataCompressor.h"
#import "ASIHTTPRequest.h"

#define DATA_CHUNK_SIZE 262144 // Deal with gzipped data in 256KB chunks
#define COMPRESSION_AMOUNT Z_DEFAULT_COMPRESSION

@interface ASIDataCompressor ()
+ (NSError *)deflateErrorWithCode:(int)code;
@end

@implementation ASIDataCompressor

+ (id)compressor
{
	ASIDataCompressor *compressor = [[[self alloc] init] autorelease];
	[compressor setupStream];
	return compressor;
}

- (void)dealloc
{
	if (streamReady) {
		[self closeStream];
	}
	[super dealloc];
}

- (NSError *)setupStream
{
	if (streamReady) {
		return nil;
	}
	// Setup the inflate stream
	zStream.zalloc = Z_NULL;
	zStream.zfree = Z_NULL;
	zStream.opaque = Z_NULL;
	zStream.avail_in = 0;
	zStream.next_in = 0;
	int status = deflateInit2(&zStream, COMPRESSION_AMOUNT, Z_DEFLATED, (15+16), 8, Z_DEFAULT_STRATEGY);
	if (status != Z_OK) {
		return [[self class] deflateErrorWithCode:status];
	}
	streamReady = YES;
	return nil;
}

- (NSError *)closeStream
{
	if (!streamReady) {
		return nil;
	}
	// Close the deflate stream
	streamReady = NO;
	int status = deflateEnd(&zStream);
	if (status != Z_OK) {
		return [[self class] deflateErrorWithCode:status];
	}
	return nil;
}

- (NSData *)compressBytes:(Bytef *)bytes length:(NSUInteger)length error:(NSError **)err shouldFinish:(BOOL)shouldFinish
{
	if (length == 0) return nil;
	
	NSUInteger halfLength = length/2;
	
	// We'll take a guess that the compressed data will fit in half the size of the original (ie the max to compress at once is half DATA_CHUNK_SIZE), if not, we'll increase it below
	NSMutableData *outputData = [NSMutableData dataWithLength:length/2]; 
	
	int status;
	
	zStream.next_in = bytes;
	zStream.avail_in = (unsigned int)length;
	zStream.avail_out = 0;

	NSUInteger bytesProcessedAlready = zStream.total_out;
	while (zStream.avail_out == 0) {
		
		if (zStream.total_out-bytesProcessedAlready >= [outputData length]) {
			[outputData increaseLengthBy:halfLength];
		}
		
		zStream.next_out = (Bytef*)[outputData mutableBytes] + zStream.total_out-bytesProcessedAlready;
		zStream.avail_out = (unsigned int)([outputData length] - (zStream.total_out-bytesProcessedAlready));
		status = deflate(&zStream, shouldFinish ? Z_FINISH : Z_NO_FLUSH);
		
		if (status == Z_STREAM_END) {
			break;
		} else if (status != Z_OK) {
			if (err) {
				*err = [[self class] deflateErrorWithCode:status];
			}
			return NO;
		}
	}

	// Set real length
	[outputData setLength: zStream.total_out-bytesProcessedAlready];
	return outputData;
}


+ (NSData *)compressData:(NSData*)uncompressedData error:(NSError **)err
{
	NSError *theError = nil;
	NSData *outputData = [[ASIDataCompressor compressor] compressBytes:(Bytef *)[uncompressedData bytes] length:[uncompressedData length] error:&theError shouldFinish:YES];
	if (theError) {
		if (err) {
			*err = theError;
		}
		return nil;
	}
	return outputData;
}



+ (BOOL)compressDataFromFile:(NSString *)sourcePath toFile:(NSString *)destinationPath error:(NSError **)err
{
	NSFileManager *fileManager = [[[NSFileManager alloc] init] autorelease];

	// Create an empty file at the destination path
	if (![fileManager createFileAtPath:destinationPath contents:[NSData data] attributes:nil]) {
		if (err) {
			*err = [NSError errorWithDomain:NetworkRequestErrorDomain code:ASICompressionError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Compression of %@ failed because we were to create a file at %@",sourcePath,destinationPath],NSLocalizedDescriptionKey,nil]];
		}
		return NO;
	}
	
	// Ensure the source file exists
	if (![fileManager fileExistsAtPath:sourcePath]) {
		if (err) {
			*err = [NSError errorWithDomain:NetworkRequestErrorDomain code:ASICompressionError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Compression of %@ failed the file does not exist",sourcePath],NSLocalizedDescriptionKey,nil]];
		}
		return NO;
	}
	
	UInt8 inputData[DATA_CHUNK_SIZE];
	NSData *outputData;
	NSInteger readLength;
	NSError *theError = nil;
	
	ASIDataCompressor *compressor = [ASIDataCompressor compressor];
	
	NSInputStream *inputStream = [NSInputStream inputStreamWithFileAtPath:sourcePath];
	[inputStream open];
	NSOutputStream *outputStream = [NSOutputStream outputStreamToFileAtPath:destinationPath append:NO];
	[outputStream open];
	
    while ([compressor streamReady]) {
		
		// Read some data from the file
		readLength = [inputStream read:inputData maxLength:DATA_CHUNK_SIZE];

		// Make sure nothing went wrong
		if ([inputStream streamStatus] == NSStreamStatusError) {
			if (err) {
				*err = [NSError errorWithDomain:NetworkRequestErrorDomain code:ASICompressionError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Compression of %@ failed because we were unable to read from the source data file",sourcePath],NSLocalizedDescriptionKey,[inputStream streamError],NSUnderlyingErrorKey,nil]];
			}
			[compressor closeStream];
			return NO;
		}
		// Have we reached the end of the input data?
		if (!readLength) {
			break;
		}
		
		// Attempt to deflate the chunk of data
		outputData = [compressor compressBytes:inputData length:(NSUInteger)readLength error:&theError shouldFinish:readLength < DATA_CHUNK_SIZE];
		if (theError) {
			if (err) {
				*err = theError;
			}
			[compressor closeStream];
			return NO;
		}
		
		// Write the deflated data out to the destination file
		[outputStream write:(const uint8_t *)[outputData bytes] maxLength:[outputData length]];
		
		// Make sure nothing went wrong
		if ([inputStream streamStatus] == NSStreamStatusError) {
			if (err) {
				*err = [NSError errorWithDomain:NetworkRequestErrorDomain code:ASICompressionError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Compression of %@ failed because we were unable to write to the destination data file at %@",sourcePath,destinationPath],NSLocalizedDescriptionKey,[outputStream streamError],NSUnderlyingErrorKey,nil]];
            }
			[compressor closeStream];
			return NO;
		}
		
    }
	[inputStream close];
	[outputStream close];

	NSError *error = [compressor closeStream];
	if (error) {
		if (err) {
			*err = error;
		}
		return NO;
	}

	return YES;
}

+ (NSError *)deflateErrorWithCode:(int)code
{
	return [NSError errorWithDomain:NetworkRequestErrorDomain code:ASICompressionError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Compression of data failed with code %d",code],NSLocalizedDescriptionKey,nil]];
}

@synthesize streamReady;
@end

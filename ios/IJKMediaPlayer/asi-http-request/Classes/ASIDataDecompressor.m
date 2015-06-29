//
//  ASIDataDecompressor.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 17/08/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "ASIDataDecompressor.h"
#import "ASIHTTPRequest.h"

#define DATA_CHUNK_SIZE 262144 // Deal with gzipped data in 256KB chunks

@interface ASIDataDecompressor ()
+ (NSError *)inflateErrorWithCode:(int)code;
@end;

@implementation ASIDataDecompressor

+ (id)decompressor
{
	ASIDataDecompressor *decompressor = [[[self alloc] init] autorelease];
	[decompressor setupStream];
	return decompressor;
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
	int status = inflateInit2(&zStream, (15+32));
	if (status != Z_OK) {
		return [[self class] inflateErrorWithCode:status];
	}
	streamReady = YES;
	return nil;
}

- (NSError *)closeStream
{
	if (!streamReady) {
		return nil;
	}
	// Close the inflate stream
	streamReady = NO;
	int status = inflateEnd(&zStream);
	if (status != Z_OK) {
		return [[self class] inflateErrorWithCode:status];
	}
	return nil;
}

- (NSData *)uncompressBytes:(Bytef *)bytes length:(NSUInteger)length error:(NSError **)err
{
	if (length == 0) return nil;
	
	NSUInteger halfLength = length/2;
	NSMutableData *outputData = [NSMutableData dataWithLength:length+halfLength];

	int status;
	
	zStream.next_in = bytes;
	zStream.avail_in = (unsigned int)length;
	zStream.avail_out = 0;
	
	NSUInteger bytesProcessedAlready = zStream.total_out;
	while (zStream.avail_in != 0) {
		
		if (zStream.total_out-bytesProcessedAlready >= [outputData length]) {
			[outputData increaseLengthBy:halfLength];
		}
		
		zStream.next_out = (Bytef*)[outputData mutableBytes] + zStream.total_out-bytesProcessedAlready;
		zStream.avail_out = (unsigned int)([outputData length] - (zStream.total_out-bytesProcessedAlready));
		
		status = inflate(&zStream, Z_NO_FLUSH);
		
		if (status == Z_STREAM_END) {
			break;
		} else if (status != Z_OK) {
			if (err) {
				*err = [[self class] inflateErrorWithCode:status];
			}
			return nil;
		}
	}
	
	// Set real length
	[outputData setLength: zStream.total_out-bytesProcessedAlready];
	return outputData;
}


+ (NSData *)uncompressData:(NSData*)compressedData error:(NSError **)err
{
	NSError *theError = nil;
	NSData *outputData = [[ASIDataDecompressor decompressor] uncompressBytes:(Bytef *)[compressedData bytes] length:[compressedData length] error:&theError];
	if (theError) {
		if (err) {
			*err = theError;
		}
		return nil;
	}
	return outputData;
}

+ (BOOL)uncompressDataFromFile:(NSString *)sourcePath toFile:(NSString *)destinationPath error:(NSError **)err
{
	NSFileManager *fileManager = [[[NSFileManager alloc] init] autorelease];

	// Create an empty file at the destination path
	if (![fileManager createFileAtPath:destinationPath contents:[NSData data] attributes:nil]) {
		if (err) {
			*err = [NSError errorWithDomain:NetworkRequestErrorDomain code:ASICompressionError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Decompression of %@ failed because we were to create a file at %@",sourcePath,destinationPath],NSLocalizedDescriptionKey,nil]];
		}
		return NO;
	}
	
	// Ensure the source file exists
	if (![fileManager fileExistsAtPath:sourcePath]) {
		if (err) {
			*err = [NSError errorWithDomain:NetworkRequestErrorDomain code:ASICompressionError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Decompression of %@ failed the file does not exist",sourcePath],NSLocalizedDescriptionKey,nil]];
		}
		return NO;
	}
	
	UInt8 inputData[DATA_CHUNK_SIZE];
	NSData *outputData;
	NSInteger readLength;
	NSError *theError = nil;
	

	ASIDataDecompressor *decompressor = [ASIDataDecompressor decompressor];

	NSInputStream *inputStream = [NSInputStream inputStreamWithFileAtPath:sourcePath];
	[inputStream open];
	NSOutputStream *outputStream = [NSOutputStream outputStreamToFileAtPath:destinationPath append:NO];
	[outputStream open];
	
    while ([decompressor streamReady]) {
		
		// Read some data from the file
		readLength = [inputStream read:inputData maxLength:DATA_CHUNK_SIZE];
		
		// Make sure nothing went wrong
		if ([inputStream streamStatus] == NSStreamStatusError) {
			if (err) {
				*err = [NSError errorWithDomain:NetworkRequestErrorDomain code:ASICompressionError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Decompression of %@ failed because we were unable to read from the source data file",sourcePath],NSLocalizedDescriptionKey,[inputStream streamError],NSUnderlyingErrorKey,nil]];
			}
            [decompressor closeStream];
			return NO;
		}
		// Have we reached the end of the input data?
		if (!readLength) {
			break;
		}

		// Attempt to inflate the chunk of data
		outputData = [decompressor uncompressBytes:inputData length:(NSUInteger)readLength error:&theError];
		if (theError) {
			if (err) {
				*err = theError;
			}
			[decompressor closeStream];
			return NO;
		}
		
		// Write the inflated data out to the destination file
		[outputStream write:(Bytef*)[outputData bytes] maxLength:[outputData length]];
		
		// Make sure nothing went wrong
		if ([inputStream streamStatus] == NSStreamStatusError) {
			if (err) {
				*err = [NSError errorWithDomain:NetworkRequestErrorDomain code:ASICompressionError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Decompression of %@ failed because we were unable to write to the destination data file at %@",sourcePath,destinationPath],NSLocalizedDescriptionKey,[outputStream streamError],NSUnderlyingErrorKey,nil]];
            }
			[decompressor closeStream];
			return NO;
		}
		
    }
	
	[inputStream close];
	[outputStream close];

	NSError *error = [decompressor closeStream];
	if (error) {
		if (err) {
			*err = error;
		}
		return NO;
	}

	return YES;
}


+ (NSError *)inflateErrorWithCode:(int)code
{
	return [NSError errorWithDomain:NetworkRequestErrorDomain code:ASICompressionError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Decompression of data failed with code %d",code],NSLocalizedDescriptionKey,nil]];
}

@synthesize streamReady;
@end

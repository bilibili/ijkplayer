#import "TestPreBuffer.h"
#import <Security/SecRandom.h>


#define COUNT 25000

/**
 * Interface definition copied from GCDAsyncSocket.m (to make it public for testing).
 * 
 * The implementation itself remains within GCDAsyncSocket.m
**/

@interface GCDAsyncSocketPreBuffer : NSObject

- (id)initWithCapacity:(size_t)numBytes;

- (void)ensureCapacityForWrite:(size_t)numBytes;

- (size_t)availableBytes;
- (uint8_t *)readBuffer;

- (void)getReadBuffer:(uint8_t **)bufferPtr availableBytes:(size_t *)availableBytesPtr;

- (size_t)availableSpace;
- (uint8_t *)writeBuffer;

- (void)getWriteBuffer:(uint8_t **)bufferPtr availableSpace:(size_t *)availableSpacePtr;

- (void)didRead:(size_t)bytesRead;
- (void)didWrite:(size_t)bytesWritten;

- (void)reset;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface TestPreBuffer ()

+ (void)test_preBuffer;

+ (void)benchmark_mutableData;
+ (void)benchmark_preBuffer;

@end


@implementation TestPreBuffer

static size_t bufferSize;

static int randomSize1;
static int randomSize2;

+ (void)start
{
	// Run unit tests
	
	[self test_preBuffer];

	// Setup benchmarks.
	// 
	// We're going to test a common pattern within GCDAsyncSocket, which is:
	// - write a chunk of data to the preBuffer
	// - read a chunk of data out of the preBuffer
	// - read another chunk of data out of the preBuffer
	
	bufferSize = 1024 * 4;
	
	randomSize1 = (arc4random() % (bufferSize / 2));
	randomSize2 = (arc4random() % (bufferSize / 2));
	
	// Run benchmarks (on different runloop cycles to be fair)
	
	[self performSelector:@selector(benchmark_mutableData) withObject:nil afterDelay:2.0];
	[self performSelector:@selector(benchmark_preBuffer)   withObject:nil afterDelay:4.0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Unit Tests
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

+ (void)test_preBuffer
{
	GCDAsyncSocketPreBuffer *preBuffer = [[GCDAsyncSocketPreBuffer alloc] initWithCapacity:1024];
	
	// Test capacity, and initial size methods
	
	size_t capacity = [preBuffer availableSpace];

	NSAssert([preBuffer availableSpace] >= 1024, @"1A");
	NSAssert([preBuffer availableBytes] == 0, @"1B");
	
	// Test write pointer
	
	uint8_t *writePointer1;
	uint8_t *writePointer2;
	
	writePointer1 = [preBuffer writeBuffer];
	[preBuffer didWrite:512];
	writePointer2 = [preBuffer writeBuffer];

	NSAssert(writePointer2 - writePointer1 == 512, @"2A");
	NSAssert([preBuffer availableBytes] == 512, @"2B");
	
	// Test read pointer
	
	uint8_t *readPointer1;
	uint8_t *readPointer2;
	
	readPointer1 = [preBuffer readBuffer];
	[preBuffer didRead:256];
	readPointer2 = [preBuffer readBuffer];

	NSAssert(readPointer2 - readPointer1 == 256, @"3A");
	NSAssert([preBuffer availableBytes] == 256, @"3B");
	
	[preBuffer didRead:256];
	
	NSAssert([preBuffer availableBytes] == 0, @"4A");
	NSAssert([preBuffer availableSpace] == capacity, @"4B");

    // At this point, the buffer should have reset
    NSAssert([preBuffer readBuffer] == [preBuffer writeBuffer], @"4C");
    NSAssert([preBuffer availableSpace] == 1024, @"4D");

	// Test write and read
	
	char *str = "test";
	size_t strLen = strlen(str);
	
	memcpy([preBuffer writeBuffer], str, strLen);
	[preBuffer didWrite:strLen];
	
	NSAssert([preBuffer availableBytes] == strLen, @"5A");
	NSAssert(memcmp([preBuffer readBuffer], str, strLen) == 0, @"5B");
	
	// Test realloc
	
	[preBuffer ensureCapacityForWrite:(capacity * 2)];
	
	NSAssert([preBuffer availableSpace] >= (capacity * 2), @"6A");
	NSAssert([preBuffer availableBytes] == strLen, @"6B");
	NSAssert(memcmp([preBuffer readBuffer], str, strLen) == 0, @"6C");

    // Test available space
    [preBuffer reset];
    size_t availableSpace = [preBuffer availableSpace];

    // Make sure the available space is correct if we write all but 1 byte of our available space
    size_t writeCount = availableSpace - 1;
    [preBuffer didWrite:writeCount];
    NSAssert([preBuffer availableSpace] == 1, @"7A");

    // Make sure it doesn't change if we read some, but not all, of the data
    [preBuffer didRead:writeCount - 1];
    NSAssert([preBuffer availableSpace] == 1, @"7B");

	NSLog(@"%@: passed", NSStringFromSelector(_cmd));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Benchmarks
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

+ (void)benchmark_mutableData
{
	NSMutableData *data = [[NSMutableData alloc] initWithCapacity:bufferSize];
	
	void *readBuffer  = malloc(bufferSize);
	void *writeBuffer = malloc(bufferSize);
	
	SecRandomCopyBytes(kSecRandomDefault, (randomSize1+randomSize2), writeBuffer);
	
	NSDate *start = [NSDate date];
	
	int i;
	for (i = 0; i < COUNT; i++)
	{
		// Copy data into buffer.
		// Simulate reading from socket into preBuffer.
		
		[data appendBytes:writeBuffer length:(randomSize1+randomSize2)];
		
		// Read 1st chunk.
		// Simulate reading partial data out of preBuffer.
		
		memcpy(readBuffer, [data mutableBytes], randomSize1);
		[data replaceBytesInRange:NSMakeRange(0, randomSize1) withBytes:NULL length:0];
		
		// Read 2nd chunk.
		// Simulate draining preBuffer.
		
		memcpy(readBuffer+randomSize1, [data mutableBytes], randomSize2);
		[data replaceBytesInRange:NSMakeRange(0, randomSize2) withBytes:NULL length:0];
	}
	
	NSTimeInterval elapsed = [start timeIntervalSinceNow] * -1.0;
	NSLog(@"%@: elapsed = %.6f", NSStringFromSelector(_cmd), elapsed);
	
	free(readBuffer);
	free(writeBuffer);
}

+ (void)benchmark_preBuffer
{
	GCDAsyncSocketPreBuffer *preBuffer = [[GCDAsyncSocketPreBuffer alloc] initWithCapacity:bufferSize];
	
	void *readBuffer  = malloc(bufferSize);
	void *writeBuffer = malloc(bufferSize);
	
	SecRandomCopyBytes(kSecRandomDefault, (randomSize1+randomSize2), writeBuffer);
	
	NSDate *start = [NSDate date];
	
	int i;
	for (i = 0; i < COUNT; i++)
	{
		// Copy data into buffer.
		// Simulate reading from socket into preBuffer.
		
		memcpy([preBuffer writeBuffer], writeBuffer, randomSize1+randomSize2);
		[preBuffer didWrite:(randomSize1+randomSize2)];
		
		// Read 1st chunk.
		// Simulate reading partial data out of preBuffer.
		
		memcpy(readBuffer, [preBuffer readBuffer], randomSize1);
		[preBuffer didRead:randomSize1];
		
		// Read 2nd chunk.
		// Simulate draining preBuffer.
		
		memcpy(readBuffer+randomSize1, [preBuffer readBuffer], randomSize2);
		[preBuffer didRead:randomSize2];
	}
	
	NSTimeInterval elapsed = [start timeIntervalSinceNow] * -1.0;
	NSLog(@"%@  : elapsed = %.6f", NSStringFromSelector(_cmd), elapsed);
	
	free(readBuffer);
	free(writeBuffer);
}

@end

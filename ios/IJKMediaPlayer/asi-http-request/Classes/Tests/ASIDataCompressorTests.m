//
//  ASIDataCompressorTests.m
//  Mac
//
//  Created by Ben Copsey on 17/08/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//
// Sadly these tests only work on Mac because of the dependency on NSTask, but I'm fairly sure this class should behave in the same way on iOS

#import "ASIDataCompressorTests.h"
#import "ASIDataCompressor.h"
#import "ASIDataDecompressor.h"
#import "ASIHTTPRequest.h"

@implementation ASIDataCompressorTests

- (void)setUp
{
	NSFileManager *fileManager = [[[NSFileManager alloc] init] autorelease];

	// Download a 1.7MB text file
	NSString *filePath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"story.txt"];
	if (![fileManager fileExistsAtPath:filePath] || [[[fileManager attributesOfItemAtPath:filePath error:NULL] objectForKey:NSFileSize] unsignedLongLongValue] < 1693961) {
		ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/the_hound_of_the_baskervilles.text"]];
		[request setDownloadDestinationPath:[[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"story.txt"]];
		[request startSynchronous];
	}
}

- (void)testInflateData
{

	NSString *originalString = [NSString stringWithContentsOfFile:[[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"story.txt"] encoding:NSUTF8StringEncoding error:NULL];
	
	// Test in-memory inflate using uncompressData:error:
	NSError *error = nil;
	NSString *filePath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"uncompressed_file.txt"];
	NSString *gzippedFilePath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"uncompressed_file.txt.gz"];
	[ASIHTTPRequest removeFileAtPath:gzippedFilePath error:&error];
	if (error) {
		GHFail(@"Failed to remove old file, cannot proceed with test");
	}	
	[originalString writeToFile:filePath atomically:NO encoding:NSUTF8StringEncoding error:&error];
	if (error) {
		GHFail(@"Failed to write string, cannot proceed with test");
	}
	
	NSTask *task = [[[NSTask alloc] init] autorelease];
	[task setLaunchPath:@"/usr/bin/gzip"];
	[task setArguments:[NSArray arrayWithObject:filePath]];
	[task launch];
	[task waitUntilExit];
	
	NSData *deflatedData = [NSData dataWithContentsOfFile:gzippedFilePath];
	
	NSData *inflatedData = [ASIDataDecompressor uncompressData:deflatedData error:&error];
	if (error) {
		GHFail(@"Inflate failed because %@",error);
	}
	
	NSString *inflatedString = [[[NSString alloc] initWithBytes:[inflatedData bytes] length:[inflatedData length] encoding:NSUTF8StringEncoding] autorelease];

	
	BOOL success = [inflatedString isEqualToString:originalString];
	GHAssertTrue(success,@"inflated data is not the same as original");
	
	// Test file to file inflate
	NSString *inflatedFilePath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"inflated_file.txt"];
	[ASIHTTPRequest removeFileAtPath:inflatedFilePath error:&error];
	if (error) {
		GHFail(@"Failed to remove old file, cannot proceed with test");
	}
	
	if (![ASIDataDecompressor uncompressDataFromFile:gzippedFilePath toFile:inflatedFilePath error:&error]) {
		GHFail(@"Inflate failed because %@",error);
	}
	
	originalString = [NSString stringWithContentsOfFile:inflatedFilePath encoding:NSUTF8StringEncoding error:&error];
	if (error) {
		GHFail(@"Failed to read the inflated data, cannot proceed with test");
	}	
	
	success = [inflatedString isEqualToString:originalString];
	GHAssertTrue(success,@"inflated data is not the same as original");
	
}

- (void)testDeflateData
{

	NSString *originalString = [NSString stringWithContentsOfFile:[[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"story.txt"] encoding:NSUTF8StringEncoding error:NULL];
	
	// Test in-memory deflate using compressData:error:
	NSError *error = nil;
	NSData *deflatedData = [ASIDataCompressor compressData:[originalString dataUsingEncoding:NSUTF8StringEncoding] error:&error];
	if (error) {
		GHFail(@"Failed to deflate the data");
	}
	
	NSString *gzippedFilePath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"uncompressed_file.txt.gz"];
	[ASIHTTPRequest removeFileAtPath:gzippedFilePath error:&error];
	if (error) {
		GHFail(@"Failed to remove old file, cannot proceed with test");
	}	
	
	[deflatedData writeToFile:gzippedFilePath options:0 error:&error];
	if (error) {
		GHFail(@"Failed to write data, cannot proceed with test");
	}
	
	NSString *filePath = [[self filePathForTemporaryTestFiles] stringByAppendingPathComponent:@"uncompressed_file.txt"];
	[ASIHTTPRequest removeFileAtPath:filePath error:&error];
	if (error) {
		GHFail(@"Failed to remove old file, cannot proceed with test");
	}
	
	NSTask *task = [[[NSTask alloc] init] autorelease];
	[task setLaunchPath:@"/usr/bin/gzip"];
	[task setArguments:[NSArray arrayWithObjects:@"-d",gzippedFilePath,nil]];
	[task launch];
	[task waitUntilExit];
	
	NSString *inflatedString = [NSString stringWithContentsOfFile:filePath encoding:NSUTF8StringEncoding error:&error];
	if (error) {
		GHFail(@"Failed to read the inflated data, cannot proceed with test");
	}	
	
	BOOL success = [inflatedString isEqualToString:originalString];
	GHAssertTrue(success,@"inflated data is not the same as original");
	
	
	// Test file to file deflate
	[ASIHTTPRequest removeFileAtPath:gzippedFilePath error:&error];
	
	if (![ASIDataCompressor compressDataFromFile:filePath toFile:gzippedFilePath error:&error]) {
		GHFail(@"Deflate failed because %@",error);
	}
	[ASIHTTPRequest removeFileAtPath:filePath error:&error];
	
	task = [[[NSTask alloc] init] autorelease];
	[task setLaunchPath:@"/usr/bin/gzip"];
	[task setArguments:[NSArray arrayWithObjects:@"-d",gzippedFilePath,nil]];
	[task launch];
	[task waitUntilExit];
	
	inflatedString = [NSString stringWithContentsOfFile:filePath encoding:NSUTF8StringEncoding error:&error];
	
	success = ([inflatedString isEqualToString:originalString]);
	GHAssertTrue(success,@"deflate data is not the same as that generated by gzip");

	// Test for bug https://github.com/pokeb/asi-http-request/issues/147
	[ASIHTTPRequest removeFileAtPath:gzippedFilePath error:&error];
	[ASIHTTPRequest removeFileAtPath:filePath error:&error];

	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://spaceharvest.com/i/screen6.png"]];
	[request setDownloadDestinationPath:filePath];
	[request startSynchronous];

	if (![ASIDataCompressor compressDataFromFile:filePath toFile:gzippedFilePath error:&error]) {
		GHFail(@"Deflate failed because %@",error);
	}

	unsigned long long originalFileSize = [[[NSFileManager defaultManager] attributesOfItemAtPath:filePath error:&error] fileSize];
	[ASIHTTPRequest removeFileAtPath:filePath error:&error];

	task = [[[NSTask alloc] init] autorelease];
	[task setLaunchPath:@"/usr/bin/gzip"];
	[task setArguments:[NSArray arrayWithObjects:@"-d",gzippedFilePath,nil]];
	[task launch];
	[task waitUntilExit];

	unsigned long long inflatedFileSize = [[[NSFileManager defaultManager] attributesOfItemAtPath:filePath error:&error] fileSize];

	success = (originalFileSize == inflatedFileSize);
	GHAssertTrue(success,@"inflated data is not the same size as the original");

}

@end

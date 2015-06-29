//
//  GHUnitTestMain.m
//  GHUnit
//
//  Created by Gabriel Handford on 2/22/09.
//  Copyright 2009. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person
//  obtaining a copy of this software and associated documentation
//  files (the "Software"), to deal in the Software without
//  restriction, including without limitation the rights to use,
//  copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following
//  conditions:
//
//  The above copyright notice and this permission notice shall be
//  included in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
//  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//  OTHER DEALINGS IN THE SOFTWARE.
//

#import <Foundation/Foundation.h>

#import <GHUnit/GHUnit.h>
#import <GHUnit/GHTestApp.h>
#import <GHUnit/GHTesting.h>

// Default exception handler
void exceptionHandler(NSException *exception) {	
	NSLog(@"%@\n%@", [exception reason], GHUStackTraceFromException(exception));
}

int main(int argc, char *argv[]) {

	/*!
	 For debugging:
	 Go into the "Get Info" contextual menu of your (test) executable (inside the "Executables" group in the left panel of XCode). 
	 Then go in the "Arguments" tab. You can add the following environment variables:
	 
	 Default:   Set to:
	 NSDebugEnabled                        NO       "YES"
	 NSZombieEnabled                       NO       "YES"
	 NSDeallocateZombies                   NO       "YES"
	 NSHangOnUncaughtException             NO       "YES"
	 
	 NSEnableAutoreleasePool              YES       "NO"
	 NSAutoreleaseFreedObjectCheckEnabled  NO       "YES"
	 NSAutoreleaseHighWaterMark             0       non-negative integer
	 NSAutoreleaseHighWaterResolution       0       non-negative integer
	 
	 For info on these varaiables see NSDebug.h; http://theshadow.uw.hu/iPhoneSDKdoc/Foundation.framework/NSDebug.h.html
	 
	 For malloc debugging see: http://developer.apple.com/mac/library/documentation/Performance/Conceptual/ManagingMemory/Articles/MallocDebug.html
	 */
	
	NSSetUncaughtExceptionHandler(&exceptionHandler);
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	// Register any special test case classes
	//[[GHTesting sharedInstance] registerClassName:@"GHSpecialTestCase"];	
	
	int retVal = 0;
	// If GHUNIT_CLI is set we are using the command line interface and run the tests
	// Otherwise load the GUI app
	if (getenv("GHUNIT_CLI")) {		
		retVal = [GHTestRunner run];
	} else {
		// To run all tests (from ENV)
		GHTestApp *app = [[GHTestApp alloc] init];
		// To run a different test suite:
		//GHTestSuite *suite = [GHTestSuite suiteWithTestFilter:@"GHSlowTest,GHAsyncTestCaseTest"];
		//GHTestApp *app = [[GHTestApp alloc] initWithSuite:suite];
		// Or set global:
		//GHUnitTest = @"GHSlowTest";
		[NSApp run];
		[app release];		
	}
	[pool release];
	return retVal;
}

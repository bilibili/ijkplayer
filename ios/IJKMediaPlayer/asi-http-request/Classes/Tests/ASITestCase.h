//
//  ASITestCase.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 26/07/2009.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//

#import <Foundation/Foundation.h>

#if TARGET_OS_IPHONE
#import <GHUnitIOS/GHUnit.h>
#else
#import <GHUnit/GHUnit.h>
#endif

@interface ASITestCase : GHTestCase {
}
- (NSString *)filePathForTemporaryTestFiles;
@end

//
//  iPhoneSampleAppDelegate.m
//  Part of the ASIHTTPRequest sample project - see http://allseeing-i.com/ASIHTTPRequest for details
//
//  Created by Ben Copsey on 07/11/2008.
//  Copyright All-Seeing Interactive 2008. All rights reserved.
//

#import "iPhoneSampleAppDelegate.h"

@implementation iPhoneSampleAppDelegate

- (void)applicationDidFinishLaunching:(UIApplication *)application
{
	[[tabBarController view] setFrame:CGRectMake(0, 0, 320, 480)];
	[window addSubview:[tabBarController view]];
}

- (void)dealloc
{
    [window release];
    [super dealloc];
}

@synthesize window;

@end


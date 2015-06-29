#import "AppDelegate.h"
#import "TestPreBuffer.h"


@implementation AppDelegate

@synthesize window = _window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	[TestPreBuffer start];
}

@end

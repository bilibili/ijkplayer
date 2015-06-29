#import "AppDelegate.h"
#import "ViewController.h"
#import "TestPreBuffer.h"


@implementation AppDelegate

@synthesize window;
@synthesize viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	[TestPreBuffer start];
	
	// Normal UI stuff
	
	window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
		viewController = [[ViewController alloc] initWithNibName:@"ViewController_iPhone" bundle:nil];
	} else {
		viewController = [[ViewController alloc] initWithNibName:@"ViewController_iPad" bundle:nil];
	}
	window.rootViewController = self.viewController;
	[window makeKeyAndVisible];
	return YES;
}

@end

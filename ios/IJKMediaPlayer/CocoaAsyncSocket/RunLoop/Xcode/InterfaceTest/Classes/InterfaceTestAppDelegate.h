#import <UIKit/UIKit.h>

@class InterfaceTestViewController;
@class AsyncSocket;


@interface InterfaceTestAppDelegate : NSObject <UIApplicationDelegate>
{
	CFHostRef host;
	AsyncSocket *asyncSocket;
	
	UIWindow *window;
	InterfaceTestViewController *viewController;
}

@property (nonatomic) IBOutlet UIWindow *window;
@property (nonatomic) IBOutlet InterfaceTestViewController *viewController;

@end


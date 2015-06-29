#import <UIKit/UIKit.h>

@class ConnectTestViewController;
@class GCDAsyncSocket;


@interface ConnectTestAppDelegate : NSObject <UIApplicationDelegate>
{
	GCDAsyncSocket *asyncSocket;
}

@property (nonatomic, strong) IBOutlet UIWindow *window;
@property (nonatomic, strong) IBOutlet ConnectTestViewController *viewController;

@end

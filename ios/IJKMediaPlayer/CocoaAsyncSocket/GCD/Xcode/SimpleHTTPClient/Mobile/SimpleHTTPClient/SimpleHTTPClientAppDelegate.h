#import <UIKit/UIKit.h>

@class SimpleHTTPClientViewController;
@class GCDAsyncSocket;


@interface SimpleHTTPClientAppDelegate : NSObject <UIApplicationDelegate>
{
	GCDAsyncSocket *asyncSocket;
}

@property (nonatomic) IBOutlet UIWindow *window;
@property (nonatomic) IBOutlet SimpleHTTPClientViewController *viewController;

@end

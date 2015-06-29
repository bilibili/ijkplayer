#import <Cocoa/Cocoa.h>

@class GCDAsyncSocket;


@interface ConnectTestAppDelegate : NSObject <NSApplicationDelegate> {
@private
	GCDAsyncSocket *asyncSocket;
	
	NSWindow *__unsafe_unretained window;
}

@property (unsafe_unretained) IBOutlet NSWindow *window;

@end

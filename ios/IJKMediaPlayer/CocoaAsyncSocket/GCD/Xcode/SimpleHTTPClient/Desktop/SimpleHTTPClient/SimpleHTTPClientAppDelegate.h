#import <Cocoa/Cocoa.h>

@class GCDAsyncSocket;


@interface SimpleHTTPClientAppDelegate : NSObject <NSApplicationDelegate> {
@private
	GCDAsyncSocket *asyncSocket;
	
	NSWindow *__unsafe_unretained window;
}

@property (unsafe_unretained) IBOutlet NSWindow *window;

@end

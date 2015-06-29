#import <Cocoa/Cocoa.h>

@class GCDAsyncSocket;


@interface BonjourServerAppDelegate : NSObject <NSApplicationDelegate, NSNetServiceDelegate>
{
	NSNetService *netService;
	GCDAsyncSocket *asyncSocket;
	NSMutableArray *connectedSockets;
	
	NSWindow *__unsafe_unretained window;
}

@property (unsafe_unretained) IBOutlet NSWindow *window;

@end

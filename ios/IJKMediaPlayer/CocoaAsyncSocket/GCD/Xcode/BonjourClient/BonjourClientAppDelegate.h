#import <Cocoa/Cocoa.h>

@class GCDAsyncSocket;


@interface BonjourClientAppDelegate : NSObject <NSApplicationDelegate, NSNetServiceBrowserDelegate, NSNetServiceDelegate>
{
	NSNetServiceBrowser *netServiceBrowser;
	NSNetService *serverService;
	NSMutableArray *serverAddresses;
	GCDAsyncSocket *asyncSocket;
	BOOL connected;
	
	NSWindow *__unsafe_unretained window;
}

@property (unsafe_unretained) IBOutlet NSWindow *window;

@end

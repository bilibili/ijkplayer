#import <Cocoa/Cocoa.h>

@class GCDAsyncSocket;


@interface EchoServerAppDelegate : NSObject <NSApplicationDelegate>
{
	dispatch_queue_t socketQueue;
	
	GCDAsyncSocket *listenSocket;
	NSMutableArray *connectedSockets;
	
	BOOL isRunning;
	
	IBOutlet id logView;
	IBOutlet id portField;
	IBOutlet id startStopButton;
	
	NSWindow *__unsafe_unretained window;
}

@property (unsafe_unretained) IBOutlet NSWindow *window;

- (IBAction)startStop:(id)sender;

@end

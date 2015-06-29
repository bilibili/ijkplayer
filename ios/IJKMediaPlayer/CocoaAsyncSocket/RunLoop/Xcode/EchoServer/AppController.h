#import <Cocoa/Cocoa.h>

@class AsyncSocket;

@interface AppController : NSObject
{
	AsyncSocket *listenSocket;
	NSMutableArray *connectedSockets;
	
	BOOL isRunning;
	
    IBOutlet id logView;
    IBOutlet id portField;
    IBOutlet id startStopButton;
}
- (IBAction)startStop:(id)sender;
@end

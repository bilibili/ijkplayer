#import <Cocoa/Cocoa.h>
#import "AsyncUdpSocket.h"


@interface AppDelegate : NSObject <NSApplicationDelegate>
{
	AsyncUdpSocket *udpSocket;
	BOOL isRunning;
}

@property (unsafe_unretained) IBOutlet NSWindow *window;

@property  IBOutlet NSTextField *portField;
@property  IBOutlet NSButton *startStopButton;
@property  IBOutlet NSTextView *logView;

- (IBAction)startStopButtonPressed:(id)sender;

@end

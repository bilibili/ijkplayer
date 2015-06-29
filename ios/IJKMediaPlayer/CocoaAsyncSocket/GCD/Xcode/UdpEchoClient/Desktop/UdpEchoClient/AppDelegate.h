#import <Cocoa/Cocoa.h>

@class GCDAsyncUdpSocket;


@interface AppDelegate : NSObject <NSApplicationDelegate>
{
	long tag;
	GCDAsyncUdpSocket *udpSocket;
}

@property (unsafe_unretained) IBOutlet NSWindow    * window;
@property  IBOutlet NSTextField * addrField;
@property  IBOutlet NSTextField * portField;
@property  IBOutlet NSTextField * messageField;
@property  IBOutlet NSButton    * sendButton;
@property  IBOutlet NSTextView  * logView;

- (IBAction)send:(id)sender;

@end

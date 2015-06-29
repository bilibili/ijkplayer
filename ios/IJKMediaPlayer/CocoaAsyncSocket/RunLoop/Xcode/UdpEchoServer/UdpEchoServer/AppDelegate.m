#import "AppDelegate.h"

#define FORMAT(format, ...) [NSString stringWithFormat:(format), ##__VA_ARGS__]


@implementation AppDelegate

@synthesize window = _window;
@synthesize portField;
@synthesize startStopButton;
@synthesize logView;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	udpSocket = [[AsyncUdpSocket alloc] initWithDelegate:self];
}

- (void)awakeFromNib
{
	[logView setEnabledTextCheckingTypes:0];
	[logView setAutomaticSpellingCorrectionEnabled:NO];
}

- (void)scrollToBottom
{
	NSScrollView *scrollView = [logView enclosingScrollView];
	NSPoint newScrollOrigin;
	
	if ([[scrollView documentView] isFlipped])
		newScrollOrigin = NSMakePoint(0.0F, NSMaxY([[scrollView documentView] frame]));
	else
		newScrollOrigin = NSMakePoint(0.0F, 0.0F);
	
	[[scrollView documentView] scrollPoint:newScrollOrigin];
}

- (void)logError:(NSString *)msg
{
	NSString *paragraph = [NSString stringWithFormat:@"%@\n", msg];
	
	NSMutableDictionary *attributes = [NSMutableDictionary dictionaryWithCapacity:1];
	[attributes setObject:[NSColor redColor] forKey:NSForegroundColorAttributeName];
	
	NSAttributedString *as = [[NSAttributedString alloc] initWithString:paragraph attributes:attributes];
	
	[[logView textStorage] appendAttributedString:as];
	[self scrollToBottom];
}

- (void)logInfo:(NSString *)msg
{
	NSString *paragraph = [NSString stringWithFormat:@"%@\n", msg];
	
	NSMutableDictionary *attributes = [NSMutableDictionary dictionaryWithCapacity:1];
	[attributes setObject:[NSColor purpleColor] forKey:NSForegroundColorAttributeName];
	
	NSAttributedString *as = [[NSAttributedString alloc] initWithString:paragraph attributes:attributes];
	
	[[logView textStorage] appendAttributedString:as];
	[self scrollToBottom];
}

- (void)logMessage:(NSString *)msg
{
	NSString *paragraph = [NSString stringWithFormat:@"%@\n", msg];
	
	NSMutableDictionary *attributes = [NSMutableDictionary dictionaryWithCapacity:1];
	[attributes setObject:[NSColor blackColor] forKey:NSForegroundColorAttributeName];
	
	NSAttributedString *as = [[NSAttributedString alloc] initWithString:paragraph attributes:attributes];
	
	[[logView textStorage] appendAttributedString:as];
	[self scrollToBottom];
}

- (IBAction)startStopButtonPressed:(id)sender
{
	if (isRunning)
	{
		// STOP udp echo server
		
		[udpSocket close];
		
		[self logInfo:@"Stopped Udp Echo server"];
		isRunning = false;
		
		[portField setEnabled:YES];
		[startStopButton setTitle:@"Start"];
	}
	else
	{
		// START udp echo server
		
		int port = [portField intValue];
		if (port < 0 || port > 65535)
		{
			[portField setStringValue:@""];
			port = 0;
		}
		
		NSError *error = nil;
		
		if (![udpSocket bindToPort:port error:&error])
		{
			[self logError:FORMAT(@"Error starting server (bind): %@", error)];
			return;
		}
		
		[udpSocket receiveWithTimeout:-1 tag:0];
		
		[self logInfo:FORMAT(@"Udp Echo server started on port %hu", [udpSocket localPort])];
		isRunning = YES;
		
		[portField setEnabled:NO];
		[startStopButton setTitle:@"Stop"];
	}
}

- (BOOL)onUdpSocket:(AsyncUdpSocket *)sock
     didReceiveData:(NSData *)data
            withTag:(long)tag
           fromHost:(NSString *)host
               port:(UInt16)port
{
	NSString *msg = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
	if (msg)
	{
		[self logMessage:msg];
	}
	else
	{
		[self logError:@"Error converting received data into UTF-8 String"];
	}

	[udpSocket sendData:data toHost:host port:port withTimeout:-1 tag:0];
	[udpSocket receiveWithTimeout:-1 tag:0];

	return YES;
}

@end

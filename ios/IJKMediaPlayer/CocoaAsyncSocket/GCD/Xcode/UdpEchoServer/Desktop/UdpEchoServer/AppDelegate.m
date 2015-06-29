#import "AppDelegate.h"
#import "DDLog.h"
#import "DDTTYLogger.h"

// Log levels: off, error, warn, info, verbose
static const int ddLogLevel = LOG_LEVEL_VERBOSE;

#define FORMAT(format, ...) [NSString stringWithFormat:(format), ##__VA_ARGS__]


@implementation AppDelegate

@synthesize window = _window;
@synthesize portField;
@synthesize startStopButton;
@synthesize logView;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	// Setup our logging framework.
	
	[DDLog addLogger:[DDTTYLogger sharedInstance]];
	
	// Setup our socket.
	// The socket will invoke our delegate methods using the usual delegate paradigm.
	// However, it will invoke the delegate methods on a specified GCD delegate dispatch queue.
	// 
	// Now we can configure the delegate dispatch queues however we want.
	// We could simply use the main dispatch queue, so the delegate methods are invoked on the main thread.
	// Or we could use a dedicated dispatch queue, which could be helpful if we were doing a lot of processing.
	// 
	// The best approach for your application will depend upon convenience, requirements and performance.
	// 
	// For this simple example, we're just going to use the main thread.
	
	udpSocket = [[GCDAsyncUdpSocket alloc] initWithDelegate:self delegateQueue:dispatch_get_main_queue()];
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
		if (![udpSocket beginReceiving:&error])
		{
			[udpSocket close];
			
			[self logError:FORMAT(@"Error starting server (recv): %@", error)];
			return;
		}
		
		[self logInfo:FORMAT(@"Udp Echo server started on port %hu", [udpSocket localPort])];
		isRunning = YES;
		
		[portField setEnabled:NO];
		[startStopButton setTitle:@"Stop"];
	}
}

- (void)udpSocket:(GCDAsyncUdpSocket *)sock didReceiveData:(NSData *)data
                                               fromAddress:(NSData *)address
                                         withFilterContext:(id)filterContext
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
	
	[udpSocket sendData:data toAddress:address withTimeout:-1 tag:0];
}

@end

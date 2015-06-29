#import "BonjourClientAppDelegate.h"
#import "GCDAsyncSocket.h"
#import "DDLog.h"
#import "DDTTYLogger.h"
#import "DDASLLogger.h"

// Log levels: off, error, warn, info, verbose
static const int ddLogLevel = LOG_LEVEL_VERBOSE;

@interface BonjourClientAppDelegate (Private)
- (void)connectToNextAddress;
@end

#pragma mark -

@implementation BonjourClientAppDelegate

@synthesize window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	// Configure logging framework
	
	[DDLog addLogger:[DDTTYLogger sharedInstance]];
	[DDLog addLogger:[DDASLLogger sharedInstance]];
	
	// Start browsing for bonjour services
	
	netServiceBrowser = [[NSNetServiceBrowser alloc] init];
	
	[netServiceBrowser setDelegate:self];
	[netServiceBrowser searchForServicesOfType:@"_YourServiceName._tcp." inDomain:@"local."];
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)sender didNotSearch:(NSDictionary *)errorInfo
{
	DDLogError(@"DidNotSearch: %@", errorInfo);
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)sender
           didFindService:(NSNetService *)netService
               moreComing:(BOOL)moreServicesComing
{
	DDLogVerbose(@"DidFindService: %@", [netService name]);
	
	// Connect to the first service we find
	
	if (serverService == nil)
	{
		DDLogVerbose(@"Resolving...");
		
		serverService = netService;
		
		[serverService setDelegate:self];
		[serverService resolveWithTimeout:5.0];
	}
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)sender
         didRemoveService:(NSNetService *)netService
               moreComing:(BOOL)moreServicesComing
{
	DDLogVerbose(@"DidRemoveService: %@", [netService name]);
}

- (void)netServiceBrowserDidStopSearch:(NSNetServiceBrowser *)sender
{
	DDLogInfo(@"DidStopSearch");
}

- (void)netService:(NSNetService *)sender didNotResolve:(NSDictionary *)errorDict
{
	DDLogError(@"DidNotResolve");
}

- (void)netServiceDidResolveAddress:(NSNetService *)sender
{
	DDLogInfo(@"DidResolve: %@", [sender addresses]);
	
	if (serverAddresses == nil)
	{
		serverAddresses = [[sender addresses] mutableCopy];
	}
	
	if (asyncSocket == nil)
	{
		asyncSocket = [[GCDAsyncSocket alloc] initWithDelegate:self delegateQueue:dispatch_get_main_queue()];
		
		[self connectToNextAddress];
	}
}

- (void)connectToNextAddress
{
	BOOL done = NO;
	
	while (!done && ([serverAddresses count] > 0))
	{
		NSData *addr;
		
		// Note: The serverAddresses array probably contains both IPv4 and IPv6 addresses.
		// 
		// If your server is also using GCDAsyncSocket then you don't have to worry about it,
		// as the socket automatically handles both protocols for you transparently.
		
		if (YES) // Iterate forwards
		{
			addr = [serverAddresses objectAtIndex:0];
			[serverAddresses removeObjectAtIndex:0];
		}
		else // Iterate backwards
		{
			addr = [serverAddresses lastObject];
			[serverAddresses removeLastObject];
		}
		
		DDLogVerbose(@"Attempting connection to %@", addr);
		
		NSError *err = nil;
		if ([asyncSocket connectToAddress:addr error:&err])
		{
			done = YES;
		}
		else
		{
			DDLogWarn(@"Unable to connect: %@", err);
		}
		
	}
	
	if (!done)
	{
		DDLogWarn(@"Unable to connect to any resolved address");
	}
}

- (void)socket:(GCDAsyncSocket *)sock didConnectToHost:(NSString *)host port:(UInt16)port
{
	DDLogInfo(@"Socket:DidConnectToHost: %@ Port: %hu", host, port);
	
	connected = YES;
}

- (void)socketDidDisconnect:(GCDAsyncSocket *)sock withError:(NSError *)err
{
	DDLogWarn(@"SocketDidDisconnect:WithError: %@", err);
	
	if (!connected)
	{
		[self connectToNextAddress];
	}
}

@end

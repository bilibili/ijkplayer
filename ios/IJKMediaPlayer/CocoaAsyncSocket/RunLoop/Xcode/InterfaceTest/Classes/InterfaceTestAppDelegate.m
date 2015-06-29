#import "InterfaceTestAppDelegate.h"
#import "InterfaceTestViewController.h"
#import "AsyncSocket.h"

#import <arpa/inet.h>
#import <net/if.h>
#import <ifaddrs.h>


@implementation InterfaceTestAppDelegate

@synthesize window;
@synthesize viewController;

- (void)listInterfaces
{
	NSLog(@"listInterfaces");
	
	struct ifaddrs *addrs;
	const struct ifaddrs *cursor;
	
	if ((getifaddrs(&addrs) == 0))
	{
		cursor = addrs;
		while (cursor != NULL)
		{
			NSString *name = [NSString stringWithUTF8String:cursor->ifa_name];
			NSLog(@"%@", name);
			
			cursor = cursor->ifa_next;
		}
		freeifaddrs(addrs);
	}
}

- (NSData *)wifiAddress
{
	// On iPhone, WiFi is always "en0"
	
	NSData *result = nil;
	
	struct ifaddrs *addrs;
	const struct ifaddrs *cursor;
	
	if ((getifaddrs(&addrs) == 0))
	{
		cursor = addrs;
		while (cursor != NULL)
		{
			NSLog(@"cursor->ifa_name = %s", cursor->ifa_name);
			
			if (strcmp(cursor->ifa_name, "en0") == 0)
			{
				if (cursor->ifa_addr->sa_family == AF_INET)
				{
					struct sockaddr_in *addr = (struct sockaddr_in *)cursor->ifa_addr;
					NSLog(@"cursor->ifa_addr = %s", inet_ntoa(addr->sin_addr));
					
					result = [NSData dataWithBytes:addr length:sizeof(struct sockaddr_in)];
					cursor = NULL;
				}
				else
				{
					cursor = cursor->ifa_next;
				}
			}
			else
			{
				cursor = cursor->ifa_next;
			}
		}
		freeifaddrs(addrs);
	}
	
	return result;
}

- (NSData *)cellAddress
{
	// On iPhone, 3G is "pdp_ipX", where X is usually 0, but may possibly be 0-3 (i'm guessing...)
	
	NSData *result = nil;
	
	struct ifaddrs *addrs;
	const struct ifaddrs *cursor;
	
	if ((getifaddrs(&addrs) == 0))
	{
		cursor = addrs;
		while (cursor != NULL)
		{
			NSLog(@"cursor->ifa_name = %s", cursor->ifa_name);
			
			if (strncmp(cursor->ifa_name, "pdp_ip", 6) == 0)
			{
				if (cursor->ifa_addr->sa_family == AF_INET)
				{
					struct sockaddr_in *addr = (struct sockaddr_in *)cursor->ifa_addr;
					NSLog(@"cursor->ifa_addr = %s", inet_ntoa(addr->sin_addr));
					
					result = [NSData dataWithBytes:addr length:sizeof(struct sockaddr_in)];
					cursor = NULL;
				}
				else
				{
					cursor = cursor->ifa_next;
				}
			}
			else
			{
				cursor = cursor->ifa_next;
			}
		}
		freeifaddrs(addrs);
	}
	
	return result;
}

- (void)dnsResolveDidFinish
{
	NSLog(@"dnsResolveDidFinish");
	
	Boolean hasBeenResolved;
	CFArrayRef addrs = CFHostGetAddressing(host, &hasBeenResolved);
	
	if (!hasBeenResolved)
	{
		NSLog(@"Failed to resolve!");
		return;
	}
	
	CFIndex count = CFArrayGetCount(addrs);
		
	if (count == 0)
	{
		NSLog(@"Found 0 addresses!");
		return;
	}
	
	
	struct sockaddr_in remoteAddr;
	NSData *remoteAddrData = nil;
	
	BOOL found = NO;
	CFIndex i;
	for (i = 0; i < count && !found; i++)
	{
		CFDataRef addr = CFArrayGetValueAtIndex(addrs, i);
		
		struct sockaddr *saddr = (struct sockaddr *)CFDataGetBytePtr(addr);
		if (saddr->sa_family == AF_INET)
		{
			struct sockaddr_in *saddr4 = (struct sockaddr_in *)saddr;
			
			NSLog(@"Found IPv4 version: %s", inet_ntoa(saddr4->sin_addr));
			
			memcpy(&remoteAddr, saddr, sizeof(remoteAddr));
			remoteAddr.sin_port = htons(80);
			
			remoteAddrData = [NSData dataWithBytes:&remoteAddr length:sizeof(remoteAddr)];
			
			found = YES;
		}
	}
	
	if (found == NO)
	{
		NSLog(@"Found no suitable addresses!");
		return;
	}
	
	NSData *interfaceAddrData = [self wifiAddress];
//	NSData *interfaceAddrData = [self cellAddress];
	if (interfaceAddrData == nil)
	{
		NSLog(@"Requested interface not available");
		return;
	}
	
	NSLog(@"Connecting...");
	
	asyncSocket = [[AsyncSocket alloc] initWithDelegate:self];
	
	NSError *err = nil;
	if (![asyncSocket connectToAddress:remoteAddrData viaInterfaceAddress:interfaceAddrData withTimeout:-1 error:&err])
	{
		NSLog(@"Error connecting: %@", err);
	}
}

static void DNSResolveCallBack(CFHostRef theHost, CFHostInfoType typeInfo, const CFStreamError *error, void *info)
{
	@autoreleasepool {
	
		InterfaceTestAppDelegate *instance = (__bridge InterfaceTestAppDelegate *)info;
		[instance dnsResolveDidFinish];
	
	}
}

- (void)startDNSResolve
{
	NSLog(@"startDNSResolve");
	
	NSLog(@"Resolving google.com...");
	host = CFHostCreateWithName(kCFAllocatorDefault, CFSTR("google.com"));
	if (host == NULL)
	{
		NSLog(@"wtf 1");
		return;
	}
	
	Boolean result;
	
	CFHostClientContext context;
	context.version         = 0;
	context.info            = (__bridge void *)(self);
	context.retain          = NULL;
	context.release         = NULL;
	context.copyDescription = NULL;
	
	result = CFHostSetClient(host, &DNSResolveCallBack, &context);
	if (!result)
	{
		NSLog(@"wtf 2");
		return;
	}
	
	CFHostScheduleWithRunLoop(host, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	
	CFStreamError error;
	bzero(&error, sizeof(error));
	
	result = CFHostStartInfoResolution(host, kCFHostAddresses, &error);
	if (!result)
	{
		NSLog(@"Failed to start DNS resolve");
		NSLog(@"error: domain(%i) code(%i)", (int)(error.domain), (int)(error.error));
	}
}

- (void)onSocket:(AsyncSocket *)sock didConnectToHost:(NSString *)remoteHost port:(UInt16)remotePort
{
	NSLog(@"Socket is connected!");
	
	NSLog(@"Remote Address: %@:%hu", remoteHost, remotePort);
	
	NSString *localHost = [sock localHost];
	UInt16 localPort = [sock localPort];
	
	NSLog(@"Local Address: %@:%hu", localHost, localPort);
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	NSLog(@"application:didFinishLaunchingWithOptions:");
	
    [self listInterfaces];
	[self startDNSResolve];
	
	// Add the view controller's view to the window and display.
	[window addSubview:viewController.view];
	[window makeKeyAndVisible];
	
	return YES;
}



@end

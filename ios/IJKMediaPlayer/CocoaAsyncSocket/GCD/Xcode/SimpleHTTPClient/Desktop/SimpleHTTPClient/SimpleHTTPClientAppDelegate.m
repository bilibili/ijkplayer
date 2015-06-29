#import "SimpleHTTPClientAppDelegate.h"
#import "GCDAsyncSocket.h"
#import "DDLog.h"
#import "DDTTYLogger.h"
#import "DDDispatchQueueLogFormatter.h"

// Log levels: off, error, warn, info, verbose
static const int ddLogLevel = LOG_LEVEL_VERBOSE;

#define  WWW_PORT 0  // 0 => automatic
#define  WWW_HOST @"www.amazon.com"
#define CERT_HOST @"www.amazon.com"

#define USE_SECURE_CONNECTION    1
#define MANUALLY_EVALUATE_TRUST  1

#define READ_HEADER_LINE_BY_LINE 0


@implementation SimpleHTTPClientAppDelegate

@synthesize window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	// AsyncSocket optionally uses the Lumberjack logging framework.
	// 
	// Lumberjack is a professional logging framework. It's extremely fast and flexible.
	// It also uses GCD, making it a great fit for GCDAsyncSocket.
	// 
	// As mentioned earlier, enabling logging in GCDAsyncSocket is entirely optional.
	// Doing so simply helps give you a deeper understanding of the inner workings of the library (if you care).
	// You can do so at the top of GCDAsyncSocket.m,
	// where you can also control things such as the log level,
	// and whether or not logging should be asynchronous (helps to improve speed, and
	// perfect for reducing interference with those pesky timing bugs in your code).
	// 
	// There is a massive amount of documentation on the Lumberjack project page:
	// http://code.google.com/p/cocoalumberjack/
	// 
	// But this one line is all you need to instruct Lumberjack to spit out log statements to the Xcode console.
	
	[DDLog addLogger:[DDTTYLogger sharedInstance]];
	
	// We're going to take advantage of some of Lumberjack's advanced features.
	//
	// Format log statements such that it outputs the queue/thread name.
	// As opposed to the not-so-helpful mach thread id.
	// 
	// Old : 2011-12-05 19:54:08:161 [17894:f803] Connecting...
	//       2011-12-05 19:54:08:161 [17894:11f03] GCDAsyncSocket: Dispatching DNS lookup...
	//       2011-12-05 19:54:08:161 [17894:13303] GCDAsyncSocket: Creating IPv4 socket
	// 
	// New : 2011-12-05 19:54:08:161 [main] Connecting...
	//       2011-12-05 19:54:08:161 [socket] GCDAsyncSocket: Dispatching DNS lookup...
	//       2011-12-05 19:54:08:161 [socket] GCDAsyncSocket: Creating IPv4 socket
	
	DDDispatchQueueLogFormatter *formatter = [[DDDispatchQueueLogFormatter alloc] init];
	[formatter setReplacementString:@"socket" forQueueLabel:GCDAsyncSocketQueueName];
	[formatter setReplacementString:@"socket-cf" forQueueLabel:GCDAsyncSocketThreadName];
	
	[[DDTTYLogger sharedInstance] setLogFormatter:formatter];
	
	// Start the socket stuff
	
	[self startSocket];
}

- (void)startSocket
{
	// Create our GCDAsyncSocket instance.
	// 
	// Notice that we give it the normal delegate AND a delegate queue.
	// The socket will do all of its operations in a background queue,
	// and you can tell it which thread/queue to invoke your delegate on.
	// In this case, we're just saying invoke us on the main thread.
	// But you can see how trivial it would be to create your own queue,
	// and parallelize your networking processing code by having your
	// delegate methods invoked and run on background queues.
	
	asyncSocket = [[GCDAsyncSocket alloc] initWithDelegate:self delegateQueue:dispatch_get_main_queue()];
	
	// Now we tell the ASYNCHRONOUS socket to connect.
	// 
	// Recall that GCDAsyncSocket is ... asynchronous.
	// This means when you tell the socket to connect, it will do so ... asynchronously.
	// After all, do you want your main thread to block on a slow network connection?
	// 
	// So what's with the BOOL return value, and error pointer?
	// These are for early detection of obvious problems, such as:
	// 
	// - The socket is already connected.
	// - You passed in an invalid parameter.
	// - The socket isn't configured properly.
	// 
	// The error message might be something like "Attempting to connect without a delegate. Set a delegate first."
	// 
	// When the asynchronous sockets connects, it will invoke the socket:didConnectToHost:port: delegate method.
	
	NSError *error = nil;
	
	uint16_t port = WWW_PORT;
	if (port == 0)
	{
	#if USE_SECURE_CONNECTION
		port = 443; // HTTPS
	#else
		port = 80;  // HTTP
	#endif
	}
	
	if (![asyncSocket connectToHost:WWW_HOST onPort:port error:&error])
	{
		DDLogError(@"Unable to connect to due to invalid configuration: %@", error);
	}
	else
	{
		DDLogVerbose(@"Connecting to \"%@\" on port %hu...", WWW_HOST, port);
	}
	
#if USE_SECURE_CONNECTION
	
	// The connect method above is asynchronous.
	// At this point, the connection has been initiated, but hasn't completed.
	// When the connection is established, our socket:didConnectToHost:port: delegate method will be invoked.
	// 
	// Now, for a secure connection we have to connect to the HTTPS server running on port 443.
	// The SSL/TLS protocol runs atop TCP, so after the connection is established we want to start the TLS handshake.
	// 
	// We already know this is what we want to do.
	// Wouldn't it be convenient if we could tell the socket to queue the security upgrade now instead of waiting?
	// Well in fact you can! This is part of the queued architecture of AsyncSocket.
	// 
	// After the connection has been established, AsyncSocket will look in its queue for the next task.
	// There it will find, dequeue and execute our request to start the TLS security protocol.
	// 
	// The options passed to the startTLS method are fully documented in the GCDAsyncSocket header file.
	
	#if MANUALLY_EVALUATE_TRUST
	{
		// Use socket:shouldTrustPeer: delegate method for manual trust evaluation
		
		NSDictionary *options = @{
			GCDAsyncSocketManuallyEvaluateTrust : @(YES),
		    GCDAsyncSocketSSLPeerName : CERT_HOST
		};
		
		DDLogVerbose(@"Requesting StartTLS with options:\n%@", options);
		[asyncSocket startTLS:options];
	}
	#else
	{
		// Use default trust evaluation, and provide basic security parameters
		
		NSDictionary *options = @{
		    GCDAsyncSocketSSLPeerName : CERT_HOST
		};
		
		DDLogVerbose(@"Requesting StartTLS with options:\n%@", options);
		[asyncSocket startTLS:options];
	}
	#endif
	
#endif
}

- (void)socket:(GCDAsyncSocket *)sock didConnectToHost:(NSString *)host port:(UInt16)port
{
	DDLogVerbose(@"socket:didConnectToHost:%@ port:%hu", host, port);
	
	// HTTP is a really simple protocol.
	// 
	// If you don't already know all about it, this is one of the best resources I know (short and sweet):
	// http://www.jmarshall.com/easy/http/
	// 
	// We're just going to tell the server to send us the metadata (essentially) about a particular resource.
	// The server will send an http response, and then immediately close the connection.
	
	NSString *requestStrFrmt = @"HEAD / HTTP/1.0\r\nHost: %@\r\nConnection: Close\r\n\r\n";
	
	NSString *requestStr = [NSString stringWithFormat:requestStrFrmt, WWW_HOST];
	NSData *requestData = [requestStr dataUsingEncoding:NSUTF8StringEncoding];
	
	[asyncSocket writeData:requestData withTimeout:-1.0 tag:0];
	
	DDLogInfo(@"Full httpRequest:\n%@", requestStr);
	
	// Side Note:
	// 
	// The AsyncSocket family supports queued reads and writes.
	// 
	// This means that you don't have to wait for the socket to connect before issuing your read or write commands.
	// If you do so before the socket is connected, it will simply queue the requests,
	// and process them after the socket is connected.
	// Also, you can issue multiple write commands (or read commands) at a time.
	// You don't have to wait for one write operation to complete before sending another write command.
	// 
	// The whole point is to make YOUR code easier to write, easier to read, and easier to maintain.
	// Do networking stuff when it is easiest for you, or when it makes the most sense for you.
	// AsyncSocket adapts to your schedule, not the other way around.
	
#if READ_HEADER_LINE_BY_LINE
	
	// Now we tell the socket to read the first line of the http response header.
	// As per the http protocol, we know each header line is terminated with a CRLF (carriage return, line feed).
	
	[asyncSocket readDataToData:[GCDAsyncSocket CRLFData] withTimeout:-1.0 tag:0];
	
#else
	
	// Now we tell the socket to read the full header for the http response.
	// As per the http protocol, we know the header is terminated with two CRLF's (carriage return, line feed).
	
	NSData *responseTerminatorData = [@"\r\n\r\n" dataUsingEncoding:NSASCIIStringEncoding];

	[asyncSocket readDataToData:responseTerminatorData withTimeout:-1.0 tag:0];
	
#endif
}

- (void)socket:(GCDAsyncSocket *)sock didReceiveTrust:(SecTrustRef)trust
                                    completionHandler:(void (^)(BOOL shouldTrustPeer))completionHandler
{
	DDLogVerbose(@"socket:shouldTrustPeer:");
	
	dispatch_queue_t bgQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	dispatch_async(bgQueue, ^{
		
		// This is where you would (eventually) invoke SecTrustEvaluate.
		// Presumably, if you're using manual trust evaluation, you're likely doing extra stuff here.
		// For example, allowing a specific self-signed certificate that is known to the app.
		
		SecTrustResultType result = kSecTrustResultDeny;
		OSStatus status = SecTrustEvaluate(trust, &result);
		
		if (status == noErr && (result == kSecTrustResultProceed || result == kSecTrustResultUnspecified)) {
			completionHandler(YES);
		}
		else {
			completionHandler(NO);
		}
	});
}

- (void)socketDidSecure:(GCDAsyncSocket *)sock
{
	// This method will be called if USE_SECURE_CONNECTION is set
	
	DDLogVerbose(@"socketDidSecure:");
}

- (void)socket:(GCDAsyncSocket *)sock didWriteDataWithTag:(long)tag
{
	DDLogVerbose(@"socket:didWriteDataWithTag:");
}

- (void)socket:(GCDAsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag
{
	DDLogVerbose(@"socket:didReadData:withTag:");
	
	NSString *httpResponse = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
	
#if READ_HEADER_LINE_BY_LINE
	
	DDLogInfo(@"Line httpResponse: %@", httpResponse);
	
	// As per the http protocol, we know the header is terminated with two CRLF's.
	// In other words, an empty line.
	
	if ([data length] == 2) // 2 bytes = CRLF
	{
		DDLogInfo(@"<done>");
	}
	else
	{
		// Read the next line of the header
		[asyncSocket readDataToData:[GCDAsyncSocket CRLFData] withTimeout:-1.0 tag:0];
	}
	
#else
	
	DDLogInfo(@"Full httpResponse:\n%@", httpResponse);
	
#endif
	
}

- (void)socketDidDisconnect:(GCDAsyncSocket *)sock withError:(NSError *)err
{
	// Since we requested HTTP/1.0, we expect the server to close the connection as soon as it has sent the response.
	
	DDLogVerbose(@"socketDidDisconnect:%p withError:%@", sock, err);
}

@end

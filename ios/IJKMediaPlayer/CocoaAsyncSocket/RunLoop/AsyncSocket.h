//
//  AsyncSocket.h
//  
//  This class is in the public domain.
//  Originally created by Dustin Voss on Wed Jan 29 2003.
//  Updated and maintained by Deusty Designs and the Mac development community.
//
//  http://code.google.com/p/cocoaasyncsocket/
//

#import <Foundation/Foundation.h>

@class AsyncSocket;
@class AsyncReadPacket;
@class AsyncWritePacket;

extern NSString *const AsyncSocketException;
extern NSString *const AsyncSocketErrorDomain;

enum AsyncSocketError
{
	AsyncSocketCFSocketError = kCFSocketError,	// From CFSocketError enum.
	AsyncSocketNoError = 0,						// Never used.
	AsyncSocketCanceledError,					// onSocketWillConnect: returned NO.
	AsyncSocketConnectTimeoutError,
	AsyncSocketReadMaxedOutError,               // Reached set maxLength without completing
	AsyncSocketReadTimeoutError,
	AsyncSocketWriteTimeoutError
};
typedef enum AsyncSocketError AsyncSocketError;

@protocol AsyncSocketDelegate
@optional

/**
 * In the event of an error, the socket is closed.
 * You may call "unreadData" during this call-back to get the last bit of data off the socket.
 * When connecting, this delegate method may be called
 * before"onSocket:didAcceptNewSocket:" or "onSocket:didConnectToHost:".
**/
- (void)onSocket:(AsyncSocket *)sock willDisconnectWithError:(NSError *)err;

/**
 * Called when a socket disconnects with or without error.  If you want to release a socket after it disconnects,
 * do so here. It is not safe to do that during "onSocket:willDisconnectWithError:".
 * 
 * If you call the disconnect method, and the socket wasn't already disconnected,
 * this delegate method will be called before the disconnect method returns.
**/
- (void)onSocketDidDisconnect:(AsyncSocket *)sock;

/**
 * Called when a socket accepts a connection.  Another socket is spawned to handle it. The new socket will have
 * the same delegate and will call "onSocket:didConnectToHost:port:".
**/
- (void)onSocket:(AsyncSocket *)sock didAcceptNewSocket:(AsyncSocket *)newSocket;

/**
 * Called when a new socket is spawned to handle a connection.  This method should return the run-loop of the
 * thread on which the new socket and its delegate should operate. If omitted, [NSRunLoop currentRunLoop] is used.
**/
- (NSRunLoop *)onSocket:(AsyncSocket *)sock wantsRunLoopForNewSocket:(AsyncSocket *)newSocket;

/**
 * Called when a socket is about to connect. This method should return YES to continue, or NO to abort.
 * If aborted, will result in AsyncSocketCanceledError.
 * 
 * If the connectToHost:onPort:error: method was called, the delegate will be able to access and configure the
 * CFReadStream and CFWriteStream as desired prior to connection.
 *
 * If the connectToAddress:error: method was called, the delegate will be able to access and configure the
 * CFSocket and CFSocketNativeHandle (BSD socket) as desired prior to connection. You will be able to access and
 * configure the CFReadStream and CFWriteStream in the onSocket:didConnectToHost:port: method.
**/
- (BOOL)onSocketWillConnect:(AsyncSocket *)sock;

/**
 * Called when a socket connects and is ready for reading and writing.
 * The host parameter will be an IP address, not a DNS name.
**/
- (void)onSocket:(AsyncSocket *)sock didConnectToHost:(NSString *)host port:(UInt16)port;

/**
 * Called when a socket has completed reading the requested data into memory.
 * Not called if there is an error.
**/
- (void)onSocket:(AsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag;

/**
 * Called when a socket has read in data, but has not yet completed the read.
 * This would occur if using readToData: or readToLength: methods.
 * It may be used to for things such as updating progress bars.
**/
- (void)onSocket:(AsyncSocket *)sock didReadPartialDataOfLength:(NSUInteger)partialLength tag:(long)tag;

/**
 * Called when a socket has completed writing the requested data. Not called if there is an error.
**/
- (void)onSocket:(AsyncSocket *)sock didWriteDataWithTag:(long)tag;

/**
 * Called when a socket has written some data, but has not yet completed the entire write.
 * It may be used to for things such as updating progress bars.
**/
- (void)onSocket:(AsyncSocket *)sock didWritePartialDataOfLength:(NSUInteger)partialLength tag:(long)tag;

/**
 * Called if a read operation has reached its timeout without completing.
 * This method allows you to optionally extend the timeout.
 * If you return a positive time interval (> 0) the read's timeout will be extended by the given amount.
 * If you don't implement this method, or return a non-positive time interval (<= 0) the read will timeout as usual.
 * 
 * The elapsed parameter is the sum of the original timeout, plus any additions previously added via this method.
 * The length parameter is the number of bytes that have been read so far for the read operation.
 * 
 * Note that this method may be called multiple times for a single read if you return positive numbers.
**/
- (NSTimeInterval)onSocket:(AsyncSocket *)sock
  shouldTimeoutReadWithTag:(long)tag
                   elapsed:(NSTimeInterval)elapsed
                 bytesDone:(NSUInteger)length;

/**
 * Called if a write operation has reached its timeout without completing.
 * This method allows you to optionally extend the timeout.
 * If you return a positive time interval (> 0) the write's timeout will be extended by the given amount.
 * If you don't implement this method, or return a non-positive time interval (<= 0) the write will timeout as usual.
 * 
 * The elapsed parameter is the sum of the original timeout, plus any additions previously added via this method.
 * The length parameter is the number of bytes that have been written so far for the write operation.
 * 
 * Note that this method may be called multiple times for a single write if you return positive numbers.
**/
- (NSTimeInterval)onSocket:(AsyncSocket *)sock
 shouldTimeoutWriteWithTag:(long)tag
                   elapsed:(NSTimeInterval)elapsed
                 bytesDone:(NSUInteger)length;

/**
 * Called after the socket has successfully completed SSL/TLS negotiation.
 * This method is not called unless you use the provided startTLS method.
 * 
 * If a SSL/TLS negotiation fails (invalid certificate, etc) then the socket will immediately close,
 * and the onSocket:willDisconnectWithError: delegate method will be called with the specific SSL error code.
**/
- (void)onSocketDidSecure:(AsyncSocket *)sock;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface AsyncSocket : NSObject
{
	CFSocketNativeHandle theNativeSocket4;
	CFSocketNativeHandle theNativeSocket6;
	
	CFSocketRef theSocket4;            // IPv4 accept or connect socket
	CFSocketRef theSocket6;            // IPv6 accept or connect socket
	
	CFReadStreamRef theReadStream;
	CFWriteStreamRef theWriteStream;

	CFRunLoopSourceRef theSource4;     // For theSocket4
	CFRunLoopSourceRef theSource6;     // For theSocket6
	CFRunLoopRef theRunLoop;
	CFSocketContext theContext;
	NSArray *theRunLoopModes;
	
	NSTimer *theConnectTimer;

	NSMutableArray *theReadQueue;
	AsyncReadPacket *theCurrentRead;
	NSTimer *theReadTimer;
	NSMutableData *partialReadBuffer;
	
	NSMutableArray *theWriteQueue;
	AsyncWritePacket *theCurrentWrite;
	NSTimer *theWriteTimer;

	id theDelegate;
	UInt16 theFlags;
	
	long theUserData;
}

- (id)init;
- (id)initWithDelegate:(id)delegate;
- (id)initWithDelegate:(id)delegate userData:(long)userData;

/* String representation is long but has no "\n". */
- (NSString *)description;

/**
 * Use "canSafelySetDelegate" to see if there is any pending business (reads and writes) with the current delegate
 * before changing it.  It is, of course, safe to change the delegate before connecting or accepting connections.
**/
- (id)delegate;
- (BOOL)canSafelySetDelegate;
- (void)setDelegate:(id)delegate;

/* User data can be a long, or an id or void * cast to a long. */
- (long)userData;
- (void)setUserData:(long)userData;

/* Don't use these to read or write. And don't close them either! */
- (CFSocketRef)getCFSocket;
- (CFReadStreamRef)getCFReadStream;
- (CFWriteStreamRef)getCFWriteStream;

// Once one of the accept or connect methods are called, the AsyncSocket instance is locked in
// and the other accept/connect methods can't be called without disconnecting the socket first.
// If the attempt fails or times out, these methods either return NO or
// call "onSocket:willDisconnectWithError:" and "onSockedDidDisconnect:".

// When an incoming connection is accepted, AsyncSocket invokes several delegate methods.
// These methods are (in chronological order):
// 1. onSocket:didAcceptNewSocket:
// 2. onSocket:wantsRunLoopForNewSocket:
// 3. onSocketWillConnect:
// 
// Your server code will need to retain the accepted socket (if you want to accept it).
// The best place to do this is probably in the onSocket:didAcceptNewSocket: method.
// 
// After the read and write streams have been setup for the newly accepted socket,
// the onSocket:didConnectToHost:port: method will be called on the proper run loop.
// 
// Multithreading Note: If you're going to be moving the newly accepted socket to another run
// loop by implementing onSocket:wantsRunLoopForNewSocket:, then you should wait until the
// onSocket:didConnectToHost:port: method before calling read, write, or startTLS methods.
// Otherwise read/write events are scheduled on the incorrect runloop, and chaos may ensue.

/**
 * Tells the socket to begin listening and accepting connections on the given port.
 * When a connection comes in, the AsyncSocket instance will call the various delegate methods (see above).
 * The socket will listen on all available interfaces (e.g. wifi, ethernet, etc)
**/
- (BOOL)acceptOnPort:(UInt16)port error:(NSError **)errPtr;

/**
 * This method is the same as acceptOnPort:error: with the additional option
 * of specifying which interface to listen on. So, for example, if you were writing code for a server that
 * has multiple IP addresses, you could specify which address you wanted to listen on.  Or you could use it
 * to specify that the socket should only accept connections over ethernet, and not other interfaces such as wifi.
 * You may also use the special strings "localhost" or "loopback" to specify that
 * the socket only accept connections from the local machine.
 * 
 * To accept connections on any interface pass nil, or simply use the acceptOnPort:error: method.
**/
- (BOOL)acceptOnInterface:(NSString *)interface port:(UInt16)port error:(NSError **)errPtr;

/**
 * Connects to the given host and port.
 * The host may be a domain name (e.g. "deusty.com") or an IP address string (e.g. "192.168.0.2")
**/
- (BOOL)connectToHost:(NSString *)hostname onPort:(UInt16)port error:(NSError **)errPtr;

/**
 * This method is the same as connectToHost:onPort:error: with an additional timeout option.
 * To not time out use a negative time interval, or simply use the connectToHost:onPort:error: method.
**/
- (BOOL)connectToHost:(NSString *)hostname
			   onPort:(UInt16)port
		  withTimeout:(NSTimeInterval)timeout
				error:(NSError **)errPtr;

/**
 * Connects to the given address, specified as a sockaddr structure wrapped in a NSData object.
 * For example, a NSData object returned from NSNetService's addresses method.
 * 
 * If you have an existing struct sockaddr you can convert it to a NSData object like so:
 * struct sockaddr sa  -> NSData *dsa = [NSData dataWithBytes:&remoteAddr length:remoteAddr.sa_len];
 * struct sockaddr *sa -> NSData *dsa = [NSData dataWithBytes:remoteAddr length:remoteAddr->sa_len];
**/
- (BOOL)connectToAddress:(NSData *)remoteAddr error:(NSError **)errPtr;

/**
 * This method is the same as connectToAddress:error: with an additional timeout option.
 * To not time out use a negative time interval, or simply use the connectToAddress:error: method.
**/
- (BOOL)connectToAddress:(NSData *)remoteAddr withTimeout:(NSTimeInterval)timeout error:(NSError **)errPtr;

- (BOOL)connectToAddress:(NSData *)remoteAddr
     viaInterfaceAddress:(NSData *)interfaceAddr
             withTimeout:(NSTimeInterval)timeout
                   error:(NSError **)errPtr;

/**
 * Disconnects immediately. Any pending reads or writes are dropped.
 * If the socket is not already disconnected, the onSocketDidDisconnect delegate method
 * will be called immediately, before this method returns.
 * 
 * Please note the recommended way of releasing an AsyncSocket instance (e.g. in a dealloc method)
 * [asyncSocket setDelegate:nil];
 * [asyncSocket disconnect];
 * [asyncSocket release];
**/
- (void)disconnect;

/**
 * Disconnects after all pending reads have completed.
 * After calling this, the read and write methods will do nothing.
 * The socket will disconnect even if there are still pending writes.
**/
- (void)disconnectAfterReading;

/**
 * Disconnects after all pending writes have completed.
 * After calling this, the read and write methods will do nothing.
 * The socket will disconnect even if there are still pending reads.
**/
- (void)disconnectAfterWriting;

/**
 * Disconnects after all pending reads and writes have completed.
 * After calling this, the read and write methods will do nothing.
**/
- (void)disconnectAfterReadingAndWriting;

/* Returns YES if the socket and streams are open, connected, and ready for reading and writing. */
- (BOOL)isConnected;

/**
 * Returns the local or remote host and port to which this socket is connected, or nil and 0 if not connected.
 * The host will be an IP address.
**/
- (NSString *)connectedHost;
- (UInt16)connectedPort;

- (NSString *)localHost;
- (UInt16)localPort;

/**
 * Returns the local or remote address to which this socket is connected,
 * specified as a sockaddr structure wrapped in a NSData object.
 * 
 * See also the connectedHost, connectedPort, localHost and localPort methods.
**/
- (NSData *)connectedAddress;
- (NSData *)localAddress;

/**
 * Returns whether the socket is IPv4 or IPv6.
 * An accepting socket may be both.
**/
- (BOOL)isIPv4;
- (BOOL)isIPv6;

// The readData and writeData methods won't block (they are asynchronous).
// 
// When a read is complete the onSocket:didReadData:withTag: delegate method is called.
// When a write is complete the onSocket:didWriteDataWithTag: delegate method is called.
// 
// You may optionally set a timeout for any read/write operation. (To not timeout, use a negative time interval.)
// If a read/write opertion times out, the corresponding "onSocket:shouldTimeout..." delegate method
// is called to optionally allow you to extend the timeout.
// Upon a timeout, the "onSocket:willDisconnectWithError:" method is called, followed by "onSocketDidDisconnect".
// 
// The tag is for your convenience.
// You can use it as an array index, step number, state id, pointer, etc.

/**
 * Reads the first available bytes that become available on the socket.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
**/
- (void)readDataWithTimeout:(NSTimeInterval)timeout tag:(long)tag;

/**
 * Reads the first available bytes that become available on the socket.
 * The bytes will be appended to the given byte buffer starting at the given offset.
 * The given buffer will automatically be increased in size if needed.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * If the buffer if nil, the socket will create a buffer for you.
 * 
 * If the bufferOffset is greater than the length of the given buffer,
 * the method will do nothing, and the delegate will not be called.
 * 
 * If you pass a buffer, you must not alter it in any way while AsyncSocket is using it.
 * After completion, the data returned in onSocket:didReadData:withTag: will be a subset of the given buffer.
 * That is, it will reference the bytes that were appended to the given buffer.
**/
- (void)readDataWithTimeout:(NSTimeInterval)timeout
					 buffer:(NSMutableData *)buffer
			   bufferOffset:(NSUInteger)offset
						tag:(long)tag;

/**
 * Reads the first available bytes that become available on the socket.
 * The bytes will be appended to the given byte buffer starting at the given offset.
 * The given buffer will automatically be increased in size if needed.
 * A maximum of length bytes will be read.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * If the buffer if nil, a buffer will automatically be created for you.
 * If maxLength is zero, no length restriction is enforced.
 * 
 * If the bufferOffset is greater than the length of the given buffer,
 * the method will do nothing, and the delegate will not be called.
 * 
 * If you pass a buffer, you must not alter it in any way while AsyncSocket is using it.
 * After completion, the data returned in onSocket:didReadData:withTag: will be a subset of the given buffer.
 * That is, it will reference the bytes that were appended to the given buffer.
**/
- (void)readDataWithTimeout:(NSTimeInterval)timeout
                     buffer:(NSMutableData *)buffer
               bufferOffset:(NSUInteger)offset
                  maxLength:(NSUInteger)length
                        tag:(long)tag;

/**
 * Reads the given number of bytes.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * 
 * If the length is 0, this method does nothing and the delegate is not called.
**/
- (void)readDataToLength:(NSUInteger)length withTimeout:(NSTimeInterval)timeout tag:(long)tag;

/**
 * Reads the given number of bytes.
 * The bytes will be appended to the given byte buffer starting at the given offset.
 * The given buffer will automatically be increased in size if needed.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * If the buffer if nil, a buffer will automatically be created for you.
 * 
 * If the length is 0, this method does nothing and the delegate is not called.
 * If the bufferOffset is greater than the length of the given buffer,
 * the method will do nothing, and the delegate will not be called.
 * 
 * If you pass a buffer, you must not alter it in any way while AsyncSocket is using it.
 * After completion, the data returned in onSocket:didReadData:withTag: will be a subset of the given buffer.
 * That is, it will reference the bytes that were appended to the given buffer.
**/
- (void)readDataToLength:(NSUInteger)length
             withTimeout:(NSTimeInterval)timeout
                  buffer:(NSMutableData *)buffer
            bufferOffset:(NSUInteger)offset
                     tag:(long)tag;

/**
 * Reads bytes until (and including) the passed "data" parameter, which acts as a separator.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * 
 * If you pass nil or zero-length data as the "data" parameter,
 * the method will do nothing, and the delegate will not be called.
 * 
 * To read a line from the socket, use the line separator (e.g. CRLF for HTTP, see below) as the "data" parameter.
 * Note that this method is not character-set aware, so if a separator can occur naturally as part of the encoding for
 * a character, the read will prematurely end.
**/
- (void)readDataToData:(NSData *)data withTimeout:(NSTimeInterval)timeout tag:(long)tag;

/**
 * Reads bytes until (and including) the passed "data" parameter, which acts as a separator.
 * The bytes will be appended to the given byte buffer starting at the given offset.
 * The given buffer will automatically be increased in size if needed.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * If the buffer if nil, a buffer will automatically be created for you.
 * 
 * If the bufferOffset is greater than the length of the given buffer,
 * the method will do nothing, and the delegate will not be called.
 * 
 * If you pass a buffer, you must not alter it in any way while AsyncSocket is using it.
 * After completion, the data returned in onSocket:didReadData:withTag: will be a subset of the given buffer.
 * That is, it will reference the bytes that were appended to the given buffer.
 * 
 * To read a line from the socket, use the line separator (e.g. CRLF for HTTP, see below) as the "data" parameter.
 * Note that this method is not character-set aware, so if a separator can occur naturally as part of the encoding for
 * a character, the read will prematurely end.
**/
- (void)readDataToData:(NSData *)data
           withTimeout:(NSTimeInterval)timeout
                buffer:(NSMutableData *)buffer
          bufferOffset:(NSUInteger)offset
                   tag:(long)tag;

/**
 * Reads bytes until (and including) the passed "data" parameter, which acts as a separator.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * 
 * If maxLength is zero, no length restriction is enforced.
 * Otherwise if maxLength bytes are read without completing the read,
 * it is treated similarly to a timeout - the socket is closed with a AsyncSocketReadMaxedOutError.
 * The read will complete successfully if exactly maxLength bytes are read and the given data is found at the end.
 * 
 * If you pass nil or zero-length data as the "data" parameter,
 * the method will do nothing, and the delegate will not be called.
 * If you pass a maxLength parameter that is less than the length of the data parameter,
 * the method will do nothing, and the delegate will not be called.
 * 
 * To read a line from the socket, use the line separator (e.g. CRLF for HTTP, see below) as the "data" parameter.
 * Note that this method is not character-set aware, so if a separator can occur naturally as part of the encoding for
 * a character, the read will prematurely end.
**/
- (void)readDataToData:(NSData *)data withTimeout:(NSTimeInterval)timeout maxLength:(NSUInteger)length tag:(long)tag;

/**
 * Reads bytes until (and including) the passed "data" parameter, which acts as a separator.
 * The bytes will be appended to the given byte buffer starting at the given offset.
 * The given buffer will automatically be increased in size if needed.
 * A maximum of length bytes will be read.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * If the buffer if nil, a buffer will automatically be created for you.
 * 
 * If maxLength is zero, no length restriction is enforced.
 * Otherwise if maxLength bytes are read without completing the read,
 * it is treated similarly to a timeout - the socket is closed with a AsyncSocketReadMaxedOutError.
 * The read will complete successfully if exactly maxLength bytes are read and the given data is found at the end.
 * 
 * If you pass a maxLength parameter that is less than the length of the data parameter,
 * the method will do nothing, and the delegate will not be called.
 * If the bufferOffset is greater than the length of the given buffer,
 * the method will do nothing, and the delegate will not be called.
 * 
 * If you pass a buffer, you must not alter it in any way while AsyncSocket is using it.
 * After completion, the data returned in onSocket:didReadData:withTag: will be a subset of the given buffer.
 * That is, it will reference the bytes that were appended to the given buffer.
 * 
 * To read a line from the socket, use the line separator (e.g. CRLF for HTTP, see below) as the "data" parameter.
 * Note that this method is not character-set aware, so if a separator can occur naturally as part of the encoding for
 * a character, the read will prematurely end.
**/
- (void)readDataToData:(NSData *)data
           withTimeout:(NSTimeInterval)timeout
                buffer:(NSMutableData *)buffer
          bufferOffset:(NSUInteger)offset
             maxLength:(NSUInteger)length
                   tag:(long)tag;

/**
 * Writes data to the socket, and calls the delegate when finished.
 * 
 * If you pass in nil or zero-length data, this method does nothing and the delegate will not be called.
 * If the timeout value is negative, the write operation will not use a timeout.
**/
- (void)writeData:(NSData *)data withTimeout:(NSTimeInterval)timeout tag:(long)tag;

/**
 * Returns progress of current read or write, from 0.0 to 1.0, or NaN if no read/write (use isnan() to check).
 * "tag", "done" and "total" will be filled in if they aren't NULL.
**/
- (float)progressOfReadReturningTag:(long *)tag bytesDone:(NSUInteger *)done total:(NSUInteger *)total;
- (float)progressOfWriteReturningTag:(long *)tag bytesDone:(NSUInteger *)done total:(NSUInteger *)total;

/**
 * Secures the connection using SSL/TLS.
 * 
 * This method may be called at any time, and the TLS handshake will occur after all pending reads and writes
 * are finished. This allows one the option of sending a protocol dependent StartTLS message, and queuing
 * the upgrade to TLS at the same time, without having to wait for the write to finish.
 * Any reads or writes scheduled after this method is called will occur over the secured connection.
 * 
 * The possible keys and values for the TLS settings are well documented.
 * Some possible keys are:
 * - kCFStreamSSLLevel
 * - kCFStreamSSLAllowsExpiredCertificates
 * - kCFStreamSSLAllowsExpiredRoots
 * - kCFStreamSSLAllowsAnyRoot
 * - kCFStreamSSLValidatesCertificateChain
 * - kCFStreamSSLPeerName
 * - kCFStreamSSLCertificates
 * - kCFStreamSSLIsServer
 * 
 * Please refer to Apple's documentation for associated values, as well as other possible keys.
 * 
 * If you pass in nil or an empty dictionary, the default settings will be used.
 * 
 * The default settings will check to make sure the remote party's certificate is signed by a
 * trusted 3rd party certificate agency (e.g. verisign) and that the certificate is not expired.
 * However it will not verify the name on the certificate unless you
 * give it a name to verify against via the kCFStreamSSLPeerName key.
 * The security implications of this are important to understand.
 * Imagine you are attempting to create a secure connection to MySecureServer.com,
 * but your socket gets directed to MaliciousServer.com because of a hacked DNS server.
 * If you simply use the default settings, and MaliciousServer.com has a valid certificate,
 * the default settings will not detect any problems since the certificate is valid.
 * To properly secure your connection in this particular scenario you
 * should set the kCFStreamSSLPeerName property to "MySecureServer.com".
 * If you do not know the peer name of the remote host in advance (for example, you're not sure
 * if it will be "domain.com" or "www.domain.com"), then you can use the default settings to validate the
 * certificate, and then use the X509Certificate class to verify the issuer after the socket has been secured.
 * The X509Certificate class is part of the CocoaAsyncSocket open source project.
**/
- (void)startTLS:(NSDictionary *)tlsSettings;

/**
 * For handling readDataToData requests, data is necessarily read from the socket in small increments.
 * The performance can be much improved by allowing AsyncSocket to read larger chunks at a time and
 * store any overflow in a small internal buffer.
 * This is termed pre-buffering, as some data may be read for you before you ask for it.
 * If you use readDataToData a lot, enabling pre-buffering will result in better performance, especially on the iPhone.
 * 
 * The default pre-buffering state is controlled by the DEFAULT_PREBUFFERING definition.
 * It is highly recommended one leave this set to YES.
 * 
 * This method exists in case pre-buffering needs to be disabled by default for some unforeseen reason.
 * In that case, this method exists to allow one to easily enable pre-buffering when ready.
**/
- (void)enablePreBuffering;

/**
 * When you create an AsyncSocket, it is added to the runloop of the current thread.
 * So for manually created sockets, it is easiest to simply create the socket on the thread you intend to use it.
 * 
 * If a new socket is accepted, the delegate method onSocket:wantsRunLoopForNewSocket: is called to
 * allow you to place the socket on a separate thread. This works best in conjunction with a thread pool design.
 * 
 * If, however, you need to move the socket to a separate thread at a later time, this
 * method may be used to accomplish the task.
 * 
 * This method must be called from the thread/runloop the socket is currently running on.
 * 
 * Note: After calling this method, all further method calls to this object should be done from the given runloop.
 * Also, all delegate calls will be sent on the given runloop.
**/
- (BOOL)moveToRunLoop:(NSRunLoop *)runLoop;

/**
 * Allows you to configure which run loop modes the socket uses.
 * The default set of run loop modes is NSDefaultRunLoopMode.
 * 
 * If you'd like your socket to continue operation during other modes, you may want to add modes such as
 * NSModalPanelRunLoopMode or NSEventTrackingRunLoopMode. Or you may simply want to use NSRunLoopCommonModes.
 * 
 * Accepted sockets will automatically inherit the same run loop modes as the listening socket.
 * 
 * Note: NSRunLoopCommonModes is defined in 10.5. For previous versions one can use kCFRunLoopCommonModes.
**/
- (BOOL)setRunLoopModes:(NSArray *)runLoopModes;
- (BOOL)addRunLoopMode:(NSString *)runLoopMode;
- (BOOL)removeRunLoopMode:(NSString *)runLoopMode;

/**
 * Returns the current run loop modes the AsyncSocket instance is operating in.
 * The default set of run loop modes is NSDefaultRunLoopMode.
**/
- (NSArray *)runLoopModes;

/**
 * In the event of an error, this method may be called during onSocket:willDisconnectWithError: to read
 * any data that's left on the socket.
**/
- (NSData *)unreadData;

/* A few common line separators, for use with the readDataToData:... methods. */
+ (NSData *)CRLFData;   // 0x0D0A
+ (NSData *)CRData;     // 0x0D
+ (NSData *)LFData;     // 0x0A
+ (NSData *)ZeroData;   // 0x00

@end

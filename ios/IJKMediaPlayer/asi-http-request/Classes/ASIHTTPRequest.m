//
//  ASIHTTPRequest.m
//
//  Created by Ben Copsey on 04/10/2007.
//  Copyright 2007-2011 All-Seeing Interactive. All rights reserved.
//
//  A guide to the main features is available at:
//  http://allseeing-i.com/ASIHTTPRequest
//
//  Portions are based on the ImageClient example from Apple:
//  See: http://developer.apple.com/samplecode/ImageClient/listing37.html

#import "ASIHTTPRequest.h"

#if TARGET_OS_IPHONE
#import "Reachability.h"
#import "ASIAuthenticationDialog.h"
#import <MobileCoreServices/MobileCoreServices.h>
#else
#import <SystemConfiguration/SystemConfiguration.h>
#endif
#import "ASIInputStream.h"
#import "ASIDataDecompressor.h"
#import "ASIDataCompressor.h"

// Automatically set on build
NSString *ASIHTTPRequestVersion = @"v1.8.1-61 2011-09-19";

static NSString *defaultUserAgent = nil;

NSString* const NetworkRequestErrorDomain = @"ASIHTTPRequestErrorDomain";

static NSString *ASIHTTPRequestRunLoopMode = @"ASIHTTPRequestRunLoopMode";

static const CFOptionFlags kNetworkEvents =  kCFStreamEventHasBytesAvailable | kCFStreamEventEndEncountered | kCFStreamEventErrorOccurred;

// In memory caches of credentials, used on when useSessionPersistence is YES
static NSMutableArray *sessionCredentialsStore = nil;
static NSMutableArray *sessionProxyCredentialsStore = nil;

// This lock mediates access to session credentials
static NSRecursiveLock *sessionCredentialsLock = nil;

// We keep track of cookies we have received here so we can remove them from the sharedHTTPCookieStorage later
static NSMutableArray *sessionCookies = nil;

// The number of times we will allow requests to redirect before we fail with a redirection error
const int RedirectionLimit = 5;

// The default number of seconds to use for a timeout
static NSTimeInterval defaultTimeOutSeconds = 10;

static void ReadStreamClientCallBack(CFReadStreamRef readStream, CFStreamEventType type, void *clientCallBackInfo) {
    [((ASIHTTPRequest*)clientCallBackInfo) handleNetworkEvent: type];
}

// This lock prevents the operation from being cancelled while it is trying to update the progress, and vice versa
static NSRecursiveLock *progressLock;

static NSError *ASIRequestCancelledError;
static NSError *ASIRequestTimedOutError;
static NSError *ASIAuthenticationError;
static NSError *ASIUnableToCreateRequestError;
static NSError *ASITooMuchRedirectionError;

static NSMutableArray *bandwidthUsageTracker = nil;
static unsigned long averageBandwidthUsedPerSecond = 0;

// These are used for queuing persistent connections on the same connection

// Incremented every time we specify we want a new connection
static unsigned int nextConnectionNumberToCreate = 0;

// An array of connectionInfo dictionaries.
// When attempting a persistent connection, we look here to try to find an existing connection to the same server that is currently not in use
static NSMutableArray *persistentConnectionsPool = nil;

// Mediates access to the persistent connections pool
static NSRecursiveLock *connectionsLock = nil;

// Each request gets a new id, we store this rather than a ref to the request itself in the connectionInfo dictionary.
// We do this so we don't have to keep the request around while we wait for the connection to expire
static unsigned int nextRequestID = 0;

// Records how much bandwidth all requests combined have used in the last second
static unsigned long bandwidthUsedInLastSecond = 0; 

// A date one second in the future from the time it was created
static NSDate *bandwidthMeasurementDate = nil;

// Since throttling variables are shared among all requests, we'll use a lock to mediate access
static NSLock *bandwidthThrottlingLock = nil;

// the maximum number of bytes that can be transmitted in one second
static unsigned long maxBandwidthPerSecond = 0;

// A default figure for throttling bandwidth on mobile devices
unsigned long const ASIWWANBandwidthThrottleAmount = 14800;

#if TARGET_OS_IPHONE
// YES when bandwidth throttling is active
// This flag does not denote whether throttling is turned on - rather whether it is currently in use
// It will be set to NO when throttling was turned on with setShouldThrottleBandwidthForWWAN, but a WI-FI connection is active
static BOOL isBandwidthThrottled = NO;

// When YES, bandwidth will be automatically throttled when using WWAN (3G/Edge/GPRS)
// Wifi will not be throttled
static BOOL shouldThrottleBandwidthForWWANOnly = NO;
#endif

// Mediates access to the session cookies so requests
static NSRecursiveLock *sessionCookiesLock = nil;

// This lock ensures delegates only receive one notification that authentication is required at once
// When using ASIAuthenticationDialogs, it also ensures only one dialog is shown at once
// If a request can't acquire the lock immediately, it means a dialog is being shown or a delegate is handling the authentication challenge
// Once it gets the lock, it will try to look for existing credentials again rather than showing the dialog / notifying the delegate
// This is so it can make use of any credentials supplied for the other request, if they are appropriate
static NSRecursiveLock *delegateAuthenticationLock = nil;

// When throttling bandwidth, Set to a date in future that we will allow all requests to wake up and reschedule their streams
static NSDate *throttleWakeUpTime = nil;

static id <ASICacheDelegate> defaultCache = nil;

// Used for tracking when requests are using the network
static unsigned int runningRequestCount = 0;

// You can use [ASIHTTPRequest setShouldUpdateNetworkActivityIndicator:NO] if you want to manage it yourself
// Alternatively, override showNetworkActivityIndicator / hideNetworkActivityIndicator
// By default this does nothing on Mac OS X, but again override the above methods for a different behaviour
static BOOL shouldUpdateNetworkActivityIndicator = YES;

// The thread all requests will run on
// Hangs around forever, but will be blocked unless there are requests underway
static NSThread *networkThread = nil;

static NSOperationQueue *sharedQueue = nil;

// Private stuff
@interface ASIHTTPRequest ()

- (void)cancelLoad;

- (void)destroyReadStream;
- (void)scheduleReadStream;
- (void)unscheduleReadStream;

- (BOOL)willAskDelegateForCredentials;
- (BOOL)willAskDelegateForProxyCredentials;
- (void)askDelegateForProxyCredentials;
- (void)askDelegateForCredentials;
- (void)failAuthentication;

+ (void)measureBandwidthUsage;
+ (void)recordBandwidthUsage;

- (void)startRequest;
- (void)updateStatus:(NSTimer *)timer;
- (void)checkRequestStatus;
- (void)reportFailure;
- (void)reportFinished;
- (void)markAsFinished;
- (void)performRedirect;
- (BOOL)shouldTimeOut;
- (BOOL)willRedirect;
- (BOOL)willAskDelegateToConfirmRedirect;

+ (void)performInvocation:(NSInvocation *)invocation onTarget:(id *)target releasingObject:(id)objectToRelease;
+ (void)hideNetworkActivityIndicatorAfterDelay;
+ (void)hideNetworkActivityIndicatorIfNeeeded;
+ (void)runRequests;

// Handling Proxy autodetection and PAC file downloads
- (BOOL)configureProxies;
- (void)fetchPACFile;
- (void)finishedDownloadingPACFile:(ASIHTTPRequest *)theRequest;
- (void)runPACScript:(NSString *)script;
- (void)timeOutPACRead;

- (void)useDataFromCache;

// Called to update the size of a partial download when starting a request, or retrying after a timeout
- (void)updatePartialDownloadSize;

#if TARGET_OS_IPHONE
+ (void)registerForNetworkReachabilityNotifications;
+ (void)unsubscribeFromNetworkReachabilityNotifications;
// Called when the status of the network changes
+ (void)reachabilityChanged:(NSNotification *)note;
#endif

#if NS_BLOCKS_AVAILABLE
- (void)performBlockOnMainThread:(ASIBasicBlock)block;
- (void)releaseBlocksOnMainThread;
+ (void)releaseBlocks:(NSArray *)blocks;
- (void)callBlock:(ASIBasicBlock)block;
#endif





@property (assign) BOOL complete;
@property (retain) NSArray *responseCookies;
@property (assign) int responseStatusCode;
@property (retain, nonatomic) NSDate *lastActivityTime;

@property (assign) unsigned long long partialDownloadSize;
@property (assign, nonatomic) unsigned long long uploadBufferSize;
@property (retain, nonatomic) NSOutputStream *postBodyWriteStream;
@property (retain, nonatomic) NSInputStream *postBodyReadStream;
@property (assign, nonatomic) unsigned long long lastBytesRead;
@property (assign, nonatomic) unsigned long long lastBytesSent;
@property (atomic, retain) NSRecursiveLock *cancelledLock;
@property (retain, nonatomic) NSOutputStream *fileDownloadOutputStream;
@property (retain, nonatomic) NSOutputStream *inflatedFileDownloadOutputStream;
@property (assign) int authenticationRetryCount;
@property (assign) int proxyAuthenticationRetryCount;
@property (assign, nonatomic) BOOL updatedProgress;
@property (assign, nonatomic) BOOL needsRedirect;
@property (assign, nonatomic) int redirectCount;
@property (retain, nonatomic) NSData *compressedPostBody;
@property (retain, nonatomic) NSString *compressedPostBodyFilePath;
@property (retain) NSString *authenticationRealm;
@property (retain) NSString *proxyAuthenticationRealm;
@property (retain) NSString *responseStatusMessage;
@property (assign) BOOL inProgress;
@property (assign) int retryCount;
@property (atomic, assign) BOOL willRetryRequest;
@property (assign) BOOL connectionCanBeReused;
@property (retain, nonatomic) NSMutableDictionary *connectionInfo;
@property (retain, nonatomic) NSInputStream *readStream;
@property (assign) ASIAuthenticationState authenticationNeeded;
@property (assign, nonatomic) BOOL readStreamIsScheduled;
@property (assign, nonatomic) BOOL downloadComplete;
@property (retain) NSNumber *requestID;
@property (assign, nonatomic) NSString *runLoopMode;
@property (retain, nonatomic) NSTimer *statusTimer;
@property (assign) BOOL didUseCachedResponse;
@property (retain, nonatomic) NSURL *redirectURL;

@property (assign, nonatomic) BOOL isPACFileRequest;
@property (retain, nonatomic) ASIHTTPRequest *PACFileRequest;
@property (retain, nonatomic) NSInputStream *PACFileReadStream;
@property (retain, nonatomic) NSMutableData *PACFileData;

@property (assign, nonatomic, setter=setSynchronous:) BOOL isSynchronous;
@end


@implementation ASIHTTPRequest

#pragma mark init / dealloc

+ (void)initialize
{
	if (self == [ASIHTTPRequest class]) {
		persistentConnectionsPool = [[NSMutableArray alloc] init];
		connectionsLock = [[NSRecursiveLock alloc] init];
		progressLock = [[NSRecursiveLock alloc] init];
		bandwidthThrottlingLock = [[NSLock alloc] init];
		sessionCookiesLock = [[NSRecursiveLock alloc] init];
		sessionCredentialsLock = [[NSRecursiveLock alloc] init];
		delegateAuthenticationLock = [[NSRecursiveLock alloc] init];
		bandwidthUsageTracker = [[NSMutableArray alloc] initWithCapacity:5];
		ASIRequestTimedOutError = [[NSError alloc] initWithDomain:NetworkRequestErrorDomain code:ASIRequestTimedOutErrorType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"The request timed out",NSLocalizedDescriptionKey,nil]];  
		ASIAuthenticationError = [[NSError alloc] initWithDomain:NetworkRequestErrorDomain code:ASIAuthenticationErrorType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Authentication needed",NSLocalizedDescriptionKey,nil]];
		ASIRequestCancelledError = [[NSError alloc] initWithDomain:NetworkRequestErrorDomain code:ASIRequestCancelledErrorType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"The request was cancelled",NSLocalizedDescriptionKey,nil]];
		ASIUnableToCreateRequestError = [[NSError alloc] initWithDomain:NetworkRequestErrorDomain code:ASIUnableToCreateRequestErrorType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Unable to create request (bad url?)",NSLocalizedDescriptionKey,nil]];
		ASITooMuchRedirectionError = [[NSError alloc] initWithDomain:NetworkRequestErrorDomain code:ASITooMuchRedirectionErrorType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"The request failed because it redirected too many times",NSLocalizedDescriptionKey,nil]];
		sharedQueue = [[NSOperationQueue alloc] init];
		[sharedQueue setMaxConcurrentOperationCount:4];

	}
}


- (id)initWithURL:(NSURL *)newURL
{
	self = [self init];
	[self setRequestMethod:@"GET"];

	[self setRunLoopMode:NSDefaultRunLoopMode];
	[self setShouldAttemptPersistentConnection:YES];
	[self setPersistentConnectionTimeoutSeconds:60.0];
	[self setShouldPresentCredentialsBeforeChallenge:YES];
	[self setShouldRedirect:YES];
	[self setShowAccurateProgress:YES];
	[self setShouldResetDownloadProgress:YES];
	[self setShouldResetUploadProgress:YES];
	[self setAllowCompressedResponse:YES];
	[self setShouldWaitToInflateCompressedResponses:YES];
	[self setDefaultResponseEncoding:NSISOLatin1StringEncoding];
	[self setShouldPresentProxyAuthenticationDialog:YES];
	
	[self setTimeOutSeconds:[ASIHTTPRequest defaultTimeOutSeconds]];
	[self setUseSessionPersistence:YES];
	[self setUseCookiePersistence:YES];
	[self setValidatesSecureCertificate:YES];
	[self setRequestCookies:[[[NSMutableArray alloc] init] autorelease]];
	[self setDidStartSelector:@selector(requestStarted:)];
	[self setDidReceiveResponseHeadersSelector:@selector(request:didReceiveResponseHeaders:)];
	[self setWillRedirectSelector:@selector(request:willRedirectToURL:)];
	[self setDidFinishSelector:@selector(requestFinished:)];
	[self setDidFailSelector:@selector(requestFailed:)];
	[self setDidReceiveDataSelector:@selector(request:didReceiveData:)];
	[self setURL:newURL];
	[self setCancelledLock:[[[NSRecursiveLock alloc] init] autorelease]];
	[self setDownloadCache:[[self class] defaultCache]];
	return self;
}

+ (id)requestWithURL:(NSURL *)newURL
{
	return [[[self alloc] initWithURL:newURL] autorelease];
}

+ (id)requestWithURL:(NSURL *)newURL usingCache:(id <ASICacheDelegate>)cache
{
	return [self requestWithURL:newURL usingCache:cache andCachePolicy:ASIUseDefaultCachePolicy];
}

+ (id)requestWithURL:(NSURL *)newURL usingCache:(id <ASICacheDelegate>)cache andCachePolicy:(ASICachePolicy)policy
{
	ASIHTTPRequest *request = [[[self alloc] initWithURL:newURL] autorelease];
	[request setDownloadCache:cache];
	[request setCachePolicy:policy];
	return request;
}

- (void)dealloc
{
	[self setAuthenticationNeeded:ASINoAuthenticationNeededYet];
	if (requestAuthentication) {
		CFRelease(requestAuthentication);
	}
	if (proxyAuthentication) {
		CFRelease(proxyAuthentication);
	}
	if (request) {
		CFRelease(request);
	}
	if (clientCertificateIdentity) {
		CFRelease(clientCertificateIdentity);
	}
	[self cancelLoad];
	[redirectURL release];
	[statusTimer invalidate];
	[statusTimer release];
	[queue release];
	[userInfo release];
	[postBody release];
	[compressedPostBody release];
	[error release];
	[requestHeaders release];
	[requestCookies release];
	[downloadDestinationPath release];
	[temporaryFileDownloadPath release];
	[temporaryUncompressedDataDownloadPath release];
	[fileDownloadOutputStream release];
	[inflatedFileDownloadOutputStream release];
	[username release];
	[password release];
	[domain release];
	[authenticationRealm release];
	[authenticationScheme release];
	[requestCredentials release];
	[proxyHost release];
	[proxyType release];
	[proxyUsername release];
	[proxyPassword release];
	[proxyDomain release];
	[proxyAuthenticationRealm release];
	[proxyAuthenticationScheme release];
	[proxyCredentials release];
	[url release];
	[originalURL release];
	[lastActivityTime release];
	[responseCookies release];
	[rawResponseData release];
	[responseHeaders release];
	[requestMethod release];
	[cancelledLock release];
	[postBodyFilePath release];
	[compressedPostBodyFilePath release];
	[postBodyWriteStream release];
	[postBodyReadStream release];
	[PACurl release];
	[clientCertificates release];
	[responseStatusMessage release];
	[connectionInfo release];
	[requestID release];
	[dataDecompressor release];
	[userAgentString release];

	#if NS_BLOCKS_AVAILABLE
	[self releaseBlocksOnMainThread];
	#endif

	[super dealloc];
}

#if NS_BLOCKS_AVAILABLE
- (void)releaseBlocksOnMainThread
{
	NSMutableArray *blocks = [NSMutableArray array];
	if (completionBlock) {
		[blocks addObject:completionBlock];
		[completionBlock release];
		completionBlock = nil;
	}
	if (failureBlock) {
		[blocks addObject:failureBlock];
		[failureBlock release];
		failureBlock = nil;
	}
	if (startedBlock) {
		[blocks addObject:startedBlock];
		[startedBlock release];
		startedBlock = nil;
	}
	if (headersReceivedBlock) {
		[blocks addObject:headersReceivedBlock];
		[headersReceivedBlock release];
		headersReceivedBlock = nil;
	}
	if (bytesReceivedBlock) {
		[blocks addObject:bytesReceivedBlock];
		[bytesReceivedBlock release];
		bytesReceivedBlock = nil;
	}
	if (bytesSentBlock) {
		[blocks addObject:bytesSentBlock];
		[bytesSentBlock release];
		bytesSentBlock = nil;
	}
	if (downloadSizeIncrementedBlock) {
		[blocks addObject:downloadSizeIncrementedBlock];
		[downloadSizeIncrementedBlock release];
		downloadSizeIncrementedBlock = nil;
	}
	if (uploadSizeIncrementedBlock) {
		[blocks addObject:uploadSizeIncrementedBlock];
		[uploadSizeIncrementedBlock release];
		uploadSizeIncrementedBlock = nil;
	}
	if (dataReceivedBlock) {
		[blocks addObject:dataReceivedBlock];
		[dataReceivedBlock release];
		dataReceivedBlock = nil;
	}
	if (proxyAuthenticationNeededBlock) {
		[blocks addObject:proxyAuthenticationNeededBlock];
		[proxyAuthenticationNeededBlock release];
		proxyAuthenticationNeededBlock = nil;
	}
	if (authenticationNeededBlock) {
		[blocks addObject:authenticationNeededBlock];
		[authenticationNeededBlock release];
		authenticationNeededBlock = nil;
	}
	if (requestRedirectedBlock) {
		[blocks addObject:requestRedirectedBlock];
		[requestRedirectedBlock release];
		requestRedirectedBlock = nil;
	}
	[[self class] performSelectorOnMainThread:@selector(releaseBlocks:) withObject:blocks waitUntilDone:[NSThread isMainThread]];
}
// Always called on main thread
+ (void)releaseBlocks:(NSArray *)blocks
{
	// Blocks will be released when this method exits
}
#endif


#pragma mark setup request

- (void)addRequestHeader:(NSString *)header value:(NSString *)value
{
	if (!requestHeaders) {
		[self setRequestHeaders:[NSMutableDictionary dictionaryWithCapacity:1]];
	}
	[requestHeaders setObject:value forKey:header];
}

// This function will be called either just before a request starts, or when postLength is needed, whichever comes first
// postLength must be set by the time this function is complete
- (void)buildPostBody
{

	if ([self haveBuiltPostBody]) {
		return;
	}
	
	// Are we submitting the request body from a file on disk
	if ([self postBodyFilePath]) {
		
		// If we were writing to the post body via appendPostData or appendPostDataFromFile, close the write stream
		if ([self postBodyWriteStream]) {
			[[self postBodyWriteStream] close];
			[self setPostBodyWriteStream:nil];
		}

		
		NSString *path;
		if ([self shouldCompressRequestBody]) {
			if (![self compressedPostBodyFilePath]) {
				[self setCompressedPostBodyFilePath:[NSTemporaryDirectory() stringByAppendingPathComponent:[[NSProcessInfo processInfo] globallyUniqueString]]];
				
				NSError *err = nil;
				if (![ASIDataCompressor compressDataFromFile:[self postBodyFilePath] toFile:[self compressedPostBodyFilePath] error:&err]) {
					[self failWithError:err];
					return;
				}
			}
			path = [self compressedPostBodyFilePath];
		} else {
			path = [self postBodyFilePath];
		}
		NSError *err = nil;
		[self setPostLength:[[[[[NSFileManager alloc] init] autorelease] attributesOfItemAtPath:path error:&err] fileSize]];
		if (err) {
			[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIFileManagementError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Failed to get attributes for file at path '%@'",path],NSLocalizedDescriptionKey,error,NSUnderlyingErrorKey,nil]]];
			return;
		}
		
	// Otherwise, we have an in-memory request body
	} else {
		if ([self shouldCompressRequestBody]) {
			NSError *err = nil;
			NSData *compressedBody = [ASIDataCompressor compressData:[self postBody] error:&err];
			if (err) {
				[self failWithError:err];
				return;
			}
			[self setCompressedPostBody:compressedBody];
			[self setPostLength:[[self compressedPostBody] length]];
		} else {
			[self setPostLength:[[self postBody] length]];
		}
	}
		
	if ([self postLength] > 0) {
		if ([requestMethod isEqualToString:@"GET"] || [requestMethod isEqualToString:@"DELETE"] || [requestMethod isEqualToString:@"HEAD"]) {
			[self setRequestMethod:@"POST"];
		}
		[self addRequestHeader:@"Content-Length" value:[NSString stringWithFormat:@"%llu",[self postLength]]];
	}
	[self setHaveBuiltPostBody:YES];

}

// Sets up storage for the post body
- (void)setupPostBody
{
	if ([self shouldStreamPostDataFromDisk]) {
		if (![self postBodyFilePath]) {
			[self setPostBodyFilePath:[NSTemporaryDirectory() stringByAppendingPathComponent:[[NSProcessInfo processInfo] globallyUniqueString]]];
			[self setDidCreateTemporaryPostDataFile:YES];
		}
		if (![self postBodyWriteStream]) {
			[self setPostBodyWriteStream:[[[NSOutputStream alloc] initToFileAtPath:[self postBodyFilePath] append:NO] autorelease]];
			[[self postBodyWriteStream] open];
		}
	} else {
		if (![self postBody]) {
			[self setPostBody:[[[NSMutableData alloc] init] autorelease]];
		}
	}	
}

- (void)appendPostData:(NSData *)data
{
	[self setupPostBody];
	if ([data length] == 0) {
		return;
	}
	if ([self shouldStreamPostDataFromDisk]) {
		[[self postBodyWriteStream] write:[data bytes] maxLength:[data length]];
	} else {
		[[self postBody] appendData:data];
	}
}

- (void)appendPostDataFromFile:(NSString *)file
{
	[self setupPostBody];
	NSInputStream *stream = [[[NSInputStream alloc] initWithFileAtPath:file] autorelease];
	[stream open];
	while ([stream hasBytesAvailable]) {
		
		unsigned char buffer[1024*256];
		NSInteger bytesRead = [stream read:buffer maxLength:sizeof(buffer)];
		if (bytesRead == 0) {
			// 0 indicates that the end of the buffer was reached.
			break;
		} else if (bytesRead < 0) {
			// A negative number means that the operation failed.
			break;
		}
		if ([self shouldStreamPostDataFromDisk]) {
			[[self postBodyWriteStream] write:buffer maxLength:(NSUInteger)bytesRead];
		} else {
			[[self postBody] appendData:[NSData dataWithBytes:buffer length:(NSUInteger)bytesRead]];
		}
	}
	[stream close];
}

- (NSString *)requestMethod
{
	[[self cancelledLock] lock];
	NSString *m = requestMethod;
	[[self cancelledLock] unlock];
	return m;
}

- (void)setRequestMethod:(NSString *)newRequestMethod
{
	[[self cancelledLock] lock];
	if (requestMethod != newRequestMethod) {
		[requestMethod release];
		requestMethod = [newRequestMethod retain];
		if ([requestMethod isEqualToString:@"POST"] || [requestMethod isEqualToString:@"PUT"] || [postBody length] || postBodyFilePath) {
			[self setShouldAttemptPersistentConnection:NO];
		}
	}
	[[self cancelledLock] unlock];
}

- (NSURL *)url
{
	[[self cancelledLock] lock];
	NSURL *u = url;
	[[self cancelledLock] unlock];
	return u;
}


- (void)setURL:(NSURL *)newURL
{
	[[self cancelledLock] lock];
	if ([newURL isEqual:[self url]]) {
		[[self cancelledLock] unlock];
		return;
	}
	[url release];
	url = [newURL retain];
	if (requestAuthentication) {
		CFRelease(requestAuthentication);
		requestAuthentication = NULL;
	}
	if (proxyAuthentication) {
		CFRelease(proxyAuthentication);
		proxyAuthentication = NULL;
	}
	if (request) {
		CFRelease(request);
		request = NULL;
	}
	[self setRedirectURL:nil];
	[[self cancelledLock] unlock];
}

- (id)delegate
{
	[[self cancelledLock] lock];
	id d = delegate;
	[[self cancelledLock] unlock];
	return d;
}

- (void)setDelegate:(id)newDelegate
{
	[[self cancelledLock] lock];
	delegate = newDelegate;
	[[self cancelledLock] unlock];
}

- (id)queue
{
	[[self cancelledLock] lock];
	id q = queue;
	[[self cancelledLock] unlock];
	return q;
}


- (void)setQueue:(id)newQueue
{
	[[self cancelledLock] lock];
	if (newQueue != queue) {
		[queue release];
		queue = [newQueue retain];
	}
	[[self cancelledLock] unlock];
}

#pragma mark get information about this request

// cancel the request - this must be run on the same thread as the request is running on
- (void)cancelOnRequestThread
{
	#if DEBUG_REQUEST_STATUS
	ASI_DEBUG_LOG(@"[STATUS] Request cancelled: %@",self);
	#endif
    
	[[self cancelledLock] lock];

    if ([self isCancelled] || [self complete]) {
		[[self cancelledLock] unlock];
		return;
	}
	[self failWithError:ASIRequestCancelledError];
	[self setComplete:YES];
	[self cancelLoad];
	
	CFRetain(self);
    [self willChangeValueForKey:@"isCancelled"];
    cancelled = YES;
    [self didChangeValueForKey:@"isCancelled"];
    
	[[self cancelledLock] unlock];
	CFRelease(self);
}

- (void)cancel
{
    [self performSelector:@selector(cancelOnRequestThread) onThread:[[self class] threadForRequest:self] withObject:nil waitUntilDone:NO];    
}

- (void)clearDelegatesAndCancel
{
	[[self cancelledLock] lock];

	// Clear delegates
	[self setDelegate:nil];
	[self setQueue:nil];
	[self setDownloadProgressDelegate:nil];
	[self setUploadProgressDelegate:nil];

	#if NS_BLOCKS_AVAILABLE
	// Clear blocks
	[self releaseBlocksOnMainThread];
	#endif

	[[self cancelledLock] unlock];
	[self cancel];
}


- (BOOL)isCancelled
{
    BOOL result;
    
	[[self cancelledLock] lock];
    result = cancelled;
    [[self cancelledLock] unlock];
    
    return result;
}

// Call this method to get the received data as an NSString. Don't use for binary data!
- (NSString *)responseString
{
	NSData *data = [self responseData];
	if (!data) {
		return nil;
	}
	
	return [[[NSString alloc] initWithBytes:[data bytes] length:[data length] encoding:[self responseEncoding]] autorelease];
}

- (BOOL)isResponseCompressed
{
	NSString *encoding = [[self responseHeaders] objectForKey:@"Content-Encoding"];
	return encoding && [encoding rangeOfString:@"gzip"].location != NSNotFound;
}

- (NSData *)responseData
{	
	if ([self isResponseCompressed] && [self shouldWaitToInflateCompressedResponses]) {
		return [ASIDataDecompressor uncompressData:[self rawResponseData] error:NULL];
	} else {
		return [self rawResponseData];
	}
	return nil;
}

#pragma mark running a request

- (void)startSynchronous
{
#if DEBUG_REQUEST_STATUS || DEBUG_THROTTLING
	ASI_DEBUG_LOG(@"[STATUS] Starting synchronous request %@",self);
#endif
	[self setSynchronous:YES];
	[self setRunLoopMode:ASIHTTPRequestRunLoopMode];
	[self setInProgress:YES];

	if (![self isCancelled] && ![self complete]) {
		[self main];
		while (!complete) {
			[[NSRunLoop currentRunLoop] runMode:[self runLoopMode] beforeDate:[NSDate distantFuture]];
		}
	}

	[self setInProgress:NO];
}

- (void)start
{
	[self setInProgress:YES];
	[self performSelector:@selector(main) onThread:[[self class] threadForRequest:self] withObject:nil waitUntilDone:NO];
}

- (void)startAsynchronous
{
#if DEBUG_REQUEST_STATUS || DEBUG_THROTTLING
	ASI_DEBUG_LOG(@"[STATUS] Starting asynchronous request %@",self);
#endif
	[sharedQueue addOperation:self];
}

#pragma mark concurrency

- (BOOL)isConcurrent
{
    return YES;
}

- (BOOL)isFinished 
{
	return finished;
}

- (BOOL)isExecuting {
	return [self inProgress];
}

#pragma mark request logic

// Create the request
- (void)main
{
	@try {
		
		[[self cancelledLock] lock];
		
		#if TARGET_OS_IPHONE && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_4_0
		if ([ASIHTTPRequest isMultitaskingSupported] && [self shouldContinueWhenAppEntersBackground]) {
            if (!backgroundTask || backgroundTask == UIBackgroundTaskInvalid) {
                backgroundTask = [[UIApplication sharedApplication] beginBackgroundTaskWithExpirationHandler:^{
                    // Synchronize the cleanup call on the main thread in case
                    // the task actually finishes at around the same time.
                    dispatch_async(dispatch_get_main_queue(), ^{
                        if (backgroundTask != UIBackgroundTaskInvalid)
                        {
                            [[UIApplication sharedApplication] endBackgroundTask:backgroundTask];
                            backgroundTask = UIBackgroundTaskInvalid;
                            [self cancel];
                        }
                    });
                }];
            }
		}
		#endif


		// A HEAD request generated by an ASINetworkQueue may have set the error already. If so, we should not proceed.
		if ([self error]) {
			[self setComplete:YES];
			[self markAsFinished];
			return;		
		}

		[self setComplete:NO];
		[self setDidUseCachedResponse:NO];
		
		if (![self url]) {
			[self failWithError:ASIUnableToCreateRequestError];
			return;		
		}
		
		// Must call before we create the request so that the request method can be set if needs be
		if (![self mainRequest]) {
			[self buildPostBody];
		}
		
		if (![[self requestMethod] isEqualToString:@"GET"]) {
			[self setDownloadCache:nil];
		}
		
		
		// If we're redirecting, we'll already have a CFHTTPMessageRef
		if (request) {
			CFRelease(request);
		}

		// Create a new HTTP request.
		request = CFHTTPMessageCreateRequest(kCFAllocatorDefault, (CFStringRef)[self requestMethod], (CFURLRef)[self url], [self useHTTPVersionOne] ? kCFHTTPVersion1_0 : kCFHTTPVersion1_1);
		if (!request) {
			[self failWithError:ASIUnableToCreateRequestError];
			return;
		}

		//If this is a HEAD request generated by an ASINetworkQueue, we need to let the main request generate its headers first so we can use them
		if ([self mainRequest]) {
			[[self mainRequest] buildRequestHeaders];
		}
		
		// Even if this is a HEAD request with a mainRequest, we still need to call to give subclasses a chance to add their own to HEAD requests (ASIS3Request does this)
		[self buildRequestHeaders];
		
		if ([self downloadCache]) {

			// If this request should use the default policy, set its policy to the download cache's default policy
			if (![self cachePolicy]) {
				[self setCachePolicy:[[self downloadCache] defaultCachePolicy]];
			}

			// If have have cached data that is valid for this request, use that and stop
			if ([[self downloadCache] canUseCachedDataForRequest:self]) {
				[self useDataFromCache];
				return;
			}

			// If cached data is stale, or we have been told to ask the server if it has been modified anyway, we need to add headers for a conditional GET
			if ([self cachePolicy] & (ASIAskServerIfModifiedWhenStaleCachePolicy|ASIAskServerIfModifiedCachePolicy)) {

				NSDictionary *cachedHeaders = [[self downloadCache] cachedResponseHeadersForURL:[self url]];
				if (cachedHeaders) {
					NSString *etag = [cachedHeaders objectForKey:@"Etag"];
					if (etag) {
						[[self requestHeaders] setObject:etag forKey:@"If-None-Match"];
					}
					NSString *lastModified = [cachedHeaders objectForKey:@"Last-Modified"];
					if (lastModified) {
						[[self requestHeaders] setObject:lastModified forKey:@"If-Modified-Since"];
					}
				}
			}
		}

		[self applyAuthorizationHeader];
		
		
		NSString *header;
		for (header in [self requestHeaders]) {
			CFHTTPMessageSetHeaderFieldValue(request, (CFStringRef)header, (CFStringRef)[[self requestHeaders] objectForKey:header]);
		}

		// If we immediately have access to proxy settings, start the request
		// Otherwise, we'll start downloading the proxy PAC file, and call startRequest once that process is complete
		if ([self configureProxies]) {
			[self startRequest];
		}

	} @catch (NSException *exception) {
		NSError *underlyingError = [NSError errorWithDomain:NetworkRequestErrorDomain code:ASIUnhandledExceptionError userInfo:[exception userInfo]];
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIUnhandledExceptionError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[exception name],NSLocalizedDescriptionKey,[exception reason],NSLocalizedFailureReasonErrorKey,underlyingError,NSUnderlyingErrorKey,nil]]];

	} @finally {
		[[self cancelledLock] unlock];
	}
}

- (void)applyAuthorizationHeader
{
	// Do we want to send credentials before we are asked for them?
	if (![self shouldPresentCredentialsBeforeChallenge]) {
		#if DEBUG_HTTP_AUTHENTICATION
		ASI_DEBUG_LOG(@"[AUTH] Request %@ will not send credentials to the server until it asks for them",self);
		#endif
		return;
	}

	NSDictionary *credentials = nil;

	// Do we already have an auth header?
	if (![[self requestHeaders] objectForKey:@"Authorization"]) {

		// If we have basic authentication explicitly set and a username and password set on the request, add a basic auth header
		if ([self username] && [self password] && [[self authenticationScheme] isEqualToString:(NSString *)kCFHTTPAuthenticationSchemeBasic]) {
			[self addBasicAuthenticationHeaderWithUsername:[self username] andPassword:[self password]];

			#if DEBUG_HTTP_AUTHENTICATION
			ASI_DEBUG_LOG(@"[AUTH] Request %@ has a username and password set, and was manually configured to use BASIC. Will send credentials without waiting for an authentication challenge",self);	
			#endif

		} else {

			// See if we have any cached credentials we can use in the session store
			if ([self useSessionPersistence]) {
				credentials = [self findSessionAuthenticationCredentials];

				if (credentials) {

					// When the Authentication key is set, the credentials were stored after an authentication challenge, so we can let CFNetwork apply them
					// (credentials for Digest and NTLM will always be stored like this)
					if ([credentials objectForKey:@"Authentication"]) {

						// If we've already talked to this server and have valid credentials, let's apply them to the request
						if (CFHTTPMessageApplyCredentialDictionary(request, (CFHTTPAuthenticationRef)[credentials objectForKey:@"Authentication"], (CFDictionaryRef)[credentials objectForKey:@"Credentials"], NULL)) {
							[self setAuthenticationScheme:[credentials objectForKey:@"AuthenticationScheme"]];
							#if DEBUG_HTTP_AUTHENTICATION
							ASI_DEBUG_LOG(@"[AUTH] Request %@ found cached credentials (%@), will reuse without waiting for an authentication challenge",self,[credentials objectForKey:@"AuthenticationScheme"]);
							#endif
						} else {
							[[self class] removeAuthenticationCredentialsFromSessionStore:[credentials objectForKey:@"Credentials"]];
							#if DEBUG_HTTP_AUTHENTICATION
							ASI_DEBUG_LOG(@"[AUTH] Failed to apply cached credentials to request %@. These will be removed from the session store, and this request will wait for an authentication challenge",self);
							#endif
						}

					// If the Authentication key is not set, these credentials were stored after a username and password set on a previous request passed basic authentication
					// When this happens, we'll need to create the Authorization header ourselves
					} else {
						NSDictionary *usernameAndPassword = [credentials objectForKey:@"Credentials"];
						[self addBasicAuthenticationHeaderWithUsername:[usernameAndPassword objectForKey:(NSString *)kCFHTTPAuthenticationUsername] andPassword:[usernameAndPassword objectForKey:(NSString *)kCFHTTPAuthenticationPassword]];
						#if DEBUG_HTTP_AUTHENTICATION
						ASI_DEBUG_LOG(@"[AUTH] Request %@ found cached BASIC credentials from a previous request. Will send credentials without waiting for an authentication challenge",self);
						#endif
					}
				}
			}
		}
	}

	// Apply proxy authentication credentials
	if ([self useSessionPersistence]) {
		credentials = [self findSessionProxyAuthenticationCredentials];
		if (credentials) {
			if (!CFHTTPMessageApplyCredentialDictionary(request, (CFHTTPAuthenticationRef)[credentials objectForKey:@"Authentication"], (CFDictionaryRef)[credentials objectForKey:@"Credentials"], NULL)) {
				[[self class] removeProxyAuthenticationCredentialsFromSessionStore:[credentials objectForKey:@"Credentials"]];
			}
		}
	}
}

- (void)applyCookieHeader
{
	// Add cookies from the persistent (mac os global) store
	if ([self useCookiePersistence]) {
		NSArray *cookies = [[NSHTTPCookieStorage sharedHTTPCookieStorage] cookiesForURL:[[self url] absoluteURL]];
		if (cookies) {
			[[self requestCookies] addObjectsFromArray:cookies];
		}
	}
	
	// Apply request cookies
	NSArray *cookies;
	if ([self mainRequest]) {
		cookies = [[self mainRequest] requestCookies];
	} else {
		cookies = [self requestCookies];
	}
	if ([cookies count] > 0) {
		NSHTTPCookie *cookie;
		NSString *cookieHeader = nil;
		for (cookie in cookies) {
			if (!cookieHeader) {
				cookieHeader = [NSString stringWithFormat: @"%@=%@",[cookie name],[cookie value]];
			} else {
				cookieHeader = [NSString stringWithFormat: @"%@; %@=%@",cookieHeader,[cookie name],[cookie value]];
			}
		}
		if (cookieHeader) {
			[self addRequestHeader:@"Cookie" value:cookieHeader];
		}
	}	
}

- (void)buildRequestHeaders
{
	if ([self haveBuiltRequestHeaders]) {
		return;
	}
	[self setHaveBuiltRequestHeaders:YES];
	
	if ([self mainRequest]) {
		for (NSString *header in [[self mainRequest] requestHeaders]) {
			[self addRequestHeader:header value:[[[self mainRequest] requestHeaders] valueForKey:header]];
		}
		return;
	}
	
	[self applyCookieHeader];
	
	// Build and set the user agent string if the request does not already have a custom user agent specified
	if (![[self requestHeaders] objectForKey:@"User-Agent"]) {
		NSString *tempUserAgentString = [self userAgentString];
		if (!tempUserAgentString) {
			tempUserAgentString = [ASIHTTPRequest defaultUserAgentString];
		}
		if (tempUserAgentString) {
			[self addRequestHeader:@"User-Agent" value:tempUserAgentString];
		}
	}
	
	
	// Accept a compressed response
	if ([self allowCompressedResponse]) {
		[self addRequestHeader:@"Accept-Encoding" value:@"gzip"];
	}
	
	// Configure a compressed request body
	if ([self shouldCompressRequestBody]) {
		[self addRequestHeader:@"Content-Encoding" value:@"gzip"];
	}
	
	// Should this request resume an existing download?
	[self updatePartialDownloadSize];
	if ([self partialDownloadSize]) {
		[self addRequestHeader:@"Range" value:[NSString stringWithFormat:@"bytes=%llu-",[self partialDownloadSize]]];
	}
}

- (void)updatePartialDownloadSize
{
	NSFileManager *fileManager = [[[NSFileManager alloc] init] autorelease];

	if ([self allowResumeForFileDownloads] && [self downloadDestinationPath] && [self temporaryFileDownloadPath] && [fileManager fileExistsAtPath:[self temporaryFileDownloadPath]]) {
		NSError *err = nil;
		[self setPartialDownloadSize:[[fileManager attributesOfItemAtPath:[self temporaryFileDownloadPath] error:&err] fileSize]];
		if (err) {
			[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIFileManagementError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Failed to get attributes for file at path '%@'",[self temporaryFileDownloadPath]],NSLocalizedDescriptionKey,error,NSUnderlyingErrorKey,nil]]];
			return;
		}
	}
}

- (void)startRequest
{
	if ([self isCancelled]) {
		return;
	}
	
	[self performSelectorOnMainThread:@selector(requestStarted) withObject:nil waitUntilDone:[NSThread isMainThread]];
	
	[self setDownloadComplete:NO];
	[self setComplete:NO];
	[self setTotalBytesRead:0];
	[self setLastBytesRead:0];
	
	if ([self redirectCount] == 0) {
		[self setOriginalURL:[self url]];
	}
	
	// If we're retrying a request, let's remove any progress we made
	if ([self lastBytesSent] > 0) {
		[self removeUploadProgressSoFar];
	}
	
	[self setLastBytesSent:0];
	[self setContentLength:0];
	[self setResponseHeaders:nil];
	if (![self downloadDestinationPath]) {
		[self setRawResponseData:[[[NSMutableData alloc] init] autorelease]];
    }
	
	
    //
	// Create the stream for the request
	//

	NSFileManager *fileManager = [[[NSFileManager alloc] init] autorelease];

	[self setReadStreamIsScheduled:NO];
	
	// Do we need to stream the request body from disk
	if ([self shouldStreamPostDataFromDisk] && [self postBodyFilePath] && [fileManager fileExistsAtPath:[self postBodyFilePath]]) {
		
		// Are we gzipping the request body?
		if ([self compressedPostBodyFilePath] && [fileManager fileExistsAtPath:[self compressedPostBodyFilePath]]) {
			[self setPostBodyReadStream:[ASIInputStream inputStreamWithFileAtPath:[self compressedPostBodyFilePath] request:self]];
		} else {
			[self setPostBodyReadStream:[ASIInputStream inputStreamWithFileAtPath:[self postBodyFilePath] request:self]];
		}
		[self setReadStream:[NSMakeCollectable(CFReadStreamCreateForStreamedHTTPRequest(kCFAllocatorDefault, request,(CFReadStreamRef)[self postBodyReadStream])) autorelease]];    
    } else {
		
		// If we have a request body, we'll stream it from memory using our custom stream, so that we can measure bandwidth use and it can be bandwidth-throttled if necessary
		if ([self postBody] && [[self postBody] length] > 0) {
			if ([self shouldCompressRequestBody] && [self compressedPostBody]) {
				[self setPostBodyReadStream:[ASIInputStream inputStreamWithData:[self compressedPostBody] request:self]];
			} else if ([self postBody]) {
				[self setPostBodyReadStream:[ASIInputStream inputStreamWithData:[self postBody] request:self]];
			}
			[self setReadStream:[NSMakeCollectable(CFReadStreamCreateForStreamedHTTPRequest(kCFAllocatorDefault, request,(CFReadStreamRef)[self postBodyReadStream])) autorelease]];
		
		} else {
			[self setReadStream:[NSMakeCollectable(CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, request)) autorelease]];
		}
	}

	if (![self readStream]) {
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIInternalErrorWhileBuildingRequestType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Unable to create read stream",NSLocalizedDescriptionKey,nil]]];
        return;
    }


    
    
    //
    // Handle SSL certificate settings
    //

    if([[[[self url] scheme] lowercaseString] isEqualToString:@"https"]) {       
       
        // Tell CFNetwork not to validate SSL certificates
        if (![self validatesSecureCertificate]) {
            // see: http://iphonedevelopment.blogspot.com/2010/05/nsstream-tcp-and-ssl.html
            
            NSDictionary *sslProperties = [[NSDictionary alloc] initWithObjectsAndKeys:
                                      [NSNumber numberWithBool:YES], kCFStreamSSLAllowsExpiredCertificates,
                                      [NSNumber numberWithBool:YES], kCFStreamSSLAllowsAnyRoot,
                                      [NSNumber numberWithBool:NO],  kCFStreamSSLValidatesCertificateChain,
                                      kCFNull,kCFStreamSSLPeerName,
                                      nil];
            
            CFReadStreamSetProperty((CFReadStreamRef)[self readStream], 
                                    kCFStreamPropertySSLSettings, 
                                    (CFTypeRef)sslProperties);
            [sslProperties release];
        } 
        
        // Tell CFNetwork to use a client certificate
        if (clientCertificateIdentity) {
            NSMutableDictionary *sslProperties = [NSMutableDictionary dictionaryWithCapacity:1];
            
			NSMutableArray *certificates = [NSMutableArray arrayWithCapacity:[clientCertificates count]+1];

			// The first object in the array is our SecIdentityRef
			[certificates addObject:(id)clientCertificateIdentity];

			// If we've added any additional certificates, add them too
			for (id cert in clientCertificates) {
				[certificates addObject:cert];
			}
            
            [sslProperties setObject:certificates forKey:(NSString *)kCFStreamSSLCertificates];
            
            CFReadStreamSetProperty((CFReadStreamRef)[self readStream], kCFStreamPropertySSLSettings, sslProperties);
        }
        
    }

	//
	// Handle proxy settings
	//

 	if ([self proxyHost] && [self proxyPort]) {
		NSString *hostKey;
		NSString *portKey;

		if (![self proxyType]) {
			[self setProxyType:(NSString *)kCFProxyTypeHTTP];
		}

		if ([[self proxyType] isEqualToString:(NSString *)kCFProxyTypeSOCKS]) {
			hostKey = (NSString *)kCFStreamPropertySOCKSProxyHost;
			portKey = (NSString *)kCFStreamPropertySOCKSProxyPort;
		} else {
			hostKey = (NSString *)kCFStreamPropertyHTTPProxyHost;
			portKey = (NSString *)kCFStreamPropertyHTTPProxyPort;
			if ([[[[self url] scheme] lowercaseString] isEqualToString:@"https"]) {
				hostKey = (NSString *)kCFStreamPropertyHTTPSProxyHost;
				portKey = (NSString *)kCFStreamPropertyHTTPSProxyPort;
			}
		}
		NSMutableDictionary *proxyToUse = [NSMutableDictionary dictionaryWithObjectsAndKeys:[self proxyHost],hostKey,[NSNumber numberWithInt:[self proxyPort]],portKey,nil];

		if ([[self proxyType] isEqualToString:(NSString *)kCFProxyTypeSOCKS]) {
			CFReadStreamSetProperty((CFReadStreamRef)[self readStream], kCFStreamPropertySOCKSProxy, proxyToUse);
		} else {
			CFReadStreamSetProperty((CFReadStreamRef)[self readStream], kCFStreamPropertyHTTPProxy, proxyToUse);
		}
	}


	//
	// Handle persistent connections
	//
	
	[ASIHTTPRequest expirePersistentConnections];

	[connectionsLock lock];
	
	
	if (![[self url] host] || ![[self url] scheme]) {
		[self setConnectionInfo:nil];
		[self setShouldAttemptPersistentConnection:NO];
	}
	
	// Will store the old stream that was using this connection (if there was one) so we can clean it up once we've opened our own stream
	NSInputStream *oldStream = nil;
	
	// Use a persistent connection if possible
	if ([self shouldAttemptPersistentConnection]) {
		

		// If we are redirecting, we will re-use the current connection only if we are connecting to the same server
		if ([self connectionInfo]) {
			
			if (![[[self connectionInfo] objectForKey:@"host"] isEqualToString:[[self url] host]] || ![[[self connectionInfo] objectForKey:@"scheme"] isEqualToString:[[self url] scheme]] || [(NSNumber *)[[self connectionInfo] objectForKey:@"port"] intValue] != [[[self url] port] intValue]) {
				[self setConnectionInfo:nil];

			// Check if we should have expired this connection
			} else if ([[[self connectionInfo] objectForKey:@"expires"] timeIntervalSinceNow] < 0) {
				#if DEBUG_PERSISTENT_CONNECTIONS
				ASI_DEBUG_LOG(@"[CONNECTION] Not re-using connection #%i because it has expired",[[[self connectionInfo] objectForKey:@"id"] intValue]);
				#endif
				[persistentConnectionsPool removeObject:[self connectionInfo]];
				[self setConnectionInfo:nil];

			} else if ([[self connectionInfo] objectForKey:@"request"] != nil) {
                //Some other request reused this connection already - we'll have to create a new one
				#if DEBUG_PERSISTENT_CONNECTIONS
                ASI_DEBUG_LOG(@"%@ - Not re-using connection #%i for request #%i because it is already used by request #%i",self,[[[self connectionInfo] objectForKey:@"id"] intValue],[[self requestID] intValue],[[[self connectionInfo] objectForKey:@"request"] intValue]);
				#endif
                [self setConnectionInfo:nil];
            }
		}
		
		
		
		if (![self connectionInfo] && [[self url] host] && [[self url] scheme]) { // We must have a proper url with a host and scheme, or this will explode
			
			// Look for a connection to the same server in the pool
			for (NSMutableDictionary *existingConnection in persistentConnectionsPool) {
				if (![existingConnection objectForKey:@"request"] && [[existingConnection objectForKey:@"host"] isEqualToString:[[self url] host]] && [[existingConnection objectForKey:@"scheme"] isEqualToString:[[self url] scheme]] && [(NSNumber *)[existingConnection objectForKey:@"port"] intValue] == [[[self url] port] intValue]) {
					[self setConnectionInfo:existingConnection];
				}
			}
		}
		
		if ([[self connectionInfo] objectForKey:@"stream"]) {
			oldStream = [[[self connectionInfo] objectForKey:@"stream"] retain];

		}
		
		// No free connection was found in the pool matching the server/scheme/port we're connecting to, we'll need to create a new one
		if (![self connectionInfo]) {
			[self setConnectionInfo:[NSMutableDictionary dictionary]];
			nextConnectionNumberToCreate++;
			[[self connectionInfo] setObject:[NSNumber numberWithInt:(int)nextConnectionNumberToCreate] forKey:@"id"];
			[[self connectionInfo] setObject:[[self url] host] forKey:@"host"];
			[[self connectionInfo] setObject:[NSNumber numberWithInt:[[[self url] port] intValue]] forKey:@"port"];
			[[self connectionInfo] setObject:[[self url] scheme] forKey:@"scheme"];
			[persistentConnectionsPool addObject:[self connectionInfo]];
		}
		
		// If we are retrying this request, it will already have a requestID
		if (![self requestID]) {
			nextRequestID++;
			[self setRequestID:[NSNumber numberWithUnsignedInt:nextRequestID]];
		}
		[[self connectionInfo] setObject:[self requestID] forKey:@"request"];		
		[[self connectionInfo] setObject:[self readStream] forKey:@"stream"];
		CFReadStreamSetProperty((CFReadStreamRef)[self readStream],  kCFStreamPropertyHTTPAttemptPersistentConnection, kCFBooleanTrue);
		
		#if DEBUG_PERSISTENT_CONNECTIONS
		ASI_DEBUG_LOG(@"[CONNECTION] Request #%@ will use connection #%i",[self requestID],[[[self connectionInfo] objectForKey:@"id"] intValue]);
		#endif
		
		
		// Tag the stream with an id that tells it which connection to use behind the scenes
		// See http://lists.apple.com/archives/macnetworkprog/2008/Dec/msg00001.html for details on this approach
		
		CFReadStreamSetProperty((CFReadStreamRef)[self readStream], CFSTR("ASIStreamID"), [[self connectionInfo] objectForKey:@"id"]);
	
	} else {
		#if DEBUG_PERSISTENT_CONNECTIONS
		ASI_DEBUG_LOG(@"[CONNECTION] Request %@ will not use a persistent connection",self);
		#endif
	}
	
	[connectionsLock unlock];

	// Schedule the stream
	if (![self readStreamIsScheduled] && (!throttleWakeUpTime || [throttleWakeUpTime timeIntervalSinceDate:[NSDate date]] < 0)) {
		[self scheduleReadStream];
	}
	
	BOOL streamSuccessfullyOpened = NO;


   // Start the HTTP connection
	CFStreamClientContext ctxt = {0, self, NULL, NULL, NULL};
    if (CFReadStreamSetClient((CFReadStreamRef)[self readStream], kNetworkEvents, ReadStreamClientCallBack, &ctxt)) {
		if (CFReadStreamOpen((CFReadStreamRef)[self readStream])) {
			streamSuccessfullyOpened = YES;
		}
	}
	
	// Here, we'll close the stream that was previously using this connection, if there was one
	// We've kept it open until now (when we've just opened a new stream) so that the new stream can make use of the old connection
	// http://lists.apple.com/archives/Macnetworkprog/2006/Mar/msg00119.html
	if (oldStream) {
		[oldStream close];
		[oldStream release];
		oldStream = nil;
	}

	if (!streamSuccessfullyOpened) {
		[self setConnectionCanBeReused:NO];
		[self destroyReadStream];
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIInternalErrorWhileBuildingRequestType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Unable to start HTTP connection",NSLocalizedDescriptionKey,nil]]];
		return;	
	}
	
	if (![self mainRequest]) {
		if ([self shouldResetUploadProgress]) {
			if ([self showAccurateProgress]) {
				[self incrementUploadSizeBy:(long long)[self postLength]];
			} else {
				[self incrementUploadSizeBy:1];	 
			}
			[ASIHTTPRequest updateProgressIndicator:&uploadProgressDelegate withProgress:0 ofTotal:1];
		}
		if ([self shouldResetDownloadProgress] && ![self partialDownloadSize]) {
			[ASIHTTPRequest updateProgressIndicator:&downloadProgressDelegate withProgress:0 ofTotal:1];
		}
	}	
	
	
	// Record when the request started, so we can timeout if nothing happens
	[self setLastActivityTime:[NSDate date]];
	[self setStatusTimer:[NSTimer timerWithTimeInterval:0.25 target:self selector:@selector(updateStatus:) userInfo:nil repeats:YES]];
	[[NSRunLoop currentRunLoop] addTimer:[self statusTimer] forMode:[self runLoopMode]];
}

- (void)setStatusTimer:(NSTimer *)timer
{
	CFRetain(self);
	// We must invalidate the old timer here, not before we've created and scheduled a new timer
	// This is because the timer may be the only thing retaining an asynchronous request
	if (statusTimer && timer != statusTimer) {
		[statusTimer invalidate];
		[statusTimer release];
	}
	statusTimer = [timer retain];
	CFRelease(self);
}

// This gets fired every 1/4 of a second to update the progress and work out if we need to timeout
- (void)updateStatus:(NSTimer*)timer
{
	[self checkRequestStatus];
	if (![self inProgress]) {
		[self setStatusTimer:nil];
	}
}

- (void)performRedirect
{
	[self setURL:[self redirectURL]];
	[self setComplete:YES];
	[self setNeedsRedirect:NO];
	[self setRedirectCount:[self redirectCount]+1];

	if ([self redirectCount] > RedirectionLimit) {
		// Some naughty / badly coded website is trying to force us into a redirection loop. This is not cool.
		[self failWithError:ASITooMuchRedirectionError];
		[self setComplete:YES];
	} else {
		// Go all the way back to the beginning and build the request again, so that we can apply any new cookies
		[self main];
	}
}

// Called by delegate to resume loading with a new url after the delegate received request:willRedirectToURL:
- (void)redirectToURL:(NSURL *)newURL
{
	[self setRedirectURL:newURL];
	[self performSelector:@selector(performRedirect) onThread:[[self class] threadForRequest:self] withObject:nil waitUntilDone:NO];
}

- (BOOL)shouldTimeOut
{
	NSTimeInterval secondsSinceLastActivity = [[NSDate date] timeIntervalSinceDate:lastActivityTime];
	// See if we need to timeout
	if ([self readStream] && [self readStreamIsScheduled] && [self lastActivityTime] && [self timeOutSeconds] > 0 && secondsSinceLastActivity > [self timeOutSeconds]) {
		
		// We have no body, or we've sent more than the upload buffer size,so we can safely time out here
		if ([self postLength] == 0 || ([self uploadBufferSize] > 0 && [self totalBytesSent] > [self uploadBufferSize])) {
			return YES;
			
		// ***Black magic warning***
		// We have a body, but we've taken longer than timeOutSeconds to upload the first small chunk of data
		// Since there's no reliable way to track upload progress for the first 32KB (iPhone) or 128KB (Mac) with CFNetwork, we'll be slightly more forgiving on the timeout, as there's a strong chance our connection is just very slow.
		} else if (secondsSinceLastActivity > [self timeOutSeconds]*1.5) {
			return YES;
		}
	}
	return NO;
}

- (void)checkRequestStatus
{
	// We won't let the request cancel while we're updating progress / checking for a timeout
	[[self cancelledLock] lock];
	// See if our NSOperationQueue told us to cancel
	if ([self isCancelled] || [self complete]) {
		[[self cancelledLock] unlock];
		return;
	}
	
	[self performThrottling];
	
	if ([self shouldTimeOut]) {			
		// Do we need to auto-retry this request?
		if ([self numberOfTimesToRetryOnTimeout] > [self retryCount]) {

			// If we are resuming a download, we may need to update the Range header to take account of data we've just downloaded
			[self updatePartialDownloadSize];
			if ([self partialDownloadSize]) {
				CFHTTPMessageSetHeaderFieldValue(request, (CFStringRef)@"Range", (CFStringRef)[NSString stringWithFormat:@"bytes=%llu-",[self partialDownloadSize]]);
			}
			[self setRetryCount:[self retryCount]+1];
			[self unscheduleReadStream];
			[[self cancelledLock] unlock];
			[self startRequest];
			return;
		}
		[self failWithError:ASIRequestTimedOutError];
		[self cancelLoad];
		[self setComplete:YES];
		[[self cancelledLock] unlock];
		return;
	}

	// readStream will be null if we aren't currently running (perhaps we're waiting for a delegate to supply credentials)
	if ([self readStream]) {
		
		// If we have a post body
		if ([self postLength]) {
		
			[self setLastBytesSent:totalBytesSent];	
			
			// Find out how much data we've uploaded so far
			[self setTotalBytesSent:[[NSMakeCollectable(CFReadStreamCopyProperty((CFReadStreamRef)[self readStream], kCFStreamPropertyHTTPRequestBytesWrittenCount)) autorelease] unsignedLongLongValue]];
			if (totalBytesSent > lastBytesSent) {
				
				// We've uploaded more data,  reset the timeout
				[self setLastActivityTime:[NSDate date]];
				[ASIHTTPRequest incrementBandwidthUsedInLastSecond:(unsigned long)(totalBytesSent-lastBytesSent)];		
						
				#if DEBUG_REQUEST_STATUS
				if ([self totalBytesSent] == [self postLength]) {
					ASI_DEBUG_LOG(@"[STATUS] Request %@ finished uploading data",self);
				}
				#endif
			}
		}
			
		[self updateProgressIndicators];

	}
	
	[[self cancelledLock] unlock];
}


// Cancel loading and clean up. DO NOT USE THIS TO CANCEL REQUESTS - use [request cancel] instead
- (void)cancelLoad
{
	// If we're in the middle of downloading a PAC file, let's stop that first
	if (PACFileReadStream) {
		[PACFileReadStream setDelegate:nil];
		[PACFileReadStream close];
		[self setPACFileReadStream:nil];
		[self setPACFileData:nil];
	} else if (PACFileRequest) {
		[PACFileRequest setDelegate:nil];
		[PACFileRequest cancel];
		[self setPACFileRequest:nil];
	}

    [self destroyReadStream];
	
	[[self postBodyReadStream] close];
	[self setPostBodyReadStream:nil];
	
    if ([self rawResponseData]) {
		if (![self complete]) {
			[self setRawResponseData:nil];
		}
	// If we were downloading to a file
	} else if ([self temporaryFileDownloadPath]) {
		[[self fileDownloadOutputStream] close];
		[self setFileDownloadOutputStream:nil];
		
		[[self inflatedFileDownloadOutputStream] close];
		[self setInflatedFileDownloadOutputStream:nil];
		
		// If we haven't said we might want to resume, let's remove the temporary file too
		if (![self complete]) {
			if (![self allowResumeForFileDownloads]) {
				[self removeTemporaryDownloadFile];
			}
			[self removeTemporaryUncompressedDownloadFile];
		}
	}
	
	// Clean up any temporary file used to store request body for streaming
	if (![self authenticationNeeded] && ![self willRetryRequest] && [self didCreateTemporaryPostDataFile]) {
		[self removeTemporaryUploadFile];
		[self removeTemporaryCompressedUploadFile];
		[self setDidCreateTemporaryPostDataFile:NO];
	}
}

#pragma mark HEAD request

// Used by ASINetworkQueue to create a HEAD request appropriate for this request with the same headers (though you can use it yourself)
- (ASIHTTPRequest *)HEADRequest
{
	ASIHTTPRequest *headRequest = [[self class] requestWithURL:[self url]];
	
	// Copy the properties that make sense for a HEAD request
	[headRequest setRequestHeaders:[[[self requestHeaders] mutableCopy] autorelease]];
	[headRequest setRequestCookies:[[[self requestCookies] mutableCopy] autorelease]];
	[headRequest setUseCookiePersistence:[self useCookiePersistence]];
	[headRequest setUseKeychainPersistence:[self useKeychainPersistence]];
	[headRequest setUseSessionPersistence:[self useSessionPersistence]];
	[headRequest setAllowCompressedResponse:[self allowCompressedResponse]];
	[headRequest setUsername:[self username]];
	[headRequest setPassword:[self password]];
	[headRequest setDomain:[self domain]];
	[headRequest setProxyUsername:[self proxyUsername]];
	[headRequest setProxyPassword:[self proxyPassword]];
	[headRequest setProxyDomain:[self proxyDomain]];
	[headRequest setProxyHost:[self proxyHost]];
	[headRequest setProxyPort:[self proxyPort]];
	[headRequest setProxyType:[self proxyType]];
	[headRequest setShouldPresentAuthenticationDialog:[self shouldPresentAuthenticationDialog]];
	[headRequest setShouldPresentProxyAuthenticationDialog:[self shouldPresentProxyAuthenticationDialog]];
	[headRequest setTimeOutSeconds:[self timeOutSeconds]];
	[headRequest setUseHTTPVersionOne:[self useHTTPVersionOne]];
	[headRequest setValidatesSecureCertificate:[self validatesSecureCertificate]];
    [headRequest setClientCertificateIdentity:clientCertificateIdentity];
	[headRequest setClientCertificates:[[clientCertificates copy] autorelease]];
	[headRequest setPACurl:[self PACurl]];
	[headRequest setShouldPresentCredentialsBeforeChallenge:[self shouldPresentCredentialsBeforeChallenge]];
	[headRequest setNumberOfTimesToRetryOnTimeout:[self numberOfTimesToRetryOnTimeout]];
	[headRequest setShouldUseRFC2616RedirectBehaviour:[self shouldUseRFC2616RedirectBehaviour]];
	[headRequest setShouldAttemptPersistentConnection:[self shouldAttemptPersistentConnection]];
	[headRequest setPersistentConnectionTimeoutSeconds:[self persistentConnectionTimeoutSeconds]];
	
	[headRequest setMainRequest:self];
	[headRequest setRequestMethod:@"HEAD"];
	return headRequest;
}


#pragma mark upload/download progress


- (void)updateProgressIndicators
{
	//Only update progress if this isn't a HEAD request used to preset the content-length
	if (![self mainRequest]) {
		if ([self showAccurateProgress] || ([self complete] && ![self updatedProgress])) {
			[self updateUploadProgress];
			[self updateDownloadProgress];
		}
	}
}

- (id)uploadProgressDelegate
{
	[[self cancelledLock] lock];
	id d = [[uploadProgressDelegate retain] autorelease];
	[[self cancelledLock] unlock];
	return d;
}

- (void)setUploadProgressDelegate:(id)newDelegate
{
	[[self cancelledLock] lock];
	uploadProgressDelegate = newDelegate;

	#if !TARGET_OS_IPHONE
	// If the uploadProgressDelegate is an NSProgressIndicator, we set its MaxValue to 1.0 so we can update it as if it were a UIProgressView
	double max = 1.0;
	[ASIHTTPRequest performSelector:@selector(setMaxValue:) onTarget:&uploadProgressDelegate withObject:nil amount:&max callerToRetain:nil];
	#endif
	[[self cancelledLock] unlock];
}

- (id)downloadProgressDelegate
{
	[[self cancelledLock] lock];
	id d = [[downloadProgressDelegate retain] autorelease];
	[[self cancelledLock] unlock];
	return d;
}

- (void)setDownloadProgressDelegate:(id)newDelegate
{
	[[self cancelledLock] lock];
	downloadProgressDelegate = newDelegate;

	#if !TARGET_OS_IPHONE
	// If the downloadProgressDelegate is an NSProgressIndicator, we set its MaxValue to 1.0 so we can update it as if it were a UIProgressView
	double max = 1.0;
	[ASIHTTPRequest performSelector:@selector(setMaxValue:) onTarget:&downloadProgressDelegate withObject:nil amount:&max callerToRetain:nil];	
	#endif
	[[self cancelledLock] unlock];
}


- (void)updateDownloadProgress
{
	// We won't update download progress until we've examined the headers, since we might need to authenticate
	if (![self responseHeaders] || [self needsRedirect] || !([self contentLength] || [self complete])) {
		return;
	}
		
	unsigned long long bytesReadSoFar = [self totalBytesRead]+[self partialDownloadSize];
	unsigned long long value = 0;
	
	if ([self showAccurateProgress] && [self contentLength]) {
		value = bytesReadSoFar-[self lastBytesRead];
		if (value == 0) {
			return;
		}
	} else {
		value = 1;
		[self setUpdatedProgress:YES];
	}
	if (!value) {
		return;
	}

	[ASIHTTPRequest performSelector:@selector(request:didReceiveBytes:) onTarget:&queue withObject:self amount:&value callerToRetain:self];
	[ASIHTTPRequest performSelector:@selector(request:didReceiveBytes:) onTarget:&downloadProgressDelegate withObject:self amount:&value callerToRetain:self];

	[ASIHTTPRequest updateProgressIndicator:&downloadProgressDelegate withProgress:[self totalBytesRead]+[self partialDownloadSize] ofTotal:[self contentLength]+[self partialDownloadSize]];

	#if NS_BLOCKS_AVAILABLE
    if (bytesReceivedBlock) {
		unsigned long long totalSize = [self contentLength] + [self partialDownloadSize];
		[self performBlockOnMainThread:^{ if (bytesReceivedBlock) { bytesReceivedBlock(value, totalSize); }}];
    }
	#endif
	[self setLastBytesRead:bytesReadSoFar];
}

- (void)updateUploadProgress
{
	if ([self isCancelled] || [self totalBytesSent] == 0) {
		return;
	}
	
	// If this is the first time we've written to the buffer, totalBytesSent will be the size of the buffer (currently seems to be 128KB on both Leopard and iPhone 2.2.1, 32KB on iPhone 3.0)
	// If request body is less than the buffer size, totalBytesSent will be the total size of the request body
	// We will remove this from any progress display, as kCFStreamPropertyHTTPRequestBytesWrittenCount does not tell us how much data has actually be written
	if ([self uploadBufferSize] == 0 && [self totalBytesSent] != [self postLength]) {
		[self setUploadBufferSize:[self totalBytesSent]];
		[self incrementUploadSizeBy:-(long long)[self uploadBufferSize]];
	}
	
	unsigned long long value = 0;
	
	if ([self showAccurateProgress]) {
		if ([self totalBytesSent] == [self postLength] || [self lastBytesSent] > 0) {
			value = [self totalBytesSent]-[self lastBytesSent];
		} else {
			return;
		}
	} else {
		value = 1;
		[self setUpdatedProgress:YES];
	}
	
	if (!value) {
		return;
	}
	
	[ASIHTTPRequest performSelector:@selector(request:didSendBytes:) onTarget:&queue withObject:self amount:&value callerToRetain:self];
	[ASIHTTPRequest performSelector:@selector(request:didSendBytes:) onTarget:&uploadProgressDelegate withObject:self amount:&value callerToRetain:self];
	[ASIHTTPRequest updateProgressIndicator:&uploadProgressDelegate withProgress:[self totalBytesSent]-[self uploadBufferSize] ofTotal:[self postLength]-[self uploadBufferSize]];

	#if NS_BLOCKS_AVAILABLE
    if(bytesSentBlock){
		unsigned long long totalSize = [self postLength];
		[self performBlockOnMainThread:^{ if (bytesSentBlock) { bytesSentBlock(value, totalSize); }}];
	}
	#endif
}


- (void)incrementDownloadSizeBy:(long long)length
{
	[ASIHTTPRequest performSelector:@selector(request:incrementDownloadSizeBy:) onTarget:&queue withObject:self amount:&length callerToRetain:self];
	[ASIHTTPRequest performSelector:@selector(request:incrementDownloadSizeBy:) onTarget:&downloadProgressDelegate withObject:self amount:&length callerToRetain:self];

	#if NS_BLOCKS_AVAILABLE
    if(downloadSizeIncrementedBlock){
		[self performBlockOnMainThread:^{ if (downloadSizeIncrementedBlock) { downloadSizeIncrementedBlock(length); }}];
    }
	#endif
}

- (void)incrementUploadSizeBy:(long long)length
{
	[ASIHTTPRequest performSelector:@selector(request:incrementUploadSizeBy:) onTarget:&queue withObject:self amount:&length callerToRetain:self];
	[ASIHTTPRequest performSelector:@selector(request:incrementUploadSizeBy:) onTarget:&uploadProgressDelegate withObject:self amount:&length callerToRetain:self];

	#if NS_BLOCKS_AVAILABLE
    if(uploadSizeIncrementedBlock) {
		[self performBlockOnMainThread:^{ if (uploadSizeIncrementedBlock) { uploadSizeIncrementedBlock(length); }}];
    }
	#endif
}


-(void)removeUploadProgressSoFar
{
	long long progressToRemove = -(long long)[self totalBytesSent];
	[ASIHTTPRequest performSelector:@selector(request:didSendBytes:) onTarget:&queue withObject:self amount:&progressToRemove callerToRetain:self];
	[ASIHTTPRequest performSelector:@selector(request:didSendBytes:) onTarget:&uploadProgressDelegate withObject:self amount:&progressToRemove callerToRetain:self];
	[ASIHTTPRequest updateProgressIndicator:&uploadProgressDelegate withProgress:0 ofTotal:[self postLength]];

	#if NS_BLOCKS_AVAILABLE
    if(bytesSentBlock){
		unsigned long long totalSize = [self postLength];
		[self performBlockOnMainThread:^{  if (bytesSentBlock) { bytesSentBlock((unsigned long long)progressToRemove, totalSize); }}];
	}
	#endif
}

#if NS_BLOCKS_AVAILABLE
- (void)performBlockOnMainThread:(ASIBasicBlock)block
{
	[self performSelectorOnMainThread:@selector(callBlock:) withObject:[[block copy] autorelease] waitUntilDone:[NSThread isMainThread]];
}

- (void)callBlock:(ASIBasicBlock)block
{
	block();
}
#endif


+ (void)performSelector:(SEL)selector onTarget:(id *)target withObject:(id)object amount:(void *)amount callerToRetain:(id)callerToRetain
{
	if ([*target respondsToSelector:selector]) {
		NSMethodSignature *signature = nil;
		signature = [*target methodSignatureForSelector:selector];
		NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:signature];

		[invocation setSelector:selector];
		
		int argumentNumber = 2;
		
		// If we got an object parameter, we pass a pointer to the object pointer
		if (object) {
			[invocation setArgument:&object atIndex:argumentNumber];
			argumentNumber++;
		}
		
		// For the amount we'll just pass the pointer directly so NSInvocation will call the method using the number itself rather than a pointer to it
		if (amount) {
			[invocation setArgument:amount atIndex:argumentNumber];
		}

        SEL callback = @selector(performInvocation:onTarget:releasingObject:);
        NSMethodSignature *cbSignature = [ASIHTTPRequest methodSignatureForSelector:callback];
        NSInvocation *cbInvocation = [NSInvocation invocationWithMethodSignature:cbSignature];
        [cbInvocation setSelector:callback];
        [cbInvocation setTarget:self];
        [cbInvocation setArgument:&invocation atIndex:2];
        [cbInvocation setArgument:&target atIndex:3];
		if (callerToRetain) {
			[cbInvocation setArgument:&callerToRetain atIndex:4];
		}

		CFRetain(invocation);

		// Used to pass in a request that we must retain until after the call
		// We're using CFRetain rather than [callerToRetain retain] so things to avoid earthquakes when using garbage collection
		if (callerToRetain) {
			CFRetain(callerToRetain);
		}
        [cbInvocation performSelectorOnMainThread:@selector(invoke) withObject:nil waitUntilDone:[NSThread isMainThread]];
    }
}

+ (void)performInvocation:(NSInvocation *)invocation onTarget:(id *)target releasingObject:(id)objectToRelease
{
    if (*target && [*target respondsToSelector:[invocation selector]]) {
        [invocation invokeWithTarget:*target];
    }
	CFRelease(invocation);
	if (objectToRelease) {
		CFRelease(objectToRelease);
	}
}
	
	
+ (void)updateProgressIndicator:(id *)indicator withProgress:(unsigned long long)progress ofTotal:(unsigned long long)total
{
	#if TARGET_OS_IPHONE
		// Cocoa Touch: UIProgressView
		SEL selector = @selector(setProgress:);
		float progressAmount = (float)((progress*1.0)/(total*1.0));
		
	#else
		// Cocoa: NSProgressIndicator
		double progressAmount = progressAmount = (progress*1.0)/(total*1.0);
		SEL selector = @selector(setDoubleValue:);
	#endif
	
	if (![*indicator respondsToSelector:selector]) {
		return;
	}
	
	[progressLock lock];
	[ASIHTTPRequest performSelector:selector onTarget:indicator withObject:nil amount:&progressAmount callerToRetain:nil];
	[progressLock unlock];
}


#pragma mark talking to delegates / calling blocks

/* ALWAYS CALLED ON MAIN THREAD! */
- (void)requestStarted
{
	if ([self error] || [self mainRequest]) {
		return;
	}
	if (delegate && [delegate respondsToSelector:didStartSelector]) {
		[delegate performSelector:didStartSelector withObject:self];
	}
	#if NS_BLOCKS_AVAILABLE
	if(startedBlock){
		startedBlock();
	}
	#endif
	if (queue && [queue respondsToSelector:@selector(requestStarted:)]) {
		[queue performSelector:@selector(requestStarted:) withObject:self];
	}
}

/* ALWAYS CALLED ON MAIN THREAD! */
- (void)requestRedirected
{
	if ([self error] || [self mainRequest]) {
		return;
	}

	if([[self delegate] respondsToSelector:@selector(requestRedirected:)]){
		[[self delegate] performSelector:@selector(requestRedirected:) withObject:self];
	}

	#if NS_BLOCKS_AVAILABLE
	if(requestRedirectedBlock){
		requestRedirectedBlock();
	}
	#endif
}


/* ALWAYS CALLED ON MAIN THREAD! */
- (void)requestReceivedResponseHeaders:(NSMutableDictionary *)newResponseHeaders
{
	if ([self error] || [self mainRequest]) {
		return;
	}

	if (delegate && [delegate respondsToSelector:didReceiveResponseHeadersSelector]) {
		[delegate performSelector:didReceiveResponseHeadersSelector withObject:self withObject:newResponseHeaders];
	}

	#if NS_BLOCKS_AVAILABLE
	if(headersReceivedBlock){
		headersReceivedBlock(newResponseHeaders);
    }
	#endif

	if (queue && [queue respondsToSelector:@selector(request:didReceiveResponseHeaders:)]) {
		[queue performSelector:@selector(request:didReceiveResponseHeaders:) withObject:self withObject:newResponseHeaders];
	}
}

/* ALWAYS CALLED ON MAIN THREAD! */
- (void)requestWillRedirectToURL:(NSURL *)newURL
{
	if ([self error] || [self mainRequest]) {
		return;
	}
	if (delegate && [delegate respondsToSelector:willRedirectSelector]) {
		[delegate performSelector:willRedirectSelector withObject:self withObject:newURL];
	}
	if (queue && [queue respondsToSelector:@selector(request:willRedirectToURL:)]) {
		[queue performSelector:@selector(request:willRedirectToURL:) withObject:self withObject:newURL];
	}
}

// Subclasses might override this method to process the result in the same thread
// If you do this, don't forget to call [super requestFinished] to let the queue / delegate know we're done
- (void)requestFinished
{
#if DEBUG_REQUEST_STATUS || DEBUG_THROTTLING
	ASI_DEBUG_LOG(@"[STATUS] Request finished: %@",self);
#endif
	if ([self error] || [self mainRequest]) {
		return;
	}
	if ([self isPACFileRequest]) {
		[self reportFinished];
	} else {
		[self performSelectorOnMainThread:@selector(reportFinished) withObject:nil waitUntilDone:[NSThread isMainThread]];
	}
}

/* ALWAYS CALLED ON MAIN THREAD! */
- (void)reportFinished
{
	if (delegate && [delegate respondsToSelector:didFinishSelector]) {
		[delegate performSelector:didFinishSelector withObject:self];
	}

	#if NS_BLOCKS_AVAILABLE
	if(completionBlock){
		completionBlock();
	}
	#endif

	if (queue && [queue respondsToSelector:@selector(requestFinished:)]) {
		[queue performSelector:@selector(requestFinished:) withObject:self];
	}
}

/* ALWAYS CALLED ON MAIN THREAD! */
- (void)reportFailure
{
	if (delegate && [delegate respondsToSelector:didFailSelector]) {
		[delegate performSelector:didFailSelector withObject:self];
	}

	#if NS_BLOCKS_AVAILABLE
    if(failureBlock){
        failureBlock();
    }
	#endif

	if (queue && [queue respondsToSelector:@selector(requestFailed:)]) {
		[queue performSelector:@selector(requestFailed:) withObject:self];
	}
}

/* ALWAYS CALLED ON MAIN THREAD! */
- (void)passOnReceivedData:(NSData *)data
{
	if (delegate && [delegate respondsToSelector:didReceiveDataSelector]) {
		[delegate performSelector:didReceiveDataSelector withObject:self withObject:data];
	}

	#if NS_BLOCKS_AVAILABLE
	if (dataReceivedBlock) {
		dataReceivedBlock(data);
	}
	#endif
}

// Subclasses might override this method to perform error handling in the same thread
// If you do this, don't forget to call [super failWithError:] to let the queue / delegate know we're done
- (void)failWithError:(NSError *)theError
{
#if DEBUG_REQUEST_STATUS || DEBUG_THROTTLING
	ASI_DEBUG_LOG(@"[STATUS] Request %@: %@",self,(theError == ASIRequestCancelledError ? @"Cancelled" : @"Failed"));
#endif
	[self setComplete:YES];
	
	// Invalidate the current connection so subsequent requests don't attempt to reuse it
	if (theError && [theError code] != ASIAuthenticationErrorType && [theError code] != ASITooMuchRedirectionErrorType) {
		[connectionsLock lock];
		#if DEBUG_PERSISTENT_CONNECTIONS
		ASI_DEBUG_LOG(@"[CONNECTION] Request #%@ failed and will invalidate connection #%@",[self requestID],[[self connectionInfo] objectForKey:@"id"]);
		#endif
		[[self connectionInfo] removeObjectForKey:@"request"];
		[persistentConnectionsPool removeObject:[self connectionInfo]];
		[connectionsLock unlock];
		[self destroyReadStream];
	}
	if ([self connectionCanBeReused]) {
		[[self connectionInfo] setObject:[NSDate dateWithTimeIntervalSinceNow:[self persistentConnectionTimeoutSeconds]] forKey:@"expires"];
	}
	
    if ([self isCancelled] || [self error]) {
		return;
	}
	
	// If we have cached data, use it and ignore the error when using ASIFallbackToCacheIfLoadFailsCachePolicy
	if ([self downloadCache] && ([self cachePolicy] & ASIFallbackToCacheIfLoadFailsCachePolicy)) {
		if ([[self downloadCache] canUseCachedDataForRequest:self]) {
			[self useDataFromCache];
			return;
		}
	}
	
	
	[self setError:theError];
	
	ASIHTTPRequest *failedRequest = self;
	
	// If this is a HEAD request created by an ASINetworkQueue or compatible queue delegate, make the main request fail
	if ([self mainRequest]) {
		failedRequest = [self mainRequest];
		[failedRequest setError:theError];
	}

	if ([self isPACFileRequest]) {
		[failedRequest reportFailure];
	} else {
		[failedRequest performSelectorOnMainThread:@selector(reportFailure) withObject:nil waitUntilDone:[NSThread isMainThread]];
	}
	
    if (!inProgress)
    {
        // if we're not in progress, we can't notify the queue we've finished (doing so can cause a crash later on)
        // "markAsFinished" will be at the start of main() when we are started
        return;
    }
	[self markAsFinished];
}

#pragma mark parsing HTTP response headers

- (void)readResponseHeaders
{
	[self setAuthenticationNeeded:ASINoAuthenticationNeededYet];

	CFHTTPMessageRef message = (CFHTTPMessageRef)CFReadStreamCopyProperty((CFReadStreamRef)[self readStream], kCFStreamPropertyHTTPResponseHeader);
	if (!message) {
		return;
	}
	
	// Make sure we've received all the headers
	if (!CFHTTPMessageIsHeaderComplete(message)) {
		CFRelease(message);
		return;
	}

	#if DEBUG_REQUEST_STATUS
	if ([self totalBytesSent] == [self postLength]) {
		ASI_DEBUG_LOG(@"[STATUS] Request %@ received response headers",self);
	}
	#endif		

	[self setResponseHeaders:[NSMakeCollectable(CFHTTPMessageCopyAllHeaderFields(message)) autorelease]];
	[self setResponseStatusCode:(int)CFHTTPMessageGetResponseStatusCode(message)];
	[self setResponseStatusMessage:[NSMakeCollectable(CFHTTPMessageCopyResponseStatusLine(message)) autorelease]];

	if ([self downloadCache] && ([[self downloadCache] canUseCachedDataForRequest:self])) {

		// Update the expiry date
		[[self downloadCache] updateExpiryForRequest:self maxAge:[self secondsToCache]];

		// Read the response from the cache
		[self useDataFromCache];

		CFRelease(message);
		return;
	}

	// Is the server response a challenge for credentials?
	if ([self responseStatusCode] == 401) {
		[self setAuthenticationNeeded:ASIHTTPAuthenticationNeeded];
	} else if ([self responseStatusCode] == 407) {
		[self setAuthenticationNeeded:ASIProxyAuthenticationNeeded];
	} else {
		#if DEBUG_HTTP_AUTHENTICATION
		if ([self authenticationScheme]) {
			ASI_DEBUG_LOG(@"[AUTH] Request %@ has passed %@ authentication",self,[self authenticationScheme]);
		}
		#endif
	}
		
	// Authentication succeeded, or no authentication was required
	if (![self authenticationNeeded]) {

		// Did we get here without an authentication challenge? (which can happen when shouldPresentCredentialsBeforeChallenge is YES and basic auth was successful)
		if (!requestAuthentication && [[self authenticationScheme] isEqualToString:(NSString *)kCFHTTPAuthenticationSchemeBasic] && [self username] && [self password] && [self useSessionPersistence]) {

			#if DEBUG_HTTP_AUTHENTICATION
			ASI_DEBUG_LOG(@"[AUTH] Request %@ passed BASIC authentication, and will save credentials in the session store for future use",self);
			#endif
			
			NSMutableDictionary *newCredentials = [NSMutableDictionary dictionaryWithCapacity:2];
			[newCredentials setObject:[self username] forKey:(NSString *)kCFHTTPAuthenticationUsername];
			[newCredentials setObject:[self password] forKey:(NSString *)kCFHTTPAuthenticationPassword];
			
			// Store the credentials in the session 
			NSMutableDictionary *sessionCredentials = [NSMutableDictionary dictionary];
			[sessionCredentials setObject:newCredentials forKey:@"Credentials"];
			[sessionCredentials setObject:[self url] forKey:@"URL"];
			[sessionCredentials setObject:(NSString *)kCFHTTPAuthenticationSchemeBasic forKey:@"AuthenticationScheme"];
			[[self class] storeAuthenticationCredentialsInSessionStore:sessionCredentials];
		}
	}

	// Read response textEncoding
	[self parseStringEncodingFromHeaders];

	// Handle cookies
	NSArray *newCookies = [NSHTTPCookie cookiesWithResponseHeaderFields:[self responseHeaders] forURL:[self url]];
	[self setResponseCookies:newCookies];
	
	if ([self useCookiePersistence]) {
		
		// Store cookies in global persistent store
		[[NSHTTPCookieStorage sharedHTTPCookieStorage] setCookies:newCookies forURL:[self url] mainDocumentURL:nil];
		
		// We also keep any cookies in the sessionCookies array, so that we have a reference to them if we need to remove them later
		NSHTTPCookie *cookie;
		for (cookie in newCookies) {
			[ASIHTTPRequest addSessionCookie:cookie];
		}
	}
	
	// Do we need to redirect?
	if (![self willRedirect]) {
		// See if we got a Content-length header
		NSString *cLength = [responseHeaders valueForKey:@"Content-Length"];
		ASIHTTPRequest *theRequest = self;
		if ([self mainRequest]) {
			theRequest = [self mainRequest];
		}

		if (cLength) {
			unsigned long long length = strtoull([cLength UTF8String], NULL, 0);

			// Workaround for Apache HEAD requests for dynamically generated content returning the wrong Content-Length when using gzip
			if ([self mainRequest] && [self allowCompressedResponse] && length == 20 && [self showAccurateProgress] && [self shouldResetDownloadProgress]) {
				[[self mainRequest] setShowAccurateProgress:NO];
				[[self mainRequest] incrementDownloadSizeBy:1];

			} else {
				[theRequest setContentLength:length];
				if ([self showAccurateProgress] && [self shouldResetDownloadProgress]) {
					[theRequest incrementDownloadSizeBy:(long long)[theRequest contentLength]+(long long)[theRequest partialDownloadSize]];
				}
			}

		} else if ([self showAccurateProgress] && [self shouldResetDownloadProgress]) {
			[theRequest setShowAccurateProgress:NO];
			[theRequest incrementDownloadSizeBy:1];
		}
	}

	// Handle connection persistence
	if ([self shouldAttemptPersistentConnection]) {
		
		NSString *connectionHeader = [[[self responseHeaders] objectForKey:@"Connection"] lowercaseString];

		NSString *httpVersion = [NSMakeCollectable(CFHTTPMessageCopyVersion(message)) autorelease];
		
		// Don't re-use the connection if the server is HTTP 1.0 and didn't send Connection: Keep-Alive
		if (![httpVersion isEqualToString:(NSString *)kCFHTTPVersion1_0] || [connectionHeader isEqualToString:@"keep-alive"]) {

			// See if server explicitly told us to close the connection
			if (![connectionHeader isEqualToString:@"close"]) {
				
				NSString *keepAliveHeader = [[self responseHeaders] objectForKey:@"Keep-Alive"];
				
				// If we got a keep alive header, we'll reuse the connection for as long as the server tells us
				if (keepAliveHeader) { 
					int timeout = 0;
					int max = 0;
					NSScanner *scanner = [NSScanner scannerWithString:keepAliveHeader];
					[scanner scanString:@"timeout=" intoString:NULL];
					[scanner scanInt:&timeout];
					[scanner scanUpToString:@"max=" intoString:NULL];
					[scanner scanString:@"max=" intoString:NULL];
					[scanner scanInt:&max];
					if (max > 5) {
						[self setConnectionCanBeReused:YES];
						[self setPersistentConnectionTimeoutSeconds:timeout];
						#if DEBUG_PERSISTENT_CONNECTIONS
							ASI_DEBUG_LOG(@"[CONNECTION] Got a keep-alive header, will keep this connection open for %f seconds", [self persistentConnectionTimeoutSeconds]);
						#endif					
					}
				
				// Otherwise, we'll assume we can keep this connection open
				} else {
					[self setConnectionCanBeReused:YES];
					#if DEBUG_PERSISTENT_CONNECTIONS
						ASI_DEBUG_LOG(@"[CONNECTION] Got no keep-alive header, will keep this connection open for %f seconds", [self persistentConnectionTimeoutSeconds]);
					#endif
				}
			}
		}
	}

	CFRelease(message);
	[self performSelectorOnMainThread:@selector(requestReceivedResponseHeaders:) withObject:[[[self responseHeaders] copy] autorelease] waitUntilDone:[NSThread isMainThread]];
}

- (BOOL)willRedirect
{
	// Do we need to redirect?
	if (![self shouldRedirect] || ![responseHeaders valueForKey:@"Location"]) {
		return NO;
	}

	// Note that ASIHTTPRequest does not currently support 305 Use Proxy
	int responseCode = [self responseStatusCode];
	if (responseCode != 301 && responseCode != 302 && responseCode != 303 && responseCode != 307) {
		return NO;
	}

	[self performSelectorOnMainThread:@selector(requestRedirected) withObject:nil waitUntilDone:[NSThread isMainThread]];

	// By default, we redirect 301 and 302 response codes as GET requests
	// According to RFC 2616 this is wrong, but this is what most browsers do, so it's probably what you're expecting to happen
	// See also:
	// http://allseeing-i.lighthouseapp.com/projects/27881/tickets/27-302-redirection-issue

	if (responseCode != 307 && (![self shouldUseRFC2616RedirectBehaviour] || responseCode == 303)) {
		[self setRequestMethod:@"GET"];
		[self setPostBody:nil];
		[self setPostLength:0];

		// Perhaps there are other headers we should be preserving, but it's hard to know what we need to keep and what to throw away.
		NSString *userAgentHeader = [[self requestHeaders] objectForKey:@"User-Agent"];
		NSString *acceptHeader = [[self requestHeaders] objectForKey:@"Accept"];
		[self setRequestHeaders:nil];
		if (userAgentHeader) {
			[self addRequestHeader:@"User-Agent" value:userAgentHeader];
		}
		if (acceptHeader) {
			[self addRequestHeader:@"Accept" value:acceptHeader];
		}
		[self setHaveBuiltRequestHeaders:NO];

	} else {
		// Force rebuild the cookie header incase we got some new cookies from this request
		// All other request headers will remain as they are for 301 / 302 redirects
		[self applyCookieHeader];
	}

	// Force the redirected request to rebuild the request headers (if not a 303, it will re-use old ones, and add any new ones)
	[self setRedirectURL:[[NSURL URLWithString:[responseHeaders valueForKey:@"Location"] relativeToURL:[self url]] absoluteURL]];
	[self setNeedsRedirect:YES];

	// Clear the request cookies
	// This means manually added cookies will not be added to the redirect request - only those stored in the global persistent store
	// But, this is probably the safest option - we might be redirecting to a different domain
	[self setRequestCookies:[NSMutableArray array]];

	#if DEBUG_REQUEST_STATUS
	ASI_DEBUG_LOG(@"[STATUS] Request will redirect (code: %i): %@",responseCode,self);
	#endif

	return YES;
}

- (void)parseStringEncodingFromHeaders
{
	// Handle response text encoding
	NSStringEncoding charset = 0;
	NSString *mimeType = nil;
	[[self class] parseMimeType:&mimeType andResponseEncoding:&charset fromContentType:[[self responseHeaders] valueForKey:@"Content-Type"]];
	if (charset != 0) {
		[self setResponseEncoding:charset];
	} else {
		[self setResponseEncoding:[self defaultResponseEncoding]];
	}
}

#pragma mark http authentication

- (void)saveProxyCredentialsToKeychain:(NSDictionary *)newCredentials
{
	NSURLCredential *authenticationCredentials = [NSURLCredential credentialWithUser:[newCredentials objectForKey:(NSString *)kCFHTTPAuthenticationUsername] password:[newCredentials objectForKey:(NSString *)kCFHTTPAuthenticationPassword] persistence:NSURLCredentialPersistencePermanent];
	if (authenticationCredentials) {
		[ASIHTTPRequest saveCredentials:authenticationCredentials forProxy:[self proxyHost] port:[self proxyPort] realm:[self proxyAuthenticationRealm]];
	}	
}


- (void)saveCredentialsToKeychain:(NSDictionary *)newCredentials
{
	NSURLCredential *authenticationCredentials = [NSURLCredential credentialWithUser:[newCredentials objectForKey:(NSString *)kCFHTTPAuthenticationUsername] password:[newCredentials objectForKey:(NSString *)kCFHTTPAuthenticationPassword] persistence:NSURLCredentialPersistencePermanent];
	
	if (authenticationCredentials) {
		[ASIHTTPRequest saveCredentials:authenticationCredentials forHost:[[self url] host] port:[[[self url] port] intValue] protocol:[[self url] scheme] realm:[self authenticationRealm]];
	}	
}

- (BOOL)applyProxyCredentials:(NSDictionary *)newCredentials
{
	[self setProxyAuthenticationRetryCount:[self proxyAuthenticationRetryCount]+1];
	
	if (newCredentials && proxyAuthentication && request) {

		// Apply whatever credentials we've built up to the old request
		if (CFHTTPMessageApplyCredentialDictionary(request, proxyAuthentication, (CFMutableDictionaryRef)newCredentials, NULL)) {
			
			//If we have credentials and they're ok, let's save them to the keychain
			if (useKeychainPersistence) {
				[self saveProxyCredentialsToKeychain:newCredentials];
			}
			if (useSessionPersistence) {
				NSMutableDictionary *sessionProxyCredentials = [NSMutableDictionary dictionary];
				[sessionProxyCredentials setObject:(id)proxyAuthentication forKey:@"Authentication"];
				[sessionProxyCredentials setObject:newCredentials forKey:@"Credentials"];
				[sessionProxyCredentials setObject:[self proxyHost] forKey:@"Host"];
				[sessionProxyCredentials setObject:[NSNumber numberWithInt:[self proxyPort]] forKey:@"Port"];
				[sessionProxyCredentials setObject:[self proxyAuthenticationScheme] forKey:@"AuthenticationScheme"];
				[[self class] storeProxyAuthenticationCredentialsInSessionStore:sessionProxyCredentials];
			}
			[self setProxyCredentials:newCredentials];
			return YES;
		} else {
			[[self class] removeProxyAuthenticationCredentialsFromSessionStore:newCredentials];
		}
	}
	return NO;
}

- (BOOL)applyCredentials:(NSDictionary *)newCredentials
{
	[self setAuthenticationRetryCount:[self authenticationRetryCount]+1];
	
	if (newCredentials && requestAuthentication && request) {
		// Apply whatever credentials we've built up to the old request
		if (CFHTTPMessageApplyCredentialDictionary(request, requestAuthentication, (CFMutableDictionaryRef)newCredentials, NULL)) {
			
			//If we have credentials and they're ok, let's save them to the keychain
			if (useKeychainPersistence) {
				[self saveCredentialsToKeychain:newCredentials];
			}
			if (useSessionPersistence) {
				
				NSMutableDictionary *sessionCredentials = [NSMutableDictionary dictionary];
				[sessionCredentials setObject:(id)requestAuthentication forKey:@"Authentication"];
				[sessionCredentials setObject:newCredentials forKey:@"Credentials"];
				[sessionCredentials setObject:[self url] forKey:@"URL"];
				[sessionCredentials setObject:[self authenticationScheme] forKey:@"AuthenticationScheme"];
				if ([self authenticationRealm]) {
					[sessionCredentials setObject:[self authenticationRealm] forKey:@"AuthenticationRealm"];
				}
				[[self class] storeAuthenticationCredentialsInSessionStore:sessionCredentials];

			}
			[self setRequestCredentials:newCredentials];
			return YES;
		} else {
			[[self class] removeAuthenticationCredentialsFromSessionStore:newCredentials];
		}
	}
	return NO;
}

- (NSMutableDictionary *)findProxyCredentials
{
	NSMutableDictionary *newCredentials = [[[NSMutableDictionary alloc] init] autorelease];
	
	NSString *user = nil;
	NSString *pass = nil;
	
	ASIHTTPRequest *theRequest = [self mainRequest];
	// If this is a HEAD request generated by an ASINetworkQueue, we'll try to use the details from the main request
	if ([theRequest proxyUsername] && [theRequest proxyPassword]) {
		user = [theRequest proxyUsername];
		pass = [theRequest proxyPassword];
		
	// Let's try to use the ones set in this object
	} else if ([self proxyUsername] && [self proxyPassword]) {
		user = [self proxyUsername];
		pass = [self proxyPassword];
	}

	// When we connect to a website using NTLM via a proxy, we will use the main credentials
	if ((!user || !pass) && [self proxyAuthenticationScheme] == (NSString *)kCFHTTPAuthenticationSchemeNTLM) {
		user = [self username];
		pass = [self password];
	}


	
	// Ok, that didn't work, let's try the keychain
	// For authenticating proxies, we'll look in the keychain regardless of the value of useKeychainPersistence
	if ((!user || !pass)) {
		NSURLCredential *authenticationCredentials = [ASIHTTPRequest savedCredentialsForProxy:[self proxyHost] port:[self proxyPort] protocol:[[self url] scheme] realm:[self proxyAuthenticationRealm]];
		if (authenticationCredentials) {
			user = [authenticationCredentials user];
			pass = [authenticationCredentials password];
		}
		
	}

	// Handle NTLM, which requires a domain to be set too
	if (CFHTTPAuthenticationRequiresAccountDomain(proxyAuthentication)) {

		NSString *ntlmDomain = [self proxyDomain];

		// If we have no domain yet
		if (!ntlmDomain || [ntlmDomain length] == 0) {

			// Let's try to extract it from the username
			NSArray* ntlmComponents = [user componentsSeparatedByString:@"\\"];
			if ([ntlmComponents count] == 2) {
				ntlmDomain = [ntlmComponents objectAtIndex:0];
				user = [ntlmComponents objectAtIndex:1];

			// If we are connecting to a website using NTLM, but we are connecting via a proxy, the string we need may be in the domain property
			} else {
				ntlmDomain = [self domain];
			}
			if (!ntlmDomain) {
				ntlmDomain = @"";
			}
		}
		[newCredentials setObject:ntlmDomain forKey:(NSString *)kCFHTTPAuthenticationAccountDomain];
	}


	// If we have a username and password, let's apply them to the request and continue
	if (user && pass) {
		[newCredentials setObject:user forKey:(NSString *)kCFHTTPAuthenticationUsername];
		[newCredentials setObject:pass forKey:(NSString *)kCFHTTPAuthenticationPassword];
		return newCredentials;
	}
	return nil;
}


- (NSMutableDictionary *)findCredentials
{
	NSMutableDictionary *newCredentials = [[[NSMutableDictionary alloc] init] autorelease];

	// First, let's look at the url to see if the username and password were included
	NSString *user = [[self url] user];
	NSString *pass = [[self url] password];

	if (user && pass) {

		#if DEBUG_HTTP_AUTHENTICATION
		ASI_DEBUG_LOG(@"[AUTH] Request %@ will use credentials set on its url",self);
		#endif

	} else {
		
		// If this is a HEAD request generated by an ASINetworkQueue, we'll try to use the details from the main request
		if ([self mainRequest] && [[self mainRequest] username] && [[self mainRequest] password]) {
			user = [[self mainRequest] username];
			pass = [[self mainRequest] password];

			#if DEBUG_HTTP_AUTHENTICATION
			ASI_DEBUG_LOG(@"[AUTH] Request %@ will use credentials from its parent request",self);
			#endif

		// Let's try to use the ones set in this object
		} else if ([self username] && [self password]) {
			user = [self username];
			pass = [self password];

			#if DEBUG_HTTP_AUTHENTICATION
			ASI_DEBUG_LOG(@"[AUTH] Request %@ will use username and password properties as credentials",self);
			#endif
		}		
	}
	
	// Ok, that didn't work, let's try the keychain
	if ((!user || !pass) && useKeychainPersistence) {
		NSURLCredential *authenticationCredentials = [ASIHTTPRequest savedCredentialsForHost:[[self url] host] port:[[[self url] port] intValue] protocol:[[self url] scheme] realm:[self authenticationRealm]];
		if (authenticationCredentials) {
			user = [authenticationCredentials user];
			pass = [authenticationCredentials password];
			#if DEBUG_HTTP_AUTHENTICATION
			if (user && pass) {
				ASI_DEBUG_LOG(@"[AUTH] Request %@ will use credentials from the keychain",self);
			}
			#endif
		}
	}

	// Handle NTLM, which requires a domain to be set too
	if (CFHTTPAuthenticationRequiresAccountDomain(requestAuthentication)) {

		NSString *ntlmDomain = [self domain];

		// If we have no domain yet, let's try to extract it from the username
		if (!ntlmDomain || [ntlmDomain length] == 0) {
			ntlmDomain = @"";
			NSArray* ntlmComponents = [user componentsSeparatedByString:@"\\"];
			if ([ntlmComponents count] == 2) {
				ntlmDomain = [ntlmComponents objectAtIndex:0];
				user = [ntlmComponents objectAtIndex:1];
			}
		}
		[newCredentials setObject:ntlmDomain forKey:(NSString *)kCFHTTPAuthenticationAccountDomain];
	}

	// If we have a username and password, let's apply them to the request and continue
	if (user && pass) {
		[newCredentials setObject:user forKey:(NSString *)kCFHTTPAuthenticationUsername];
		[newCredentials setObject:pass forKey:(NSString *)kCFHTTPAuthenticationPassword];
		return newCredentials;
	}
	return nil;
}

// Called by delegate or authentication dialog to resume loading once authentication info has been populated
- (void)retryUsingSuppliedCredentials
{
	#if DEBUG_HTTP_AUTHENTICATION
		ASI_DEBUG_LOG(@"[AUTH] Request %@ received credentials from its delegate or an ASIAuthenticationDialog, will retry",self);
	#endif
	//If the url was changed by the delegate, our CFHTTPMessageRef will be NULL and we'll go back to the start
	if (!request) {
		[self performSelector:@selector(main) onThread:[[self class] threadForRequest:self] withObject:nil waitUntilDone:NO];
		return;
	}
	[self performSelector:@selector(attemptToApplyCredentialsAndResume) onThread:[[self class] threadForRequest:self] withObject:nil waitUntilDone:NO];
}

// Called by delegate or authentication dialog to cancel authentication
- (void)cancelAuthentication
{
	#if DEBUG_HTTP_AUTHENTICATION
		ASI_DEBUG_LOG(@"[AUTH] Request %@ had authentication cancelled by its delegate or an ASIAuthenticationDialog",self);
	#endif
	[self performSelector:@selector(failAuthentication) onThread:[[self class] threadForRequest:self] withObject:nil waitUntilDone:NO];
}

- (void)failAuthentication
{
	[self failWithError:ASIAuthenticationError];
}

- (BOOL)showProxyAuthenticationDialog
{
	if ([self isSynchronous]) {
		return NO;
	}

	// Mac authentication dialog coming soon!
	#if TARGET_OS_IPHONE
	if ([self shouldPresentProxyAuthenticationDialog]) {
		[ASIAuthenticationDialog performSelectorOnMainThread:@selector(presentAuthenticationDialogForRequest:) withObject:self waitUntilDone:[NSThread isMainThread]];
		return YES;
	}
	return NO;
	#else
	return NO;
	#endif
}


- (BOOL)willAskDelegateForProxyCredentials
{
	if ([self isSynchronous]) {
		return NO;
	}

	// If we have a delegate, we'll see if it can handle proxyAuthenticationNeededForRequest:.
	// Otherwise, we'll try the queue (if this request is part of one) and it will pass the message on to its own delegate
	id authenticationDelegate = [self delegate];
	if (!authenticationDelegate) {
		authenticationDelegate = [self queue];
	}
	
	BOOL delegateOrBlockWillHandleAuthentication = NO;

	if ([authenticationDelegate respondsToSelector:@selector(proxyAuthenticationNeededForRequest:)]) {
		delegateOrBlockWillHandleAuthentication = YES;
	}

	#if NS_BLOCKS_AVAILABLE
	if(proxyAuthenticationNeededBlock){
		delegateOrBlockWillHandleAuthentication = YES;
	}
	#endif

	if (delegateOrBlockWillHandleAuthentication) {
		[self performSelectorOnMainThread:@selector(askDelegateForProxyCredentials) withObject:nil waitUntilDone:NO];
	}
	
	return delegateOrBlockWillHandleAuthentication;
}

/* ALWAYS CALLED ON MAIN THREAD! */
- (void)askDelegateForProxyCredentials
{
	id authenticationDelegate = [self delegate];
	if (!authenticationDelegate) {
		authenticationDelegate = [self queue];
	}
	if ([authenticationDelegate respondsToSelector:@selector(proxyAuthenticationNeededForRequest:)]) {
		[authenticationDelegate performSelector:@selector(proxyAuthenticationNeededForRequest:) withObject:self];
		return;
	}
	#if NS_BLOCKS_AVAILABLE
	if(proxyAuthenticationNeededBlock){
		proxyAuthenticationNeededBlock();
	}
	#endif
}


- (BOOL)willAskDelegateForCredentials
{
	if ([self isSynchronous]) {
		return NO;
	}

	// If we have a delegate, we'll see if it can handle proxyAuthenticationNeededForRequest:.
	// Otherwise, we'll try the queue (if this request is part of one) and it will pass the message on to its own delegate
	id authenticationDelegate = [self delegate];
	if (!authenticationDelegate) {
		authenticationDelegate = [self queue];
	}

	BOOL delegateOrBlockWillHandleAuthentication = NO;

	if ([authenticationDelegate respondsToSelector:@selector(authenticationNeededForRequest:)]) {
		delegateOrBlockWillHandleAuthentication = YES;
	}

	#if NS_BLOCKS_AVAILABLE
	if (authenticationNeededBlock) {
		delegateOrBlockWillHandleAuthentication = YES;
	}
	#endif

	if (delegateOrBlockWillHandleAuthentication) {
		[self performSelectorOnMainThread:@selector(askDelegateForCredentials) withObject:nil waitUntilDone:NO];
	}
	return delegateOrBlockWillHandleAuthentication;
}

/* ALWAYS CALLED ON MAIN THREAD! */
- (void)askDelegateForCredentials
{
	id authenticationDelegate = [self delegate];
	if (!authenticationDelegate) {
		authenticationDelegate = [self queue];
	}
	
	if ([authenticationDelegate respondsToSelector:@selector(authenticationNeededForRequest:)]) {
		[authenticationDelegate performSelector:@selector(authenticationNeededForRequest:) withObject:self];
		return;
	}
	
	#if NS_BLOCKS_AVAILABLE
	if (authenticationNeededBlock) {
		authenticationNeededBlock();
	}
	#endif	
}

- (void)attemptToApplyProxyCredentialsAndResume
{
	
	if ([self error] || [self isCancelled]) {
		return;
	}
	
	// Read authentication data
	if (!proxyAuthentication) {
		CFHTTPMessageRef responseHeader = (CFHTTPMessageRef) CFReadStreamCopyProperty((CFReadStreamRef)[self readStream],kCFStreamPropertyHTTPResponseHeader);
		proxyAuthentication = CFHTTPAuthenticationCreateFromResponse(NULL, responseHeader);
		CFRelease(responseHeader);
		[self setProxyAuthenticationScheme:[NSMakeCollectable(CFHTTPAuthenticationCopyMethod(proxyAuthentication)) autorelease]];
	}
	
	// If we haven't got a CFHTTPAuthenticationRef by now, something is badly wrong, so we'll have to give up
	if (!proxyAuthentication) {
		[self cancelLoad];
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIInternalErrorWhileApplyingCredentialsType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Failed to get authentication object from response headers",NSLocalizedDescriptionKey,nil]]];
		return;
	}
	
	// Get the authentication realm
	[self setProxyAuthenticationRealm:nil];
	if (!CFHTTPAuthenticationRequiresAccountDomain(proxyAuthentication)) {
		[self setProxyAuthenticationRealm:[NSMakeCollectable(CFHTTPAuthenticationCopyRealm(proxyAuthentication)) autorelease]];
	}
	
	// See if authentication is valid
	CFStreamError err;		
	if (!CFHTTPAuthenticationIsValid(proxyAuthentication, &err)) {
		
		CFRelease(proxyAuthentication);
		proxyAuthentication = NULL;
		
		// check for bad credentials, so we can give the delegate a chance to replace them
		if (err.domain == kCFStreamErrorDomainHTTP && (err.error == kCFStreamErrorHTTPAuthenticationBadUserName || err.error == kCFStreamErrorHTTPAuthenticationBadPassword)) {
			
			// Prevent more than one request from asking for credentials at once
			[delegateAuthenticationLock lock];
			
			// We know the credentials we just presented are bad, we should remove them from the session store too
			[[self class] removeProxyAuthenticationCredentialsFromSessionStore:proxyCredentials];
			[self setProxyCredentials:nil];
			
			
			// If the user cancelled authentication via a dialog presented by another request, our queue may have cancelled us
			if ([self error] || [self isCancelled]) {
				[delegateAuthenticationLock unlock];
				return;
			}
			
			
			// Now we've acquired the lock, it may be that the session contains credentials we can re-use for this request
			if ([self useSessionPersistence]) {
				NSDictionary *credentials = [self findSessionProxyAuthenticationCredentials];
				if (credentials && [self applyProxyCredentials:[credentials objectForKey:@"Credentials"]]) {
					[delegateAuthenticationLock unlock];
					[self startRequest];
					return;
				}
			}
			
			[self setLastActivityTime:nil];
			
			if ([self willAskDelegateForProxyCredentials]) {
				[self attemptToApplyProxyCredentialsAndResume];
				[delegateAuthenticationLock unlock];
				return;
			}
			if ([self showProxyAuthenticationDialog]) {
				[self attemptToApplyProxyCredentialsAndResume];
				[delegateAuthenticationLock unlock];
				return;
			}
			[delegateAuthenticationLock unlock];
		}
		[self cancelLoad];
		[self failWithError:ASIAuthenticationError];
		return;
	}

	[self cancelLoad];
	
	if (proxyCredentials) {
		
		// We use startRequest rather than starting all over again in load request because NTLM requires we reuse the request
		if ((([self proxyAuthenticationScheme] != (NSString *)kCFHTTPAuthenticationSchemeNTLM) || [self proxyAuthenticationRetryCount] < 2) && [self applyProxyCredentials:proxyCredentials]) {
			[self startRequest];
			
		// We've failed NTLM authentication twice, we should assume our credentials are wrong
		} else if ([self proxyAuthenticationScheme] == (NSString *)kCFHTTPAuthenticationSchemeNTLM && [self proxyAuthenticationRetryCount] == 2) {
			[self failWithError:ASIAuthenticationError];
			
		// Something went wrong, we'll have to give up
		} else {
			[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIInternalErrorWhileApplyingCredentialsType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Failed to apply proxy credentials to request",NSLocalizedDescriptionKey,nil]]];
		}
		
	// Are a user name & password needed?
	}  else if (CFHTTPAuthenticationRequiresUserNameAndPassword(proxyAuthentication)) {
		
		// Prevent more than one request from asking for credentials at once
		[delegateAuthenticationLock lock];
		
		// If the user cancelled authentication via a dialog presented by another request, our queue may have cancelled us
		if ([self error] || [self isCancelled]) {
			[delegateAuthenticationLock unlock];
			return;
		}
		
		// Now we've acquired the lock, it may be that the session contains credentials we can re-use for this request
		if ([self useSessionPersistence]) {
			NSDictionary *credentials = [self findSessionProxyAuthenticationCredentials];
			if (credentials && [self applyProxyCredentials:[credentials objectForKey:@"Credentials"]]) {
				[delegateAuthenticationLock unlock];
				[self startRequest];
				return;
			}
		}
		
		NSMutableDictionary *newCredentials = [self findProxyCredentials];
		
		//If we have some credentials to use let's apply them to the request and continue
		if (newCredentials) {
			
			if ([self applyProxyCredentials:newCredentials]) {
				[delegateAuthenticationLock unlock];
				[self startRequest];
			} else {
				[delegateAuthenticationLock unlock];
				[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIInternalErrorWhileApplyingCredentialsType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Failed to apply proxy credentials to request",NSLocalizedDescriptionKey,nil]]];
			}
			
			return;
		}
		
		if ([self willAskDelegateForProxyCredentials]) {
			[delegateAuthenticationLock unlock];
			return;
		}
		
		if ([self showProxyAuthenticationDialog]) {
			[delegateAuthenticationLock unlock];
			return;
		}
		[delegateAuthenticationLock unlock];
		
		// The delegate isn't interested and we aren't showing the authentication dialog, we'll have to give up
		[self failWithError:ASIAuthenticationError];
		return;
	}
	
}

- (BOOL)showAuthenticationDialog
{
	if ([self isSynchronous]) {
		return NO;
	}
	// Mac authentication dialog coming soon!
	#if TARGET_OS_IPHONE
	if ([self shouldPresentAuthenticationDialog]) {
		[ASIAuthenticationDialog performSelectorOnMainThread:@selector(presentAuthenticationDialogForRequest:) withObject:self waitUntilDone:[NSThread isMainThread]];
		return YES;
	}
	return NO;
	#else
	return NO;
	#endif
}

- (void)attemptToApplyCredentialsAndResume
{
	if ([self error] || [self isCancelled]) {
		return;
	}
	
	// Do we actually need to authenticate with a proxy?
	if ([self authenticationNeeded] == ASIProxyAuthenticationNeeded) {
		[self attemptToApplyProxyCredentialsAndResume];
		return;
	}
	
	// Read authentication data
	if (!requestAuthentication) {
		CFHTTPMessageRef responseHeader = (CFHTTPMessageRef) CFReadStreamCopyProperty((CFReadStreamRef)[self readStream],kCFStreamPropertyHTTPResponseHeader);
		requestAuthentication = CFHTTPAuthenticationCreateFromResponse(NULL, responseHeader);
		CFRelease(responseHeader);
		[self setAuthenticationScheme:[NSMakeCollectable(CFHTTPAuthenticationCopyMethod(requestAuthentication)) autorelease]];
	}
	
	if (!requestAuthentication) {
		#if DEBUG_HTTP_AUTHENTICATION
		ASI_DEBUG_LOG(@"[AUTH] Request %@ failed to read authentication information from response headers",self);
		#endif

		[self cancelLoad];
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIInternalErrorWhileApplyingCredentialsType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Failed to get authentication object from response headers",NSLocalizedDescriptionKey,nil]]];
		return;
	}
	
	// Get the authentication realm
	[self setAuthenticationRealm:nil];
	if (!CFHTTPAuthenticationRequiresAccountDomain(requestAuthentication)) {
		[self setAuthenticationRealm:[NSMakeCollectable(CFHTTPAuthenticationCopyRealm(requestAuthentication)) autorelease]];
	}
	
	#if DEBUG_HTTP_AUTHENTICATION
		NSString *realm = [self authenticationRealm];
		if (realm) {
			realm = [NSString stringWithFormat:@" (Realm: %@)",realm];
		} else {
			realm = @"";
		}
		if ([self authenticationScheme] != (NSString *)kCFHTTPAuthenticationSchemeNTLM || [self authenticationRetryCount] == 0) {
			ASI_DEBUG_LOG(@"[AUTH] Request %@ received 401 challenge and must authenticate using %@%@",self,[self authenticationScheme],realm);
		} else {
			ASI_DEBUG_LOG(@"[AUTH] Request %@ NTLM handshake step %i",self,[self authenticationRetryCount]+1);
		}
	#endif

	// See if authentication is valid
	CFStreamError err;		
	if (!CFHTTPAuthenticationIsValid(requestAuthentication, &err)) {
		
		CFRelease(requestAuthentication);
		requestAuthentication = NULL;
		
		// check for bad credentials, so we can give the delegate a chance to replace them
		if (err.domain == kCFStreamErrorDomainHTTP && (err.error == kCFStreamErrorHTTPAuthenticationBadUserName || err.error == kCFStreamErrorHTTPAuthenticationBadPassword)) {

			#if DEBUG_HTTP_AUTHENTICATION
			ASI_DEBUG_LOG(@"[AUTH] Request %@ had bad credentials, will remove them from the session store if they are cached",self);
			#endif

			// Prevent more than one request from asking for credentials at once
			[delegateAuthenticationLock lock];
			
			// We know the credentials we just presented are bad, we should remove them from the session store too
			[[self class] removeAuthenticationCredentialsFromSessionStore:requestCredentials];
			[self setRequestCredentials:nil];
			
			// If the user cancelled authentication via a dialog presented by another request, our queue may have cancelled us
			if ([self error] || [self isCancelled]) {

				#if DEBUG_HTTP_AUTHENTICATION
				ASI_DEBUG_LOG(@"[AUTH] Request %@ failed or was cancelled while waiting to access credentials",self);
				#endif

				[delegateAuthenticationLock unlock];
				return;
			}

			// Now we've acquired the lock, it may be that the session contains credentials we can re-use for this request
			if ([self useSessionPersistence]) {
				NSDictionary *credentials = [self findSessionAuthenticationCredentials];
				if (credentials && [self applyCredentials:[credentials objectForKey:@"Credentials"]]) {

					#if DEBUG_HTTP_AUTHENTICATION
					ASI_DEBUG_LOG(@"[AUTH] Request %@ will reuse cached credentials from the session (%@)",self,[credentials objectForKey:@"AuthenticationScheme"]);
					#endif

					[delegateAuthenticationLock unlock];
					[self startRequest];
					return;
				}
			}
			
			[self setLastActivityTime:nil];
			
			if ([self willAskDelegateForCredentials]) {

				#if DEBUG_HTTP_AUTHENTICATION
				ASI_DEBUG_LOG(@"[AUTH] Request %@ will ask its delegate for credentials to use",self);
				#endif

				[delegateAuthenticationLock unlock];
				return;
			}
			if ([self showAuthenticationDialog]) {

				#if DEBUG_HTTP_AUTHENTICATION
				ASI_DEBUG_LOG(@"[AUTH] Request %@ will ask ASIAuthenticationDialog for credentials",self);
				#endif

				[delegateAuthenticationLock unlock];
				return;
			}
			[delegateAuthenticationLock unlock];
		}

		#if DEBUG_HTTP_AUTHENTICATION
		ASI_DEBUG_LOG(@"[AUTH] Request %@ has no credentials to present and must give up",self);
		#endif

		[self cancelLoad];
		[self failWithError:ASIAuthenticationError];
		return;
	}
	
	[self cancelLoad];
	
	if (requestCredentials) {
		
		if ((([self authenticationScheme] != (NSString *)kCFHTTPAuthenticationSchemeNTLM) || [self authenticationRetryCount] < 2) && [self applyCredentials:requestCredentials]) {
			[self startRequest];
			
			// We've failed NTLM authentication twice, we should assume our credentials are wrong
		} else if ([self authenticationScheme] == (NSString *)kCFHTTPAuthenticationSchemeNTLM && [self authenticationRetryCount ] == 2) {
			#if DEBUG_HTTP_AUTHENTICATION
			ASI_DEBUG_LOG(@"[AUTH] Request %@ has failed NTLM authentication",self);
			#endif

			[self failWithError:ASIAuthenticationError];
			
		} else {

			#if DEBUG_HTTP_AUTHENTICATION
			ASI_DEBUG_LOG(@"[AUTH] Request %@ had credentials and they were not marked as bad, but we got a 401 all the same.",self);
			#endif

			[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIInternalErrorWhileApplyingCredentialsType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Failed to apply credentials to request",NSLocalizedDescriptionKey,nil]]];
		}
		
		// Are a user name & password needed?
	}  else if (CFHTTPAuthenticationRequiresUserNameAndPassword(requestAuthentication)) {
		
		// Prevent more than one request from asking for credentials at once
		[delegateAuthenticationLock lock];
		
		// If the user cancelled authentication via a dialog presented by another request, our queue may have cancelled us
		if ([self error] || [self isCancelled]) {

			#if DEBUG_HTTP_AUTHENTICATION
			ASI_DEBUG_LOG(@"[AUTH] Request %@ failed or was cancelled while waiting to access credentials",self);
			#endif

			[delegateAuthenticationLock unlock];
			return;
		}
		
		// Now we've acquired the lock, it may be that the session contains credentials we can re-use for this request
		if ([self useSessionPersistence]) {
			NSDictionary *credentials = [self findSessionAuthenticationCredentials];
			if (credentials && [self applyCredentials:[credentials objectForKey:@"Credentials"]]) {

				#if DEBUG_HTTP_AUTHENTICATION
				ASI_DEBUG_LOG(@"[AUTH] Request %@ will reuse cached credentials from the session (%@)",self,[credentials objectForKey:@"AuthenticationScheme"]);
				#endif

				[delegateAuthenticationLock unlock];
				[self startRequest];
				return;
			}
		}
		

		NSMutableDictionary *newCredentials = [self findCredentials];
		
		//If we have some credentials to use let's apply them to the request and continue
		if (newCredentials) {
			
			if ([self applyCredentials:newCredentials]) {
				[delegateAuthenticationLock unlock];
				[self startRequest];
			} else {
				#if DEBUG_HTTP_AUTHENTICATION
				ASI_DEBUG_LOG(@"[AUTH] Request %@ failed to apply credentials",self);
				#endif
				[delegateAuthenticationLock unlock];
				[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIInternalErrorWhileApplyingCredentialsType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Failed to apply credentials to request",NSLocalizedDescriptionKey,nil]]];
			}
			return;
		}
		if ([self willAskDelegateForCredentials]) {

			#if DEBUG_HTTP_AUTHENTICATION
			ASI_DEBUG_LOG(@"[AUTH] Request %@ will ask its delegate for credentials to use",self);
			#endif

			[delegateAuthenticationLock unlock];
			return;
		}
		if ([self showAuthenticationDialog]) {

			#if DEBUG_HTTP_AUTHENTICATION
			ASI_DEBUG_LOG(@"[AUTH] Request %@ will ask ASIAuthenticationDialog for credentials",self);
			#endif

			[delegateAuthenticationLock unlock];
			return;
		}

		#if DEBUG_HTTP_AUTHENTICATION
		ASI_DEBUG_LOG(@"[AUTH] Request %@ has no credentials to present and must give up",self);
		#endif
		[delegateAuthenticationLock unlock];
		[self failWithError:ASIAuthenticationError];
		return;
	}
	
}

- (void)addBasicAuthenticationHeaderWithUsername:(NSString *)theUsername andPassword:(NSString *)thePassword
{
	[self addRequestHeader:@"Authorization" value:[NSString stringWithFormat:@"Basic %@",[ASIHTTPRequest base64forData:[[NSString stringWithFormat:@"%@:%@",theUsername,thePassword] dataUsingEncoding:NSUTF8StringEncoding]]]];	
	[self setAuthenticationScheme:(NSString *)kCFHTTPAuthenticationSchemeBasic];

}


#pragma mark stream status handlers

- (void)handleNetworkEvent:(CFStreamEventType)type
{	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	[[self cancelledLock] lock];
	
	if ([self complete] || [self isCancelled]) {
		[[self cancelledLock] unlock];
		[pool drain];
		return;
	}

	CFRetain(self);

    // Dispatch the stream events.
    switch (type) {
        case kCFStreamEventHasBytesAvailable:
            [self handleBytesAvailable];
            break;
            
        case kCFStreamEventEndEncountered:
            [self handleStreamComplete];
            break;
            
        case kCFStreamEventErrorOccurred:
            [self handleStreamError];
            break;
            
        default:
            break;
    }
	
	[self performThrottling];
	
	[[self cancelledLock] unlock];
	
	if ([self downloadComplete] && [self needsRedirect]) {
		if (![self willAskDelegateToConfirmRedirect]) {
			[self performRedirect];
		}
	} else if ([self downloadComplete] && [self authenticationNeeded]) {
		[self attemptToApplyCredentialsAndResume];
	}

	CFRelease(self);
	[pool drain];
}

- (BOOL)willAskDelegateToConfirmRedirect
{
	// We must lock to ensure delegate / queue aren't changed while we check them
	[[self cancelledLock] lock];

	// Here we perform an initial check to see if either the delegate or the queue wants to be asked about the redirect, because if not we should redirect straight away
	// We will check again on the main thread later
	BOOL needToAskDelegateAboutRedirect = (([self delegate] && [[self delegate] respondsToSelector:[self willRedirectSelector]]) || ([self queue] && [[self queue] respondsToSelector:@selector(request:willRedirectToURL:)]));

	[[self cancelledLock] unlock];

	// Either the delegate or the queue's delegate is interested in being told when we are about to redirect
	if (needToAskDelegateAboutRedirect) {
		NSURL *newURL = [[[self redirectURL] copy] autorelease];
		[self setRedirectURL:nil];
		[self performSelectorOnMainThread:@selector(requestWillRedirectToURL:) withObject:newURL waitUntilDone:[NSThread isMainThread]];
		return true;
	}
	return false;
}

- (void)handleBytesAvailable
{
	if (![self responseHeaders]) {
		[self readResponseHeaders];
	}
	
	// If we've cancelled the load part way through (for example, after deciding to use a cached version)
	if ([self complete]) {
		return;
	}
	
	// In certain (presumably very rare) circumstances, handleBytesAvailable seems to be called when there isn't actually any data available
	// We'll check that there is actually data available to prevent blocking on CFReadStreamRead()
	// So far, I've only seen this in the stress tests, so it might never happen in real-world situations.
	if (!CFReadStreamHasBytesAvailable((CFReadStreamRef)[self readStream])) {
		return;
	}

	long long bufferSize = 16384;
	if (contentLength > 262144) {
		bufferSize = 262144;
	} else if (contentLength > 65536) {
		bufferSize = 65536;
	}
	
	// Reduce the buffer size if we're receiving data too quickly when bandwidth throttling is active
	// This just augments the throttling done in measureBandwidthUsage to reduce the amount we go over the limit
	
	if ([[self class] isBandwidthThrottled]) {
		[bandwidthThrottlingLock lock];
		if (maxBandwidthPerSecond > 0) {
			long long maxiumumSize  = (long long)maxBandwidthPerSecond-(long long)bandwidthUsedInLastSecond;
			if (maxiumumSize < 0) {
				// We aren't supposed to read any more data right now, but we'll read a single byte anyway so the CFNetwork's buffer isn't full
				bufferSize = 1;
			} else if (maxiumumSize/4 < bufferSize) {
				// We were going to fetch more data that we should be allowed, so we'll reduce the size of our read
				bufferSize = maxiumumSize/4;
			}
		}
		if (bufferSize < 1) {
			bufferSize = 1;
		}
		[bandwidthThrottlingLock unlock];
	}
	
	
    UInt8 buffer[bufferSize];
    NSInteger bytesRead = [[self readStream] read:buffer maxLength:sizeof(buffer)];

    // Less than zero is an error
    if (bytesRead < 0) {
        [self handleStreamError];
		
	// If zero bytes were read, wait for the EOF to come.
    } else if (bytesRead) {

		// If we are inflating the response on the fly
		NSData *inflatedData = nil;
		if ([self isResponseCompressed] && ![self shouldWaitToInflateCompressedResponses]) {
			if (![self dataDecompressor]) {
				[self setDataDecompressor:[ASIDataDecompressor decompressor]];
			}
			NSError *err = nil;
			inflatedData = [[self dataDecompressor] uncompressBytes:buffer length:(NSUInteger)bytesRead error:&err];
			if (err) {
				[self failWithError:err];
				return;
			}
		}
		
		[self setTotalBytesRead:[self totalBytesRead]+(NSUInteger)bytesRead];
		[self setLastActivityTime:[NSDate date]];

		// For bandwidth measurement / throttling
		[ASIHTTPRequest incrementBandwidthUsedInLastSecond:(NSUInteger)bytesRead];
		
		// If we need to redirect, and have automatic redirect on, and might be resuming a download, let's do nothing with the content
		if ([self needsRedirect] && [self shouldRedirect] && [self allowResumeForFileDownloads]) {
			return;
		}
		
		BOOL dataWillBeHandledExternally = NO;
		if ([[self delegate] respondsToSelector:[self didReceiveDataSelector]]) {
			dataWillBeHandledExternally = YES;
		}
		#if NS_BLOCKS_AVAILABLE
		if (dataReceivedBlock) {
			dataWillBeHandledExternally = YES;
		}
		#endif
		// Does the delegate want to handle the data manually?
		if (dataWillBeHandledExternally) {

			NSData *data = nil;
			if ([self isResponseCompressed] && ![self shouldWaitToInflateCompressedResponses]) {
				data = inflatedData;
			} else {
				data = [NSData dataWithBytes:buffer length:(NSUInteger)bytesRead];
			}
			[self performSelectorOnMainThread:@selector(passOnReceivedData:) withObject:data waitUntilDone:[NSThread isMainThread]];
			
		// Are we downloading to a file?
		} else if ([self downloadDestinationPath]) {
			BOOL append = NO;
			if (![self fileDownloadOutputStream]) {
				if (![self temporaryFileDownloadPath]) {
					[self setTemporaryFileDownloadPath:[NSTemporaryDirectory() stringByAppendingPathComponent:[[NSProcessInfo processInfo] globallyUniqueString]]];
				} else if ([self allowResumeForFileDownloads] && [[self requestHeaders] objectForKey:@"Range"]) {
					if ([[self responseHeaders] objectForKey:@"Content-Range"]) {
						append = YES;
					} else {
						[self incrementDownloadSizeBy:-(long long)[self partialDownloadSize]];
						[self setPartialDownloadSize:0];
					}
				}

				[self setFileDownloadOutputStream:[[[NSOutputStream alloc] initToFileAtPath:[self temporaryFileDownloadPath] append:append] autorelease]];
				[[self fileDownloadOutputStream] open];

			}
			[[self fileDownloadOutputStream] write:buffer maxLength:(NSUInteger)bytesRead];

			if ([self isResponseCompressed] && ![self shouldWaitToInflateCompressedResponses]) {
				
				if (![self inflatedFileDownloadOutputStream]) {
					if (![self temporaryUncompressedDataDownloadPath]) {
						[self setTemporaryUncompressedDataDownloadPath:[NSTemporaryDirectory() stringByAppendingPathComponent:[[NSProcessInfo processInfo] globallyUniqueString]]];
					}
					
					[self setInflatedFileDownloadOutputStream:[[[NSOutputStream alloc] initToFileAtPath:[self temporaryUncompressedDataDownloadPath] append:append] autorelease]];
					[[self inflatedFileDownloadOutputStream] open];
				}

				[[self inflatedFileDownloadOutputStream] write:[inflatedData bytes] maxLength:[inflatedData length]];
			}

			
		//Otherwise, let's add the data to our in-memory store
		} else {
			if ([self isResponseCompressed] && ![self shouldWaitToInflateCompressedResponses]) {
				[rawResponseData appendData:inflatedData];
			} else {
				[rawResponseData appendBytes:buffer length:(NSUInteger)bytesRead];
			}
		}
    }
}

- (void)handleStreamComplete
{	

#if DEBUG_REQUEST_STATUS
	ASI_DEBUG_LOG(@"[STATUS] Request %@ finished downloading data (%qu bytes)",self, [self totalBytesRead]);
#endif
	[self setStatusTimer:nil];
	[self setDownloadComplete:YES];
	
	if (![self responseHeaders]) {
		[self readResponseHeaders];
	}

	[progressLock lock];	
	// Find out how much data we've uploaded so far
	[self setLastBytesSent:totalBytesSent];	
	[self setTotalBytesSent:[[NSMakeCollectable(CFReadStreamCopyProperty((CFReadStreamRef)[self readStream], kCFStreamPropertyHTTPRequestBytesWrittenCount)) autorelease] unsignedLongLongValue]];
	[self setComplete:YES];
	if (![self contentLength]) {
		[self setContentLength:[self totalBytesRead]];
	}
	[self updateProgressIndicators];

	
	[[self postBodyReadStream] close];
	[self setPostBodyReadStream:nil];
	
	[self setDataDecompressor:nil];

	NSError *fileError = nil;
	
	// Delete up the request body temporary file, if it exists
	if ([self didCreateTemporaryPostDataFile] && ![self authenticationNeeded]) {
		[self removeTemporaryUploadFile];
		[self removeTemporaryCompressedUploadFile];
	}
	
	// Close the output stream as we're done writing to the file
	if ([self temporaryFileDownloadPath]) {
		
		[[self fileDownloadOutputStream] close];
		[self setFileDownloadOutputStream:nil];

		[[self inflatedFileDownloadOutputStream] close];
		[self setInflatedFileDownloadOutputStream:nil];

		// If we are going to redirect and we are resuming, let's ignore this download
		if ([self shouldRedirect] && [self needsRedirect] && [self allowResumeForFileDownloads]) {
		
		} else if ([self isResponseCompressed]) {
			
			// Decompress the file directly to the destination path
			if ([self shouldWaitToInflateCompressedResponses]) {
				[ASIDataDecompressor uncompressDataFromFile:[self temporaryFileDownloadPath] toFile:[self downloadDestinationPath] error:&fileError];

			// Response should already have been inflated, move the temporary file to the destination path
			} else {
				NSError *moveError = nil;
				[[[[NSFileManager alloc] init] autorelease] moveItemAtPath:[self temporaryUncompressedDataDownloadPath] toPath:[self downloadDestinationPath] error:&moveError];
				if (moveError) {
					fileError = [NSError errorWithDomain:NetworkRequestErrorDomain code:ASIFileManagementError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Failed to move file from '%@' to '%@'",[self temporaryFileDownloadPath],[self downloadDestinationPath]],NSLocalizedDescriptionKey,moveError,NSUnderlyingErrorKey,nil]];
				}
				[self setTemporaryUncompressedDataDownloadPath:nil];

			}
			[self removeTemporaryDownloadFile];

		} else {
	
			//Remove any file at the destination path
			NSError *moveError = nil;
			if (![[self class] removeFileAtPath:[self downloadDestinationPath] error:&moveError]) {
				fileError = moveError;

			}

			//Move the temporary file to the destination path
			if (!fileError) {
				[[[[NSFileManager alloc] init] autorelease] moveItemAtPath:[self temporaryFileDownloadPath] toPath:[self downloadDestinationPath] error:&moveError];
				if (moveError) {
					fileError = [NSError errorWithDomain:NetworkRequestErrorDomain code:ASIFileManagementError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Failed to move file from '%@' to '%@'",[self temporaryFileDownloadPath],[self downloadDestinationPath]],NSLocalizedDescriptionKey,moveError,NSUnderlyingErrorKey,nil]];
				}
				[self setTemporaryFileDownloadPath:nil];
			}
			
		}
	}
	
	// Save to the cache
	if ([self downloadCache] && ![self didUseCachedResponse]) {
		[[self downloadCache] storeResponseForRequest:self maxAge:[self secondsToCache]];
	}
	
	[progressLock unlock];

	
	[connectionsLock lock];
	if (![self connectionCanBeReused]) {
		[self unscheduleReadStream];
	}
	#if DEBUG_PERSISTENT_CONNECTIONS
	if ([self requestID]) {
		ASI_DEBUG_LOG(@"[CONNECTION] Request #%@ finished using connection #%@",[self requestID], [[self connectionInfo] objectForKey:@"id"]);
	}
	#endif
	[[self connectionInfo] removeObjectForKey:@"request"];
	[[self connectionInfo] setObject:[NSDate dateWithTimeIntervalSinceNow:[self persistentConnectionTimeoutSeconds]] forKey:@"expires"];
	[connectionsLock unlock];
	
	if (![self authenticationNeeded]) {
		[self destroyReadStream];
	}
	

	if (![self needsRedirect] && ![self authenticationNeeded] && ![self didUseCachedResponse]) {
		
		if (fileError) {
			[self failWithError:fileError];
		} else {
			[self requestFinished];
		}

		[self markAsFinished];
		
	// If request has asked delegate or ASIAuthenticationDialog for credentials
	} else if ([self authenticationNeeded]) {
        // Do nothing.
	}

}

- (void)markAsFinished
{
	// Autoreleased requests may well be dealloced here otherwise
	CFRetain(self);

	// dealloc won't be called when running with GC, so we'll clean these up now
	if (request) {
		CFRelease(request);
		request = nil;
	}
	if (requestAuthentication) {
		CFRelease(requestAuthentication);
		requestAuthentication = nil;
	}
	if (proxyAuthentication) {
		CFRelease(proxyAuthentication);
		proxyAuthentication = nil;
	}

    BOOL wasInProgress = inProgress;
    BOOL wasFinished = finished;

    if (!wasFinished)
        [self willChangeValueForKey:@"isFinished"];
    if (wasInProgress)
        [self willChangeValueForKey:@"isExecuting"];

	[self setInProgress:NO];
    finished = YES;

    if (wasInProgress)
        [self didChangeValueForKey:@"isExecuting"];
    if (!wasFinished)
        [self didChangeValueForKey:@"isFinished"];

	#if TARGET_OS_IPHONE && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_4_0
	if ([ASIHTTPRequest isMultitaskingSupported] && [self shouldContinueWhenAppEntersBackground]) {
		dispatch_async(dispatch_get_main_queue(), ^{
			if (backgroundTask != UIBackgroundTaskInvalid) {
				[[UIApplication sharedApplication] endBackgroundTask:backgroundTask];
				backgroundTask = UIBackgroundTaskInvalid;
			}
		});
	}
	#endif
	CFRelease(self);
}

- (void)useDataFromCache
{
	NSDictionary *headers = [[self downloadCache] cachedResponseHeadersForURL:[self url]];
	NSString *dataPath = [[self downloadCache] pathToCachedResponseDataForURL:[self url]];

	ASIHTTPRequest *theRequest = self;
	if ([self mainRequest]) {
		theRequest = [self mainRequest];
	}

	if (headers && dataPath) {

		[self setResponseStatusCode:[[headers objectForKey:@"X-ASIHTTPRequest-Response-Status-Code"] intValue]];
		[self setDidUseCachedResponse:YES];
		[theRequest setResponseHeaders:headers];

		if ([theRequest downloadDestinationPath]) {
			[theRequest setDownloadDestinationPath:dataPath];
		} else {
			[theRequest setRawResponseData:[NSMutableData dataWithData:[[self downloadCache] cachedResponseDataForURL:[self url]]]];
		}
		[theRequest setContentLength:(unsigned long long)[[[self responseHeaders] objectForKey:@"Content-Length"] longLongValue]];
		[theRequest setTotalBytesRead:[self contentLength]];

		[theRequest parseStringEncodingFromHeaders];

		[theRequest setResponseCookies:[NSHTTPCookie cookiesWithResponseHeaderFields:headers forURL:[self url]]];

		// See if we need to redirect
		if ([self willRedirect]) {
			if (![self willAskDelegateToConfirmRedirect]) {
				[self performRedirect];
			}
			return;
		}
	}

	[theRequest setComplete:YES];
	[theRequest setDownloadComplete:YES];

	// If we're pulling data from the cache without contacting the server at all, we won't have set originalURL yet
	if ([self redirectCount] == 0) {
		[theRequest setOriginalURL:[theRequest url]];
	}

	[theRequest updateProgressIndicators];
	[theRequest requestFinished];
	[theRequest markAsFinished];	
	if ([self mainRequest]) {
		[self markAsFinished];
	}
}

- (BOOL)retryUsingNewConnection
{
	if ([self retryCount] == 0) {

		[self setWillRetryRequest:YES];
		[self cancelLoad];
		[self setWillRetryRequest:NO];

		#if DEBUG_PERSISTENT_CONNECTIONS
			ASI_DEBUG_LOG(@"[CONNECTION] Request attempted to use connection #%@, but it has been closed - will retry with a new connection", [[self connectionInfo] objectForKey:@"id"]);
		#endif
		[connectionsLock lock];
		[[self connectionInfo] removeObjectForKey:@"request"];
		[persistentConnectionsPool removeObject:[self connectionInfo]];
		[self setConnectionInfo:nil];
		[connectionsLock unlock];
		[self setRetryCount:[self retryCount]+1];
		[self startRequest];
		return YES;
	}
	#if DEBUG_PERSISTENT_CONNECTIONS
		ASI_DEBUG_LOG(@"[CONNECTION] Request attempted to use connection #%@, but it has been closed - we have already retried with a new connection, so we must give up", [[self connectionInfo] objectForKey:@"id"]);
	#endif	
	return NO;
}

- (void)handleStreamError

{
	NSError *underlyingError = [NSMakeCollectable(CFReadStreamCopyError((CFReadStreamRef)[self readStream])) autorelease];

	if (![self error]) { // We may already have handled this error
		
		// First, check for a 'socket not connected', 'broken pipe' or 'connection lost' error
		// This may occur when we've attempted to reuse a connection that should have been closed
		// If we get this, we need to retry the request
		// We'll only do this once - if it happens again on retry, we'll give up
		// -1005 = kCFURLErrorNetworkConnectionLost - this doesn't seem to be declared on Mac OS 10.5
		if (([[underlyingError domain] isEqualToString:NSPOSIXErrorDomain] && ([underlyingError code] == ENOTCONN || [underlyingError code] == EPIPE)) 
			|| ([[underlyingError domain] isEqualToString:(NSString *)kCFErrorDomainCFNetwork] && [underlyingError code] == -1005)) {
			if ([self retryUsingNewConnection]) {
				return;
			}
		}
		
		NSString *reason = @"A connection failure occurred";
		
		// We'll use a custom error message for SSL errors, but you should always check underlying error if you want more details
		// For some reason SecureTransport.h doesn't seem to be available on iphone, so error codes hard-coded
		// Also, iPhone seems to handle errors differently from Mac OS X - a self-signed certificate returns a different error code on each platform, so we'll just provide a general error
		if ([[underlyingError domain] isEqualToString:NSOSStatusErrorDomain]) {
			if ([underlyingError code] <= -9800 && [underlyingError code] >= -9818) {
				reason = [NSString stringWithFormat:@"%@: SSL problem (Possible causes may include a bad/expired/self-signed certificate, clock set to wrong date)",reason];
			}
		}
		[self cancelLoad];
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIConnectionFailureErrorType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:reason,NSLocalizedDescriptionKey,underlyingError,NSUnderlyingErrorKey,nil]]];
	} else {
		[self cancelLoad];
	}
	[self checkRequestStatus];
}

#pragma mark managing the read stream

- (void)destroyReadStream
{
    if ([self readStream]) {
		[self unscheduleReadStream];
		if (![self connectionCanBeReused]) {
			[[self readStream] removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:[self runLoopMode]];
			[[self readStream] close];
		}
		[self setReadStream:nil];
    }	
}

- (void)scheduleReadStream
{
	if ([self readStream] && ![self readStreamIsScheduled]) {

		[connectionsLock lock];
		runningRequestCount++;
		if (shouldUpdateNetworkActivityIndicator) {
			[[self class] showNetworkActivityIndicator];
		}
		[connectionsLock unlock];

		// Reset the timeout
		[self setLastActivityTime:[NSDate date]];
		CFStreamClientContext ctxt = {0, self, NULL, NULL, NULL};
		CFReadStreamSetClient((CFReadStreamRef)[self readStream], kNetworkEvents, ReadStreamClientCallBack, &ctxt);
		[[self readStream] scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:[self runLoopMode]];
		[self setReadStreamIsScheduled:YES];
	}
}


- (void)unscheduleReadStream
{
	if ([self readStream] && [self readStreamIsScheduled]) {

		[connectionsLock lock];
		runningRequestCount--;
		if (shouldUpdateNetworkActivityIndicator && runningRequestCount == 0) {
			// This call will wait half a second before turning off the indicator
			// This can prevent flicker when you have a single request finish and then immediately start another request
			// We run this on the main thread because we have no guarantee this thread will have a runloop in 0.5 seconds time
			// We don't bother the cancel this call if we start a new request, because we'll check if requests are running before we hide it
			[[self class] performSelectorOnMainThread:@selector(hideNetworkActivityIndicatorAfterDelay) withObject:nil waitUntilDone:[NSThread isMainThread]];
		}
		[connectionsLock unlock];

		CFReadStreamSetClient((CFReadStreamRef)[self readStream], kCFStreamEventNone, NULL, NULL);
		[[self readStream] removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:[self runLoopMode]];
		[self setReadStreamIsScheduled:NO];
	}
}

#pragma mark cleanup

- (BOOL)removeTemporaryDownloadFile
{
	NSError *err = nil;
	if ([self temporaryFileDownloadPath]) {
		if (![[self class] removeFileAtPath:[self temporaryFileDownloadPath] error:&err]) {
			[self failWithError:err];
		}
		[self setTemporaryFileDownloadPath:nil];
	}
	return (!err);
}

- (BOOL)removeTemporaryUncompressedDownloadFile
{
	NSError *err = nil;
	if ([self temporaryUncompressedDataDownloadPath]) {
		if (![[self class] removeFileAtPath:[self temporaryUncompressedDataDownloadPath] error:&err]) {
			[self failWithError:err];
		}
		[self setTemporaryUncompressedDataDownloadPath:nil];
	}
	return (!err);
}

- (BOOL)removeTemporaryUploadFile
{
	NSError *err = nil;
	if ([self postBodyFilePath]) {
		if (![[self class] removeFileAtPath:[self postBodyFilePath] error:&err]) {
			[self failWithError:err];
		}
		[self setPostBodyFilePath:nil];
	}
	return (!err);
}

- (BOOL)removeTemporaryCompressedUploadFile
{
	NSError *err = nil;
	if ([self compressedPostBodyFilePath]) {
		if (![[self class] removeFileAtPath:[self compressedPostBodyFilePath] error:&err]) {
			[self failWithError:err];
		}
		[self setCompressedPostBodyFilePath:nil];
	}
	return (!err);
}

+ (BOOL)removeFileAtPath:(NSString *)path error:(NSError **)err
{
	NSFileManager *fileManager = [[[NSFileManager alloc] init] autorelease];

	if ([fileManager fileExistsAtPath:path]) {
		NSError *removeError = nil;
		[fileManager removeItemAtPath:path error:&removeError];
		if (removeError) {
			if (err) {
				*err = [NSError errorWithDomain:NetworkRequestErrorDomain code:ASIFileManagementError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"Failed to delete file at path '%@'",path],NSLocalizedDescriptionKey,removeError,NSUnderlyingErrorKey,nil]];
			}
			return NO;
		}
	}
	return YES;
}

#pragma mark Proxies

- (BOOL)configureProxies
{
	// Have details of the proxy been set on this request
	if (![self isPACFileRequest] && (![self proxyHost] && ![self proxyPort])) {

		// If not, we need to figure out what they'll be
		NSArray *proxies = nil;

		// Have we been given a proxy auto config file?
		if ([self PACurl]) {

			// If yes, we'll need to fetch the PAC file asynchronously, so we stop this request to wait until we have the proxy details.
			[self fetchPACFile];
			return NO;

			// Detect proxy settings and apply them
		} else {

#if TARGET_OS_IPHONE
			NSDictionary *proxySettings = [NSMakeCollectable(CFNetworkCopySystemProxySettings()) autorelease];
#else
			NSDictionary *proxySettings = [NSMakeCollectable(SCDynamicStoreCopyProxies(NULL)) autorelease];
#endif

			proxies = [NSMakeCollectable(CFNetworkCopyProxiesForURL((CFURLRef)[self url], (CFDictionaryRef)proxySettings)) autorelease];

			// Now check to see if the proxy settings contained a PAC url, we need to run the script to get the real list of proxies if so
			NSDictionary *settings = [proxies objectAtIndex:0];
			if ([settings objectForKey:(NSString *)kCFProxyAutoConfigurationURLKey]) {
				[self setPACurl:[settings objectForKey:(NSString *)kCFProxyAutoConfigurationURLKey]];
				[self fetchPACFile];
				return NO;
			}
		}

		if (!proxies) {
			[self setReadStream:nil];
			[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:ASIInternalErrorWhileBuildingRequestType userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Unable to obtain information on proxy servers needed for request",NSLocalizedDescriptionKey,nil]]];
			return NO;
		}
		// I don't really understand why the dictionary returned by CFNetworkCopyProxiesForURL uses different key names from CFNetworkCopySystemProxySettings/SCDynamicStoreCopyProxies
		// and why its key names are documented while those we actually need to use don't seem to be (passing the kCF* keys doesn't seem to work)
		if ([proxies count] > 0) {
			NSDictionary *settings = [proxies objectAtIndex:0];
			[self setProxyHost:[settings objectForKey:(NSString *)kCFProxyHostNameKey]];
			[self setProxyPort:[[settings objectForKey:(NSString *)kCFProxyPortNumberKey] intValue]];
			[self setProxyType:[settings objectForKey:(NSString *)kCFProxyTypeKey]];
		}
	}
	return YES;
}



// Attempts to download a PAC (Proxy Auto-Configuration) file
// PAC files at file://, http:// and https:// addresses are supported
- (void)fetchPACFile
{
	// For file:// urls, we'll use an async NSInputStream (ASIHTTPRequest does not support file:// urls)
	if ([[self PACurl] isFileURL]) {
		NSInputStream *stream = [[[NSInputStream alloc] initWithFileAtPath:[[self PACurl] path]] autorelease];
		[self setPACFileReadStream:stream];
		[stream setDelegate:(id)self];
		[stream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:[self runLoopMode]];
		[stream open];
		// If it takes more than timeOutSeconds to read the PAC, we'll just give up and assume no proxies
		// We won't bother to handle cases where the first part of the PAC is read within timeOutSeconds, but the whole thing takes longer
		// Either our PAC file is in easy reach, or it's going to slow things down to the point that it's probably better requests fail
		[self performSelector:@selector(timeOutPACRead) withObject:nil afterDelay:[self timeOutSeconds]];
		return;
	}

	NSString *scheme = [[[self PACurl] scheme] lowercaseString];
	if (![scheme isEqualToString:@"http"] && ![scheme isEqualToString:@"https"]) {
		// Don't know how to read data from this URL, we'll have to give up
		// We'll simply assume no proxies, and start the request as normal
		[self startRequest];
		return;
	}

	// Create an ASIHTTPRequest to fetch the PAC file
	ASIHTTPRequest *PACRequest = [ASIHTTPRequest requestWithURL:[self PACurl]];

	// Will prevent this request attempting to configure proxy settings for itself
	[PACRequest setIsPACFileRequest:YES];

	[PACRequest setTimeOutSeconds:[self timeOutSeconds]];

	// If we're a synchronous request, we'll download the PAC file synchronously
	if ([self isSynchronous]) {
		[PACRequest startSynchronous];
		if (![PACRequest error] && [PACRequest responseString]) {
			[self runPACScript:[PACRequest responseString]];
		}
		[self startRequest];
		return;
	}

	[self setPACFileRequest:PACRequest];

	// Force this request to run before others in the shared queue
	[PACRequest setQueuePriority:NSOperationQueuePriorityHigh];

	// We'll treat failure to download the PAC file the same as success - if we were unable to fetch a PAC file, we proceed as if we have no proxy server and let this request fail itself if necessary
	[PACRequest setDelegate:self];
	[PACRequest setDidFinishSelector:@selector(finishedDownloadingPACFile:)];
	[PACRequest setDidFailSelector:@selector(finishedDownloadingPACFile:)];
	[PACRequest startAsynchronous];

	// Temporarily increase the number of operations in the shared queue to give our request a chance to run
	[connectionsLock lock];
	[sharedQueue setMaxConcurrentOperationCount:[sharedQueue maxConcurrentOperationCount]+1];
	[connectionsLock unlock];
}

// Called as we read the PAC file from a file:// url
- (void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)eventCode
{
	if (![self PACFileReadStream]) {
		return;
	}
	if (eventCode == NSStreamEventHasBytesAvailable) {

		if (![self PACFileData]) {
			[self setPACFileData:[NSMutableData data]];
		}
		// If your PAC file is larger than 16KB, you're just being cruel.
		uint8_t buf[16384];
		NSInteger len = [(NSInputStream *)stream read:buf maxLength:16384];
		// Append only if something was actually read.
		if (len > 0) {
			[[self PACFileData] appendBytes:(const void *)buf length:(NSUInteger)len];
		}

	} else if (eventCode == NSStreamEventErrorOccurred || eventCode == NSStreamEventEndEncountered) {

		[NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(timeOutPACRead) object:nil];

		[stream close];
		[stream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:[self runLoopMode]];
		[self setPACFileReadStream:nil];

		if (eventCode == NSStreamEventEndEncountered) {
			// It sounds as though we have no idea what encoding a PAC file will use
			static NSStringEncoding encodingsToTry[2] = {NSUTF8StringEncoding,NSISOLatin1StringEncoding};
			NSUInteger i;
			for (i=0; i<2; i++) {
				NSString *pacScript =  [[[NSString alloc] initWithBytes:[[self PACFileData] bytes] length:[[self PACFileData] length] encoding:encodingsToTry[i]] autorelease];
				if (pacScript) {
					[self runPACScript:pacScript];
					break;
				}
			}
		}
		[self setPACFileData:nil];
		[self startRequest];
	}
}

// Called if it takes longer than timeOutSeconds to read the whole PAC file (when reading from a file:// url)
- (void)timeOutPACRead
{
	[self stream:[self PACFileReadStream] handleEvent:NSStreamEventErrorOccurred];
}

// Runs the downloaded PAC script
- (void)runPACScript:(NSString *)script
{
	if (script) {
		// From: http://developer.apple.com/samplecode/CFProxySupportTool/listing1.html
		// Work around <rdar://problem/5530166>.  This dummy call to 
		// CFNetworkCopyProxiesForURL initialise some state within CFNetwork 
		// that is required by CFNetworkCopyProxiesForAutoConfigurationScript.
		CFRelease(CFNetworkCopyProxiesForURL((CFURLRef)[self url], NULL));

		// Obtain the list of proxies by running the autoconfiguration script
		CFErrorRef err = NULL;
		NSArray *proxies = [NSMakeCollectable(CFNetworkCopyProxiesForAutoConfigurationScript((CFStringRef)script,(CFURLRef)[self url], &err)) autorelease];
		if (!err && [proxies count] > 0) {
			NSDictionary *settings = [proxies objectAtIndex:0];
			[self setProxyHost:[settings objectForKey:(NSString *)kCFProxyHostNameKey]];
			[self setProxyPort:[[settings objectForKey:(NSString *)kCFProxyPortNumberKey] intValue]];
			[self setProxyType:[settings objectForKey:(NSString *)kCFProxyTypeKey]];
		}
	}
}

// Called if we successfully downloaded a PAC file from a webserver
- (void)finishedDownloadingPACFile:(ASIHTTPRequest *)theRequest
{
	if (![theRequest error] && [theRequest responseString]) {
		[self runPACScript:[theRequest responseString]];
	}

	// Set the shared queue's maxConcurrentOperationCount back to normal
	[connectionsLock lock];
	[sharedQueue setMaxConcurrentOperationCount:[sharedQueue maxConcurrentOperationCount]-1];
	[connectionsLock unlock];

	// We no longer need our PAC file request
	[self setPACFileRequest:nil];

	// Start the request
	[self startRequest];
}


#pragma mark persistent connections

- (NSNumber *)connectionID
{
	return [[self connectionInfo] objectForKey:@"id"];
}

+ (void)expirePersistentConnections
{
	[connectionsLock lock];
	NSUInteger i;
	for (i=0; i<[persistentConnectionsPool count]; i++) {
		NSDictionary *existingConnection = [persistentConnectionsPool objectAtIndex:i];
		if (![existingConnection objectForKey:@"request"] && [[existingConnection objectForKey:@"expires"] timeIntervalSinceNow] <= 0) {
#if DEBUG_PERSISTENT_CONNECTIONS
			ASI_DEBUG_LOG(@"[CONNECTION] Closing connection #%i because it has expired",[[existingConnection objectForKey:@"id"] intValue]);
#endif
			NSInputStream *stream = [existingConnection objectForKey:@"stream"];
			if (stream) {
				[stream close];
			}
			[persistentConnectionsPool removeObject:existingConnection];
			i--;
		}
	}	
	[connectionsLock unlock];
}

#pragma mark NSCopying
- (id)copyWithZone:(NSZone *)zone
{
	// Don't forget - this will return a retained copy!
	ASIHTTPRequest *newRequest = [[[self class] alloc] initWithURL:[self url]];
	[newRequest setDelegate:[self delegate]];
	[newRequest setRequestMethod:[self requestMethod]];
	[newRequest setPostBody:[self postBody]];
	[newRequest setShouldStreamPostDataFromDisk:[self shouldStreamPostDataFromDisk]];
	[newRequest setPostBodyFilePath:[self postBodyFilePath]];
	[newRequest setRequestHeaders:[[[self requestHeaders] mutableCopyWithZone:zone] autorelease]];
	[newRequest setRequestCookies:[[[self requestCookies] mutableCopyWithZone:zone] autorelease]];
	[newRequest setUseCookiePersistence:[self useCookiePersistence]];
	[newRequest setUseKeychainPersistence:[self useKeychainPersistence]];
	[newRequest setUseSessionPersistence:[self useSessionPersistence]];
	[newRequest setAllowCompressedResponse:[self allowCompressedResponse]];
	[newRequest setDownloadDestinationPath:[self downloadDestinationPath]];
	[newRequest setTemporaryFileDownloadPath:[self temporaryFileDownloadPath]];
	[newRequest setUsername:[self username]];
	[newRequest setPassword:[self password]];
	[newRequest setDomain:[self domain]];
	[newRequest setProxyUsername:[self proxyUsername]];
	[newRequest setProxyPassword:[self proxyPassword]];
	[newRequest setProxyDomain:[self proxyDomain]];
	[newRequest setProxyHost:[self proxyHost]];
	[newRequest setProxyPort:[self proxyPort]];
	[newRequest setProxyType:[self proxyType]];
	[newRequest setUploadProgressDelegate:[self uploadProgressDelegate]];
	[newRequest setDownloadProgressDelegate:[self downloadProgressDelegate]];
	[newRequest setShouldPresentAuthenticationDialog:[self shouldPresentAuthenticationDialog]];
	[newRequest setShouldPresentProxyAuthenticationDialog:[self shouldPresentProxyAuthenticationDialog]];
	[newRequest setPostLength:[self postLength]];
	[newRequest setHaveBuiltPostBody:[self haveBuiltPostBody]];
	[newRequest setDidStartSelector:[self didStartSelector]];
	[newRequest setDidFinishSelector:[self didFinishSelector]];
	[newRequest setDidFailSelector:[self didFailSelector]];
	[newRequest setTimeOutSeconds:[self timeOutSeconds]];
	[newRequest setShouldResetDownloadProgress:[self shouldResetDownloadProgress]];
	[newRequest setShouldResetUploadProgress:[self shouldResetUploadProgress]];
	[newRequest setShowAccurateProgress:[self showAccurateProgress]];
	[newRequest setDefaultResponseEncoding:[self defaultResponseEncoding]];
	[newRequest setAllowResumeForFileDownloads:[self allowResumeForFileDownloads]];
	[newRequest setUserInfo:[[[self userInfo] copyWithZone:zone] autorelease]];
	[newRequest setTag:[self tag]];
	[newRequest setUseHTTPVersionOne:[self useHTTPVersionOne]];
	[newRequest setShouldRedirect:[self shouldRedirect]];
	[newRequest setValidatesSecureCertificate:[self validatesSecureCertificate]];
    [newRequest setClientCertificateIdentity:clientCertificateIdentity];
	[newRequest setClientCertificates:[[clientCertificates copy] autorelease]];
	[newRequest setPACurl:[self PACurl]];
	[newRequest setShouldPresentCredentialsBeforeChallenge:[self shouldPresentCredentialsBeforeChallenge]];
	[newRequest setNumberOfTimesToRetryOnTimeout:[self numberOfTimesToRetryOnTimeout]];
	[newRequest setShouldUseRFC2616RedirectBehaviour:[self shouldUseRFC2616RedirectBehaviour]];
	[newRequest setShouldAttemptPersistentConnection:[self shouldAttemptPersistentConnection]];
	[newRequest setPersistentConnectionTimeoutSeconds:[self persistentConnectionTimeoutSeconds]];
    [newRequest setAuthenticationScheme:[self authenticationScheme]];
	return newRequest;
}

#pragma mark default time out

+ (NSTimeInterval)defaultTimeOutSeconds
{
	return defaultTimeOutSeconds;
}

+ (void)setDefaultTimeOutSeconds:(NSTimeInterval)newTimeOutSeconds
{
	defaultTimeOutSeconds = newTimeOutSeconds;
}


#pragma mark client certificate

- (void)setClientCertificateIdentity:(SecIdentityRef)anIdentity {
    if(clientCertificateIdentity) {
        CFRelease(clientCertificateIdentity);
    }
    
    clientCertificateIdentity = anIdentity;
    
	if (clientCertificateIdentity) {
		CFRetain(clientCertificateIdentity);
	}
}


#pragma mark session credentials

+ (NSMutableArray *)sessionProxyCredentialsStore
{
	[sessionCredentialsLock lock];
	if (!sessionProxyCredentialsStore) {
		sessionProxyCredentialsStore = [[NSMutableArray alloc] init];
	}
	[sessionCredentialsLock unlock];
	return sessionProxyCredentialsStore;
}

+ (NSMutableArray *)sessionCredentialsStore
{
	[sessionCredentialsLock lock];
	if (!sessionCredentialsStore) {
		sessionCredentialsStore = [[NSMutableArray alloc] init];
	}
	[sessionCredentialsLock unlock];
	return sessionCredentialsStore;
}

+ (void)storeProxyAuthenticationCredentialsInSessionStore:(NSDictionary *)credentials
{
	[sessionCredentialsLock lock];
	[self removeProxyAuthenticationCredentialsFromSessionStore:[credentials objectForKey:@"Credentials"]];
	[[[self class] sessionProxyCredentialsStore] addObject:credentials];
	[sessionCredentialsLock unlock];
}

+ (void)storeAuthenticationCredentialsInSessionStore:(NSDictionary *)credentials
{
	[sessionCredentialsLock lock];
	[self removeAuthenticationCredentialsFromSessionStore:[credentials objectForKey:@"Credentials"]];
	[[[self class] sessionCredentialsStore] addObject:credentials];
	[sessionCredentialsLock unlock];
}

+ (void)removeProxyAuthenticationCredentialsFromSessionStore:(NSDictionary *)credentials
{
	[sessionCredentialsLock lock];
	NSMutableArray *sessionCredentialsList = [[self class] sessionProxyCredentialsStore];
	NSUInteger i;
	for (i=0; i<[sessionCredentialsList count]; i++) {
		NSDictionary *theCredentials = [sessionCredentialsList objectAtIndex:i];
		if ([theCredentials objectForKey:@"Credentials"] == credentials) {
			[sessionCredentialsList removeObjectAtIndex:i];
			[sessionCredentialsLock unlock];
			return;
		}
	}
	[sessionCredentialsLock unlock];
}

+ (void)removeAuthenticationCredentialsFromSessionStore:(NSDictionary *)credentials
{
	[sessionCredentialsLock lock];
	NSMutableArray *sessionCredentialsList = [[self class] sessionCredentialsStore];
	NSUInteger i;
	for (i=0; i<[sessionCredentialsList count]; i++) {
		NSDictionary *theCredentials = [sessionCredentialsList objectAtIndex:i];
		if ([theCredentials objectForKey:@"Credentials"] == credentials) {
			[sessionCredentialsList removeObjectAtIndex:i];
			[sessionCredentialsLock unlock];
			return;
		}
	}
	[sessionCredentialsLock unlock];
}

- (NSDictionary *)findSessionProxyAuthenticationCredentials
{
	[sessionCredentialsLock lock];
	NSMutableArray *sessionCredentialsList = [[self class] sessionProxyCredentialsStore];
	for (NSDictionary *theCredentials in sessionCredentialsList) {
		if ([[theCredentials objectForKey:@"Host"] isEqualToString:[self proxyHost]] && [[theCredentials objectForKey:@"Port"] intValue] == [self proxyPort]) {
			[sessionCredentialsLock unlock];
			return theCredentials;
		}
	}
	[sessionCredentialsLock unlock];
	return nil;
}


- (NSDictionary *)findSessionAuthenticationCredentials
{
	[sessionCredentialsLock lock];
	NSMutableArray *sessionCredentialsList = [[self class] sessionCredentialsStore];
	NSURL *requestURL = [self url];

	BOOL haveFoundExactMatch;
	NSDictionary *closeMatch = nil;

	// Loop through all the cached credentials we have, looking for the best match for this request
	for (NSDictionary *theCredentials in sessionCredentialsList) {
		
		haveFoundExactMatch = NO;
		NSURL *cachedCredentialsURL = [theCredentials objectForKey:@"URL"];

		// Find an exact match (same url)
		if ([cachedCredentialsURL isEqual:[self url]]) {
			haveFoundExactMatch = YES;

		// This is not an exact match for the url, and we already have a close match we can use
		} else if (closeMatch) {
			continue;

		// Find a close match (same host, scheme and port)
		} else if ([[cachedCredentialsURL host] isEqualToString:[requestURL host]] && ([cachedCredentialsURL port] == [requestURL port] || ([requestURL port] && [[cachedCredentialsURL port] isEqualToNumber:[requestURL port]])) && [[cachedCredentialsURL scheme] isEqualToString:[requestURL scheme]]) {
		} else {
			continue;
		}

		// Just a sanity check to ensure we never choose credentials from a different realm. Can't really do more than that, as either this request or the stored credentials may not have a realm when the other does
		if ([self authenticationRealm] && ([theCredentials objectForKey:@"AuthenticationRealm"] && ![[theCredentials objectForKey:@"AuthenticationRealm"] isEqualToString:[self authenticationRealm]])) {
			continue;
		}

		// If we have a username and password set on the request, check that they are the same as the cached ones
		if ([self username] && [self password]) {
			NSDictionary *usernameAndPassword = [theCredentials objectForKey:@"Credentials"];
			NSString *storedUsername = [usernameAndPassword objectForKey:(NSString *)kCFHTTPAuthenticationUsername];
			NSString *storedPassword = [usernameAndPassword objectForKey:(NSString *)kCFHTTPAuthenticationPassword];
			if (![storedUsername isEqualToString:[self username]] || ![storedPassword isEqualToString:[self password]]) {
				continue;
			}
		}

		// If we have an exact match for the url, use those credentials
		if (haveFoundExactMatch) {
			[sessionCredentialsLock unlock];
			return theCredentials;
		}

		// We have no exact match, let's remember that we have a good match for this server, and we'll use it at the end if we don't find an exact match
		closeMatch = theCredentials;
	}
	[sessionCredentialsLock unlock];

	// Return credentials that matched on host, port and scheme, or nil if we didn't find any
	return closeMatch;
}

#pragma mark keychain storage

+ (void)saveCredentials:(NSURLCredential *)credentials forHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm
{
	NSURLProtectionSpace *protectionSpace = [[[NSURLProtectionSpace alloc] initWithHost:host port:port protocol:protocol realm:realm authenticationMethod:NSURLAuthenticationMethodDefault] autorelease];
	[[NSURLCredentialStorage sharedCredentialStorage] setDefaultCredential:credentials forProtectionSpace:protectionSpace];
}

+ (void)saveCredentials:(NSURLCredential *)credentials forProxy:(NSString *)host port:(int)port realm:(NSString *)realm
{
	NSURLProtectionSpace *protectionSpace = [[[NSURLProtectionSpace alloc] initWithProxyHost:host port:port type:NSURLProtectionSpaceHTTPProxy realm:realm authenticationMethod:NSURLAuthenticationMethodDefault] autorelease];
	[[NSURLCredentialStorage sharedCredentialStorage] setDefaultCredential:credentials forProtectionSpace:protectionSpace];
}

+ (NSURLCredential *)savedCredentialsForHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm
{
	NSURLProtectionSpace *protectionSpace = [[[NSURLProtectionSpace alloc] initWithHost:host port:port protocol:protocol realm:realm authenticationMethod:NSURLAuthenticationMethodDefault] autorelease];
	return [[NSURLCredentialStorage sharedCredentialStorage] defaultCredentialForProtectionSpace:protectionSpace];
}

+ (NSURLCredential *)savedCredentialsForProxy:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm
{
	NSURLProtectionSpace *protectionSpace = [[[NSURLProtectionSpace alloc] initWithProxyHost:host port:port type:NSURLProtectionSpaceHTTPProxy realm:realm authenticationMethod:NSURLAuthenticationMethodDefault] autorelease];
	return [[NSURLCredentialStorage sharedCredentialStorage] defaultCredentialForProtectionSpace:protectionSpace];
}

+ (void)removeCredentialsForHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm
{
	NSURLProtectionSpace *protectionSpace = [[[NSURLProtectionSpace alloc] initWithHost:host port:port protocol:protocol realm:realm authenticationMethod:NSURLAuthenticationMethodDefault] autorelease];
	NSURLCredential *credential = [[NSURLCredentialStorage sharedCredentialStorage] defaultCredentialForProtectionSpace:protectionSpace];
	if (credential) {
		[[NSURLCredentialStorage sharedCredentialStorage] removeCredential:credential forProtectionSpace:protectionSpace];
	}
}

+ (void)removeCredentialsForProxy:(NSString *)host port:(int)port realm:(NSString *)realm
{
	NSURLProtectionSpace *protectionSpace = [[[NSURLProtectionSpace alloc] initWithProxyHost:host port:port type:NSURLProtectionSpaceHTTPProxy realm:realm authenticationMethod:NSURLAuthenticationMethodDefault] autorelease];
	NSURLCredential *credential = [[NSURLCredentialStorage sharedCredentialStorage] defaultCredentialForProtectionSpace:protectionSpace];
	if (credential) {
		[[NSURLCredentialStorage sharedCredentialStorage] removeCredential:credential forProtectionSpace:protectionSpace];
	}
}

+ (NSMutableArray *)sessionCookies
{
	[sessionCookiesLock lock];
	if (!sessionCookies) {
		[ASIHTTPRequest setSessionCookies:[NSMutableArray array]];
	}
	NSMutableArray *cookies = [[sessionCookies retain] autorelease];
	[sessionCookiesLock unlock];
	return cookies;
}

+ (void)setSessionCookies:(NSMutableArray *)newSessionCookies
{
	[sessionCookiesLock lock];
	// Remove existing cookies from the persistent store
	for (NSHTTPCookie *cookie in sessionCookies) {
		[[NSHTTPCookieStorage sharedHTTPCookieStorage] deleteCookie:cookie];
	}
	[sessionCookies release];
	sessionCookies = [newSessionCookies retain];
	[sessionCookiesLock unlock];
}

+ (void)addSessionCookie:(NSHTTPCookie *)newCookie
{
	[sessionCookiesLock lock];
	NSHTTPCookie *cookie;
	NSUInteger i;
	NSUInteger max = [[ASIHTTPRequest sessionCookies] count];
	for (i=0; i<max; i++) {
		cookie = [[ASIHTTPRequest sessionCookies] objectAtIndex:i];
		if ([[cookie domain] isEqualToString:[newCookie domain]] && [[cookie path] isEqualToString:[newCookie path]] && [[cookie name] isEqualToString:[newCookie name]]) {
			[[ASIHTTPRequest sessionCookies] removeObjectAtIndex:i];
			break;
		}
	}
	[[ASIHTTPRequest sessionCookies] addObject:newCookie];
	[sessionCookiesLock unlock];
}

// Dump all session data (authentication and cookies)
+ (void)clearSession
{
	[sessionCredentialsLock lock];
	[[[self class] sessionCredentialsStore] removeAllObjects];
	[sessionCredentialsLock unlock];
	[[self class] setSessionCookies:nil];
	[[[self class] defaultCache] clearCachedResponsesForStoragePolicy:ASICacheForSessionDurationCacheStoragePolicy];
}

#pragma mark get user agent

+ (NSString *)defaultUserAgentString
{
	@synchronized (self) {

		if (!defaultUserAgent) {

			NSBundle *bundle = [NSBundle bundleForClass:[self class]];

			// Attempt to find a name for this application
			NSString *appName = [bundle objectForInfoDictionaryKey:@"CFBundleDisplayName"];
			if (!appName) {
				appName = [bundle objectForInfoDictionaryKey:@"CFBundleName"];
			}

			NSData *latin1Data = [appName dataUsingEncoding:NSUTF8StringEncoding];
			appName = [[[NSString alloc] initWithData:latin1Data encoding:NSISOLatin1StringEncoding] autorelease];

			// If we couldn't find one, we'll give up (and ASIHTTPRequest will use the standard CFNetwork user agent)
			if (!appName) {
				return nil;
			}

			NSString *appVersion = nil;
			NSString *marketingVersionNumber = [bundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
			NSString *developmentVersionNumber = [bundle objectForInfoDictionaryKey:@"CFBundleVersion"];
			if (marketingVersionNumber && developmentVersionNumber) {
				if ([marketingVersionNumber isEqualToString:developmentVersionNumber]) {
					appVersion = marketingVersionNumber;
				} else {
					appVersion = [NSString stringWithFormat:@"%@ rv:%@",marketingVersionNumber,developmentVersionNumber];
				}
			} else {
				appVersion = (marketingVersionNumber ? marketingVersionNumber : developmentVersionNumber);
			}

			NSString *deviceName;
			NSString *OSName;
			NSString *OSVersion;
			NSString *locale = [[NSLocale currentLocale] localeIdentifier];

			#if TARGET_OS_IPHONE
				UIDevice *device = [UIDevice currentDevice];
				deviceName = [device model];
				OSName = [device systemName];
				OSVersion = [device systemVersion];

			#else
				deviceName = @"Macintosh";
				OSName = @"Mac OS X";

				// From http://www.cocoadev.com/index.pl?DeterminingOSVersion
				// We won't bother to check for systems prior to 10.4, since ASIHTTPRequest only works on 10.5+
				OSErr err;
				SInt32 versionMajor, versionMinor, versionBugFix;
				err = Gestalt(gestaltSystemVersionMajor, &versionMajor);
				if (err != noErr) return nil;
				err = Gestalt(gestaltSystemVersionMinor, &versionMinor);
				if (err != noErr) return nil;
				err = Gestalt(gestaltSystemVersionBugFix, &versionBugFix);
				if (err != noErr) return nil;
				OSVersion = [NSString stringWithFormat:@"%u.%u.%u", versionMajor, versionMinor, versionBugFix];
			#endif

			// Takes the form "My Application 1.0 (Macintosh; Mac OS X 10.5.7; en_GB)"
			[self setDefaultUserAgentString:[NSString stringWithFormat:@"%@ %@ (%@; %@ %@; %@)", appName, appVersion, deviceName, OSName, OSVersion, locale]];	
		}
		return [[defaultUserAgent retain] autorelease];
	}
	return nil;
}

+ (void)setDefaultUserAgentString:(NSString *)agent
{
	@synchronized (self) {
		if (defaultUserAgent == agent) {
			return;
		}
		[defaultUserAgent release];
		defaultUserAgent = [agent copy];
	}
}


#pragma mark mime-type detection

+ (NSString *)mimeTypeForFileAtPath:(NSString *)path
{
	if (![[[[NSFileManager alloc] init] autorelease] fileExistsAtPath:path]) {
		return nil;
	}
	// Borrowed from http://stackoverflow.com/questions/2439020/wheres-the-iphone-mime-type-database
	CFStringRef UTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, (CFStringRef)[path pathExtension], NULL);
    CFStringRef MIMEType = UTTypeCopyPreferredTagWithClass (UTI, kUTTagClassMIMEType);
    CFRelease(UTI);
	if (!MIMEType) {
		return @"application/octet-stream";
	}
    return [NSMakeCollectable(MIMEType) autorelease];
}

#pragma mark bandwidth measurement / throttling

- (void)performThrottling
{
	if (![self readStream]) {
		return;
	}
	[ASIHTTPRequest measureBandwidthUsage];
	if ([ASIHTTPRequest isBandwidthThrottled]) {
		[bandwidthThrottlingLock lock];
		// Handle throttling
		if (throttleWakeUpTime) {
			if ([throttleWakeUpTime timeIntervalSinceDate:[NSDate date]] > 0) {
				if ([self readStreamIsScheduled]) {
					[self unscheduleReadStream];
					#if DEBUG_THROTTLING
					ASI_DEBUG_LOG(@"[THROTTLING] Sleeping request %@ until after %@",self,throttleWakeUpTime);
					#endif
				}
			} else {
				if (![self readStreamIsScheduled]) {
					[self scheduleReadStream];
					#if DEBUG_THROTTLING
					ASI_DEBUG_LOG(@"[THROTTLING] Waking up request %@",self);
					#endif
				}
			}
		} 
		[bandwidthThrottlingLock unlock];
		
	// Bandwidth throttling must have been turned off since we last looked, let's re-schedule the stream
	} else if (![self readStreamIsScheduled]) {
		[self scheduleReadStream];			
	}
}

+ (BOOL)isBandwidthThrottled
{
#if TARGET_OS_IPHONE
	[bandwidthThrottlingLock lock];

	BOOL throttle = isBandwidthThrottled || (!shouldThrottleBandwidthForWWANOnly && (maxBandwidthPerSecond > 0));
	[bandwidthThrottlingLock unlock];
	return throttle;
#else
	[bandwidthThrottlingLock lock];
	BOOL throttle = (maxBandwidthPerSecond > 0);
	[bandwidthThrottlingLock unlock];
	return throttle;
#endif
}

+ (unsigned long)maxBandwidthPerSecond
{
	[bandwidthThrottlingLock lock];
	unsigned long amount = maxBandwidthPerSecond;
	[bandwidthThrottlingLock unlock];
	return amount;
}

+ (void)setMaxBandwidthPerSecond:(unsigned long)bytes
{
	[bandwidthThrottlingLock lock];
	maxBandwidthPerSecond = bytes;
	[bandwidthThrottlingLock unlock];
}

+ (void)incrementBandwidthUsedInLastSecond:(unsigned long)bytes
{
	[bandwidthThrottlingLock lock];
	bandwidthUsedInLastSecond += bytes;
	[bandwidthThrottlingLock unlock];
}

+ (void)recordBandwidthUsage
{
	if (bandwidthUsedInLastSecond == 0) {
		[bandwidthUsageTracker removeAllObjects];
	} else {
		NSTimeInterval interval = [bandwidthMeasurementDate timeIntervalSinceNow];
		while ((interval < 0 || [bandwidthUsageTracker count] > 5) && [bandwidthUsageTracker count] > 0) {
			[bandwidthUsageTracker removeObjectAtIndex:0];
			interval++;
		}
	}
	#if DEBUG_THROTTLING
	ASI_DEBUG_LOG(@"[THROTTLING] ===Used: %u bytes of bandwidth in last measurement period===",bandwidthUsedInLastSecond);
	#endif
	[bandwidthUsageTracker addObject:[NSNumber numberWithUnsignedLong:bandwidthUsedInLastSecond]];
	[bandwidthMeasurementDate release];
	bandwidthMeasurementDate = [[NSDate dateWithTimeIntervalSinceNow:1] retain];
	bandwidthUsedInLastSecond = 0;
	
	NSUInteger measurements = [bandwidthUsageTracker count];
	unsigned long totalBytes = 0;
	for (NSNumber *bytes in bandwidthUsageTracker) {
		totalBytes += [bytes unsignedLongValue];
	}
	if (measurements > 0)
		averageBandwidthUsedPerSecond = totalBytes/measurements;
}

+ (unsigned long)averageBandwidthUsedPerSecond
{
	[bandwidthThrottlingLock lock];
	unsigned long amount = 	averageBandwidthUsedPerSecond;
	[bandwidthThrottlingLock unlock];
	return amount;
}

+ (void)measureBandwidthUsage
{
	// Other requests may have to wait for this lock if we're sleeping, but this is fine, since in that case we already know they shouldn't be sending or receiving data
	[bandwidthThrottlingLock lock];

	if (!bandwidthMeasurementDate || [bandwidthMeasurementDate timeIntervalSinceNow] < -0) {
		[ASIHTTPRequest recordBandwidthUsage];
	}
	
	// Are we performing bandwidth throttling?
	if (
	#if TARGET_OS_IPHONE
	isBandwidthThrottled || (!shouldThrottleBandwidthForWWANOnly && (maxBandwidthPerSecond))
	#else
	maxBandwidthPerSecond
	#endif
	) {
		// How much data can we still send or receive this second?
		long long bytesRemaining = (long long)maxBandwidthPerSecond - (long long)bandwidthUsedInLastSecond;
			
		// Have we used up our allowance?
		if (bytesRemaining < 0) {
			
			// Yes, put this request to sleep until a second is up, with extra added punishment sleeping time for being very naughty (we have used more bandwidth than we were allowed)
			double extraSleepyTime = (-bytesRemaining/(maxBandwidthPerSecond*1.0));
			[throttleWakeUpTime release];
			throttleWakeUpTime = [[NSDate alloc] initWithTimeInterval:extraSleepyTime sinceDate:bandwidthMeasurementDate];
		}
	}
	[bandwidthThrottlingLock unlock];
}
	
+ (unsigned long)maxUploadReadLength
{
	[bandwidthThrottlingLock lock];
	
	// We'll split our bandwidth allowance into 4 (which is the default for an ASINetworkQueue's max concurrent operations count) to give all running requests a fighting chance of reading data this cycle
	long long toRead = maxBandwidthPerSecond/4;
	if (maxBandwidthPerSecond > 0 && (bandwidthUsedInLastSecond + toRead > maxBandwidthPerSecond)) {
		toRead = (long long)maxBandwidthPerSecond-(long long)bandwidthUsedInLastSecond;
		if (toRead < 0) {
			toRead = 0;
		}
	}
	
	if (toRead == 0 || !bandwidthMeasurementDate || [bandwidthMeasurementDate timeIntervalSinceNow] < -0) {
		[throttleWakeUpTime release];
		throttleWakeUpTime = [bandwidthMeasurementDate retain];
	}
	[bandwidthThrottlingLock unlock];	
	return (unsigned long)toRead;
}
	

#if TARGET_OS_IPHONE
+ (void)setShouldThrottleBandwidthForWWAN:(BOOL)throttle
{
	if (throttle) {
		[ASIHTTPRequest throttleBandwidthForWWANUsingLimit:ASIWWANBandwidthThrottleAmount];
	} else {
		[ASIHTTPRequest unsubscribeFromNetworkReachabilityNotifications];
		[ASIHTTPRequest setMaxBandwidthPerSecond:0];
		[bandwidthThrottlingLock lock];
		isBandwidthThrottled = NO;
		shouldThrottleBandwidthForWWANOnly = NO;
		[bandwidthThrottlingLock unlock];
	}
}

+ (void)throttleBandwidthForWWANUsingLimit:(unsigned long)limit
{	
	[bandwidthThrottlingLock lock];
	shouldThrottleBandwidthForWWANOnly = YES;
	maxBandwidthPerSecond = limit;
	[ASIHTTPRequest registerForNetworkReachabilityNotifications];	
	[bandwidthThrottlingLock unlock];
	[ASIHTTPRequest reachabilityChanged:nil];
}

#pragma mark reachability

+ (void)registerForNetworkReachabilityNotifications
{
	[[Reachability reachabilityForInternetConnection] startNotifier];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(reachabilityChanged:) name:kReachabilityChangedNotification object:nil];
}


+ (void)unsubscribeFromNetworkReachabilityNotifications
{
	[[NSNotificationCenter defaultCenter] removeObserver:self name:kReachabilityChangedNotification object:nil];
}

+ (BOOL)isNetworkReachableViaWWAN
{
	return ([[Reachability reachabilityForInternetConnection] currentReachabilityStatus] == ReachableViaWWAN);	
}

+ (void)reachabilityChanged:(NSNotification *)note
{
	[bandwidthThrottlingLock lock];
	isBandwidthThrottled = [ASIHTTPRequest isNetworkReachableViaWWAN];
	[bandwidthThrottlingLock unlock];
}
#endif

#pragma mark queue

// Returns the shared queue
+ (NSOperationQueue *)sharedQueue
{
    return [[sharedQueue retain] autorelease];
}

#pragma mark cache

+ (void)setDefaultCache:(id <ASICacheDelegate>)cache
{
	@synchronized (self) {
		[cache retain];
		[defaultCache release];
		defaultCache = cache;
	}
}

+ (id <ASICacheDelegate>)defaultCache
{
    @synchronized(self) {
        return [[defaultCache retain] autorelease];
    }
	return nil;
}


#pragma mark network activity

+ (BOOL)isNetworkInUse
{
	[connectionsLock lock];
	BOOL inUse = (runningRequestCount > 0);
	[connectionsLock unlock];
	return inUse;
}

+ (void)setShouldUpdateNetworkActivityIndicator:(BOOL)shouldUpdate
{
	[connectionsLock lock];
	shouldUpdateNetworkActivityIndicator = shouldUpdate;
	[connectionsLock unlock];
}

+ (void)showNetworkActivityIndicator
{
#if TARGET_OS_IPHONE
	[[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:YES];
#endif
}

+ (void)hideNetworkActivityIndicator
{
#if TARGET_OS_IPHONE
	[[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:NO];	
#endif
}


/* Always called on main thread */
+ (void)hideNetworkActivityIndicatorAfterDelay
{
	[self performSelector:@selector(hideNetworkActivityIndicatorIfNeeeded) withObject:nil afterDelay:0.5];
}

+ (void)hideNetworkActivityIndicatorIfNeeeded
{
	[connectionsLock lock];
	if (runningRequestCount == 0) {
		[self hideNetworkActivityIndicator];
	}
	[connectionsLock unlock];
}


#pragma mark threading behaviour

// In the default implementation, all requests run in a single background thread
// Advanced users only: Override this method in a subclass for a different threading behaviour
// Eg: return [NSThread mainThread] to run all requests in the main thread
// Alternatively, you can create a thread on demand, or manage a pool of threads
// Threads returned by this method will need to run the runloop in default mode (eg CFRunLoopRun())
// Requests will stop the runloop when they complete
// If you have multiple requests sharing the thread or you want to re-use the thread, you'll need to restart the runloop
+ (NSThread *)threadForRequest:(ASIHTTPRequest *)request
{
	if (networkThread == nil) {
		@synchronized(self) {
			if (networkThread == nil) {
				networkThread = [[NSThread alloc] initWithTarget:self selector:@selector(runRequests) object:nil];
				[networkThread start];
			}
		}
	}
	return networkThread;
}

+ (void)runRequests
{
	// Should keep the runloop from exiting
	CFRunLoopSourceContext context = {0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	CFRunLoopSourceRef source = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &context);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode);

    BOOL runAlways = YES; // Introduced to cheat Static Analyzer
	while (runAlways) {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0e10, true);
		[pool drain];
	}

	// Should never be called, but anyway
	CFRunLoopRemoveSource(CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode);
	CFRelease(source);
}

#pragma mark miscellany 

#if TARGET_OS_IPHONE
+ (BOOL)isMultitaskingSupported
{
	BOOL multiTaskingSupported = NO;
	if ([[UIDevice currentDevice] respondsToSelector:@selector(isMultitaskingSupported)]) {
		multiTaskingSupported = [(id)[UIDevice currentDevice] isMultitaskingSupported];
	}
	return multiTaskingSupported;
}
#endif

// From: http://www.cocoadev.com/index.pl?BaseSixtyFour

+ (NSString*)base64forData:(NSData*)theData {
	
	const uint8_t* input = (const uint8_t*)[theData bytes];
	NSUInteger length = [theData length];
	
    static char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	
    NSMutableData* data = [NSMutableData dataWithLength:((length + 2) / 3) * 4];
    uint8_t* output = (uint8_t*)data.mutableBytes;
	
	NSUInteger i,i2;
    for (i=0; i < length; i += 3) {
        NSInteger value = 0;
		for (i2=0; i2<3; i2++) {
            value <<= 8;
            if (i+i2 < length) {
                value |= (0xFF & input[i+i2]);
            }
        }
		
        NSInteger theIndex = (i / 3) * 4;
        output[theIndex + 0] =                    (uint8_t)table[(value >> 18) & 0x3F];
        output[theIndex + 1] =                    (uint8_t)table[(value >> 12) & 0x3F];
        output[theIndex + 2] = (i + 1) < length ? (uint8_t)table[(value >> 6)  & 0x3F] : '=';
        output[theIndex + 3] = (i + 2) < length ? (uint8_t)table[(value >> 0)  & 0x3F] : '=';
    }
	
    return [[[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding] autorelease];
}

+ (NSDate *)expiryDateForRequest:(ASIHTTPRequest *)request maxAge:(NSTimeInterval)maxAge
{
	NSDictionary *responseHeaders = [request responseHeaders];
  
	// If we weren't given a custom max-age, lets look for one in the response headers
	if (!maxAge) {
		NSString *cacheControl = [[responseHeaders objectForKey:@"Cache-Control"] lowercaseString];
		if (cacheControl) {
			NSScanner *scanner = [NSScanner scannerWithString:cacheControl];
			[scanner scanUpToString:@"max-age" intoString:NULL];
			if ([scanner scanString:@"max-age" intoString:NULL]) {
				[scanner scanString:@"=" intoString:NULL];
				[scanner scanDouble:&maxAge];
			}
		}
	}
  
	// RFC 2612 says max-age must override any Expires header
	if (maxAge) {
		NSDate *date = [NSDate date];
		if ([date respondsToSelector:@selector(dateByAddingTimeInterval:)]) {
			return [date dateByAddingTimeInterval:maxAge];
		} else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
			return [date addTimeInterval:maxAge];
#pragma clang diagnostic pop
        }
	} else {
		NSString *expires = [responseHeaders objectForKey:@"Expires"];
		if (expires) {
			return [ASIHTTPRequest dateFromRFC1123String:expires];
		}
	}
	return nil;
}

// Based on hints from http://stackoverflow.com/questions/1850824/parsing-a-rfc-822-date-with-nsdateformatter
+ (NSDate *)dateFromRFC1123String:(NSString *)string
{
	NSDateFormatter *formatter = [[[NSDateFormatter alloc] init] autorelease];
	[formatter setLocale:[[[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"] autorelease]];
	// Does the string include a week day?
	NSString *day = @"";
	if ([string rangeOfString:@","].location != NSNotFound) {
		day = @"EEE, ";
	}
	// Does the string include seconds?
	NSString *seconds = @"";
	if ([[string componentsSeparatedByString:@":"] count] == 3) {
		seconds = @":ss";
	}
	[formatter setDateFormat:[NSString stringWithFormat:@"%@dd MMM yyyy HH:mm%@ z",day,seconds]];
	return [formatter dateFromString:string];
}

+ (void)parseMimeType:(NSString **)mimeType andResponseEncoding:(NSStringEncoding *)stringEncoding fromContentType:(NSString *)contentType
{
	if (!contentType) {
		return;
	}
	NSScanner *charsetScanner = [NSScanner scannerWithString: contentType];
	if (![charsetScanner scanUpToString:@";" intoString:mimeType] || [charsetScanner scanLocation] == [contentType length]) {
		*mimeType = [contentType stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
		return;
	}
	*mimeType = [*mimeType stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
	NSString *charsetSeparator = @"charset=";
	NSString *IANAEncoding = nil;

	if ([charsetScanner scanUpToString: charsetSeparator intoString: NULL] && [charsetScanner scanLocation] < [contentType length]) {
		[charsetScanner setScanLocation: [charsetScanner scanLocation] + [charsetSeparator length]];
		[charsetScanner scanUpToString: @";" intoString: &IANAEncoding];
	}

	if (IANAEncoding) {
		CFStringEncoding cfEncoding = CFStringConvertIANACharSetNameToEncoding((CFStringRef)IANAEncoding);
		if (cfEncoding != kCFStringEncodingInvalidId) {
			*stringEncoding = CFStringConvertEncodingToNSStringEncoding(cfEncoding);
		}
	}
}

#pragma mark -
#pragma mark blocks
#if NS_BLOCKS_AVAILABLE
- (void)setStartedBlock:(ASIBasicBlock)aStartedBlock
{
	[startedBlock release];
	startedBlock = [aStartedBlock copy];
}

- (void)setHeadersReceivedBlock:(ASIHeadersBlock)aReceivedBlock
{
	[headersReceivedBlock release];
	headersReceivedBlock = [aReceivedBlock copy];
}

- (void)setCompletionBlock:(ASIBasicBlock)aCompletionBlock
{
	[completionBlock release];
	completionBlock = [aCompletionBlock copy];
}

- (void)setFailedBlock:(ASIBasicBlock)aFailedBlock
{
	[failureBlock release];
	failureBlock = [aFailedBlock copy];
}

- (void)setBytesReceivedBlock:(ASIProgressBlock)aBytesReceivedBlock
{
	[bytesReceivedBlock release];
	bytesReceivedBlock = [aBytesReceivedBlock copy];
}

- (void)setBytesSentBlock:(ASIProgressBlock)aBytesSentBlock
{
	[bytesSentBlock release];
	bytesSentBlock = [aBytesSentBlock copy];
}

- (void)setDownloadSizeIncrementedBlock:(ASISizeBlock)aDownloadSizeIncrementedBlock{
	[downloadSizeIncrementedBlock release];
	downloadSizeIncrementedBlock = [aDownloadSizeIncrementedBlock copy];
}

- (void)setUploadSizeIncrementedBlock:(ASISizeBlock)anUploadSizeIncrementedBlock
{
	[uploadSizeIncrementedBlock release];
	uploadSizeIncrementedBlock = [anUploadSizeIncrementedBlock copy];
}

- (void)setDataReceivedBlock:(ASIDataBlock)aReceivedBlock
{
	[dataReceivedBlock release];
	dataReceivedBlock = [aReceivedBlock copy];
}

- (void)setAuthenticationNeededBlock:(ASIBasicBlock)anAuthenticationBlock
{
	[authenticationNeededBlock release];
	authenticationNeededBlock = [anAuthenticationBlock copy];
}
- (void)setProxyAuthenticationNeededBlock:(ASIBasicBlock)aProxyAuthenticationBlock
{
	[proxyAuthenticationNeededBlock release];
	proxyAuthenticationNeededBlock = [aProxyAuthenticationBlock copy];
}
- (void)setRequestRedirectedBlock:(ASIBasicBlock)aRedirectBlock
{
	[requestRedirectedBlock release];
	requestRedirectedBlock = [aRedirectBlock copy];
}
#endif

#pragma mark ===

@synthesize username;
@synthesize password;
@synthesize userAgentString;
@synthesize domain;
@synthesize proxyUsername;
@synthesize proxyPassword;
@synthesize proxyDomain;
@synthesize url;
@synthesize originalURL;
@synthesize delegate;
@synthesize queue;
@synthesize uploadProgressDelegate;
@synthesize downloadProgressDelegate;
@synthesize useKeychainPersistence;
@synthesize useSessionPersistence;
@synthesize useCookiePersistence;
@synthesize downloadDestinationPath;
@synthesize temporaryFileDownloadPath;
@synthesize temporaryUncompressedDataDownloadPath;
@synthesize didStartSelector;
@synthesize didReceiveResponseHeadersSelector;
@synthesize willRedirectSelector;
@synthesize didFinishSelector;
@synthesize didFailSelector;
@synthesize didReceiveDataSelector;
@synthesize authenticationRealm;
@synthesize proxyAuthenticationRealm;
@synthesize error;
@synthesize complete;
@synthesize requestHeaders;
@synthesize responseHeaders;
@synthesize responseCookies;
@synthesize requestCookies;
@synthesize requestCredentials;
@synthesize responseStatusCode;
@synthesize rawResponseData;
@synthesize lastActivityTime;
@synthesize timeOutSeconds;
@synthesize requestMethod;
@synthesize postBody;
@synthesize compressedPostBody;
@synthesize contentLength;
@synthesize partialDownloadSize;
@synthesize postLength;
@synthesize shouldResetDownloadProgress;
@synthesize shouldResetUploadProgress;
@synthesize mainRequest;
@synthesize totalBytesRead;
@synthesize totalBytesSent;
@synthesize showAccurateProgress;
@synthesize uploadBufferSize;
@synthesize defaultResponseEncoding;
@synthesize responseEncoding;
@synthesize allowCompressedResponse;
@synthesize allowResumeForFileDownloads;
@synthesize userInfo;
@synthesize tag;
@synthesize postBodyFilePath;
@synthesize compressedPostBodyFilePath;
@synthesize postBodyWriteStream;
@synthesize postBodyReadStream;
@synthesize shouldStreamPostDataFromDisk;
@synthesize didCreateTemporaryPostDataFile;
@synthesize useHTTPVersionOne;
@synthesize lastBytesRead;
@synthesize lastBytesSent;
@synthesize cancelledLock;
@synthesize haveBuiltPostBody;
@synthesize fileDownloadOutputStream;
@synthesize inflatedFileDownloadOutputStream;
@synthesize authenticationRetryCount;
@synthesize proxyAuthenticationRetryCount;
@synthesize updatedProgress;
@synthesize shouldRedirect;
@synthesize validatesSecureCertificate;
@synthesize needsRedirect;
@synthesize redirectCount;
@synthesize shouldCompressRequestBody;
@synthesize proxyCredentials;
@synthesize proxyHost;
@synthesize proxyPort;
@synthesize proxyType;
@synthesize PACurl;
@synthesize authenticationScheme;
@synthesize proxyAuthenticationScheme;
@synthesize shouldPresentAuthenticationDialog;
@synthesize shouldPresentProxyAuthenticationDialog;
@synthesize authenticationNeeded;
@synthesize responseStatusMessage;
@synthesize shouldPresentCredentialsBeforeChallenge;
@synthesize haveBuiltRequestHeaders;
@synthesize inProgress;
@synthesize numberOfTimesToRetryOnTimeout;
@synthesize retryCount;
@synthesize willRetryRequest;
@synthesize shouldAttemptPersistentConnection;
@synthesize persistentConnectionTimeoutSeconds;
@synthesize connectionCanBeReused;
@synthesize connectionInfo;
@synthesize readStream;
@synthesize readStreamIsScheduled;
@synthesize shouldUseRFC2616RedirectBehaviour;
@synthesize downloadComplete;
@synthesize requestID;
@synthesize runLoopMode;
@synthesize statusTimer;
@synthesize downloadCache;
@synthesize cachePolicy;
@synthesize cacheStoragePolicy;
@synthesize didUseCachedResponse;
@synthesize secondsToCache;
@synthesize clientCertificates;
@synthesize redirectURL;
#if TARGET_OS_IPHONE && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_4_0
@synthesize shouldContinueWhenAppEntersBackground;
#endif
@synthesize dataDecompressor;
@synthesize shouldWaitToInflateCompressedResponses;

@synthesize isPACFileRequest;
@synthesize PACFileRequest;
@synthesize PACFileReadStream;
@synthesize PACFileData;

@synthesize isSynchronous;
@end

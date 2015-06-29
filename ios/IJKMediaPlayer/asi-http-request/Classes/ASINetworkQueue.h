//
//  ASINetworkQueue.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 07/11/2008.
//  Copyright 2008-2009 All-Seeing Interactive. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ASIHTTPRequestDelegate.h"
#import "ASIProgressDelegate.h"

@interface ASINetworkQueue : NSOperationQueue <ASIProgressDelegate, ASIHTTPRequestDelegate, NSCopying> {
	
	// Delegate will get didFail + didFinish messages (if set)
	id delegate;

	// Will be called when a request starts with the request as the argument
	SEL requestDidStartSelector;
	
	// Will be called when a request receives response headers
	// Should take the form request:didRecieveResponseHeaders:, where the first argument is the request, and the second the headers dictionary
	SEL requestDidReceiveResponseHeadersSelector;
	
	// Will be called when a request is about to redirect
	// Should take the form request:willRedirectToURL:, where the first argument is the request, and the second the new url
	SEL requestWillRedirectSelector;

	// Will be called when a request completes with the request as the argument
	SEL requestDidFinishSelector;
	
	// Will be called when a request fails with the request as the argument
	SEL requestDidFailSelector;
	
	// Will be called when the queue finishes with the queue as the argument
	SEL queueDidFinishSelector;
	
	// Upload progress indicator, probably an NSProgressIndicator or UIProgressView
	id uploadProgressDelegate;
	
	// Total amount uploaded so far for all requests in this queue
	unsigned long long bytesUploadedSoFar;
	
	// Total amount to be uploaded for all requests in this queue - requests add to this figure as they work out how much data they have to transmit
	unsigned long long totalBytesToUpload;

	// Download progress indicator, probably an NSProgressIndicator or UIProgressView
	id downloadProgressDelegate;
	
	// Total amount downloaded so far for all requests in this queue
	unsigned long long bytesDownloadedSoFar;
	
	// Total amount to be downloaded for all requests in this queue - requests add to this figure as they receive Content-Length headers
	unsigned long long totalBytesToDownload;
	
	// When YES, the queue will cancel all requests when a request fails. Default is YES
	BOOL shouldCancelAllRequestsOnFailure;
	
	//Number of real requests (excludes HEAD requests created to manage showAccurateProgress)
	int requestsCount;
	
	// When NO, this request will only update the progress indicator when it completes
	// When YES, this request will update the progress indicator according to how much data it has received so far
	// When YES, the queue will first perform HEAD requests for all GET requests in the queue, so it can calculate the total download size before it starts
	// NO means better performance, because it skips this step for GET requests, and it won't waste time updating the progress indicator until a request completes 
	// Set to YES if the size of a requests in the queue varies greatly for much more accurate results
	// Default for requests in the queue is NO
	BOOL showAccurateProgress;

	// Storage container for additional queue information.
	NSDictionary *userInfo;
	
}

// Convenience constructor
+ (id)queue;

// Call this to reset a queue - it will cancel all operations, clear delegates, and suspend operation
- (void)reset;

// Used internally to manage HEAD requests when showAccurateProgress is YES, do not use!
- (void)addHEADOperation:(NSOperation *)operation;

// All ASINetworkQueues are paused when created so that total size can be calculated before the queue starts
// This method will start the queue
- (void)go;

@property (assign, nonatomic, setter=setUploadProgressDelegate:) id uploadProgressDelegate;
@property (assign, nonatomic, setter=setDownloadProgressDelegate:) id downloadProgressDelegate;

@property (assign, atomic) SEL requestDidStartSelector;
@property (assign, atomic) SEL requestDidReceiveResponseHeadersSelector;
@property (assign, atomic) SEL requestWillRedirectSelector;
@property (assign, atomic) SEL requestDidFinishSelector;
@property (assign, atomic) SEL requestDidFailSelector;
@property (assign, atomic) SEL queueDidFinishSelector;
@property (assign, atomic) BOOL shouldCancelAllRequestsOnFailure;
@property (assign, atomic) id delegate;
@property (assign, atomic) BOOL showAccurateProgress;
@property (assign, atomic, readonly) int requestsCount;
@property (retain, atomic) NSDictionary *userInfo;

@property (assign, atomic) unsigned long long bytesUploadedSoFar;
@property (assign, atomic) unsigned long long totalBytesToUpload;
@property (assign, atomic) unsigned long long bytesDownloadedSoFar;
@property (assign, atomic) unsigned long long totalBytesToDownload;

@end

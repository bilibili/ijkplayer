//
//  ASIWebPageRequest.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 29/06/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//
//  This is an EXPERIMENTAL class - use at your own risk!
//  It is strongly recommend to set a downloadDestinationPath when using ASIWebPageRequest
//  Also, performance will be better if your ASIWebPageRequest has a downloadCache setup
//  Known issue: You cannot use startSychronous with an ASIWebPageRequest

#import "ASIHTTPRequest.h"

@class ASINetworkQueue;

// Used internally for storing what type of data we got from the server
typedef enum _ASIWebContentType {
    ASINotParsedWebContentType = 0,
    ASIHTMLWebContentType = 1,
    ASICSSWebContentType = 2
} ASIWebContentType;

// These correspond with the urlReplacementMode property of ASIWebPageRequest
typedef enum _ASIURLReplacementMode {

	// Don't modify html or css content at all
    ASIDontModifyURLs = 0,

	// Replace external resources urls (images, stylesheets etc) with data uris, so their content is embdedded directly in the html/css
    ASIReplaceExternalResourcesWithData = 1,

	// Replace external resource urls with the url of locally cached content
	// You must set the baseURL of a WebView / UIWebView to a file url pointing at the downloadDestinationPath of the main ASIWebPageRequest if you want to display your content
    // See the Mac or iPhone example projects for a demonstration of how to do this
	// The hrefs of all hyperlinks are changed to use absolute urls when using this mode
	ASIReplaceExternalResourcesWithLocalURLs = 2
} ASIURLReplacementMode;



@interface ASIWebPageRequest : ASIHTTPRequest {

	// Each ASIWebPageRequest for an HTML or CSS file creates its own internal queue to download external resources
	ASINetworkQueue *externalResourceQueue;

	// This dictionary stores a list of external resources to download, along with their content-type data or a path to the data
	NSMutableDictionary *resourceList;

	// Used internally for parsing HTML (with libxml)
	struct _xmlDoc *doc;

	// If the response is an HTML or CSS file, this will be set so the content can be correctly parsed when it has finished fetching external resources
	ASIWebContentType webContentType;

	// Stores a reference to the ASIWebPageRequest that created this request
	// Note that a parentRequest can also have a parent request because ASIWebPageRequests parse their contents to look for external resources recursively
	// For example, a request for an image can be created by a request for a stylesheet which was created by a request for a web page
	ASIWebPageRequest *parentRequest;

	// Controls what ASIWebPageRequest does with external resources. See the notes above for more.
	ASIURLReplacementMode urlReplacementMode;

	// When set to NO, loading will stop when an external resource fails to load. Defaults to YES
	BOOL shouldIgnoreExternalResourceErrors;
}

// Will return a data URI that contains a base64 version of the content at this url
// This is used when replacing urls in the html and css with actual data
// If you subclass ASIWebPageRequest, you can override this function to return different content or a url pointing at another location
- (NSString *)contentForExternalURL:(NSString *)theURL;

// Returns the location that a downloaded external resource's content will be stored in
- (NSString *)cachePathForRequest:(ASIWebPageRequest *)theRequest;


@property (retain, nonatomic) ASIWebPageRequest *parentRequest;
@property (assign, nonatomic) ASIURLReplacementMode urlReplacementMode;
@property (assign, nonatomic) BOOL shouldIgnoreExternalResourceErrors;
@end

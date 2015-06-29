//
//  ASIWebPageRequest.m
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 29/06/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//
//  This is an EXPERIMENTAL class - use at your own risk!

#import "ASIWebPageRequest.h"
#import "ASINetworkQueue.h"
#import <CommonCrypto/CommonHMAC.h>
#import <libxml/HTMLparser.h>
#import <libxml/xmlsave.h>
#import <libxml/xpath.h>
#import <libxml/xpathInternals.h>

// An xPath query that controls the external resources ASIWebPageRequest will fetch
// By default, it will fetch stylesheets, javascript files, images, frames, iframes, and html 5 video / audio
static xmlChar *xpathExpr = (xmlChar *)"//link/@href|//a/@href|//script/@src|//img/@src|//frame/@src|//iframe/@src|//style|//*/@style|//source/@src|//video/@poster|//audio/@src";

static NSLock *xmlParsingLock = nil;
static NSMutableArray *requestsUsingXMLParser = nil;

@interface ASIWebPageRequest ()
- (void)readResourceURLs;
- (void)updateResourceURLs;
- (void)parseAsHTML;
- (void)parseAsCSS;
- (void)addURLToFetch:(NSString *)newURL;
+ (NSArray *)CSSURLsFromString:(NSString *)string;
- (NSString *)relativePathTo:(NSString *)destinationPath fromPath:(NSString *)sourcePath;

- (void)finishedFetchingExternalResources:(ASINetworkQueue *)queue;
- (void)externalResourceFetchSucceeded:(ASIHTTPRequest *)externalResourceRequest;
- (void)externalResourceFetchFailed:(ASIHTTPRequest *)externalResourceRequest;

@property (retain, nonatomic) ASINetworkQueue *externalResourceQueue;
@property (retain, nonatomic) NSMutableDictionary *resourceList;
@end

@implementation ASIWebPageRequest

+ (void)initialize
{
	if (self == [ASIWebPageRequest class]) {
		xmlParsingLock = [[NSLock alloc] init];
		requestsUsingXMLParser = [[NSMutableArray alloc] init];
	}
}

- (id)initWithURL:(NSURL *)newURL
{
	self = [super initWithURL:newURL];
	[self setShouldIgnoreExternalResourceErrors:YES];
	return self;
}

- (void)dealloc
{
	[externalResourceQueue cancelAllOperations];
	[externalResourceQueue release];
	[resourceList release];
	[parentRequest release];
	[super dealloc];
}

// This is a bit of a hack
// The role of this method in normal ASIHTTPRequests is to tell the queue we are done with the request, and perform some cleanup
// We override it to stop that happening, and instead do that work in the bottom of finishedFetchingExternalResources:
- (void)markAsFinished
{
	if ([self error]) {
		[super markAsFinished];
	}
}

// This method is normally responsible for telling delegates we are done, but it happens to be the most convenient place to parse the responses
// Again, we call the super implementation in finishedFetchingExternalResources:, or here if this download was not an HTML or CSS file
- (void)requestFinished
{
	complete = NO;
	if ([self mainRequest] || [self didUseCachedResponse]) {
		[super requestFinished];
		[super markAsFinished];
		return;
	}
	webContentType = ASINotParsedWebContentType;
	NSString *contentType = [[[self responseHeaders] objectForKey:@"Content-Type"] lowercaseString];
	contentType = [[contentType componentsSeparatedByString:@";"] objectAtIndex:0];
	if ([contentType isEqualToString:@"text/html"] || [contentType isEqualToString:@"text/xhtml"] || [contentType isEqualToString:@"text/xhtml+xml"] || [contentType isEqualToString:@"application/xhtml+xml"]) {
		[self parseAsHTML];
		return;
	} else if ([contentType isEqualToString:@"text/css"]) {
		[self parseAsCSS];
		return;
	}
	[super requestFinished];
	[super markAsFinished];
}

- (void)parseAsCSS
{
	webContentType = ASICSSWebContentType;

	NSString *responseCSS = nil;
	NSError *err = nil;
	if ([self downloadDestinationPath]) {
		responseCSS = [NSString stringWithContentsOfFile:[self downloadDestinationPath] encoding:[self responseEncoding] error:&err];
	} else {
		responseCSS = [self responseString];
	}
	if (err) {
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:100 userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Unable to read HTML string from response",NSLocalizedDescriptionKey,err,NSUnderlyingErrorKey,nil]]];
		return;
	} else if (!responseCSS) {
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:100 userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Unable to read HTML string from response",NSLocalizedDescriptionKey,nil]]];
		return;
	}
	NSArray *urls = [[self class] CSSURLsFromString:responseCSS];

	[self setResourceList:[NSMutableDictionary dictionary]];

	for (NSString *theURL in urls) {
		[self addURLToFetch:theURL];
	}
	if (![[self resourceList] count]) {
		[super requestFinished];
		[super markAsFinished];
		return;
	}

	// Create a new request for every item in the queue
	[[self externalResourceQueue] cancelAllOperations];
	[self setExternalResourceQueue:[ASINetworkQueue queue]];
	[[self externalResourceQueue] setDelegate:self];
	[[self externalResourceQueue] setShouldCancelAllRequestsOnFailure:[self shouldIgnoreExternalResourceErrors]];
	[[self externalResourceQueue] setShowAccurateProgress:[self showAccurateProgress]];
	[[self externalResourceQueue] setQueueDidFinishSelector:@selector(finishedFetchingExternalResources:)];
	[[self externalResourceQueue] setRequestDidFinishSelector:@selector(externalResourceFetchSucceeded:)];
	[[self externalResourceQueue] setRequestDidFailSelector:@selector(externalResourceFetchFailed:)];
	for (NSString *theURL in [[self resourceList] keyEnumerator]) {
		ASIWebPageRequest *externalResourceRequest = [ASIWebPageRequest requestWithURL:[NSURL URLWithString:theURL relativeToURL:[self url]]];
		[externalResourceRequest setRequestHeaders:[self requestHeaders]];
		[externalResourceRequest setDownloadCache:[self downloadCache]];
		[externalResourceRequest setCachePolicy:[self cachePolicy]];
		[externalResourceRequest setCacheStoragePolicy:[self cacheStoragePolicy]];
		[externalResourceRequest setUserInfo:[NSDictionary dictionaryWithObject:theURL forKey:@"Path"]];
		[externalResourceRequest setParentRequest:self];
		[externalResourceRequest setUrlReplacementMode:[self urlReplacementMode]];
		[externalResourceRequest setShouldResetDownloadProgress:NO];
		[externalResourceRequest setDelegate:self];
		[externalResourceRequest setUploadProgressDelegate:self];
		[externalResourceRequest setDownloadProgressDelegate:self];
		if ([self downloadDestinationPath]) {
			[externalResourceRequest setDownloadDestinationPath:[self cachePathForRequest:externalResourceRequest]];
		}
		[[self externalResourceQueue] addOperation:externalResourceRequest];
	}
	[[self externalResourceQueue] go];
}

- (const char *)encodingName
{
	xmlCharEncoding encoding = XML_CHAR_ENCODING_NONE;
	switch ([self responseEncoding])
	{
		case NSASCIIStringEncoding:
			encoding = XML_CHAR_ENCODING_ASCII;
			break;
		case NSJapaneseEUCStringEncoding:
			encoding = XML_CHAR_ENCODING_EUC_JP;
			break;
		case NSUTF8StringEncoding:
			encoding = XML_CHAR_ENCODING_UTF8;
			break;
		case NSISOLatin1StringEncoding:
			encoding = XML_CHAR_ENCODING_8859_1;
			break;
		case NSShiftJISStringEncoding:
			encoding = XML_CHAR_ENCODING_SHIFT_JIS;
			break;
		case NSISOLatin2StringEncoding:
			encoding = XML_CHAR_ENCODING_8859_2;
			break;
		case NSISO2022JPStringEncoding:
			encoding = XML_CHAR_ENCODING_2022_JP;
			break;
		case NSUTF16BigEndianStringEncoding:
			encoding = XML_CHAR_ENCODING_UTF16BE;
			break;
		case NSUTF16LittleEndianStringEncoding:
			encoding = XML_CHAR_ENCODING_UTF16LE;
			break;
		case NSUTF32BigEndianStringEncoding:
			encoding = XML_CHAR_ENCODING_UCS4BE;
			break;
		case NSUTF32LittleEndianStringEncoding:
			encoding = XML_CHAR_ENCODING_UCS4LE;
			break;
		case NSNEXTSTEPStringEncoding:
		case NSSymbolStringEncoding:
		case NSNonLossyASCIIStringEncoding:
		case NSUnicodeStringEncoding:
		case NSMacOSRomanStringEncoding:
		case NSUTF32StringEncoding:
		default:
			encoding = XML_CHAR_ENCODING_ERROR;
			break;
	}
	return xmlGetCharEncodingName(encoding);
}

- (void)parseAsHTML
{
	webContentType = ASIHTMLWebContentType;

	// Only allow parsing of a single document at a time
	[xmlParsingLock lock];

	if (![requestsUsingXMLParser count]) {
		xmlInitParser();
	}
	[requestsUsingXMLParser addObject:self];


    /* Load XML document */
	if ([self downloadDestinationPath]) {
		doc = htmlReadFile([[self downloadDestinationPath] cStringUsingEncoding:NSUTF8StringEncoding], [self encodingName], HTML_PARSE_NONET | HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR);
	} else {
		NSData *data = [self responseData];
		doc = htmlReadMemory([data bytes], (int)[data length], "", [self encodingName], HTML_PARSE_NONET | HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR);
	}
    if (doc == NULL) {
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:101 userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Error: unable to parse reponse XML",NSLocalizedDescriptionKey,nil]]];
		return;
    }
	
	[self setResourceList:[NSMutableDictionary dictionary]];

    // Populate the list of URLS to download
    [self readResourceURLs];

	if ([self error] || ![[self resourceList] count]) {
		[requestsUsingXMLParser removeObject:self];
		xmlFreeDoc(doc);
		doc = NULL;
	}

	[xmlParsingLock unlock];

	if ([self error]) {
		return;
	} else if (![[self resourceList] count]) {
		[super requestFinished];
		[super markAsFinished];
		return;
	}
	
	// Create a new request for every item in the queue
	[[self externalResourceQueue] cancelAllOperations];
	[self setExternalResourceQueue:[ASINetworkQueue queue]];
	[[self externalResourceQueue] setDelegate:self];
	[[self externalResourceQueue] setShouldCancelAllRequestsOnFailure:[self shouldIgnoreExternalResourceErrors]];
	[[self externalResourceQueue] setShowAccurateProgress:[self showAccurateProgress]];
	[[self externalResourceQueue] setQueueDidFinishSelector:@selector(finishedFetchingExternalResources:)];
	[[self externalResourceQueue] setRequestDidFinishSelector:@selector(externalResourceFetchSucceeded:)];
	[[self externalResourceQueue] setRequestDidFailSelector:@selector(externalResourceFetchFailed:)];
	for (NSString *theURL in [[self resourceList] keyEnumerator]) {
		ASIWebPageRequest *externalResourceRequest = [ASIWebPageRequest requestWithURL:[NSURL URLWithString:theURL relativeToURL:[self url]]];
		[externalResourceRequest setRequestHeaders:[self requestHeaders]];
		[externalResourceRequest setDownloadCache:[self downloadCache]];
		[externalResourceRequest setCachePolicy:[self cachePolicy]];
		[externalResourceRequest setCacheStoragePolicy:[self cacheStoragePolicy]];
		[externalResourceRequest setUserInfo:[NSDictionary dictionaryWithObject:theURL forKey:@"Path"]];
		[externalResourceRequest setParentRequest:self];
		[externalResourceRequest setUrlReplacementMode:[self urlReplacementMode]];
		[externalResourceRequest setShouldResetDownloadProgress:NO];
		[externalResourceRequest setDelegate:self];
		[externalResourceRequest setUploadProgressDelegate:self];
		[externalResourceRequest setDownloadProgressDelegate:self];
		if ([self downloadDestinationPath]) {
			[externalResourceRequest setDownloadDestinationPath:[self cachePathForRequest:externalResourceRequest]];
		}
		[[self externalResourceQueue] addOperation:externalResourceRequest];
	}
	[[self externalResourceQueue] go];
}

- (void)externalResourceFetchSucceeded:(ASIHTTPRequest *)externalResourceRequest
{
	NSString *originalPath = [[externalResourceRequest userInfo] objectForKey:@"Path"];
	NSMutableDictionary *requestResponse = [[self resourceList] objectForKey:originalPath];
	NSString *contentType = [[externalResourceRequest responseHeaders] objectForKey:@"Content-Type"];
	if (!contentType) {
		contentType = @"application/octet-stream";
	}
	[requestResponse setObject:contentType forKey:@"ContentType"];
	if ([self downloadDestinationPath]) {
		[requestResponse setObject:[externalResourceRequest downloadDestinationPath] forKey:@"DataPath"];
	} else {
		NSData *data = [externalResourceRequest responseData];
		if (data) {
			[requestResponse setObject:data forKey:@"Data"];
		}
	}
}

- (void)externalResourceFetchFailed:(ASIHTTPRequest *)externalResourceRequest
{
	if ([[self externalResourceQueue] shouldCancelAllRequestsOnFailure]) {
		[self failWithError:[externalResourceRequest error]];
	}
}

- (void)finishedFetchingExternalResources:(ASINetworkQueue *)queue
{
	if ([self urlReplacementMode] != ASIDontModifyURLs) {
		if (webContentType == ASICSSWebContentType) {
			NSMutableString *parsedResponse;
			NSError *err = nil;
			if ([self downloadDestinationPath]) {
				parsedResponse = [NSMutableString stringWithContentsOfFile:[self downloadDestinationPath] encoding:[self responseEncoding] error:&err];
			} else {
				parsedResponse = [[[self responseString] mutableCopy] autorelease];
			}
			if (err) {
				[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:101 userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Error: unable to read response CSS from disk",NSLocalizedDescriptionKey,nil]]];
				return;
			}
			if (![self error]) {
				for (NSString *resource in [[self resourceList] keyEnumerator]) {
					if ([parsedResponse rangeOfString:resource].location != NSNotFound) {
						NSString *newURL = [self contentForExternalURL:resource];
						if (newURL) {
							[parsedResponse replaceOccurrencesOfString:resource withString:newURL options:0 range:NSMakeRange(0, [parsedResponse length])];
						}
					}
				}
			}
			if ([self downloadDestinationPath]) {
				[parsedResponse writeToFile:[self downloadDestinationPath] atomically:NO encoding:[self responseEncoding] error:&err];
				if (err) {
					[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:101 userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Error: unable to write response CSS to disk",NSLocalizedDescriptionKey,nil]]];
					return;
				}
			} else {
				[self setRawResponseData:(id)[parsedResponse dataUsingEncoding:[self responseEncoding]]];
			}
		} else {
			[xmlParsingLock lock];

			[self updateResourceURLs];

			if (![self error]) {

				// We'll use the xmlsave API so we can strip the xml declaration
				xmlSaveCtxtPtr saveContext;

				if ([self downloadDestinationPath]) {

					// Truncate the file first
					[[[[NSFileManager alloc] init] autorelease] createFileAtPath:[self downloadDestinationPath] contents:nil attributes:nil];

					saveContext = xmlSaveToFd([[NSFileHandle fileHandleForWritingAtPath:[self downloadDestinationPath]] fileDescriptor],NULL,2); // 2 == XML_SAVE_NO_DECL, this isn't declared on Mac OS 10.5
					xmlSaveDoc(saveContext, doc);
					xmlSaveClose(saveContext);

				} else {
	#if TARGET_OS_MAC && MAC_OS_X_VERSION_MAX_ALLOWED <= __MAC_10_5
					// xmlSaveToBuffer() is not implemented in the 10.5 version of libxml
					NSString *tempPath = [NSTemporaryDirectory() stringByAppendingPathComponent:[[NSProcessInfo processInfo] globallyUniqueString]];
					[[[[NSFileManager alloc] init] autorelease] createFileAtPath:tempPath contents:nil attributes:nil];
					saveContext = xmlSaveToFd([[NSFileHandle fileHandleForWritingAtPath:tempPath] fileDescriptor],NULL,2); // 2 == XML_SAVE_NO_DECL, this isn't declared on Mac OS 10.5
					xmlSaveDoc(saveContext, doc);
					xmlSaveClose(saveContext);
					[self setRawResponseData:[NSMutableData dataWithContentsOfFile:tempPath]];
	#else
					xmlBufferPtr buffer = xmlBufferCreate();
					saveContext = xmlSaveToBuffer(buffer,NULL,2); // 2 == XML_SAVE_NO_DECL, this isn't declared on Mac OS 10.5
					xmlSaveDoc(saveContext, doc);
					xmlSaveClose(saveContext);
					[self setRawResponseData:[[[NSMutableData alloc] initWithBytes:buffer->content length:buffer->use] autorelease]];
					xmlBufferFree(buffer);
	#endif
				}

				// Strip the content encoding if the original response was gzipped
				if ([self isResponseCompressed]) {
					NSMutableDictionary *headers = [[[self responseHeaders] mutableCopy] autorelease];
					[headers removeObjectForKey:@"Content-Encoding"];
					[self setResponseHeaders:headers];
				}
			}

			xmlFreeDoc(doc);
			doc = nil;

			[requestsUsingXMLParser removeObject:self];
			if (![requestsUsingXMLParser count]) {
				xmlCleanupParser();
			}
			[xmlParsingLock unlock];
		}
	}
	if (![self parentRequest]) {
		[[self class] updateProgressIndicator:&downloadProgressDelegate withProgress:contentLength ofTotal:contentLength];
	}

	NSMutableDictionary *newHeaders = [[[self responseHeaders] mutableCopy] autorelease];
	[newHeaders removeObjectForKey:@"Content-Encoding"];
	[self setResponseHeaders:newHeaders];

	// Write the parsed content back to the cache
	if ([self urlReplacementMode] != ASIDontModifyURLs) {
		[[self downloadCache] storeResponseForRequest:self maxAge:[self secondsToCache]];
	}

	[super requestFinished];
	[super markAsFinished];
}

- (void)readResourceURLs
{
	// Create xpath evaluation context
    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:101 userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Error: unable to create new XPath context",NSLocalizedDescriptionKey,nil]]];
		return;
    }

    // Evaluate xpath expression
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
    if(xpathObj == NULL) {
        xmlXPathFreeContext(xpathCtx); 
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:101 userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Error: unable to evaluate XPath expression!",NSLocalizedDescriptionKey,nil]]];
		return;
    }
	
	// Now loop through our matches
	xmlNodeSetPtr nodes = xpathObj->nodesetval;

    int size = (nodes) ? nodes->nodeNr : 0;
	int i;
    for(i = size - 1; i >= 0; i--) {
		assert(nodes->nodeTab[i]);
		NSString *parentName  = [NSString stringWithCString:(char *)nodes->nodeTab[i]->parent->name encoding:[self responseEncoding]];
		NSString *nodeName = [NSString stringWithCString:(char *)nodes->nodeTab[i]->name encoding:[self responseEncoding]];

		xmlChar *nodeValue = xmlNodeGetContent(nodes->nodeTab[i]);
		NSString *value = [NSString stringWithCString:(char *)nodeValue encoding:[self responseEncoding]];
		xmlFree(nodeValue);

		// Our xpath query matched all <link> elements, but we're only interested in stylesheets
		// We do the work here rather than in the xPath query because the query is case-sensitive, and we want to match on 'stylesheet', 'StyleSHEEt' etc
		if ([[parentName lowercaseString] isEqualToString:@"link"]) {
			xmlChar *relAttribute = xmlGetNoNsProp(nodes->nodeTab[i]->parent,(xmlChar *)"rel");
			if (relAttribute) {
				NSString *rel = [NSString stringWithCString:(char *)relAttribute encoding:[self responseEncoding]];
				xmlFree(relAttribute);
				if ([[rel lowercaseString] isEqualToString:@"stylesheet"]) {
					[self addURLToFetch:value];
				}
			}

		// Parse the content of <style> tags and style attributes to find external image urls or external css files
		} else if ([[nodeName lowercaseString] isEqualToString:@"style"]) {
			NSArray *externalResources = [[self class] CSSURLsFromString:value];
			for (NSString *theURL in externalResources) {
				[self addURLToFetch:theURL];
			}

		// Parse the content of <source src=""> tags (HTML 5 audio + video)
		// We explictly disable the download of files with .webm, .ogv and .ogg extensions, since it's highly likely they won't be useful to us
		} else if ([[parentName lowercaseString] isEqualToString:@"source"] || [[parentName lowercaseString] isEqualToString:@"audio"]) {
			NSString *fileExtension = [[value pathExtension] lowercaseString];
			if (![fileExtension isEqualToString:@"ogg"] && ![fileExtension isEqualToString:@"ogv"] && ![fileExtension isEqualToString:@"webm"]) {
				[self addURLToFetch:value];
			}

		// For all other elements matched by our xpath query (except hyperlinks), add the content as an external url to fetch
		} else if (![[parentName lowercaseString] isEqualToString:@"a"]) {
			[self addURLToFetch:value];
		}
		if (nodes->nodeTab[i]->type != XML_NAMESPACE_DECL) {
			nodes->nodeTab[i] = NULL;
		}
    }
	
	xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx); 
}

- (void)addURLToFetch:(NSString *)newURL
{
	// Get rid of any surrounding whitespace
	newURL = [newURL stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
	// Don't attempt to fetch data URIs
	if ([newURL length] > 4) {
		if (![[[newURL substringToIndex:5] lowercaseString] isEqualToString:@"data:"]) {
			NSURL *theURL = [NSURL URLWithString:newURL relativeToURL:[self url]];
			if (theURL) {
				if (![[self resourceList] objectForKey:newURL]) {
					[[self resourceList] setObject:[NSMutableDictionary dictionary] forKey:newURL];
				}
			}
		}
	}
}


- (void)updateResourceURLs
{
	// Create xpath evaluation context
	xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
	if(xpathCtx == NULL) {
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:101 userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Error: unable to create new XPath context",NSLocalizedDescriptionKey,nil]]];
		return;
	}

 	// Evaluate xpath expression
	xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
	if(xpathObj == NULL) {
		xmlXPathFreeContext(xpathCtx);
		[self failWithError:[NSError errorWithDomain:NetworkRequestErrorDomain code:101 userInfo:[NSDictionary dictionaryWithObjectsAndKeys:@"Error: unable to evaluate XPath expression!",NSLocalizedDescriptionKey,nil]]];
		return;
	}

	// Loop through all the matches, replacing urls where nescessary
	xmlNodeSetPtr nodes = xpathObj->nodesetval;
	int size = (nodes) ? nodes->nodeNr : 0;
	int i;
	for(i = size - 1; i >= 0; i--) {
		assert(nodes->nodeTab[i]);
		NSString *parentName  = [NSString stringWithCString:(char *)nodes->nodeTab[i]->parent->name encoding:[self responseEncoding]];
		NSString *nodeName  = [NSString stringWithCString:(char *)nodes->nodeTab[i]->name encoding:[self responseEncoding]];

		xmlChar *nodeValue = xmlNodeGetContent(nodes->nodeTab[i]);
		NSString *value = [NSString stringWithCString:(char *)nodeValue encoding:[self responseEncoding]];
		xmlFree(nodeValue);

		// Replace external urls in <style> tags or in style attributes
		if ([[nodeName lowercaseString] isEqualToString:@"style"]) {
			NSArray *externalResources = [[self class] CSSURLsFromString:value];
			for (NSString *theURL in externalResources) {
				if ([value rangeOfString:theURL].location != NSNotFound) {
					NSString *newURL = [self contentForExternalURL:theURL];
					if (newURL) {
						value = [value stringByReplacingOccurrencesOfString:theURL withString:newURL];
					}
				}
			}
			xmlNodeSetContent(nodes->nodeTab[i], (xmlChar *)[value cStringUsingEncoding:[self responseEncoding]]);

		// Replace relative hyperlinks with absolute ones, since we will need to set a local baseURL when loading this in a web view
		} else if ([self urlReplacementMode] == ASIReplaceExternalResourcesWithLocalURLs && [[parentName lowercaseString] isEqualToString:@"a"]) {
			NSString *newURL = [[NSURL URLWithString:value relativeToURL:[self url]] absoluteString];
			if (newURL) {
				xmlNodeSetContent(nodes->nodeTab[i], (xmlChar *)[newURL cStringUsingEncoding:[self responseEncoding]]);
			}

		// Replace all other external resource urls
		} else {
			NSString *newURL = [self contentForExternalURL:value];
			if (newURL) {
				xmlNodeSetContent(nodes->nodeTab[i], (xmlChar *)[newURL cStringUsingEncoding:[self responseEncoding]]);
			}
		}

		if (nodes->nodeTab[i]->type != XML_NAMESPACE_DECL) {
			nodes->nodeTab[i] = NULL;
		}
	}
	xmlXPathFreeObject(xpathObj);
	xmlXPathFreeContext(xpathCtx);
}

// The three methods below are responsible for forwarding delegate methods we want to handle to the parent request's approdiate delegate
// Certain delegate methods are ignored (eg setProgress: / setDoubleValue: / setMaxValue:)
- (BOOL)respondsToSelector:(SEL)selector
{
	if ([self parentRequest]) {
		return [[self parentRequest] respondsToSelector:selector];
	}
	//Ok, now check for selectors we want to pass on to the delegate
	if (selector == @selector(requestStarted:) || selector == @selector(request:didReceiveResponseHeaders:) || selector == @selector(request:willRedirectToURL:) || selector == @selector(requestFinished:) || selector == @selector(requestFailed:) || selector == @selector(request:didReceiveData:) || selector == @selector(authenticationNeededForRequest:) || selector == @selector(proxyAuthenticationNeededForRequest:)) {
		return [delegate respondsToSelector:selector];
	} else if (selector == @selector(request:didReceiveBytes:) || selector == @selector(request:incrementDownloadSizeBy:)) {
		return [downloadProgressDelegate respondsToSelector:selector];
	} else if (selector == @selector(request:didSendBytes:)  || selector == @selector(request:incrementUploadSizeBy:)) {
		return [uploadProgressDelegate respondsToSelector:selector];
	}
	return [super respondsToSelector:selector];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)selector
{
	if ([self parentRequest]) {
		return [[self parentRequest] methodSignatureForSelector:selector];
	}
	if (selector == @selector(requestStarted:) || selector == @selector(request:didReceiveResponseHeaders:) || selector == @selector(request:willRedirectToURL:) || selector == @selector(requestFinished:) || selector == @selector(requestFailed:) || selector == @selector(request:didReceiveData:) || selector == @selector(authenticationNeededForRequest:) || selector == @selector(proxyAuthenticationNeededForRequest:)) {
		return [(id)delegate methodSignatureForSelector:selector];
	} else if (selector == @selector(request:didReceiveBytes:) || selector == @selector(request:incrementDownloadSizeBy:)) {
		return [(id)downloadProgressDelegate methodSignatureForSelector:selector];
	} else if (selector == @selector(request:didSendBytes:)  || selector == @selector(request:incrementUploadSizeBy:)) {
		return [(id)uploadProgressDelegate methodSignatureForSelector:selector];
	}
	return nil;
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	if ([self parentRequest]) {
		return [[self parentRequest] forwardInvocation:anInvocation];
	}
	SEL selector = [anInvocation selector];
	if (selector == @selector(requestStarted:) || selector == @selector(request:didReceiveResponseHeaders:) || selector == @selector(request:willRedirectToURL:) || selector == @selector(requestFinished:) || selector == @selector(requestFailed:) || selector == @selector(request:didReceiveData:) || selector == @selector(authenticationNeededForRequest:) || selector == @selector(proxyAuthenticationNeededForRequest:)) {
		[anInvocation invokeWithTarget:delegate];
	} else if (selector == @selector(request:didReceiveBytes:) || selector == @selector(request:incrementDownloadSizeBy:)) {
		[anInvocation invokeWithTarget:downloadProgressDelegate];
	} else if (selector == @selector(request:didSendBytes:)  || selector == @selector(request:incrementUploadSizeBy:)) {
		[anInvocation invokeWithTarget:uploadProgressDelegate];
	}
}

// A quick and dirty way to build a list of external resource urls from a css string
+ (NSArray *)CSSURLsFromString:(NSString *)string
{
	NSMutableArray *urls = [NSMutableArray array];
	NSScanner *scanner = [NSScanner scannerWithString:string];
	[scanner setCaseSensitive:NO];
	while (1) {
		NSString *theURL = nil;
		[scanner scanUpToString:@"url(" intoString:NULL];
		[scanner scanString:@"url(" intoString:NULL];
		[scanner scanUpToString:@")" intoString:&theURL];
		if (!theURL) {
			break;
		}
		// Remove any quotes or whitespace around the url
		theURL = [theURL stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
		theURL = [theURL stringByTrimmingCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@"\"'"]];
		theURL = [theURL stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
		[urls addObject:theURL];
	}
	return urls;
}

// Returns a relative file path from sourcePath to destinationPath (eg ../../foo/bar.txt)
- (NSString *)relativePathTo:(NSString *)destinationPath fromPath:(NSString *)sourcePath
{
	NSArray *sourcePathComponents = [sourcePath pathComponents];
	NSArray *destinationPathComponents = [destinationPath pathComponents];
	NSUInteger i;
	NSString *newPath = @"";
	NSString *sourcePathComponent, *destinationPathComponent;
	for (i=0; i<[sourcePathComponents count]; i++) {
		sourcePathComponent = [sourcePathComponents objectAtIndex:i];
		if ([destinationPathComponents count] > i) {
			destinationPathComponent = [destinationPathComponents objectAtIndex:i];
			if (![sourcePathComponent isEqualToString:destinationPathComponent]) {
				NSUInteger i2;
				for (i2=i+1; i2<[sourcePathComponents count]; i2++) {
					newPath = [newPath stringByAppendingPathComponent:@".."];
				}
				newPath = [newPath stringByAppendingPathComponent:destinationPathComponent];
				for (i2=i+1; i2<[destinationPathComponents count]; i2++) {
					newPath = [newPath stringByAppendingPathComponent:[destinationPathComponents objectAtIndex:i2]];
				}
				break;
			}
		}
	}
	return newPath;
}

- (NSString *)contentForExternalURL:(NSString *)theURL
{
	if ([self urlReplacementMode] == ASIReplaceExternalResourcesWithLocalURLs) {
		NSString *resourcePath = [[resourceList objectForKey:theURL] objectForKey:@"DataPath"];
		return [self relativePathTo:resourcePath fromPath:[self downloadDestinationPath]];
	}
	NSData *data;
	if ([[resourceList objectForKey:theURL] objectForKey:@"DataPath"]) {
		data = [NSData dataWithContentsOfFile:[[resourceList objectForKey:theURL] objectForKey:@"DataPath"]];
	} else {
		data = [[resourceList objectForKey:theURL] objectForKey:@"Data"];
	}
	NSString *contentType = [[resourceList objectForKey:theURL] objectForKey:@"ContentType"];
	if (data && contentType) {
		NSString *dataURI = [NSString stringWithFormat:@"data:%@;base64,",contentType];
		dataURI = [dataURI stringByAppendingString:[ASIHTTPRequest base64forData:data]];
		return dataURI;
	}
	return nil;
}

- (NSString *)cachePathForRequest:(ASIWebPageRequest *)theRequest
{
	// If we're using a download cache (and its a good idea to do so when using ASIWebPageRequest), ask it for the location to store this file
	// This ends up being quite efficient, as we download directly to the cache
	if ([self downloadCache]) {
		return [[self downloadCache] pathToStoreCachedResponseDataForRequest:theRequest];

	// This is a fallback for when we don't have a download cache - we store the external resource in a file in the temporary directory
	} else {
		// Borrowed from: http://stackoverflow.com/questions/652300/using-md5-hash-on-a-string-in-cocoa
		const char *cStr = [[[theRequest url] absoluteString] UTF8String];
		unsigned char result[16];
		CC_MD5(cStr, (CC_LONG)strlen(cStr), result);
		NSString *md5 = [NSString stringWithFormat:@"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",result[0], result[1], result[2], result[3], result[4], result[5], result[6], result[7],result[8], result[9], result[10], result[11],result[12], result[13], result[14], result[15]]; 	
		return [[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0] stringByAppendingPathComponent:[md5 stringByAppendingPathExtension:@"html"]];
	}
}


@synthesize externalResourceQueue;
@synthesize resourceList;
@synthesize parentRequest;
@synthesize urlReplacementMode;
@synthesize shouldIgnoreExternalResourceErrors;
@end

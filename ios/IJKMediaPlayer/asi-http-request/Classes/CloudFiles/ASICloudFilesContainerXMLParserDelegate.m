//
//  ASICloudFilesContainerXMLParserDelegate.m
//
//  Created by Michael Mayo on 1/10/10.
//

#import "ASICloudFilesContainerXMLParserDelegate.h"
#import "ASICloudFilesContainer.h"


@implementation ASICloudFilesContainerXMLParserDelegate

@synthesize containerObjects, currentElement, currentContent, currentObject;

#pragma mark -
#pragma mark XML Parser Delegate

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict {
	[self setCurrentElement:elementName];
	
	if ([elementName isEqualToString:@"container"]) {
		[self setCurrentObject:[ASICloudFilesContainer container]];
	}
	[self setCurrentContent:@""];
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName {

	if ([elementName isEqualToString:@"name"]) {
		[self currentObject].name = [self currentContent];
	} else if ([elementName isEqualToString:@"count"]) {
		[self currentObject].count = [[self currentContent] intValue];
	} else if ([elementName isEqualToString:@"bytes"]) {
		[self currentObject].bytes = [[self currentContent] intValue];
	} else if ([elementName isEqualToString:@"cdn_enabled"]) {
		[self currentObject].cdnEnabled = [[self currentObject] isEqual:@"True"];
	} else if ([elementName isEqualToString:@"ttl"]) {
		[self currentObject].ttl = [[self currentContent] intValue];
	} else if ([elementName isEqualToString:@"cdn_url"]) {
		[self currentObject].cdnURL = [self currentContent];
	} else if ([elementName isEqualToString:@"log_retention"]) {
		[self currentObject].logRetention = [[self currentObject] isEqual:@"True"];
	} else if ([elementName isEqualToString:@"referrer_acl"]) {
		[self currentObject].referrerACL = [self currentContent];
	} else if ([elementName isEqualToString:@"useragent_acl"]) {
		[self currentObject].useragentACL = [self currentContent];
	} else if ([elementName isEqualToString:@"container"]) {
		// we're done with this container.  time to move on to the next
		if (containerObjects == nil) {
			containerObjects = [[NSMutableArray alloc] init];
		}
		[containerObjects addObject:currentObject];
		[self setCurrentObject:nil];
	}
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string {
	[self setCurrentContent:[[self currentContent] stringByAppendingString:string]];
}

#pragma mark -
#pragma mark Memory Management

- (void)dealloc {
	[containerObjects release];
	[currentElement release];
	[currentContent release];
	[currentObject release];
	[super dealloc];
}

@end

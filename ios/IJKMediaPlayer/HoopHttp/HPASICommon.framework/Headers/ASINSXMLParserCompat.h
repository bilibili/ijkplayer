//
//  ASINSXMLParserCompat.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//  This file exists to prevent warnings about the NSXMLParserDelegate protocol when building S3 or Cloud Files stuff
//
//  Created by Ben Copsey on 17/06/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//


#if (!TARGET_OS_IPHONE  && __MAC_OS_X_VERSION_MAX_ALLOWED < __MAC_10_6) || (TARGET_OS_IPHONE && __IPHONE_OS_VERSION_MAX_ALLOWED <= __IPHONE_4_0)
@protocol NSXMLParserDelegate

@optional
- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict;
- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName;

@end
#endif





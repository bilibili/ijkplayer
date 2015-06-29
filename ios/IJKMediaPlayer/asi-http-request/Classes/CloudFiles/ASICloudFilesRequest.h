//
//  ASICloudFilesRequest.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Michael Mayo on 22/12/09.  
//  mike.mayo@rackspace.com or mike@overhrd.com
//  twitter.com/greenisus
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//
// A class for accessing data stored on the Rackspace Cloud Files Service
// http://www.rackspacecloud.com/cloud_hosting_products/files
// 
// Cloud Files Developer Guide:
// http://docs.rackspacecloud.com/servers/api/cs-devguide-latest.pdf

#import <Foundation/Foundation.h>
#import "ASIHTTPRequest.h"


@interface ASICloudFilesRequest : ASIHTTPRequest {
}

+ (NSString *)storageURL;
+ (NSString *)cdnManagementURL;
+ (NSString *)authToken;

#pragma mark Rackspace Cloud Authentication

+ (id)authenticationRequest;
+ (NSError *)authenticate;
+ (NSString *)username;
+ (void)setUsername:(NSString *)username;
+ (NSString *)apiKey;
+ (void)setApiKey:(NSString *)apiKey;

// helper to parse dates in the format returned by Cloud Files
-(NSDate *)dateFromString:(NSString *)dateString;


@end

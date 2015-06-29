/*
 
 File: Reachability.h
 Abstract: Basic demonstration of how to use the SystemConfiguration Reachablity APIs.
 
 Version: 2.0.4ddg
 */

/*
 Significant additions made by Andrew W. Donoho, August 11, 2009.
 This is a derived work of Apple's Reachability v2.0 class.
 
 The below license is the new BSD license with the OSI recommended personalizations.
 <http://www.opensource.org/licenses/bsd-license.php>
 
 Extensions Copyright (C) 2009 Donoho Design Group, LLC. All Rights Reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:
 
 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 
 * Neither the name of Andrew W. Donoho nor Donoho Design Group, L.L.C.
 may be used to endorse or promote products derived from this software
 without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY DONOHO DESIGN GROUP, L.L.C. "AS IS" AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 */


/*
 
 Apple's Original License on Reachability v2.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc.
 ("Apple") in consideration of your agreement to the following terms, and your
 use, installation, modification or redistribution of this Apple software
 constitutes acceptance of these terms.  If you do not agree with these terms,
 please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject
 to these terms, Apple grants you a personal, non-exclusive license, under
 Apple's copyrights in this original Apple software (the "Apple Software"), to
 use, reproduce, modify and redistribute the Apple Software, with or without
 modifications, in source and/or binary forms; provided that if you redistribute
 the Apple Software in its entirety and without modifications, you must retain
 this notice and the following text and disclaimers in all such redistributions
 of the Apple Software.
 
 Neither the name, trademarks, service marks or logos of Apple Inc. may be used
 to endorse or promote products derived from the Apple Software without specific
 prior written permission from Apple.  Except as expressly stated in this notice,
 no other rights or licenses, express or implied, are granted by Apple herein,
 including but not limited to any patent rights that may be infringed by your
 derivative works or by other works in which the Apple Software may be
 incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
 WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
 COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
 DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF
 CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF
 APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2009 Apple Inc. All Rights Reserved.
 
 */


/*
 DDG extensions include:
 Each reachability object now has a copy of the key used to store it in a
 dictionary. This allows each observer to quickly determine if the event is
 important to them.
 
 -currentReachabilityStatus also has a significantly different decision criteria than 
 Apple's code.
 
 A multiple convenience test methods have been added.
 */

#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <netinet/in.h>

#define USE_DDG_EXTENSIONS 1 // Use DDG's Extensions to test network criteria.
// Since NSAssert and NSCAssert are used in this code, 
// I recommend you set NS_BLOCK_ASSERTIONS=1 in the release versions of your projects.

enum {
	
	// DDG NetworkStatus Constant Names.
	kNotReachable = 0, // Apple's code depends upon 'NotReachable' being the same value as 'NO'.
	kReachableViaWWAN, // Switched order from Apple's enum. WWAN is active before WiFi.
	kReachableViaWiFi
	
};
typedef	uint32_t NetworkStatus;

enum {
	
	// Apple NetworkStatus Constant Names.
	NotReachable     = kNotReachable,
	ReachableViaWiFi = kReachableViaWiFi,
	ReachableViaWWAN = kReachableViaWWAN
	
};


extern NSString *const kInternetConnection;
extern NSString *const kLocalWiFiConnection;
extern NSString *const kReachabilityChangedNotification;

@interface Reachability: NSObject {
	
@private
	NSString                *key_;
	SCNetworkReachabilityRef reachabilityRef;

}

@property (atomic, copy) NSString *key; // Atomic because network operations are asynchronous.

// Designated Initializer.
- (Reachability *) initWithReachabilityRef: (SCNetworkReachabilityRef) ref;

// Use to check the reachability of a particular host name. 
+ (Reachability *) reachabilityWithHostName: (NSString*) hostName;

// Use to check the reachability of a particular IP address. 
+ (Reachability *) reachabilityWithAddress: (const struct sockaddr_in*) hostAddress;

// Use to check whether the default route is available.  
// Should be used to, at minimum, establish network connectivity.
+ (Reachability *) reachabilityForInternetConnection;

// Use to check whether a local wifi connection is available.
+ (Reachability *) reachabilityForLocalWiFi;

//Start listening for reachability notifications on the current run loop.
- (BOOL) startNotifier;
- (void)  stopNotifier;

// Comparison routines to enable choosing actions in a notification.
- (BOOL) isEqual: (Reachability *) r;

// These are the status tests.
- (NetworkStatus) currentReachabilityStatus;

// The main direct test of reachability.
- (BOOL) isReachable;

// WWAN may be available, but not active until a connection has been established.
// WiFi may require a connection for VPN on Demand.
- (BOOL) isConnectionRequired; // Identical DDG variant.
- (BOOL)   connectionRequired; // Apple's routine.

// Dynamic, on demand connection?
- (BOOL) isConnectionOnDemand;

// Is user intervention required?
- (BOOL) isInterventionRequired;

// Routines for specific connection testing by your app.
- (BOOL) isReachableViaWWAN;
- (BOOL) isReachableViaWiFi;

- (SCNetworkReachabilityFlags) reachabilityFlags;

@end

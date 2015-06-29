//
//  AuthenticationViewController.h
//  Part of the ASIHTTPRequest sample project - see http://allseeing-i.com/ASIHTTPRequest for details
//
//  Created by Ben Copsey on 01/08/2009.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//

#import <UIKit/UIKit.h>
#import	"SampleViewController.h"
@class ASIHTTPRequest;

@interface AuthenticationViewController : SampleViewController {
	
	ASIHTTPRequest *request;
	
	UISwitch *useKeychain;
	UISwitch *useBuiltInDialog;
	UITextView *responseField;
}
- (IBAction)fetchTopSecretInformation:(id)sender;

@property (retain, nonatomic) ASIHTTPRequest *request;
@end

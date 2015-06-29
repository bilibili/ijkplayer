//
//  AuthenticationViewController.m
//  Part of the ASIHTTPRequest sample project - see http://allseeing-i.com/ASIHTTPRequest for details
//
//  Created by Ben Copsey on 01/08/2009.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//

#import "AuthenticationViewController.h"
#import "ASIHTTPRequest.h"
#import "InfoCell.h"
#import "DetailCell.h"
#import "ToggleCell.h"

@interface UIAlertView (SPI)
- (void) addTextFieldWithValue:(NSString *) value label:(NSString *) label;
- (void) addTextFieldAtIndex:(NSUInteger) index;
- (UITextField *) textFieldAtIndex:(NSUInteger) index;
@end

// Private stuff
@interface AuthenticationViewController ()
- (IBAction)topSecretFetchFailed:(ASIHTTPRequest *)theRequest;
- (IBAction)topSecretFetchComplete:(ASIHTTPRequest *)theRequest;
@end

@implementation AuthenticationViewController

- (IBAction)fetchTopSecretInformation:(id)sender
{
	[self setRequest:[ASIHTTPRequest requestWithURL:[NSURL URLWithString:@"http://allseeing-i.com/top_secret/"]]];
	[request setUseKeychainPersistence:[useKeychain isOn]];
	[request setDelegate:self];
	[request setShouldPresentAuthenticationDialog:[useBuiltInDialog isOn]];
	[request setDidFinishSelector:@selector(topSecretFetchComplete:)];
	[request setDidFailSelector:@selector(topSecretFetchFailed:)];
	[request startAsynchronous];
}

- (IBAction)topSecretFetchFailed:(ASIHTTPRequest *)theRequest
{
	[responseField setText:[[request error] localizedDescription]];
	[responseField setFont:[UIFont boldSystemFontOfSize:12]];
}

- (IBAction)topSecretFetchComplete:(ASIHTTPRequest *)theRequest
{
	[responseField setText:[request responseString]];
	[responseField setFont:[UIFont boldSystemFontOfSize:12]];
}

- (void)authenticationNeededForRequest:(ASIHTTPRequest *)theRequest
{
	UIAlertView *alertView = [[[UIAlertView alloc] initWithTitle:@"Please Login" message:[request authenticationRealm] delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"OK",nil] autorelease];
	// These are undocumented, use at your own risk!
	// A better general approach would be to subclass UIAlertView, or just use ASIHTTPRequest's built-in dialog
	[alertView addTextFieldWithValue:@"" label:@"Username"];
	[alertView addTextFieldWithValue:@"" label:@"Password"];
	[alertView show];

}

- (void)proxyAuthenticationNeededForRequest:(ASIHTTPRequest *)theRequest
{
	UIAlertView *alertView = [[[UIAlertView alloc] initWithTitle:@"Please Login to proxy" message:[request authenticationRealm] delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"OK",nil] autorelease];
	[alertView addTextFieldWithValue:@"" label:@"Username"];
	[alertView addTextFieldWithValue:@"" label:@"Password"];
	[alertView show];
}



- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	if (buttonIndex == 1) {
		if ([[self request] authenticationNeeded] == ASIHTTPAuthenticationNeeded) {
			[[self request] setUsername:[[alertView textFieldAtIndex:0] text]];
			[[self request] setPassword:[[alertView textFieldAtIndex:1] text]];
			[[self request] retryUsingSuppliedCredentials];
		} else if ([[self request] authenticationNeeded] == ASIProxyAuthenticationNeeded) {
			[[self request] setProxyUsername:[[alertView textFieldAtIndex:0] text]];
			[[self request] setProxyPassword:[[alertView textFieldAtIndex:1] text]];
			[[self request] retryUsingSuppliedCredentials];
		}
	} else {
		[[self request] cancelAuthentication];
	}
}

- (BOOL)respondsToSelector:(SEL)selector
{
	if (selector == @selector(authenticationNeededForRequest:) || selector == @selector(proxyAuthenticationNeededForRequest:)) {
		if ([useBuiltInDialog isOn]) {
			return NO;
		}
		return YES;
	}
	return [super respondsToSelector:selector];
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}


- (void)dealloc
{
	[request cancel];
	[request release];
	[responseField release];
    [super dealloc];
}

/*
 Most of the code below here relates to the table view, and isn't that interesting
 */

- (void)viewDidLoad
{
	[super viewDidLoad];
	[[[self navigationBar] topItem] setTitle:@"HTTP Authentication"];
	responseField = [[UITextView alloc] initWithFrame:CGRectZero];
	[responseField setBackgroundColor:[UIColor clearColor]];
	[responseField setEditable:NO];
	[responseField setText:@"Secret information will appear here if authentication succeeds"];
}

static NSString *intro = @"Demonstrates fetching content from an area that requires HTTP authentication. You will be prompted for a username and password, enter 'topsecret' for both.\nIf you turn on keychain support, successful authentication will result in the username and password you provided being stored in your keychain. The application will use these details rather than prompt you the next time.\nToggle 'Use built-in dialog' to switch between ASIHTTPRequest's built-in dialog, and one created by the delegate.";

- (UIView *)tableView:(UITableView *)theTableView viewForHeaderInSection:(NSInteger)section
{
	if (section == 1) {
		int tablePadding = 40;
		int tableWidth = [tableView frame].size.width;
		if (tableWidth > 480) { // iPad
			tablePadding = 110;
		}
		
		UIView *view = [[[UIView alloc] initWithFrame:CGRectMake(0,0,tableWidth-(tablePadding/2),30)] autorelease];
		UIButton *goButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
		[goButton setTitle:@"Go!" forState:UIControlStateNormal];
		[goButton sizeToFit];
		[goButton setFrame:CGRectMake([view frame].size.width-[goButton frame].size.width+10,7,[goButton frame].size.width,[goButton frame].size.height)];

		[goButton addTarget:self action:@selector(fetchTopSecretInformation:) forControlEvents:UIControlEventTouchDown];
		[view addSubview:goButton];
		
		return view;
	}
	return nil;
}

- (UITableViewCell *)tableView:(UITableView *)theTableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	int tablePadding = 40;
	int tableWidth = [tableView frame].size.width;
	if (tableWidth > 480) { // iPad
		tablePadding = 110;
	}
	
	
	UITableViewCell *cell;
	if ([indexPath section] == 0) {
		
		cell = [tableView dequeueReusableCellWithIdentifier:@"InfoCell"];
		if (!cell) {
			cell = [InfoCell cell];	
		}
		[[cell textLabel] setText:intro];
		[cell layoutSubviews];
		
	} else if ([indexPath section] == 1) {
		cell = [tableView dequeueReusableCellWithIdentifier:@"ToggleCell"];
		if (!cell) {
			cell = [ToggleCell cell];
			if ([indexPath row] == 0) {
				[[cell textLabel] setText:@"Use Keychain"];
				useKeychain = [(ToggleCell *)cell toggle];
			} else {
				[[cell textLabel] setText:@"Use Built-In Dialog"];
				useBuiltInDialog = [(ToggleCell *)cell toggle];
			}
		}

	} else {
		
		cell = [tableView dequeueReusableCellWithIdentifier:@"Response"];
		if (!cell) {
			cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"Response"] autorelease];
			

			[[cell contentView] addSubview:responseField];
		}	
		[responseField setFrame:CGRectMake(5,5,tableWidth-tablePadding,150)];
		

	}
	[cell setSelectionStyle:UITableViewCellSelectionStyleNone];
	

	return cell;
}

- (NSInteger)tableView:(UITableView *)theTableView numberOfRowsInSection:(NSInteger)section
{
	if (section == 1) {
		return 2;
	} else {
		return 1;
	}
}

- (CGFloat)tableView:(UITableView *)theTableView heightForHeaderInSection:(NSInteger)section
{
	if (section == 1) {
		return 50;
	}
	return 34;
}

- (CGFloat)tableView:(UITableView *)theTableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	if ([indexPath section] == 0) {
		return [InfoCell neededHeightForDescription:intro withTableWidth:[tableView frame].size.width]+20;
	} else if ([indexPath section] == 2) {
		return 160;
	} else {
		return 42;
	}
}

- (NSString *)tableView:(UITableView *)theTableView titleForHeaderInSection:(NSInteger)section
{
	return nil;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
	return 3;
}


@synthesize request;
@end

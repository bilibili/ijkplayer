//
//  AppDelegate.h
//
//  Created by Ben Copsey on 09/07/2008.
//  Copyright 2008 All-Seeing Interactive Ltd. All rights reserved.
//

#import <WebKit/WebKit.h>

@class ASIHTTPRequest;
@class ASINetworkQueue;

@interface AppDelegate : NSObject {
	ASINetworkQueue *networkQueue;
	IBOutlet NSProgressIndicator *progressIndicator;
	IBOutlet NSTextView *htmlSource;
	IBOutlet NSTextField *fileLocation;
	IBOutlet NSWindow *window;
	IBOutlet NSWindow *loginWindow;
	
	IBOutlet NSButton *showAccurateProgress;
	
	IBOutlet NSTextField *host;
	IBOutlet NSTextField *realm;	
	IBOutlet NSTextField *username;
	IBOutlet NSTextField *password;
	
	IBOutlet NSTextField *topSecretInfo;
	IBOutlet NSButton *keychainCheckbox;
	
	IBOutlet NSImageView *imageView1;
	IBOutlet NSImageView *imageView2;
	IBOutlet NSImageView *imageView3;
	IBOutlet NSProgressIndicator *imageProgress1;
	IBOutlet NSProgressIndicator *imageProgress2;
	IBOutlet NSProgressIndicator *imageProgress3;
	
	IBOutlet NSButton *startButton;
	IBOutlet NSButton *resumeButton;
	
	IBOutlet NSTextField *bandwidthUsed;
	
	ASIHTTPRequest *bigFetchRequest;
	IBOutlet NSTextField *postStatus;
	
	IBOutlet NSTableView *tableView;
	IBOutlet NSTextField *tableLoadStatus;
	NSMutableArray *rowData;
	ASINetworkQueue *tableQueue;

	IBOutlet WebView *webView;
	IBOutlet NSTextView *webPageSource;
	IBOutlet NSTextField *urlField;
	IBOutlet NSButton *dataURICheckbox;
}

- (IBAction)simpleURLFetch:(id)sender;
- (IBAction)URLFetchWithProgress:(id)sender;
- (IBAction)stopURLFetchWithProgress:(id)sender;
- (IBAction)resumeURLFetchWithProgress:(id)sender;

- (IBAction)fetchThreeImages:(id)sender;


- (IBAction)dismissAuthSheet:(id)sender;
- (IBAction)fetchTopSecretInformation:(id)sender;

- (IBAction)postWithProgress:(id)sender;

- (IBAction)throttleBandwidth:(id)sender;

- (IBAction)reloadTableData:(id)sender;
- (IBAction)clearCache:(id)sender;
- (IBAction)fetchWebPage:(id)sender;

@property (retain, nonatomic) ASIHTTPRequest *bigFetchRequest;
@property (retain, nonatomic) NSMutableArray *rowData;
@property (retain, nonatomic) ASINetworkQueue *tableQueue;
@end

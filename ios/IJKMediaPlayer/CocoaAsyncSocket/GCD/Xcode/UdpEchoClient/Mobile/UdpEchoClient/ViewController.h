#import <UIKit/UIKit.h>


@interface ViewController : UIViewController
{
	IBOutlet UITextField *addrField;
	IBOutlet UITextField *portField;
	IBOutlet UITextField *messageField;
	IBOutlet UIWebView *webView;
}

- (IBAction)send:(id)sender;

@end

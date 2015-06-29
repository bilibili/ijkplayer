#import <UIKit/UIKit.h>


@interface ViewController : UIViewController
{
	IBOutlet UITextField *portField;
	IBOutlet UIButton *startStopButton;
	IBOutlet UIWebView *webView;
}

- (IBAction)startStop:(id)sender;

@end

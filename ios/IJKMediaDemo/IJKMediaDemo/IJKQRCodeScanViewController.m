/*
 * Copyright (C) 2015 Gdier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import "IJKQRCodeScanViewController.h"
#import "IJKMoviePlayerViewController.h"
#import "Barcode.h"
#import <AVFoundation/AVFoundation.h>

@interface IJKQRCodeScanViewController () <AVCaptureMetadataOutputObjectsDelegate>

@property (weak, nonatomic) IBOutlet UIView *previewView;
@property (strong, nonatomic) NSMutableArray * allowedBarcodeTypes;

//@property (strong, nonatomic) SettingsViewController * settingsVC;

@end

@implementation IJKQRCodeScanViewController {

    AVCaptureSession *_captureSession;
    AVCaptureDevice *_videoDevice;
    AVCaptureDeviceInput *_videoInput;
    AVCaptureVideoPreviewLayer *_previewLayer;
    BOOL _running;
    AVCaptureMetadataOutput *_metadataOutput;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        self.title = @"Scan QRCode";
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [self setupCaptureSession];
    _previewLayer.frame = _previewView.bounds;
    [_previewView.layer addSublayer:_previewLayer];
    
    // listen for going into the background and stop the session
    
    // set default allowed barcode types, remove types via setings menu if you don't want them to be able to be scanned
    self.allowedBarcodeTypes = [NSMutableArray new];
    [self.allowedBarcodeTypes addObject:@"org.iso.QRCode"];
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillEnterForeground:)
                                                 name:UIApplicationWillEnterForegroundNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidEnterBackground:)
                                                 name:UIApplicationDidEnterBackgroundNotification
                                               object:nil];

    _previewLayer.frame = _previewView.bounds;
    
    [self setupCaptureOrientation];
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    [self startRunning];
}


- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
    [self stopRunning];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - AV capture methods

- (void)setupCaptureSession {

    if (_captureSession) {
        return;
    }

    _videoDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    if (!_videoDevice) {
        return;
    }

    _captureSession = [[AVCaptureSession alloc] init];
    _videoInput = [[AVCaptureDeviceInput alloc] initWithDevice:_videoDevice error:nil];

    if ([_captureSession canAddInput:_videoInput]) {
        [_captureSession addInput:_videoInput];
    }
    _previewLayer = [[AVCaptureVideoPreviewLayer alloc] initWithSession:_captureSession];
    _previewLayer.videoGravity = AVLayerVideoGravityResizeAspectFill;
    
    // capture and process the metadata
    _metadataOutput = [[AVCaptureMetadataOutput alloc] init];

    [_metadataOutput setMetadataObjectsDelegate:self
                                          queue:dispatch_get_main_queue()];
    
    if ([_captureSession canAddOutput:_metadataOutput]) {
        [_captureSession addOutput:_metadataOutput];
    }
}

- (void)setupCaptureOrientation {
    if([_previewLayer.connection isVideoOrientationSupported]) {
        AVCaptureVideoOrientation orientation;
        
        switch ([[UIApplication sharedApplication] statusBarOrientation]) {
            case UIInterfaceOrientationLandscapeLeft:
                orientation = AVCaptureVideoOrientationLandscapeLeft;
                break;
                
            case UIInterfaceOrientationLandscapeRight:
                orientation = AVCaptureVideoOrientationLandscapeRight;
                break;
                
            default:
                orientation = AVCaptureVideoOrientationLandscapeLeft;
                break;
        }
        
        [_previewLayer.connection setVideoOrientation:orientation];
    }
}

- (void)startRunning {
    if (_running) return;
    [_captureSession startRunning];
    _metadataOutput.metadataObjectTypes = @[AVMetadataObjectTypeQRCode];//_metadataOutput.availableMetadataObjectTypes;

    _running = YES;
}

- (void)stopRunning {
    if (!_running) return;
    [_captureSession stopRunning];
    _running = NO;
}

- (void)applicationWillEnterForeground:(NSNotification*)note {
    [self startRunning];
}
- (void)applicationDidEnterBackground:(NSNotification*)note {
    [self stopRunning];
}

#pragma mark - Delegate functions

- (void)captureOutput:(AVCaptureOutput *)captureOutput
didOutputMetadataObjects:(NSArray *)metadataObjects
       fromConnection:(AVCaptureConnection *)connection {
    
    [metadataObjects enumerateObjectsUsingBlock:^(AVMetadataObject *obj, NSUInteger idx, BOOL *stop) {
         if ([obj isKindOfClass:[AVMetadataMachineReadableCodeObject class]]) {

             AVMetadataMachineReadableCodeObject *code = (AVMetadataMachineReadableCodeObject *) [_previewLayer transformedMetadataObjectForMetadataObject:obj];

             Barcode * barcode = [Barcode processMetadataObject:code];
             
             for(NSString * str in self.allowedBarcodeTypes) {
                 if([barcode.getBarcodeType isEqualToString:str]) {
                     [self validBarcodeFound:barcode];
                     return;
                 }
             }
         }
     }];
}

- (void) validBarcodeFound:(Barcode *)barcode {
    [self stopRunning];
    [self showBarcodeAlert:barcode];
}

- (void) showBarcodeAlert:(Barcode *)barcode {
    UIAlertView *message = [[UIAlertView alloc] initWithTitle:nil
                                                      message:[barcode getBarcodeData]
                                                     delegate:self
                                            cancelButtonTitle:@"Scan Again"
                                            otherButtonTitles:@"Play", nil];
    
    [message show];
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex {
    if(buttonIndex == alertView.cancelButtonIndex) {
        [self startRunning];
    } else {
        NSURL *url = [NSURL URLWithString:alertView.message];
        
        [IJKVideoViewController presentFromViewController:self withTitle:[NSString stringWithFormat:@"URL: %@", url] URL:url completion:^{
            [self.navigationController popViewControllerAnimated:NO];
        }];
    }
}

@end

//
//  ASICloudFilesRequestTests.h
//
//  Created by Michael Mayo on 1/6/10.
//

#import "ASITestCase.h"

@class ASINetworkQueue;

@interface ASICloudFilesRequestTests : ASITestCase {
	ASINetworkQueue *networkQueue;
	float progress;
}

@property (retain,nonatomic) ASINetworkQueue *networkQueue;

@end

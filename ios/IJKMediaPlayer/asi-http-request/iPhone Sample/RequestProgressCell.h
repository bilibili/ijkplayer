//
//  RequestProgressCell.h
//  iPhone
//
//  Created by Ben Copsey on 03/10/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface RequestProgressCell : UITableViewCell {
	UIProgressView *progressView;
}
+ (id)cell;

@property (retain, nonatomic) UIProgressView *progressView;
@end

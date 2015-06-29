//
//  ToggleCell.h
//  Part of the ASIHTTPRequest sample project - see http://allseeing-i.com/ASIHTTPRequest for details
//
//  Created by Ben Copsey on 17/06/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface ToggleCell : UITableViewCell {
	UISwitch *toggle;
}
+ (id)cell;

@property (assign, nonatomic) UISwitch *toggle;
@end

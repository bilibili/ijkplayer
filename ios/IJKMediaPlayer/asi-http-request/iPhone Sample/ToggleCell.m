//
//  ToggleCell.m
//  Part of the ASIHTTPRequest sample project - see http://allseeing-i.com/ASIHTTPRequest for details
//
//  Created by Ben Copsey on 17/06/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "ToggleCell.h"


@implementation ToggleCell

+ (id)cell
{
	ToggleCell *cell = [[[ToggleCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:@"ToggleCell"] autorelease];
	[[cell textLabel] setTextAlignment:UITextAlignmentLeft];
	[cell setToggle:[[[UISwitch alloc] initWithFrame:CGRectMake(0,0,20,20)] autorelease]];
	[cell setAccessoryView:[cell toggle]];
	return cell;
}

@synthesize toggle;
@end

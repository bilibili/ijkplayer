//
//  DetailCell.m
//  Part of the ASIHTTPRequest sample project - see http://allseeing-i.com/ASIHTTPRequest for details
//
//  Created by Ben Copsey on 16/06/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "DetailCell.h"


@implementation DetailCell

+ (id)cell
{
	DetailCell *cell = [[[DetailCell alloc] initWithStyle:UITableViewCellStyleValue2 reuseIdentifier:@"HeaderCell"] autorelease];
	[[cell detailTextLabel] setTextAlignment:UITextAlignmentLeft];
	[[cell detailTextLabel] setFont:[UIFont systemFontOfSize:14]];
	return cell;
}

- (void)layoutSubviews
{
	[super layoutSubviews];
	int tablePadding = 40;
	int tableWidth = [[self superview] frame].size.width;
	if (tableWidth > 480) { // iPad
		tablePadding = 110;
	}
	[[self textLabel] setFrame:CGRectMake(5,5,120,20)];
	[[self detailTextLabel] setFrame:CGRectMake(135,5,[self frame].size.width-145-tablePadding,20)];
	
}

@end

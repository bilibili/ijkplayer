//
//  InfoCell.m
//  Part of the ASIHTTPRequest sample project - see http://allseeing-i.com/ASIHTTPRequest for details
//
//  Created by Ben Copsey on 17/06/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import "InfoCell.h"


@implementation InfoCell

+ (id)cell
{
	InfoCell *cell = [[[InfoCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"InfoCell"] autorelease];
	if ([[UIScreen mainScreen] bounds].size.width > 480) { // iPad
		[[cell textLabel] setFont:[UIFont systemFontOfSize:14]];
	} else {
		[[cell textLabel] setFont:[UIFont systemFontOfSize:13]];
	}
	[[cell textLabel] setLineBreakMode:UILineBreakModeWordWrap];
	[[cell textLabel] setNumberOfLines:0];
	
	if ([[UIScreen mainScreen] bounds].size.width > 480) { // iPad
		UIImageView *imageView = [[[UIImageView alloc] initWithFrame:CGRectMake(10,10,48,48)] autorelease];
		[imageView setImage:[UIImage imageNamed:@"info.png"]];
		[[cell contentView] addSubview:imageView];
	}
	return cell;	
}

- (void)layoutSubviews
{
	[super layoutSubviews];
	int tablePadding = 40;
	int tableWidth = [[self superview] frame].size.width;
	if (tableWidth > 480) { // iPad
		tablePadding = 110;
		[[self textLabel] setFrame:CGRectMake(70,10,tableWidth-tablePadding-70,[[self class] neededHeightForDescription:[[self textLabel] text] withTableWidth:tableWidth])];	
	} else {
		[[self textLabel] setFrame:CGRectMake(10,10,tableWidth-tablePadding,[[self class] neededHeightForDescription:[[self textLabel] text] withTableWidth:tableWidth])];	
	}

}

+ (NSUInteger)neededHeightForDescription:(NSString *)description withTableWidth:(NSUInteger)tableWidth
{
	int tablePadding = 40;
	int offset = 0;
	int textSize = 13;
	if (tableWidth > 480) { // iPad
		tablePadding = 110;
		offset = 70;
		textSize = 14;
	}
	CGSize labelSize = [description sizeWithFont:[UIFont systemFontOfSize:textSize] constrainedToSize:CGSizeMake(tableWidth-tablePadding-offset,1000) lineBreakMode:UILineBreakModeWordWrap];
	if (labelSize.height < 48) {
		return 58;
	}
	return labelSize.height;
}

@end

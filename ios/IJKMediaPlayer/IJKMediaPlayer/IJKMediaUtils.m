/*
 * IJKMediaUtils.m
 *
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#import "IJKMediaUtils.h"

@implementation IJKMediaUtils

+ (NSError*)createErrorWithDomain: (NSString*)domain
                             code: (NSInteger)code
                      description: (NSString*)description
                           reason: (NSString*)reason
{
    /* Generate an error describing the failure. */
    if (description == nil)
        description = @"";
    if (reason == nil)
        reason = @"";

    NSString *localizedDescription = NSLocalizedString(description, description);
    NSString *localizedFailureReason = NSLocalizedString(reason, reason);
    NSDictionary *errorDict = [NSDictionary dictionaryWithObjectsAndKeys:
                               localizedDescription, NSLocalizedDescriptionKey,
                               localizedFailureReason, NSLocalizedFailureReasonErrorKey,
                               nil];
    NSError *error = [NSError errorWithDomain:domain
                                         code:0
                                     userInfo:errorDict];
    return error;
}

@end

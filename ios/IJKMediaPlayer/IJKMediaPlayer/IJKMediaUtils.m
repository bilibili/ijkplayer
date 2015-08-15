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

+ (int)createTempFDForFFConcat
{
    return [IJKMediaUtils createTempFDWithPrefix:@"ffconcat-"];
}

+ (int)createTempFDWithPrefix: (NSString*)aPrefix
{
    return [IJKMediaUtils createTempFDInDirectory: NSTemporaryDirectory()
                                       withPrefix: aPrefix];
}

+ (int)createTempFDInDirectory: (NSString*)aDirectory
                    withPrefix: (NSString*)aPrefix
{
    NSString* templateStr = [NSString stringWithFormat:@"%@/%@XXXXXX", aDirectory, aPrefix];
    char template[templateStr.length + 32];
    strcpy(template, [templateStr cStringUsingEncoding:NSASCIIStringEncoding]);
    int fd = mkstemp(template);
    if (fd <= 0) {
        NSLog(@"Could not create fd in directory %@", aDirectory);
        return -1;
    }
    unlink(template);
    return fd;
}

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

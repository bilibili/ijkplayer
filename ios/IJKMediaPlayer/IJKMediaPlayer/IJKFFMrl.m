/*
 * IJKFFMrl.m
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

#import "IJKFFMrl.h"
#import "IJKMediaUtils.h"
#include "libavutil/base64.h"
#include "libavutil/mem.h"

@implementation IJKFFMrl {
    NSString *_tmpFile;
}

@synthesize rawMrl      = _rawMrl;
@synthesize resolvedMrl = _resolvedMrl;

- (IJKFFMrl *)initWithMrl: (NSString*)aMrl
{
    self = [super init];
    if (self) {
        _rawMrl      = aMrl;
        _resolvedMrl = aMrl;
        if ([_rawMrl hasPrefix:@"data://"]) {
            NSRange range = [_rawMrl rangeOfString:@","];
            if (range.length > 0) {
                NSString *data = [_rawMrl substringFromIndex:(range.location + 1)];
                NSString *ffConcat = [IJKFFMrl base64Decode:data];

                _tmpFile = [IJKMediaUtils createTempFileNameForFFConcat];
                [ffConcat writeToFile:_tmpFile
                           atomically:YES
                             encoding:NSStringEncodingConversionAllowLossy
                                error:nil];

                _resolvedMrl = [[NSString alloc] initWithFormat:@"concat://%@", _tmpFile];
            }
        }

        if (!_resolvedMrl)
            _resolvedMrl = _rawMrl;
    }
    return self;
}

- (void) removeTempFiles
{
    if (_tmpFile) {
        [[NSFileManager defaultManager] removeItemAtPath:_tmpFile error:nil];
        _tmpFile = nil;
    }
}

+ (NSString *) base64Decode: (NSString*)cipher
{
    int ret = 0;
    const char *utf8Cipher = [cipher UTF8String];
    uint8_t *utf8Plain = NULL;
    size_t in_size = strlen(utf8Cipher);

    size_t out_size = (in_size + 1) * 3 / 4 + 1;

    if (out_size > INT_MAX || !(utf8Plain = av_mallocz(out_size)))
        return NULL;
    if ((ret = av_base64_decode(utf8Plain, utf8Cipher, (int)out_size)) < 0) {
        av_free(utf8Plain);
        NSLog(@"Invalid base64 in MRL\n");
        return NULL;
    }

    NSString *plain = [[NSString alloc] initWithUTF8String:(const char *)utf8Plain];
    av_free(utf8Plain);

    return plain;
}

@end

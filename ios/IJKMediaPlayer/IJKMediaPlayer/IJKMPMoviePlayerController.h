//
//  IJKMPMoviePlayerController.h
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-22.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <MediaPlayer/MediaPlayer.h>
#import "IJKMediaPlayback.h"

@interface IJKMPMoviePlayerController : MPMoviePlayerController <IJKMediaPlayback>

- (id)initWithContentURL:(NSURL *)url;

@property(nonatomic, weak) id<IJKMediaPlaybackDelegate> delegate;

@end

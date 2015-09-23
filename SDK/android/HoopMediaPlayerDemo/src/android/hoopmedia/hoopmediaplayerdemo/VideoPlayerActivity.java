/*
 * Copyright (C) 2013 Zhang Rui <bbcallen@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package android.hoopmedia.hoopmediaplayerdemo;

import tv.danmaku.ijk.media.widget.MediaController;
import tv.danmaku.ijk.media.widget.VideoView;
import tv.danmaku.ijk.media.player.IMediaPlayer;
import tv.danmaku.ijk.media.player.IMediaPlayer.OnCompletionListener;
import tv.danmaku.ijk.media.player.IMediaPlayer.OnErrorListener;
import tv.danmaku.ijk.media.player.IMediaPlayer.OnPreparedListener;
import android.app.Activity;
import android.os.Bundle;


public class VideoPlayerActivity extends Activity {
	private VideoView mVideoView;
	private String mVideoPath;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		mVideoPath = getIntent().getExtras().getString("RtmpAddress");

		mVideoView = new VideoView(this);
		mVideoView.setOnErrorListener(mErrorListener);
		mVideoView.setOnCompletionListener(mCompletionListener);
		mVideoView.setOnPreparedListener(mPreparedListener);
		
//		MediaController mediaController = new MediaController(this);
//		mVideoView.setMediaController(mediaController);
		
//		mVideoView.setDataSourceType(VideoView.LOWDELAY_LIVE_STREAMING_TYPE);
//		mVideoView.setDataCache(10000);//10s
		mVideoView.setVideoPath(mVideoPath);
		
		setContentView(mVideoView);
	}
	private OnPreparedListener mPreparedListener = new OnPreparedListener() {

		@Override
		public void onPrepared(IMediaPlayer arg0) {
		}
	};
	
	private OnErrorListener mErrorListener = new OnErrorListener() {

		@Override
		public boolean onError(IMediaPlayer arg0, int arg1, int arg2) {
			mVideoView.stopPlayback();
			finish();
			return true;
		}
	};

	private OnCompletionListener mCompletionListener = new OnCompletionListener() {

		@Override
		public void onCompletion(IMediaPlayer arg0) {
			mVideoView.stopPlayback();
			
			finish();
		}
	};
}


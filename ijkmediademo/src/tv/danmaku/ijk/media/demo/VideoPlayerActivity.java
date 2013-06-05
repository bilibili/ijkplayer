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

package tv.danmaku.ijk.media.demo;

import java.io.File;

import tv.danmaku.ijk.media.widget.MediaController;
import tv.danmaku.ijk.media.widget.VideoView;
import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;

public class VideoPlayerActivity extends Activity {
    private VideoView mVideoView;
    private MediaController mMediaController;

    /*-
    private String mVideoPath = "httplive://v.youku.com/player/getM3U8/vid/XNTY1ODE5NjMy/type//v.m3u8";
     */
    private String mVideoPath = "http://v.iask.com/v_play_ipad.php?vid=99979978";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_player);

        mVideoPath = new File(Environment.getExternalStorageDirectory(),
                "download/test.mp4").getAbsolutePath();

        mMediaController = new MediaController(this);

        mVideoView = (VideoView) findViewById(R.id.video_view);
        mVideoView.setMediaController(mMediaController);
        mVideoView.setVideoPath(mVideoPath);
        mVideoView.requestFocus();
        mVideoView.start();
    }
}

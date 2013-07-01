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

    private String mVideoPath;

    /*-
    private String mVideoPath = "httplive://v.youku.com/player/getM3U8/vid/XNTY1ODE5NjMy/type//v.m3u8";
     */
    // private String mVideoPath =
    // "http://v.iask.com/v_play_ipad.php?vid=99979978";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_player);

        mVideoPath = new File(Environment.getExternalStorageDirectory(),
                "download/test.mp4").getAbsolutePath();

        /*-
        mVideoPath = "http://gslb.bestvcdn.com/gslb/url/Bestv/live/live/kknews/workflow1.m3u8";
        mVideoPath = "http://iosvideo.kankanews.com/2012/07/20/h264_450k_mp4_SHYiShu201207…67640_aac.ssm/h264_450k_mp4_SHYiShu20120720210930854798181167640_aac.m3u8";
        mVideoPath = "http://video1.kksmg.com/rendition/201302/88000/39/104342962523078658/104342967891787778/r104342967891787778.m3u8";
        mVideoPath = "http://video1.kksmg.com/rendition/201305/88000/a2/110409765426823170/110409776432677378/r110409776432677378.m3u8";
        mVideoPath = "http://gslb.coop.letv.com/movie/A51456D18B8BB6865ADBA0EE972B0B7F-situoke.mp4?tag=ios";
         */

        mMediaController = new MediaController(this);

        mVideoView = (VideoView) findViewById(R.id.video_view);
        mVideoView.setMediaController(mMediaController);
        mVideoView.setVideoPath(mVideoPath);
        mVideoView.requestFocus();
        mVideoView.start();
    }
}

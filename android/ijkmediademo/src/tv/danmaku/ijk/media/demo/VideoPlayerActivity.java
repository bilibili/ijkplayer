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
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.text.TextUtils;
import android.view.View;

public class VideoPlayerActivity extends Activity {
    private VideoView mVideoView;
    private View mBufferingIndicator;
    private MediaController mMediaController;

    private String mVideoPath;

    /*-
    mVideoPath = "http://v.iask.com/v_play_ipad.php?vid=99979978";
    mVideoPath = "http://gslb.bestvcdn.com/gslb/url/Bestv/live/live/kknews/workflow1.m3u8";
    mVideoPath = "http://iosvideo.kankanews.com/2012/07/20/h264_450k_mp4_SHYiShu201207…67640_aac.ssm/h264_450k_mp4_SHYiShu20120720210930854798181167640_aac.m3u8";
    mVideoPath = "http://video1.kksmg.com/rendition/201302/88000/39/104342962523078658/104342967891787778/r104342967891787778.m3u8";
    mVideoPath = "http://video1.kksmg.com/rendition/201305/88000/a2/110409765426823170/110409776432677378/r110409776432677378.m3u8";
    mVideoPath = "http://gslb.coop.letv.com/movie/A51456D18B8BB6865ADBA0EE972B0B7F-situoke.mp4?tag=ios";
    mVideoPath = "http://gslb.bestvcdn.com/gslb/url/Bestv/live/live/kknews/workflow1.m3u8";        
    mVideoPath = "http://gslb.tv.sohu.com/live?cid=12&type=hls";

    // EXTINF floating-point
    mVideoPath = "http://v.youku.com/player/getM3U8/vid/XNTY1ODE5NjMy/type//v.m3u8";

    // buffering immediately after prepared
    mVideoPath = "http://meta.video.qiyi.com/380/56070c92f215c514e17fd04b07e22e27.m3u8";

    // 3gp seek around
    mVideoPath = "http://v.cctv.com/flash/mp4video28/TMS/2013/05/06/265114d5f2e641278098503f1676d017_h264418000nero_aac32-1.mp4";

    // slow vod
    mVideoPath = "http://wtv.v.iask.com/player/index_vod_ios.meta?record_id=201304295369";
    mVideoPath = "http://mtv.v.iask.com/manifest/201304295369_450.m3u8"

    // youku insane discontinuity time
    mVideoPath = "http://v.youku.com/player/getM3U8/vid/XNTc4NjA0MzIw/type/flv/ts/useKeyFrame/0/v.m3u8";

    // gzip
    mVideoPath = "http://64k.kankanews.com/hls-smgvod/2013/07/12/h264_450k_mp4_fc398b217ff9f93ef8fb0ea939c86766_ncd.mp4.m3u8";

    // invalid variant
    mVideoPath = "http://64k.kankanews.com/hls-smgvod/2013/06/30/f969bb5169365989b2e9bace44353a2b.m3u8";

    // 720p sina
    mVideoPath = "http://edge.v.iask.com/101692214.hlv?KID=sina,viask&Expires=1374249600&ssig=8dvnUJ%2Fb2n"

    // 1080p sina
    mVideoPath = "http://edge.v.iask.com/44897225.hlv?KID=sina,viask&Expires=1374249600&ssig=QRllIFSYp0"
     */

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_player);

        mVideoPath = "http://v.youku.com/player/getM3U8/vid/XNTY1ODE5NjMy/type//v.m3u8";

        Intent intent = getIntent();
        String intentAction = intent.getAction();
        if (!TextUtils.isEmpty(intentAction)
                && intentAction.equals(Intent.ACTION_VIEW)) {
            mVideoPath = intent.getDataString();
        }

        if (TextUtils.isEmpty(mVideoPath)) {
            mVideoPath = new File(Environment.getExternalStorageDirectory(),
                    "download/test.mp4").getAbsolutePath();
        }

        mBufferingIndicator = findViewById(R.id.buffering_indicator);
        mMediaController = new MediaController(this);

        mVideoView = (VideoView) findViewById(R.id.video_view);
        mVideoView.setMediaController(mMediaController);
        mVideoView.setMediaBufferingIndicator(mBufferingIndicator);
        mVideoView.setVideoPath(mVideoPath);
        mVideoView.requestFocus();
        mVideoView.start();
    }
}

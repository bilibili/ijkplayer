/*
 * Copyright (C) 2015 Bilibili
 * Copyright (C) 2015 Zhang Rui <bbcallen@gmail.com>
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

package tv.danmaku.ijk.media.player.misc;

import android.annotation.TargetApi;
import android.media.MediaFormat;
import android.media.MediaPlayer;
import android.os.Build;

public class AndroidTrackInfo implements ITrackInfo {
    private final MediaPlayer.TrackInfo mTrackInfo;

    public static AndroidTrackInfo[] fromMediaPlayer(MediaPlayer mp) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN)
            return fromTrackInfo(mp.getTrackInfo());

        return null;
    }

    private static AndroidTrackInfo[] fromTrackInfo(MediaPlayer.TrackInfo[] trackInfos) {
        if (trackInfos == null)
            return null;

        AndroidTrackInfo androidTrackInfo[] = new AndroidTrackInfo[trackInfos.length];
        for (int i = 0; i < trackInfos.length; ++i) {
            androidTrackInfo[i] = new AndroidTrackInfo(trackInfos[i]);
        }

        return androidTrackInfo;
    }

    private AndroidTrackInfo(MediaPlayer.TrackInfo trackInfo) {
        mTrackInfo = trackInfo;
    }

    @TargetApi(Build.VERSION_CODES.KITKAT)
    @Override
    public IMediaFormat getFormat() {
        if (mTrackInfo == null)
            return null;

        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT)
            return null;

        MediaFormat mediaFormat = mTrackInfo.getFormat();
        if (mediaFormat == null)
            return null;

        return new AndroidMediaFormat(mediaFormat);
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    @Override
    public String getLanguage() {
        if (mTrackInfo == null)
            return "und";

        return mTrackInfo.getLanguage();
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    @Override
    public int getTrackType() {
        if (mTrackInfo == null)
            return MEDIA_TRACK_TYPE_UNKNOWN;

        return mTrackInfo.getTrackType();
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    @Override
    public String toString() {
        StringBuilder out = new StringBuilder(128);
        out.append(getClass().getSimpleName());
        out.append('{');
        if (mTrackInfo != null) {
            out.append(mTrackInfo.toString());
        } else {
            out.append("null");
        }
        out.append('}');
        return out.toString();
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    @Override
    public String getInfoInline() {
        if (mTrackInfo != null) {
            return mTrackInfo.toString();
        } else {
            return "null";
        }
    }
}

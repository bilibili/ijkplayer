/*
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

import tv.danmaku.ijk.media.player.IjkMediaMeta;

public class IjkTrackInfo implements ITrackInfo {
    private int mTrackType = MEDIA_TRACK_TYPE_UNKNOWN;
    private IjkMediaMeta.IjkStreamMeta mStreamMeta;

    public IjkTrackInfo(IjkMediaMeta.IjkStreamMeta streamMeta) {
        mStreamMeta = streamMeta;
    }

    @Override
    public IMediaFormat getFormat() {
        return new IjkMediaFormat(mStreamMeta);
    }

    public void setMediaMeta(IjkMediaMeta.IjkStreamMeta streamMeta) {
        mStreamMeta = streamMeta;
    }

    @Override
    public int getTrackType() {
        return mTrackType;
    }

    public void setTrackType(int trackType) {
        mTrackType = trackType;
    }

    @Override
    public String toString() {
        StringBuilder out = new StringBuilder(128);
        out.append(getClass().getName());
        out.append('{');
        switch (mTrackType) {
            case MEDIA_TRACK_TYPE_VIDEO:
                out.append("VIDEO");
                break;
            case MEDIA_TRACK_TYPE_AUDIO:
                out.append("AUDIO");
                break;
            case MEDIA_TRACK_TYPE_TIMEDTEXT:
                out.append("TIMEDTEXT");
                break;
            case MEDIA_TRACK_TYPE_SUBTITLE:
                out.append("SUBTITLE");
                break;
            default:
                out.append("UNKNOWN");
                break;
        }
        // out.append(", " + mFormat.toString());
        out.append("}");
        return out.toString();
    }
}

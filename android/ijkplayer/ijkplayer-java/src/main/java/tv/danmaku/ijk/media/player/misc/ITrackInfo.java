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

public interface ITrackInfo {
    int MEDIA_TRACK_TYPE_AUDIO = 2;
    int MEDIA_TRACK_TYPE_METADATA = 5;
    int MEDIA_TRACK_TYPE_SUBTITLE = 4;
    int MEDIA_TRACK_TYPE_TIMEDTEXT = 3;
    int MEDIA_TRACK_TYPE_UNKNOWN = 0;
    int MEDIA_TRACK_TYPE_VIDEO = 1;

    IMediaFormat getFormat();

    String getLanguage();

    int getTrackType();

    String getInfoInline();
}

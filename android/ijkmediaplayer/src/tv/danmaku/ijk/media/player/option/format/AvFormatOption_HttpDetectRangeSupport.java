/*
 * Copyright (C) 2013-2014 Zhang Rui <bbcallen@gmail.com>
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

package tv.danmaku.ijk.media.player.option.format;

import tv.danmaku.ijk.media.player.option.AvFormatOption;

// some video servers do not accept "Range: bytes=0-"
public final class AvFormatOption_HttpDetectRangeSupport implements
        AvFormatOption {
    public static AvFormatOption_HttpDetectRangeSupport Enable = new AvFormatOption_HttpDetectRangeSupport(
            "1");
    public static AvFormatOption_HttpDetectRangeSupport Disable = new AvFormatOption_HttpDetectRangeSupport(
            "0");
    private final String mValue;

    public AvFormatOption_HttpDetectRangeSupport(String value) {
        mValue = value;
    }

    public String getName() {
        return "http-detect-range-support";
    }

    public String getValue() {
        return mValue;
    }
}

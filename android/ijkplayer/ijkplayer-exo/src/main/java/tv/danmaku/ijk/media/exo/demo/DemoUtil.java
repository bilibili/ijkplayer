/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package tv.danmaku.ijk.media.exo.demo;

import android.text.TextUtils;
import com.google.android.exoplayer2.Format;
import com.google.android.exoplayer2.util.MimeTypes;
import java.util.Locale;

/**
 * Utility methods for demo application.
 */
/*package*/ final class DemoUtil {

  /**
   * Builds a track name for display.
   *
   * @param format {link Format} of the track.
   * @return a generated name specific to the track.
   */
  public static String buildTrackName(Format format) {
    String trackName;
    if (MimeTypes.isVideo(format.sampleMimeType)) {
      trackName = joinWithSeparator(joinWithSeparator(joinWithSeparator(
          buildResolutionString(format), buildBitrateString(format)), buildTrackIdString(format)),
          buildSampleMimeTypeString(format));
    } else if (MimeTypes.isAudio(format.sampleMimeType)) {
      trackName = joinWithSeparator(joinWithSeparator(joinWithSeparator(joinWithSeparator(
          buildLanguageString(format), buildAudioPropertyString(format)),
          buildBitrateString(format)), buildTrackIdString(format)),
          buildSampleMimeTypeString(format));
    } else {
      trackName = joinWithSeparator(joinWithSeparator(joinWithSeparator(buildLanguageString(format),
          buildBitrateString(format)), buildTrackIdString(format)),
          buildSampleMimeTypeString(format));
    }
    return trackName.length() == 0 ? "unknown" : trackName;
  }

  private static String buildResolutionString(Format format) {
    return format.width == Format.NO_VALUE || format.height == Format.NO_VALUE
        ? "" : format.width + "x" + format.height;
  }

  private static String buildAudioPropertyString(Format format) {
    return format.channelCount == Format.NO_VALUE || format.sampleRate == Format.NO_VALUE
        ? "" : format.channelCount + "ch, " + format.sampleRate + "Hz";
  }

  private static String buildLanguageString(Format format) {
    return TextUtils.isEmpty(format.language) || "und".equals(format.language) ? ""
        : format.language;
  }

  private static String buildBitrateString(Format format) {
    return format.bitrate == Format.NO_VALUE ? ""
        : String.format(Locale.US, "%.2fMbit", format.bitrate / 1000000f);
  }

  private static String joinWithSeparator(String first, String second) {
    return first.length() == 0 ? second : (second.length() == 0 ? first : first + ", " + second);
  }

  private static String buildTrackIdString(Format format) {
    return format.id == null ? "" : ("id:" + format.id);
  }

  private static String buildSampleMimeTypeString(Format format) {
    return format.sampleMimeType == null ? "" : format.sampleMimeType;
  }

  private DemoUtil() {}
}

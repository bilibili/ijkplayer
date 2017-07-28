/*
 * Copyright (C) 2016 The Android Open Source Project
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
package tv.danmaku.ijk.media.exo.demo;

import android.os.SystemClock;
import android.util.Log;
import android.view.Surface;
import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.ExoPlayer;
import com.google.android.exoplayer2.Format;
import com.google.android.exoplayer2.PlaybackParameters;
import com.google.android.exoplayer2.RendererCapabilities;
import com.google.android.exoplayer2.Timeline;
import com.google.android.exoplayer2.audio.AudioRendererEventListener;
import com.google.android.exoplayer2.decoder.DecoderCounters;
import com.google.android.exoplayer2.drm.DefaultDrmSessionManager;
import com.google.android.exoplayer2.metadata.Metadata;
import com.google.android.exoplayer2.metadata.MetadataRenderer;
import com.google.android.exoplayer2.metadata.emsg.EventMessage;
import com.google.android.exoplayer2.metadata.id3.ApicFrame;
import com.google.android.exoplayer2.metadata.id3.CommentFrame;
import com.google.android.exoplayer2.metadata.id3.GeobFrame;
import com.google.android.exoplayer2.metadata.id3.Id3Frame;
import com.google.android.exoplayer2.metadata.id3.PrivFrame;
import com.google.android.exoplayer2.metadata.id3.TextInformationFrame;
import com.google.android.exoplayer2.metadata.id3.UrlLinkFrame;
import com.google.android.exoplayer2.source.AdaptiveMediaSourceEventListener;
import com.google.android.exoplayer2.source.ExtractorMediaSource;
import com.google.android.exoplayer2.source.TrackGroup;
import com.google.android.exoplayer2.source.TrackGroupArray;
import com.google.android.exoplayer2.trackselection.MappingTrackSelector;
import com.google.android.exoplayer2.trackselection.MappingTrackSelector.MappedTrackInfo;
import com.google.android.exoplayer2.trackselection.TrackSelection;
import com.google.android.exoplayer2.trackselection.TrackSelectionArray;
import com.google.android.exoplayer2.upstream.DataSpec;
import com.google.android.exoplayer2.video.VideoRendererEventListener;
import java.io.IOException;
import java.text.NumberFormat;
import java.util.Locale;

/**
 * Logs player events using {link Log}.
 */
public class EventLogger implements ExoPlayer.EventListener,
    AudioRendererEventListener, VideoRendererEventListener, AdaptiveMediaSourceEventListener,
    ExtractorMediaSource.EventListener, DefaultDrmSessionManager.EventListener,
    MetadataRenderer.Output {

  private static final String TAG = "EventLogger";
  private static final int MAX_TIMELINE_ITEM_LINES = 3;
  private static final NumberFormat TIME_FORMAT;
  static {
    TIME_FORMAT = NumberFormat.getInstance(Locale.US);
    TIME_FORMAT.setMinimumFractionDigits(2);
    TIME_FORMAT.setMaximumFractionDigits(2);
    TIME_FORMAT.setGroupingUsed(false);
  }

  private final MappingTrackSelector trackSelector;
  private final Timeline.Window window;
  private final Timeline.Period period;
  private final long startTimeMs;

  public EventLogger(MappingTrackSelector trackSelector) {
    this.trackSelector = trackSelector;
    window = new Timeline.Window();
    period = new Timeline.Period();
    startTimeMs = SystemClock.elapsedRealtime();
  }

  // ExoPlayer.EventListener

  @Override
  public void onLoadingChanged(boolean isLoading) {
    Log.d(TAG, "loading [" + isLoading + "]");
  }

  @Override
  public void onPlayerStateChanged(boolean playWhenReady, int state) {
    Log.d(TAG, "state [" + getSessionTimeString() + ", " + playWhenReady + ", "
        + getStateString(state) + "]");
  }

  @Override
  public void onPositionDiscontinuity() {
    Log.d(TAG, "positionDiscontinuity");
  }

  @Override
  public void onPlaybackParametersChanged(PlaybackParameters playbackParameters) {
    Log.d(TAG, "playbackParameters " + String.format(
        "[speed=%.2f, pitch=%.2f]", playbackParameters.speed, playbackParameters.pitch));
  }

  @Override
  public void onTimelineChanged(Timeline timeline, Object manifest) {
    int periodCount = timeline.getPeriodCount();
    int windowCount = timeline.getWindowCount();
    Log.d(TAG, "sourceInfo [periodCount=" + periodCount + ", windowCount=" + windowCount);
    for (int i = 0; i < Math.min(periodCount, MAX_TIMELINE_ITEM_LINES); i++) {
      timeline.getPeriod(i, period);
      Log.d(TAG, "  " +  "period [" + getTimeString(period.getDurationMs()) + "]");
    }
    if (periodCount > MAX_TIMELINE_ITEM_LINES) {
      Log.d(TAG, "  ...");
    }
    for (int i = 0; i < Math.min(windowCount, MAX_TIMELINE_ITEM_LINES); i++) {
      timeline.getWindow(i, window);
      Log.d(TAG, "  " +  "window [" + getTimeString(window.getDurationMs()) + ", "
          + window.isSeekable + ", " + window.isDynamic + "]");
    }
    if (windowCount > MAX_TIMELINE_ITEM_LINES) {
      Log.d(TAG, "  ...");
    }
    Log.d(TAG, "]");
  }

  @Override
  public void onPlayerError(ExoPlaybackException e) {
    Log.e(TAG, "playerFailed [" + getSessionTimeString() + "]", e);
  }

  @Override
  public void onTracksChanged(TrackGroupArray ignored, TrackSelectionArray trackSelections) {
    MappedTrackInfo mappedTrackInfo = trackSelector.getCurrentMappedTrackInfo();
    if (mappedTrackInfo == null) {
      Log.d(TAG, "Tracks []");
      return;
    }
    Log.d(TAG, "Tracks [");
    // Log tracks associated to renderers.
    for (int rendererIndex = 0; rendererIndex < mappedTrackInfo.length; rendererIndex++) {
      TrackGroupArray rendererTrackGroups = mappedTrackInfo.getTrackGroups(rendererIndex);
      TrackSelection trackSelection = trackSelections.get(rendererIndex);
      if (rendererTrackGroups.length > 0) {
        Log.d(TAG, "  Renderer:" + rendererIndex + " [");
        for (int groupIndex = 0; groupIndex < rendererTrackGroups.length; groupIndex++) {
          TrackGroup trackGroup = rendererTrackGroups.get(groupIndex);
          String adaptiveSupport = getAdaptiveSupportString(trackGroup.length,
              mappedTrackInfo.getAdaptiveSupport(rendererIndex, groupIndex, false));
          Log.d(TAG, "    Group:" + groupIndex + ", adaptive_supported=" + adaptiveSupport + " [");
          for (int trackIndex = 0; trackIndex < trackGroup.length; trackIndex++) {
            String status = getTrackStatusString(trackSelection, trackGroup, trackIndex);
            String formatSupport = getFormatSupportString(
                mappedTrackInfo.getTrackFormatSupport(rendererIndex, groupIndex, trackIndex));
            Log.d(TAG, "      " + status + " Track:" + trackIndex + ", "
                + Format.toLogString(trackGroup.getFormat(trackIndex))
                + ", supported=" + formatSupport);
          }
          Log.d(TAG, "    ]");
        }
        // Log metadata for at most one of the tracks selected for the renderer.
        if (trackSelection != null) {
          for (int selectionIndex = 0; selectionIndex < trackSelection.length(); selectionIndex++) {
            Metadata metadata = trackSelection.getFormat(selectionIndex).metadata;
            if (metadata != null) {
              Log.d(TAG, "    Metadata [");
              printMetadata(metadata, "      ");
              Log.d(TAG, "    ]");
              break;
            }
          }
        }
        Log.d(TAG, "  ]");
      }
    }
    // Log tracks not associated with a renderer.
    TrackGroupArray unassociatedTrackGroups = mappedTrackInfo.getUnassociatedTrackGroups();
    if (unassociatedTrackGroups.length > 0) {
      Log.d(TAG, "  Renderer:None [");
      for (int groupIndex = 0; groupIndex < unassociatedTrackGroups.length; groupIndex++) {
        Log.d(TAG, "    Group:" + groupIndex + " [");
        TrackGroup trackGroup = unassociatedTrackGroups.get(groupIndex);
        for (int trackIndex = 0; trackIndex < trackGroup.length; trackIndex++) {
          String status = getTrackStatusString(false);
          String formatSupport = getFormatSupportString(
              RendererCapabilities.FORMAT_UNSUPPORTED_TYPE);
          Log.d(TAG, "      " + status + " Track:" + trackIndex + ", "
              + Format.toLogString(trackGroup.getFormat(trackIndex))
              + ", supported=" + formatSupport);
        }
        Log.d(TAG, "    ]");
      }
      Log.d(TAG, "  ]");
    }
    Log.d(TAG, "]");
  }

  // MetadataRenderer.Output

  @Override
  public void onMetadata(Metadata metadata) {
    Log.d(TAG, "onMetadata [");
    printMetadata(metadata, "  ");
    Log.d(TAG, "]");
  }

  // AudioRendererEventListener

  @Override
  public void onAudioEnabled(DecoderCounters counters) {
    Log.d(TAG, "audioEnabled [" + getSessionTimeString() + "]");
  }

  @Override
  public void onAudioSessionId(int audioSessionId) {
    Log.d(TAG, "audioSessionId [" + audioSessionId + "]");
  }

  @Override
  public void onAudioDecoderInitialized(String decoderName, long elapsedRealtimeMs,
      long initializationDurationMs) {
    Log.d(TAG, "audioDecoderInitialized [" + getSessionTimeString() + ", " + decoderName + "]");
  }

  @Override
  public void onAudioInputFormatChanged(Format format) {
    Log.d(TAG, "audioFormatChanged [" + getSessionTimeString() + ", " + Format.toLogString(format)
        + "]");
  }

  @Override
  public void onAudioDisabled(DecoderCounters counters) {
    Log.d(TAG, "audioDisabled [" + getSessionTimeString() + "]");
  }

  @Override
  public void onAudioTrackUnderrun(int bufferSize, long bufferSizeMs, long elapsedSinceLastFeedMs) {
    printInternalError("audioTrackUnderrun [" + bufferSize + ", " + bufferSizeMs + ", "
        + elapsedSinceLastFeedMs + "]", null);
  }

  // VideoRendererEventListener

  @Override
  public void onVideoEnabled(DecoderCounters counters) {
    Log.d(TAG, "videoEnabled [" + getSessionTimeString() + "]");
  }

  @Override
  public void onVideoDecoderInitialized(String decoderName, long elapsedRealtimeMs,
      long initializationDurationMs) {
    Log.d(TAG, "videoDecoderInitialized [" + getSessionTimeString() + ", " + decoderName + "]");
  }

  @Override
  public void onVideoInputFormatChanged(Format format) {
    Log.d(TAG, "videoFormatChanged [" + getSessionTimeString() + ", " + Format.toLogString(format)
        + "]");
  }

  @Override
  public void onVideoDisabled(DecoderCounters counters) {
    Log.d(TAG, "videoDisabled [" + getSessionTimeString() + "]");
  }

  @Override
  public void onDroppedFrames(int count, long elapsed) {
    Log.d(TAG, "droppedFrames [" + getSessionTimeString() + ", " + count + "]");
  }

  @Override
  public void onVideoSizeChanged(int width, int height, int unappliedRotationDegrees,
      float pixelWidthHeightRatio) {
    // Do nothing.
  }

  @Override
  public void onRenderedFirstFrame(Surface surface) {
    Log.d(TAG, "renderedFirstFrame [" + surface + "]");
  }

  // DefaultDrmSessionManager.EventListener

  @Override
  public void onDrmSessionManagerError(Exception e) {
    printInternalError("drmSessionManagerError", e);
  }

  @Override
  public void onDrmKeysRestored() {
    Log.d(TAG, "drmKeysRestored [" + getSessionTimeString() + "]");
  }

  @Override
  public void onDrmKeysRemoved() {
    Log.d(TAG, "drmKeysRemoved [" + getSessionTimeString() + "]");
  }

  @Override
  public void onDrmKeysLoaded() {
    Log.d(TAG, "drmKeysLoaded [" + getSessionTimeString() + "]");
  }

  // ExtractorMediaSource.EventListener

  @Override
  public void onLoadError(IOException error) {
    printInternalError("loadError", error);
  }

  // AdaptiveMediaSourceEventListener

  private long mBytesLoaded = 0;
  private long mBytesLoadedSeconds = 0;
  private long mLastBytesLoadedTime = 0;

  @Override
  public void onLoadStarted(DataSpec dataSpec, int dataType, int trackType, Format trackFormat,
      int trackSelectionReason, Object trackSelectionData, long mediaStartTimeMs,
      long mediaEndTimeMs, long elapsedRealtimeMs) {
    if(mLastBytesLoadedTime == 0) mLastBytesLoadedTime = System.currentTimeMillis();
  }

  @Override
  public void onLoadError(DataSpec dataSpec, int dataType, int trackType, Format trackFormat,
      int trackSelectionReason, Object trackSelectionData, long mediaStartTimeMs,
      long mediaEndTimeMs, long elapsedRealtimeMs, long loadDurationMs, long bytesLoaded,
      IOException error, boolean wasCanceled) {
    printInternalError("loadError", error);
  }

  @Override
  public void onLoadCanceled(DataSpec dataSpec, int dataType, int trackType, Format trackFormat,
      int trackSelectionReason, Object trackSelectionData, long mediaStartTimeMs,
      long mediaEndTimeMs, long elapsedRealtimeMs, long loadDurationMs, long bytesLoaded) {
    // Do nothing.
  }

  @Override
  public void onLoadCompleted(DataSpec dataSpec, int dataType, int trackType, Format trackFormat,
      int trackSelectionReason, Object trackSelectionData, long mediaStartTimeMs,
      long mediaEndTimeMs, long elapsedRealtimeMs, long loadDurationMs, long bytesLoaded) {
    long now = System.currentTimeMillis();
      if (mLastBytesLoadedTime == 0) {
          return;
      }
    float diffInSeconds = (now - mLastBytesLoadedTime) / 1000;
    logBytesLoadedInSeconds(bytesLoaded, diffInSeconds); // helper function, explain bellow
    mLastBytesLoadedTime = now;
  }

  /**
   * Logs an amount of bytes loaded in an amount of seconds
   *
   * @param bytes amount of bytes loaded
   * @param seconds time in seconds that it took to load those bytes
   */
  private void logBytesLoadedInSeconds(long bytes, float seconds){
    mBytesLoaded += bytes;
    mBytesLoadedSeconds += seconds;
  }

  public int getObservedBitrate() {
    if(mBytesLoadedSeconds != 0) {
      double bytesPerSecond = mBytesLoaded / mBytesLoadedSeconds;
      double bitsPerSecond = bytesPerSecond * 8; // (8 bits in a byte)
      Log.d(TAG," mBytesLoaded " + mBytesLoaded + " in "+mBytesLoadedSeconds+" seconds ("+(int)bitsPerSecond+" b/s indicated ");
      return (int)bitsPerSecond;
    }
    return 0;
  }

  @Override
  public void onUpstreamDiscarded(int trackType, long mediaStartTimeMs, long mediaEndTimeMs) {
    // Do nothing.
  }

  @Override
  public void onDownstreamFormatChanged(int trackType, Format trackFormat, int trackSelectionReason,
      Object trackSelectionData, long mediaTimeMs) {
    // Do nothing.
  }

  // Internal methods

  private void printInternalError(String type, Exception e) {
    Log.e(TAG, "internalError [" + getSessionTimeString() + ", " + type + "]", e);
  }

  private void printMetadata(Metadata metadata, String prefix) {
    for (int i = 0; i < metadata.length(); i++) {
      Metadata.Entry entry = metadata.get(i);
      if (entry instanceof TextInformationFrame) {
        TextInformationFrame textInformationFrame = (TextInformationFrame) entry;
        Log.d(TAG, prefix + String.format("%s: value=%s", textInformationFrame.id,
            textInformationFrame.value));
      } else if (entry instanceof UrlLinkFrame) {
        UrlLinkFrame urlLinkFrame = (UrlLinkFrame) entry;
        Log.d(TAG, prefix + String.format("%s: url=%s", urlLinkFrame.id, urlLinkFrame.url));
      } else if (entry instanceof PrivFrame) {
        PrivFrame privFrame = (PrivFrame) entry;
        Log.d(TAG, prefix + String.format("%s: owner=%s", privFrame.id, privFrame.owner));
      } else if (entry instanceof GeobFrame) {
        GeobFrame geobFrame = (GeobFrame) entry;
        Log.d(TAG, prefix + String.format("%s: mimeType=%s, filename=%s, description=%s",
            geobFrame.id, geobFrame.mimeType, geobFrame.filename, geobFrame.description));
      } else if (entry instanceof ApicFrame) {
        ApicFrame apicFrame = (ApicFrame) entry;
        Log.d(TAG, prefix + String.format("%s: mimeType=%s, description=%s",
            apicFrame.id, apicFrame.mimeType, apicFrame.description));
      } else if (entry instanceof CommentFrame) {
        CommentFrame commentFrame = (CommentFrame) entry;
        Log.d(TAG, prefix + String.format("%s: language=%s, description=%s", commentFrame.id,
            commentFrame.language, commentFrame.description));
      } else if (entry instanceof Id3Frame) {
        Id3Frame id3Frame = (Id3Frame) entry;
        Log.d(TAG, prefix + String.format("%s", id3Frame.id));
      } else if (entry instanceof EventMessage) {
        EventMessage eventMessage = (EventMessage) entry;
        Log.d(TAG, prefix + String.format("EMSG: scheme=%s, id=%d, value=%s",
            eventMessage.schemeIdUri, eventMessage.id, eventMessage.value));
      }
    }
  }

  private String getSessionTimeString() {
    return getTimeString(SystemClock.elapsedRealtime() - startTimeMs);
  }

  private static String getTimeString(long timeMs) {
    return timeMs == C.TIME_UNSET ? "?" : TIME_FORMAT.format((timeMs) / 1000f);
  }

  private static String getStateString(int state) {
    switch (state) {
      case ExoPlayer.STATE_BUFFERING:
        return "B";
      case ExoPlayer.STATE_ENDED:
        return "E";
      case ExoPlayer.STATE_IDLE:
        return "I";
      case ExoPlayer.STATE_READY:
        return "R";
      default:
        return "?";
    }
  }

  private static String getFormatSupportString(int formatSupport) {
    switch (formatSupport) {
      case RendererCapabilities.FORMAT_HANDLED:
        return "YES";
      case RendererCapabilities.FORMAT_EXCEEDS_CAPABILITIES:
        return "NO_EXCEEDS_CAPABILITIES";
      case RendererCapabilities.FORMAT_UNSUPPORTED_SUBTYPE:
        return "NO_UNSUPPORTED_TYPE";
      case RendererCapabilities.FORMAT_UNSUPPORTED_TYPE:
        return "NO";
      default:
        return "?";
    }
  }

  private static String getAdaptiveSupportString(int trackCount, int adaptiveSupport) {
    if (trackCount < 2) {
      return "N/A";
    }
    switch (adaptiveSupport) {
      case RendererCapabilities.ADAPTIVE_SEAMLESS:
        return "YES";
      case RendererCapabilities.ADAPTIVE_NOT_SEAMLESS:
        return "YES_NOT_SEAMLESS";
      case RendererCapabilities.ADAPTIVE_NOT_SUPPORTED:
        return "NO";
      default:
        return "?";
    }
  }

  private static String getTrackStatusString(TrackSelection selection, TrackGroup group,
      int trackIndex) {
    return getTrackStatusString(selection != null && selection.getTrackGroup() == group
        && selection.indexOf(trackIndex) != C.INDEX_UNSET);
  }

  private static String getTrackStatusString(boolean enabled) {
    return enabled ? "[X]" : "[ ]";
  }

}

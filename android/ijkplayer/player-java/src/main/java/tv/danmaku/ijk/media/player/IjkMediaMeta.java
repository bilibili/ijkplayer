package tv.danmaku.ijk.media.player;

import java.util.ArrayList;
import java.util.Locale;

import android.os.Bundle;
import android.text.TextUtils;

public class IjkMediaMeta {
    // media meta
    public static final String IJKM_KEY_FORMAT = "format";
    public static final String IJKM_KEY_DURATION_US = "duration_us";
    public static final String IJKM_KEY_START_US = "start_us";
    public static final String IJKM_KEY_BITRATE = "bitrate";
    public static final String IJKM_KEY_VIDEO_STREAM = "video";
    public static final String IJKM_KEY_AUDIO_STREAM = "audio";

    // stream meta
    public static final String IJKM_KEY_TYPE = "type";
    public static final String IJKM_VAL_TYPE__VIDEO = "video";
    public static final String IJKM_VAL_TYPE__AUDIO = "audio";
    public static final String IJKM_VAL_TYPE__UNKNOWN = "unknown";

    public static final String IJKM_KEY_CODEC_NAME = "codec_name";
    public static final String IJKM_KEY_CODEC_PROFILE = "codec_profile";
    public static final String IJKM_KEY_CODEC_LONG_NAME = "codec_long_name";

    // stream: video
    public static final String IJKM_KEY_WIDTH = "width";
    public static final String IJKM_KEY_HEIGHT = "height";
    public static final String IJKM_KEY_FPS_NUM = "fps_num";
    public static final String IJKM_KEY_FPS_DEN = "fps_den";
    public static final String IJKM_KEY_TBR_NUM = "tbr_num";
    public static final String IJKM_KEY_TBR_DEN = "tbr_den";
    public static final String IJKM_KEY_SAR_NUM = "sar_num";
    public static final String IJKM_KEY_SAR_DEN = "sar_den";
    // stream: audio
    public static final String IJKM_KEY_SAMPLE_RATE = "sample_rate";
    public static final String IJKM_KEY_CHANNEL_LAYOUT = "channel_layout";

    public static final String IJKM_KEY_STREAMS = "streams";

    public static final long AV_CH_FRONT_LEFT = 0x00000001;
    public static final long AV_CH_FRONT_RIGHT = 0x00000002;
    public static final long AV_CH_FRONT_CENTER = 0x00000004;
    public static final long AV_CH_LOW_FREQUENCY = 0x00000008;
    public static final long AV_CH_BACK_LEFT = 0x00000010;
    public static final long AV_CH_BACK_RIGHT = 0x00000020;
    public static final long AV_CH_FRONT_LEFT_OF_CENTER = 0x00000040;
    public static final long AV_CH_FRONT_RIGHT_OF_CENTER = 0x00000080;
    public static final long AV_CH_BACK_CENTER = 0x00000100;
    public static final long AV_CH_SIDE_LEFT = 0x00000200;
    public static final long AV_CH_SIDE_RIGHT = 0x00000400;
    public static final long AV_CH_TOP_CENTER = 0x00000800;
    public static final long AV_CH_TOP_FRONT_LEFT = 0x00001000;
    public static final long AV_CH_TOP_FRONT_CENTER = 0x00002000;
    public static final long AV_CH_TOP_FRONT_RIGHT = 0x00004000;
    public static final long AV_CH_TOP_BACK_LEFT = 0x00008000;
    public static final long AV_CH_TOP_BACK_CENTER = 0x00010000;
    public static final long AV_CH_TOP_BACK_RIGHT = 0x00020000;
    public static final long AV_CH_STEREO_LEFT = 0x20000000;
    public static final long AV_CH_STEREO_RIGHT = 0x40000000;
    public static final long AV_CH_WIDE_LEFT = 0x0000000080000000L;
    public static final long AV_CH_WIDE_RIGHT = 0x0000000100000000L;
    public static final long AV_CH_SURROUND_DIRECT_LEFT = 0x0000000200000000L;
    public static final long AV_CH_SURROUND_DIRECT_RIGHT = 0x0000000400000000L;
    public static final long AV_CH_LOW_FREQUENCY_2 = 0x0000000800000000L;

    public static final long AV_CH_LAYOUT_MONO = (AV_CH_FRONT_CENTER);
    public static final long AV_CH_LAYOUT_STEREO = (AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT);
    public static final long AV_CH_LAYOUT_2POINT1 = (AV_CH_LAYOUT_STEREO | AV_CH_LOW_FREQUENCY);
    public static final long AV_CH_LAYOUT_2_1 = (AV_CH_LAYOUT_STEREO | AV_CH_BACK_CENTER);
    public static final long AV_CH_LAYOUT_SURROUND = (AV_CH_LAYOUT_STEREO | AV_CH_FRONT_CENTER);
    public static final long AV_CH_LAYOUT_3POINT1 = (AV_CH_LAYOUT_SURROUND | AV_CH_LOW_FREQUENCY);
    public static final long AV_CH_LAYOUT_4POINT0 = (AV_CH_LAYOUT_SURROUND | AV_CH_BACK_CENTER);
    public static final long AV_CH_LAYOUT_4POINT1 = (AV_CH_LAYOUT_4POINT0 | AV_CH_LOW_FREQUENCY);
    public static final long AV_CH_LAYOUT_2_2 = (AV_CH_LAYOUT_STEREO
            | AV_CH_SIDE_LEFT | AV_CH_SIDE_RIGHT);
    public static final long AV_CH_LAYOUT_QUAD = (AV_CH_LAYOUT_STEREO
            | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT);
    public static final long AV_CH_LAYOUT_5POINT0 = (AV_CH_LAYOUT_SURROUND
            | AV_CH_SIDE_LEFT | AV_CH_SIDE_RIGHT);
    public static final long AV_CH_LAYOUT_5POINT1 = (AV_CH_LAYOUT_5POINT0 | AV_CH_LOW_FREQUENCY);
    public static final long AV_CH_LAYOUT_5POINT0_BACK = (AV_CH_LAYOUT_SURROUND
            | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT);
    public static final long AV_CH_LAYOUT_5POINT1_BACK = (AV_CH_LAYOUT_5POINT0_BACK | AV_CH_LOW_FREQUENCY);
    public static final long AV_CH_LAYOUT_6POINT0 = (AV_CH_LAYOUT_5POINT0 | AV_CH_BACK_CENTER);
    public static final long AV_CH_LAYOUT_6POINT0_FRONT = (AV_CH_LAYOUT_2_2
            | AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER);
    public static final long AV_CH_LAYOUT_HEXAGONAL = (AV_CH_LAYOUT_5POINT0_BACK | AV_CH_BACK_CENTER);
    public static final long AV_CH_LAYOUT_6POINT1 = (AV_CH_LAYOUT_5POINT1 | AV_CH_BACK_CENTER);
    public static final long AV_CH_LAYOUT_6POINT1_BACK = (AV_CH_LAYOUT_5POINT1_BACK | AV_CH_BACK_CENTER);
    public static final long AV_CH_LAYOUT_6POINT1_FRONT = (AV_CH_LAYOUT_6POINT0_FRONT | AV_CH_LOW_FREQUENCY);
    public static final long AV_CH_LAYOUT_7POINT0 = (AV_CH_LAYOUT_5POINT0
            | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT);
    public static final long AV_CH_LAYOUT_7POINT0_FRONT = (AV_CH_LAYOUT_5POINT0
            | AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER);
    public static final long AV_CH_LAYOUT_7POINT1 = (AV_CH_LAYOUT_5POINT1
            | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT);
    public static final long AV_CH_LAYOUT_7POINT1_WIDE = (AV_CH_LAYOUT_5POINT1
            | AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER);
    public static final long AV_CH_LAYOUT_7POINT1_WIDE_BACK = (AV_CH_LAYOUT_5POINT1_BACK
            | AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER);
    public static final long AV_CH_LAYOUT_OCTAGONAL = (AV_CH_LAYOUT_5POINT0
            | AV_CH_BACK_LEFT | AV_CH_BACK_CENTER | AV_CH_BACK_RIGHT);
    public static final long AV_CH_LAYOUT_STEREO_DOWNMIX = (AV_CH_STEREO_LEFT | AV_CH_STEREO_RIGHT);

    public Bundle mMediaMeta;

    public String mFormat;
    public long mDurationUS;
    public long mStartUS;
    public long mBitrate;

    public ArrayList<IjkStreamMeta> mStreams;
    public IjkStreamMeta mVideoStream;
    public IjkStreamMeta mAudioStream;

    public String getString(String key) {
        return mMediaMeta.getString(key);
    }

    public int getInt(String key) {
        return getInt(key, 0);
    }

    public int getInt(String key, int defaultValue) {
        String value = getString(key);
        if (TextUtils.isEmpty(value))
            return defaultValue;

        try {
            return Integer.parseInt(value);
        } catch (NumberFormatException e) {
            return defaultValue;
        }
    }

    public long getLong(String key) {
        return getLong(key, 0);
    }

    public long getLong(String key, long defaultValue) {
        String value = getString(key);
        if (TextUtils.isEmpty(value))
            return defaultValue;

        try {
            return Long.parseLong(value);
        } catch (NumberFormatException e) {
            return defaultValue;
        }
    }

    public ArrayList<Bundle> getParcelableArrayList(String key) {
        return mMediaMeta.getParcelableArrayList(key);
    }

    public String getDurationInline() {
        long duration = mDurationUS + 5000;
        long secs = duration / 1000000;
        long mins = secs / 60;
        secs %= 60;
        long hours = mins / 60;
        mins %= 60;
        return String.format(Locale.US, "%02d:%02d:%02d", hours, mins, secs);
    }

    public static IjkMediaMeta parse(Bundle mediaMeta) {
        if (mediaMeta == null)
            return null;

        IjkMediaMeta meta = new IjkMediaMeta();
        meta.mMediaMeta = mediaMeta;

        meta.mFormat = meta.getString(IJKM_KEY_FORMAT);
        meta.mDurationUS = meta.getLong(IJKM_KEY_DURATION_US);
        meta.mStartUS = meta.getLong(IJKM_KEY_START_US);
        meta.mBitrate = meta.getLong(IJKM_KEY_BITRATE);

        int videoStreamIndex = meta.getInt(IJKM_KEY_VIDEO_STREAM, -1);
        int audioStreamIndex = meta.getInt(IJKM_KEY_AUDIO_STREAM, -1);

        ArrayList<Bundle> streams = meta
                .getParcelableArrayList(IJKM_KEY_STREAMS);
        if (streams == null)
            return meta;

        int index = -1;
        for (Bundle streamBundle : streams) {
            index++;

            if (streamBundle == null) {
                continue;
            }

            IjkStreamMeta streamMeta = new IjkStreamMeta(index);
            streamMeta.mMeta = streamBundle;
            streamMeta.mType = streamMeta.getString(IJKM_KEY_TYPE);
            if (TextUtils.isEmpty(streamMeta.mType))
                continue;

            streamMeta.mCodecName = streamMeta.getString(IJKM_KEY_CODEC_NAME);
            streamMeta.mCodecProfile = streamMeta
                    .getString(IJKM_KEY_CODEC_PROFILE);
            streamMeta.mCodecLongName = streamMeta
                    .getString(IJKM_KEY_CODEC_LONG_NAME);
            streamMeta.mBitrate = streamMeta.getInt(IJKM_KEY_BITRATE);

            if (streamMeta.mType.equalsIgnoreCase(IJKM_VAL_TYPE__VIDEO)) {
                streamMeta.mWidth = streamMeta.getInt(IJKM_KEY_WIDTH);
                streamMeta.mHeight = streamMeta.getInt(IJKM_KEY_HEIGHT);
                streamMeta.mFpsNum = streamMeta.getInt(IJKM_KEY_FPS_NUM);
                streamMeta.mFpsDen = streamMeta.getInt(IJKM_KEY_FPS_DEN);
                streamMeta.mTbrNum = streamMeta.getInt(IJKM_KEY_TBR_NUM);
                streamMeta.mTbrDen = streamMeta.getInt(IJKM_KEY_TBR_DEN);
                streamMeta.mSarNum = streamMeta.getInt(IJKM_KEY_SAR_NUM);
                streamMeta.mSarDen = streamMeta.getInt(IJKM_KEY_SAR_DEN);

                if (videoStreamIndex == index) {
                    meta.mVideoStream = streamMeta;
                }
            } else if (streamMeta.mType.equalsIgnoreCase(IJKM_VAL_TYPE__AUDIO)) {
                streamMeta.mSampleRate = streamMeta
                        .getInt(IJKM_KEY_SAMPLE_RATE);
                streamMeta.mChannelLayout = streamMeta
                        .getLong(IJKM_KEY_CHANNEL_LAYOUT);

                if (audioStreamIndex == index) {
                    meta.mAudioStream = streamMeta;
                }
            }
        }

        return meta;
    }

    public static class IjkStreamMeta {
        public Bundle mMeta;

        public int mIndex;
        public String mType;

        // common
        public String mCodecName;
        public String mCodecProfile;
        public String mCodecLongName;
        public long mBitrate;

        // video
        public int mWidth;
        public int mHeight;
        public int mFpsNum;
        public int mFpsDen;
        public int mTbrNum;
        public int mTbrDen;
        public int mSarNum;
        public int mSarDen;

        // audio
        public int mSampleRate;
        public long mChannelLayout;

        public IjkStreamMeta(int index) {
            mIndex = index;
        }

        public String getString(String key) {
            return mMeta.getString(key);
        }

        public int getInt(String key) {
            return getInt(key, 0);
        }

        public int getInt(String key, int defaultValue) {
            String value = getString(key);
            if (TextUtils.isEmpty(value))
                return defaultValue;

            try {
                return Integer.parseInt(value);
            } catch (NumberFormatException e) {
                return defaultValue;
            }
        }

        public long getLong(String key) {
            return getLong(key, 0);
        }

        public long getLong(String key, long defaultValue) {
            String value = getString(key);
            if (TextUtils.isEmpty(value))
                return defaultValue;

            try {
                return Long.parseLong(value);
            } catch (NumberFormatException e) {
                return defaultValue;
            }
        }

        public String getCodecLongNameInline() {
            if (!TextUtils.isEmpty(mCodecLongName)) {
                return mCodecLongName;
            } else if (!TextUtils.isEmpty(mCodecName)) {
                return mCodecName;
            } else {
                return "N/A";
            }
        }

        public String getResolutionInline() {
            if (mWidth <= 0 || mHeight <= 0) {
                return "N/A";
            } else if (mSarNum <= 0 || mSarDen <= 0) {
                return String.format(Locale.US, "%d x %d", mWidth, mHeight);
            } else {
                return String.format(Locale.US, "%d x %d [SAR %d:%d]", mWidth,
                        mHeight, mSarNum, mSarDen);
            }
        }

        public String getFpsInline() {
            if (mFpsNum <= 0 || mFpsDen <= 0) {
                return "N/A";
            } else {
                return String.valueOf(((float) (mFpsNum)) / mFpsDen);
            }
        }

        public String getBitrateInline() {
            if (mBitrate <= 0) {
                return "N/A";
            } else if (mBitrate < 1000) {
                return String.format(Locale.US, "%d bit/s", mBitrate);
            } else {
                return String.format(Locale.US, "%d kb/s", mBitrate / 1000);
            }
        }

        public String getSampleRateInline() {
            if (mSampleRate <= 0) {
                return "N/A";
            } else {
                return String.format(Locale.US, "%d Hz", mSampleRate);
            }
        }

        public String getChannelLayoutInline() {
            if (mChannelLayout <= 0) {
                return "N/A";
            } else {
                if (mChannelLayout == AV_CH_LAYOUT_MONO) {
                    return "mono";
                } else if (mChannelLayout == AV_CH_LAYOUT_STEREO) {
                    return "stereo";
                } else {
                    return String.format(Locale.US, "%x", mChannelLayout);
                }
            }
        }
    }
}

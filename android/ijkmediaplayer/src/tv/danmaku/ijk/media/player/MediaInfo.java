package tv.danmaku.ijk.media.player;

import android.text.TextUtils;

public class MediaInfo {
    public String mVideoDecoder;
    public String mVideoDecoderImpl;

    public String mAudioDecoder;
    public String mAudioDecoderImpl;

    public final String getVideoDecoderInline(String defaultValue) {
        if (TextUtils.isEmpty(mVideoDecoder))
            return defaultValue;

        if (mVideoDecoder.equalsIgnoreCase("mediacodec")) {
            mVideoDecoder = "MediaCodec";
            mVideoDecoderImpl = "V2-HW+";
        }

        StringBuilder sb = new StringBuilder(mVideoDecoder);
        sb.append(": ");
        if (TextUtils.isEmpty(mVideoDecoderImpl)) {
            sb.append("SW");
        } else {
            sb.append(mVideoDecoderImpl);
        }

        return sb.toString();
    }

    public final String getAudioDecoderInline(String defaultValue) {
        if (TextUtils.isEmpty(mAudioDecoder))
            return defaultValue;

        if (mVideoDecoder.equalsIgnoreCase("mediacodec")) {
            mVideoDecoder = "MediaCodec";
            mVideoDecoderImpl = "V2-HW+";
        }

        StringBuilder sb = new StringBuilder(mAudioDecoder);
        sb.append(": ");
        if (TextUtils.isEmpty(mAudioDecoderImpl)) {
            sb.append("SW");
        } else {
            sb.append(mAudioDecoderImpl);
        }

        return sb.toString();
    }
}

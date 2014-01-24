package tv.danmaku.ijk.media.player.misc;

import android.os.Bundle;
import android.text.TextUtils;

public class MediaInfo {
    private static final String BUNDLE_VIDEO_DECODER = "video_decoder";
    private static final String BUNDLE_VIDEO_DECODER_IMPL = "video_decoder_impl";

    private static final String BUNDLE_AUDIO_DECODER = "audio_decoder";
    private static final String BUNDLE_AUDIO_DECODER_IMPL = "audio_decoder_impl";

    public String mVideoDecoder;
    public String mVideoDecoderImpl;

    public String mAudioDecoder;
    public String mAudioDecoderImpl;

    public static MediaInfo sAndroidMediaInfo;
    public static MediaInfo sAndroidListMediaInfo;

    public static MediaInfo parseModuleInfo(Bundle args) {
        MediaInfo moduleInfo = new MediaInfo();

        moduleInfo.mVideoDecoder = args.getString(BUNDLE_VIDEO_DECODER);
        moduleInfo.mVideoDecoderImpl = args
                .getString(BUNDLE_VIDEO_DECODER_IMPL);

        moduleInfo.mAudioDecoder = args.getString(BUNDLE_AUDIO_DECODER);
        moduleInfo.mAudioDecoderImpl = args
                .getString(BUNDLE_AUDIO_DECODER_IMPL);

        return moduleInfo;
    }

    public static MediaInfo getNullModuleInfo() {
        MediaInfo module = new MediaInfo();
        return module;
    }

    public final String getVideoDecoderInline() {
        if (TextUtils.isEmpty(mVideoDecoder))
            return "N/A";

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

    public final String getAudioDecoderInline() {
        if (TextUtils.isEmpty(mAudioDecoder))
            return "N/A";

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

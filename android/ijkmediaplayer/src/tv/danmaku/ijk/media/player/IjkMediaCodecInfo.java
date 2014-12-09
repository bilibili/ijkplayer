package tv.danmaku.ijk.media.player;

import java.util.Locale;

import android.media.MediaCodecInfo;
import android.media.MediaCodecInfo.CodecCapabilities;
import android.media.MediaCodecInfo.CodecProfileLevel;
import android.os.Build;
import android.text.TextUtils;
import android.util.Log;

public class IjkMediaCodecInfo {
    private final static String TAG = "IjkMediaCodecInfo";

    public static int RANK_MAX = 100;
    public static int RANK_TESTED = 70;
    public static int RANK_ACCETABLE = 60;
    public static int RANK_SOFTWARE = 20;
    public static int RANK_NON_STANDARD = 10;
    public static int RANK_NO_SENSE = 0;

    public MediaCodecInfo mCodecInfo;
    public int mRank = 0;

    public static IjkMediaCodecInfo setupCandidate(MediaCodecInfo codecInfo) {
        if (codecInfo == null)
            return null;

        String name = codecInfo.getName();
        if (TextUtils.isEmpty(name))
            return null;

        name = name.toLowerCase(Locale.US);
        int rank = RANK_NO_SENSE;
        if (!name.startsWith("omx.")) {
            rank = RANK_NON_STANDARD;
        } else if (name.startsWith("omx.pv")) {
            rank = RANK_SOFTWARE;
        } else if (name.startsWith("omx.google")) {
            rank = RANK_SOFTWARE;
        } else if (name.startsWith("omx.ffmpeg")) {
            rank = RANK_SOFTWARE;
        } else if (name.startsWith("omx.avcodec")) {
            rank = RANK_SOFTWARE;
        } else if (name.startsWith("omx.ittiam.")) {
            // unknown codec in qualcomm SoC
            rank = RANK_NO_SENSE;
        } else if (name.startsWith("omx.mtk.")) {
            // MTK only works on 4.3 and above
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR2)
                rank = RANK_NO_SENSE;
            else
                rank = RANK_ACCETABLE;
        } else {
            rank = RANK_ACCETABLE;
        }

        IjkMediaCodecInfo candidate = new IjkMediaCodecInfo();
        candidate.mCodecInfo = codecInfo;
        candidate.mRank = rank;
        return candidate;
    }

    public void dumpProfileLevels(String mimeType) {
        try {
            CodecCapabilities caps = mCodecInfo
                    .getCapabilitiesForType(mimeType);
            int maxProfile = 0;
            int maxLevel = 0;
            if (caps != null) {
                if (caps.profileLevels != null) {
                    for (CodecProfileLevel profileLevel : caps.profileLevels) {
                        if (profileLevel == null)
                            continue;

                        maxProfile = Math.max(maxProfile, profileLevel.profile);
                        maxLevel = Math.max(maxLevel, profileLevel.level);
                    }
                }
            }

            Log.i(TAG,
                    String.format(Locale.US, "%s",
                            getProfileLevelName(maxProfile, maxLevel)));
        } catch (Throwable e) {
            Log.i(TAG, "profile-level: exception");
        }
    }

    public static String getProfileLevelName(int profile, int level) {
        return String.format(Locale.US, " %s Profile Level %s (%d,%d)",
                getProfileName(profile), getLevelName(level), profile, level);
    }

    public static String getProfileName(int profile) {
        switch (profile) {
        case CodecProfileLevel.AVCProfileBaseline:
            return "Baseline";
        case CodecProfileLevel.AVCProfileMain:
            return "Main";
        case CodecProfileLevel.AVCProfileExtended:
            return "Extends";
        case CodecProfileLevel.AVCProfileHigh:
            return "High";
        case CodecProfileLevel.AVCProfileHigh10:
            return "High10";
        case CodecProfileLevel.AVCProfileHigh422:
            return "High422";
        case CodecProfileLevel.AVCProfileHigh444:
            return "High444";
        default:
            return "Unknown";
        }
    }

    public static String getLevelName(int level) {
        switch (level) {
        case CodecProfileLevel.AVCLevel1:
            return "1";
        case CodecProfileLevel.AVCLevel1b:
            return "1b";
        case CodecProfileLevel.AVCLevel11:
            return "11";
        case CodecProfileLevel.AVCLevel12:
            return "12";
        case CodecProfileLevel.AVCLevel13:
            return "13";
        case CodecProfileLevel.AVCLevel2:
            return "2";
        case CodecProfileLevel.AVCLevel21:
            return "21";
        case CodecProfileLevel.AVCLevel22:
            return "22";
        case CodecProfileLevel.AVCLevel3:
            return "3";
        case CodecProfileLevel.AVCLevel31:
            return "31";
        case CodecProfileLevel.AVCLevel32:
            return "32";
        case CodecProfileLevel.AVCLevel4:
            return "4";
        case CodecProfileLevel.AVCLevel41:
            return "41";
        case CodecProfileLevel.AVCLevel42:
            return "42";
        case CodecProfileLevel.AVCLevel5:
            return "5";
        case CodecProfileLevel.AVCLevel51:
            return "51";
        case 65536: // CodecProfileLevel.AVCLevel52:
            return "52";
        default:
            return "0";
        }
    }
}

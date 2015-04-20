package tv.danmaku.ijk.media.player;

import java.util.Locale;
import java.util.Map;
import java.util.TreeMap;

import android.annotation.TargetApi;
import android.media.MediaCodecInfo;
import android.media.MediaCodecInfo.CodecCapabilities;
import android.media.MediaCodecInfo.CodecProfileLevel;
import android.os.Build;
import android.text.TextUtils;
import android.util.Log;

public class IjkMediaCodecInfo {
    private final static String TAG = "IjkMediaCodecInfo";

    public static int RANK_MAX = 1000;
    public static int RANK_TESTED = 800;
    public static int RANK_ACCEPTABLE = 700;
    public static int RANK_LAST_CHANCE = 600;
    public static int RANK_SECURE = 300;
    public static int RANK_SOFTWARE = 200;
    public static int RANK_NON_STANDARD = 100;
    public static int RANK_NO_SENSE = 0;

    public MediaCodecInfo mCodecInfo;
    public int mRank = 0;
    public String mMimeType;

    private static Map<String, Integer> sKnownCodecList;

    private static synchronized Map<String, Integer> getKnownCodecList() {
        if (sKnownCodecList != null)
            return sKnownCodecList;

        sKnownCodecList = new TreeMap<String, Integer>(
                String.CASE_INSENSITIVE_ORDER);

        // ----- Nvidia -----
        // Tegra3
        // Nexus 7 (2012)
        // Tegra K1
        // Nexus 9
        sKnownCodecList.put("OMX.Nvidia.h264.decode", RANK_TESTED);
        sKnownCodecList.put("OMX.Nvidia.h264.decode.secure", RANK_SECURE);

        // ----- Intel -----
        // Atom Z3735
        // Teclast X98 Air
        sKnownCodecList.put("OMX.Intel.hw_vd.h264", RANK_TESTED + 1);
        // Atom Z2560
        // Dell Venue 7 3730
        sKnownCodecList.put("OMX.Intel.VideoDecoder.AVC", RANK_TESTED);

        // ----- Qualcomm -----
        // MSM8260
        // Xiaomi MI 1S
        sKnownCodecList.put("OMX.qcom.video.decoder.avc", RANK_TESTED);
        sKnownCodecList.put("OMX.ittiam.video.decoder.avc", RANK_NO_SENSE);

        // ----- Samsung -----
        // Exynos 3110
        // Nexus S
        sKnownCodecList.put("OMX.SEC.avc.dec", RANK_TESTED);
        sKnownCodecList.put("OMX.SEC.AVC.Decoder", RANK_TESTED - 1);
        // OMX.SEC.avcdec doesn't reorder output pictures on GT-9100
        sKnownCodecList.put("OMX.SEC.avcdec", RANK_TESTED - 2);
        sKnownCodecList.put("OMX.SEC.avc.sw.dec", RANK_SOFTWARE);
        // Exynos 5 ?
        sKnownCodecList.put("OMX.Exynos.avc.dec", RANK_TESTED);
        sKnownCodecList.put("OMX.Exynos.AVC.Decoder", RANK_TESTED - 1);

        // ------ Huawei hisilicon ------
        // Kirin 910, Mali 450 MP
        // Huawei HONOR 3C (H30-L01)
        sKnownCodecList.put("OMX.k3.video.decoder.avc", RANK_TESTED);
        // Kirin 920, Mali T624
        // Huawei HONOR 6
        sKnownCodecList.put("OMX.IMG.MSVDX.Decoder.AVC", RANK_TESTED);

        // ----- TI -----
        // TI OMAP4460
        // Galaxy Nexus
        sKnownCodecList.put("OMX.TI.DUCATI1.VIDEO.DECODER", RANK_TESTED);

        // ------ RockChip ------
        // Youku TVBox
        sKnownCodecList.put("OMX.rk.video_decoder.avc", RANK_TESTED);

        // ------ AMLogic -----
        // MiBox1, 1s, 2
        sKnownCodecList.put("OMX.amlogic.avc.decoder.awesome", RANK_TESTED);

        // ------ Marvell ------
        // Lenovo A788t
        sKnownCodecList.put("OMX.MARVELL.VIDEO.HW.CODA7542DECODER", RANK_TESTED);
        sKnownCodecList.put("OMX.MARVELL.VIDEO.H264DECODER", RANK_SOFTWARE);

        // ----- TODO: need test -----
        sKnownCodecList.remove("OMX.BRCM.vc4.decoder.avc");
        sKnownCodecList.remove("OMX.brcm.video.h264.hw.decoder");
        sKnownCodecList.remove("OMX.brcm.video.h264.decoder");
        sKnownCodecList.remove("OMX.ST.VFM.H264Dec");
        sKnownCodecList.remove("OMX.allwinner.video.decoder.avc");
        sKnownCodecList.remove("OMX.MS.AVC.Decoder");
        sKnownCodecList.remove("OMX.hantro.81x0.video.decoder");
        sKnownCodecList.remove("OMX.hisi.video.decoder");
        sKnownCodecList.remove("OMX.cosmo.video.decoder.avc");
        sKnownCodecList.remove("OMX.duos.h264.decoder");

        // Really ?
        sKnownCodecList.remove("OMX.bluestacks.hw.decoder");

        // ---------------
        // Useless codec
        // ----- google -----
        sKnownCodecList.put("OMX.google.h264.decoder", RANK_SOFTWARE);
        sKnownCodecList.put("OMX.google.h264.lc.decoder", RANK_SOFTWARE);
        // ----- huawei k920 -----
        sKnownCodecList.put("OMX.k3.ffmpeg.decoder", RANK_SOFTWARE);
        sKnownCodecList.put("OMX.ffmpeg.video.decoder", RANK_SOFTWARE);

        return sKnownCodecList;
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public static IjkMediaCodecInfo setupCandidate(MediaCodecInfo codecInfo,
            String mimeType) {
        if (codecInfo == null
                || Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN)
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
        } else if (name.startsWith("omx.google.")) {
            rank = RANK_SOFTWARE;
        } else if (name.startsWith("omx.ffmpeg.")) {
            rank = RANK_SOFTWARE;
        } else if (name.startsWith("omx.k3.ffmpeg.")) {
            rank = RANK_SOFTWARE;
        } else if (name.startsWith("omx.avcodec.")) {
            rank = RANK_SOFTWARE;
        } else if (name.startsWith("omx.ittiam.")) {
            // unknown codec in qualcomm SoC
            rank = RANK_NO_SENSE;
        } else if (name.startsWith("omx.mtk.")) {
            // 1. MTK only works on 4.3 and above
            // 2. MTK works on MIUI 6 (4.2.1)
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR2)
                rank = RANK_NO_SENSE;
            else
                rank = RANK_TESTED;
        } else {
            Integer knownRank = getKnownCodecList().get(name);
            if (knownRank != null) {
                rank = knownRank;
            } else {
                try {
                    CodecCapabilities cap = codecInfo
                            .getCapabilitiesForType(mimeType);
                    if (cap != null)
                        rank = RANK_ACCEPTABLE;
                    else
                        rank = RANK_LAST_CHANCE;
                } catch (Throwable e) {
                    rank = RANK_LAST_CHANCE;
                }
            }
        }

        IjkMediaCodecInfo candidate = new IjkMediaCodecInfo();
        candidate.mCodecInfo = codecInfo;
        candidate.mRank = rank;
        candidate.mMimeType = mimeType;
        return candidate;
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public void dumpProfileLevels(String mimeType) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN)
            return;

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

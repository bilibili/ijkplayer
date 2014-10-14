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

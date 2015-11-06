package tv.danmaku.ijk.media.sample.content;

/**
 * Created by shenchuan on 15-11-6.
 */
public class MediaBean {
    public static final String KEY_PATH = "path";
    public static final String KEY_FILE_NAME = "file_name";

    public String path;
    public String fileName;

    @Override
    public String toString() {
        return fileName;
    }
}

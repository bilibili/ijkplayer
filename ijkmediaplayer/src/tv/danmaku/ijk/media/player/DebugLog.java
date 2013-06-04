package tv.danmaku.ijk.media.player;

import java.util.Locale;
import android.util.Log;

public class DebugLog {
    public static int e(String tag, String msg) {
        if (Pragma.ENABLE_ERROR) {
            return Log.e(tag, msg);
        }

        return 0;
    }

    public static int e(String tag, String msg, Throwable tr) {
        if (Pragma.ENABLE_ERROR) {
            return Log.e(tag, msg, tr);
        }

        return 0;
    }

    public static int efmt(String tag, String fmt, Object... args) {
        if (Pragma.ENABLE_ERROR) {
            String msg = String.format(Locale.US, fmt, args);
            return Log.e(tag, msg);
        }

        return 0;
    }

    public static int i(String tag, String msg) {
        if (Pragma.ENABLE_INFO) {
            return Log.i(tag, msg);
        }

        return 0;
    }

    public static int i(String tag, String msg, Throwable tr) {
        if (Pragma.ENABLE_INFO) {
            return Log.i(tag, msg, tr);
        }

        return 0;
    }

    public static int ifmt(String tag, String fmt, Object... args) {
        if (Pragma.ENABLE_INFO) {
            String msg = String.format(Locale.US, fmt, args);
            return Log.i(tag, msg);
        }

        return 0;
    }

    public static int w(String tag, String msg) {
        if (Pragma.ENABLE_WARN) {
            return Log.w(tag, msg);
        }

        return 0;
    }

    public static int w(String tag, String msg, Throwable tr) {
        if (Pragma.ENABLE_WARN) {
            return Log.w(tag, msg, tr);
        }

        return 0;
    }

    public static int wfmt(String tag, String fmt, Object... args) {
        if (Pragma.ENABLE_WARN) {
            String msg = String.format(Locale.US, fmt, args);
            return Log.w(tag, msg);
        }

        return 0;
    }

    public static int d(String tag, String msg) {
        if (Pragma.ENABLE_DEBUG) {
            return Log.d(tag, msg);
        }

        return 0;
    }

    public static int d(String tag, String msg, Throwable tr) {
        if (Pragma.ENABLE_DEBUG) {
            return Log.d(tag, msg, tr);
        }

        return 0;
    }

    public static int dfmt(String tag, String fmt, Object... args) {
        if (Pragma.ENABLE_DEBUG) {
            String msg = String.format(Locale.US, fmt, args);
            return Log.d(tag, msg);
        }

        return 0;
    }

    public static int v(String tag, String msg) {
        if (Pragma.ENABLE_VERBOSE) {
            return Log.v(tag, msg);
        }

        return 0;
    }

    public static int v(String tag, String msg, Throwable tr) {
        if (Pragma.ENABLE_VERBOSE) {
            return Log.v(tag, msg, tr);
        }

        return 0;
    }

    public static int vfmt(String tag, String fmt, Object... args) {
        if (Pragma.ENABLE_VERBOSE) {
            String msg = String.format(Locale.US, fmt, args);
            return Log.v(tag, msg);
        }

        return 0;
    }

    public static void printStackTrace(Throwable e) {
        if (Pragma.ENABLE_WARN) {
            e.printStackTrace();
        }
    }

    public static void printCause(Throwable e) {
        if (Pragma.ENABLE_WARN) {
            Throwable cause = e.getCause();
            if (cause != null)
                e = cause;

            printStackTrace(e);
        }
    }
}

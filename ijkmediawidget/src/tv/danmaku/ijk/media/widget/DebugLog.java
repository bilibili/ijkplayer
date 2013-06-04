/*
 * Copyright (C) 2013 Zhang Rui <bbcallen@gmail.com>
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

package tv.danmaku.ijk.media.widget;

import java.util.Locale;
import android.util.Log;

public class DebugLog {
    public static boolean ENABLE_ERROR = false;
    public static boolean ENABLE_INFO = false;
    public static boolean ENABLE_WARN = false;
    public static boolean ENABLE_DEBUG = false;
    public static boolean ENABLE_VERBOSE = false;

    public static void setDebug(boolean debug) {
        ENABLE_ERROR = debug;
        ENABLE_INFO = debug;
        ENABLE_WARN = debug;
        ENABLE_DEBUG = debug;
        ENABLE_VERBOSE = debug;
    }

    public static int e(String tag, String msg) {
        if (ENABLE_ERROR) {
            return Log.e(tag, msg);
        }

        return 0;
    }

    public static int e(String tag, String msg, Throwable tr) {
        if (ENABLE_ERROR) {
            return Log.e(tag, msg, tr);
        }

        return 0;
    }

    public static int efmt(String tag, String fmt, Object... args) {
        if (ENABLE_ERROR) {
            String msg = String.format(Locale.US, fmt, args);
            return Log.e(tag, msg);
        }

        return 0;
    }

    public static int i(String tag, String msg) {
        if (ENABLE_INFO) {
            return Log.i(tag, msg);
        }

        return 0;
    }

    public static int i(String tag, String msg, Throwable tr) {
        if (ENABLE_INFO) {
            return Log.i(tag, msg, tr);
        }

        return 0;
    }

    public static int ifmt(String tag, String fmt, Object... args) {
        if (ENABLE_INFO) {
            String msg = String.format(Locale.US, fmt, args);
            return Log.i(tag, msg);
        }

        return 0;
    }

    public static int w(String tag, String msg) {
        if (ENABLE_WARN) {
            return Log.w(tag, msg);
        }

        return 0;
    }

    public static int w(String tag, String msg, Throwable tr) {
        if (ENABLE_WARN) {
            return Log.w(tag, msg, tr);
        }

        return 0;
    }

    public static int wfmt(String tag, String fmt, Object... args) {
        if (ENABLE_WARN) {
            String msg = String.format(Locale.US, fmt, args);
            return Log.w(tag, msg);
        }

        return 0;
    }

    public static int d(String tag, String msg) {
        if (ENABLE_DEBUG) {
            return Log.d(tag, msg);
        }

        return 0;
    }

    public static int d(String tag, String msg, Throwable tr) {
        if (ENABLE_DEBUG) {
            return Log.d(tag, msg, tr);
        }

        return 0;
    }

    public static int dfmt(String tag, String fmt, Object... args) {
        if (ENABLE_DEBUG) {
            String msg = String.format(Locale.US, fmt, args);
            return Log.d(tag, msg);
        }

        return 0;
    }

    public static int v(String tag, String msg) {
        if (ENABLE_VERBOSE) {
            return Log.v(tag, msg);
        }

        return 0;
    }

    public static int v(String tag, String msg, Throwable tr) {
        if (ENABLE_VERBOSE) {
            return Log.v(tag, msg, tr);
        }

        return 0;
    }

    public static int vfmt(String tag, String fmt, Object... args) {
        if (ENABLE_VERBOSE) {
            String msg = String.format(Locale.US, fmt, args);
            return Log.v(tag, msg);
        }

        return 0;
    }

    public static void printStackTrace(Throwable e) {
        if (ENABLE_WARN) {
            e.printStackTrace();
        }
    }

    public static void printCause(Throwable e) {
        if (ENABLE_WARN) {
            Throwable cause = e.getCause();
            if (cause != null)
                e = cause;

            printStackTrace(e);
        }
    }
}

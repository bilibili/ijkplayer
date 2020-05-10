/*
 * Copyright (C) 2013 Bilibili
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

package tv.danmaku.ijk.media.player.pragma;

import android.util.Log;

import java.util.Locale;

public class DebugLog {

    private static int level = Log.INFO;

    public static void setLogLevel(int logLevel) {
        level = logLevel;
    }

    public static void e(String tag, String msg) {
        if (level <= Log.ERROR) {
            Log.e(tag, msg);
        }
    }

    public static void e(String tag, String msg, Throwable tr) {
        if (level <= Log.ERROR) {
            Log.e(tag, msg, tr);
        }
    }

    public static void efmt(String tag, String fmt, Object... args) {
        if (level <= Log.ERROR) {
            String msg = String.format(Locale.US, fmt, args);
            Log.e(tag, msg);
        }
    }

    public static void i(String tag, String msg) {
        if (level <= Log.INFO) {
            Log.i(tag, msg);
        }
    }

    public static void i(String tag, String msg, Throwable tr) {
        if (level <= Log.INFO) {
            Log.i(tag, msg, tr);
        }
    }

    public static void ifmt(String tag, String fmt, Object... args) {
        if (level <= Log.INFO) {
            String msg = String.format(Locale.US, fmt, args);
            Log.i(tag, msg);
        }
    }

    public static void w(String tag, String msg) {
        if (level <= Log.WARN) {
            Log.w(tag, msg);
        }
    }

    public static void w(String tag, String msg, Throwable tr) {
        if (level <= Log.WARN) {
            Log.w(tag, msg, tr);
        }
    }

    public static void wfmt(String tag, String fmt, Object... args) {
        if (level <= Log.WARN) {
            String msg = String.format(Locale.US, fmt, args);
            Log.w(tag, msg);
        }
    }

    public static void d(String tag, String msg) {
        if (level <= Log.DEBUG) {
            Log.d(tag, msg);
        }
    }

    public static void d(String tag, String msg, Throwable tr) {
        if (level <= Log.DEBUG) {
            Log.d(tag, msg, tr);
        }
    }

    public static void dfmt(String tag, String fmt, Object... args) {
        if (level <= Log.DEBUG) {
            String msg = String.format(Locale.US, fmt, args);
            Log.d(tag, msg);
        }
    }

    public static void v(String tag, String msg) {
        if (level <= Log.VERBOSE) {
            Log.v(tag, msg);
        }
    }

    public static void v(String tag, String msg, Throwable tr) {
        if (level <= Log.VERBOSE) {
            Log.v(tag, msg, tr);
        }
    }

    public static void vfmt(String tag, String fmt, Object... args) {
        if (level <= Log.VERBOSE) {
            String msg = String.format(Locale.US, fmt, args);
            Log.v(tag, msg);
        }
    }

    public static void printStackTrace(Throwable e) {
        if (level <= Log.ERROR) {
            e.printStackTrace();
        }
    }

    public static void printCause(Throwable e) {
        if (level <= Log.ERROR) {
            Throwable cause = e.getCause();
            if (cause != null)
                e = cause;

            printStackTrace(e);
        }
    }
}

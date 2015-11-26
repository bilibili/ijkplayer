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

package tv.danmaku.ijk.media.player.pragma;

import java.util.Locale;


import android.util.Log;

@SuppressWarnings({"SameParameterValue", "WeakerAccess"})
public class DebugLog {
    public static final boolean ENABLE_ERROR = Pragma.ENABLE_VERBOSE;
    public static final boolean ENABLE_INFO = Pragma.ENABLE_VERBOSE;
    public static final boolean ENABLE_WARN = Pragma.ENABLE_VERBOSE;
    public static final boolean ENABLE_DEBUG = Pragma.ENABLE_VERBOSE;
    public static final boolean ENABLE_VERBOSE = Pragma.ENABLE_VERBOSE;

    public static void e(String tag, String msg) {
        if (ENABLE_ERROR) {
            Log.e(tag, msg);
        }
    }

    public static void e(String tag, String msg, Throwable tr) {
        if (ENABLE_ERROR) {
            Log.e(tag, msg, tr);
        }
    }

    public static void efmt(String tag, String fmt, Object... args) {
        if (ENABLE_ERROR) {
            String msg = String.format(Locale.US, fmt, args);
            Log.e(tag, msg);
        }
    }

    public static void i(String tag, String msg) {
        if (ENABLE_INFO) {
            Log.i(tag, msg);
        }
    }

    public static void i(String tag, String msg, Throwable tr) {
        if (ENABLE_INFO) {
            Log.i(tag, msg, tr);
        }
    }

    public static void ifmt(String tag, String fmt, Object... args) {
        if (ENABLE_INFO) {
            String msg = String.format(Locale.US, fmt, args);
            Log.i(tag, msg);
        }
    }

    public static void w(String tag, String msg) {
        if (ENABLE_WARN) {
            Log.w(tag, msg);
        }
    }

    public static void w(String tag, String msg, Throwable tr) {
        if (ENABLE_WARN) {
            Log.w(tag, msg, tr);
        }
    }

    public static void wfmt(String tag, String fmt, Object... args) {
        if (ENABLE_WARN) {
            String msg = String.format(Locale.US, fmt, args);
            Log.w(tag, msg);
        }
    }

    public static void d(String tag, String msg) {
        if (ENABLE_DEBUG) {
            Log.d(tag, msg);
        }
    }

    public static void d(String tag, String msg, Throwable tr) {
        if (ENABLE_DEBUG) {
            Log.d(tag, msg, tr);
        }
    }

    public static void dfmt(String tag, String fmt, Object... args) {
        if (ENABLE_DEBUG) {
            String msg = String.format(Locale.US, fmt, args);
            Log.d(tag, msg);
        }
    }

    public static void v(String tag, String msg) {
        if (ENABLE_VERBOSE) {
            Log.v(tag, msg);
        }
    }

    public static void v(String tag, String msg, Throwable tr) {
        if (ENABLE_VERBOSE) {
            Log.v(tag, msg, tr);
        }
    }

    public static void vfmt(String tag, String fmt, Object... args) {
        if (ENABLE_VERBOSE) {
            String msg = String.format(Locale.US, fmt, args);
            Log.v(tag, msg);
        }
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

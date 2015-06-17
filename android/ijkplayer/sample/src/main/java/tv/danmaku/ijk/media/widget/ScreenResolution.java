/*
 * Copyright (C) 2006 The Android Open Source Project
 * Copyright (C) 2013 YIXIA.COM
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

import android.annotation.TargetApi;
import android.content.Context;
import android.os.Build;
import android.util.DisplayMetrics;
import android.util.Pair;
import android.view.Display;
import android.view.WindowManager;

import java.lang.reflect.Method;

/**
 * Class to get the real screen resolution includes the system status bar.
 * We can get the value by calling the getRealMetrics method if API >= 17
 * Reflection needed on old devices..
 * */
public class ScreenResolution {
  /**
   * Gets the resolution,
   * @return a pair to return the width and height
   * */
  public static Pair<Integer,Integer> getResolution(Context ctx){
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
      return getRealResolution(ctx);
    }
    else {
      return getRealResolutionOnOldDevice(ctx);
    }
  }

  /**
   * Gets resolution on old devices.
   * Tries the reflection to get the real resolution first.
   * Fall back to getDisplayMetrics if the above method failed.
   * */
  private static Pair<Integer, Integer> getRealResolutionOnOldDevice(Context ctx) {
    try{
      WindowManager wm = (WindowManager) ctx.getSystemService(Context.WINDOW_SERVICE);
      Display display = wm.getDefaultDisplay();
      Method mGetRawWidth = Display.class.getMethod("getRawWidth");
      Method mGetRawHeight = Display.class.getMethod("getRawHeight");
      Integer realWidth = (Integer) mGetRawWidth.invoke(display);
      Integer realHeight = (Integer) mGetRawHeight.invoke(display);
      return new Pair<Integer, Integer>(realWidth, realHeight);
    }
    catch (Exception e) {
      DisplayMetrics disp = ctx.getResources().getDisplayMetrics();
      return new Pair<Integer, Integer>(disp.widthPixels, disp.heightPixels);
    }
  }

  /**
   * Gets real resolution via the new getRealMetrics API.
   * */
  @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
  private static Pair<Integer,Integer> getRealResolution(Context ctx) {
    WindowManager wm = (WindowManager) ctx.getSystemService(Context.WINDOW_SERVICE);
    Display display = wm.getDefaultDisplay();
    DisplayMetrics metrics = new DisplayMetrics();
    display.getRealMetrics(metrics);
    return new Pair<Integer, Integer>(metrics.widthPixels, metrics.heightPixels);
  }
}
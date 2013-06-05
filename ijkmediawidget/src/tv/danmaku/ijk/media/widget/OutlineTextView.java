/*
 * Copyright (C) 2011 Cedric Fung (wolfplanet@gmail.com)
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

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.text.Layout;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.util.AttributeSet;
import android.widget.TextView;

/**
 * Display text with border, use the same XML attrs as
 * {@link android.widget.TextView}, except that {@link OutlineTextView} will
 * transform the shadow to border
 */
@SuppressLint("DrawAllocation")
public class OutlineTextView extends TextView {
    public OutlineTextView(Context context) {
        super(context);
        initPaint();
    }

    public OutlineTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initPaint();
    }

    public OutlineTextView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initPaint();
    }

    private void initPaint() {
        mTextPaint = new TextPaint();
        mTextPaint.setAntiAlias(true);
        mTextPaint.setTextSize(getTextSize());
        mTextPaint.setColor(mColor);
        mTextPaint.setStyle(Paint.Style.FILL);
        mTextPaint.setTypeface(getTypeface());

        mTextPaintOutline = new TextPaint();
        mTextPaintOutline.setAntiAlias(true);
        mTextPaintOutline.setTextSize(getTextSize());
        mTextPaintOutline.setColor(mBorderColor);
        mTextPaintOutline.setStyle(Paint.Style.STROKE);
        mTextPaintOutline.setTypeface(getTypeface());
        mTextPaintOutline.setStrokeWidth(mBorderSize);
    }

    public void setText(String text) {
        super.setText(text);
        mText = text.toString();
        requestLayout();
        invalidate();
    }

    public void setTextSize(float size) {
        super.setTextSize(size);
        requestLayout();
        invalidate();
        initPaint();
    }

    public void setTextColor(int color) {
        super.setTextColor(color);
        mColor = color;
        invalidate();
        initPaint();
    }

    public void setShadowLayer(float radius, float dx, float dy, int color) {
        super.setShadowLayer(radius, dx, dy, color);
        mBorderSize = radius;
        mBorderColor = color;
        requestLayout();
        invalidate();
        initPaint();
    }

    public void setTypeface(Typeface tf, int style) {
        super.setTypeface(tf, style);
        requestLayout();
        invalidate();
        initPaint();
    }

    public void setTypeface(Typeface tf) {
        super.setTypeface(tf);
        requestLayout();
        invalidate();
        initPaint();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        Layout layout = new StaticLayout(getText(), mTextPaintOutline,
                getWidth(), Layout.Alignment.ALIGN_CENTER, mSpacingMult,
                mSpacingAdd, mIncludePad);
        layout.draw(canvas);
        layout = new StaticLayout(getText(), mTextPaint, getWidth(),
                Layout.Alignment.ALIGN_CENTER, mSpacingMult, mSpacingAdd,
                mIncludePad);
        layout.draw(canvas);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        Layout layout = new StaticLayout(getText(), mTextPaintOutline,
                measureWidth(widthMeasureSpec), Layout.Alignment.ALIGN_CENTER,
                mSpacingMult, mSpacingAdd, mIncludePad);
        int ex = (int) (mBorderSize * 2 + 1);
        setMeasuredDimension(measureWidth(widthMeasureSpec) + ex,
                measureHeight(heightMeasureSpec) * layout.getLineCount() + ex);
    }

    private int measureWidth(int measureSpec) {
        int result = 0;
        int specMode = MeasureSpec.getMode(measureSpec);
        int specSize = MeasureSpec.getSize(measureSpec);

        if (specMode == MeasureSpec.EXACTLY) {
            result = specSize;
        } else {
            result = (int) mTextPaintOutline.measureText(mText)
                    + getPaddingLeft() + getPaddingRight();
            if (specMode == MeasureSpec.AT_MOST) {
                result = Math.min(result, specSize);
            }
        }

        return result;
    }

    private int measureHeight(int measureSpec) {
        int result = 0;
        int specMode = MeasureSpec.getMode(measureSpec);
        int specSize = MeasureSpec.getSize(measureSpec);

        mAscent = (int) mTextPaintOutline.ascent();
        if (specMode == MeasureSpec.EXACTLY) {
            result = specSize;
        } else {
            result = (int) (-mAscent + mTextPaintOutline.descent())
                    + getPaddingTop() + getPaddingBottom();
            if (specMode == MeasureSpec.AT_MOST) {
                result = Math.min(result, specSize);
            }
        }
        return result;
    }

    private TextPaint mTextPaint;
    private TextPaint mTextPaintOutline;
    private String mText = "";
    private int mAscent = 0;
    private float mBorderSize;
    private int mBorderColor;
    private int mColor;
    private float mSpacingMult = 1.0f;
    private float mSpacingAdd = 0;
    private boolean mIncludePad = true;
}
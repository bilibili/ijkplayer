/*
 * Copyright (C) 2012 YIXIA.COM
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

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RemoteViews.RemoteView;

@RemoteView
public class CenterLayout extends ViewGroup {
	private int mPaddingLeft = 0;
	private int mPaddingRight = 0;
	private int mPaddingTop = 0;
	private int mPaddingBottom = 0;
	private int mWidth, mHeight;

	public CenterLayout(Context context) {
		super(context);
	}

	public CenterLayout(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public CenterLayout(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		int count = getChildCount();

		int maxHeight = 0;
		int maxWidth = 0;

		measureChildren(widthMeasureSpec, heightMeasureSpec);

		for (int i = 0; i < count; i++) {
			View child = getChildAt(i);
			if (child.getVisibility() != GONE) {
				int childRight;
				int childBottom;

				CenterLayout.LayoutParams lp = (CenterLayout.LayoutParams) child.getLayoutParams();

				childRight = lp.x + child.getMeasuredWidth();
				childBottom = lp.y + child.getMeasuredHeight();

				maxWidth = Math.max(maxWidth, childRight);
				maxHeight = Math.max(maxHeight, childBottom);
			}
		}

		maxWidth += mPaddingLeft + mPaddingRight;
		maxHeight += mPaddingTop + mPaddingBottom;

		maxHeight = Math.max(maxHeight, getSuggestedMinimumHeight());
		maxWidth = Math.max(maxWidth, getSuggestedMinimumWidth());

		setMeasuredDimension(resolveSize(maxWidth, widthMeasureSpec), resolveSize(maxHeight, heightMeasureSpec));
	}

	@Override
	protected void onLayout(boolean changed, int l, int t, int r, int b) {
		int count = getChildCount();
		mWidth = getMeasuredWidth();
		mHeight = getMeasuredHeight();
		for (int i = 0; i < count; i++) {
			View child = getChildAt(i);
			if (child.getVisibility() != GONE) {
				CenterLayout.LayoutParams lp = (CenterLayout.LayoutParams) child.getLayoutParams();
				int childLeft = mPaddingLeft + lp.x;
				if (lp.width > 0)
					childLeft += (int) ((mWidth - lp.width) / 2.0);
				else
					childLeft += (int) ((mWidth - child.getMeasuredWidth()) / 2.0);
				int childTop = mPaddingTop + lp.y;
				if (lp.height > 0)
					childTop += (int) ((mHeight - lp.height) / 2.0);
				else
					childTop += (int) ((mHeight - child.getMeasuredHeight()) / 2.0);
				child.layout(childLeft, childTop, childLeft + child.getMeasuredWidth(), childTop + child.getMeasuredHeight());
			}
		}
	}

	@Override
	protected boolean checkLayoutParams(ViewGroup.LayoutParams p) {
		return p instanceof CenterLayout.LayoutParams;
	}

	@Override
	protected ViewGroup.LayoutParams generateLayoutParams(ViewGroup.LayoutParams p) {
		return new LayoutParams(p);
	}

	public static class LayoutParams extends ViewGroup.LayoutParams {
		public int x;
		public int y;

		public LayoutParams(int width, int height, int x, int y) {
			super(width, height);
			this.x = x;
			this.y = y;
		}

		public LayoutParams(ViewGroup.LayoutParams source) {
			super(source);
		}
	}
}

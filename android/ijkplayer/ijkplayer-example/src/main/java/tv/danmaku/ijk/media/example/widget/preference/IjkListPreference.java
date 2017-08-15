/*
 * Copyright (C) 2015 Bilibili
 * Copyright (C) 2015 Zhang Rui <bbcallen@gmail.com>
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

package tv.danmaku.ijk.media.example.widget.preference;

import android.content.Context;
import android.content.res.TypedArray;
import android.support.v7.preference.ListPreference;
import android.text.TextUtils;
import android.util.AttributeSet;

import tv.danmaku.ijk.media.example.R;

public class IjkListPreference extends ListPreference {
    private CharSequence[] mEntrySummaries;

    public IjkListPreference(Context context) {
        super(context);
        initPreference(context, null);
    }

    public IjkListPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        initPreference(context, attrs);
    }

    public IjkListPreference(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initPreference(context, attrs);
    }

    public IjkListPreference(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        initPreference(context, attrs);
    }

    private void initPreference(Context context, AttributeSet attrs) {
        TypedArray a = context.obtainStyledAttributes(attrs,
                R.styleable.IjkListPreference, 0, 0);
        if (a == null)
            return;

        mEntrySummaries = a
                .getTextArray(R.styleable.IjkListPreference_entrySummaries);

        a.recycle();
    }

    @Override
    protected void onSetInitialValue(boolean restoreValue, Object defaultValue) {
        super.onSetInitialValue(restoreValue, defaultValue);
        syncSummary();
    }

    @Override
    public void setValue(String value) {
        super.setValue(value);
        syncSummary();
    }

    @Override
    public void setValueIndex(int index) {
        super.setValueIndex(index);
        syncSummary();
    }

    public int getEntryIndex() {
        CharSequence[] entryValues = getEntryValues();
        CharSequence value = getValue();

        if (entryValues == null || value == null) {
            return -1;
        }

        for (int i = 0; i < entryValues.length; ++i) {
            if (TextUtils.equals(value, entryValues[i])) {
                return i;
            }
        }

        return -1;
    }

    // ----- summary --------------------
    public void setEntrySummaries(Context context, int resId) {
        setEntrySummaries(context.getResources().getTextArray(resId));
    }

    public void setEntrySummaries(CharSequence[] entrySummaries) {
        mEntrySummaries = entrySummaries;
        notifyChanged();
    }

    public CharSequence[] getEntrySummaries() {
        return mEntrySummaries;
    }

    private void syncSummary() {
        int index = getEntryIndex();
        if (index < 0)
            return;

        if (mEntrySummaries != null && index < mEntrySummaries.length) {
            setSummary(mEntrySummaries[index]);
        } else {
            setSummary(getEntries()[index]);
        }
    }
}

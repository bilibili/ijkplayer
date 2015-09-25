/*
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

package tv.danmaku.ijk.media.sample.widget.media;

import android.content.Context;
import android.support.v7.app.AlertDialog;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.TableLayout;
import android.widget.TextView;

import tv.danmaku.ijk.media.sample.R;

public class TableDialogBuilder {
    private Context mContext;
    public ViewGroup mTableView;
    public TableLayout mTableLayout;

    public TableDialogBuilder(Context context) {
        mContext = context;
        mTableView = (ViewGroup) LayoutInflater.from(mContext).inflate(R.layout.table_media_info, null);
        mTableLayout = (TableLayout) mTableView.findViewById(R.id.table);
    }

    public void appendRow1(String name, String value) {
        appendRow(R.layout.table_media_info_row1, name, value);
    }

    public void appendRow1(int nameId, String value) {
        appendRow1(mContext.getString(nameId), value);
    }

    public void appendRow2(String name, String value) {
        appendRow(R.layout.table_media_info_row2, name, value);
    }

    public void appendRow2(int nameId, String value) {
        appendRow2(mContext.getString(nameId), value);
    }

    public void appendSection(String name) {
        appendRow(R.layout.table_media_info_section, name, null);
    }

    public void appendSection(int nameId) {
        appendSection(mContext.getString(nameId));
    }

    public void appendRow(int layoutId, String name, String value) {
        ViewGroup infoRow = (ViewGroup) LayoutInflater.from(mContext).inflate(layoutId, mTableLayout, false);

        if (!TextUtils.isEmpty(name)) {
            TextView keyView = (TextView) infoRow.findViewById(R.id.name);
            keyView.setText(name);
        }

        if (!TextUtils.isEmpty(value)) {
            TextView valueView = (TextView) infoRow.findViewById(R.id.value);
            valueView.setText(value);
        }

        mTableLayout.addView(infoRow);
    }

    public ViewGroup buildLayout() {
        return mTableView;
    }

    public android.support.v7.app.AlertDialog.Builder buildAlertDialogBuilder() {
        AlertDialog.Builder dlgBuilder = new AlertDialog.Builder(mContext);
        dlgBuilder.setView(buildLayout());
        return dlgBuilder;
    }
}

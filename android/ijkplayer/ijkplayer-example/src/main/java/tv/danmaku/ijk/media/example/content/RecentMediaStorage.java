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

package tv.danmaku.ijk.media.example.content;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.os.AsyncTask;
import android.support.v4.content.AsyncTaskLoader;
import android.text.TextUtils;

public class RecentMediaStorage {
    private Context mAppContext;

    public RecentMediaStorage(Context context) {
        mAppContext = context.getApplicationContext();
    }

    public void saveUrlAsync(String url) {
        new AsyncTask<String, Void, Void>() {
            @Override
            protected Void doInBackground(String... params) {
                saveUrl(params[0]);
                return null;
            }
        }.execute(url);
    }

    public void saveUrl(String url) {
        ContentValues cv = new ContentValues();
        cv.putNull(Entry.COLUMN_NAME_ID);
        cv.put(Entry.COLUMN_NAME_URL, url);
        cv.put(Entry.COLUMN_NAME_LAST_ACCESS, System.currentTimeMillis());
        cv.put(Entry.COLUMN_NAME_NAME, getNameOfUrl(url));
        save(cv);
    }

    public void save(ContentValues contentValue) {
        OpenHelper openHelper = new OpenHelper(mAppContext);
        SQLiteDatabase db = openHelper.getWritableDatabase();
        db.replace(Entry.TABLE_NAME, null, contentValue);
    }

    public static String getNameOfUrl(String url) {
        return getNameOfUrl(url, "");
    }

    public static String getNameOfUrl(String url, String defaultName) {
        String name = null;
        int pos = url.lastIndexOf('/');
        if (pos >= 0)
            name = url.substring(pos + 1);

        if (TextUtils.isEmpty(name))
            name = defaultName;

        return name;
    }

    public static class Entry {
        public static final String TABLE_NAME = "RecentMedia";
        public static final String COLUMN_NAME_ID = "id";
        public static final String COLUMN_NAME_URL = "url";
        public static final String COLUMN_NAME_NAME = "name";
        public static final String COLUMN_NAME_LAST_ACCESS = "last_access";
    }

    public static final String ALL_COLUMNS[] = new String[]{
            Entry.COLUMN_NAME_ID + " as _id",
            Entry.COLUMN_NAME_ID,
            Entry.COLUMN_NAME_URL,
            Entry.COLUMN_NAME_NAME,
            Entry.COLUMN_NAME_LAST_ACCESS};

    public static class OpenHelper extends SQLiteOpenHelper {
        private static final int DATABASE_VERSION = 1;
        private static final String DATABASE_NAME = "RecentMedia.db";
        private static final String SQL_CREATE_ENTRIES =
                " CREATE TABLE IF NOT EXISTS " + Entry.TABLE_NAME + " (" +
                        Entry.COLUMN_NAME_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                        Entry.COLUMN_NAME_URL + " VARCHAR UNIQUE, " +
                        Entry.COLUMN_NAME_NAME + " VARCHAR, " +
                        Entry.COLUMN_NAME_LAST_ACCESS + " INTEGER) ";

        public OpenHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL(SQL_CREATE_ENTRIES);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        }
    }

    public static class CursorLoader extends AsyncTaskLoader<Cursor> {
        public CursorLoader(Context context) {
            super(context);
        }

        @Override
        public Cursor loadInBackground() {
            Context context = getContext();
            OpenHelper openHelper = new OpenHelper(context);
            SQLiteDatabase db = openHelper.getReadableDatabase();

            return db.query(Entry.TABLE_NAME, ALL_COLUMNS, null, null, null, null,
                    Entry.COLUMN_NAME_LAST_ACCESS + " DESC",
                    "100");
        }

        @Override
        protected void onStartLoading() {
            forceLoad();
        }
    }
}

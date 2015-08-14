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

package tv.danmaku.ijk.media.sample.content;

import android.database.AbstractCursor;
import android.provider.BaseColumns;
import android.provider.MediaStore;

import java.io.File;

public class PathCursor extends AbstractCursor {
    private File[] fileList;

    public static final String _ID = BaseColumns._ID;
    public static final String DISPLAY_NAME = MediaStore.Video.Media.DISPLAY_NAME;
    public static final String DATA = MediaStore.Video.Media.DATA;
    public static final String[] columnNames = new String[] {_ID, DISPLAY_NAME, DATA};
    public static final int COL_ID = 0;
    public static final int COL_DISPLAY_NAME = 1;
    public static final int COL_DATA = 2;

    PathCursor(File[] fileList) {
        this.fileList = fileList;
    }

    @Override
    public int getCount() {
        return fileList.length;
    }

    @Override
    public String[] getColumnNames() {
        return columnNames;
    }

    @Override
    public String getString(int column) {
        switch (column) {
            case COL_DATA:
            case COL_DISPLAY_NAME:
                return fileList[getPosition()].toString();
        }
        return null;
    }

    @Override
    public short getShort(int column) {
        return 0;
    }

    @Override
    public int getInt(int column) {
        return 0;
    }

    @Override
    public long getLong(int column) {
        return 0;
    }

    @Override
    public float getFloat(int column) {
        return 0;
    }

    @Override
    public double getDouble(int column) {
        return 0;
    }

    @Override
    public boolean isNull(int column) {
        return fileList == null;
    }
}

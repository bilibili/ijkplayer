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
import android.text.TextUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

public class PathCursor extends AbstractCursor {
    private List<FileItem> mFileList = new ArrayList<>();

    public static final String CN_ID = BaseColumns._ID;
    public static final String CN_FILE_NAME = "file_name";
    public static final String CN_FILE_PATH = "file_path";
    public static final String CN_IS_DIRECTORY = "is_directory";
    public static final String CN_IS_VIDEO = "is_video";
    public static final String[] columnNames = new String[]{CN_ID, CN_FILE_NAME, CN_FILE_PATH, CN_IS_DIRECTORY, CN_IS_VIDEO};
    public static final int CI_ID = 0;
    public static final int CI_FILE_NAME = 1;
    public static final int CI_FILE_PATH = 2;
    public static final int CI_IS_DIRECTORY = 3;
    public static final int CI_IS_VIDEO = 4;

    PathCursor(File parentDirectory, File[] fileList) {
        if (parentDirectory.getParent() != null) {
            FileItem parentFile = new FileItem(new File(parentDirectory, ".."));
            parentFile.isDirectory = true;
            mFileList.add(parentFile);
        }

        if (fileList != null) {
            for (File file : fileList) {
                mFileList.add(new FileItem(file));
            }
            Collections.sort(this.mFileList, sComparator);
        }
    }

    @Override
    public int getCount() {
        return mFileList.size();
    }

    @Override
    public String[] getColumnNames() {
        return columnNames;
    }

    @Override
    public String getString(int column) {
        switch (column) {
            case CI_FILE_NAME:
                return mFileList.get(getPosition()).file.getName().toString();
            case CI_FILE_PATH:
                return mFileList.get(getPosition()).file.toString();
        }
        return null;
    }

    @Override
    public short getShort(int column) {
        return (short) getLong(column);
    }

    @Override
    public int getInt(int column) {
        return (int) getLong(column);
    }

    @Override
    public long getLong(int column) {
        switch (column) {
            case CI_ID:
                return getPosition();
            case CI_IS_DIRECTORY:
                return mFileList.get(getPosition()).isDirectory ? 1 : 0;
            case CI_IS_VIDEO:
                return mFileList.get(getPosition()).isVideo ? 1 : 0;
        }
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
        return mFileList == null;
    }

    public static Comparator<FileItem> sComparator = new Comparator<FileItem>() {
        @Override
        public int compare(FileItem lhs, FileItem rhs) {
            if (lhs.isDirectory && !rhs.isDirectory)
                return -1;
            else if (!lhs.isDirectory && rhs.isDirectory)
                return 1;
            return lhs.file.compareTo(rhs.file);
        }
    };

    private static Set<String> sMediaExtSet = new TreeSet<String>(String.CASE_INSENSITIVE_ORDER);

    {
        sMediaExtSet.add("flv");
        sMediaExtSet.add("mp4");
    }

    private class FileItem {
        public File file;
        public boolean isDirectory;
        public boolean isVideo;

        public FileItem(String file) {
            this(new File(file));
        }

        public FileItem(File file) {
            this.file = file;
            this.isDirectory = file.isDirectory();

            String fileName = file.getName();
            if (!TextUtils.isEmpty(fileName)) {
                int extPos = fileName.lastIndexOf('.');
                if (extPos >= 0) {
                    String ext = fileName.substring(extPos + 1);
                    if (!TextUtils.isEmpty(ext) && sMediaExtSet.contains(ext)) {
                        this.isVideo = true;
                    }
                }
            }
        }
    }
}

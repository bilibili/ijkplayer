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

package tv.danmaku.ijk.media.player.misc;

import android.annotation.TargetApi;
import android.media.MediaDataSource;
import android.os.Build;

import java.io.IOException;

@TargetApi(Build.VERSION_CODES.M)
public class AndroidMediaDataSource extends MediaDataSource implements IMediaDataSource {
    private IMediaDataSource mMediaDataSource;

    public AndroidMediaDataSource(IMediaDataSource mediaDataSource) {
        mMediaDataSource = mediaDataSource;
    }

    @Override
    public int readAt(long position, byte[] buffer, int offset, int size) throws IOException {
        return mMediaDataSource.readAt(position, buffer, offset, size);
    }

    @Override
    public long getSize() throws IOException {
        return mMediaDataSource.getSize();
    }

    @Override
    public void close() throws IOException {
        mMediaDataSource.close();
    }
}

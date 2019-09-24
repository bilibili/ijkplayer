/*
 * Copyright (C) 2015 Bilibili
 * Copyright (C) 2019 Befovy <befovy@gmail.com>
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

import java.io.IOException;
import java.io.InputStream;

public class StreamDataSource implements IMediaDataSource {
    private InputStream mIs;
    private long mPosition = 0;

    public StreamDataSource(InputStream mIs) {
        this.mIs = mIs;
    }

    @Override
    public int readAt(long position, byte[] buffer, int offset, int size) throws IOException {
        if (size <= 0)
            return size;
        if (mPosition != position) {
            mIs.reset();
            mPosition = mIs.skip(position);
        }
        int length = mIs.read(buffer, offset, size);
        mPosition += length;
        return length;
    }

    @Override
    public long getSize() throws IOException {
        return mIs.available();
    }

    @Override
    public void close() throws IOException {
        if (mIs != null)
            mIs.close();
        mIs = null;
    }
}

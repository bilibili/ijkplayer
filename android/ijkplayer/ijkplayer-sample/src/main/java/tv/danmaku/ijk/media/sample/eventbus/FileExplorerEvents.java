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

package tv.danmaku.ijk.media.sample.eventbus;

import com.squareup.otto.Bus;

import java.io.File;

public class FileExplorerEvents {
    private static final Bus BUS = new Bus();

    public static Bus getBus() {
        return BUS;
    }

    private FileExplorerEvents() {
        // No instances.
    }

    public static class OnClickFile {
        public File mFile;

        public OnClickFile(String path) {
            this(new File(path));
        }

        public OnClickFile(File file) {
            mFile = file;
        }
    }
}

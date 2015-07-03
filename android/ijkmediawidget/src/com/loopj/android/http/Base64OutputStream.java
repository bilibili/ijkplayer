/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.loopj.android.http;

import java.io.FilterOutputStream;
import java.io.IOException;
import java.io.OutputStream;

public class Base64OutputStream extends FilterOutputStream {
    private final Base64.Coder coder;
    private final int flags;

    private byte[] buffer = null;
    private int bpos = 0;

    private static byte[] EMPTY = new byte[0];

    /**
     * Performs Base64 encoding on the data written to the stream, writing the encoded data to
     * another OutputStream.
     *
     * @param out   the OutputStream to write the encoded data to
     * @param flags bit flags for controlling the encoder; see the constants in {@link Base64}
     */
    public Base64OutputStream(OutputStream out, int flags) {
        this(out, flags, true);
    }

    /**
     * Performs Base64 encoding or decoding on the data written to the stream, writing the
     * encoded/decoded data to another OutputStream.
     *
     * @param out    the OutputStream to write the encoded data to
     * @param flags  bit flags for controlling the encoder; see the constants in {@link Base64}
     * @param encode true to encode, false to decode
     */
    public Base64OutputStream(OutputStream out, int flags, boolean encode) {
        super(out);
        this.flags = flags;
        if (encode) {
            coder = new Base64.Encoder(flags, null);
        } else {
            coder = new Base64.Decoder(flags, null);
        }
    }

    @Override
    public void write(int b) throws IOException {
        // To avoid invoking the encoder/decoder routines for single
        // bytes, we buffer up calls to write(int) in an internal
        // byte array to transform them into writes of decently-sized
        // arrays.

        if (buffer == null) {
            buffer = new byte[1024];
        }
        if (bpos >= buffer.length) {
            // internal buffer full; write it out.
            internalWrite(buffer, 0, bpos, false);
            bpos = 0;
        }
        buffer[bpos++] = (byte) b;
    }

    /**
     * Flush any buffered data from calls to write(int).  Needed before doing a write(byte[], int,
     * int) or a close().
     */
    private void flushBuffer() throws IOException {
        if (bpos > 0) {
            internalWrite(buffer, 0, bpos, false);
            bpos = 0;
        }
    }

    @Override
    public void write(byte[] b, int off, int len) throws IOException {
        if (len <= 0) return;
        flushBuffer();
        internalWrite(b, off, len, false);
    }

    @Override
    public void close() throws IOException {
        IOException thrown = null;
        try {
            flushBuffer();
            internalWrite(EMPTY, 0, 0, true);
        } catch (IOException e) {
            thrown = e;
        }

        try {
            if ((flags & Base64.NO_CLOSE) == 0) {
                out.close();
            } else {
                out.flush();
            }
        } catch (IOException e) {
            if (thrown != null) {
                thrown = e;
            }
        }

        if (thrown != null) {
            throw thrown;
        }
    }

    /**
     * Write the given bytes to the encoder/decoder.
     *
     * @param finish true if this is the last batch of input, to cause encoder/decoder state to be
     *               finalized.
     */
    private void internalWrite(byte[] b, int off, int len, boolean finish) throws IOException {
        coder.output = embiggen(coder.output, coder.maxOutputSize(len));
        if (!coder.process(b, off, len, finish)) {
            throw new Base64DataException("bad base-64");
        }
        out.write(coder.output, 0, coder.op);
    }

    /**
     * If b.length is at least len, return b.  Otherwise return a new byte array of length len.
     */
    private byte[] embiggen(byte[] b, int len) {
        if (b == null || b.length < len) {
            return new byte[len];
        } else {
            return b;
        }
    }
}
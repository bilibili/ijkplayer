/*
    Android Asynchronous Http Client
    Copyright (c) 2011 James Smith <james@loopj.com>
    http://loopj.com

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

package com.loopj.android.http;

import android.os.Message;
import android.util.Log;

import org.apache.http.HttpEntity;
import org.apache.http.util.ByteArrayBuffer;

import java.io.IOException;
import java.io.InputStream;

@SuppressWarnings("ALL")
public abstract class DataAsyncHttpResponseHandler extends AsyncHttpResponseHandler {
    private static final String LOG_TAG = "DataAsyncHttpResponseHandler";

    protected static final int PROGRESS_DATA_MESSAGE = 7;

    /**
     * Creates a new AsyncHttpResponseHandler
     */
    public DataAsyncHttpResponseHandler() {
        super();
    }

    /**
     * Fired when the request progress, override to handle in your own code
     *
     * @param responseBody response body received so far
     */
    public void onProgressData(byte[] responseBody) {
        Log.d(LOG_TAG, "onProgressData(byte[]) was not overriden, but callback was received");
    }


    final public void sendProgressDataMessage(byte[] responseBytes) {
        sendMessage(obtainMessage(PROGRESS_DATA_MESSAGE, new Object[]{responseBytes}));
    }

    // Methods which emulate android's Handler and Message methods
    @Override
    protected void handleMessage(Message message) {
        super.handleMessage(message);
        Object[] response;

        switch (message.what) {
            case PROGRESS_DATA_MESSAGE:
                response = (Object[]) message.obj;
                if (response != null && response.length >= 1) {
                    try {
                        onProgressData((byte[]) response[0]);
                    } catch (Throwable t) {
                        Log.e(LOG_TAG, "custom onProgressData contains an error", t);
                    }
                } else {
                    Log.e(LOG_TAG, "PROGRESS_DATA_MESSAGE didn't got enough params");
                }
                break;
        }
    }

    /**
     * Returns byte array of response HttpEntity contents
     *
     * @param entity can be null
     * @return response entity body or null
     * @throws java.io.IOException if reading entity or creating byte array failed
     */
    @Override
    byte[] getResponseData(HttpEntity entity) throws IOException {

        byte[] responseBody = null;
        if (entity != null) {
            InputStream instream = entity.getContent();
            if (instream != null) {
                long contentLength = entity.getContentLength();
                if (contentLength > Integer.MAX_VALUE) {
                    throw new IllegalArgumentException("HTTP entity too large to be buffered in memory");
                }
                if (contentLength < 0) {
                    contentLength = BUFFER_SIZE;
                }
                try {
                    ByteArrayBuffer buffer = new ByteArrayBuffer((int) contentLength);
                    try {
                        byte[] tmp = new byte[BUFFER_SIZE];
                        int l, count = 0;
                        // do not send messages if request has been cancelled
                        while ((l = instream.read(tmp)) != -1 && !Thread.currentThread().isInterrupted()) {
                            buffer.append(tmp, 0, l);
                            sendProgressDataMessage(copyOfRange(tmp, 0, l));
                            sendProgressMessage(count, contentLength);
                        }
                    } finally {
                        AsyncHttpClient.silentCloseInputStream(instream);
                    }
                    responseBody = buffer.toByteArray();
                } catch (OutOfMemoryError e) {
                    System.gc();
                    throw new IOException("File too large to fit into available memory");
                }
            }
        }
        return responseBody;
    }

    /**
     * Copies elements from {@code original} into a new array, from indexes start (inclusive) to end
     * (exclusive). The original order of elements is preserved. If {@code end} is greater than
     * {@code original.length}, the result is padded with the value {@code (byte) 0}.
     *
     * @param original the original array
     * @param start    the start index, inclusive
     * @param end      the end index, exclusive
     * @return the new array
     * @throws ArrayIndexOutOfBoundsException if {@code start < 0 || start > original.length}
     * @throws IllegalArgumentException       if {@code start > end}
     * @throws NullPointerException           if {@code original == null}
     * @see java.util.Arrays
     * @since 1.6
     */
    public static byte[] copyOfRange(byte[] original, int start, int end) throws ArrayIndexOutOfBoundsException, IllegalArgumentException, NullPointerException {
        if (start > end) {
            throw new IllegalArgumentException();
        }
        int originalLength = original.length;
        if (start < 0 || start > originalLength) {
            throw new ArrayIndexOutOfBoundsException();
        }
        int resultLength = end - start;
        int copyLength = Math.min(resultLength, originalLength - start);
        byte[] result = new byte[resultLength];
        System.arraycopy(original, start, result, 0, copyLength);
        return result;
    }
}


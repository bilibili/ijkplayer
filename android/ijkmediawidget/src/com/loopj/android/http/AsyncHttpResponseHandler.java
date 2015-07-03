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

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import org.apache.http.Header;
import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.StatusLine;
import org.apache.http.client.HttpResponseException;
import org.apache.http.util.ByteArrayBuffer;

import java.io.IOException;
import java.io.InputStream;
import java.net.URI;

/**
 * Used to intercept and handle the responses from requests made using {@link AsyncHttpClient}. The
 * {@link #onSuccess(int, org.apache.http.Header[], byte[])} method is designed to be anonymously
 * overridden with your own response handling code. <p>&nbsp;</p> Additionally, you can override the
 * {@link #onFailure(int, org.apache.http.Header[], byte[], Throwable)}, {@link #onStart()}, {@link
 * #onFinish()}, {@link #onRetry(int)} and {@link #onProgress(long, long)} methods as required.
 * <p>&nbsp;</p> For example: <p>&nbsp;</p>
 * <pre>
 * AsyncHttpClient client = new AsyncHttpClient();
 * client.get("http://www.google.com", new AsyncHttpResponseHandler() {
 *     &#064;Override
 *     public void onStart() {
 *         // Initiated the request
 *     }
 *
 *     &#064;Override
 *     public void onSuccess(int statusCode, Header[] headers, byte[] responseBody) {
 *         // Successfully got a response
 *     }
 *
 *     &#064;Override
 *     public void onFailure(int statusCode, Header[] headers, byte[] responseBody, Throwable
 * error)
 * {
 *         // Response failed :(
 *     }
 *
 *     &#064;Override
 *     public void onRetry(int retryNo) {
 *         // Request was retried
 *     }
 *
 *     &#064;Override
 *     public void onProgress(int bytesWritten, int totalSize) {
 *         // Progress notification
 *     }
 *
 *     &#064;Override
 *     public void onFinish() {
 *         // Completed the request (either success or failure)
 *     }
 * });
 * </pre>
 */
@SuppressWarnings("ALL")
public abstract class AsyncHttpResponseHandler implements ResponseHandlerInterface {

    private static final String LOG_TAG = "AsyncHttpResponseHandler";

    protected static final int SUCCESS_MESSAGE = 0;
    protected static final int FAILURE_MESSAGE = 1;
    protected static final int START_MESSAGE = 2;
    protected static final int FINISH_MESSAGE = 3;
    protected static final int PROGRESS_MESSAGE = 4;
    protected static final int RETRY_MESSAGE = 5;
    protected static final int CANCEL_MESSAGE = 6;

    protected static final int BUFFER_SIZE = 4096;

    public static final String DEFAULT_CHARSET = "UTF-8";
    public static final String UTF8_BOM = "\uFEFF";
    private String responseCharset = DEFAULT_CHARSET;
    private Handler handler;
    private boolean useSynchronousMode;
    private boolean usePoolThread;

    private URI requestURI = null;
    private Header[] requestHeaders = null;
    private Looper looper = null;

    /**
     * Creates a new AsyncHttpResponseHandler
     */
    public AsyncHttpResponseHandler() {
        this(null);
    }

    /**
     * Creates a new AsyncHttpResponseHandler with a user-supplied looper. If
     * the passed looper is null, the looper attached to the current thread will
     * be used.
     *
     * @param looper The looper to work with
     */
    public AsyncHttpResponseHandler(Looper looper) {
        this.looper = looper == null ? Looper.myLooper() : looper;

        // Use asynchronous mode by default.
        setUseSynchronousMode(false);

        // Do not use the pool's thread to fire callbacks by default.
        setUsePoolThread(false);
    }

    /**
     * Creates a new AsyncHttpResponseHandler and decide whether the callbacks
     * will be fired on current thread's looper or the pool thread's.
     *
     * @param usePoolThread Whether to use the pool's thread to fire callbacks
     */
    public AsyncHttpResponseHandler(boolean usePoolThread) {
        // Whether to use the pool's thread to fire callbacks.
        setUsePoolThread(usePoolThread);

        // When using the pool's thread, there's no sense in having a looper.
        if (!getUsePoolThread()) {
            // Use the current thread's looper.
            this.looper = Looper.myLooper();

            // Use asynchronous mode by default.
            setUseSynchronousMode(false);
        }
    }

    @Override
    public URI getRequestURI() {
        return this.requestURI;
    }

    @Override
    public Header[] getRequestHeaders() {
        return this.requestHeaders;
    }

    @Override
    public void setRequestURI(URI requestURI) {
        this.requestURI = requestURI;
    }

    @Override
    public void setRequestHeaders(Header[] requestHeaders) {
        this.requestHeaders = requestHeaders;
    }

    /**
     * Avoid leaks by using a non-anonymous handler class.
     */
    private static class ResponderHandler extends Handler {
        private final AsyncHttpResponseHandler mResponder;

        ResponderHandler(AsyncHttpResponseHandler mResponder, Looper looper) {
            super(looper);
            this.mResponder = mResponder;
        }

        @Override
        public void handleMessage(Message msg) {
            mResponder.handleMessage(msg);
        }
    }

    @Override
    public boolean getUseSynchronousMode() {
        return useSynchronousMode;
    }

    @Override
    public void setUseSynchronousMode(boolean sync) {
        // A looper must be prepared before setting asynchronous mode.
        if (!sync && looper == null) {
            sync = true;
            Log.w(LOG_TAG, "Current thread has not called Looper.prepare(). Forcing synchronous mode.");
        }

        // If using asynchronous mode.
        if (!sync && handler == null) {
            // Create a handler on current thread to submit tasks
            handler = new ResponderHandler(this, looper);
        } else if (sync && handler != null) {
            // TODO: Consider adding a flag to remove all queued messages.
            handler = null;
        }

        useSynchronousMode = sync;
    }

    @Override
    public boolean getUsePoolThread() {
        return usePoolThread;
    }

    @Override
    public void setUsePoolThread(boolean pool) {
        // If pool thread is to be used, there's no point in keeping a reference
        // to the looper and no need for a handler.
        if (pool) {
            looper = null;
            handler = null;
        }

        usePoolThread = pool;
    }

    /**
     * Sets the charset for the response string. If not set, the default is UTF-8.
     *
     * @param charset to be used for the response string.
     * @see <a href="http://docs.oracle.com/javase/7/docs/api/java/nio/charset/Charset.html">Charset</a>
     */
    public void setCharset(final String charset) {
        this.responseCharset = charset;
    }

    public String getCharset() {
        return this.responseCharset == null ? DEFAULT_CHARSET : this.responseCharset;
    }

    /**
     * Fired when the request progress, override to handle in your own code
     *
     * @param bytesWritten offset from start of file
     * @param totalSize    total size of file
     */
    public void onProgress(long bytesWritten, long totalSize) {
        Log.v(LOG_TAG, String.format("Progress %d from %d (%2.0f%%)", bytesWritten, totalSize, (totalSize > 0) ? (bytesWritten * 1.0 / totalSize) * 100 : -1));
    }

    /**
     * Fired when the request is started, override to handle in your own code
     */
    public void onStart() {
        // default log warning is not necessary, because this method is just optional notification
    }

    /**
     * Fired in all cases when the request is finished, after both success and failure, override to
     * handle in your own code
     */
    public void onFinish() {
        // default log warning is not necessary, because this method is just optional notification
    }

    @Override
    public void onPreProcessResponse(ResponseHandlerInterface instance, HttpResponse response) {
        // default action is to do nothing...
    }

    @Override
    public void onPostProcessResponse(ResponseHandlerInterface instance, HttpResponse response) {
        // default action is to do nothing...
    }

    /**
     * Fired when a request returns successfully, override to handle in your own code
     *
     * @param statusCode   the status code of the response
     * @param headers      return headers, if any
     * @param responseBody the body of the HTTP response from the server
     */
    public abstract void onSuccess(int statusCode, Header[] headers, byte[] responseBody);

    /**
     * Fired when a request fails to complete, override to handle in your own code
     *
     * @param statusCode   return HTTP status code
     * @param headers      return headers, if any
     * @param responseBody the response body, if any
     * @param error        the underlying cause of the failure
     */
    public abstract void onFailure(int statusCode, Header[] headers, byte[] responseBody, Throwable error);

    /**
     * Fired when a retry occurs, override to handle in your own code
     *
     * @param retryNo number of retry
     */
    public void onRetry(int retryNo) {
        Log.d(LOG_TAG, String.format("Request retry no. %d", retryNo));
    }

    public void onCancel() {
        Log.d(LOG_TAG, "Request got cancelled");
    }

    public void onUserException(Throwable error) {
        Log.e(LOG_TAG, "User-space exception detected!", error);
        throw new RuntimeException(error);
    }

    @Override
    final public void sendProgressMessage(long bytesWritten, long bytesTotal) {
        sendMessage(obtainMessage(PROGRESS_MESSAGE, new Object[]{bytesWritten, bytesTotal}));
    }

    @Override
    final public void sendSuccessMessage(int statusCode, Header[] headers, byte[] responseBytes) {
        sendMessage(obtainMessage(SUCCESS_MESSAGE, new Object[]{statusCode, headers, responseBytes}));
    }

    @Override
    final public void sendFailureMessage(int statusCode, Header[] headers, byte[] responseBody, Throwable throwable) {
        sendMessage(obtainMessage(FAILURE_MESSAGE, new Object[]{statusCode, headers, responseBody, throwable}));
    }

    @Override
    final public void sendStartMessage() {
        sendMessage(obtainMessage(START_MESSAGE, null));
    }

    @Override
    final public void sendFinishMessage() {
        sendMessage(obtainMessage(FINISH_MESSAGE, null));
    }

    @Override
    final public void sendRetryMessage(int retryNo) {
        sendMessage(obtainMessage(RETRY_MESSAGE, new Object[]{retryNo}));
    }

    @Override
    final public void sendCancelMessage() {
        sendMessage(obtainMessage(CANCEL_MESSAGE, null));
    }

    // Methods which emulate android's Handler and Message methods
    protected void handleMessage(Message message) {
        Object[] response;

        try {
            switch (message.what) {
                case SUCCESS_MESSAGE:
                    response = (Object[]) message.obj;
                    if (response != null && response.length >= 3) {
                        onSuccess((Integer) response[0], (Header[]) response[1], (byte[]) response[2]);
                    } else {
                        Log.e(LOG_TAG, "SUCCESS_MESSAGE didn't got enough params");
                    }
                    break;
                case FAILURE_MESSAGE:
                    response = (Object[]) message.obj;
                    if (response != null && response.length >= 4) {
                        onFailure((Integer) response[0], (Header[]) response[1], (byte[]) response[2], (Throwable) response[3]);
                    } else {
                        Log.e(LOG_TAG, "FAILURE_MESSAGE didn't got enough params");
                    }
                    break;
                case START_MESSAGE:
                    onStart();
                    break;
                case FINISH_MESSAGE:
                    onFinish();
                    break;
                case PROGRESS_MESSAGE:
                    response = (Object[]) message.obj;
                    if (response != null && response.length >= 2) {
                        try {
                            onProgress((Long) response[0], (Long) response[1]);
                        } catch (Throwable t) {
                            Log.e(LOG_TAG, "custom onProgress contains an error", t);
                        }
                    } else {
                        Log.e(LOG_TAG, "PROGRESS_MESSAGE didn't got enough params");
                    }
                    break;
                case RETRY_MESSAGE:
                    response = (Object[]) message.obj;
                    if (response != null && response.length == 1) {
                        onRetry((Integer) response[0]);
                    } else {
                        Log.e(LOG_TAG, "RETRY_MESSAGE didn't get enough params");
                    }
                    break;
                case CANCEL_MESSAGE:
                    onCancel();
                    break;
            }
        } catch(Throwable error) {
            onUserException(error);
        }
    }

    protected void sendMessage(Message msg) {
        if (getUseSynchronousMode() || handler == null) {
            handleMessage(msg);
        } else if (!Thread.currentThread().isInterrupted()) { // do not send messages if request has been cancelled
            Utils.asserts(handler != null, "handler should not be null!");
            handler.sendMessage(msg);
        }
    }

    /**
     * Helper method to send runnable into local handler loop
     *
     * @param runnable runnable instance, can be null
     */
    protected void postRunnable(Runnable runnable) {
        if (runnable != null) {
            if (getUseSynchronousMode() || handler == null) {
                // This response handler is synchronous, run on current thread
                runnable.run();
            } else {
                // Otherwise, run on provided handler
                handler.post(runnable);
            }
        }
    }

    /**
     * Helper method to create Message instance from handler
     *
     * @param responseMessageId   constant to identify Handler message
     * @param responseMessageData object to be passed to message receiver
     * @return Message instance, should not be null
     */
    protected Message obtainMessage(int responseMessageId, Object responseMessageData) {
        return Message.obtain(handler, responseMessageId, responseMessageData);
    }

    @Override
    public void sendResponseMessage(HttpResponse response) throws IOException {
        // do not process if request has been cancelled
        if (!Thread.currentThread().isInterrupted()) {
            StatusLine status = response.getStatusLine();
            byte[] responseBody;
            responseBody = getResponseData(response.getEntity());
            // additional cancellation check as getResponseData() can take non-zero time to process
            if (!Thread.currentThread().isInterrupted()) {
                if (status.getStatusCode() >= 300) {
                    sendFailureMessage(status.getStatusCode(), response.getAllHeaders(), responseBody, new HttpResponseException(status.getStatusCode(), status.getReasonPhrase()));
                } else {
                    sendSuccessMessage(status.getStatusCode(), response.getAllHeaders(), responseBody);
                }
            }
        }
    }

    /**
     * Returns byte array of response HttpEntity contents
     *
     * @param entity can be null
     * @return response entity body or null
     * @throws java.io.IOException if reading entity or creating byte array failed
     */
    byte[] getResponseData(HttpEntity entity) throws IOException {
        byte[] responseBody = null;
        if (entity != null) {
            InputStream instream = entity.getContent();
            if (instream != null) {
                long contentLength = entity.getContentLength();
                if (contentLength > Integer.MAX_VALUE) {
                    throw new IllegalArgumentException("HTTP entity too large to be buffered in memory");
                }
                int buffersize = (contentLength <= 0) ? BUFFER_SIZE : (int) contentLength;
                try {
                    ByteArrayBuffer buffer = new ByteArrayBuffer(buffersize);
                    try {
                        byte[] tmp = new byte[BUFFER_SIZE];
                        long count = 0;
                        int l;
                        // do not send messages if request has been cancelled
                        while ((l = instream.read(tmp)) != -1 && !Thread.currentThread().isInterrupted()) {
                            count += l;
                            buffer.append(tmp, 0, l);
                            sendProgressMessage(count, (contentLength <= 0 ? 1 : contentLength));
                        }
                    } finally {
                        AsyncHttpClient.silentCloseInputStream(instream);
                        AsyncHttpClient.endEntityViaReflection(entity);
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
}

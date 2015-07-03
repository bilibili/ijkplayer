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

import android.util.Log;

import org.apache.http.HttpResponse;
import org.apache.http.client.HttpRequestRetryHandler;
import org.apache.http.client.methods.HttpUriRequest;
import org.apache.http.impl.client.AbstractHttpClient;
import org.apache.http.protocol.HttpContext;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.UnknownHostException;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Internal class, representing the HttpRequest, done in asynchronous manner
 */
public class AsyncHttpRequest implements Runnable {
    private final AbstractHttpClient client;
    private final HttpContext context;
    private final HttpUriRequest request;
    private final ResponseHandlerInterface responseHandler;
    private int executionCount;
    private final AtomicBoolean isCancelled = new AtomicBoolean();
    private boolean cancelIsNotified;
    private volatile boolean isFinished;
    private boolean isRequestPreProcessed;

    public AsyncHttpRequest(AbstractHttpClient client, HttpContext context, HttpUriRequest request, ResponseHandlerInterface responseHandler) {
        this.client = Utils.notNull(client, "client");
        this.context = Utils.notNull(context, "context");
        this.request = Utils.notNull(request, "request");
        this.responseHandler = Utils.notNull(responseHandler, "responseHandler");
    }

    /**
     * This method is called once by the system when the request is about to be
     * processed by the system. The library makes sure that a single request
     * is pre-processed only once.
     *
     * Please note: pre-processing does NOT run on the main thread, and thus
     * any UI activities that you must perform should be properly dispatched to
     * the app's UI thread.
     *
     * @param request The request to pre-process
     */
    public void onPreProcessRequest(AsyncHttpRequest request) {
        // default action is to do nothing...
    }

    /**
     * This method is called once by the system when the request has been fully
     * sent, handled and finished. The library makes sure that a single request
     * is post-processed only once.
     *
     * Please note: post-processing does NOT run on the main thread, and thus
     * any UI activities that you must perform should be properly dispatched to
     * the app's UI thread.
     *
     * @param request The request to post-process
     */
    public void onPostProcessRequest(AsyncHttpRequest request) {
        // default action is to do nothing...
    }

    @Override
    public void run() {
        if (isCancelled()) {
            return;
        }

        // Carry out pre-processing for this request only once.
        if (!isRequestPreProcessed) {
            isRequestPreProcessed = true;
            onPreProcessRequest(this);
        }

        if (isCancelled()) {
            return;
        }

        responseHandler.sendStartMessage();

        if (isCancelled()) {
            return;
        }

        try {
            makeRequestWithRetries();
        } catch (IOException e) {
            if (!isCancelled()) {
                responseHandler.sendFailureMessage(0, null, null, e);
            } else {
                Log.e("AsyncHttpRequest", "makeRequestWithRetries returned error", e);
            }
        }

        if (isCancelled()) {
            return;
        }

        responseHandler.sendFinishMessage();

        if (isCancelled()) {
            return;
        }

        // Carry out post-processing for this request.
        onPostProcessRequest(this);

        isFinished = true;
    }

    private void makeRequest() throws IOException {
        if (isCancelled()) {
            return;
        }

        // Fixes #115
        if (request.getURI().getScheme() == null) {
            // subclass of IOException so processed in the caller
            throw new MalformedURLException("No valid URI scheme was provided");
        }

        if (responseHandler instanceof RangeFileAsyncHttpResponseHandler) {
            ((RangeFileAsyncHttpResponseHandler) responseHandler).updateRequestHeaders(request);
        }

        HttpResponse response = client.execute(request, context);

        if (isCancelled()) {
            return;
        }

        // Carry out pre-processing for this response.
        responseHandler.onPreProcessResponse(responseHandler, response);

        if (isCancelled()) {
            return;
        }

        // The response is ready, handle it.
        responseHandler.sendResponseMessage(response);

        if (isCancelled()) {
            return;
        }

        // Carry out post-processing for this response.
        responseHandler.onPostProcessResponse(responseHandler, response);
    }

    private void makeRequestWithRetries() throws IOException {
        boolean retry = true;
        IOException cause = null;
        HttpRequestRetryHandler retryHandler = client.getHttpRequestRetryHandler();
        try {
            while (retry) {
                try {
                    makeRequest();
                    return;
                } catch (UnknownHostException e) {
                    // switching between WI-FI and mobile data networks can cause a retry which then results in an UnknownHostException
                    // while the WI-FI is initialising. The retry logic will be invoked here, if this is NOT the first retry
                    // (to assist in genuine cases of unknown host) which seems better than outright failure
                    cause = new IOException("UnknownHostException exception: " + e.getMessage());
                    retry = (executionCount > 0) && retryHandler.retryRequest(e, ++executionCount, context);
                } catch (NullPointerException e) {
                    // there's a bug in HttpClient 4.0.x that on some occasions causes
                    // DefaultRequestExecutor to throw an NPE, see
                    // http://code.google.com/p/android/issues/detail?id=5255
                    cause = new IOException("NPE in HttpClient: " + e.getMessage());
                    retry = retryHandler.retryRequest(cause, ++executionCount, context);
                } catch (IOException e) {
                    if (isCancelled()) {
                        // Eating exception, as the request was cancelled
                        return;
                    }
                    cause = e;
                    retry = retryHandler.retryRequest(cause, ++executionCount, context);
                }
                if (retry) {
                    responseHandler.sendRetryMessage(executionCount);
                }
            }
        } catch (Exception e) {
            // catch anything else to ensure failure message is propagated
            Log.e("AsyncHttpRequest", "Unhandled exception origin cause", e);
            cause = new IOException("Unhandled exception: " + e.getMessage());
        }

        // cleaned up to throw IOException
        throw (cause);
    }

    public boolean isCancelled() {
        boolean cancelled = isCancelled.get();
        if (cancelled) {
            sendCancelNotification();
        }
        return cancelled;
    }

    private synchronized void sendCancelNotification() {
        if (!isFinished && isCancelled.get() && !cancelIsNotified) {
            cancelIsNotified = true;
            responseHandler.sendCancelMessage();
        }
    }

    public boolean isDone() {
        return isCancelled() || isFinished;
    }

    public boolean cancel(boolean mayInterruptIfRunning) {
        isCancelled.set(true);
        request.abort();
        return isCancelled();
    }
}

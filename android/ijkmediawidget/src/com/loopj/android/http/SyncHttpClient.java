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

import android.content.Context;

import org.apache.http.client.methods.HttpUriRequest;
import org.apache.http.conn.scheme.SchemeRegistry;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.protocol.HttpContext;

/**
 * Processes http requests in synchronous mode, so your caller thread will be blocked on each
 * request
 *
 * @see com.loopj.android.http.AsyncHttpClient
 */
public class SyncHttpClient extends AsyncHttpClient {

    /**
     * Creates a new SyncHttpClient with default constructor arguments values
     */
    public SyncHttpClient() {
        super(false, 80, 443);
    }

    /**
     * Creates a new SyncHttpClient.
     *
     * @param httpPort non-standard HTTP-only port
     */
    public SyncHttpClient(int httpPort) {
        super(false, httpPort, 443);
    }

    /**
     * Creates a new SyncHttpClient.
     *
     * @param httpPort  non-standard HTTP-only port
     * @param httpsPort non-standard HTTPS-only port
     */
    public SyncHttpClient(int httpPort, int httpsPort) {
        super(false, httpPort, httpsPort);
    }

    /**
     * Creates new SyncHttpClient using given params
     *
     * @param fixNoHttpResponseException Whether to fix or not issue, by ommiting SSL verification
     * @param httpPort                   HTTP port to be used, must be greater than 0
     * @param httpsPort                  HTTPS port to be used, must be greater than 0
     */
    public SyncHttpClient(boolean fixNoHttpResponseException, int httpPort, int httpsPort) {
        super(fixNoHttpResponseException, httpPort, httpsPort);
    }

    /**
     * Creates a new SyncHttpClient.
     *
     * @param schemeRegistry SchemeRegistry to be used
     */
    public SyncHttpClient(SchemeRegistry schemeRegistry) {
        super(schemeRegistry);
    }

    @Override
    protected RequestHandle sendRequest(DefaultHttpClient client,
                                        HttpContext httpContext, HttpUriRequest uriRequest,
                                        String contentType, ResponseHandlerInterface responseHandler,
                                        Context context) {
        if (contentType != null) {
            uriRequest.addHeader(AsyncHttpClient.HEADER_CONTENT_TYPE, contentType);
        }

        responseHandler.setUseSynchronousMode(true);

		/*
         * will execute the request directly
		*/
        newAsyncHttpRequest(client, httpContext, uriRequest, contentType, responseHandler, context).run();

        // Return a Request Handle that cannot be used to cancel the request
        // because it is already complete by the time this returns
        return new RequestHandle(null);
    }
}

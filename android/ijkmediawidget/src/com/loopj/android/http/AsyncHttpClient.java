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
import android.os.Looper;
import android.util.Log;

import org.apache.http.Header;
import org.apache.http.HeaderElement;
import org.apache.http.HttpEntity;
import org.apache.http.HttpException;
import org.apache.http.HttpHost;
import org.apache.http.HttpRequest;
import org.apache.http.HttpRequestInterceptor;
import org.apache.http.HttpResponse;
import org.apache.http.HttpResponseInterceptor;
import org.apache.http.HttpVersion;
import org.apache.http.auth.AuthScope;
import org.apache.http.auth.AuthState;
import org.apache.http.auth.Credentials;
import org.apache.http.auth.UsernamePasswordCredentials;
import org.apache.http.client.CookieStore;
import org.apache.http.client.CredentialsProvider;
import org.apache.http.client.HttpClient;
import org.apache.http.client.RedirectHandler;
import org.apache.http.client.methods.HttpEntityEnclosingRequestBase;
import org.apache.http.client.methods.HttpHead;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpPut;
import org.apache.http.client.methods.HttpUriRequest;
import org.apache.http.client.params.ClientPNames;
import org.apache.http.client.protocol.ClientContext;
import org.apache.http.conn.ClientConnectionManager;
import org.apache.http.conn.params.ConnManagerParams;
import org.apache.http.conn.params.ConnPerRouteBean;
import org.apache.http.conn.params.ConnRoutePNames;
import org.apache.http.conn.scheme.PlainSocketFactory;
import org.apache.http.conn.scheme.Scheme;
import org.apache.http.conn.scheme.SchemeRegistry;
import org.apache.http.conn.ssl.SSLSocketFactory;
import org.apache.http.entity.HttpEntityWrapper;
import org.apache.http.impl.auth.BasicScheme;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.conn.tsccm.ThreadSafeClientConnManager;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.HttpConnectionParams;
import org.apache.http.params.HttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.apache.http.protocol.BasicHttpContext;
import org.apache.http.protocol.ExecutionContext;
import org.apache.http.protocol.HttpContext;
import org.apache.http.protocol.SyncBasicHttpContext;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PushbackInputStream;
import java.lang.reflect.Field;
import java.net.URI;
import java.net.URL;
import java.net.URLDecoder;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.WeakHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.zip.GZIPInputStream;


/**
 * The AsyncHttpClient can be used to make asynchronous GET, POST, PUT and DELETE HTTP requests in
 * your Android applications. Requests can be made with additional parameters by passing a {@link
 * RequestParams} instance, and responses can be handled by passing an anonymously overridden {@link
 * ResponseHandlerInterface} instance. <p>&nbsp;</p> For example: <p>&nbsp;</p>
 * <pre>
 * AsyncHttpClient client = new AsyncHttpClient();
 * client.get("http://www.google.com", new AsyncHttpResponseHandler() {
 *     &#064;Override
 *     public void onSuccess(int statusCode, Header[] headers, byte[] responseBody) {
 *          System.out.println(response);
 *     }
 *     &#064;Override
 *     public void onFailure(int statusCode, Header[] headers, byte[] responseBody, Throwable
 * error)
 * {
 *          error.printStackTrace(System.out);
 *     }
 * });
 * </pre>
 *
 * @see com.loopj.android.http.AsyncHttpResponseHandler
 * @see com.loopj.android.http.ResponseHandlerInterface
 * @see com.loopj.android.http.RequestParams
 */
public class AsyncHttpClient {

    public static final String LOG_TAG = "AsyncHttpClient";

    public static final String HEADER_CONTENT_TYPE = "Content-Type";
    public static final String HEADER_CONTENT_RANGE = "Content-Range";
    public static final String HEADER_CONTENT_ENCODING = "Content-Encoding";
    public static final String HEADER_CONTENT_DISPOSITION = "Content-Disposition";
    public static final String HEADER_ACCEPT_ENCODING = "Accept-Encoding";
    public static final String ENCODING_GZIP = "gzip";

    public static final int DEFAULT_MAX_CONNECTIONS = 10;
    public static final int DEFAULT_SOCKET_TIMEOUT = 10 * 1000;
    public static final int DEFAULT_MAX_RETRIES = 5;
    public static final int DEFAULT_RETRY_SLEEP_TIME_MILLIS = 1500;
    public static final int DEFAULT_SOCKET_BUFFER_SIZE = 8192;

    private int maxConnections = DEFAULT_MAX_CONNECTIONS;
    private int connectTimeout = DEFAULT_SOCKET_TIMEOUT;
    private int responseTimeout = DEFAULT_SOCKET_TIMEOUT;

    private final DefaultHttpClient httpClient;
    private final HttpContext httpContext;
    private ExecutorService threadPool;
    private final Map<Context, List<RequestHandle>> requestMap;
    private final Map<String, String> clientHeaderMap;
    private boolean isUrlEncodingEnabled = true;

    /**
     * Creates a new AsyncHttpClient with default constructor arguments values
     */
    public AsyncHttpClient() {
        this(false, 80, 443);
    }

    /**
     * Creates a new AsyncHttpClient.
     *
     * @param httpPort non-standard HTTP-only port
     */
    public AsyncHttpClient(int httpPort) {
        this(false, httpPort, 443);
    }

    /**
     * Creates a new AsyncHttpClient.
     *
     * @param httpPort  non-standard HTTP-only port
     * @param httpsPort non-standard HTTPS-only port
     */
    public AsyncHttpClient(int httpPort, int httpsPort) {
        this(false, httpPort, httpsPort);
    }

    /**
     * Creates new AsyncHttpClient using given params
     *
     * @param fixNoHttpResponseException Whether to fix issue or not, by omitting SSL verification
     * @param httpPort                   HTTP port to be used, must be greater than 0
     * @param httpsPort                  HTTPS port to be used, must be greater than 0
     */
    public AsyncHttpClient(boolean fixNoHttpResponseException, int httpPort, int httpsPort) {
        this(getDefaultSchemeRegistry(fixNoHttpResponseException, httpPort, httpsPort));
    }

    /**
     * Returns default instance of SchemeRegistry
     *
     * @param fixNoHttpResponseException Whether to fix issue or not, by omitting SSL verification
     * @param httpPort                   HTTP port to be used, must be greater than 0
     * @param httpsPort                  HTTPS port to be used, must be greater than 0
     */
    private static SchemeRegistry getDefaultSchemeRegistry(boolean fixNoHttpResponseException, int httpPort, int httpsPort) {
        if (fixNoHttpResponseException) {
            Log.d(LOG_TAG, "Beware! Using the fix is insecure, as it doesn't verify SSL certificates.");
        }

        if (httpPort < 1) {
            httpPort = 80;
            Log.d(LOG_TAG, "Invalid HTTP port number specified, defaulting to 80");
        }

        if (httpsPort < 1) {
            httpsPort = 443;
            Log.d(LOG_TAG, "Invalid HTTPS port number specified, defaulting to 443");
        }

        // Fix to SSL flaw in API < ICS
        // See https://code.google.com/p/android/issues/detail?id=13117
        SSLSocketFactory sslSocketFactory;
        if (fixNoHttpResponseException) {
            sslSocketFactory = MySSLSocketFactory.getFixedSocketFactory();
        } else {
            sslSocketFactory = SSLSocketFactory.getSocketFactory();
        }

        SchemeRegistry schemeRegistry = new SchemeRegistry();
        schemeRegistry.register(new Scheme("http", PlainSocketFactory.getSocketFactory(), httpPort));
        schemeRegistry.register(new Scheme("https", sslSocketFactory, httpsPort));

        return schemeRegistry;
    }

    /**
     * Creates a new AsyncHttpClient.
     *
     * @param schemeRegistry SchemeRegistry to be used
     */
    public AsyncHttpClient(SchemeRegistry schemeRegistry) {

        BasicHttpParams httpParams = new BasicHttpParams();

        ConnManagerParams.setTimeout(httpParams, connectTimeout);
        ConnManagerParams.setMaxConnectionsPerRoute(httpParams, new ConnPerRouteBean(maxConnections));
        ConnManagerParams.setMaxTotalConnections(httpParams, DEFAULT_MAX_CONNECTIONS);

        HttpConnectionParams.setSoTimeout(httpParams, responseTimeout);
        HttpConnectionParams.setConnectionTimeout(httpParams, connectTimeout);
        HttpConnectionParams.setTcpNoDelay(httpParams, true);
        HttpConnectionParams.setSocketBufferSize(httpParams, DEFAULT_SOCKET_BUFFER_SIZE);

        HttpProtocolParams.setVersion(httpParams, HttpVersion.HTTP_1_1);

        ClientConnectionManager cm = createConnectionManager(schemeRegistry, httpParams);
        Utils.asserts(cm != null, "Custom implementation of #createConnectionManager(SchemeRegistry, BasicHttpParams) returned null");

        threadPool = getDefaultThreadPool();
        requestMap = Collections.synchronizedMap(new WeakHashMap<Context, List<RequestHandle>>());
        clientHeaderMap = new HashMap<String, String>();

        httpContext = new SyncBasicHttpContext(new BasicHttpContext());
        httpClient = new DefaultHttpClient(cm, httpParams);
        httpClient.addRequestInterceptor(new HttpRequestInterceptor() {
            @Override
            public void process(HttpRequest request, HttpContext context) {
                if (!request.containsHeader(HEADER_ACCEPT_ENCODING)) {
                    request.addHeader(HEADER_ACCEPT_ENCODING, ENCODING_GZIP);
                }
                for (String header : clientHeaderMap.keySet()) {
                    if (request.containsHeader(header)) {
                        Header overwritten = request.getFirstHeader(header);
                        Log.d(LOG_TAG,
                                String.format("Headers were overwritten! (%s | %s) overwrites (%s | %s)",
                                        header, clientHeaderMap.get(header),
                                        overwritten.getName(), overwritten.getValue())
                        );

                        //remove the overwritten header
                        request.removeHeader(overwritten);
                    }
                    request.addHeader(header, clientHeaderMap.get(header));
                }
            }
        });

        httpClient.addResponseInterceptor(new HttpResponseInterceptor() {
            @Override
            public void process(HttpResponse response, HttpContext context) {
                final HttpEntity entity = response.getEntity();
                if (entity == null) {
                    return;
                }
                final Header encoding = entity.getContentEncoding();
                if (encoding != null) {
                    for (HeaderElement element : encoding.getElements()) {
                        if (element.getName().equalsIgnoreCase(ENCODING_GZIP)) {
                            response.setEntity(new InflatingEntity(entity));
                            break;
                        }
                    }
                }
            }
        });

        httpClient.addRequestInterceptor(new HttpRequestInterceptor() {
            @Override
            public void process(final HttpRequest request, final HttpContext context) throws HttpException, IOException {
                AuthState authState = (AuthState) context.getAttribute(ClientContext.TARGET_AUTH_STATE);
                CredentialsProvider credsProvider = (CredentialsProvider) context.getAttribute(
                        ClientContext.CREDS_PROVIDER);
                HttpHost targetHost = (HttpHost) context.getAttribute(ExecutionContext.HTTP_TARGET_HOST);

                if (authState.getAuthScheme() == null) {
                    AuthScope authScope = new AuthScope(targetHost.getHostName(), targetHost.getPort());
                    Credentials creds = credsProvider.getCredentials(authScope);
                    if (creds != null) {
                        authState.setAuthScheme(new BasicScheme());
                        authState.setCredentials(creds);
                    }
                }
            }
        }, 0);

        httpClient.setHttpRequestRetryHandler(new RetryHandler(DEFAULT_MAX_RETRIES, DEFAULT_RETRY_SLEEP_TIME_MILLIS));
    }

    public static void allowRetryExceptionClass(Class<?> cls) {
        if (cls != null) {
            RetryHandler.addClassToWhitelist(cls);
        }
    }

    public static void blockRetryExceptionClass(Class<?> cls) {
        if (cls != null) {
            RetryHandler.addClassToBlacklist(cls);
        }
    }

    /**
     * Get the underlying HttpClient instance. This is useful for setting additional fine-grained
     * settings for requests by accessing the client's ConnectionManager, HttpParams and
     * SchemeRegistry.
     *
     * @return underlying HttpClient instance
     */
    public HttpClient getHttpClient() {
        return this.httpClient;
    }

    /**
     * Get the underlying HttpContext instance. This is useful for getting and setting fine-grained
     * settings for requests by accessing the context's attributes such as the CookieStore.
     *
     * @return underlying HttpContext instance
     */
    public HttpContext getHttpContext() {
        return this.httpContext;
    }

    /**
     * Sets an optional CookieStore to use when making requests
     *
     * @param cookieStore The CookieStore implementation to use, usually an instance of {@link
     *                    PersistentCookieStore}
     */
    public void setCookieStore(CookieStore cookieStore) {
        httpContext.setAttribute(ClientContext.COOKIE_STORE, cookieStore);
    }

    /**
     * Overrides the threadpool implementation used when queuing/pooling requests. By default,
     * Executors.newCachedThreadPool() is used.
     *
     * @param threadPool an instance of {@link ExecutorService} to use for queuing/pooling
     *                   requests.
     */
    public void setThreadPool(ExecutorService threadPool) {
        this.threadPool = threadPool;
    }

    /**
     * Returns the current executor service used. By default, Executors.newCachedThreadPool() is
     * used.
     *
     * @return current executor service used
     */
    public ExecutorService getThreadPool() {
        return threadPool;
    }

    /**
     * Get the default threading pool to be used for this HTTP client.
     *
     * @return The default threading pool to be used
     */
    protected ExecutorService getDefaultThreadPool() {
        return Executors.newCachedThreadPool();
    }

    /**
     * Provided so it is easier for developers to provide custom ThreadSafeClientConnManager implementation
     *
     * @param schemeRegistry SchemeRegistry, usually provided by {@link #getDefaultSchemeRegistry(boolean, int, int)}
     * @param httpParams     BasicHttpParams
     * @return ClientConnectionManager instance
     */
    protected ClientConnectionManager createConnectionManager(SchemeRegistry schemeRegistry, BasicHttpParams httpParams) {
        return new ThreadSafeClientConnManager(httpParams, schemeRegistry);
    }

    /**
     * Simple interface method, to enable or disable redirects. If you set manually RedirectHandler
     * on underlying HttpClient, effects of this method will be canceled. <p>&nbsp;</p> Default
     * setting is to disallow redirects.
     *
     * @param enableRedirects         boolean
     * @param enableRelativeRedirects boolean
     * @param enableCircularRedirects boolean
     */
    public void setEnableRedirects(final boolean enableRedirects, final boolean enableRelativeRedirects, final boolean enableCircularRedirects) {
        httpClient.getParams().setBooleanParameter(ClientPNames.REJECT_RELATIVE_REDIRECT, !enableRelativeRedirects);
        httpClient.getParams().setBooleanParameter(ClientPNames.ALLOW_CIRCULAR_REDIRECTS, enableCircularRedirects);
        httpClient.setRedirectHandler(new MyRedirectHandler(enableRedirects));
    }

    /**
     * Circular redirects are enabled by default
     *
     * @param enableRedirects         boolean
     * @param enableRelativeRedirects boolean
     * @see #setEnableRedirects(boolean, boolean, boolean)
     */
    public void setEnableRedirects(final boolean enableRedirects, final boolean enableRelativeRedirects) {
        setEnableRedirects(enableRedirects, enableRelativeRedirects, true);
    }

    /**
     * @param enableRedirects boolean
     * @see #setEnableRedirects(boolean, boolean, boolean)
     */
    public void setEnableRedirects(final boolean enableRedirects) {
        setEnableRedirects(enableRedirects, enableRedirects, enableRedirects);
    }

    /**
     * Allows you to set custom RedirectHandler implementation, if the default provided doesn't suit
     * your needs
     *
     * @param customRedirectHandler RedirectHandler instance
     * @see com.loopj.android.http.MyRedirectHandler
     */
    public void setRedirectHandler(final RedirectHandler customRedirectHandler) {
        httpClient.setRedirectHandler(customRedirectHandler);
    }

    /**
     * Sets the User-Agent header to be sent with each request. By default, "Android Asynchronous
     * Http Client/VERSION (http://loopj.com/android-async-http/)" is used.
     *
     * @param userAgent the string to use in the User-Agent header.
     */
    public void setUserAgent(String userAgent) {
        HttpProtocolParams.setUserAgent(this.httpClient.getParams(), userAgent);
    }


    /**
     * Returns current limit of parallel connections
     *
     * @return maximum limit of parallel connections, default is 10
     */
    public int getMaxConnections() {
        return maxConnections;
    }

    /**
     * Sets maximum limit of parallel connections
     *
     * @param maxConnections maximum parallel connections, must be at least 1
     */
    public void setMaxConnections(int maxConnections) {
        if (maxConnections < 1)
            maxConnections = DEFAULT_MAX_CONNECTIONS;
        this.maxConnections = maxConnections;
        final HttpParams httpParams = this.httpClient.getParams();
        ConnManagerParams.setMaxConnectionsPerRoute(httpParams, new ConnPerRouteBean(this.maxConnections));
    }

    /**
     * Returns current socket timeout limit (milliseconds). By default, this is
     * set to 10 seconds.
     *
     * @return Socket Timeout limit in milliseconds
     * @deprecated Use either {@link #getConnectTimeout()} or {@link #getResponseTimeout()}
     */
    public int getTimeout() {
        return connectTimeout;
    }

    /**
     * Set both the connection and socket timeouts. By default, both are set to
     * 10 seconds.
     *
     * @param value the connect/socket timeout in milliseconds, at least 1 second
     * @see #setConnectTimeout(int)
     * @see #setResponseTimeout(int)
     */
    public void setTimeout(int value) {
        value = value < 1000 ? DEFAULT_SOCKET_TIMEOUT : value;
        setConnectTimeout(value);
        setResponseTimeout(value);
    }

    /**
     * Returns current connection timeout limit (milliseconds). By default, this
     * is set to 10 seconds.
     *
     * @return Connection timeout limit in milliseconds
     */
    public int getConnectTimeout() {
        return connectTimeout;
    }

    /**
     * Set connection timeout limit (milliseconds). By default, this is set to
     * 10 seconds.
     *
     * @param value Connection timeout in milliseconds, minimal value is 1000 (1 second).
     */
    public void setConnectTimeout(int value) {
        connectTimeout = value < 1000 ? DEFAULT_SOCKET_TIMEOUT : value;
        final HttpParams httpParams = httpClient.getParams();
        ConnManagerParams.setTimeout(httpParams, connectTimeout);
        HttpConnectionParams.setConnectionTimeout(httpParams, connectTimeout);
    }

    /**
     * Returns current response timeout limit (milliseconds). By default, this
     * is set to 10 seconds.
     *
     * @return Response timeout limit in milliseconds
     */
    public int getResponseTimeout() {
        return responseTimeout;
    }

    /**
     * Set response timeout limit (milliseconds). By default, this is set to
     * 10 seconds.
     *
     * @param value Response timeout in milliseconds, minimal value is 1000 (1 second).
     */
    public void setResponseTimeout(int value) {
        responseTimeout = value < 1000 ? DEFAULT_SOCKET_TIMEOUT : value;
        final HttpParams httpParams = httpClient.getParams();
        HttpConnectionParams.setSoTimeout(httpParams, responseTimeout);
    }

    /**
     * Sets the Proxy by it's hostname and port
     *
     * @param hostname the hostname (IP or DNS name)
     * @param port     the port number. -1 indicates the scheme default port.
     */
    public void setProxy(String hostname, int port) {
        final HttpHost proxy = new HttpHost(hostname, port);
        final HttpParams httpParams = this.httpClient.getParams();
        httpParams.setParameter(ConnRoutePNames.DEFAULT_PROXY, proxy);
    }

    /**
     * Sets the Proxy by it's hostname,port,username and password
     *
     * @param hostname the hostname (IP or DNS name)
     * @param port     the port number. -1 indicates the scheme default port.
     * @param username the username
     * @param password the password
     */
    public void setProxy(String hostname, int port, String username, String password) {
        httpClient.getCredentialsProvider().setCredentials(
                new AuthScope(hostname, port),
                new UsernamePasswordCredentials(username, password));
        final HttpHost proxy = new HttpHost(hostname, port);
        final HttpParams httpParams = this.httpClient.getParams();
        httpParams.setParameter(ConnRoutePNames.DEFAULT_PROXY, proxy);
    }

    /**
     * Sets the SSLSocketFactory to user when making requests. By default, a new, default
     * SSLSocketFactory is used.
     *
     * @param sslSocketFactory the socket factory to use for https requests.
     */
    public void setSSLSocketFactory(SSLSocketFactory sslSocketFactory) {
        this.httpClient.getConnectionManager().getSchemeRegistry().register(new Scheme("https", sslSocketFactory, 443));
    }

    /**
     * Sets the maximum number of retries and timeout for a particular Request.
     *
     * @param retries maximum number of retries per request
     * @param timeout sleep between retries in milliseconds
     */
    public void setMaxRetriesAndTimeout(int retries, int timeout) {
        this.httpClient.setHttpRequestRetryHandler(new RetryHandler(retries, timeout));
    }

    /**
     * Will, before sending, remove all headers currently present in AsyncHttpClient instance, which
     * applies on all requests this client makes
     */
    public void removeAllHeaders() {
        clientHeaderMap.clear();
    }

    /**
     * Sets headers that will be added to all requests this client makes (before sending).
     *
     * @param header the name of the header
     * @param value  the contents of the header
     */
    public void addHeader(String header, String value) {
        clientHeaderMap.put(header, value);
    }

    /**
     * Remove header from all requests this client makes (before sending).
     *
     * @param header the name of the header
     */
    public void removeHeader(String header) {
        clientHeaderMap.remove(header);
    }

    /**
     * Sets basic authentication for the request. Uses AuthScope.ANY. This is the same as
     * setBasicAuth('username','password',AuthScope.ANY)
     *
     * @param username Basic Auth username
     * @param password Basic Auth password
     */
    public void setBasicAuth(String username, String password) {
        setBasicAuth(username, password, false);
    }

    /**
     * Sets basic authentication for the request. Uses AuthScope.ANY. This is the same as
     * setBasicAuth('username','password',AuthScope.ANY)
     *
     * @param username   Basic Auth username
     * @param password   Basic Auth password
     * @param preemptive sets authorization in preemptive manner
     */
    public void setBasicAuth(String username, String password, boolean preemptive) {
        setBasicAuth(username, password, null, preemptive);
    }

    /**
     * Sets basic authentication for the request. You should pass in your AuthScope for security. It
     * should be like this setBasicAuth("username","password", new AuthScope("host",port,AuthScope.ANY_REALM))
     *
     * @param username Basic Auth username
     * @param password Basic Auth password
     * @param scope    - an AuthScope object
     */
    public void setBasicAuth(String username, String password, AuthScope scope) {
        setBasicAuth(username, password, scope, false);
    }

    /**
     * Sets basic authentication for the request. You should pass in your AuthScope for security. It
     * should be like this setBasicAuth("username","password", new AuthScope("host",port,AuthScope.ANY_REALM))
     *
     * @param username   Basic Auth username
     * @param password   Basic Auth password
     * @param scope      an AuthScope object
     * @param preemptive sets authorization in preemptive manner
     */
    public void setBasicAuth(String username, String password, AuthScope scope, boolean preemptive) {
        UsernamePasswordCredentials credentials = new UsernamePasswordCredentials(username, password);
        setCredentials(scope, credentials);
        setAuthenticationPreemptive(preemptive);
    }

    public void setCredentials(AuthScope authScope, Credentials credentials) {
        if (credentials == null) {
            Log.d(LOG_TAG, "Provided credentials are null, not setting");
            return;
        }
        this.httpClient.getCredentialsProvider().setCredentials(authScope == null ? AuthScope.ANY : authScope, credentials);
    }

    /**
     * Sets HttpRequestInterceptor which handles authorization in preemptive way, as workaround you
     * can use call `AsyncHttpClient.addHeader("Authorization","Basic base64OfUsernameAndPassword==")`
     *
     * @param isPreemptive whether the authorization is processed in preemptive way
     */
    public void setAuthenticationPreemptive(boolean isPreemptive) {
        if (isPreemptive) {
            httpClient.addRequestInterceptor(new PreemptiveAuthorizationHttpRequestInterceptor(), 0);
        } else {
            httpClient.removeRequestInterceptorByClass(PreemptiveAuthorizationHttpRequestInterceptor.class);
        }
    }

    /**
     * Removes previously set basic auth credentials
     *
     * @deprecated
     */
    @Deprecated
    public void clearBasicAuth() {
        clearCredentialsProvider();
    }

    /**
     * Removes previously set auth credentials
     */
    public void clearCredentialsProvider() {
        this.httpClient.getCredentialsProvider().clear();
    }

    /**
     * Cancels any pending (or potentially active) requests associated with the passed Context.
     * <p>&nbsp;</p> <b>Note:</b> This will only affect requests which were created with a non-null
     * android Context. This method is intended to be used in the onDestroy method of your android
     * activities to destroy all requests which are no longer required.
     *
     * @param context               the android Context instance associated to the request.
     * @param mayInterruptIfRunning specifies if active requests should be cancelled along with
     *                              pending requests.
     */
    public void cancelRequests(final Context context, final boolean mayInterruptIfRunning) {
        if (context == null) {
            Log.e(LOG_TAG, "Passed null Context to cancelRequests");
            return;
        }

        final List<RequestHandle> requestList = requestMap.get(context);
        requestMap.remove(context);

        if (Looper.myLooper() == Looper.getMainLooper()) {
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    cancelRequests(requestList, mayInterruptIfRunning);
                }
            };
            threadPool.submit(runnable);
        } else {
            cancelRequests(requestList, mayInterruptIfRunning);
        }
    }

    private void cancelRequests(final List<RequestHandle> requestList, final boolean mayInterruptIfRunning) {
        if (requestList != null) {
            for (RequestHandle requestHandle : requestList) {
                requestHandle.cancel(mayInterruptIfRunning);
            }
        }
    }

    /**
     * Cancels all pending (or potentially active) requests. <p>&nbsp;</p> <b>Note:</b> This will
     * only affect requests which were created with a non-null android Context. This method is
     * intended to be used in the onDestroy method of your android activities to destroy all
     * requests which are no longer required.
     *
     * @param mayInterruptIfRunning specifies if active requests should be cancelled along with
     *                              pending requests.
     */
    public void cancelAllRequests(boolean mayInterruptIfRunning) {
        for (List<RequestHandle> requestList : requestMap.values()) {
            if (requestList != null) {
                for (RequestHandle requestHandle : requestList) {
                    requestHandle.cancel(mayInterruptIfRunning);
                }
            }
        }
        requestMap.clear();
    }

    // [+] HTTP HEAD

    /**
     * Perform a HTTP HEAD request, without any parameters.
     *
     * @param url             the URL to send the request to.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle head(String url, ResponseHandlerInterface responseHandler) {
        return head(null, url, null, responseHandler);
    }

    /**
     * Perform a HTTP HEAD request with parameters.
     *
     * @param url             the URL to send the request to.
     * @param params          additional HEAD parameters to send with the request.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle head(String url, RequestParams params, ResponseHandlerInterface responseHandler) {
        return head(null, url, params, responseHandler);
    }

    /**
     * Perform a HTTP HEAD request without any parameters and track the Android Context which
     * initiated the request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle head(Context context, String url, ResponseHandlerInterface responseHandler) {
        return head(context, url, null, responseHandler);
    }

    /**
     * Perform a HTTP HEAD request and track the Android Context which initiated the request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param params          additional HEAD parameters to send with the request.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle head(Context context, String url, RequestParams params, ResponseHandlerInterface responseHandler) {
        return sendRequest(httpClient, httpContext, new HttpHead(getUrlWithQueryString(isUrlEncodingEnabled, url, params)), null, responseHandler, context);
    }

    /**
     * Perform a HTTP HEAD request and track the Android Context which initiated the request with
     * customized headers
     *
     * @param context         Context to execute request against
     * @param url             the URL to send the request to.
     * @param headers         set headers only for this request
     * @param params          additional HEAD parameters to send with the request.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle head(Context context, String url, Header[] headers, RequestParams params, ResponseHandlerInterface responseHandler) {
        HttpUriRequest request = new HttpHead(getUrlWithQueryString(isUrlEncodingEnabled, url, params));
        if (headers != null) request.setHeaders(headers);
        return sendRequest(httpClient, httpContext, request, null, responseHandler,
                context);
    }

    // [-] HTTP HEAD
    // [+] HTTP GET

    /**
     * Perform a HTTP GET request, without any parameters.
     *
     * @param url             the URL to send the request to.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle get(String url, ResponseHandlerInterface responseHandler) {
        return get(null, url, null, responseHandler);
    }

    /**
     * Perform a HTTP GET request with parameters.
     *
     * @param url             the URL to send the request to.
     * @param params          additional GET parameters to send with the request.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle get(String url, RequestParams params, ResponseHandlerInterface responseHandler) {
        return get(null, url, params, responseHandler);
    }

    /**
     * Perform a HTTP GET request without any parameters and track the Android Context which
     * initiated the request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle get(Context context, String url, ResponseHandlerInterface responseHandler) {
        return get(context, url, null, responseHandler);
    }

    /**
     * Perform a HTTP GET request and track the Android Context which initiated the request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param params          additional GET parameters to send with the request.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle get(Context context, String url, RequestParams params, ResponseHandlerInterface responseHandler) {
        return sendRequest(httpClient, httpContext, new HttpGet(getUrlWithQueryString(isUrlEncodingEnabled, url, params)), null, responseHandler, context);
    }

    /**
     * Perform a HTTP GET request and track the Android Context which initiated the request with
     * customized headers
     *
     * @param context         Context to execute request against
     * @param url             the URL to send the request to.
     * @param headers         set headers only for this request
     * @param params          additional GET parameters to send with the request.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle get(Context context, String url, Header[] headers, RequestParams params, ResponseHandlerInterface responseHandler) {
        HttpUriRequest request = new HttpGet(getUrlWithQueryString(isUrlEncodingEnabled, url, params));
        if (headers != null) request.setHeaders(headers);
        return sendRequest(httpClient, httpContext, request, null, responseHandler,
                context);
    }

    /**
     * Perform a HTTP GET request and track the Android Context which initiated the request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param entity          a raw {@link org.apache.http.HttpEntity} to send with the request, for
     *                        example, use this to send string/json/xml payloads to a server by
     *                        passing a {@link org.apache.http.entity.StringEntity}.
     * @param contentType     the content type of the payload you are sending, for example
     *                        application/json if sending a json payload.
     * @param responseHandler the response ha   ndler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle get(Context context, String url, HttpEntity entity, String contentType, ResponseHandlerInterface responseHandler) {
        return sendRequest(httpClient, httpContext, addEntityToRequestBase(new HttpGet(URI.create(url).normalize()), entity), contentType, responseHandler, context);
    }

    // [-] HTTP GET
    // [+] HTTP POST

    /**
     * Perform a HTTP POST request, without any parameters.
     *
     * @param url             the URL to send the request to.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle post(String url, ResponseHandlerInterface responseHandler) {
        return post(null, url, null, responseHandler);
    }

    /**
     * Perform a HTTP POST request with parameters.
     *
     * @param url             the URL to send the request to.
     * @param params          additional POST parameters or files to send with the request.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle post(String url, RequestParams params, ResponseHandlerInterface responseHandler) {
        return post(null, url, params, responseHandler);
    }

    /**
     * Perform a HTTP POST request and track the Android Context which initiated the request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param params          additional POST parameters or files to send with the request.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle post(Context context, String url, RequestParams params, ResponseHandlerInterface responseHandler) {
        return post(context, url, paramsToEntity(params, responseHandler), null, responseHandler);
    }

    /**
     * Perform a HTTP POST request and track the Android Context which initiated the request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param entity          a raw {@link org.apache.http.HttpEntity} to send with the request, for
     *                        example, use this to send string/json/xml payloads to a server by
     *                        passing a {@link org.apache.http.entity.StringEntity}.
     * @param contentType     the content type of the payload you are sending, for example
     *                        application/json if sending a json payload.
     * @param responseHandler the response ha   ndler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle post(Context context, String url, HttpEntity entity, String contentType, ResponseHandlerInterface responseHandler) {
        return sendRequest(httpClient, httpContext, addEntityToRequestBase(new HttpPost(getURI(url)), entity), contentType, responseHandler, context);
    }

    /**
     * Perform a HTTP POST request and track the Android Context which initiated the request. Set
     * headers only for this request
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param headers         set headers only for this request
     * @param params          additional POST parameters to send with the request.
     * @param contentType     the content type of the payload you are sending, for example
     *                        application/json if sending a json payload.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle post(Context context, String url, Header[] headers, RequestParams params, String contentType,
                              ResponseHandlerInterface responseHandler) {
        HttpEntityEnclosingRequestBase request = new HttpPost(getURI(url));
        if (params != null) request.setEntity(paramsToEntity(params, responseHandler));
        if (headers != null) request.setHeaders(headers);
        return sendRequest(httpClient, httpContext, request, contentType,
                responseHandler, context);
    }

    /**
     * Perform a HTTP POST request and track the Android Context which initiated the request. Set
     * headers only for this request
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param headers         set headers only for this request
     * @param entity          a raw {@link HttpEntity} to send with the request, for example, use
     *                        this to send string/json/xml payloads to a server by passing a {@link
     *                        org.apache.http.entity.StringEntity}.
     * @param contentType     the content type of the payload you are sending, for example
     *                        application/json if sending a json payload.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle post(Context context, String url, Header[] headers, HttpEntity entity, String contentType,
                              ResponseHandlerInterface responseHandler) {
        HttpEntityEnclosingRequestBase request = addEntityToRequestBase(new HttpPost(getURI(url)), entity);
        if (headers != null) request.setHeaders(headers);
        return sendRequest(httpClient, httpContext, request, contentType, responseHandler, context);
    }

    // [-] HTTP POST
    // [+] HTTP PUT

    /**
     * Perform a HTTP PUT request, without any parameters.
     *
     * @param url             the URL to send the request to.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle put(String url, ResponseHandlerInterface responseHandler) {
        return put(null, url, null, responseHandler);
    }

    /**
     * Perform a HTTP PUT request with parameters.
     *
     * @param url             the URL to send the request to.
     * @param params          additional PUT parameters or files to send with the request.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle put(String url, RequestParams params, ResponseHandlerInterface responseHandler) {
        return put(null, url, params, responseHandler);
    }

    /**
     * Perform a HTTP PUT request and track the Android Context which initiated the request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param params          additional PUT parameters or files to send with the request.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle put(Context context, String url, RequestParams params, ResponseHandlerInterface responseHandler) {
        return put(context, url, paramsToEntity(params, responseHandler), null, responseHandler);
    }

    /**
     * Perform a HTTP PUT request and track the Android Context which initiated the request. And set
     * one-time headers for the request
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param entity          a raw {@link HttpEntity} to send with the request, for example, use
     *                        this to send string/json/xml payloads to a server by passing a {@link
     *                        org.apache.http.entity.StringEntity}.
     * @param contentType     the content type of the payload you are sending, for example
     *                        application/json if sending a json payload.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle put(Context context, String url, HttpEntity entity, String contentType, ResponseHandlerInterface responseHandler) {
        return sendRequest(httpClient, httpContext, addEntityToRequestBase(new HttpPut(getURI(url)), entity), contentType, responseHandler, context);
    }

    /**
     * Perform a HTTP PUT request and track the Android Context which initiated the request. And set
     * one-time headers for the request
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param headers         set one-time headers for this request
     * @param entity          a raw {@link HttpEntity} to send with the request, for example, use
     *                        this to send string/json/xml payloads to a server by passing a {@link
     *                        org.apache.http.entity.StringEntity}.
     * @param contentType     the content type of the payload you are sending, for example
     *                        application/json if sending a json payload.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle put(Context context, String url, Header[] headers, HttpEntity entity, String contentType, ResponseHandlerInterface responseHandler) {
        HttpEntityEnclosingRequestBase request = addEntityToRequestBase(new HttpPut(getURI(url)), entity);
        if (headers != null) request.setHeaders(headers);
        return sendRequest(httpClient, httpContext, request, contentType, responseHandler, context);
    }

    /**
     * Perform a HTTP
     * request, without any parameters.
     *
     * @param url             the URL to send the request to.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle patch(String url, ResponseHandlerInterface responseHandler) {
        return patch(null, url, null, responseHandler);
    }

    /**
     * Perform a HTTP PATCH request with parameters.
     *
     * @param url             the URL to send the request to.
     * @param params          additional PUT parameters or files to send with the request.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle patch(String url, RequestParams params, ResponseHandlerInterface responseHandler) {
        return patch(null, url, params, responseHandler);
    }

    /**
     * Perform a HTTP PATCH request and track the Android Context which initiated the request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param params          additional PUT parameters or files to send with the request.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle patch(Context context, String url, RequestParams params, ResponseHandlerInterface responseHandler) {
        return patch(context, url, paramsToEntity(params, responseHandler), null, responseHandler);
    }

    /**
     * Perform a HTTP PATCH request and track the Android Context which initiated the request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param responseHandler the response handler instance that should handle the response.
     * @param entity          a raw {@link HttpEntity} to send with the request, for example, use
     *                        this to send string/json/xml payloads to a server by passing a {@link
     *                        org.apache.http.entity.StringEntity}
     * @param contentType     the content type of the payload you are sending, for example
     *                        "application/json" if sending a json payload.
     * @return RequestHandle of future request process
     */
    public RequestHandle patch(Context context, String url, HttpEntity entity, String contentType, ResponseHandlerInterface responseHandler) {
        return sendRequest(httpClient, httpContext, addEntityToRequestBase(new HttpPatch(getURI(url)), entity), contentType, responseHandler, context);
    }

    /**
     * Perform a HTTP PATCH request and track the Android Context which initiated the request. And set
     * one-time headers for the request
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param headers         set one-time headers for this request
     * @param entity          a raw {@link HttpEntity} to send with the request, for example, use
     *                        this to send string/json/xml payloads to a server by passing a {@link
     *                        org.apache.http.entity.StringEntity}.
     * @param contentType     the content type of the payload you are sending, for example
     *                        application/json if sending a json payload.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle patch(Context context, String url, Header[] headers, HttpEntity entity, String contentType, ResponseHandlerInterface responseHandler) {
        HttpEntityEnclosingRequestBase request = addEntityToRequestBase(new HttpPatch(getURI(url)), entity);
        if (headers != null) request.setHeaders(headers);
        return sendRequest(httpClient, httpContext, request, contentType, responseHandler, context);
    }

    // [-] HTTP PUT
    // [+] HTTP DELETE

    /**
     * Perform a HTTP DELETE request.
     *
     * @param url             the URL to send the request to.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle delete(String url, ResponseHandlerInterface responseHandler) {
        return delete(null, url, responseHandler);
    }

    /**
     * Perform a HTTP DELETE request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle delete(Context context, String url, ResponseHandlerInterface responseHandler) {
        final HttpDelete delete = new HttpDelete(getURI(url));
        return sendRequest(httpClient, httpContext, delete, null, responseHandler, context);
    }

    /**
     * Perform a HTTP DELETE request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param headers         set one-time headers for this request
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle delete(Context context, String url, Header[] headers, ResponseHandlerInterface responseHandler) {
        final HttpDelete delete = new HttpDelete(getURI(url));
        if (headers != null) delete.setHeaders(headers);
        return sendRequest(httpClient, httpContext, delete, null, responseHandler, context);
    }

    /**
     * Perform a HTTP DELETE request.
     *
     * @param url             the URL to send the request to.
     * @param params          additional DELETE parameters or files to send with the request.
     * @param responseHandler the response handler instance that should handle the response.
     */
    public void delete(String url, RequestParams params, AsyncHttpResponseHandler responseHandler) {
        final HttpDelete delete = new HttpDelete(getUrlWithQueryString(isUrlEncodingEnabled, url, params));
        sendRequest(httpClient, httpContext, delete, null, responseHandler, null);
    }

    /**
     * Perform a HTTP DELETE request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param headers         set one-time headers for this request
     * @param params          additional DELETE parameters or files to send along with request
     * @param responseHandler the response handler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle delete(Context context, String url, Header[] headers, RequestParams params, ResponseHandlerInterface responseHandler) {
        HttpDelete httpDelete = new HttpDelete(getUrlWithQueryString(isUrlEncodingEnabled, url, params));
        if (headers != null) httpDelete.setHeaders(headers);
        return sendRequest(httpClient, httpContext, httpDelete, null, responseHandler, context);
    }

    /**
     * Perform a HTTP DELETE request and track the Android Context which initiated the request.
     *
     * @param context         the Android Context which initiated the request.
     * @param url             the URL to send the request to.
     * @param entity          a raw {@link org.apache.http.HttpEntity} to send with the request, for
     *                        example, use this to send string/json/xml payloads to a server by
     *                        passing a {@link org.apache.http.entity.StringEntity}.
     * @param contentType     the content type of the payload you are sending, for example
     *                        application/json if sending a json payload.
     * @param responseHandler the response ha   ndler instance that should handle the response.
     * @return RequestHandle of future request process
     */
    public RequestHandle delete(Context context, String url, HttpEntity entity, String contentType, ResponseHandlerInterface responseHandler) {
        return sendRequest(httpClient, httpContext, addEntityToRequestBase(new HttpDelete(URI.create(url).normalize()), entity), contentType, responseHandler, context);
    }

    // [-] HTTP DELETE

    /**
     * Instantiate a new asynchronous HTTP request for the passed parameters.
     *
     * @param client          HttpClient to be used for request, can differ in single requests
     * @param contentType     MIME body type, for POST and PUT requests, may be null
     * @param context         Context of Android application, to hold the reference of request
     * @param httpContext     HttpContext in which the request will be executed
     * @param responseHandler ResponseHandler or its subclass to put the response into
     * @param uriRequest      instance of HttpUriRequest, which means it must be of HttpDelete,
     *                        HttpPost, HttpGet, HttpPut, etc.
     * @return AsyncHttpRequest ready to be dispatched
     */
    protected AsyncHttpRequest newAsyncHttpRequest(DefaultHttpClient client, HttpContext httpContext, HttpUriRequest uriRequest, String contentType, ResponseHandlerInterface responseHandler, Context context) {
        return new AsyncHttpRequest(client, httpContext, uriRequest, responseHandler);
    }

    /**
     * Puts a new request in queue as a new thread in pool to be executed
     *
     * @param client          HttpClient to be used for request, can differ in single requests
     * @param contentType     MIME body type, for POST and PUT requests, may be null
     * @param context         Context of Android application, to hold the reference of request
     * @param httpContext     HttpContext in which the request will be executed
     * @param responseHandler ResponseHandler or its subclass to put the response into
     * @param uriRequest      instance of HttpUriRequest, which means it must be of HttpDelete,
     *                        HttpPost, HttpGet, HttpPut, etc.
     * @return RequestHandle of future request process
     */
    protected RequestHandle sendRequest(DefaultHttpClient client, HttpContext httpContext, HttpUriRequest uriRequest, String contentType, ResponseHandlerInterface responseHandler, Context context) {
        if (uriRequest == null) {
            throw new IllegalArgumentException("HttpUriRequest must not be null");
        }

        if (responseHandler == null) {
            throw new IllegalArgumentException("ResponseHandler must not be null");
        }

        if (responseHandler.getUseSynchronousMode() && !responseHandler.getUsePoolThread()) {
            throw new IllegalArgumentException("Synchronous ResponseHandler used in AsyncHttpClient. You should create your response handler in a looper thread or use SyncHttpClient instead.");
        }
        
        if (contentType != null) {
            if (uriRequest instanceof HttpEntityEnclosingRequestBase && ((HttpEntityEnclosingRequestBase) uriRequest).getEntity() != null && uriRequest.containsHeader(HEADER_CONTENT_TYPE)) {
                Log.w(LOG_TAG, "Passed contentType will be ignored because HttpEntity sets content type");
            } else {
                uriRequest.setHeader(HEADER_CONTENT_TYPE, contentType);
            }
        }

        responseHandler.setRequestHeaders(uriRequest.getAllHeaders());
        responseHandler.setRequestURI(uriRequest.getURI());

        AsyncHttpRequest request = newAsyncHttpRequest(client, httpContext, uriRequest, contentType, responseHandler, context);
        threadPool.submit(request);
        RequestHandle requestHandle = new RequestHandle(request);

        if (context != null) {
            List<RequestHandle> requestList;
            // Add request to request map
            synchronized (requestMap) {
                requestList = requestMap.get(context);
                if (requestList == null) {
                    requestList = Collections.synchronizedList(new LinkedList<RequestHandle>());
                    requestMap.put(context, requestList);
                }
            }

            requestList.add(requestHandle);

            Iterator<RequestHandle> iterator = requestList.iterator();
            while (iterator.hasNext()) {
                if (iterator.next().shouldBeGarbageCollected()) {
                    iterator.remove();
                }
            }
        }

        return requestHandle;
    }

    /**
     * Returns a {@link URI} instance for the specified, absolute URL string.
     *
     * @param url absolute URL string, containing scheme, host and path
     * @return URI instance for the URL string
     */
    protected URI getURI(String url) {
        return URI.create(url).normalize();
    }

    /**
     * Sets state of URL encoding feature, see bug #227, this method allows you to turn off and on
     * this auto-magic feature on-demand.
     *
     * @param enabled desired state of feature
     */
    public void setURLEncodingEnabled(boolean enabled) {
        this.isUrlEncodingEnabled = enabled;
    }

    /**
     * Will encode url, if not disabled, and adds params on the end of it
     *
     * @param url             String with URL, should be valid URL without params
     * @param params          RequestParams to be appended on the end of URL
     * @param shouldEncodeUrl whether url should be encoded (replaces spaces with %20)
     * @return encoded url if requested with params appended if any available
     */
    public static String getUrlWithQueryString(boolean shouldEncodeUrl, String url, RequestParams params) {
        if (url == null)
            return null;

        if (shouldEncodeUrl) {
            try {
                String decodedURL = URLDecoder.decode(url, "UTF-8");
                URL _url = new URL(decodedURL);
                URI _uri = new URI(_url.getProtocol(), _url.getUserInfo(), _url.getHost(), _url.getPort(), _url.getPath(), _url.getQuery(), _url.getRef());
                url = _uri.toASCIIString();
            } catch (Exception ex) {
                // Should not really happen, added just for sake of validity
                Log.e(LOG_TAG, "getUrlWithQueryString encoding URL", ex);
            }
        }

        if (params != null) {
            // Construct the query string and trim it, in case it
            // includes any excessive white spaces.
            String paramString = params.getParamString().trim();

            // Only add the query string if it isn't empty and it
            // isn't equal to '?'.
            if (!paramString.equals("") && !paramString.equals("?")) {
                url += url.contains("?") ? "&" : "?";
                url += paramString;
            }
        }

        return url;
    }

    /**
     * Checks the InputStream if it contains  GZIP compressed data
     *
     * @param inputStream InputStream to be checked
     * @return true or false if the stream contains GZIP compressed data
     * @throws java.io.IOException if read from inputStream fails
     */
    public static boolean isInputStreamGZIPCompressed(final PushbackInputStream inputStream) throws IOException {
        if (inputStream == null)
            return false;

        byte[] signature = new byte[2];
        int readStatus = inputStream.read(signature);
        inputStream.unread(signature);
        int streamHeader = ((int) signature[0] & 0xff) | ((signature[1] << 8) & 0xff00);
        return readStatus == 2 && GZIPInputStream.GZIP_MAGIC == streamHeader;
    }

    /**
     * A utility function to close an input stream without raising an exception.
     *
     * @param is input stream to close safely
     */
    public static void silentCloseInputStream(InputStream is) {
        try {
            if (is != null) {
                is.close();
            }
        } catch (IOException e) {
            Log.w(LOG_TAG, "Cannot close input stream", e);
        }
    }

    /**
     * A utility function to close an output stream without raising an exception.
     *
     * @param os output stream to close safely
     */
    public static void silentCloseOutputStream(OutputStream os) {
        try {
            if (os != null) {
                os.close();
            }
        } catch (IOException e) {
            Log.w(LOG_TAG, "Cannot close output stream", e);
        }
    }

    /**
     * Returns HttpEntity containing data from RequestParams included with request declaration.
     * Allows also passing progress from upload via provided ResponseHandler
     *
     * @param params          additional request params
     * @param responseHandler ResponseHandlerInterface or its subclass to be notified on progress
     */
    private HttpEntity paramsToEntity(RequestParams params, ResponseHandlerInterface responseHandler) {
        HttpEntity entity = null;

        try {
            if (params != null) {
                entity = params.getEntity(responseHandler);
            }
        } catch (IOException e) {
            if (responseHandler != null) {
                responseHandler.sendFailureMessage(0, null, null, e);
            } else {
                e.printStackTrace();
            }
        }

        return entity;
    }

    public boolean isUrlEncodingEnabled() {
        return isUrlEncodingEnabled;
    }

    /**
     * Applicable only to HttpRequest methods extending HttpEntityEnclosingRequestBase, which is for
     * example not DELETE
     *
     * @param entity      entity to be included within the request
     * @param requestBase HttpRequest instance, must not be null
     */
    private HttpEntityEnclosingRequestBase addEntityToRequestBase(HttpEntityEnclosingRequestBase requestBase, HttpEntity entity) {
        if (entity != null) {
            requestBase.setEntity(entity);
        }

        return requestBase;
    }

    /**
     * This horrible hack is required on Android, due to implementation of BasicManagedEntity, which
     * doesn't chain call consumeContent on underlying wrapped HttpEntity
     *
     * @param entity HttpEntity, may be null
     */
    public static void endEntityViaReflection(HttpEntity entity) {
        if (entity instanceof HttpEntityWrapper) {
            try {
                Field f = null;
                Field[] fields = HttpEntityWrapper.class.getDeclaredFields();
                for (Field ff : fields) {
                    if (ff.getName().equals("wrappedEntity")) {
                        f = ff;
                        break;
                    }
                }
                if (f != null) {
                    f.setAccessible(true);
                    HttpEntity wrapped = (HttpEntity) f.get(entity);
                    if (wrapped != null) {
                        wrapped.consumeContent();
                    }
                }
            } catch (Throwable t) {
                Log.e(LOG_TAG, "wrappedEntity consume", t);
            }
        }
    }

    /**
     * Enclosing entity to hold stream of gzip decoded data for accessing HttpEntity contents
     */
    private static class InflatingEntity extends HttpEntityWrapper {

        public InflatingEntity(HttpEntity wrapped) {
            super(wrapped);
        }

        InputStream wrappedStream;
        PushbackInputStream pushbackStream;
        GZIPInputStream gzippedStream;

        @Override
        public InputStream getContent() throws IOException {
            wrappedStream = wrappedEntity.getContent();
            pushbackStream = new PushbackInputStream(wrappedStream, 2);
            if (isInputStreamGZIPCompressed(pushbackStream)) {
                gzippedStream = new GZIPInputStream(pushbackStream);
                return gzippedStream;
            } else {
                return pushbackStream;
            }
        }

        @Override
        public long getContentLength() {
            return wrappedEntity == null ? 0 : wrappedEntity.getContentLength();
        }

        @Override
        public void consumeContent() throws IOException {
            AsyncHttpClient.silentCloseInputStream(wrappedStream);
            AsyncHttpClient.silentCloseInputStream(pushbackStream);
            AsyncHttpClient.silentCloseInputStream(gzippedStream);
            super.consumeContent();
        }
    }
}

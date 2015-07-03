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

import org.apache.http.Header;
import org.apache.http.HttpEntity;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.DefaultHandler;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

/**
 * Provides interface to deserialize SAX responses, using AsyncHttpResponseHandler. Can be used like
 * this
 * <p>&nbsp;</p>
 * <pre>
 *     AsyncHttpClient ahc = new AsyncHttpClient();
 *     FontHandler handlerInstance = ... ; // init handler instance
 *     ahc.post("https://server.tld/api/call", new SaxAsyncHttpResponseHandler{@literal <}FontHandler{@literal >}(handlerInstance){
 *         &#064;Override
 *         public void onSuccess(int statusCode, Header[] headers, FontHandler t) {
 *              // Request got HTTP success statusCode
 *         }
 *         &#064;Override
 *         public void onFailure(int statusCode, Header[] headers, FontHandler t){
 *              // Request got HTTP fail statusCode
 *         }
 *     });
 * </pre>
 *
 * @param <T> Handler extending {@link org.xml.sax.helpers.DefaultHandler}
 * @see org.xml.sax.helpers.DefaultHandler
 * @see com.loopj.android.http.AsyncHttpResponseHandler
 */
public abstract class SaxAsyncHttpResponseHandler<T extends DefaultHandler> extends AsyncHttpResponseHandler {

    /**
     * Generic Type of handler
     */
    private T handler = null;
    private final static String LOG_TAG = "SaxAsyncHttpResponseHandler";

    /**
     * Constructs new SaxAsyncHttpResponseHandler with given handler instance
     *
     * @param t instance of Handler extending DefaultHandler
     * @see org.xml.sax.helpers.DefaultHandler
     */
    public SaxAsyncHttpResponseHandler(T t) {
        super();
        if (t == null) {
            throw new Error("null instance of <T extends DefaultHandler> passed to constructor");
        }
        this.handler = t;
    }

    /**
     * Deconstructs response into given content handler
     *
     * @param entity returned HttpEntity
     * @return deconstructed response
     * @throws java.io.IOException if there is problem assembling SAX response from stream
     * @see org.apache.http.HttpEntity
     */
    @Override
    protected byte[] getResponseData(HttpEntity entity) throws IOException {
        if (entity != null) {
            InputStream instream = entity.getContent();
            InputStreamReader inputStreamReader = null;
            if (instream != null) {
                try {
                    SAXParserFactory sfactory = SAXParserFactory.newInstance();
                    SAXParser sparser = sfactory.newSAXParser();
                    XMLReader rssReader = sparser.getXMLReader();
                    rssReader.setContentHandler(handler);
                    inputStreamReader = new InputStreamReader(instream, DEFAULT_CHARSET);
                    rssReader.parse(new InputSource(inputStreamReader));
                } catch (SAXException e) {
                    Log.e(LOG_TAG, "getResponseData exception", e);
                } catch (ParserConfigurationException e) {
                    Log.e(LOG_TAG, "getResponseData exception", e);
                } finally {
                    AsyncHttpClient.silentCloseInputStream(instream);
                    if (inputStreamReader != null) {
                        try {
                            inputStreamReader.close();
                        } catch (IOException e) { /*ignore*/ }
                    }
                }
            }
        }
        return null;
    }

    /**
     * Default onSuccess method for this AsyncHttpResponseHandler to override
     *
     * @param statusCode returned HTTP status code
     * @param headers    returned HTTP headers
     * @param t          instance of Handler extending DefaultHandler
     */
    public abstract void onSuccess(int statusCode, Header[] headers, T t);

    @Override
    public void onSuccess(int statusCode, Header[] headers, byte[] responseBody) {
        onSuccess(statusCode, headers, handler);
    }

    /**
     * Default onFailure method for this AsyncHttpResponseHandler to override
     *
     * @param statusCode returned HTTP status code
     * @param headers    returned HTTP headers
     * @param t          instance of Handler extending DefaultHandler
     */
    public abstract void onFailure(int statusCode, Header[] headers, T t);

    @Override
    public void onFailure(int statusCode, Header[] headers,
                          byte[] responseBody, Throwable error) {
        onSuccess(statusCode, headers, handler);
    }
}

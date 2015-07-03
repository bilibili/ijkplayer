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

import org.apache.http.HttpEntity;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.utils.URLEncodedUtils;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.protocol.HTTP;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.Serializable;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

/**
 * A collection of string request parameters or files to send along with requests made from an
 * {@link AsyncHttpClient} instance. <p>&nbsp;</p> For example: <p>&nbsp;</p>
 * <pre>
 * RequestParams params = new RequestParams();
 * params.put("username", "james");
 * params.put("password", "123456");
 * params.put("email", "my&#064;email.com");
 * params.put("profile_picture", new File("pic.jpg")); // Upload a File
 * params.put("profile_picture2", someInputStream); // Upload an InputStream
 * params.put("profile_picture3", new ByteArrayInputStream(someBytes)); // Upload some bytes
 *
 * Map&lt;String, String&gt; map = new HashMap&lt;String, String&gt;();
 * map.put("first_name", "James");
 * map.put("last_name", "Smith");
 * params.put("user", map); // url params: "user[first_name]=James&amp;user[last_name]=Smith"
 *
 * Set&lt;String&gt; set = new HashSet&lt;String&gt;(); // unordered collection
 * set.add("music");
 * set.add("art");
 * params.put("like", set); // url params: "like=music&amp;like=art"
 *
 * List&lt;String&gt; list = new ArrayList&lt;String&gt;(); // Ordered collection
 * list.add("Java");
 * list.add("C");
 * params.put("languages", list); // url params: "languages[]=Java&amp;languages[]=C"
 *
 * String[] colors = { "blue", "yellow" }; // Ordered collection
 * params.put("colors", colors); // url params: "colors[]=blue&amp;colors[]=yellow"
 *
 * File[] files = { new File("pic.jpg"), new File("pic1.jpg") }; // Ordered collection
 * params.put("files", files); // url params: "files[]=pic.jpg&amp;files[]=pic1.jpg"
 *
 * List&lt;Map&lt;String, String&gt;&gt; listOfMaps = new ArrayList&lt;Map&lt;String,
 * String&gt;&gt;();
 * Map&lt;String, String&gt; user1 = new HashMap&lt;String, String&gt;();
 * user1.put("age", "30");
 * user1.put("gender", "male");
 * Map&lt;String, String&gt; user2 = new HashMap&lt;String, String&gt;();
 * user2.put("age", "25");
 * user2.put("gender", "female");
 * listOfMaps.add(user1);
 * listOfMaps.add(user2);
 * params.put("users", listOfMaps); // url params: "users[][age]=30&amp;users[][gender]=male&amp;users[][age]=25&amp;users[][gender]=female"
 *
 * AsyncHttpClient client = new AsyncHttpClient();
 * client.post("http://myendpoint.com", params, responseHandler);
 * </pre>
 */
public class RequestParams implements Serializable {

    public final static String APPLICATION_OCTET_STREAM =
            "application/octet-stream";

    public final static String APPLICATION_JSON =
            "application/json";

    protected final static String LOG_TAG = "RequestParams";
    protected boolean isRepeatable;
    protected boolean forceMultipartEntity = false;
    protected boolean useJsonStreamer;
    protected String elapsedFieldInJsonStreamer = "_elapsed";
    protected boolean autoCloseInputStreams;
    protected final ConcurrentHashMap<String, String> urlParams = new ConcurrentHashMap<String, String>();
    protected final ConcurrentHashMap<String, StreamWrapper> streamParams = new ConcurrentHashMap<String, StreamWrapper>();
    protected final ConcurrentHashMap<String, FileWrapper> fileParams = new ConcurrentHashMap<String, FileWrapper>();
    protected final ConcurrentHashMap<String, List<FileWrapper>> fileArrayParams = new ConcurrentHashMap<String, List<FileWrapper>>();
    protected final ConcurrentHashMap<String, Object> urlParamsWithObjects = new ConcurrentHashMap<String, Object>();
    protected String contentEncoding = HTTP.UTF_8;

    /**
     * Sets content encoding for return value of {@link #getParamString()} and {@link
     * #createFormEntity()} <p>&nbsp;</p> Default encoding is "UTF-8"
     *
     * @param encoding String constant from {@link HTTP}
     */
    public void setContentEncoding(final String encoding) {
        if (encoding != null) {
            this.contentEncoding = encoding;
        } else {
            Log.d(LOG_TAG, "setContentEncoding called with null attribute");
        }
    }

    /**
     * If set to true will force Content-Type header to `multipart/form-data`
     * even if there are not Files or Streams to be send
     * <p>&nbsp;</p>
     * Default value is false
     *
     * @param force boolean, should declare content-type multipart/form-data even without files or streams present
     */
    public void setForceMultipartEntityContentType(boolean force) {
        this.forceMultipartEntity = force;
    }

    /**
     * Constructs a new empty {@code RequestParams} instance.
     */
    public RequestParams() {
        this((Map<String, String>) null);
    }

    /**
     * Constructs a new RequestParams instance containing the key/value string params from the
     * specified map.
     *
     * @param source the source key/value string map to add.
     */
    public RequestParams(Map<String, String> source) {
        if (source != null) {
            for (Map.Entry<String, String> entry : source.entrySet()) {
                put(entry.getKey(), entry.getValue());
            }
        }
    }

    /**
     * Constructs a new RequestParams instance and populate it with a single initial key/value
     * string param.
     *
     * @param key   the key name for the intial param.
     * @param value the value string for the initial param.
     */
    public RequestParams(final String key, final String value) {
        this(new HashMap<String, String>() {{
            put(key, value);
        }});
    }

    /**
     * Constructs a new RequestParams instance and populate it with multiple initial key/value
     * string param.
     *
     * @param keysAndValues a sequence of keys and values. Objects are automatically converted to
     *                      Strings (including the value {@code null}).
     * @throws IllegalArgumentException if the number of arguments isn't even.
     */
    public RequestParams(Object... keysAndValues) {
        int len = keysAndValues.length;
        if (len % 2 != 0)
            throw new IllegalArgumentException("Supplied arguments must be even");
        for (int i = 0; i < len; i += 2) {
            String key = String.valueOf(keysAndValues[i]);
            String val = String.valueOf(keysAndValues[i + 1]);
            put(key, val);
        }
    }

    /**
     * Adds a key/value string pair to the request.
     *
     * @param key   the key name for the new param.
     * @param value the value string for the new param.
     */
    public void put(String key, String value) {
        if (key != null && value != null) {
            urlParams.put(key, value);
        }
    }

    /**
     * Adds files array to the request.
     *
     * @param key   the key name for the new param.
     * @param files the files array to add.
     * @throws FileNotFoundException if one of passed files is not found at time of assembling the requestparams into request
     */
    public void put(String key, File files[]) throws FileNotFoundException {
        put(key, files, null, null);
    }

    /**
     * Adds files array to the request with both custom provided file content-type and files name
     *
     * @param key            the key name for the new param.
     * @param files          the files array to add.
     * @param contentType    the content type of the file, eg. application/json
     * @param customFileName file name to use instead of real file name
     * @throws FileNotFoundException throws if wrong File argument was passed
     */
    public void put(String key, File files[], String contentType, String customFileName) throws FileNotFoundException {

        if (key != null) {
            List<FileWrapper> fileWrappers = new ArrayList<FileWrapper>();
            for (int i = 0; i < files.length; i++) {
                if (files[i] == null || !files[i].exists()) {
                    throw new FileNotFoundException();
                }
                fileWrappers.add(new FileWrapper(files[i], contentType, customFileName));
            }
            fileArrayParams.put(key, fileWrappers);
        }
    }

    /**
     * Adds a file to the request.
     *
     * @param key  the key name for the new param.
     * @param file the file to add.
     * @throws FileNotFoundException throws if wrong File argument was passed
     */
    public void put(String key, File file) throws FileNotFoundException {
        put(key, file, null, null);
    }

    /**
     * Adds a file to the request with custom provided file name
     *
     * @param key            the key name for the new param.
     * @param file           the file to add.
     * @param customFileName file name to use instead of real file name
     * @throws FileNotFoundException throws if wrong File argument was passed
     */
    public void put(String key, String customFileName, File file) throws FileNotFoundException {
        put(key, file, null, customFileName);
    }

    /**
     * Adds a file to the request with custom provided file content-type
     *
     * @param key         the key name for the new param.
     * @param file        the file to add.
     * @param contentType the content type of the file, eg. application/json
     * @throws FileNotFoundException throws if wrong File argument was passed
     */
    public void put(String key, File file, String contentType) throws FileNotFoundException {
        put(key, file, contentType, null);
    }

    /**
     * Adds a file to the request with both custom provided file content-type and file name
     *
     * @param key            the key name for the new param.
     * @param file           the file to add.
     * @param contentType    the content type of the file, eg. application/json
     * @param customFileName file name to use instead of real file name
     * @throws FileNotFoundException throws if wrong File argument was passed
     */
    public void put(String key, File file, String contentType, String customFileName) throws FileNotFoundException {
        if (file == null || !file.exists()) {
            throw new FileNotFoundException();
        }
        if (key != null) {
            fileParams.put(key, new FileWrapper(file, contentType, customFileName));
        }
    }

    /**
     * Adds an input stream to the request.
     *
     * @param key    the key name for the new param.
     * @param stream the input stream to add.
     */
    public void put(String key, InputStream stream) {
        put(key, stream, null);
    }

    /**
     * Adds an input stream to the request.
     *
     * @param key    the key name for the new param.
     * @param stream the input stream to add.
     * @param name   the name of the stream.
     */
    public void put(String key, InputStream stream, String name) {
        put(key, stream, name, null);
    }

    /**
     * Adds an input stream to the request.
     *
     * @param key         the key name for the new param.
     * @param stream      the input stream to add.
     * @param name        the name of the stream.
     * @param contentType the content type of the file, eg. application/json
     */
    public void put(String key, InputStream stream, String name, String contentType) {
        put(key, stream, name, contentType, autoCloseInputStreams);
    }

    /**
     * Adds an input stream to the request.
     *
     * @param key         the key name for the new param.
     * @param stream      the input stream to add.
     * @param name        the name of the stream.
     * @param contentType the content type of the file, eg. application/json
     * @param autoClose   close input stream automatically on successful upload
     */
    public void put(String key, InputStream stream, String name, String contentType, boolean autoClose) {
        if (key != null && stream != null) {
            streamParams.put(key, StreamWrapper.newInstance(stream, name, contentType, autoClose));
        }
    }

    /**
     * Adds param with non-string value (e.g. Map, List, Set).
     *
     * @param key   the key name for the new param.
     * @param value the non-string value object for the new param.
     */
    public void put(String key, Object value) {
        if (key != null && value != null) {
            urlParamsWithObjects.put(key, value);
        }
    }

    /**
     * Adds a int value to the request.
     *
     * @param key   the key name for the new param.
     * @param value the value int for the new param.
     */
    public void put(String key, int value) {
        if (key != null) {
            urlParams.put(key, String.valueOf(value));
        }
    }

    /**
     * Adds a long value to the request.
     *
     * @param key   the key name for the new param.
     * @param value the value long for the new param.
     */
    public void put(String key, long value) {
        if (key != null) {
            urlParams.put(key, String.valueOf(value));
        }
    }

    /**
     * Adds string value to param which can have more than one value.
     *
     * @param key   the key name for the param, either existing or new.
     * @param value the value string for the new param.
     */
    public void add(String key, String value) {
        if (key != null && value != null) {
            Object params = urlParamsWithObjects.get(key);
            if (params == null) {
                // Backward compatible, which will result in "k=v1&k=v2&k=v3"
                params = new HashSet<String>();
                this.put(key, params);
            }
            if (params instanceof List) {
                ((List<Object>) params).add(value);
            } else if (params instanceof Set) {
                ((Set<Object>) params).add(value);
            }
        }
    }

    /**
     * Removes a parameter from the request.
     *
     * @param key the key name for the parameter to remove.
     */
    public void remove(String key) {
        urlParams.remove(key);
        streamParams.remove(key);
        fileParams.remove(key);
        urlParamsWithObjects.remove(key);
        fileArrayParams.remove(key);
    }

    /**
     * Check if a parameter is defined.
     *
     * @param key the key name for the parameter to check existence.
     * @return Boolean
     */
    public boolean has(String key) {
        return urlParams.get(key) != null ||
                streamParams.get(key) != null ||
                fileParams.get(key) != null ||
                urlParamsWithObjects.get(key) != null ||
                fileArrayParams.get(key) != null;
    }

    @Override
    public String toString() {
        StringBuilder result = new StringBuilder();
        for (ConcurrentHashMap.Entry<String, String> entry : urlParams.entrySet()) {
            if (result.length() > 0)
                result.append("&");

            result.append(entry.getKey());
            result.append("=");
            result.append(entry.getValue());
        }

        for (ConcurrentHashMap.Entry<String, StreamWrapper> entry : streamParams.entrySet()) {
            if (result.length() > 0)
                result.append("&");

            result.append(entry.getKey());
            result.append("=");
            result.append("STREAM");
        }

        for (ConcurrentHashMap.Entry<String, FileWrapper> entry : fileParams.entrySet()) {
            if (result.length() > 0)
                result.append("&");

            result.append(entry.getKey());
            result.append("=");
            result.append("FILE");
        }

        for (ConcurrentHashMap.Entry<String, List<FileWrapper>> entry : fileArrayParams.entrySet()) {
            if (result.length() > 0)
                result.append("&");

            result.append(entry.getKey());
            result.append("=");
            result.append("FILE");
        }

        List<BasicNameValuePair> params = getParamsList(null, urlParamsWithObjects);
        for (BasicNameValuePair kv : params) {
            if (result.length() > 0)
                result.append("&");

            result.append(kv.getName());
            result.append("=");
            result.append(kv.getValue());
        }

        return result.toString();
    }

    public void setHttpEntityIsRepeatable(boolean flag) {
        this.isRepeatable = flag;
    }

    public void setUseJsonStreamer(boolean flag) {
        this.useJsonStreamer = flag;
    }

    /**
     * Sets an additional field when upload a JSON object through the streamer
     * to hold the time, in milliseconds, it took to upload the payload. By
     * default, this field is set to "_elapsed".
     * <p>&nbsp;</p>
     * To disable this feature, call this method with null as the field value.
     *
     * @param value field name to add elapsed time, or null to disable
     */
    public void setElapsedFieldInJsonStreamer(String value) {
        this.elapsedFieldInJsonStreamer = value;
    }

    /**
     * Set global flag which determines whether to automatically close input streams on successful
     * upload.
     *
     * @param flag boolean whether to automatically close input streams
     */
    public void setAutoCloseInputStreams(boolean flag) {
        autoCloseInputStreams = flag;
    }

    /**
     * Returns an HttpEntity containing all request parameters.
     *
     * @param progressHandler HttpResponseHandler for reporting progress on entity submit
     * @return HttpEntity resulting HttpEntity to be included along with {@link
     * org.apache.http.client.methods.HttpEntityEnclosingRequestBase}
     * @throws IOException if one of the streams cannot be read
     */
    public HttpEntity getEntity(ResponseHandlerInterface progressHandler) throws IOException {
        if (useJsonStreamer) {
            return createJsonStreamerEntity(progressHandler);
        } else if (!forceMultipartEntity && streamParams.isEmpty() && fileParams.isEmpty() && fileArrayParams.isEmpty()) {
            return createFormEntity();
        } else {
            return createMultipartEntity(progressHandler);
        }
    }

    private HttpEntity createJsonStreamerEntity(ResponseHandlerInterface progressHandler) throws IOException {
        JsonStreamerEntity entity = new JsonStreamerEntity(
                progressHandler,
                !fileParams.isEmpty() || !streamParams.isEmpty(),
                elapsedFieldInJsonStreamer);

        // Add string params
        for (ConcurrentHashMap.Entry<String, String> entry : urlParams.entrySet()) {
            entity.addPart(entry.getKey(), entry.getValue());
        }

        // Add non-string params
        for (ConcurrentHashMap.Entry<String, Object> entry : urlParamsWithObjects.entrySet()) {
            entity.addPart(entry.getKey(), entry.getValue());
        }

        // Add file params
        for (ConcurrentHashMap.Entry<String, FileWrapper> entry : fileParams.entrySet()) {
            entity.addPart(entry.getKey(), entry.getValue());
        }

        // Add stream params
        for (ConcurrentHashMap.Entry<String, StreamWrapper> entry : streamParams.entrySet()) {
            StreamWrapper stream = entry.getValue();
            if (stream.inputStream != null) {
                entity.addPart(entry.getKey(),
                        StreamWrapper.newInstance(
                                stream.inputStream,
                                stream.name,
                                stream.contentType,
                                stream.autoClose)
                );
            }
        }

        return entity;
    }

    private HttpEntity createFormEntity() {
        try {
            return new UrlEncodedFormEntity(getParamsList(), contentEncoding);
        } catch (UnsupportedEncodingException e) {
            Log.e(LOG_TAG, "createFormEntity failed", e);
            return null; // Can happen, if the 'contentEncoding' won't be HTTP.UTF_8
        }
    }

    private HttpEntity createMultipartEntity(ResponseHandlerInterface progressHandler) throws IOException {
        SimpleMultipartEntity entity = new SimpleMultipartEntity(progressHandler);
        entity.setIsRepeatable(isRepeatable);

        // Add string params
        for (ConcurrentHashMap.Entry<String, String> entry : urlParams.entrySet()) {
            entity.addPartWithCharset(entry.getKey(), entry.getValue(), contentEncoding);
        }

        // Add non-string params
        List<BasicNameValuePair> params = getParamsList(null, urlParamsWithObjects);
        for (BasicNameValuePair kv : params) {
            entity.addPartWithCharset(kv.getName(), kv.getValue(), contentEncoding);
        }

        // Add stream params
        for (ConcurrentHashMap.Entry<String, StreamWrapper> entry : streamParams.entrySet()) {
            StreamWrapper stream = entry.getValue();
            if (stream.inputStream != null) {
                entity.addPart(entry.getKey(), stream.name, stream.inputStream,
                        stream.contentType);
            }
        }

        // Add file params
        for (ConcurrentHashMap.Entry<String, FileWrapper> entry : fileParams.entrySet()) {
            FileWrapper fileWrapper = entry.getValue();
            entity.addPart(entry.getKey(), fileWrapper.file, fileWrapper.contentType, fileWrapper.customFileName);
        }

        // Add file collection
        for (ConcurrentHashMap.Entry<String, List<FileWrapper>> entry : fileArrayParams.entrySet()) {
            List<FileWrapper> fileWrapper = entry.getValue();
            for (FileWrapper fw : fileWrapper) {
                entity.addPart(entry.getKey(), fw.file, fw.contentType, fw.customFileName);
            }
        }

        return entity;
    }

    protected List<BasicNameValuePair> getParamsList() {
        List<BasicNameValuePair> lparams = new LinkedList<BasicNameValuePair>();

        for (ConcurrentHashMap.Entry<String, String> entry : urlParams.entrySet()) {
            lparams.add(new BasicNameValuePair(entry.getKey(), entry.getValue()));
        }

        lparams.addAll(getParamsList(null, urlParamsWithObjects));

        return lparams;
    }

    private List<BasicNameValuePair> getParamsList(String key, Object value) {
        List<BasicNameValuePair> params = new LinkedList<BasicNameValuePair>();
        if (value instanceof Map) {
            Map map = (Map) value;
            List list = new ArrayList<Object>(map.keySet());
            // Ensure consistent ordering in query string
            if (list.size() > 0 && list.get(0) instanceof Comparable) {
                Collections.sort(list);
            }
            for (Object nestedKey : list) {
                if (nestedKey instanceof String) {
                    Object nestedValue = map.get(nestedKey);
                    if (nestedValue != null) {
                        params.addAll(getParamsList(key == null ? (String) nestedKey : String.format(Locale.US, "%s[%s]", key, nestedKey),
                                nestedValue));
                    }
                }
            }
        } else if (value instanceof List) {
            List list = (List) value;
            int listSize = list.size();
            for (int nestedValueIndex = 0; nestedValueIndex < listSize; nestedValueIndex++) {
                params.addAll(getParamsList(String.format(Locale.US, "%s[%d]", key, nestedValueIndex), list.get(nestedValueIndex)));
            }
        } else if (value instanceof Object[]) {
            Object[] array = (Object[]) value;
            int arrayLength = array.length;
            for (int nestedValueIndex = 0; nestedValueIndex < arrayLength; nestedValueIndex++) {
                params.addAll(getParamsList(String.format(Locale.US, "%s[%d]", key, nestedValueIndex), array[nestedValueIndex]));
            }
        } else if (value instanceof Set) {
            Set set = (Set) value;
            for (Object nestedValue : set) {
                params.addAll(getParamsList(key, nestedValue));
            }
        } else {
            params.add(new BasicNameValuePair(key, value.toString()));
        }
        return params;
    }

    protected String getParamString() {
        return URLEncodedUtils.format(getParamsList(), contentEncoding);
    }

    public static class FileWrapper implements Serializable {
        public final File file;
        public final String contentType;
        public final String customFileName;

        public FileWrapper(File file, String contentType, String customFileName) {
            this.file = file;
            this.contentType = contentType;
            this.customFileName = customFileName;
        }
    }

    public static class StreamWrapper {
        public final InputStream inputStream;
        public final String name;
        public final String contentType;
        public final boolean autoClose;

        public StreamWrapper(InputStream inputStream, String name, String contentType, boolean autoClose) {
            this.inputStream = inputStream;
            this.name = name;
            this.contentType = contentType;
            this.autoClose = autoClose;
        }

        static StreamWrapper newInstance(InputStream inputStream, String name, String contentType, boolean autoClose) {
            return new StreamWrapper(
                    inputStream,
                    name,
                    contentType == null ? APPLICATION_OCTET_STREAM : contentType,
                    autoClose);
        }
    }
}

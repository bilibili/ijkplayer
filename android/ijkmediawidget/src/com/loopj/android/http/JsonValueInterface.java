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

/**
 * This interface is used to encapsulate JSON values that are handled entirely
 * by the app. For example, apps could manage any type of JSON on their own (and
 * not rely on {@link org.json.JSONArray} or {@link org.json.JSONObject} to
 * exchange data.
 *
 * @author Noor Dawod {@literal <github@fineswap.com>}
 */
public interface JsonValueInterface {

    /**
     * Returns the escaped, ready-to-be used value of this encapsulated object.
     *
     * @return byte array holding the data to be used (as-is) in a JSON object
     */
    byte[] getEscapedJsonValue();
}

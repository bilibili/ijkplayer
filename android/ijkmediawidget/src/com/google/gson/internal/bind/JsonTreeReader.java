/*
 * Copyright (C) 2011 Google Inc.
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

package com.google.gson.internal.bind;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonNull;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import com.google.gson.stream.JsonReader;
import com.google.gson.stream.JsonToken;
import java.io.IOException;
import java.io.Reader;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 * This reader walks the elements of a JsonElement as if it was coming from a
 * character stream.
 *
 * @author Jesse Wilson
 */
public final class JsonTreeReader extends JsonReader {
  private static final Reader UNREADABLE_READER = new Reader() {
    @Override public int read(char[] buffer, int offset, int count) throws IOException {
      throw new AssertionError();
    }
    @Override public void close() throws IOException {
      throw new AssertionError();
    }
  };
  private static final Object SENTINEL_CLOSED = new Object();

  private final List<Object> stack = new ArrayList<Object>();

  public JsonTreeReader(JsonElement element) {
    super(UNREADABLE_READER);
    stack.add(element);
  }

  @Override public void beginArray() throws IOException {
    expect(JsonToken.BEGIN_ARRAY);
    JsonArray array = (JsonArray) peekStack();
    stack.add(array.iterator());
  }

  @Override public void endArray() throws IOException {
    expect(JsonToken.END_ARRAY);
    popStack(); // empty iterator
    popStack(); // array
  }

  @Override public void beginObject() throws IOException {
    expect(JsonToken.BEGIN_OBJECT);
    JsonObject object = (JsonObject) peekStack();
    stack.add(object.entrySet().iterator());
  }

  @Override public void endObject() throws IOException {
    expect(JsonToken.END_OBJECT);
    popStack(); // empty iterator
    popStack(); // object
  }

  @Override public boolean hasNext() throws IOException {
    JsonToken token = peek();
    return token != JsonToken.END_OBJECT && token != JsonToken.END_ARRAY;
  }

  @Override public JsonToken peek() throws IOException {
    if (stack.isEmpty()) {
      return JsonToken.END_DOCUMENT;
    }

    Object o = peekStack();
    if (o instanceof Iterator) {
      boolean isObject = stack.get(stack.size() - 2) instanceof JsonObject;
      Iterator<?> iterator = (Iterator<?>) o;
      if (iterator.hasNext()) {
        if (isObject) {
          return JsonToken.NAME;
        } else {
          stack.add(iterator.next());
          return peek();
        }
      } else {
        return isObject ? JsonToken.END_OBJECT : JsonToken.END_ARRAY;
      }
    } else if (o instanceof JsonObject) {
      return JsonToken.BEGIN_OBJECT;
    } else if (o instanceof JsonArray) {
      return JsonToken.BEGIN_ARRAY;
    } else if (o instanceof JsonPrimitive) {
      JsonPrimitive primitive = (JsonPrimitive) o;
      if (primitive.isString()) {
        return JsonToken.STRING;
      } else if (primitive.isBoolean()) {
        return JsonToken.BOOLEAN;
      } else if (primitive.isNumber()) {
        return JsonToken.NUMBER;
      } else {
        throw new AssertionError();
      }
    } else if (o instanceof JsonNull) {
      return JsonToken.NULL;
    } else if (o == SENTINEL_CLOSED) {
      throw new IllegalStateException("JsonReader is closed");
    } else {
      throw new AssertionError();
    }
  }

  private Object peekStack() {
    return stack.get(stack.size() - 1);
  }

  private Object popStack() {
    return stack.remove(stack.size() - 1);
  }

  private void expect(JsonToken expected) throws IOException {
    if (peek() != expected) {
      throw new IllegalStateException("Expected " + expected + " but was " + peek());
    }
  }

  @Override public String nextName() throws IOException {
    expect(JsonToken.NAME);
    Iterator<?> i = (Iterator<?>) peekStack();
    Map.Entry<?, ?> entry = (Map.Entry<?, ?>) i.next();
    stack.add(entry.getValue());
    return (String) entry.getKey();
  }

  @Override public String nextString() throws IOException {
    JsonToken token = peek();
    if (token != JsonToken.STRING && token != JsonToken.NUMBER) {
      throw new IllegalStateException("Expected " + JsonToken.STRING + " but was " + token);
    }
    return ((JsonPrimitive) popStack()).getAsString();
  }

  @Override public boolean nextBoolean() throws IOException {
    expect(JsonToken.BOOLEAN);
    return ((JsonPrimitive) popStack()).getAsBoolean();
  }

  @Override public void nextNull() throws IOException {
    expect(JsonToken.NULL);
    popStack();
  }

  @Override public double nextDouble() throws IOException {
    JsonToken token = peek();
    if (token != JsonToken.NUMBER && token != JsonToken.STRING) {
      throw new IllegalStateException("Expected " + JsonToken.NUMBER + " but was " + token);
    }
    double result = ((JsonPrimitive) peekStack()).getAsDouble();
    if (!isLenient() && (Double.isNaN(result) || Double.isInfinite(result))) {
      throw new NumberFormatException("JSON forbids NaN and infinities: " + result);
    }
    popStack();
    return result;
  }

  @Override public long nextLong() throws IOException {
    JsonToken token = peek();
    if (token != JsonToken.NUMBER && token != JsonToken.STRING) {
      throw new IllegalStateException("Expected " + JsonToken.NUMBER + " but was " + token);
    }
    long result = ((JsonPrimitive) peekStack()).getAsLong();
    popStack();
    return result;
  }

  @Override public int nextInt() throws IOException {
    JsonToken token = peek();
    if (token != JsonToken.NUMBER && token != JsonToken.STRING) {
      throw new IllegalStateException("Expected " + JsonToken.NUMBER + " but was " + token);
    }
    int result = ((JsonPrimitive) peekStack()).getAsInt();
    popStack();
    return result;
  }

  @Override public void close() throws IOException {
    stack.clear();
    stack.add(SENTINEL_CLOSED);
  }

  @Override public void skipValue() throws IOException {
    if (peek() == JsonToken.NAME) {
      nextName();
    } else {
      popStack();
    }
  }

  @Override public String toString() {
    return getClass().getSimpleName();
  }

  public void promoteNameToValue() throws IOException {
    expect(JsonToken.NAME);
    Iterator<?> i = (Iterator<?>) peekStack();
    Map.Entry<?, ?> entry = (Map.Entry<?, ?>) i.next();
    stack.add(entry.getValue());
    stack.add(new JsonPrimitive((String)entry.getKey()));
  }
}

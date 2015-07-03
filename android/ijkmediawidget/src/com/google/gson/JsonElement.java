/*
 * Copyright (C) 2008 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.gson;

import com.google.gson.internal.Streams;
import com.google.gson.stream.JsonWriter;
import java.io.IOException;
import java.io.StringWriter;
import java.math.BigDecimal;
import java.math.BigInteger;

/**
 * A class representing an element of Json. It could either be a {@link JsonObject}, a
 * {@link JsonArray}, a {@link JsonPrimitive} or a {@link JsonNull}.
 *
 * @author Inderjeet Singh
 * @author Joel Leitch
 */
public abstract class JsonElement {
  /**
   * Returns a deep copy of this element. Immutable elements like primitives
   * and nulls are not copied.
   */
  abstract JsonElement deepCopy();

  /**
   * provides check for verifying if this element is an array or not.
   *
   * @return true if this element is of type {@link JsonArray}, false otherwise.
   */
  public boolean isJsonArray() {
    return this instanceof JsonArray;
  }

  /**
   * provides check for verifying if this element is a Json object or not.
   *
   * @return true if this element is of type {@link JsonObject}, false otherwise.
   */
  public boolean isJsonObject() {
    return this instanceof JsonObject;
  }

  /**
   * provides check for verifying if this element is a primitive or not.
   *
   * @return true if this element is of type {@link JsonPrimitive}, false otherwise.
   */
  public boolean isJsonPrimitive() {
    return this instanceof JsonPrimitive;
  }

  /**
   * provides check for verifying if this element represents a null value or not.
   *
   * @return true if this element is of type {@link JsonNull}, false otherwise.
   * @since 1.2
   */
  public boolean isJsonNull() {
    return this instanceof JsonNull;
  }

  /**
   * convenience method to get this element as a {@link JsonObject}. If the element is of some
   * other type, a {@link IllegalStateException} will result. Hence it is best to use this method
   * after ensuring that this element is of the desired type by calling {@link #isJsonObject()}
   * first.
   *
   * @return get this element as a {@link JsonObject}.
   * @throws IllegalStateException if the element is of another type.
   */
  public JsonObject getAsJsonObject() {
    if (isJsonObject()) {
      return (JsonObject) this;
    }
    throw new IllegalStateException("Not a JSON Object: " + this);
  }

  /**
   * convenience method to get this element as a {@link JsonArray}. If the element is of some
   * other type, a {@link IllegalStateException} will result. Hence it is best to use this method
   * after ensuring that this element is of the desired type by calling {@link #isJsonArray()}
   * first.
   *
   * @return get this element as a {@link JsonArray}.
   * @throws IllegalStateException if the element is of another type.
   */
  public JsonArray getAsJsonArray() {
    if (isJsonArray()) {
      return (JsonArray) this;
    }
    throw new IllegalStateException("This is not a JSON Array.");
  }

  /**
   * convenience method to get this element as a {@link JsonPrimitive}. If the element is of some
   * other type, a {@link IllegalStateException} will result. Hence it is best to use this method
   * after ensuring that this element is of the desired type by calling {@link #isJsonPrimitive()}
   * first.
   *
   * @return get this element as a {@link JsonPrimitive}.
   * @throws IllegalStateException if the element is of another type.
   */
  public JsonPrimitive getAsJsonPrimitive() {
    if (isJsonPrimitive()) {
      return (JsonPrimitive) this;
    }
    throw new IllegalStateException("This is not a JSON Primitive.");
  }

  /**
   * convenience method to get this element as a {@link JsonNull}. If the element is of some
   * other type, a {@link IllegalStateException} will result. Hence it is best to use this method
   * after ensuring that this element is of the desired type by calling {@link #isJsonNull()}
   * first.
   *
   * @return get this element as a {@link JsonNull}.
   * @throws IllegalStateException if the element is of another type.
   * @since 1.2
   */
  public JsonNull getAsJsonNull() {
    if (isJsonNull()) {
      return (JsonNull) this;
    }
    throw new IllegalStateException("This is not a JSON Null.");
  }

  /**
   * convenience method to get this element as a boolean value.
   *
   * @return get this element as a primitive boolean value.
   * @throws ClassCastException if the element is of not a {@link JsonPrimitive} and is not a valid
   * boolean value.
   * @throws IllegalStateException if the element is of the type {@link JsonArray} but contains
   * more than a single element.
   */
  public boolean getAsBoolean() {
    throw new UnsupportedOperationException(getClass().getSimpleName());
  }

  /**
   * convenience method to get this element as a {@link Boolean} value.
   *
   * @return get this element as a {@link Boolean} value.
   * @throws ClassCastException if the element is of not a {@link JsonPrimitive} and is not a valid
   * boolean value.
   * @throws IllegalStateException if the element is of the type {@link JsonArray} but contains
   * more than a single element.
   */
  Boolean getAsBooleanWrapper() {
    throw new UnsupportedOperationException(getClass().getSimpleName());
  }

  /**
   * convenience method to get this element as a {@link Number}.
   *
   * @return get this element as a {@link Number}.
   * @throws ClassCastException if the element is of not a {@link JsonPrimitive} and is not a valid
   * number.
   * @throws IllegalStateException if the element is of the type {@link JsonArray} but contains
   * more than a single element.
   */
  public Number getAsNumber() {
    throw new UnsupportedOperationException(getClass().getSimpleName());
  }

  /**
   * convenience method to get this element as a string value.
   *
   * @return get this element as a string value.
   * @throws ClassCastException if the element is of not a {@link JsonPrimitive} and is not a valid
   * string value.
   * @throws IllegalStateException if the element is of the type {@link JsonArray} but contains
   * more than a single element.
   */
  public String getAsString() {
    throw new UnsupportedOperationException(getClass().getSimpleName());
  }

  /**
   * convenience method to get this element as a primitive double value.
   *
   * @return get this element as a primitive double value.
   * @throws ClassCastException if the element is of not a {@link JsonPrimitive} and is not a valid
   * double value.
   * @throws IllegalStateException if the element is of the type {@link JsonArray} but contains
   * more than a single element.
   */
  public double getAsDouble() {
    throw new UnsupportedOperationException(getClass().getSimpleName());
  }

  /**
   * convenience method to get this element as a primitive float value.
   *
   * @return get this element as a primitive float value.
   * @throws ClassCastException if the element is of not a {@link JsonPrimitive} and is not a valid
   * float value.
   * @throws IllegalStateException if the element is of the type {@link JsonArray} but contains
   * more than a single element.
   */
  public float getAsFloat() {
    throw new UnsupportedOperationException(getClass().getSimpleName());
  }

  /**
   * convenience method to get this element as a primitive long value.
   *
   * @return get this element as a primitive long value.
   * @throws ClassCastException if the element is of not a {@link JsonPrimitive} and is not a valid
   * long value.
   * @throws IllegalStateException if the element is of the type {@link JsonArray} but contains
   * more than a single element.
   */
  public long getAsLong() {
    throw new UnsupportedOperationException(getClass().getSimpleName());
  }

  /**
   * convenience method to get this element as a primitive integer value.
   *
   * @return get this element as a primitive integer value.
   * @throws ClassCastException if the element is of not a {@link JsonPrimitive} and is not a valid
   * integer value.
   * @throws IllegalStateException if the element is of the type {@link JsonArray} but contains
   * more than a single element.
   */
  public int getAsInt() {
    throw new UnsupportedOperationException(getClass().getSimpleName());
  }

  /**
   * convenience method to get this element as a primitive byte value.
   *
   * @return get this element as a primitive byte value.
   * @throws ClassCastException if the element is of not a {@link JsonPrimitive} and is not a valid
   * byte value.
   * @throws IllegalStateException if the element is of the type {@link JsonArray} but contains
   * more than a single element.
   * @since 1.3
   */
  public byte getAsByte() {
    throw new UnsupportedOperationException(getClass().getSimpleName());
  }

  /**
   * convenience method to get this element as a primitive character value.
   *
   * @return get this element as a primitive char value.
   * @throws ClassCastException if the element is of not a {@link JsonPrimitive} and is not a valid
   * char value.
   * @throws IllegalStateException if the element is of the type {@link JsonArray} but contains
   * more than a single element.
   * @since 1.3
   */
  public char getAsCharacter() {
    throw new UnsupportedOperationException(getClass().getSimpleName());
  }

  /**
   * convenience method to get this element as a {@link BigDecimal}.
   *
   * @return get this element as a {@link BigDecimal}.
   * @throws ClassCastException if the element is of not a {@link JsonPrimitive}.
   * * @throws NumberFormatException if the element is not a valid {@link BigDecimal}.
   * @throws IllegalStateException if the element is of the type {@link JsonArray} but contains
   * more than a single element.
   * @since 1.2
   */
  public BigDecimal getAsBigDecimal() {
    throw new UnsupportedOperationException(getClass().getSimpleName());
  }

  /**
   * convenience method to get this element as a {@link BigInteger}.
   *
   * @return get this element as a {@link BigInteger}.
   * @throws ClassCastException if the element is of not a {@link JsonPrimitive}.
   * @throws NumberFormatException if the element is not a valid {@link BigInteger}.
   * @throws IllegalStateException if the element is of the type {@link JsonArray} but contains
   * more than a single element.
   * @since 1.2
   */
  public BigInteger getAsBigInteger() {
    throw new UnsupportedOperationException(getClass().getSimpleName());
  }

  /**
   * convenience method to get this element as a primitive short value.
   *
   * @return get this element as a primitive short value.
   * @throws ClassCastException if the element is of not a {@link JsonPrimitive} and is not a valid
   * short value.
   * @throws IllegalStateException if the element is of the type {@link JsonArray} but contains
   * more than a single element.
   */
  public short getAsShort() {
    throw new UnsupportedOperationException(getClass().getSimpleName());
  }

  /**
   * Returns a String representation of this element.
   */
  @Override
  public String toString() {
    try {
      StringWriter stringWriter = new StringWriter();
      JsonWriter jsonWriter = new JsonWriter(stringWriter);
      jsonWriter.setLenient(true);
      Streams.write(this, jsonWriter);
      return stringWriter.toString();
    } catch (IOException e) {
      throw new AssertionError(e);
    }
  }
}

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

import java.lang.reflect.Type;
import java.sql.Timestamp;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

/**
 * This type adapter supports three subclasses of date: Date, Timestamp, and
 * java.sql.Date.
 *
 * @author Inderjeet Singh
 * @author Joel Leitch
 */
final class DefaultDateTypeAdapter implements JsonSerializer<Date>, JsonDeserializer<Date> {

  // TODO: migrate to streaming adapter

  private final DateFormat enUsFormat;
  private final DateFormat localFormat;
  private final DateFormat iso8601Format;

  DefaultDateTypeAdapter() {
    this(DateFormat.getDateTimeInstance(DateFormat.DEFAULT, DateFormat.DEFAULT, Locale.US),
        DateFormat.getDateTimeInstance(DateFormat.DEFAULT, DateFormat.DEFAULT));
  }

  DefaultDateTypeAdapter(String datePattern) {
    this(new SimpleDateFormat(datePattern, Locale.US), new SimpleDateFormat(datePattern));
  }

  DefaultDateTypeAdapter(int style) {
    this(DateFormat.getDateInstance(style, Locale.US), DateFormat.getDateInstance(style));
  }

  public DefaultDateTypeAdapter(int dateStyle, int timeStyle) {
    this(DateFormat.getDateTimeInstance(dateStyle, timeStyle, Locale.US),
        DateFormat.getDateTimeInstance(dateStyle, timeStyle));
  }

  DefaultDateTypeAdapter(DateFormat enUsFormat, DateFormat localFormat) {
    this.enUsFormat = enUsFormat;
    this.localFormat = localFormat;
    this.iso8601Format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
    this.iso8601Format.setTimeZone(TimeZone.getTimeZone("UTC"));
  }

  // These methods need to be synchronized since JDK DateFormat classes are not thread-safe
  // See issue 162
  public JsonElement serialize(Date src, Type typeOfSrc, JsonSerializationContext context) {
    synchronized (localFormat) {
      String dateFormatAsString = enUsFormat.format(src);
      return new JsonPrimitive(dateFormatAsString);
    }
  }

  public Date deserialize(JsonElement json, Type typeOfT, JsonDeserializationContext context)
      throws JsonParseException {
    if (!(json instanceof JsonPrimitive)) {
      throw new JsonParseException("The date should be a string value");
    }
    Date date = deserializeToDate(json);
    if (typeOfT == Date.class) {
      return date;
    } else if (typeOfT == Timestamp.class) {
      return new Timestamp(date.getTime());
    } else if (typeOfT == java.sql.Date.class) {
      return new java.sql.Date(date.getTime());
    } else {
      throw new IllegalArgumentException(getClass() + " cannot deserialize to " + typeOfT);
    }
  }

  private Date deserializeToDate(JsonElement json) {
    synchronized (localFormat) {
      try {
        return localFormat.parse(json.getAsString());
      } catch (ParseException ignored) {
      }
      try {
        return enUsFormat.parse(json.getAsString());
      } catch (ParseException ignored) {
      }
      try {
        return iso8601Format.parse(json.getAsString());
      } catch (ParseException e) {
        throw new JsonSyntaxException(json.getAsString(), e);
      }
    }
  }

  @Override
  public String toString() {
    StringBuilder sb = new StringBuilder();
    sb.append(DefaultDateTypeAdapter.class.getSimpleName());
    sb.append('(').append(localFormat.getClass().getSimpleName()).append(')');
    return sb.toString();
  }
}

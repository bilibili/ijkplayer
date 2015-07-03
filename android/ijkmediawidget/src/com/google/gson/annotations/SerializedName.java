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

package com.google.gson.annotations;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * An annotation that indicates this member should be serialized to JSON with
 * the provided name value as its field name.
 *
 * <p>This annotation will override any {@link com.google.gson.FieldNamingPolicy}, including
 * the default field naming policy, that may have been set on the {@link com.google.gson.Gson}
 * instance.  A different naming policy can set using the {@code GsonBuilder} class.  See
 * {@link com.google.gson.GsonBuilder#setFieldNamingPolicy(com.google.gson.FieldNamingPolicy)}
 * for more information.</p>
 *
 * <p>Here is an example of how this annotation is meant to be used:</p>
 * <pre>
 * public class SomeClassWithFields {
 *   &#64SerializedName("name") private final String someField;
 *   private final String someOtherField;
 *
 *   public SomeClassWithFields(String a, String b) {
 *     this.someField = a;
 *     this.someOtherField = b;
 *   }
 * }
 * </pre>
 *
 * <p>The following shows the output that is generated when serializing an instance of the
 * above example class:</p>
 * <pre>
 * SomeClassWithFields objectToSerialize = new SomeClassWithFields("a", "b");
 * Gson gson = new Gson();
 * String jsonRepresentation = gson.toJson(objectToSerialize);
 * System.out.println(jsonRepresentation);
 *
 * ===== OUTPUT =====
 * {"name":"a","someOtherField":"b"}
 * </pre>
 *
 * <p>NOTE: The value you specify in this annotation must be a valid JSON field name.</p>
 *
 * @see com.google.gson.FieldNamingPolicy
 *
 * @author Inderjeet Singh
 * @author Joel Leitch
 */
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.FIELD)
public @interface SerializedName {

  /**
   * @return the desired name of the field when it is serialized
   */
  String value();
}

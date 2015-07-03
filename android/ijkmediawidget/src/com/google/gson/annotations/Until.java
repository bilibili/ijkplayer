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
 * An annotation that indicates the version number until a member or a type should be present.
 * Basically, if Gson is created with a version number that exceeds the value stored in the
 * {@code Until} annotation then the field will be ignored from the JSON output.  This annotation
 * is useful to manage versioning of your JSON classes for a web-service.
 *
 * <p>
 * This annotation has no effect unless you build {@link com.google.gson.Gson} with a
 * {@link com.google.gson.GsonBuilder} and invoke
 * {@link com.google.gson.GsonBuilder#setVersion(double)} method.
 *
 * <p>Here is an example of how this annotation is meant to be used:</p>
 * <pre>
 * public class User {
 *   private String firstName;
 *   private String lastName;
 *   &#64Until(1.1) private String emailAddress;
 *   &#64Until(1.1) private String password;
 * }
 * </pre>
 *
 * <p>If you created Gson with {@code new Gson()}, the {@code toJson()} and {@code fromJson()}
 * methods will use all the fields for serialization and deserialization. However, if you created
 * Gson with {@code Gson gson = new GsonBuilder().setVersion(1.2).create()} then the
 * {@code toJson()} and {@code fromJson()} methods of Gson will exclude the {@code emailAddress}
 * and {@code password} fields from the example above, because the version number passed to the 
 * GsonBuilder, {@code 1.2}, exceeds the version number set on the {@code Until} annotation,
 * {@code 1.1}, for those fields.
 *
 * @author Inderjeet Singh
 * @author Joel Leitch
 * @since 1.3
 */
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.FIELD, ElementType.TYPE})
public @interface Until {

  /**
   * the value indicating a version number until this member
   * or type should be ignored.
   */
  double value();
}

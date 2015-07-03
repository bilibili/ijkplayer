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
 * An annotation that indicates this member should be exposed for JSON
 * serialization or deserialization.
 *
 * <p>This annotation has no effect unless you build {@link com.google.gson.Gson}
 * with a {@link com.google.gson.GsonBuilder} and invoke
 * {@link com.google.gson.GsonBuilder#excludeFieldsWithoutExposeAnnotation()}
 * method.</p>
 *
 * <p>Here is an example of how this annotation is meant to be used:
 * <p><pre>
 * public class User {
 *   &#64Expose private String firstName;
 *   &#64Expose(serialize = false) private String lastName;
 *   &#64Expose (serialize = false, deserialize = false) private String emailAddress;
 *   private String password;
 * }
 * </pre></p>
 * If you created Gson with {@code new Gson()}, the {@code toJson()} and {@code fromJson()}
 * methods will use the {@code password} field along-with {@code firstName}, {@code lastName},
 * and {@code emailAddress} for serialization and deserialization. However, if you created Gson
 * with {@code Gson gson = new GsonBuilder().excludeFieldsWithoutExposeAnnotation().create()}
 * then the {@code toJson()} and {@code fromJson()} methods of Gson will exclude the
 * {@code password} field. This is because the {@code password} field is not marked with the
 * {@code @Expose} annotation. Gson will also exclude {@code lastName} and {@code emailAddress}
 * from serialization since {@code serialize} is set to {@code false}. Similarly, Gson will
 * exclude {@code emailAddress} from deserialization since {@code deserialize} is set to false.
 *
 * <p>Note that another way to achieve the same effect would have been to just mark the
 * {@code password} field as {@code transient}, and Gson would have excluded it even with default
 * settings. The {@code @Expose} annotation is useful in a style of programming where you want to
 * explicitly specify all fields that should get considered for serialization or deserialization.
 *
 * @author Inderjeet Singh
 * @author Joel Leitch
 */
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.FIELD)
public @interface Expose {
  
  /**
   * If {@code true}, the field marked with this annotation is written out in the JSON while
   * serializing. If {@code false}, the field marked with this annotation is skipped from the
   * serialized output. Defaults to {@code true}.
   * @since 1.4
   */
  public boolean serialize() default true;

  /**
   * If {@code true}, the field marked with this annotation is deserialized from the JSON.
   * If {@code false}, the field marked with this annotation is skipped during deserialization. 
   * Defaults to {@code true}.
   * @since 1.4
   */
  public boolean deserialize() default true;
}

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

/**
 * This exception is raised when Gson was unable to read an input stream
 * or write to one.
 * 
 * @author Inderjeet Singh
 * @author Joel Leitch
 */
public final class JsonIOException extends JsonParseException {
  private static final long serialVersionUID = 1L;

  public JsonIOException(String msg) {
    super(msg);
  }

  public JsonIOException(String msg, Throwable cause) {
    super(msg, cause);
  }

  /**
   * Creates exception with the specified cause. Consider using
   * {@link #JsonIOException(String, Throwable)} instead if you can describe what happened.
   *
   * @param cause root exception that caused this exception to be thrown.
   */
  public JsonIOException(Throwable cause) {
    super(cause);
  }
}

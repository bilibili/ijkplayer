/*
 * copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "ast_reference_type.hpp"
#include "ast_identifier.hpp"
#include "ast_class.hpp"
#include "ast__context.hpp"

using namespace ast;

bool ReferenceType::is_string_type()
{
    if (0 == get_name()->compare("String")) {
        if (0 == _get_java_long_name()->compare("java.lang.String")) {
            return true;
        }
    }

    return false;
}

bfc::string_ptr ReferenceType::_get_java_long_name()
{
    Identifier *Identifier = Context::instance()->find_identifier(this, get_name());
    if (Identifier) {
        if (get_is_array()) {
            std::ostringstream os;
            os << Identifier->get_java_long_name();
            os << "[]";
            return bfc::string::make_ptr(os);
        } else {
            return Identifier->get_java_long_name();
        }
    }

    return bfc::string::make_ptr("<unknown>");
}

bfc::string_ptr ReferenceType::get_c_type()
{
    if (get_is_array())
        return bfc::string::make_ptr("jobjectArray");

    if (is_string_type())
        return bfc::string::make_ptr("jstring");

    return bfc::string::make_ptr("jobject");
}

bfc::string_ptr ReferenceType::get_c_sign_in_method()
{
    Identifier::pointer_type Identifier = Context::instance()->find_identifier(this, get_name());
    if (Identifier) {
        std::ostringstream os;
        if (get_is_array())
            os << "[";
        os << "L" << Identifier->get_c_jni_sign() << ";";
        return bfc::string::make_ptr(os);
    }

    return bfc::string::make_ptr("<unknown>");
}

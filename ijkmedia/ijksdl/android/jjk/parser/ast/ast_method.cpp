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

#include "ast_method.hpp"
#include "ast_compilation_unit.hpp"

using namespace ast;


const int Method::FLAG_NORMAL        = 0x0000;
const int Method::FLAG_CATCH_ALL     = 0x0001;
const int Method::FLAG_WITH_C_STRING = 0x0002;
const int Method::FLAG_AS_GLOBAL_REF = 0x0004;
const int Method::FLAG_AS_C_BUFFER   = 0x0008;

bfc::string_ptr Method::get_c_jni_sign()
{
    std::ostringstream os;
    os << "(";

    ArgumentList::iterator begin = get_argument_list()->begin();
    ArgumentList::iterator end   = get_argument_list()->end();
    for (NULL; begin != end; ++begin) {
        os << (*begin)->get_type()->get_c_sign_in_method();
    }

    os << ")";
    os << get_type()->get_c_sign_in_method();
    return bfc::string::make_ptr(os);
}

bfc::string_ptr Method::get_c_jni_id_name()
{
    std::ostringstream os;
    os << "method_" << get_name();
    return bfc::string::make_ptr(os);
}

bfc::string_ptr Method::get_c_jni_method_name()
{
    return get_name();
}

bfc::string_ptr Method::get_c_call_api()
{
    if (is_static()) {
        return get_type()->get_c_call_static_method_api();
    } else {
        return get_type()->get_c_call_instance_method_api();
    }
}

bfc::string_ptr Method::get_c_call_object_id()
{
    if (get_c_call_need_this()) {
        return bfc::string::make_ptr("thiz");
    } else {
        return get_this_class()->get_c_jni_id();
    }
}

bfc::string_ptr Method::get_c_call_method_id()
{
    std::ostringstream os;
    os << get_this_class()->get_c_jni_class_instance() << "." << get_c_jni_id_name();
    return bfc::string::make_ptr(os);
}

bool Method::get_c_call_need_this()
{
    return !is_static();
}

bool Method::_has_string_arg()
{
    ArgumentList::iterator begin = get_argument_list()->begin();
    ArgumentList::iterator end   = get_argument_list()->end();
    for (NULL; begin != end; ++begin) {
        if ((*begin)->get_type()->is_string_type())
            return true;
    }

    return false;
}

void Method::_build_c_call_jni_statement(std::ostream &os, int flags)
{
    os << "(*env)->" << get_c_call_api();
    os << "(env, " << get_c_call_object_id();
    os << ", " << get_c_call_method_id();

    ArgumentList::iterator begin = get_argument_list()->begin();
    ArgumentList::iterator end   = get_argument_list()->end();
    for (NULL; begin != end; ++begin) {
        os << ", " << (*begin)->get_name();
    }

    os << ")";
}

void Method::_build_c_func_name(std::ostream &os, int flags)
{
    os << get_this_class()->get_c_class_name();
    os << "__";
    os << get_name();

    if (flags & FLAG_WITH_C_STRING) {
        os << "__withCString";
    }

    if (flags & FLAG_AS_C_BUFFER) {
        assert(get_type()->is_reference_type());
        os << "__asCBuffer";
    } else if (flags & FLAG_AS_GLOBAL_REF) {
        assert(get_type()->is_reference_type());
        os << "__asGlobalRef";
    }

    if (flags & FLAG_CATCH_ALL) {
        os << "__catchAll";
    }
}

void Method::_build_c_func_decl_statement(std::ostream &os, int flags)
{
    if (flags & FLAG_AS_C_BUFFER) {
        assert(get_type()->is_string_type());
        os << "const char *";
    } else {
        os << get_type()->get_c_type() << " ";
    }

    _build_c_func_name(os, flags);

    os << "(JNIEnv *env";

    if (get_c_call_need_this())
        os << ", jobject thiz";

    ArgumentList::iterator begin = get_argument_list()->begin();
    ArgumentList::iterator end   = get_argument_list()->end();
    for (NULL; begin != end; ++begin) {
        if (flags & FLAG_WITH_C_STRING && (*begin)->get_type()->is_string_type()) {
            os << ", const char *" << (*begin)->get_name() << "_cstr__";
        } else {
            os << ", " << (*begin)->get_type()->get_c_type();
            os << " "  << (*begin)->get_name();
        }
    }

    if (flags & FLAG_AS_C_BUFFER) {
        assert(get_type()->is_string_type());
        os << ", char *out_buf, int out_len";
    }

    os << ")";
}

void Method::_build_c_func_call_statement(std::ostream &os, int flags)
{
    _build_c_func_name(os, flags);

    os << "(env";

    if (get_c_call_need_this())
        os << ", thiz";

    ArgumentList::iterator begin = get_argument_list()->begin();
    ArgumentList::iterator end   = get_argument_list()->end();
    for (NULL; begin != end; ++begin) {
        if (flags & FLAG_WITH_C_STRING && (*begin)->get_type()->is_string_type()) {
            os << ", " << (*begin)->get_name() << "_cstr__";
        } else {
            os << ", " << (*begin)->get_name();   
        }
    }

    if (flags & FLAG_AS_C_BUFFER) {
        assert(get_type()->is_string_type());
        os << ", out_buf, out_len";
    }

    os << ")";
}

//@Override
void Method::build_c_func_decl(std::ostream &os)
{
    _build_c_func_decl_statement(os, FLAG_NORMAL);      os << ";" << std::endl;
    _build_c_func_decl_statement(os, FLAG_CATCH_ALL);   os << ";" << std::endl;
    if (get_type()->is_string_type()) {
        _build_c_func_decl_statement(os, FLAG_AS_GLOBAL_REF | FLAG_CATCH_ALL);
        os << ";" << std::endl;
        _build_c_func_decl_statement(os, FLAG_AS_C_BUFFER);
        os << ";" << std::endl;
        _build_c_func_decl_statement(os, FLAG_AS_C_BUFFER | FLAG_CATCH_ALL);
        os << ";" << std::endl;
    } else if (get_type()->is_reference_type()) {
        _build_c_func_decl_statement(os, FLAG_AS_GLOBAL_REF | FLAG_CATCH_ALL);
        os << ";" << std::endl;
    }

    if (_has_string_arg()) {
        _build_c_func_decl_statement(os, FLAG_NORMAL | FLAG_WITH_C_STRING);      os << ";" << std::endl;
        _build_c_func_decl_statement(os, FLAG_CATCH_ALL | FLAG_WITH_C_STRING);   os << ";" << std::endl;
        if (get_type()->is_string_type()) {
            _build_c_func_decl_statement(os, FLAG_AS_GLOBAL_REF | FLAG_CATCH_ALL | FLAG_WITH_C_STRING);
            os << ";" << std::endl;
            _build_c_func_decl_statement(os, FLAG_AS_C_BUFFER | FLAG_WITH_C_STRING);
            os << ";" << std::endl;
            _build_c_func_decl_statement(os, FLAG_AS_C_BUFFER | FLAG_CATCH_ALL | FLAG_WITH_C_STRING);
            os << ";" << std::endl;
        } else if (get_type()->is_reference_type()) {
            _build_c_func_decl_statement(os, FLAG_AS_GLOBAL_REF | FLAG_CATCH_ALL | FLAG_WITH_C_STRING);
            os << ";" << std::endl;
        }        
    }
}

//@Override
void Method::build_c_member_id_decl(std::ostream &os)
{
    os << build_indent() << "jmethodID " << get_c_jni_id_name() << ";" << std::endl;
}

//@Override
void Method::build_c_member_id_load(std::ostream &os)
{
    os << std::endl;
    os << build_indent() << "class_id = " << get_this_class()->get_c_jni_id() << ";\n";
    os << build_indent() << "name     = \"" << get_c_jni_method_name() << "\";\n";
    os << build_indent() << "sign     = \"" << get_c_jni_sign() << "\";\n";
    os << build_indent() << get_c_jni_id() << " = " << (is_static() ? "JJK_GetStaticMethodID__catchAll" : "JJK_GetMethodID__catchAll")
                            << "(env, class_id, name, sign);\n";
    os << build_indent() << "if (" << get_c_jni_id() << " == NULL)\n";
    os << build_indent() << "    goto fail;\n";
}

void Method::_build_c_func_string_argument_cast_statements(std::ostream &os, int flags)
{
    ArgumentList::iterator begin = get_argument_list()->begin();
    ArgumentList::iterator end   = get_argument_list()->end();
    for (NULL; begin != end; ++begin) {
        if (!(*begin)->get_type()->is_string_type())
            continue;
        os << "    jstring " << (*begin)->get_name() << " = NULL;" << std::endl;
    }

    os << std::endl;

    begin = get_argument_list()->begin();
    end   = get_argument_list()->end();
    for (NULL; begin != end; ++begin) {
        if (!(*begin)->get_type()->is_string_type())
            continue;
        bfc::string_ptr name = (*begin)->get_name();

        os << "    " << name << " = (*env)->NewStringUTF(env, " << name << "_cstr__);" << std::endl;
        if (flags & FLAG_CATCH_ALL) {
            os << "    if (JJK_ExceptionCheck__catchAll(env) || !" << name << ")" << std::endl;
        } else {
            os << "    if (JJK_ExceptionCheck__throwAny(env) || !" << name << ")" << std::endl;
        }
        os << "        goto fail;" << std::endl;
    }
}

void Method::_build_c_func_string_argument_release_statements(std::ostream &os, int flags)
{
    ArgumentList::iterator begin = get_argument_list()->begin();
    ArgumentList::iterator end   = get_argument_list()->end();
    for (NULL; begin != end; ++begin) {
        if (!(*begin)->get_type()->is_string_type())
            continue;
        bfc::string_ptr name = (*begin)->get_name();

        os << "    JJK_DeleteLocalRef__p(env, &" << name << ");" << std::endl;
    }
}

void Method::_build_c_func_impl_void_type_statement(std::ostream &os, int flags)
{
    assert(get_type());
    assert(get_type()->is_void_type());

    os << std::endl;
    _build_c_func_decl_statement(os, flags); os << std::endl;
    os << "{" << std::endl;
    if (flags & FLAG_WITH_C_STRING) {
        assert(_has_string_arg());
        _build_c_func_string_argument_cast_statements(os, flags);
        os << std::endl;
        os << "    "; _build_c_func_call_statement(os, flags & ~FLAG_WITH_C_STRING); os << ";" << std::endl;
        os << std::endl;
        os << "fail:" << std::endl;
        _build_c_func_string_argument_release_statements(os, flags);
    } else if (flags & FLAG_CATCH_ALL) {
        os << "    "; _build_c_func_call_statement(os, flags & ~FLAG_CATCH_ALL); os << ";" << std::endl;
        os << "    JJK_ExceptionCheck__catchAll(env);" << std::endl;
    } else {
        os << "    "; _build_c_call_jni_statement(os, flags); os << ";" << std::endl;
    }
    os << "}" << std::endl;
}

void Method::_build_c_func_impl_basic_type_statement(std::ostream &os, int flags)
{
    assert(get_type());
    assert(!get_type()->is_void_type());
    assert(!get_type()->is_reference_type());

    os << std::endl;
    _build_c_func_decl_statement(os, flags); os << "\n";
    os << "{" << std::endl;
    if (flags & FLAG_WITH_C_STRING) {
        assert(_has_string_arg());
        os << "    " << get_type()->get_c_type() << " ret_value = " << get_type()->get_c_default_value() << ";" << std::endl;
        _build_c_func_string_argument_cast_statements(os, flags);
        os << std::endl;
        os << "    ret_value = "; _build_c_func_call_statement(os, flags & ~FLAG_WITH_C_STRING); os << ";" << std::endl;
        if (flags & FLAG_CATCH_ALL) {
            os << "    if (JJK_ExceptionCheck__catchAll(env)) {" << std::endl;
        } else {
            os << "    if (JJK_ExceptionCheck__throwAny(env)) {" << std::endl;
        }
        os << "        ret_value = " << get_type()->get_c_default_value() << ";" << std::endl;
        os << "        goto fail;" << std::endl;
        os << "    }" << std::endl;
        os << std::endl;
        os << "fail:" << std::endl;
        _build_c_func_string_argument_release_statements(os, flags);
        os << "    return ret_value;" << std::endl;
    } else if (flags & FLAG_CATCH_ALL) {
        os << "    " << get_type()->get_c_type() << " ret_value = "; _build_c_func_call_statement(os, flags & ~FLAG_CATCH_ALL); os << ";" << std::endl;
        os << "    if (JJK_ExceptionCheck__catchAll(env)) {" << std::endl;
        os << "        return " << get_type()->get_c_default_value() << ";" << std::endl;
        os << "    }" << std::endl;
        os << std::endl;
        os << "    return ret_value;" << std::endl;
    } else {
        os << "    return "; _build_c_call_jni_statement(os, flags); os << ";" << std::endl;
    }
    os << "}" << std::endl;
}

void Method::_build_c_func_impl_reference_type_statement(std::ostream &os, int flags)
{
    assert(get_type());
    assert(get_type()->is_reference_type());

    os << std::endl;
    _build_c_func_decl_statement(os, flags); os << std::endl;
    os << "{" << std::endl;
    if (flags & FLAG_AS_C_BUFFER) {
        assert(get_type()->is_string_type());
        os << "    const char *ret_value = NULL;" << std::endl;
        os << "    const char *c_str     = NULL;" << std::endl;
        os << "    " << get_type()->get_c_type() << " local_string = "; _build_c_func_call_statement(os, flags & ~(FLAG_AS_GLOBAL_REF | FLAG_AS_C_BUFFER)); os << ";" << std::endl;
        if (flags & FLAG_CATCH_ALL) {
            os << "    if (JJK_ExceptionCheck__catchAll(env) || !local_string) {" << std::endl;
        } else {
            os << "    if (JJK_ExceptionCheck__throwAny(env) || !local_string) {" << std::endl;
        }
        os << "        goto fail;" << std::endl;
        os << "    }" << std::endl;
        os << std::endl;
        os << "    c_str = (*env)->GetStringUTFChars(env, local_string, NULL );" << std::endl;
        if (flags & FLAG_CATCH_ALL) {
            os << "    if (JJK_ExceptionCheck__catchAll(env) || !c_str) {" << std::endl;
        } else {
            os << "    if (JJK_ExceptionCheck__throwAny(env) || !c_str) {" << std::endl;
        }
        os << "        goto fail;" << std::endl;
        os << "    }" << std::endl;
        os << std::endl;
        os << "    strlcpy(out_buf, c_str, out_len);" << std::endl;
        os << "    ret_value = out_buf;" << std::endl;
        os << std::endl;
        os << "fail:" << std::endl;
        os << "    JJK_ReleaseStringUTFChars__p(env, local_string, &c_str);" << std::endl;
        os << "    JJK_DeleteLocalRef__p(env, &local_string);" << std::endl;
        os << "    return ret_value;" << std::endl;
    } else if (flags & FLAG_AS_GLOBAL_REF) {
        assert(get_type()->is_reference_type());
        os << "    " << get_type()->get_c_type() << " ret_object   = " << get_type()->get_c_default_value() << ";" << std::endl;
        os << "    " << get_type()->get_c_type() << " local_object = "; _build_c_func_call_statement(os, flags & ~FLAG_AS_GLOBAL_REF); os << ";" << std::endl;
        if (flags & FLAG_CATCH_ALL) {
            os << "    if (JJK_ExceptionCheck__catchAll(env) || !local_object) {" << std::endl;
        } else {
            os << "    if (JJK_ExceptionCheck__throwAny(env) || !local_object) {" << std::endl;
        }
        os << "        ret_object = " << get_type()->get_c_default_value() << ";" << std::endl;
        os << "        goto fail;" << std::endl;
        os << "    }" << std::endl;
        os << std::endl;
        os << "    ret_object = JJK_NewGlobalRef__catchAll(env, local_object);\n";
        os << "    if (!ret_object) {" << std::endl;
        os << "        ret_object = " << get_type()->get_c_default_value() << ";" << std::endl;
        os << "        goto fail;" << std::endl;
        os << "    }" << std::endl;
        os << std::endl;
        os << "fail:" << std::endl;
        os << "    JJK_DeleteLocalRef__p(env, &local_object);\n";
        os << "    return ret_object;\n";
    } else if (flags & FLAG_WITH_C_STRING) {
        assert(_has_string_arg());
        os << "    " << get_type()->get_c_type() << " ret_object = " << get_type()->get_c_default_value() << ";" << std::endl;
        _build_c_func_string_argument_cast_statements(os, flags);
        os << std::endl;
        os << "    ret_object = "; _build_c_func_call_statement(os, flags & ~FLAG_WITH_C_STRING); os << ";" << std::endl;
        if (flags & FLAG_CATCH_ALL) {
            os << "    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {" << std::endl;
        } else {
            os << "    if (JJK_ExceptionCheck__throwAny(env) || !ret_object) {" << std::endl;
        }
        os << "        ret_object = " << get_type()->get_c_default_value() << ";" << std::endl;
        os << "        goto fail;" << std::endl;
        os << "    }" << std::endl;
        os << std::endl;
        os << "fail:" << std::endl;
        _build_c_func_string_argument_release_statements(os, flags);
        os << "    return ret_object;" << std::endl;
    } else if (flags & FLAG_CATCH_ALL) {
        os << "    " << get_type()->get_c_type() << " ret_object = "; _build_c_func_call_statement(os, flags & ~FLAG_CATCH_ALL); os << ";" << std::endl;
        os << "    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {" << std::endl;
        os << "        return " << get_type()->get_c_default_value() << ";" << std::endl;
        os << "    }" << std::endl;
        os << std::endl;
        os << "    return ret_object;" << std::endl;
    } else {
        os << "    return "; _build_c_call_jni_statement(os, flags); os << ";" << std::endl;
    }
    os << "}" << std::endl;
}

void Method::build_c_func_impl(std::ostream &os)
{
    if (get_type()->is_string_type()) {
        _build_c_func_impl_reference_type_statement(os, FLAG_NORMAL);
        _build_c_func_impl_reference_type_statement(os, FLAG_CATCH_ALL);
        _build_c_func_impl_reference_type_statement(os, FLAG_AS_GLOBAL_REF | FLAG_CATCH_ALL);
        _build_c_func_impl_reference_type_statement(os, FLAG_AS_C_BUFFER);
        _build_c_func_impl_reference_type_statement(os, FLAG_AS_C_BUFFER | FLAG_CATCH_ALL);
    } else if (get_type()->is_reference_type()) {
        _build_c_func_impl_reference_type_statement(os, FLAG_NORMAL);
        _build_c_func_impl_reference_type_statement(os, FLAG_CATCH_ALL);
        _build_c_func_impl_reference_type_statement(os, FLAG_AS_GLOBAL_REF | FLAG_CATCH_ALL);
    } else if (get_type()->is_void_type()) {
        _build_c_func_impl_void_type_statement(os, FLAG_NORMAL);
        _build_c_func_impl_void_type_statement(os, FLAG_CATCH_ALL);
    } else {
        _build_c_func_impl_basic_type_statement(os, FLAG_NORMAL);
        _build_c_func_impl_basic_type_statement(os, FLAG_CATCH_ALL);
    }

    if (_has_string_arg()) {
        if (get_type()->is_string_type()) {
            _build_c_func_impl_reference_type_statement(os, FLAG_NORMAL | FLAG_WITH_C_STRING);
            _build_c_func_impl_reference_type_statement(os, FLAG_CATCH_ALL | FLAG_WITH_C_STRING);
            _build_c_func_impl_reference_type_statement(os, FLAG_AS_GLOBAL_REF | FLAG_CATCH_ALL | FLAG_WITH_C_STRING);
            _build_c_func_impl_reference_type_statement(os, FLAG_AS_C_BUFFER | FLAG_WITH_C_STRING);
            _build_c_func_impl_reference_type_statement(os, FLAG_AS_C_BUFFER | FLAG_CATCH_ALL | FLAG_WITH_C_STRING);
        } else if (get_type()->is_reference_type()) {
            _build_c_func_impl_reference_type_statement(os, FLAG_NORMAL | FLAG_WITH_C_STRING);
            _build_c_func_impl_reference_type_statement(os, FLAG_CATCH_ALL | FLAG_WITH_C_STRING);
            _build_c_func_impl_reference_type_statement(os, FLAG_AS_GLOBAL_REF | FLAG_CATCH_ALL | FLAG_WITH_C_STRING);
        } else if (get_type()->is_void_type()) {
            _build_c_func_impl_void_type_statement(os, FLAG_NORMAL | FLAG_WITH_C_STRING);
            _build_c_func_impl_void_type_statement(os, FLAG_CATCH_ALL | FLAG_WITH_C_STRING);
        } else {
            _build_c_func_impl_basic_type_statement(os, FLAG_NORMAL | FLAG_WITH_C_STRING);
            _build_c_func_impl_basic_type_statement(os, FLAG_CATCH_ALL | FLAG_WITH_C_STRING);
        }      
    }
}

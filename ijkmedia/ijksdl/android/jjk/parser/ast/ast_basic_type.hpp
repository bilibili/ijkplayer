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

#ifndef JJK_PARSER_AST__AST_BASIC_TYPE__HPP
#define JJK_PARSER_AST__AST_BASIC_TYPE__HPP

#include "ast__def.hpp"
#include "ast_type.hpp"

NAMESPACE_AST_BEGIN

class BasicType: public Type
{
public:
    // @Override
    virtual bool is_reference_type() override {return get_is_array();}
    virtual bool is_void_type() override {return m_token == T_VOID;}
    virtual bool is_string_type() override {return false;}

private:
    const int m_token;

public:
    AST_IMPLEMENT_ABSTRACT(BasicType);
protected:
    explicit BasicType(const bfc::string_ptr &name, int token): Type(name), m_token(token) {;}
    explicit BasicType(BasicType *other): Type(other), m_token(other->m_token) {;}
};

#define AST_DEFINE__BasicType(class__, token_id__, name__, ctype__, c_sign__, c_name_in_call_api, default__) \
class class__: public BasicType \
{ \
public: \
    virtual bfc::string_ptr get_c_type() override { \
        if (get_is_array()) \
            return bfc::string::make_ptr(ctype__ "Array"); \
        else \
            return bfc::string::make_ptr(ctype__); \
    } \
    virtual bfc::string_ptr get_c_sign_in_method() override { \
        if (get_is_array()) { \
            std::ostringstream os; \
            os << "[" << c_sign__; \
            return bfc::string::make_ptr(os); \
        } else \
            return bfc::string::make_ptr(c_sign__); \
    } \
    virtual bfc::string_ptr get_c_default_value() override { \
        if (get_is_array()) { \
            return bfc::string::make_ptr("NULL"); \
        } else \
            return bfc::string::make_ptr(default__); \
    } \
\
    virtual bfc::string_ptr get_c_call_instance_method_api() override { \
        std::ostringstream os; \
        os << "Call" << c_name_in_call_api << (get_is_array() ? "ArrayMethod" : "Method"); \
        return bfc::string::make_ptr(os); \
    } \
    virtual bfc::string_ptr get_c_call_static_method_api() override { \
        std::ostringstream os; \
        os << "CallStatic" << c_name_in_call_api << (get_is_array() ? "ArrayMethod" : "Method"); \
        return bfc::string::make_ptr(os); \
    } \
    virtual bfc::string_ptr get_c_get_instance_field_api() override { \
        std::ostringstream os; \
        os << "Get" << c_name_in_call_api << (get_is_array() ? "ArrayField" : "Field"); \
        return bfc::string::make_ptr(os); \
    } \
    virtual bfc::string_ptr get_c_get_static_field_api() override { \
        std::ostringstream os; \
        os << "GetStatic" << c_name_in_call_api << (get_is_array() ? "ArrayField" : "Field"); \
        return bfc::string::make_ptr(os); \
    } \
    virtual bfc::string_ptr get_c_set_instance_field_api() override { \
        std::ostringstream os; \
        os << "Set" << c_name_in_call_api << (get_is_array() ? "ArrayField" : "Field"); \
        return bfc::string::make_ptr(os); \
    } \
    virtual bfc::string_ptr get_c_set_static_field_api() override { \
        std::ostringstream os; \
        os << "SetStatic" << c_name_in_call_api << (get_is_array() ? "ArrayField" : "Field"); \
        return bfc::string::make_ptr(os); \
    } \
\
public: \
    AST_IMPLEMENT(class__); \
protected: \
    explicit class__(): BasicType(bfc::string::make_ptr(name__), token_id__) {;} \
    explicit class__(class__ *other): BasicType(other) {;} \
public: \
    static pointer_type make_ptr() {return pointer_type(new class__());} \
};

AST_DEFINE__BasicType(BooleanType,  T_BOOLEAN,  "boolean",  "jboolean", "Z", "Boolean", "false");
AST_DEFINE__BasicType(ByteType,     T_BYTE,     "byte",     "jbyte",    "B", "Byte",    "0");
AST_DEFINE__BasicType(FloatType,    T_FLOAT,    "float",    "jfloat",   "F", "Float",   "0");
AST_DEFINE__BasicType(IntType,      T_INT,      "int",      "jint",     "I", "Int",     "0");
AST_DEFINE__BasicType(LongType,     T_LONG,     "long",     "jlong",    "J", "Long",    "0");
AST_DEFINE__BasicType(VoidType,     T_VOID,     "void",     "void",     "V", "Void",    "");

NAMESPACE_AST_END

#endif

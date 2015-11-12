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

#ifndef JJK_PARSER_AST__AST_TYPE__HPP
#define JJK_PARSER_AST__AST_TYPE__HPP

#include "ast__def.hpp"
#include "ast_identifier.hpp"

NAMESPACE_AST_BEGIN

class Type: public Identifier
{
public:
    AST_PROPERTY_DEFINE(bool, is_array);

    virtual bool is_reference_type() = 0;
    virtual bool is_void_type() = 0;
    virtual bool is_string_type() = 0;

    AST_GETTER_DECL_0(bfc::string_ptr, c_type);
    AST_GETTER_DECL_0(bfc::string_ptr, c_sign_in_method);
    AST_GETTER_DECL_0(bfc::string_ptr, c_default_value);

    AST_GETTER_DECL_0(bfc::string_ptr, c_call_instance_method_api);
    AST_GETTER_DECL_0(bfc::string_ptr, c_call_static_method_api);
    AST_GETTER_DECL_0(bfc::string_ptr, c_get_instance_field_api);
    AST_GETTER_DECL_0(bfc::string_ptr, c_get_static_field_api);
    AST_GETTER_DECL_0(bfc::string_ptr, c_set_instance_field_api);
    AST_GETTER_DECL_0(bfc::string_ptr, c_set_static_field_api);

public:
    // @Override
    virtual void debug_print(int indent) override {
        if (get_is_array()) {
            printf("%s[]", get_name()->c_str());
        } else {
            printf("%s", get_name()->c_str());
        }
    }

public:
    AST_IMPLEMENT_ABSTRACT(Type);
protected:
    explicit Type(const bfc::string_ptr &name): Identifier(name) {init(NULL);}
    explicit Type(Node *other): Identifier(other) {init(other);}
private:
    void init(Node *other) {
        set_is_array(false);

        if (other) {
            Type *other_ = dynamic_cast<Type *>(other);
            if (other_)
                set_is_array(other_->get_is_array());
        }
    }
};

NAMESPACE_AST_END

#endif

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

#ifndef JJK_PARSER_AST__AST_PROPERTY_ACCESSOR__HPP
#define JJK_PARSER_AST__AST_PROPERTY_ACCESSOR__HPP

#include "ast__def.hpp"
#include "ast_method.hpp"
#include "ast_field.hpp"
#include "ast_basic_type.hpp"

NAMESPACE_AST_BEGIN

class PropertyAccessor: public Method
{
public:
    AST_PROPERTY_DEFINE(Field*, field);

private:
    void _init_with_field(Field *field) {
        set_annotations(field->get_annotations());
        set_modifier_set(field->get_modifier_set());
        set_parent(field->get_parent());
    }

public:
    AST_IMPLEMENT_ABSTRACT(PropertyAccessor);
protected:
    explicit PropertyAccessor(const bfc::string_ptr &name, Field *field): Method(name), AST_PROPERTY(field)(field) {_init_with_field(field);}
};




class PropertyGetter: public PropertyAccessor
{
public:
    AST_IMPLEMENT(PropertyGetter);

    AST_GETTER_DECL_OVERRIDE(bfc::string_ptr,   c_call_api);
    AST_GETTER_DECL_OVERRIDE(bfc::string_ptr,   c_call_method_id);

private:
    void _init_with_field(Field *field) {
        set_type(field->get_type());
        set_argument_list(ArgumentList::make_ptr().get());
    }

protected:
    explicit PropertyGetter(const bfc::string_ptr &name, Field *field): PropertyAccessor(name, field) {_init_with_field(field);}
public:
    static pointer_type make_ptr(Field *field)
    {
        std::ostringstream os;
        os << field->get_name() << "__get";
        bfc::string_ptr name = bfc::string::make_ptr(os);

        return pointer_type(new PropertyGetter(name, field));
    }
};



class PropertySetter: public PropertyAccessor
{
public:
    AST_IMPLEMENT(PropertySetter);

    AST_GETTER_DECL_OVERRIDE(bfc::string_ptr,   c_call_api);
    AST_GETTER_DECL_OVERRIDE(bfc::string_ptr,   c_call_method_id);

protected:
    explicit PropertySetter(const bfc::string_ptr &name, Field *field): PropertyAccessor(name, field) {_init_with_field(field);}
private:
    void _init_with_field(Field *field) {
        set_type(VoidType::make_ptr());

        Argument::pointer_type argument = Argument::make_ptr(bfc::string::make_ptr("value"));
        argument->set_type(field->get_type());

        ArgumentList::pointer_type argument_list = ArgumentList::make_ptr();
        argument_list->push_back(argument);

        set_argument_list(argument_list);
    }
public:
    static pointer_type make_ptr(Field *field)
    {
        std::ostringstream os;
        os << field->get_name() << "__set";
        bfc::string_ptr name = bfc::string::make_ptr(os);

        return pointer_type(new PropertySetter(name, field));
    }
};

NAMESPACE_AST_END

#endif

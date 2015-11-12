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

#ifndef JJK_PARSER_AST__AST_ARGUMENT__HPP
#define JJK_PARSER_AST__AST_ARGUMENT__HPP

#include "ast__def.hpp"
#include "ast_type.hpp"

NAMESPACE_AST_BEGIN

class Argument: public Identifier
{
public:
    AST_CHILD_DEFINE(Type, type);

public:
    // @Override
    virtual void debug_print(int indent) override {
        get_type()->debug_print(0);
        printf(" %s", get_name()->c_str());
    }

public:
    AST_IMPLEMENT(Argument);
protected:
    explicit Argument(const bfc::string_ptr &name): Identifier(name) {;}
public:
    static pointer_type make_ptr(const bfc::string_ptr& name) {return pointer_type(new Argument(name));}
};

class ArgumentList: public NodeList<Argument>
{
public:
    // @Override
    virtual void debug_print(int indent) override {
        iterator begin = this->begin();
        iterator end   = this->end();

        if (begin != end) {
            (*begin)->debug_print(indent);
            ++begin;

            for (NULL; begin != end; ++begin) {
                printf(", ");
                (*begin)->debug_print(indent);
            }
        }
    }

public:
    AST_IMPLEMENT_ABSTRACT(ArgumentList);
protected:
    explicit ArgumentList() {;}
public:
    static pointer_type make_ptr() {return pointer_type(new ArgumentList());}
};

NAMESPACE_AST_END

#endif

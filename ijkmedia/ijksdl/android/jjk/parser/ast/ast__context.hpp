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

#ifndef JJK_PARSER_AST__AST__CONTEXT__HPP
#define JJK_PARSER_AST__AST__CONTEXT__HPP

#include "ast__def.hpp"
#include "ast__namespace.hpp"
#include "ast_identifier.hpp"
#include <map>
#include <string>

NAMESPACE_AST_BEGIN

class QualifiedIdentifier;
class Class;

class Context
{
public:
    typedef std::vector<Namespace*> NamespaceStack;

    AST_PROPERTY_DEFINE(std::string,        h_file_path);
    AST_PROPERTY_DEFINE(std::string,        c_file_path);
    AST_PROPERTY_DEFINE(std::string,        java_class_dir);
    AST_PROPERTY_DEFINE(Namespace*,         global_name_space);

    Identifier *find_identifier(Node *node, const bfc::string_ptr& name);

    Namespace *get_local_namespace();
    void push_local_namespace(Namespace *local_namespace);
    void pop_local_namespace();

    bfc::string_ptr get_sign_in_method(bfc::string_ptr name);

private:
    Context() {
        Namespace *global_namespace = new Namespace();
        global_namespace->add_class_identifier(Identifier::parse("java.lang.Object"));
        global_namespace->add_class_identifier(Identifier::parse("java.lang.String"));
        set_global_name_space(global_namespace);
    }

public:
    static Context *instance() {
        if (!s_instance)
            s_instance = new Context();
        return s_instance;
    }

private:
    NamespaceStack m_namespace_stack;
    static Context *s_instance;
};

NAMESPACE_AST_END

#endif

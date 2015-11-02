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

#include "ast__context.hpp"

#include "ast.hpp"

using namespace ast;

Context *Context::s_instance;

Namespace *Context::get_local_namespace()
{
    assert(!m_namespace_stack.empty());
    return m_namespace_stack.back();
}

void Context::push_local_namespace(Namespace *local_namespace)
{
    m_namespace_stack.push_back(local_namespace);
}

void Context::pop_local_namespace()
{
    assert(!m_namespace_stack.empty());
    m_namespace_stack.pop_back();
}

Identifier *Context::find_identifier(Node *node, const bfc::string_ptr& name)
{
    Identifier *qid = NULL;

    CompilationUnit *this_compilation_unit = node->get_this_compilation_unit();
    if (this_compilation_unit) {
        // before AST build
        Class *this_class = node->get_this_class();
        while (this_class) {
            qid = this_class->get_local_namespace()->find_identifier(name);
            if (qid)
                return qid;

            if (!this_class->get_parent())
                break;

            this_class = this_class->get_parent()->get_this_class();
        }

        qid = this_compilation_unit->get_local_namespace()->find_identifier(name);
        if (qid)
            return qid;
    } else {
        // after AST build
        NamespaceStack::reverse_iterator rbegin = m_namespace_stack.rbegin();
        NamespaceStack::reverse_iterator rend   = m_namespace_stack.rend();

        for (NULL; rbegin != rend; ++rbegin) {
            qid = (*rbegin)->find_identifier(name);
            if (qid)
                return qid;
        }
    }

    qid = get_global_name_space()->find_identifier(name);
    return qid;
}

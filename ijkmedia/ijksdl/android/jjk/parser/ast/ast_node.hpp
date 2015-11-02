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

#ifndef JJK_PARSER_AST__AST_NODE__HPP
#define JJK_PARSER_AST__AST_NODE__HPP

#include <list>
#include <typeinfo>
#include "ast__def.hpp"

NAMESPACE_AST_BEGIN

class File;
class Class;
class Method;

class IContext
{
public:
};

class Node: public bfc::object
{
public:
    AST_PROPERTY_DEFINE(Node*,              parent);
    AST_GETTER_DECL(Class*,                 this_class);
    AST_GETTER_DECL(CompilationUnit*,       this_compilation_unit);
    AST_GETTER_DECL(Method*,                this_method);
    AST_GETTER_DECL(Identifier*,            this_package);

    bfc::string_ptr quote_string(const char *str) {
        std::ostringstream os;
        os << "\"" << str << "\"";
        return bfc::string::make_ptr(os);
    }

public:
    // output
    virtual void debug_print(int indent) {;}

    void print_space(int width) {
        for (int i = 0; i < width; ++i) {
            printf(" ");
        }
    }

public:
    AST_IMPLEMENT_ABSTRACT(Node);
    virtual Node::pointer_type _make_clone_node() {return NULL;}
protected:
    explicit Node() {init(NULL);}
    explicit Node(Node *other) {init(other);}
private:
    void init(Node *other) {
        if (other) {
            set_parent(other->get_parent());
        }
    }
};



template <class T>
class NodeList: public Node
{
public:
    typedef T                                           element_type;
    typedef typename bfc::rc_ptr<element_type>          element_pointer_type;
    typedef typename std::list<element_pointer_type>    container;
    typedef typename container::iterator                iterator;

    iterator begin() {return m_node_list.begin();}
    iterator end()   {return m_node_list.end();}

    element_pointer_type back() {return m_node_list.back();}

    void    push_back(element_type        *node)    {m_node_list.push_back(node);       node->set_parent(this);}
    void    push_back(element_pointer_type node)    {m_node_list.push_back(node.get()); node->set_parent(this);}

    // FIXME: deep copy
    void    assign(NodeList<T> *node_list) {m_node_list = node_list->m_node_list;}
    size_t  size() {return m_node_list.size();}
    bool    empty() {return m_node_list.empty();}

private:
    container m_node_list;

public:
    AST_IMPLEMENT_ABSTRACT(NodeList);
protected:
    explicit NodeList() {;}
    explicit NodeList(NodeList *other): Node(other) {
        iterator begin = this->begin();
        iterator end   = this->end();

        for (NULL; begin != end; ++begin) {
            element_pointer_type node = *begin;
            if (!node.is_null())
                continue;

            push_back((T*)node.get());
        }
    }
public:
    // static pointer_type make_ptr() {return pointer_type(new class__());}
};

NAMESPACE_AST_END

#endif

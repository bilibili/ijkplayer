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

#ifndef JJK_PARSER_AST__AST_QUALIFIED_IDNETIFIER__HPP
#define JJK_PARSER_AST__AST_QUALIFIED_IDNETIFIER__HPP

#include "ast_node.hpp"

NAMESPACE_AST_BEGIN

class Identifier: public Node
{
public:
    AST_IMPLEMENT(Identifier);

    AST_PROPERTY_DEFINE(pointer_type,   prefix);
    AST_GETTER_DEFINE(bfc::string_ptr,  name);

    AST_GETTER_DECL(pointer_type,       outer_class_identifier);
    AST_GETTER_DECL(bfc::string_ptr,    java_long_name);
    AST_GETTER_DECL(bfc::string_ptr,    c_long_name);
    AST_GETTER_DECL(bfc::string_ptr,    c_jni_sign);
    AST_GETTER_DECL(bfc::string_ptr,    fs_long_path);

    virtual bool is_class_identifier() {return false;}

public:
    virtual void debug_print(int indent) override {
        if (get_prefix()) {
            get_prefix()->debug_print(indent);
            printf(".\n");
        }
        printf("%s", get_name()->c_str());
    }

protected:
    explicit Identifier(const bfc::string_ptr &name): AST_PROPERTY(name)(name) {init(NULL);}
    explicit Identifier(Node *other): Node(other) {init(other);}
private:
    void init(Node *other) {
        if (other) {
            Identifier *other_ = dynamic_cast<Identifier *>(other);
            if (other_) {
                AST_PROPERTY(name) = other_->get_name();
                set_prefix(other_->get_prefix());
            }
        }
    }
public:
    static pointer_type make_ptr(const bfc::string_ptr &name) {return pointer_type(new Identifier(name));}

    pointer_type make_by_join(const bfc::string_ptr &child_name) {
        pointer_type identifier = make_ptr(child_name);
        identifier->set_prefix(this);
        return identifier;
    }

    static pointer_type parse(const char *long_name, char separator = '.') {
        pointer_type identifier;

        const char *lookup = long_name;
        const char *next   = strchr(lookup, separator);
        while (next && next > lookup) {
            bfc::string_ptr child_id = bfc::string::make_ptr(lookup, next - lookup);

            if (identifier) {
                identifier = identifier->make_by_join(child_id);
            } else {
                identifier = make_ptr(child_id);    
            }

            lookup = next + 1;
            next   = strchr(lookup, separator);
        }

        if (*lookup) {
            bfc::string_ptr child_id = bfc::string::make_ptr(lookup);

            if (identifier) {
                identifier = identifier->make_by_join(child_id);
            } else {
                identifier = make_ptr(child_id);    
            }
        }

        return identifier;
    }
};



template <class T>
class IdentifierMap: public Node
{
public:
    typedef bfc::string_ptr                                     key_type;
    typedef T                                                   element_type;
    typedef typename bfc::rc_ptr<element_type>                  element_pointer_type;
    typedef typename std::map<key_type, element_pointer_type>   container;
    typedef typename container::iterator                        iterator;

    iterator begin() {return m_node_map.begin();}
    iterator end()   {return m_node_map.end();}

    void insert(element_type        *node)  {_insert(node);}
    void insert(element_pointer_type node)  {_insert(node.get());}

    // void assign(NamedNodeMap *node_map) {m_node_map = node_map->m_node_map;}
    size_t size() {return m_node_map.size();}

    element_pointer_type get_at(const char *name)               {return _get_at(bfc::string::make_ptr(name));}
    element_pointer_type get_at(const bfc::string_ptr &name)    {return _get_at(name);}

private:
    container m_node_map;

private:
    element_pointer_type _get_at(const bfc::string_ptr &name) {
        iterator find = m_node_map.find(name);
        if (find == m_node_map.end())
            return element_pointer_type(NULL);
        return find->second;
    }

    void _insert(T *node) {
        bfc::string_ptr name = node->get_name();

        iterator find = m_node_map.find(name);
        assert(find == m_node_map.end());
        if (find != m_node_map.end()) {
            if (find->second)
                find->second->set_parent(NULL);
            find->second = node;
        } else {
            m_node_map.insert(std::make_pair(name, node));
        }

        node->set_parent(this);
    }

public:
    AST_IMPLEMENT_ABSTRACT(IdentifierMap);
protected:
    explicit IdentifierMap() {;}
    explicit IdentifierMap(IdentifierMap *other): Node(other) {
        iterator begin = this->begin();
        iterator end   = this->end();

        for (NULL; begin != end; ++begin) {
            insert(begin->second);
        }
    }
public:
    // static pointer_type make_ptr() {return pointer_type(new class__());}
};



class ClassIdentifier: public Identifier
{
public:
    AST_GETTER_DECL_OVERRIDE(bfc::string_ptr, c_jni_sign);

    virtual bool is_class_identifier() override {return true;}

public:
    AST_IMPLEMENT(ClassIdentifier);
protected:
    explicit ClassIdentifier(const bfc::string_ptr &name): Identifier(name) {init(NULL);}
    explicit ClassIdentifier(Node *other): Identifier(other) {init(other);}
private:
    void init(Node *other) {;}
public:
    static pointer_type make_ptr(const bfc::string_ptr &name) {return pointer_type(new ClassIdentifier(name));}
    static pointer_type make_ptr(Node *other) {return pointer_type(new ClassIdentifier(other));}
};



class PackageIdentifier: public Identifier
{
public:
    AST_IMPLEMENT(PackageIdentifier);
protected:
    explicit PackageIdentifier(const bfc::string_ptr &name): Identifier(name) {;}
    explicit PackageIdentifier(PackageIdentifier *other): Identifier(other) {;}
public:
    static pointer_type make_ptr(const bfc::string_ptr &name) {return pointer_type(new PackageIdentifier(name));}
};


#if 0
class QualifiedIdentifier: public NodeList<Identifier>
{
public:
    void push_back_identifier_name(const bfc::string_ptr &child_name) {
        Identifier::pointer_type id = Identifier::make_ptr(child_name);
        push_back(id.get());
    }

    // FIXME: deprecated
    // void assign(QualifiedIdentifier *another) {m_node_list = another->m_node_list;}

public:
    // FIXME: Deprecated
    bfc::string_ptr get_name() {
        if (empty())
            return bfc::string::make_ptr("<QualifiedIdentifier:name:N/A>");
        else
            return back()->get_name();
    }

    bfc::string_ptr get_long_name(char separator) {
        if (empty())
            return bfc::string::make_ptr("<QualifiedIdentifier:long_name:<N/A>");

        iterator begin = this->begin();
        iterator end   = this->end();

        std::ostringstream os;
        os << (*begin)->get_name();
        ++begin;

        for (NULL; begin != end; ++begin) {
            os << separator;
            os << (*begin)->get_name();
        }

        return bfc::string::make_ptr(os);
    }

    bfc::string_ptr get_c_sign_in_method() {
        std::ostringstream os;
        os << "L";
        os << get_long_name('/')->c_str();
        os << ";";

        return bfc::string::make_ptr(os);
    }

public:
    // @Override
    virtual void debug_print(int indent) override {
        printf("%s", get_long_name('.')->c_str());
    }

public:
    AST_IMPLEMENT(QualifiedIdentifier);
protected:
    explicit QualifiedIdentifier(const bfc::string_ptr &name) {;}
    explicit QualifiedIdentifier(QualifiedIdentifier *other): NodeList(other) {;}
public:
    static pointer_type make_ptr() {return pointer_type(new QualifiedIdentifier(bfc::string::make_ptr("")));}

    static pointer_type parse(const char *long_name, char separator) {
        pointer_type qualified_identifier  = QualifiedIdentifier::make_ptr();
        const char *lookup    = long_name;
        const char *next      = strchr(lookup, separator);

        while (next && next > lookup) {
            qualified_identifier->push_back_identifier_name(bfc::string::make_ptr(lookup, next - lookup));
            lookup = next + 1;
            next   = strchr(lookup, separator);
        }

        if (*lookup) {
            qualified_identifier->push_back_identifier_name(bfc::string::make_ptr(lookup));
        }

        return qualified_identifier;
    }

    pointer_type make_qualified_identifier(const bfc::string_ptr &child_name) {
        pointer_type qualified_identifier = QualifiedIdentifier::make_ptr();
        iterator begin = this->begin();
        iterator end   = this->end();
        for (NULL; begin != end; ++begin) {
            qualified_identifier->push_back(*begin);
        }

        qualified_identifier->push_back_identifier_name(child_name);
        return qualified_identifier;
    }
};
#endif

NAMESPACE_AST_END

#endif

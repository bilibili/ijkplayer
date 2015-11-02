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

#ifndef JJK_PARSER_AST__AST__NAMESPACE__HPP
#define JJK_PARSER_AST__AST__NAMESPACE__HPP

#include "ast__def.hpp"
#include "ast_identifier.hpp"

NAMESPACE_AST_BEGIN

class Namespace
{
public:
    typedef std::map<bfc::string_ptr, Identifier::pointer_type> IdentifierMap;
    typedef IdentifierMap::iterator iterator;

    AST_PROPERTY_DEFINE(Identifier::pointer_type, identifier);

    iterator begin() {return m_id_map.begin();}
    iterator end()   {return m_id_map.end();}

    void add_package_identifier(Identifier *package_identifier);
    void add_class_identifier(Identifier *class_identifier);
    template<class T>
    void add_class_identifiers(T begin, T end) {
        for (NULL; begin != end; ++begin) {
            add_class_identifier(*begin);
        }
    }

    Identifier *find_identifier(const bfc::string_ptr &name);

    void clear() {m_id_map.clear();}

private:
    IdentifierMap m_id_map;
};

NAMESPACE_AST_END

#endif

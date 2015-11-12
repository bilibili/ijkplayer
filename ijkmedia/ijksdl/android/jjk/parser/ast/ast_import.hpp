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

#ifndef JJK_PARSER_AST__AST_IMPORT__HPP
#define JJK_PARSER_AST__AST_IMPORT__HPP

#include "ast__def.hpp"
#include "ast_node.hpp"
#include "ast_identifier.hpp"

NAMESPACE_AST_BEGIN

class ImportList: public NodeList<Identifier>
{
public:
    // @Override
    virtual void debug_print(int indent) override {
        if (size() > 0) {
            printf("\n");

            iterator begin = this->begin();
            iterator end   = this->end();

            for (NULL; begin != end; ++begin) {
                printf("import ");
                (*begin)->debug_print(indent);
                printf(";\n");
            }
        }
    }

public:
    AST_IMPLEMENT(ImportList);
protected:
    explicit ImportList() {;}
public:
    static pointer_type make_ptr() {return pointer_type(new ImportList());}
};

NAMESPACE_AST_END

#endif

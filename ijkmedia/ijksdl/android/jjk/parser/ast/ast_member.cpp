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

#include "ast_member.hpp"
#include "ast_class.hpp"

using namespace ast;

bfc::string_ptr Member::get_c_jni_id()
{
    std::ostringstream os;
    os << get_this_class()->get_c_jni_class_instance();
    os << "." << get_c_jni_id_name();
    return bfc::string::make_ptr(os);
}

void MemberList::build_all_c_class_decl(std::ostream &os)
{
    MemberList::iterator begin = this->begin();
    MemberList::iterator end   = this->end();

    for (NULL; begin != end; ++begin) {
        (*begin)->reset_build_format();

        (*begin)->build_c_class_decl(os);

        (*begin)->reset_build_format();
    }
}

void MemberList::build_all_c_func_decl(std::ostream &os)
{
    if (this->size() > 0) {
        MemberList::iterator begin = this->begin();
        MemberList::iterator end   = this->end();

        for (NULL; begin != end; ++begin) {
            (*begin)->reset_build_format();

            (*begin)->build_c_func_decl(os);

            (*begin)->reset_build_format();
        }
    }
}

void MemberList::build_all_c_member_id_decl(std::ostream &os)
{
    if (this->size() > 0) {
        MemberList::iterator begin = this->begin();
        MemberList::iterator end   = this->end();

        os << std::endl;
        for (NULL; begin != end; ++begin) {
            (*begin)->reset_build_format();
            (*begin)->set_build_indent(get_this_class()->get_build_indent() + 4);

            (*begin)->build_c_member_id_decl(os);

            (*begin)->reset_build_format();
        }
    }
}

void MemberList::build_all_c_member_id_load(std::ostream &os)
{
    if (this->size() > 0) {
        MemberList::iterator begin = this->begin();
        MemberList::iterator end   = this->end(); 

        for (NULL; begin != end; ++begin) {
            (*begin)->reset_build_format();
            (*begin)->set_build_indent(get_this_class()->get_build_indent() + 4);

            (*begin)->build_c_member_id_load(os);

            (*begin)->reset_build_format();
        }
    }
}

void MemberList::build_all_c_func_impl(std::ostream &os)
{
    if (this->size() > 0) {
        MemberList::iterator begin = this->begin();
        MemberList::iterator end   = this->end();

        for (NULL; begin != end; ++begin) {
            (*begin)->reset_build_format();

            (*begin)->build_c_func_impl(os);

            (*begin)->reset_build_format();
        }
    }
}

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

#ifndef BFC__STRING__H
#define BFC__STRING__H

#include <string>
#include <sstream>
#include "bfc_object.h"

NAMESPACE_BFC_BEGIN

class string: public object
{
public:
    typedef rc_ptr<string> pointer_type;

public:
    const char *c_str() const {return m_str.c_str();}
    int compare (const char* s) const {return m_str.compare(s);}

    bool operator<(const bfc::string &rhs) const { return m_str < rhs.m_str; }

public:
    static pointer_type make_ptr()                              {return pointer_type(new string(""));}
    static pointer_type make_ptr(const char *str)               {return pointer_type(new string(str));}
    static pointer_type make_ptr(const std::string& str)        {return pointer_type(new string(str));}
    static pointer_type make_ptr(const std::ostringstream& os)  {return pointer_type(new string(os));}

    static pointer_type make_ptr(const char *str, size_t len)   {return pointer_type(new string(str, len));}

protected:
    string() {;}
    virtual ~string() {;}

    string(const char *str): m_str(str ? str : "") {;}
    string(const std::string &str): m_str(str) {;}
    string(const std::ostringstream &os): m_str(os.str()) {;}

    string(const char *str, size_t len): m_str(str ? str : NULL, len) {;}

private:
    const std::string m_str;
};

typedef string::pointer_type string_ptr;

inline bool operator<(const string_ptr &lhs, const string_ptr &rhs)
{
    if (lhs.is_null())
        return !rhs.is_null();

    if (rhs.is_null())
        return false;

    return *lhs.get() < *rhs.get();
}


NAMESPACE_BFC_END

NAMESPACE_STD_BEGIN

template<class char_t, class traits_t>
inline std::basic_ostream<char_t, traits_t>&
operator<<(std::basic_ostream<char_t, traits_t> &os, const bfc::string &str)
{
    return os << str.c_str();
}

template<class char_t, class traits_t>
inline std::basic_ostream<char_t, traits_t>&
operator<<(std::basic_ostream<char_t, traits_t> &os, const bfc::string *str)
{
    return os << str->c_str();
}

template<class char_t, class traits_t>
inline std::basic_ostream<char_t, traits_t>&
operator<<(std::basic_ostream<char_t, traits_t> &os, const bfc::string_ptr &str)
{
    return os << str->c_str();
}

NAMESPACE_STD_END

#endif

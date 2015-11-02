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

#include "ast_compilation_unit.hpp"

#include <fstream>
#include <iostream>
#include "ast_method.hpp"
#include "ast_class.hpp"
#include "ast__context.hpp"

using namespace ast;

static const char *JJK_LICENSE_HEADER =
"/*\n"
" * copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>\n"
" *\n"
" * This file is part of ijkPlayer.\n"
" *\n"
" * ijkPlayer is free software; you can redistribute it and/or\n"
" * modify it under the terms of the GNU Lesser General Public\n"
" * License as published by the Free Software Foundation; either\n"
" * version 2.1 of the License, or (at your option) any later version.\n"
" *\n"
" * ijkPlayer is distributed in the hope that it will be useful,\n"
" * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
" * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
" * Lesser General Public License for more details.\n"
" *\n"
" * You should have received a copy of the GNU Lesser General Public\n"
" * License along with ijkPlayer; if not, write to the Free Software\n"
" * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA\n"
" */\n";

void CompilationUnit::do_build(std::ostream &h_os, std::ostream &c_os)
{
    Class      *clazz       = get_clazz();

    printf("==========\n");
    // printf("%s\n", __func__);

    // .h
    h_os << JJK_LICENSE_HEADER << std::endl;
    h_os << "#ifndef " << get_header_macro() << std::endl;
    h_os << "#define " << get_header_macro() << std::endl;
    h_os << std::endl;
    h_os << "#include \"ijksdl/android/jjk/internal/jjk_internal.h\"" << std::endl;
    h_os << std::endl;

    clazz->build_c_func_decl(h_os);
    h_os << std::endl;
    if (clazz->is_include_util()) {
        h_os << "#define JJK_HAVE__" << clazz->get_c_class_name() << std::endl;
        h_os << std::endl;
        h_os << "#include \"" << clazz->get_name() << ".util.h\"" << std::endl;
        h_os << std::endl;
    }
    h_os << "#endif//" << get_header_macro() << std::endl;

    // .c
    c_os << JJK_LICENSE_HEADER << std::endl;
    c_os << "#include \"" << clazz->get_name() << ".h\"" << std::endl;

    clazz->build_c_class_decl(c_os);
    clazz->build_c_func_impl(c_os);
}

void CompilationUnit::build()
{
    std::ofstream h_stream;
    std::ofstream c_stream;
    std::ostream *h_stream_ptr = &std::cout;
    std::ostream *c_stream_ptr = &std::cout;

    if (!Context::instance()->get_h_file_path().empty()) {
        h_stream.open(Context::instance()->get_h_file_path().c_str());
        if (!h_stream.is_open()) {
            printf("failed to open output .h file %s\n", Context::instance()->get_h_file_path().c_str());
            return;
        }
        h_stream_ptr = &h_stream;
    }

    if (!Context::instance()->get_c_file_path().empty()) {
        c_stream.open(Context::instance()->get_c_file_path().c_str());
        if (!c_stream) {
            printf("failed to open output .c file %s\n", Context::instance()->get_c_file_path().c_str());
            return;
        }
        c_stream_ptr = &c_stream;
    }

    do_build(*h_stream_ptr, *c_stream_ptr);
}

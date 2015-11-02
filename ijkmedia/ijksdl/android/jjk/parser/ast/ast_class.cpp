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

#include "ast_class.hpp"
#include "ast_identifier.hpp"
#include "ast_compilation_unit.hpp"

using namespace ast;

bfc::string_ptr Class::get_c_jni_sign()
{
    std::ostringstream os;
    if (get_parent()->get_this_class()) {
        // inner class
        os << get_parent()->get_this_class()->get_c_jni_sign();
        os << "$";
    } else {
        os << get_this_package()->get_c_jni_sign();
        os << "/";
    }
    os << get_name();
    return bfc::string::make_ptr(os);
}

//@Override
bfc::string_ptr Class::get_c_jni_id_name()
{
    return bfc::string::make_ptr("id");
}

//@Override
void Class::build_c_func_decl(std::ostream &os)
{
    get_member_list()->build_all_c_func_decl(os);
    if (get_parent()->get_this_class() == NULL) {
        // do not export inner class loader
        os << "int JJK_loadClass__" << get_c_class_name() << "(JNIEnv *env);" << std::endl;
    }
}

//@Override
void Class::build_c_class_decl(std::ostream &os)
{
    get_member_list()->build_all_c_class_decl(os);

    os << std::endl;
    os << build_indent() << "typedef struct " << get_c_class_name() << " {" << std::endl;
    os << build_indent() << "    jclass id;" << std::endl;
    get_member_list()->build_all_c_member_id_decl(os);
    os << build_indent() << "} " << get_c_class_name() << ";" << std::endl;
    os << build_indent() << "static " << get_c_class_name() << " " << get_c_jni_class_instance() << ";" << std::endl;
}

//@Override
void Class::build_c_member_id_load(std::ostream &os)
{
    os << std::endl;
    os << build_indent() << "ret = JJK_loadClass__" << get_c_class_name() << "(env);" << std::endl;
    os << build_indent() << "if (ret)" << std::endl;
    os << build_indent() << "    goto fail;" << std::endl;
}

//@Override
void Class::build_c_func_impl(std::ostream &os)
{
    bool need_label_ignore = false;
    get_member_list()->build_all_c_func_impl(os);

    os << std::endl;
    os << "int JJK_loadClass__" << get_c_class_name() << "(JNIEnv *env)" << std::endl;
    os << "{" << std::endl;
    os << "    int         ret                   = -1;" << std::endl;
    os << "    const char *JJK_UNUSED(name)      = NULL;" << std::endl;
    os << "    const char *JJK_UNUSED(sign)      = NULL;" << std::endl;
    os << "    jclass      JJK_UNUSED(class_id)  = NULL;" << std::endl;
    os << "    int         JJK_UNUSED(api_level) = 0;" << std::endl;

    Annotation *annotation = get_annotation_at("MinApi");
    if (annotation) {
        need_label_ignore = true;
        os << std::endl;
        os << "    api_level = JJK_GetSystemAndroidApiLevel(env);\n" << std::endl;
        os << "    if (api_level < " << annotation->get_value() << ") {" << std::endl;
        os << "        ALOGW(" << quote_string("JJKLoader: Ignore: '%s' need API %d\\n") << ", " << quote_string(get_java_long_name()->c_str()) << ", api_level);" << std::endl;
        os << "        goto ignore;" << std::endl;
        os << "    }" << std::endl;
    }

    os << std::endl;
    os << "    sign = \"" << get_c_jni_sign() << "\";" << std::endl;
    os << "    " << get_c_jni_id() << " = JJK_FindClass__asGlobalRef__catchAll(env, sign);" << std::endl;
    os << "    if (" << get_c_jni_id() << " == NULL)" << std::endl;
    os << "        goto fail;" << std::endl;
    get_member_list()->build_all_c_member_id_load(os);
    os << std::endl;
    os << "    ALOGD(" << quote_string("JJKLoader: OK: '%s' loaded\\n") << ", " << quote_string(get_java_long_name()->c_str()) << ");" << std::endl;
    if (need_label_ignore)
        os << "ignore:" << std::endl;
    os << "    ret = 0;" << std::endl;
    os << "fail:" << std::endl;
    os << "    return ret;" << std::endl;
    os << "}" << std::endl;
}

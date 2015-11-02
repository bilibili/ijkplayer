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

#ifndef JJK_PARSER_AST__AST__FORWARD__HPP
#define JJK_PARSER_AST__AST__FORWARD__HPP

#include <cassert>
#include "bfc/bfc.h"

#define NAMESPACE_AST_BEGIN namespace ast {
#define NAMESPACE_AST_END   }

NAMESPACE_AST_BEGIN

// https://docs.oracle.com/javase/specs/jls/se7/html/jls-18.html
class Annotation;
class Annotations;
class Argument;
class ArgumentList;
class BasicType;
class Class;
class ClassIdentifier;
class CompilationUnit;
class Constructor;
class Field;
class Identifier;
class ImportList;
class MemberList;
class Method;
class MethodList;
class ModifierSet;
class Node;
class PackageIdentifier;
class ReferenceType;
class Type;

NAMESPACE_AST_END

#endif

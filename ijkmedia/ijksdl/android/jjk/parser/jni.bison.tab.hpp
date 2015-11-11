/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_JNI_BISON_TAB_HPP_INCLUDED
# define YY_YY_JNI_BISON_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 9 "jni.bison.y" /* yacc.c:1915  */

#include "ast/ast__forward.hpp"

#line 48 "jni.bison.tab.hpp" /* yacc.c:1915  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_EOF = 0,
    T_PACKAGE = 258,
    T_IMPORT = 259,
    T_BOOLEAN = 260,
    T_BYTE = 261,
    T_FLOAT = 262,
    T_INT = 263,
    T_LONG = 264,
    T_VOID = 265,
    T_PRIVATE = 266,
    T_PROTECTED = 267,
    T_PUBLIC = 268,
    T_ABSTRACT = 269,
    T_FINAL = 270,
    T_STATIC = 271,
    T_CLASS = 272,
    T_INTERFACE = 273,
    T_INTEGER_LITERAL = 274,
    T_ID = 275
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 38 "jni.bison.y" /* yacc.c:1915  */

    int                     num_int;

    bfc::string             *string;
    class int_set           *int_set;

    ast::Annotation           *annotation;
    ast::Annotations          *annotations;
    ast::Argument             *argument;
    ast::ArgumentList         *argument_list;
    ast::Class                *clazz;
    ast::CompilationUnit      *compilation_unit;
    ast::Identifier           *identifier;
    ast::ImportList           *import_list;
    ast::Field                *field;
    ast::MemberList           *member_list;
    ast::Method               *method;
    ast::ModifierSet          *modifier_set;
    ast::Identifier           *qualified_identifier;
    ast::Type                 *type;

#line 104 "jni.bison.tab.hpp" /* yacc.c:1915  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_JNI_BISON_TAB_HPP_INCLUDED  */

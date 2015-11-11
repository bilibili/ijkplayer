%require "2.4"

%{
#include <set>
#include "parser.hpp"
#include "ast/ast.hpp"
%}

%code requires {
#include "ast/ast__forward.hpp"
}

%token              T_PACKAGE
%token              T_IMPORT

%token <num_int>    T_BOOLEAN
%token <num_int>    T_BYTE
%token <num_int>    T_FLOAT
%token <num_int>    T_INT
%token <num_int>    T_LONG
%token <num_int>    T_VOID

%token <num_int>    T_PRIVATE
%token <num_int>    T_PROTECTED
%token <num_int>    T_PUBLIC

%token <num_int>    T_ABSTRACT
%token <num_int>    T_FINAL
%token <num_int>    T_STATIC

%token <num_int>    T_CLASS
%token <num_int>    T_INTERFACE

%token <string>     T_INTEGER_LITERAL
%token <string>     T_ID
%token              T_EOF       0   "end of file"

%union {
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
}

%type <annotation>            annotation
%type <annotations>           annotations
%type <argument>              argument
%type <argument_list>         argument_list
%type <type>                  basic_type
%type <clazz>                 class
%type <clazz>                 class_head
%type <identifier>            class_identifier
%type <compilation_unit>      compilation_unit
%type <compilation_unit>      compilation_unit_head
%type <identifier>            identifier
%type <qualified_identifier>  import
%type <import_list>           import_list
%type <field>                 field
%type <member_list>           member_list
%type <method>                method
%type <num_int>               modifier
%type <modifier_set>          modifier_set
%type <qualified_identifier>  package
%type <qualified_identifier>  qualified_identifier
%type <type>                  reference_type
%type <type>                  type
%type <num_int>               t_class_or_interface


%destructor { BFC_RELEASE($$); }      T_ID
%destructor { BFC_RELEASE($$); }      T_INTEGER_LITERAL


%destructor { BFC_RELEASE($$); }      annotation
%destructor { BFC_RELEASE($$); }      annotations
%destructor { BFC_RELEASE($$); }      argument
%destructor { BFC_RELEASE($$); }      argument_list
%destructor { BFC_RELEASE($$); }      basic_type
%destructor { BFC_RELEASE($$); }      class
%destructor { BFC_RELEASE($$); }      class_head
%destructor { BFC_RELEASE($$); }      class_identifier
%destructor { BFC_RELEASE($$); }      compilation_unit
%destructor { BFC_RELEASE($$); }      identifier
%destructor { BFC_RELEASE($$); }      import
%destructor { BFC_RELEASE($$); }      import_list
%destructor { BFC_RELEASE($$); }      field
%destructor { BFC_RELEASE($$); }      member_list
%destructor { BFC_RELEASE($$); }      method
%destructor { $$ = 0;          }      modifier
%destructor { BFC_RELEASE($$); }      modifier_set
%destructor { BFC_RELEASE($$); }      package
%destructor { BFC_RELEASE($$); }      qualified_identifier
%destructor { BFC_RELEASE($$); }      reference_type
%destructor { BFC_RELEASE($$); }      type
%destructor { $$ = 0;          }      t_class_or_interface

%start compilation_unit

%%

annotation:
      '@' T_ID
      {
          BISON_LOGF("annotation: %s\n", $2->c_str());
          $$ = ast::Annotation::make_ptr($2).detach();
      }
    | '@' T_ID '(' T_INTEGER_LITERAL ')'
      {
          BISON_LOGF("annotation: %s(%s)\n", $2->c_str(), $4->c_str());
          $$ = ast::Annotation::make_ptr($2, $4).detach();
      }
;

annotations:
      {
          BISON_LOGF("annotations: NULL\n");
          $$ = ast::Annotations::make_ptr().detach();
      }
    | annotations
      annotation
      {
          BISON_LOGF("annotations: ++\n");
          $$ = $1;
          $$->insert($2);
      }
;

argument:
      type T_ID
      {
          BISON_LOGF("argument: %s %s\n", $1->get_name()->c_str(), $2->c_str());
          $$ = ast::Argument::make_ptr($2).detach();
          $$->set_type($1);
      }
;

argument_list:
      {
          BISON_LOGF("argument_list: NULL\n");
          $$ = ast::ArgumentList::make_ptr().detach();
      }
    | argument
      {
          BISON_LOGF("argument_list: << %s %s\n", $1->get_type()->get_name()->c_str(), $1->get_name()->c_str());
          $$ = ast::ArgumentList::make_ptr().detach();
          $$->push_back($1);
      }
    | argument_list ','
      argument
      {
          BISON_LOGF("argument_list: << , %s %s\n", $3->get_type()->get_name()->c_str(), $3->get_name()->c_str());
          $$ = $1;
          $$->push_back($3);
      }
;

t_class_or_interface:
      T_CLASS     {$$ = $1;}
    | T_INTERFACE {$$ = $1;}

class_identifier:
      reference_type {$$ = ast::ClassIdentifier::make_ptr($1).detach();}

class_head:
      annotations
      modifier_set
      t_class_or_interface
      class_identifier
      '{' {
          BISON_LOGF("class\n");

          $4->set_prefix(ast::Context::instance()->get_local_namespace()->get_identifier());
          ast::Context::instance()->get_local_namespace()->add_class_identifier($4);

          $$ = ast::Class::make_ptr($4).detach();
          if ($3 == T_INTERFACE)
              $$->set_is_interface(true);

          $$->set_annotations($1);
          $$->set_modifier_set($2);
          $$->get_local_namespace()->set_identifier($4);
          ast::Context::instance()->push_local_namespace($$->get_local_namespace());
      }

class:
      class_head
      member_list
      '}' {
          BISON_LOGF("class_body\n");
          $$ = $1;
          $$->set_member_list($2);

          ast::Context::instance()->pop_local_namespace();
      }
;

compilation_unit_head:
      package
      import_list {
          BISON_LOGF("compilation_unit_head:\n");
          $$ = ast::CompilationUnit::make_ptr().detach();
          $$->set_package($1);
          $$->set_import_list($2);

          $$->get_local_namespace()->set_identifier($1);
          $$->get_local_namespace()->add_class_identifiers($2->begin(), $2->end());
          $$->get_local_namespace()->add_package_identifier($1);

          ast::Context::instance()->push_local_namespace($$->get_local_namespace());
      }


compilation_unit:
      compilation_unit_head
      class
      T_EOF {
          BISON_LOGF("compilation_unit_body:\n");
          $$ = $1;
          $$->set_clazz($2);

          ast::Context::instance()->pop_local_namespace();

          printf("----------\n");
          $$->debug_print(0);
          printf("----------\n");
          $$->build();
      }
;

field:
      annotations
      modifier_set type T_ID ';'
      {
          BISON_LOGF("method:\n");
          $$ = ast::Field::make_ptr($4).detach();
          $$->set_annotations($1);
          $$->set_modifier_set($2);
          $$->set_type($3);
      }
;

method:
      annotations
      modifier_set type T_ID '(' argument_list ')' ';'
      {
          BISON_LOGF("method:\n");
          $$ = ast::Method::make_ptr($4).detach();
          $$->set_annotations($1);
          $$->set_modifier_set($2);
          $$->set_type($3);
          $$->set_argument_list($6);
      }
    | annotations
      modifier_set T_ID '(' argument_list ')' ';'
      {
          BISON_LOGF("method:\n");
          $$ = ast::Constructor::make_ptr($3).detach();
          $$->set_annotations($1);
          $$->set_modifier_set($2);
          $$->set_type(ast::ReferenceType::make_ptr($$->get_name()).get());
          $$->set_argument_list($5);
      }
;

member_list:
      {
          BISON_LOGF("member_list: NULL\n");
          $$ = ast::MemberList::make_ptr().detach();
      }
    | member_list
      class
      {
          BISON_LOGF("member_list: + class: %s\n", $2->get_name()->c_str());
          $$ = $1;
          $$->push_back($2);
      }
    | member_list
      field
      {
          BISON_LOGF("member_list: + field: %s\n", $2->get_name()->c_str());
          $$ = $1;
          $$->push_back($2);
      }
    | member_list
      method
      {
          BISON_LOGF("member_list: + method: %s\n", $2->get_name()->c_str());
          $$ = $1;
          $$->push_back($2);
      }
;

modifier:
      T_ABSTRACT          {BISON_LOGF("modifier: T_ABSTRACT\n");  $$ = T_ABSTRACT;}
    | T_FINAL             {BISON_LOGF("modifier: T_FINAL\n");     $$ = T_FINAL;}
    | T_PRIVATE           {BISON_LOGF("modifier: T_PRIVATE\n");   $$ = T_PRIVATE;}
    | T_PROTECTED         {BISON_LOGF("modifier: T_PROTECTED\n"); $$ = T_PROTECTED;}
    | T_PUBLIC            {BISON_LOGF("modifier: T_PUBLIC\n");    $$ = T_PUBLIC;}
    | T_STATIC            {BISON_LOGF("modifier: T_STATIC\n");    $$ = T_STATIC;}
;

modifier_set:
      {
          BISON_LOGF("modifier_set: NULL\n");
          $$ = ast::ModifierSet::make_ptr().detach();
      }
    | modifier_set
      modifier
      {
          BISON_LOGF("modifier_set: ++\n");
          $$ = $1;
          $$->insert_token($2);
      }
;

package:
      T_PACKAGE qualified_identifier ';' {$$ = $2;}
;

identifier:
      T_ID {$$ = ast::Identifier::make_ptr($1).detach();}
;

qualified_identifier:
      identifier {
          $$ = $1;
      }
    | qualified_identifier '.' identifier {
          $$ = $3;
          $$->set_prefix($1);
      }
;

import:
      T_IMPORT qualified_identifier ';' {$$ = $2;}
;

import_list:
      {
          BISON_LOGF("import_list: NULL\n");
          $$ = ast::ImportList::make_ptr().detach();
      }
    | import_list
      import
      {
          BISON_LOGF("import_list: ++\n");
          $$ = $1;
          $$->push_back($2);
      }
;

basic_type:
      basic_type '[' ']' {
        $$ = $1;
        $$->set_is_array(true);
        BISON_LOGF("basic_type: %s[]\n", $$->get_name()->c_str());
      }
    | T_BOOLEAN {
        $$ = ast::BooleanType::make_ptr().detach();
        BISON_LOGF("basic_type: %s\n", $$->get_name()->c_str()); }
    | T_BYTE {
        $$ = ast::ByteType::make_ptr().detach();
        BISON_LOGF("basic_type: %s\n", $$->get_name()->c_str()); }
    | T_FLOAT {
        $$ = ast::FloatType::make_ptr().detach();
        BISON_LOGF("basic_type: %s\n", $$->get_name()->c_str()); }
    | T_INT {
        $$ = ast::IntType::make_ptr().detach();
        BISON_LOGF("basic_type: %s\n", $$->get_name()->c_str()); }
    | T_LONG {
        $$ = ast::LongType::make_ptr().detach();   
        BISON_LOGF("basic_type: %s\n", $$->get_name()->c_str()); }
    | T_VOID {
        $$ = ast::VoidType::make_ptr().detach();   
        BISON_LOGF("basic_type: %s\n", $$->get_name()->c_str()); }
;

reference_type:
      reference_type '[' ']' {
        $$ = $1;
        $$->set_is_array(true);
        BISON_LOGF("reference_type: %s[]\n", $$->get_name()->c_str());
      }
    | qualified_identifier {
        $$ = ast::ReferenceType::make_ptr($1).detach();
        BISON_LOGF("reference_type: %s\n", $$->get_name()->c_str());
    }
;

type:
    basic_type     { $$ = $1; }
  | reference_type { $$ = $1; }

%%

int bison_main(int argc, char **argv)
{
    if ( argc <= 1 ) {
        printf(
            " usage:\n"
            "   jjk <input_file>\n");
        return -1;
    }

    yyin = fopen( argv[1], "r" );
    return yyparse();
}

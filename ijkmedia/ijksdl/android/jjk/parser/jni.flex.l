%{
#include <inttypes.h>
#include "jni.bison.tab.hpp"
#include "parser.hpp"
#include "bfc/bfc.h"
%}
%option yylineno
%option noyywrap

ID              ([a-zA-Z_][a-zA-Z0-9_]*)
INTEGER         ([0-9]*)
WHITESPACE      ([ \t\v\r\n])

%%

","                         {return yytext[0];}
"."                         {return yytext[0];}
";"                         {return yytext[0];}
"{"                         {return yytext[0];}
"}"                         {return yytext[0];}
"["                         {return yytext[0];}
"]"                         {return yytext[0];}
"("                         {return yytext[0];}
")"                         {return yytext[0];}
"@"                         {return yytext[0];}

"package"                   {return T_PACKAGE;}
"import"                    {return T_IMPORT;}

"boolean"                   {return T_BOOLEAN;}
"byte"                      {return T_BYTE;}
"float"                     {return T_FLOAT;}
"int"                       {return T_INT;}
"long"                      {return T_LONG;}
"void"                      {return T_VOID;}

"private"                   {return T_PRIVATE;}
"protected"                 {return T_PROTECTED;}
"public"                    {return T_PUBLIC;}

"abstract"                  {return T_ABSTRACT;}
"final"                     {return T_FINAL;}
"static"                    {return T_STATIC;}

"class"                     {return T_CLASS;}
"interface"                 {return T_INTERFACE;}

\n                          {yylineno++;}
{WHITESPACE}                {;}
{INTEGER}                   {FLEX_LOGF("<integer>:%s\n", yytext); yylval.string = bfc::string::make_ptr(yytext).detach(); return T_INTEGER_LITERAL;}
{ID}                        {FLEX_LOGF("<id>:%s\n", yytext); yylval.string = bfc::string::make_ptr(yytext).detach(); return T_ID;}
<<EOF>>                     {yyterminate(); return T_EOF;}
.                           {FLEX_LOGF("Unknown token at line %d: '%s'\n", yylineno, yytext); yyterminate();}

%%

int flex_main(int argc, char **argv)
{
    int token;

    if ( argc <= 1 ) {
        printf(
            " usage:\n"
            "   jjk <input_file>\n");
        return -1;
    }

    printf("yylex %s\n", argv[1]);
    yyin = fopen( argv[1], "r" );

    while ((token = yylex()) > 0) {
        printf("token: %d\n", token);
    }
    return 0;
}
#ifndef JJK_PARSER_HPP
#define JJK_PARSER_HPP

#include <cstdio>
#include <list>
#include <set>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include "bfc/bfc.h"

#define FLEX_LOGF(...)
// #define BISON_LOGF(...)
#define BISON_LOGF printf

extern int yylex();
extern FILE *yyin;
extern FILE *yyout;
extern int yylineno;
static void yyerror(const char *s) { printf("ERROR: at line %d\n\t%s\n", yylineno, s);}

NAMESPACE_STD_BEGIN

template<class char_t, class traits_t>
inline std::basic_ostream<char_t, traits_t>&
operator<<(
    std::basic_ostream<char_t, traits_t>            &os,
    const std::auto_ptr<std::basic_string<char_t> > &str)
{
    return os << str->c_str();
}

class ast_fill
{
public:
    ast_fill(int width): m_width(width) {;}
    int m_width;
};

template<class char_t, class traits_t>
inline std::basic_ostream<char_t, traits_t>&
operator<<(
    std::basic_ostream<char_t, traits_t>    &os,
    const ast_fill                          &rhs)
{
    for (int i = 0; i < rhs.m_width; ++i)
        os << ' ';
    return os;
}

NAMESPACE_STD_END

#endif

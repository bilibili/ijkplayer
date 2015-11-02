#include "parser.hpp"
#include "jni.bison.tab.hpp"
#include "ast/ast.hpp"
#include "bfc/bfc.h"

extern int flex_main(int argc, char **argv);
extern int bison_main(int argc, char **argv);

extern int yyparse();

int main(int argc, char **argv)
{
    printf("%s\n", argv[0]);

    if ( argc < 2 ) {
        printf(
            " usage:\n"
            "   jjk <input_file> <output_c_file> <output_h_file>\n");
        return -1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        printf("failed to open input file %s\n", argv[1]);
        return -1;
    }

    ast::Context::instance()->set_h_file_path(argc >= 3 ? argv[2] : "");
    ast::Context::instance()->set_c_file_path(argc >= 4 ? argv[3] : "");

    const char *file_name = strrchr(argv[1], '/');
    std::string dir_name = std::string(argv[1], file_name - argv[1]);
    // printf("add java path: %s\n", dir_name->c_str());
    ast::Context::instance()->set_java_class_dir(dir_name.c_str());

    int ret = yyparse();
    fclose(yyin);
    return ret;
}

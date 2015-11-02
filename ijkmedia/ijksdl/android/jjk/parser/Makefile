CC = g++
CPPFLAGS = -std=c++11 -I./ -Wno-deprecated-register
YACC = /opt/bin/bison
YACC_FLAGS = --debug --verbose -d
YYLEX = flex
YYLEX_FLAGS = 

all: jjk

YACC_DEPS = \
	jni.bison.y \
	jni.flex.l \

CXX_OBJS = \
	ast/ast_class.o \
	ast/ast_compilation_unit.o \
	ast/ast_constructor.o \
	ast/ast_field.o \
	ast/ast_identifier.o \
	ast/ast_member.o \
	ast/ast_method.o \
	ast/ast_node.o \
	ast/ast_property_accessor.o \
	ast/ast_reference_type.o \
	ast/ast__context.o \
	ast/ast__namespace.o \
	jni.flex.yy.o \
	jni.bison.tab.o \
	jjk.o \

DEPS := $(CXX_OBJS:.o=.d)
-include $(DEPS)

$(CXX_OBJS): Makefile jni.bison.tab.cpp jni.flex.yy.cpp

%.o: %.cpp
	$(CC) $(CPPFLAGS) -MM -MT $@ -MF $(patsubst %.o,%.d,$@) $<
	$(CC) $(CPPFLAGS) -c -o $@ $<

jjk: $(CXX_OBJS)
	$(CC) -o jjk $(CXX_OBJS)

jni.bison.tab.cpp: $(YACC_DEPS)
	$(YACC) $(YACC_FLAGS) -o $@ jni.bison.y

jni.flex.yy.cpp: $(YACC_DEPS)
	$(YYLEX) $(YYLEX_FLAGS) -o $@ jni.flex.l

.PHONY: all clean
clean:
	rm -f jni.bison.tab.cpp
	rm -f jni.bison.tab.hpp
	rm -f jni.flex.yy.cpp
	rm -f $(CXX_OBJS)
	rm -f $(DEPS)
	rm -f jjk

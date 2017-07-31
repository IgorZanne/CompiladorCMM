# Specify compiler
CC=gcc
 
# Specify linker
LINK=gcc
 
.PHONY : all
all : lexer.c parser.c app 
 
# Link the object files into a binary
app : 
    $(LINK) -o app -w main.c cmm.tab.c lex.yy.c errormsg.c util.c escape.c semantic.c absyn.c prabsyn.c env.c printtree.c symbol.c table.c temp.c tree.c types.c translate.c frame-llvm.c
 
# Compile the source files into object files
#main.o : main.c
#    $(CC) -oteste main.c cmm.tab.c lex.yy.c errormsg.c util.c 

parser.c: cmm.y
    bison -d cmm.y
    
lexer.c: cmm.flex
    flex cmm.flex
 
# Clean target
.PHONY : clean
clean :
   del -f *.tab.*
   del -f *.yy.c
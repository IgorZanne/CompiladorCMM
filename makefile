# Specify compiler
CC=gcc
 
# Specify linker
LINK=gcc
 
.PHONY : all
all : app
 
# Link the object files into a binary
app : main.o
    $(LINK) -o app main.c cmm.tab.c lex.yy.c errormsg.c util.c escape.c semantic.c absyn.c env.c printtree.c symbol.c table.c temp.c tree.c types.c translate.c frame-llvm.c
 
# Compile the source files into object files
#main.o : main.c
#    $(CC) -oteste main.c cmm.tab.c lex.yy.c errormsg.c util.c 
 
# Clean target
.PHONY : clean
clean :
    rm main.o app 
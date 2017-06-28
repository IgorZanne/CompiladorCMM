#ifndef INCLUDE__SEMANTIC_H
#define INCLUDE__SEMANTIC_H

#include "absyn.h"
#include "types.h"
#include "translate.h"

typedef struct {
    Tr_exp exp; // BITCODE - TODO
    Ty_ty type; // TIPO - OK
    char code[65536];
} Ty_exp;

char* semantic_proc(A_exp program);

#endif

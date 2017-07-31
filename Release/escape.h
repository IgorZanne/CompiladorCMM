#ifndef INCLUDE__ESCAPE_H
#define INCLUDE__ESCAPE_H

#include "absyn.h"

typedef struct ESC_entry_s *ESC_entry;
struct ESC_entry_s
{
    int depth;
    bool *escape;
};
static int _depth;
static TAB_table _env;


void esc_find_escape(A_exp exp);

#endif

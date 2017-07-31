#ifndef INCLUDE__PARSER_WRAP_H
#define INCLUDE__PARSER_WRAP_H

#include "absyn.h"
#include "symbol.h"
#include "util.h"

extern int yydebug;

A_exp parse(string filename);

#endif
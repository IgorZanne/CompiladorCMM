/*
 * printtree.c - functions to print out intermediate representation (IR) trees.
 *
 */
#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "printtree.h"

/* local function prototype */
static void pr_tree_exp(FILE *out, T_exp exp);

static char bin_oper[][12] = {
   "add", "sub", "mul", "sdiv", 
   "and", "or", "shl", "lshr", "ashr", "xor"};

static char rel_oper[][12] = {
  "EQ", "NE", "LT", "GT", "LE", "GE", "ULT", "ULE", "UGT", "UGE"};

static void pr_stm(FILE *out, T_stm stm)
{
  switch (stm->kind) {
  case T_SEQ:
    fprintf(out, "SEQ(\n"); 
    pr_stm(out, stm->u.SEQ.left);  
    fprintf(out, ",\n"); 
    pr_stm(out, stm->u.SEQ.right); 
    fprintf(out, ")");
    break;
  case T_LABEL:
    fprintf(out, "LABEL %s", S_name(stm->u.LABEL));
    break;
  case T_JUMP:
    fprintf(out, "JUMP(\n"); 
    pr_tree_exp(out, stm->u.JUMP.exp); 
    fprintf(out, ")");
    break;
  case T_CJUMP:
    fprintf(out, "CJUMP(%s,\n", rel_oper[stm->u.CJUMP.op]);
    pr_tree_exp(out, stm->u.CJUMP.left); 
    fprintf(out, ",\n"); 
    pr_tree_exp(out, stm->u.CJUMP.right); 
    fprintf(out, ",\n");
    fprintf(out, "%s,", S_name(stm->u.CJUMP.true));
    fprintf(out, "%s", S_name(stm->u.CJUMP.false)); 
    fprintf(out, ")");
    break;
  case T_MOVE:
    fprintf(out, "MOVE(\n"); 
    pr_tree_exp(out, stm->u.MOVE.dst); 
    fprintf(out, ",\n");
    pr_tree_exp(out, stm->u.MOVE.src); 
    fprintf(out, ")");
    break;
  case T_EXP:
    fprintf(out, "EXP(\n"); 
    pr_tree_exp(out, stm->u.EXP); 
    fprintf(out, ")");
    break;
  }
}

static void pr_tree_exp(FILE *out, T_exp exp)
{
  switch (exp->kind) {
  case T_BINOP:
    fprintf(out, "%s i32 %s, %s\n", bin_oper[exp->u.BINOP.op], exp->u.BINOP.left, exp->u.BINOP.right); 
    break;
  case T_MEM:
    fprintf(out, "MEM");
    fprintf(out, "(\n"); pr_tree_exp(out, exp->u.MEM); fprintf(out, ")");
    break;
  case T_TEMP:
    fprintf(out, "TEMP t%s", 
			   Temp_look(Temp_name(), exp->u.TEMP));
    break;
  case T_ESEQ:
    fprintf(out, "ESEQ(\n"); pr_stm(out, exp->u.ESEQ.stm); 
    fprintf(out, ",\n");
    pr_tree_exp(out, exp->u.ESEQ.exp); fprintf(out, ")");
    break;
  case T_NAME:
    fprintf(out, "NAME %s", S_name(exp->u.NAME));
    break;
  case T_CONST:
    fprintf(out, "CONST %d", exp->u.CONST);
    break;
  case T_CALL:
    {T_expList args = exp->u.CALL.args;
     fprintf(out, "CALL(\n"); pr_tree_exp(out, exp->u.CALL.fun);
     for (;args; args=args->tail) {
       fprintf(out, ",\n"); 
       pr_tree_exp(out, args->head);
     }
     fprintf(out, ")");
     break;
   }
  } /* end of switch */
}

void printStmList (FILE *out, T_stmList stmList) 
{
  for (; stmList; stmList=stmList->tail) {
    pr_stm(out, stmList->head); 
    fprintf(out, "\n");
  }
}

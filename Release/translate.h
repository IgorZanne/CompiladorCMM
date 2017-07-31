#ifndef INCLUDE__TRANSLATE_H
#define INCLUDE__TRANSLATE_H

#include "frame.h"
#include "tree.h"
#include "temp.h"
#include "util.h"

typedef struct Tr_access_ *Tr_access;
typedef struct Tr_level_ *Tr_level;
typedef struct Tr_exp_ *Tr_exp;

typedef struct Tr_accessList_ *Tr_accessList;
struct Tr_accessList_ {Tr_access head; Tr_accessList tail;};

typedef struct Tr_expList_ *Tr_expList;
struct Tr_expList_ {Tr_exp head; Tr_expList tail;};

typedef struct cx_ cx_t;


struct Tr_access_
{
    Tr_level level;
    F_access access;
};

struct Tr_level_
{
    Tr_level parent;
    F_frame frame;
    Tr_accessList formals;
    Tr_accessList locals;
};

struct cx_
{
    Temp_labelList trues;
    Temp_labelList falses;
    T_stm stm;
};

struct Tr_exp_
{
    enum { Tr_ex, Tr_nx, Tr_cx } kind;
    union
    {
        T_exp ex;
        T_stm nx;
        cx_t cx;
    } u;
};

Tr_accessList Tr_AccessList(Tr_access head, Tr_accessList tail);
Tr_expList Tr_ExpList(Tr_exp head, Tr_expList tail);


Tr_level Tr_Outermost(void);
Tr_level Tr_Level(Tr_level parent, Temp_label name, U_boolList formals);
Tr_accessList Tr_Formals(Tr_level level);
Tr_access Tr_AllocLocal(Tr_level level, bool escape);


Tr_exp Tr_NumExp(int num);
Tr_exp Tr_StringExp(string str);
Tr_exp Tr_CallExp(Tr_level level, Temp_label label, Tr_expList args);
Tr_exp Tr_OpExp(int op, Tr_exp left, Tr_exp right);
Tr_exp Tr_RelExp(int op, Tr_exp left, Tr_exp right);
Tr_exp Tr_StringRelExp(int op, Tr_exp left, Tr_exp right);
Tr_exp Tr_RecordExp(Tr_expList fields, int size);
Tr_exp Tr_ArrayExp(Tr_exp size, Tr_exp init);
Tr_exp Tr_SeqExp(Tr_expList stms);
Tr_exp Tr_IfExp(Tr_exp test, Tr_exp then, Tr_exp elsee);
Tr_exp Tr_WhileExp(Tr_exp test, Tr_exp body);
Tr_exp Tr_ForExp(Tr_access access, Tr_exp low, Tr_exp high, Tr_exp body);
Tr_exp Tr_AssignExp(Tr_exp lhs, Tr_exp rhs);

Tr_exp Tr_SimpleVar(Tr_access access, Tr_level level);

#endif

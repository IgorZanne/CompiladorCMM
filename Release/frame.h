#ifndef INCLUDE__FRAME_H
#define INCLUDE__FRAME_H

#include "tree.h"
#include "temp.h"
#include "util.h"

extern const int F_WORDSIZE;

typedef struct F_frame_ *F_frame;
typedef struct F_access_ *F_access;

typedef struct F_accessList_ *F_accessList;
struct F_accessList_ {F_access head; F_accessList tail;};

F_accessList F_AccessList(F_access head, F_accessList tail); 

F_frame F_NewFrame(Temp_label name, U_boolList formals);
Temp_label F_Name(F_frame fr);

F_accessList F_Formals(F_frame fr);
F_access F_AllocLocal(F_frame fr, bool escape);
int F_Offset(F_access access);

typedef struct F_frag_ *F_frag;
struct F_frag_
{
    enum { F_stringFrag, F_procFrag } kind;
    union
    {
        struct {Temp_label label; string string; } string;
        struct {T_stm stm; F_frame frame; } proc;
    } u;
};

F_frag F_StringFrag(Temp_label label, string string);
F_frag F_ProcFrag(T_stm stm, F_frame frame);
void F_AddFrag(F_frag frag);

Temp_temp F_Fp(void);

T_exp F_Exp(F_access access, T_exp frame_ptr);
T_exp F_ExternalCall(string name, T_expList args);

#endif

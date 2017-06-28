#include "frame.h"

#define K 4
const int F_WORDSIZE = 4;

F_accessList F_AccessList(F_access head, F_accessList tail)
{
    F_accessList p = checked_malloc(sizeof(*p));
    p->head=head;
    p->tail=tail;
    return p;
}

F_frame F_NewFrame(Temp_label name, U_boolList formals)
{

}

Temp_label F_Name(F_frame fr)
{

}

F_accessList F_Formals(F_frame fr)
{

}

F_access F_AllocLocal(F_frame fr, bool escape)
{

}

int F_Offset(F_access access)
{

}

F_frag F_StringFrag(Temp_label label, string string)
{

}

F_frag F_ProcFrag(T_stm stm, F_frame frame)
{

}

void F_AddFrag(F_frag frag)
{

}

Temp_temp F_Fp(void)
{

}

T_exp F_Exp(F_access access, T_exp frame_ptr)
{

}

T_exp F_ExternalCall(string name, T_expList args)
{
	
}

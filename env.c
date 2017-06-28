#include <stdlib.h>

#include "env.h"

E_entry E_VarEntry(Tr_access access, Ty_ty type, bool forr)
{
    E_entry p = checked_malloc(sizeof(*p));
    p->kind = E_varEntry;
    p->u.var.access = access;
    p->u.var.type = type;
    p->u.var.forr = forr;
    return p;
}

E_entry E_FuncEntry(Tr_level level, Temp_label label, Ty_tyList formals, Ty_ty result)
{
    E_entry p = checked_malloc(sizeof(*p));
    p->kind = E_funcEntry;
    p->u.func.level = level;
    p->u.func.label = label;
    p->u.func.formals = formals;
    p->u.func.result = result;
    return p;
}

TAB_table E_base_tenv(void)
{
    TAB_table tab = S_empty();
    S_enter(tab, S_Symbol("int"), Ty_Int());
    S_enter(tab, S_Symbol("string"), Ty_String());
    return tab;
}


TAB_table E_base_venv(void)
{
    TAB_table tab = S_empty();
    S_enter(tab, S_Symbol("print"), E_FuncEntry(Tr_Outermost(),
                                                Temp_newlabel(), 
                                                Ty_TyList(Ty_String(), NULL), 
                                                Ty_Void()));

    S_enter(tab, S_Symbol("printi"), E_FuncEntry(Tr_Outermost(),
                                                Temp_newlabel(), 
                                                Ty_TyList(Ty_Int(), NULL), 
                                                Ty_Void()));

    S_enter(tab, S_Symbol("flush"), E_FuncEntry(Tr_Outermost(),
                                                Temp_newlabel(), 
                                                NULL, 
                                                Ty_Void()));

    S_enter(tab, S_Symbol("getchar"), E_FuncEntry(  Tr_Outermost(),
                                                    Temp_newlabel(), 
                                                    NULL, 
                                                    Ty_String()));

    S_enter(tab, S_Symbol("ord"), E_FuncEntry(  Tr_Outermost(),
                                                Temp_newlabel(), 
                                                Ty_TyList(Ty_String(), NULL), 
                                                Ty_Int()));

    S_enter(tab, S_Symbol("chr"), E_FuncEntry(  Tr_Outermost(),
                                                Temp_newlabel(), 
                                                Ty_TyList(Ty_Int(), NULL), 
                                                Ty_String()));

    S_enter(tab, S_Symbol("size"), E_FuncEntry( Tr_Outermost(),
                                                Temp_newlabel(), 
                                                Ty_TyList(Ty_String(), NULL), 
                                                Ty_Int()));

    S_enter(tab, S_Symbol("substring"), E_FuncEntry(Tr_Outermost(),
                                                    Temp_newlabel(), 
                                                    Ty_TyList(Ty_String(), Ty_TyList(Ty_Int(), Ty_TyList(Ty_Int(), NULL))), 
                                                    Ty_String()));

    S_enter(tab, S_Symbol("concat"), E_FuncEntry(   Tr_Outermost(),
                                                    Temp_newlabel(), 
                                                    Ty_TyList(Ty_String(), Ty_TyList(Ty_String(), NULL)), 
                                                    Ty_String()));

    S_enter(tab, S_Symbol("not"), E_FuncEntry(  Tr_Outermost(),
                                                Temp_newlabel(), 
                                                Ty_TyList(Ty_Int(), NULL), 
                                                Ty_Int()));

    S_enter(tab, S_Symbol("exit"), E_FuncEntry( Tr_Outermost(),
                                                Temp_newlabel(), 
                                                Ty_TyList(Ty_Int(), NULL), 
                                                Ty_Void()));

    return tab;
}

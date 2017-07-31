#ifndef INCLUDE__TAB_H
#define INCLUDE__TAB_H

#include "symbol.h"
#include "temp.h"
#include "translate.h"
#include "types.h"

typedef struct
{
    enum { E_varEntry, E_funcEntry } kind;
    union
    {
        struct
        {
            Tr_access access;
            Ty_ty type;
            bool forr;
        } var;

        struct
        {
            Tr_level level;
            Temp_label label;
            Ty_tyList formals;
            Ty_ty result;
        } func;
    } u;
} *E_entry;

E_entry E_VarEntry(Tr_access access, Ty_ty type, bool forr);
E_entry E_FuncEntry(Tr_level level,  Temp_label label, Ty_tyList formals, Ty_ty result);

TAB_table E_base_tenv(void);
TAB_table E_base_venv(void);

#endif

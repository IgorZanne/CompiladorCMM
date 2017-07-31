    #include <stdlib.h>

#include "env.h"
#include "errormsg.h"
#include "tree.h"
#include "prabsyn.h"
#include "semantic.h"
#include "types.h"
#include "translate.h"

static TAB_table E_venv;
static TAB_table E_tenv;
static char codeHeader[65536];
static char codeBody[65536];
static char codeFooter[65536];

static int virtualRegister = 0;


static Ty_exp exp_type(Tr_exp exp, Ty_ty type, char code[])
{

    Ty_exp result;
    result.exp = exp;
    result.type = type;
    strcpy(result.code, code);

    return result;
}

/* Função que busca o tipo na tabela de símbolos */
static Ty_ty search_type(S_symbol name, int pos)
{
    Ty_ty type = S_look(E_tenv, name);
    if (type)
        type = Ty_actual(type);
    else
        EM_error(pos, "Undefined type '%s'", S_name(name));

    return type;
}

static Ty_tyList formal_type_list(A_fieldList params, int pos)
{
    A_fieldList p, q = NULL, r = NULL;

    for (p = params; p; p = p->tail)
    {
        A_field field = p->head;
        Ty_ty type = search_type(field->typ, pos);
        if (!type)
            type = Ty_Int();

        if (r)
        {
            r->tail = Ty_TyList(type, NULL);
            r = r->tail;
        }
        else
            q = r = Ty_TyList(type, NULL);
    }
    return q;
}

static U_boolList formal_escape_list(A_fieldList params)
{
    A_fieldList p = params, q = NULL, r = NULL;
    for (; p; p = p->tail)
    {
        U_boolList bl = U_BoolList((p->head)->escape, NULL);
        if (r)
        {
            r->tail = bl;
            r = r->tail;
        }
        else
            q = r = bl;
    }
    return q;
}


static void trans_dec(Tr_level level, A_dec dec);
static Ty_ty trans_type(A_ty type);
static Ty_exp trans_var(Tr_level level, A_var var);
static Ty_exp trans_exp(Tr_level level, A_exp exp);

static void trans_func_dec(Tr_level level, A_dec dec)
{
    //Check for function redefinitions.
    A_fundec function = S_look(E_venv, dec->u.function.name);
    A_fundec decFunction = A_Fundec(dec->pos, dec->u.function.name, dec->u.function.params, dec->u.function.result, dec->u.function.body);

    if (dec->u.function.name == function->name)
    {
        EM_error(function->pos, "Function '%s' redefined", S_name(function->name));
    }
    else
    {
        //Enter function prototypes into symbol table. 
        
        Ty_tyList formals = formal_type_list(decFunction->params, dec->pos);
        U_boolList escapes = formal_escape_list(decFunction->params);
        Ty_ty result;

        Temp_label label = Temp_newlabel();

        if (decFunction->result)
        {
            result = search_type(decFunction->result, dec->pos);
            if (!result)
                result = Ty_Int();
        }
        else
            result = Ty_Void();

        
        S_enter(E_venv, decFunction->name, E_FuncEntry(Tr_Level(level, label, escapes), label, formals, result));

        
        // Translate the possibly mutually recursive functions
        E_entry entry = S_look(E_venv, decFunction->name);
        A_fieldList params = decFunction->params;
        Ty_tyList entryFormals = entry->u.func.formals;
        
        Tr_accessList s = Tr_Formals(entry->u.func.level)->tail;

        Ty_exp recResult;

        S_beginScope(E_venv);
        for (; params; params = params->tail, entryFormals = entryFormals->tail)
        {
            S_enter(E_venv, (params->head)->name, E_VarEntry(0, entryFormals->head, FALSE));
        }
        assert(!params && !entryFormals);
        
        recResult = trans_exp(entry->u.func.level, decFunction->body);
        if (!Ty_match(recResult.type, entry->u.func.result))
            EM_error(decFunction->pos, "Function body's type is incorrect");

        S_endScope(E_venv);
    }
   
}

static void trans_var_dec(Tr_level level, A_dec dec)
{

    printf("declarando var");
       
    Ty_exp init = trans_exp(level, dec->u.var.init);

    Ty_ty initType = init.type;
    Tr_access access = Tr_AllocLocal(level, FALSE); //sempre FALSE para ir tudo para temporários
    //Tr_access access = Tr_AllocLocal(level, dec->u.var.escape);


    Ty_ty type;
    if (dec->u.var.typ)
    {
        type = search_type(dec->u.var.typ, dec->pos);

        if (!type){
            type = Ty_Int();
        }
     
      
        if (!Ty_match(type, initType))
            EM_error(dec->pos, "Initializer has incorrect type.");
    }
    else if (init.type->kind == Ty_nil)
        EM_error(dec->pos, "Don't know which record type to take.");
    else if (init.type->kind == Ty_void)
        EM_error(dec->pos, "Can't assign void value to a variable.");
    else
        type = initType;

    switch(type->kind){
        case Ty_record: 
            break;
        case Ty_nil: 
            break;
        case Ty_int: 
            strcat(codeBody, "\%");
            strcat(codeBody, S_name(dec->u.var.var));
            strcat(codeBody, " = alloca i32, align 4\n");

            strcat(codeBody, "store ");
            strcat(codeBody, init.code);
            strcat(codeBody, ", i32* \%");
            strcat(codeBody, S_name(dec->u.var.var));
            strcat(codeBody, ", align 4");
            strcat(codeBody, "\n");

            //strcat(codeBody, init.code);
            break;
        case Ty_string: 
            strcat(codeHeader, "\n@program.");
            strcat(codeHeader, S_name(dec->u.var.var));
            int len = strlen(dec->u.var.init->u.stringg);
            strcat(codeHeader, " = private unnamed_addr constant [");
            strcat(codeHeader, len);
            strcat(codeHeader, " x i8] c\"");
            strcat(codeHeader, dec->u.var.init->u.stringg);
            strcat(codeHeader, "\" align 1");
            break;
        case Ty_array: 
            break;
        case Ty_name: 
            break;
        case Ty_void:
            break;
    }

    S_enter(E_venv, dec->u.var.var, E_VarEntry(access, type, FALSE));
}

static void trans_type_dec(Tr_level level, A_dec dec)
{
    /* Check for type redefinitions. */
    Ty_ty type = S_look(E_tenv, dec->u.type.name);
    if(type)
    {
        EM_error(dec->pos, "Type '%s' redefined", S_name(dec->u.type.name));
    }
    else
    {
        /* Enter type placeholder into symbol table. */
        Ty_ty type = trans_type(dec->u.type.ty);
        S_enter(E_tenv, dec->u.type.name, Ty_Name(dec->u.type.name, type));

        /* Check for infinite recursive types. */
        /*if (Ty_actual(type) == type)
            EM_error(dec->pos, "Infinite recursive type '%s'", S_name(dec->u.type.name));*/
    }

}


typedef void (*trans_dec_func)(Tr_level level, A_dec);
static trans_dec_func _trans_dec_funcs[] =
{
    trans_func_dec,
    trans_var_dec,
    trans_type_dec
};

static void trans_dec(Tr_level level, A_dec dec)
{
    _trans_dec_funcs[dec->kind](level, dec);
}

static Ty_exp trans_nil_exp(Tr_level level, A_exp exp)
{
    return exp_type(Tr_NumExp(0), Ty_Nil(), "");
}

static Ty_exp trans_var_exp(Tr_level level, A_exp exp)
{
    return trans_var(level, exp->u.var);
}

static Ty_exp trans_int_exp(Tr_level level, A_exp exp)
{

    char code[1000];
    strcpy(code, "");

    char s[11];
    sprintf(s, "%i", exp->u.intt);
    strcat(code, "i32 ");
    strcat(code, s);

    return exp_type(Tr_NumExp(exp->u.intt), Ty_Int(), code);
}

static Ty_exp trans_string_exp(Tr_level level, A_exp exp)
{
    return exp_type(Tr_StringExp(exp->u.stringg), Ty_String(), "");
}

static Ty_exp trans_call_exp(Tr_level level, A_exp exp)
{

    E_entry entry = S_look(E_venv, exp->u.call.func);

    Ty_tyList formals;
    A_expList args;
    Tr_expList args2 = NULL, tail = NULL;

    int i;

    if (!entry)
    {
        EM_error(exp->pos, "Undefined function '%s'.", S_name(exp->u.call.func));
        return exp_type(NULL, Ty_Int(), "");
    }
    else if (entry->kind != E_funcEntry)
    {
        EM_error(exp->pos, "'%s' is not a function.", S_name(exp->u.call.func));
        return exp_type(NULL, Ty_Int(), "");
    }


    for (formals = entry->u.func.formals, args = exp->u.call.args, i = 1; formals && args; formals = formals->tail, args = args->tail, i++)
    {
        Ty_exp et = trans_exp(level, args->head);
        if (!Ty_match(formals->head, et.type))
            EM_error(exp->pos, "Passing argument %d of '%s' with wrong type.", i, S_name(exp->u.call.func));

        if (args2)
            tail = tail->tail = Tr_ExpList(et.exp, NULL);
        else
            args2 = tail = Tr_ExpList(et.exp, NULL);
    }
    if (formals)
        EM_error(exp->pos, "Expect more arguments.");
    else if (args)
        EM_error(exp->pos, "Expect less arguments.");

    if(strstr(S_name(exp->u.call.func), "print"))
    {
        strcat(codeHeader, "\n@.str = private unnamed_addr constant [4 x i8] c\"%i\\0A\\00\", align 1\n");
        
        char leftReg[100];
        sprintf(leftReg, "%d", virtualRegister++);
        strcat(codeBody, "\%");
        strcat(codeBody, leftReg);
        strcat(codeBody, " = load i32* \%");
        strcat(codeBody, S_name(exp->u.call.args->head->u.var->u.simple));
        strcat(codeBody, ", align 4");
        strcat(codeBody, "\n");

        strcat(codeBody, "\%call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x i8]* @.str, i32 0, i32 0), i32 \%");
        strcat(codeBody, leftReg);
        strcat(codeBody, ")");

        strcat(codeFooter, "attributes #1 = { \"less-precise-fpmad\"=\"false\" \"no-frame-pointer-elim\"=\"true\" \"no-frame-pointer-elim-non-leaf\" \"no-infs-fp-math\"=\"false\" \"no-nans-fp-math\"=\"false\" \"stack-protector-buffer-size\"=\"8\" \"unsafe-fp-math\"=\"false\" \"use-soft-float\"=\"false\" }");


    }
    return exp_type(Tr_CallExp(entry->u.func.level, entry->u.func.label, args2), Ty_actual(entry->u.func.result), "");
}

static char ir_op[][12] = {"add", "sub", "mul", "sdiv"};

static Ty_exp trans_op_exp(Tr_level level, A_exp exp)
{
    A_oper op = exp->u.op.oper;
    Ty_exp left = trans_exp(level, exp->u.op.left);
    Ty_exp right = trans_exp(level, exp->u.op.right);
    Tr_exp result;

    switch (op) {
        case A_plusOp:
        case A_minusOp:
        case A_timesOp:
        case A_divideOp:
            if (left.type->kind != Ty_int)
                EM_error(exp->u.op.left->pos, "Integer required.");
            if (right.type->kind != Ty_int)
                EM_error(exp->u.op.right->pos, "Integer required.");

            char code[1000];
            strcpy(code, "");

            char leftReg[100];
            sprintf(leftReg, "%d", virtualRegister++);
            strcat(codeBody, "\%");
            strcat(codeBody, leftReg);
            strcat(codeBody, " = load ");
            strcat(codeBody, left.code);
            strcat(codeBody, "\n");

            char rightReg[100];
            sprintf(rightReg, "%d", virtualRegister++);
            strcat(codeBody, "\%");
            strcat(codeBody, rightReg);
            strcat(codeBody, " = load ");
            strcat(codeBody, right.code);
            strcat(codeBody, "\n");

            strcat(codeBody, "\%");
            strcat(codeBody, ir_op[op]);
            strcat(codeBody, " = ");
            strcat(codeBody, ir_op[op]);
            strcat(codeBody, " i32 \%");
            strcat(codeBody, leftReg);
            strcat(codeBody, ", \%");
            strcat(codeBody, rightReg);
            strcat(codeBody, "\n");

            strcat(code, "i32 \%");
            strcat(code, ir_op[op]);

            result = Tr_OpExp(op-A_plusOp+T_plus, left.exp, right.exp);
            return exp_type(result, Ty_Int(), code);

        case A_eqOp:
        case A_neqOp: {
            if (!Ty_match(left.type, right.type))
                EM_error(exp->pos, "The type of two operands must be the same. (Incompatible types)");
            else if (left.type->kind == Ty_string)
                result = Tr_StringRelExp(op-A_eqOp+T_eq, left.exp, right.exp);
            else
                result = Tr_RelExp(op-A_eqOp+T_eq, left.exp, right.exp);

            return exp_type(result, Ty_Int(), "");
        }

        case A_ltOp:
        case A_leOp:
        case A_gtOp:
        case A_geOp: {
            if (!Ty_match(left.type, right.type))
                EM_error(exp->pos, "The type of two operands must be the same. (Incompatible types)");
            if (left.type->kind != Ty_int && left.type->kind != Ty_string)
                EM_error(exp->pos, "The type of comparison's operand must be int or string.");
            if (left.type->kind == Ty_string)
                result = Tr_StringRelExp(op-A_ltOp+T_lt, left.exp, right.exp);
            else
                result = Tr_RelExp(op-A_ltOp+T_lt, left.exp, right.exp);

            return exp_type(result, left.type, "");
        }

        /*NÃO SEI O QUE VALIDAR NEM RETORNAR NESSE CASO, POIS DE ACORDO COM O LIVRO, NON-ZERO &/| NON-ZERO É VERDADEIRO, APENAS INTEGER ZERO É FALSE*/
        case A_andOp:
        {
            result = Tr_OpExp(T_and, left.exp, right.exp);
            return exp_type(result, Ty_Int(), "");
        }
        case A_orOp:
        {
            result = Tr_OpExp(T_or, left.exp, right.exp);
            return exp_type(result, Ty_Int(), "");
        }
    }

    assert(0);
}

static Ty_exp trans_record_exp(Tr_level level, A_exp exp)
{
    Ty_ty type = search_type(exp->u.record.typ, exp->pos);

    Ty_fieldList p;
    A_efieldList q;
    A_expList fields = NULL, tail = NULL;
    Tr_exp tr;

    int size = 0;

    
    if (!type)
        return exp_type(NULL, Ty_Nil(), "");
    if (type->kind != Ty_record)
        EM_error(exp->pos, "'%s' is not a record type.", S_name(exp->u.record.typ));

        
    for (p = type->u.record, q = exp->u.record.fields; p && q; p = p->tail, q = q->tail, size++)
    {
        //segfault, entrando uma vez a mais do que o número de fields
        A_efield efield = q->head;
        Ty_exp et = trans_exp(level, efield->exp);
        
        if (!Ty_match(p->head->ty, et.type))
            EM_error(efield->pos, "Wrong field type.");

        if (fields)
            tail = tail->tail = A_ExpList(et.exp, NULL);
        else
            fields = tail = A_ExpList(et.exp, NULL);
    }

    if (p || q)
        EM_error(exp->pos, "Wrong field number.");

    return exp_type(tr, type, "");
}

static Ty_exp trans_array_exp(Tr_level level, A_exp exp)
{
    Ty_ty type = search_type(exp->u.array.typ, exp->pos);
    Ty_exp size = trans_exp(level, exp->u.array.size);
    Ty_exp init = trans_exp(level, exp->u.array.init);

    if (!type)
        return exp_type(NULL, Ty_Int(), "");
    if (type->kind != Ty_array)
        EM_error(exp->pos, "'%s' is not an array type.", S_name(exp->u.array.typ));
    if (size.type->kind != Ty_int)
        EM_error(exp->pos, "Array's size must be the int type.");
    if (!Ty_match(type->u.array, init.type))
        EM_error(exp->pos, "Initializer has incorrect type.");

    return exp_type(Tr_ArrayExp(size.exp, init.exp), type, "");
}

static Ty_exp trans_seq_exp(Tr_level level, A_exp exp)
{
    A_expList exps = exp->u.seq;
    A_expList stms = NULL, tail = NULL;
    Tr_exp tr;

    char code[1000];
    strcpy(code, "");

    for (; exps; exps = exps->tail)
    {
        Ty_exp t_exp = trans_exp(level, exps->head);
        strcat(code, t_exp.code);
        strcat(code, "\n");

        if (stms)
            tail = tail->tail = A_ExpList(t_exp.exp, NULL);

        else
            stms = tail = A_ExpList(t_exp.exp, NULL);

        if (!exps->tail)
        {
            return exp_type(tr, t_exp.type, code);
        }
    }
    return exp_type(Tr_NumExp(0), Ty_Void(), "");
}

static Ty_exp trans_if_exp(Tr_level level, A_exp exp)
{
    Ty_exp test = trans_exp(level, exp->u.iff.test);
    Ty_exp then = trans_exp(level, exp->u.iff.then);
    Tr_exp tr;

    if (test.type->kind != Ty_int)
        EM_error(exp->pos, "Test's expression type must be integer.'");
    if (exp->u.iff.elsee)
    {
        Ty_exp elsee = trans_exp(level, exp->u.iff.elsee);
        if (!Ty_match(then.type, elsee.type))
            EM_error(exp->pos, "Types of then's expression and else's expression differ.");
        
        return exp_type(tr, then.type, "");
    }
    else if (then.type->kind != Ty_void)
        EM_error(exp->pos, "If-then's expression should return nothing.");

    return exp_type(tr, Ty_Void(), "");
}

static Ty_exp trans_while_exp(Tr_level level, A_exp exp)
{
    Ty_exp test = trans_exp(level, exp->u.whilee.test);
    Ty_exp body = trans_exp(level, exp->u.whilee.body);
    Tr_exp tr;
    if (test.type->kind != Ty_int)
        EM_error(exp->pos, "Test's type must be integer.");
    if (body.type->kind != Ty_void)
        EM_error(exp->pos, "While expession should return nothing.");

    return exp_type(tr, Ty_Void(), "");
}
static Ty_exp trans_for_exp(Tr_level level, A_exp exp)
{
    Ty_exp lo = trans_exp(level, exp->u.forr.lo);
    Ty_exp hi = trans_exp(level, exp->u.forr.hi);
    Ty_exp body;
    Tr_exp tr;

    //Tr_access access = Tr_AllocLocal(level, exp->u.forr.escape);

    if (lo.type->kind != Ty_int)
        EM_error(exp->pos, "Lo expession should be int type.");
    if (hi.type->kind != Ty_int)
        EM_error(exp->pos, "Hi expession should be int type.");
    S_beginScope(E_venv);

    S_enter(E_venv, exp->u.forr.var, E_VarEntry(0, Ty_Int(), TRUE));

    body = trans_exp(level, exp->u.forr.body);
    if (body.type->kind != Ty_Void)
        EM_error(exp->pos, "For expession should return nothing.");
    S_endScope(E_venv);

    return exp_type(tr, Ty_Void(), "");
}
static Ty_exp trans_break_exp(Tr_level level, A_exp exp)
{
    Ty_exp t;
    return t;
}
static Ty_exp trans_let_exp(Tr_level level, A_exp exp)
{
    Ty_exp result;
    A_decList p;

    S_beginScope(E_venv);
    S_beginScope(E_tenv);
    for (p = exp->u.let.decs; p; p = p->tail)
        trans_dec(level, p->head);
    result = trans_exp(level, exp->u.let.body);
    S_endScope(E_venv);
    S_endScope(E_tenv);
    return result;
}
static Ty_exp trans_assign_exp(Tr_level level, A_exp exp)
{
    Ty_exp t_exp = trans_exp(level, exp->u.assign.exp);
    Ty_exp var = trans_var(level, exp->u.assign.var);
    Tr_exp tr;

    if (!Ty_match(var.type, t_exp.type))
        EM_error(exp->pos, "Type mismatch.");

    /*if (exp->u.assign.var->kind == A_simpleVar && var.type->kind == Ty_int)
    {
        // Check for the assignment to the for variable.
        ast_var_t v = exp->u.assign.var;
        env_entry_t entry = sym_lookup(_venv, v->u.simple);
        if (entry && entry->kind == ENV_VAR_ENTRY && entry->u.var.for_)
            em_error(exp->pos, "assigning to the for variable");
    }*/


    char code[1000];
    strcpy(code, "");
        
    strcat(code, "store ");
    strcat(code, t_exp.code);
    strcat(code, ", ");
    strcat(code, var.code);
    strcat(code, "\n");

    return exp_type(tr, Ty_Void(), code);
}

typedef Ty_exp (*trans_exp_func)(Tr_level level, A_exp);
static trans_exp_func _trans_exp_funcs[] =
{
    trans_nil_exp,
    trans_var_exp,
    trans_int_exp,
    trans_string_exp,
    trans_call_exp,
    trans_op_exp,
    trans_record_exp,
    trans_seq_exp,
    trans_assign_exp,
    trans_if_exp,
    trans_while_exp,
    trans_for_exp,
    trans_break_exp,
    trans_let_exp,
    trans_array_exp
};

static Ty_exp trans_exp(Tr_level level, A_exp exp)
{
    Ty_exp result = _trans_exp_funcs[exp->kind](level, exp);
    return result;
}


/* Tradução de Tipos */

/* Tradução de NAMETYPE */
static Ty_ty trans_name_type(A_ty type)
{
    Ty_ty t = S_look(E_tenv, type->u.name);
    if (!t)
    {
        EM_error(type->pos, "Undefined type '%s'", S_name(type->u.name));
        t = Ty_Int();
    }
    return t;
}

/* Tradução de RECORDTYPE*/
static Ty_ty trans_record_type(A_ty type)
{
    A_fieldList p = type->u.record;
    Ty_fieldList q = NULL, r = NULL;

    for (; p; p = p->tail)
    {
        A_field field = p->head;
        Ty_ty t = S_look(E_tenv, field->typ);

        if (!t)
        {
            EM_error(type->pos, "Undefined type '%s'", S_name(field->typ));
            t = Ty_Int();
        }
        if (r)
        {
            r->tail = Ty_FieldList(Ty_Field(field->name, t), NULL);
            r = r->tail;
        }
        else
            q = r = Ty_FieldList(Ty_Field(field->name, t), NULL);
    }

    return Ty_Record(q);
}

/* Tradução de ARRAYTYPE */
static Ty_ty trans_array_type(A_ty type)
{
    Ty_ty t = S_look(E_tenv, type->u.array);
    if (!t)
    {
        EM_error(type->pos, "Undefined type '%s'", type->u.array);
        t = Ty_Int();
    }
    return Ty_Array(t);
}

/* Simplificação de um switch no kind pra saber qual translate utilizar */
typedef Ty_ty (*trans_type_func)(A_ty);
static trans_type_func _trans_type_funcs[] =
{
    trans_name_type,
    trans_record_type,
    trans_array_type,
};

static Ty_ty trans_type(A_ty type)
{
    return _trans_type_funcs[type->kind](type);
}

/* Tradução de Variáveis */

static Ty_exp trans_simple_var(Tr_level level, A_var var)
{
    E_entry entry = S_look(E_venv, var->u.simple);

    if (!entry)
    {
        EM_error(var->pos, "Undefined variable '%s'.", S_name(var->u.simple));
        return exp_type(Tr_NumExp(0), Ty_Int(), "");
    }
    else if (entry->kind != E_varEntry)
    {
        EM_error(var->pos, "Expected '%s' to be a variable, not a function.", S_name(var->u.simple));
        return exp_type(Tr_NumExp(0), Ty_Int(), "");
    }

    char code[1000];
    strcpy(code, "");

    switch(entry->u.var.type->kind)
    {
        case Ty_record:
            break;
        case Ty_nil: 
            break;
        case Ty_int:
            strcat(code, "i32* \%");
            strcat(code, S_name(var->u.simple));
            strcat(code, ", align 4"); 
            break;
        case Ty_string: 
            strcat(code, "\%");
            strcat(code, S_name(var->u.simple));
            strcat(code, " to i8*");
            break;
        case Ty_array: 
            break;
        case Ty_name: 
            break;
        case Ty_void:
            break;
    }

    return exp_type(Tr_SimpleVar(entry->u.var.access, level), Ty_actual(entry->u.var.type), code);

    
}

static Ty_exp trans_field_var(Tr_level level, A_var var)
{
    Ty_exp et = trans_var(level, var->u.field.var);
    Ty_fieldList p;

    if (et.type->kind != Ty_record)
    {
        EM_error(var->pos, "Expected record type variable.");
        return exp_type(NULL, Ty_Int(), "");
    }
    for (p = et.type->u.record; p; p = p->tail)
    {
        Ty_field field = p->head;
        if (field->name == var->u.field.sym)
            return exp_type(NULL, Ty_actual(field->ty), "");
    }
    EM_error(var->pos, "There is no field named '%s'", S_name(var->u.field.sym));
    return exp_type(NULL, Ty_Int(), "");
}

static Ty_exp trans_subscript_var(Tr_level level, A_var var)
{
    Ty_exp et = trans_var(level, var->u.subscript.var);
    Ty_exp sub = trans_exp(level, var->u.subscript.exp);

    if (et.type->kind != Ty_array)
    {
        EM_error(var->pos, "Expected array type variable.");
        return exp_type(NULL, Ty_Int(), "");
    }
    if (sub.type->kind != Ty_int)
        EM_error(var->pos, "Expected integer type subscript.");
    return exp_type(NULL, Ty_actual(et.type->u.array), "");
}

typedef Ty_exp (*trans_var_func)(Tr_level level, A_var);
static trans_var_func _trans_var_funcs[] =
{
    trans_simple_var,
    trans_field_var,
    trans_subscript_var
};

static Ty_exp trans_var(Tr_level level, A_var var)
{
    return _trans_var_funcs[var->kind](level, var);
}



char* semantic_proc(A_exp prog)
{
    E_tenv = E_base_tenv();
    E_venv = E_base_venv();
    strcpy(codeHeader, "");
    strcpy(codeBody, "");
    strcpy(codeFooter, "");


    strcat(codeHeader, ";ModuleID = '/tmp/webcompile/_5832_0.bc'\ntarget datalayout = \"e-m:e-i64:64-f80:128-n8:16:32:64-S128\"\ntarget triple = \"x86_64-unknown-linux-gnu\"\n\n");


    strcat(codeBody, "define i32 @main() #0 {\n");
    strcat(codeBody, "entry:\n");

    strcat(codeFooter, "declare i32 @printf(i8*, ...) #1\n\n");
    strcat(codeFooter, "attributes #0 = { nounwind uwtable \"less-precise-fpmad\"=\"false\" \"no-frame-pointer-elim\"=\"true\" \"no-frame-pointer-elim-non-leaf\" \"no-infs-fp-math\"=\"false\" \"no-nans-fp-math\"=\"false\" \"stack-protector-buffer-size\"=\"8\" \"unsafe-fp-math\"=\"false\" \"use-soft-float\"=\"false\" }\n\n");

    
    Ty_exp result = trans_exp(Tr_Outermost(), prog);

    strcat(codeFooter, "\n\n!llvm.ident = !{!0}\n");
    strcat(codeFooter, "!0 = metadata !{metadata !\"clang version 3.6.0 (trunk)\"}");

    strcat(codeBody, result.code);
    strcat(codeBody, "\nret i32 0\n}\n\n");




    char stringResult[65536];
    strcat(stringResult, "");

    strcat(stringResult, codeHeader);
    strcat(stringResult, codeBody);
    strcat(stringResult, codeFooter);

    return stringResult;

    // printf("%s\n", stringResult);
}
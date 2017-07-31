#include "escape.h"
#include "symbol.h"


static ESC_entry escape_entry(int depth, bool *escape)
{
    assert(escape);
    ESC_entry p = checked_malloc(sizeof(*p));
    p->depth = depth;
    p->escape = escape;
    *escape = FALSE;
    return p;
}

static void traverse_dec(A_dec dec);
static void traverse_exp(A_exp exp);
static void traverse_var(A_var var);

static void traverse_dec(A_dec dec)
{
    switch (dec->kind)
    {
        case A_functionDec: {
            A_fundec func = A_Fundec(dec->pos, dec->u.function.name, dec->u.function.params,dec->u.function.result, dec->u.function.body);
            A_fieldList q;

            _depth++;
            S_beginScope(_env);
            for (q = func->params; q; q = q->tail)
            {
                A_field field = q->head;
                S_enter(_env, field->name, escape_entry(_depth, &field->escape));
            }
            traverse_exp(func->body);
            S_endScope(_env);
            _depth--;
            break;
        }

        case A_typeDec:
            break;

        case A_varDec:
            S_enter(_env, dec->u.var.var, escape_entry(_depth, &dec->u.var.escape));
            traverse_exp(dec->u.var.init);
            break;
    }
}

static void traverse_exp(A_exp exp)
{
    A_expList p, r;
    A_efieldList q;
    A_decList s;

    switch (exp->kind)
    {
        case A_nilExp:
            break;

        case A_varExp:
            traverse_var(exp->u.var);
            break;

        case A_intExp:
        case A_stringExp:
            break;

        case A_callExp:
            for (p = exp->u.call.args; p; p = p->tail)
                traverse_exp(p->head);
            break;

        case A_opExp:
            traverse_exp(exp->u.op.left);
            traverse_exp(exp->u.op.right);
            break;

        case A_recordExp:
            for (q = exp->u.record.fields; q; q = q->tail)
                traverse_exp((q->head)->exp);
            break;

        case A_seqExp:
            for (r = exp->u.seq; r; r = r->tail)
                traverse_exp(r->head);
            break;

        case A_assignExp:
            traverse_var(exp->u.assign.var);
            traverse_exp(exp->u.assign.exp);
            break;

        case A_ifExp:
            traverse_exp(exp->u.iff.test);
            traverse_exp(exp->u.iff.then);
            if (exp->u.iff.elsee)
                traverse_exp(exp->u.iff.elsee);
            break;

        case A_whileExp:
            traverse_exp(exp->u.whilee.test);
            traverse_exp(exp->u.whilee.body);
            break;

        case A_forExp:
            traverse_exp(exp->u.forr.lo);
            traverse_exp(exp->u.forr.hi);
            S_beginScope(_env);
            S_enter(_env, exp->u.forr.var, escape_entry(_depth, &exp->u.forr.escape));
            traverse_exp(exp->u.forr.body);
            S_endScope(_env);
            break;
            
        case A_breakExp:
            break;

        case A_arrayExp:
            traverse_exp(exp->u.array.size);
            traverse_exp(exp->u.array.init);
            break;
    }
}

static void traverse_var(A_var var)
{
    switch (var->kind)
    {
        case A_simpleVar: {
            ESC_entry entry = S_look(_env, var->u.simple);
            if (entry && entry->depth < _depth)
                *entry->escape = TRUE;
            break;
        }

        case A_fieldVar:
            traverse_var(var->u.field.var);
            break;

        case A_subscriptVar:
            traverse_var(var->u.subscript.var);
            traverse_exp(var->u.subscript.exp);
            break;
    }
}

void esc_find_escape(A_exp exp)
{
    _depth = 0;
    _env = S_empty();
    traverse_exp(exp);
}

#include <assert.h>

#include "frame.h"
#include "tree.h"
#include "translate.h"


static Tr_level _outermost = NULL;

static Tr_access Tr_Access(Tr_level level, F_access access) //FRAME
{
    Tr_access p = checked_malloc(sizeof(*p));
    p->level = level;
    p->access = access;
    return p;
}

Tr_accessList Tr_AccessList(Tr_access head, Tr_accessList tail)
{
    Tr_accessList p = checked_malloc(sizeof(*p));
    p->head=head;
    p->tail=tail;
    return p;
}

Tr_expList Tr_ExpList(Tr_exp head, Tr_expList tail)
{
    Tr_expList p = checked_malloc(sizeof(*p));
    p->head=head;
    p->tail=tail;
    return p;
}


Tr_level Tr_Outermost(void)
{
    if (!_outermost)
        _outermost = Tr_Level(NULL, Temp_newlabel(), NULL);
    return _outermost;
}

Tr_level Tr_Level(Tr_level parent, Temp_label name, U_boolList formals) //FRAME
{
    Tr_level p = checked_malloc(sizeof(*p));
    F_accessList f_formals, q = NULL;  //FRAME

    p->parent = parent;
    /* extra formal for static link */
    p->frame = F_NewFrame(name, U_BoolList(TRUE, formals));
    f_formals = F_Formals(p->frame);  //FRAME
    for (; f_formals; f_formals = f_formals->tail)  //FRAME
    {
        Tr_access access = Tr_Access(p, f_formals->head);  //FRAME
        if (q)
        {
            q->tail = F_AccessList(access, NULL);
            q = q->tail;
        }
        else
            p->formals = q = F_AccessList(access, NULL);
    }
    p->locals = NULL;
    return p;
}

Tr_accessList Tr_Formals(Tr_level level)
{
    return level->formals;
}

Tr_access Tr_AllocLocal(Tr_level level, bool escape)
{
    F_access f_access = F_AllocLocal(level->frame, escape); //FRAME
    Tr_access access = Tr_Access(level, f_access); //FRAME

    if (level->locals)
    {
        Tr_accessList p = level->locals;
        while (p->tail)
            p = p->tail;
        p->tail = Tr_AccessList(access, NULL);
    }
    else
        level->locals = Tr_AccessList(access, NULL);
    return access;
}

static Tr_access tr_static_link(Tr_level level)
{
    assert(level);
    return level->formals->head;
}

static Tr_exp tr_ex(T_exp exp)
{
    Tr_exp p = checked_malloc(sizeof(*p));
    p->kind = Tr_ex;
    p->u.ex = exp;
    return p;
}

static Tr_exp tr_nx(T_stm stm)
{
    Tr_exp p = checked_malloc(sizeof(*p));
    p->kind = Tr_nx;
    p->u.nx = stm;
    return p;
}

static Tr_exp tr_cx(Temp_labelList trues, Temp_labelList falses, T_stm stm)
{
    Tr_exp p = checked_malloc(sizeof(*p));
    p->kind = Tr_cx;
    p->u.cx.trues = trues;
    p->u.cx.falses = falses;
    p->u.cx.stm = stm;
    return p;
}

static void fill_patch(Temp_labelList patchs, Temp_label label)
{
    Temp_labelList p = patchs;
    for (; p; p = p->tail)
        *((Temp_label *) p->head) = label;
}

static T_exp un_ex(Tr_exp exp)
{
    switch (exp->kind)
    {
        case Tr_ex:
            return exp->u.ex;
        case Tr_nx:
            return T_Eseq(exp->u.nx, T_Const(0));
        case Tr_cx: {
            Temp_temp tmp = Temp_newtemp();
            Temp_label t = Temp_newlabel();
            Temp_label f = Temp_newlabel();
            fill_patch(exp->u.cx.trues, t);
            fill_patch(exp->u.cx.falses, f);

            return T_Eseq(T_Seq(T_Move(T_Temp(tmp), T_Const(1)),
                            T_Seq(exp->u.cx.stm,
                                T_Seq(T_Label(f),
                                    T_Seq(T_Move(T_Temp(tmp), T_Const(0)), T_Label(t))))),
                        T_Temp(tmp));
        }
    }

    assert(0);
}

static T_stm un_nx(Tr_exp exp)
{
    switch (exp->kind)
    {
        case Tr_ex:
            return T_Exp(exp->u.ex);
        case Tr_nx:
            return exp->u.nx;
        case Tr_cx: {
            Temp_label label = Temp_newlabel();
            fill_patch(exp->u.cx.trues, label);
            fill_patch(exp->u.cx.falses, label);

            return T_Seq(exp->u.cx.stm, T_Label(label));
        }
    }

    assert(0);
}

static cx_t un_cx(Tr_exp exp)
{
    T_stm T_Cjump(T_relOp op, T_exp left, T_exp right, 
          Temp_label true, Temp_label false);
    switch (exp->kind)
    {
        case Tr_ex: {
            cx_t cx;
            cx.stm = T_Cjump(
              T_eq, exp->u.ex, T_Const(0), NULL, NULL);
            cx.trues = Temp_TempList(&(cx.stm->u.CJUMP.true), NULL);
            cx.falses = Temp_TempList(&(cx.stm->u.CJUMP.false), NULL);
            return cx;
        }
        case Tr_nx:
            assert(0);
        case Tr_cx:
            return exp->u.cx;
    }

    assert(0);
}

Tr_exp Tr_NumExp(int num)
{
    return tr_ex(T_Const(num));
}

Tr_exp Tr_StringExp(string str)
{
    Temp_label label = Temp_newlabel();
    F_frag frag = F_StringFrag(label, str); //FRAME
    F_AddFrag(frag);
    return tr_ex(T_Name(label));
}

Tr_exp Tr_CallExp(Tr_level level, Temp_label label, Tr_expList args)
{
    T_exp func = T_Name(label);
    T_exp fp = T_Const(F_Offset(tr_static_link(level)->access)); //FRAME

    T_expList l_args = T_ExpList(fp, NULL);
    T_expList l_tail = l_args;

    for (; args; args = args->tail)
        l_tail = l_tail->tail = T_ExpList(un_ex(args->head), NULL);
   
    return tr_ex(T_Call(func, l_args));
}

Tr_exp Tr_OpExp(int op, Tr_exp left, Tr_exp right)
{
    T_exp l = un_ex(left);
    T_exp r = un_ex(right);
    return tr_ex(T_Binop(op, l, r));
}

Tr_exp Tr_RelExp(int op, Tr_exp left, Tr_exp right)
{
    T_stm stm = T_Cjump(op, un_ex(left), un_ex(right), NULL, NULL); 
    return tr_cx(Temp_LabelList(&stm->u.CJUMP.true, NULL),
                 Temp_LabelList(&stm->u.CJUMP.false, NULL),
                 stm);
}

Tr_exp Tr_StringRelExp(int op, Tr_exp left, Tr_exp right)
{
    T_exp exp = F_ExternalCall("_CompareString", T_ExpList(un_ex(left), T_ExpList(un_ex(right), NULL))); //FRAME
    T_stm stm = T_Cjump(op, exp, T_Const(0), NULL, NULL);
    return tr_cx(Temp_LabelList(&stm->u.CJUMP.true, NULL),
                 Temp_LabelList(&stm->u.CJUMP.false, NULL),
                 stm);
}

Tr_exp Tr_RecordExp(Tr_expList fields, int size)
{
    T_exp addr = T_Temp(Temp_newtemp());
    T_exp alloc = F_ExternalCall("_Alloc", T_ExpList(T_Const(size * F_WORDSIZE), NULL)); //FRAME
    Tr_expList p;
    T_stmList q = NULL, r = NULL;
    int i;

    for (p = fields, i = 0; p; p = p->tail, i++)
    {
        Tr_exp field = p->head;
        T_exp offset = T_Binop(T_plus, addr, T_Const(F_WORDSIZE * i));
        T_stm stm = T_Move(T_Mem(offset), un_ex(field));
        T_stmList tail = T_StmList(stm, NULL);
        if (q)
        {
            r->tail = tail;
            r = tail;
        }
        else
            q = r = tail;
    }
    return tr_ex(T_Eseq(T_Seq(T_Move(addr, alloc), q),addr));
}

Tr_exp Tr_ArrayExp(Tr_exp size, Tr_exp init)
{
    return tr_ex(F_ExternalCall("_InitArray", T_ExpList(un_ex(size), T_ExpList(un_ex(init), NULL)))); //FRAME
}

Tr_exp Tr_SeqExp(Tr_expList stms)
{
    Tr_expList result = NULL, tail = NULL;
    Tr_expList p = stms;
    for (; p; p = p->tail)
    {
        T_exp exp = un_ex(p->head);
        if (result)
            tail = tail->tail = Tr_ExpList(exp, NULL);
        else
            result = tail = Tr_ExpList(exp, NULL);
    }
    return tr_nx(T_Seq(result->head, NULL)); //verificar.. ta errado
}

Tr_exp Tr_IfExp(Tr_exp test, Tr_exp then, Tr_exp elsee)
{
    Temp_label t = Temp_newlabel();
    Temp_label f = Temp_newlabel();
    Temp_label done = Temp_newlabel();
    cx_t cx = un_cx(test);
    T_exp result = T_Temp(Temp_newtemp());

    fill_patch(cx.trues, t);
    fill_patch(cx.falses, f);
    if (elsee)
    {
        return tr_ex(T_Eseq(T_Seq(cx.stm, 
                                T_Seq(T_Label(t), 
                                    T_Seq(T_Move(T_Mem(result), un_ex(then)), 
                                        T_Seq(T_Jump(T_Name(done), T_ExpList(done, NULL)),
                                            T_Seq(T_Label(f),
                                                T_Seq(T_Move(T_Mem(result), un_ex(elsee)), T_Label(done))))))),
                                                    result));
    }
    else
    {
        return tr_nx(T_Seq(cx.stm,
            T_Seq(T_Label(t), 
                T_Seq(un_nx(then), T_Label(f)))));
    }
    return NULL;
}

Tr_exp Tr_WhileExp(Tr_exp test, Tr_exp body)
{
    Temp_label start = Temp_newlabel();
    Temp_label loop = Temp_newlabel();
    Temp_label done = Temp_newlabel();
    cx_t cx = un_cx(test);

    fill_patch(cx.trues, loop);
    fill_patch(cx.falses, done);
    return tr_nx(T_Seq(T_Label(start),
                    T_Seq(cx.stm, 
                        T_Seq(T_Label(loop),
                            T_Seq(un_nx(body),
                                T_Seq(T_Jump(T_Name(start), T_ExpList(start, NULL)), T_Label(done)))))));
}

Tr_exp Tr_ForExp(Tr_access access, Tr_exp low, Tr_exp high, Tr_exp body)
{
    T_exp var = F_Exp(access->access, T_Temp(F_Fp()));  //FRAME

    Temp_label start = Temp_newlabel();
    Temp_label loop = Temp_newlabel();
    Temp_label done = Temp_newlabel();
    T_stm test = T_Cjump(T_le, var, un_ex(high), loop, done);
    return tr_nx(T_Seq(T_Move(var, un_ex(low)),
                    T_Seq(T_Label(start),
                        T_Seq(test,
                            T_Seq(T_Label(loop),
                                T_Seq(un_nx(body),
                                    T_Seq(T_Move(var, T_Binop(T_plus, var, T_Const(1))), T_Label(done))))))));
}

Tr_exp Tr_AssignExp(Tr_exp lhs, Tr_exp rhs)
{
    return tr_nx(T_Move(un_ex(lhs), un_ex(rhs)));
}

Tr_exp Tr_SimpleVar(Tr_access access, Tr_level level)
{
    T_exp fp = T_Temp(F_Fp()); //FRAME

    while (level != access->level)
    {
        F_access f_access = tr_static_link(level)->access; //FRAME
        fp = T_Mem(T_Binop(T_plus, fp, T_Const(F_Offset(f_access)))); //FRAME
        level = level->parent;
    }
    return tr_ex(T_Mem(T_Binop(T_plus, fp, T_Const(F_Offset(access->access))))); //FRAME
}

#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#define CLONE \
    cloned->parent = m_parent; \
    cloned->value = value;\
    if (left!=nullptr) cloned->left = left->clone(cloned); \
    if (right!=nullptr) cloned->right = right->clone(cloned);\
    return cloned; \

#include "myvariant.h"

class Block;
typedef void (Block::* PointerToRun) (MyVariant*, QList<MyVariant*>*);
typedef void (Block::*PointerToRunChild) (int);
typedef void (Block::* PointerToAdd) (const QString&, DataType, int, real_type*);

class AbstractExpr
{    
public:
    AbstractExpr* parent;
    AbstractExpr* left;
    AbstractExpr* right;
    QList<AbstractExpr*> arguments;
    MyCache* stack;
    MyVariant* value;
    AbstractExpr(MyVariant* m_value) : value (m_value)
    {
        parent = left = right = nullptr;
    }
    virtual ~AbstractExpr() {}
    inline virtual void eval() = 0;
    virtual bool assign(MyVariant* rvalue) = 0;
    virtual AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const = 0;
};

class Literal: public AbstractExpr
{

public:
    Literal(MyVariant* m_value) : AbstractExpr(m_value) {}

    bool assign(MyVariant* rvalue) {
        return false;
    }

    void eval()
    {

    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        Literal* cloned = new Literal(value);
        CLONE
    }
};

class ArrayElement: public AbstractExpr
{

public:
    ArrayElement(MyVariant* m_value) : AbstractExpr(m_value) {}

   /* int getIndex()
    {
        right->eval();
        MyVariant temp( right->value) ;
        if (temp.getDataType()==INTEGER && !temp.isArray())
        {
            int result = temp.toInt();
            if (result>=0 && result < value->getSize())
                return result;
            else {
                reportError("Bad index");
                return -1;
            }
        }
        reportError("Index must be an integer number");
        return -1;
    }*/

    bool assign(MyVariant *rvalue) {
       /* if (rvalue->getDataType()!=value->getDataType() ) {
            reportError("Array element could be assigned same type");
            return false;
        }

        if (!value->isArray()) {
            reportError("It isn't' array");
            return false;
        }
        int index = getIndex();
        value->setElement(rvalue, index);
        return true;*/
    }

    void eval()
    {
       /* if (!value->isArray()) {
            reportError("It isn't' array");
        }

        int index = getIndex();

        if (index!=-1)
        {
            value = MyVariant(value->getElement(index));
        }
        else
            return;*/
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        /*
        ArrayElement* cloned = new ArrayElement(value);
        CLONE*/
    }
};

class Variable: public AbstractExpr
{

public:
    Variable(MyVariant* m_value) : AbstractExpr(m_value) {}

    void eval()
    {

    }

    bool assign(MyVariant *rvalue) {
        *value = *rvalue;
        //std::cout << value->print().toStdString();
        return true;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        Variable* cloned = new Variable(value);
        CLONE
    }
};

class Op : public AbstractExpr
{
public:
    Op(MyVariant* m_value) : AbstractExpr(m_value) {}
    bool assign(MyVariant* rvalue) {
        return false;
    }
};



class CallFunction : public Op
{

private:
    PointerToRun pToRun;
    Block* parentBlock;

public:
    CallFunction(Block* m_parentBlock, PointerToRun m_pToRun, MyVariant* m_value) : parentBlock(m_parentBlock) , pToRun(m_pToRun), Op(m_value) {}
    void eval()
    {
        QList<MyVariant*> parameters;
        foreach (AbstractExpr* expr, arguments)
        {
            expr->eval();
            parameters.append(expr->value);
        }

        (parentBlock->*pToRun)(value, &parameters);

    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        CallFunction* cloned = new CallFunction(parentBlock, pToRun, value);
        cloned->parent = m_parent;
        foreach (AbstractExpr* expr, arguments)
        {
            cloned->arguments.append(expr->clone(cloned));
        }
        return cloned;
    }
};

class IfOp : public Op
{
private:
    PointerToRunChild pToRunChild;
    Block* parentBlock;
    int ifTrueId, ifFalseId;
public:
    IfOp( MyVariant* m_value, Block* m_parentBlock, PointerToRunChild m_pToRunChild, int m_IfTrueId, int m_IfFalseId = -1) : Op(m_value), parentBlock(m_parentBlock) , pToRunChild(m_pToRunChild), ifTrueId(m_IfTrueId), ifFalseId(m_IfFalseId) {}

    void eval()
    {
        left->eval();
        if (left->value->toInt())
        {
            (parentBlock->*pToRunChild)(ifTrueId);
        }
        else {
            (parentBlock->*pToRunChild)(ifFalseId);
        }

    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IfOp* cloned = new IfOp(value, parentBlock, pToRunChild, ifTrueId, ifFalseId);
        CLONE
    }
};

class WhileOp: public Op
{
private:
    PointerToRunChild pToRunChild;
    Block* parentBlock;
    int ifTrueId;
public:
    WhileOp(MyVariant* m_value, Block* m_parentBlock, PointerToRunChild m_pToRunChild, int m_IfTrueId) : Op(m_value), parentBlock(m_parentBlock) , pToRunChild(m_pToRunChild), ifTrueId(m_IfTrueId) {}

    void eval()
    {

        forever {
            left->eval();
            if (!left->value->toInt()) break;
            (parentBlock->*pToRunChild)(ifTrueId);
        }

    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        WhileOp* cloned = new WhileOp(value, parentBlock, pToRunChild, ifTrueId);
        CLONE
    }
};

class LoopOp : public Op
{
private:
    PointerToRunChild pToRunChild;
    Block* parentBlock;
    int ifTrueId;
public:
    LoopOp(MyVariant* m_value, Block* m_parentBlock, PointerToRunChild m_pToRunChild, int m_IfTrueId) : Op(m_value), parentBlock(m_parentBlock) , pToRunChild(m_pToRunChild), ifTrueId(m_IfTrueId) {}
    void eval()
    {
        left->eval();
        int count = left->value->toInt();
        std::cout << count << std::endl;
        while (count--) {
            (parentBlock->*pToRunChild)(ifTrueId);

        }

    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        LoopOp* cloned = new LoopOp(value, parentBlock, pToRunChild, ifTrueId);
        CLONE
    }

};

class InitializerList : public Op
{

public:
    InitializerList(MyVariant* m_value) : Op(m_value)

    {}

    void eval()
    {
        /*int size = arguments.size();
        /*real_type* array = new real_type[size];
        for (int i=0; i<size; ++i)
            array[i] = arguments[i]->eval().toRealType();
        QList<MyVariant*> elements;
        bool isTypesSame = true;
        for (int i=0; i<size; ++i)
        {
            arguments[i]->eval();
            elements.append(new MyVariant(arguments[i]->value));
            if (i>0)
                isTypesSame &= (elements[i]->getDataType() == elements[i-1]->getDataType());
        }

        if (!isTypesSame) {
            reportError("All the elements of array must belong to the same type");
            return;
        }
        value = MyVariant(elements);
        //return MyVariant(elements);*/
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        InitializerList* cloned = new InitializerList(value);
        cloned->parent = m_parent;
        foreach (AbstractExpr* expr, arguments)
        {
            cloned->arguments.append(expr->clone(cloned));
        }
        return cloned;
    }
};

class DisplayVariable : public Op
{

public:
    DisplayVariable(MyVariant* m_value) : Op(m_value) {}
    void eval()
    {
        if (left!=nullptr) left->eval();
        value->print();
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        DisplayVariable* cloned = new DisplayVariable(value);
        CLONE
    }
};

class UnaryMinus :public Op
{
public:
    UnaryMinus(MyVariant* m_value) : Op(m_value) {}

    void eval() {
        right->eval();
        *value = -*(right->value);
    }
    AbstractExpr* clone(AbstractExpr *m_parent) const override
    {
        UnaryMinus* cloned = new UnaryMinus(value);
        CLONE
    }
};

class AddOp : public Op
{
public:
    AddOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        left->eval();
        right->eval();
        *value = *left->value + *right->value;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        AddOp* cloned = new AddOp(value);
        CLONE
    }
};

class SubstractOp : public Op
{
public:
    SubstractOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        left->eval();
        right->eval();
        *value = *left->value - *right->value;

    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        SubstractOp* cloned = new SubstractOp(value);
        CLONE
    }
};

class MultiplyOp : public Op
{
public:
    MultiplyOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        left->eval();
        right->eval();
        *value = *left->value * *right->value;

    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        MultiplyOp* cloned = new MultiplyOp(value);
        CLONE
    }
};

class DivideOp : public Op
{
public:
    DivideOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        left->eval();
        right->eval();
        *value = *left->value / *right->value;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        DivideOp* cloned = new DivideOp(value);
        CLONE
    }
};

class PowerOp : public Op
{
public:
    PowerOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        left->eval();
        right->eval();
        *value = *left->value ^ *right->value;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        PowerOp* cloned = new PowerOp(value);
        CLONE
    }
};

class SqrtOp : public Op
{
public:
    SqrtOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        right->eval();
        *value = mySqrt(*right->value);
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        SqrtOp* cloned = new SqrtOp(value);
        CLONE
    }
};

class LogOp : public Op
{
public:
    LogOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        right->eval();
        *value = myLog(*right->value);

    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        LogOp* cloned = new LogOp(value);
        CLONE
    }
};

class AbsOp : public Op
{
public:
    AbsOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        right->eval();
        *value = myAbs(*right->value);
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        AbsOp* cloned = new AbsOp(value);
        CLONE
    }
};


class AssignOp : public Op
{

public:
    AssignOp(MyCache* m_stack, MyVariant* m_value) : Op(m_value)
    {
        stack = m_stack;
    }

    void eval() {
        right->eval();
        value = right->value;
        if (!left->assign(value)) {
            reportError("lvalue of assignment isn't correct");
            value = nullptr;
        }
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        AssignOp* cloned = new AssignOp(stack, value);
        CLONE
    }
};

class IsEqualOp : public Op
{

public:
    IsEqualOp(MyVariant* m_value) : Op(m_value) {}

    void eval() {
        left->eval();
        right->eval();
        *value = *left->value == *right->value;

    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsEqualOp* cloned = new IsEqualOp(value);
        CLONE
    }
};

class IsLessOp : public Op
{
public:
    IsLessOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        left->eval();
        right->eval();
        *value = *left->value < *right->value;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsLessOp* cloned = new IsLessOp(value);
        CLONE
    }
};

class IsLessEqualOp : public Op
{

public:
    IsLessEqualOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        left->eval();
        right->eval();
        *value = *left->value <= *right->value;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsLessEqualOp* cloned = new IsLessEqualOp(value);
        CLONE
    }
};

class IsMoreOp : public Op
{
public:
    IsMoreOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        left->eval();
        right->eval();
        *value = *left->value > *right->value;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsMoreOp* cloned = new IsMoreOp(value);
        CLONE
    }
};

class IsMoreEqualOp : public Op
{
public:
    IsMoreEqualOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        left->eval();
        right->eval();
        *value = *left->value >= *right->value;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsMoreEqualOp* cloned = new IsMoreEqualOp(value);
        CLONE
    }
};

class IsNotEqualOp : public Op
{
public:
    IsNotEqualOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        left->eval();
        right->eval();
        *value = *left->value != *right->value;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsNotEqualOp* cloned = new IsNotEqualOp(value);
        CLONE
    }
};

class BoolAndOp : public Op
{
public:
    BoolAndOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        left->eval();
        right->eval();
        *value = *left->value && *right->value;

    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BoolAndOp* cloned = new BoolAndOp(value);
        CLONE
    }
};

class BoolOrOp : public Op
{
public:
    BoolOrOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        left->eval();
        right->eval();
        *value = *left->value || *right->value;

    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BoolOrOp* cloned = new BoolOrOp(value);
        CLONE
    }
};

class BoolXorOp : public Op
{
public:
    BoolXorOp (MyVariant* m_value) : Op(m_value) {}
    void eval() {
        left->eval();
        right->eval();
        *value = doXor(*left->value, *right->value);

    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BoolXorOp* cloned = new BoolXorOp(value);
        CLONE
    }
};

class BoolNotOp : public Op
{
public:
    BoolNotOp(MyVariant* m_value) : Op(m_value) {}
    void eval() {
        right->eval();
        *value = !*right->value;

    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BoolNotOp* cloned = new BoolNotOp(value);
        CLONE
    }
};

#endif // EXPRESSIONS_H

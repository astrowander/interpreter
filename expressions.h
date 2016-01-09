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
typedef int (Block::* PointerToRun) (MyVariant*, QList<MyVariant*>*);
typedef int (Block::*PointerToRunChild) (int);
typedef void (Block::* PointerToAdd) (const QString&, DataType, int, real_type*);

class AbstractExpr
{    
public:
    AbstractExpr* parent;
    AbstractExpr* left;
    AbstractExpr* right;
    QList<AbstractExpr*> arguments;    
    MyVariant* value;
    AbstractExpr(MyVariant* m_value) : value (m_value)
    {
        parent = left = right = nullptr;
    }
    virtual ~AbstractExpr() {}
    virtual int eval() = 0;
    virtual int assign(MyVariant* rvalue) = 0;
    virtual AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const = 0;   
};

class Literal: public AbstractExpr
{

public:
    Literal(MyVariant* m_value) : AbstractExpr(m_value) {}

    int assign(MyVariant* rvalue) {
        return 1000;
    }

    int eval()
    {
        return 0;
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

    int getIndex()
    {

        if (right->value->getDataType()==INTEGER && !right->value->isArray())
        {
            int result = right->value->toInt();
            if (result>=0 && result < left->value->getSize())
                return result;
            else {
                reportError("Bad index");
                return -1;
            }
        }
        reportError("Index must be an integer number");
        return -1;
    }

    int assign(MyVariant *rvalue) {
        if (rvalue->getDataType()!=left->value->getDataType() ) {
            reportError("Array element could be assigned same type");
            return 1000;
        }

        if (!left->value->isArray()) {
            reportError("It isn't' array");
            return 1000;
        }
        int index = getIndex();
        left->value->setElement(rvalue, index);
        return 0;
    }

    int eval()
    {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        if (!left->value->isArray()) {
            reportError("It isn't' array");
        }

        int index = getIndex();

        if (index!=-1)
        {
            value = left->value->getElement(index);
            return 0;
        }
        else
            return 1000;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {

        ArrayElement* cloned = new ArrayElement(value);
        CLONE
    }
};

class Variable: public AbstractExpr
{

public:
    Variable(MyVariant* m_value) : AbstractExpr(m_value) {}

    int eval()
    {
        return 0;
    }

    int assign(MyVariant *rvalue) {
        *value = *rvalue;
        //std::cout << value->print().toStdString();
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        Variable* cloned = new Variable(value);
        CLONE
    }
};

class Op : public AbstractExpr
{
protected:
    MyCache* stack;
public:
    Op(MyVariant* m_value, MyCache* m_stack) : stack(m_stack), AbstractExpr(m_value) {}
    int assign(MyVariant* rvalue) {
        return 1000;
    }

    bool allocateArray(MyVariant* toAlloc, MyVariant* m_v1)
    {
        int mysize = toAlloc->getSize(), v1size = m_v1->getSize();
        if (mysize == v1size && mysize == -1)
            return 0;

        if (mysize<v1size) {
            if (mysize==-1) ++mysize;//Костыль. Исправить.
            int dif = v1size - mysize;
            for (int i=0; i<dif; ++i)
            {
                if (!stack->allocate( toAlloc->getElement(i + mysize) )) return 1000;
            }
            toAlloc->setSize(mysize+dif);
        }

        for (int i=0; i<v1size; ++i)
        {
            if (!allocateArray(toAlloc->getElement(i), m_v1->getElement(i))) return 1000;
        }
        return 0;
    }
};

class SizeOp: public Op
{
public:
    SizeOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}

    int eval()
    {
        if (right->eval()) return 1000;
        value->setDataType(INTEGER);
        value->setSize(-1);
        value->setIsArray(false);
        if (!right->value->isArray()) {
            value->setData(0);
            return 0;
        }
        value->setData(right->value->getSize());
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        SizeOp* cloned = new SizeOp(value, stack);
        CLONE
    }
};


class CallFunction : public Op
{

private:
    PointerToRun pToRun;
    Block* parentBlock;    

public:
    CallFunction(Block* m_parentBlock, PointerToRun m_pToRun, MyVariant* m_value, MyCache* m_stack) : parentBlock(m_parentBlock) , pToRun(m_pToRun), Op(m_value, m_stack) {}
    int eval()
    {
        QList<MyVariant*> parameters;
        foreach (AbstractExpr* expr, arguments)
        {
            if (expr->eval()) return 1000;
            parameters.append(expr->value);
        }
        int n = (parentBlock->*pToRun)(value, &parameters);
        return n ? --n : n;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        CallFunction* cloned = new CallFunction(parentBlock, pToRun, value, stack);
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
    IfOp( MyVariant* m_value, MyCache* m_stack, Block* m_parentBlock, PointerToRunChild m_pToRunChild, int m_IfTrueId, int m_IfFalseId = -1) : Op(m_value, m_stack), parentBlock(m_parentBlock) , pToRunChild(m_pToRunChild), ifTrueId(m_IfTrueId), ifFalseId(m_IfFalseId) {}

    int eval()
    {
        if(left->eval()) return 1000;
        int n;
        if (left->value->toInt())
        {
            n =  (parentBlock->*pToRunChild)(ifTrueId);
        }
        else {
            n = (parentBlock->*pToRunChild)(ifFalseId);
        }
        return n ? --n : n;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IfOp* cloned = new IfOp(value, stack, parentBlock, pToRunChild, ifTrueId, ifFalseId);
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
    WhileOp(MyVariant* m_value, MyCache* m_stack, Block* m_parentBlock, PointerToRunChild m_pToRunChild, int m_IfTrueId) : Op(m_value, m_stack), parentBlock(m_parentBlock) , pToRunChild(m_pToRunChild), ifTrueId(m_IfTrueId) {}

    int eval()
    {
        forever {
            if (left->eval()) return 1000;
            if (!left->value->toInt()) return 0;
            int n = (parentBlock->*pToRunChild)(ifTrueId);
            if (n) return --n;
        }
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        WhileOp* cloned = new WhileOp(value, stack, parentBlock, pToRunChild, ifTrueId);
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
    LoopOp(MyVariant* m_value, MyCache* m_stack, Block* m_parentBlock, PointerToRunChild m_pToRunChild, int m_IfTrueId) : Op(m_value, m_stack), parentBlock(m_parentBlock) , pToRunChild(m_pToRunChild), ifTrueId(m_IfTrueId) {}
    int eval()
    {
        if (left->eval()) return 1000;
        int count = left->value->toInt();
        std::cout << count << std::endl;
        while (count--) {
            int n = (parentBlock->*pToRunChild)(ifTrueId);
            if (n) return --n;
        }
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        LoopOp* cloned = new LoopOp(value, stack, parentBlock, pToRunChild, ifTrueId);
        CLONE
    }

};

class BreakOp: public Op
{
public:
    BreakOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}

    int eval()
    {
        if (right->eval()) return 1000;
        return right->value->toInt() * 2;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BreakOp* cloned = new BreakOp(value, stack);
        CLONE
    }
};

class InitializerList : public Op
{

public:
    InitializerList(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack)

    {}

    int eval()
    {
        value->setIsArray(true);
        bool isTypesSame = true;
        for (int i=0; i<arguments.size(); ++i)
        {
            if (arguments[i]->eval()) return 1000;
            value->addElement(arguments[i]->value);
            if (i>0)
                isTypesSame &= (arguments[i]->value->getDataType() == arguments[i-1]->value->getDataType());
        }

        if (!isTypesSame) {
            reportError("All the elements of array must belong to the same type");
            value->reset();
            return 1000;
        }
        if (arguments.size())
            value->setDataType(arguments[0]->value->getDataType());
        else
            value->setDataType(INTEGER);

        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        InitializerList* cloned = new InitializerList(value, stack);
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
    DisplayVariable(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval()
    {
        if (left!=nullptr)
            if (left->eval())
                return 1000;
        value->print();
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        DisplayVariable* cloned = new DisplayVariable(value, stack);
        CLONE
    }
};

class UnaryMinus :public Op
{
public:
    UnaryMinus(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}

    int eval() {
        if (right->eval()) return 1000;
        if (right->value->isArray())
            if (!allocateArray(value, right->value)) return 1000;
        unaryMinus(value, right->value);
        return 0;
    }
    AbstractExpr* clone(AbstractExpr *m_parent) const override
    {
        UnaryMinus* cloned = new UnaryMinus(value, stack);
        CLONE
    }
};

class AddOp : public Op
{
public:
    AddOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        //fitValueSize(std::min(left->value->getSize(), right->value->getSize()));
        if (left->value->isArray())
            if (!allocateArray(value, left->value)) return 1000;
        if (!add(value, left->value, right->value)) return 1000;
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        AddOp* cloned = new AddOp(value, stack);
        CLONE
    }
};

class SubstractOp : public Op
{
public:
    SubstractOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        if (left->value->isArray())
            if (!allocateArray(value, left->value)) return 1000;
        if (!substract(value, left->value, right->value)) return 1000;
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        SubstractOp* cloned = new SubstractOp(value, stack);
        CLONE
    }
};

class MultiplyOp : public Op
{
public:
    MultiplyOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (left->eval()) return 1000;
        int rightres = right->eval();
        if (rightres) return 1000;
        if (left->value->isArray())
            if (!allocateArray(value, left->value)) return 1000;
        if (right->value->isArray())
            if (!allocateArray(value, right->value)) return 1000;
        if (!multiply(value, left->value, right->value)) return 1000;
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        MultiplyOp* cloned = new MultiplyOp(value, stack);
        CLONE
    }
};

class DivideOp : public Op
{
public:
    DivideOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        if (left->value->isArray())
            if (!allocateArray(value, left->value)) return 1000;
        if (!divide(value, left->value, right->value)) return 1000;
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        DivideOp* cloned = new DivideOp(value, stack);
        CLONE
    }
};

class PowerOp : public Op
{
public:
    PowerOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        if (!power(value, left->value, right->value)) return 1000;
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        PowerOp* cloned = new PowerOp(value, stack);
        CLONE
    }
};

class SqrtOp : public Op
{
public:
    SqrtOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (right->eval()) return 1000;
        if (right->value->isArray())
            if (!allocateArray(value, right->value)) return 1000;
        if (!mySqrt(value,right->value)) return 1000;
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        SqrtOp* cloned = new SqrtOp(value, stack);
        CLONE
    }
};

class LogOp : public Op
{
public:
    LogOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (right->eval()) return 1000;
        if (right->value->isArray())
            if (!allocateArray(value, right->value)) return 1000;
        if (!myLog(value,right->value)) return 1000;
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        LogOp* cloned = new LogOp(value, stack);
        CLONE
    }
};

class AbsOp : public Op
{
public:
    AbsOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (right->eval()) return 1000;
        if (right->value->isArray())
            if (!allocateArray(value, right->value)) return 1000;
        if (!myAbs(value,right->value)) return 1000;
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        AbsOp* cloned = new AbsOp(value, stack);
        CLONE
    }
};


class AssignOp : public Op
{

public:
    AssignOp(MyCache* m_stack, MyVariant* m_value) : Op(m_value, m_stack)
    {
        stack = m_stack;
    }

    int eval() {
        if (right->eval()) return 1000;
        value = right->value;
        if (left->assign(value)) {
            reportError("lvalue of assignment isn't correct");
            value->reset();
            return 1000;
        }
        return 0;
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
    IsEqualOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}

    int eval() {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        if (left->value->isArray())
            if (!allocateArray(value, left->value)) return 1000;
        if (!isEqual(value, left->value, right->value)) return 1000;
        return 0;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsEqualOp* cloned = new IsEqualOp(value, stack);
        CLONE
    }
};

class IsLessOp : public Op
{
public:
    IsLessOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        if (!isLess(value, left->value, right->value)) return 1000;
        return 0;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsLessOp* cloned = new IsLessOp(value, stack);
        CLONE
    }
};

class IsLessEqualOp : public Op
{

public:
    IsLessEqualOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        if (!isLessEqual(value, left->value, right->value)) return 1000;
        return 0;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsLessEqualOp* cloned = new IsLessEqualOp(value, stack);
        CLONE
    }
};

class IsMoreOp : public Op
{
public:
    IsMoreOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        if (!isMore(value, left->value, right->value)) return 1000;
        return 0;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsMoreOp* cloned = new IsMoreOp(value, stack);
        CLONE
    }
};

class IsMoreEqualOp : public Op
{
public:
    IsMoreEqualOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        if (!isMoreEqual(value, left->value, right->value)) return 1000;
        return 0;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsMoreEqualOp* cloned = new IsMoreEqualOp(value, stack);
        CLONE
    }
};

class IsNotEqualOp : public Op
{
public:
    IsNotEqualOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        if (!isNotEqual(value, left->value, right->value)) return 1000;
        return 0;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsNotEqualOp* cloned = new IsNotEqualOp(value, stack);
        CLONE
    }
};

class BoolAndOp : public Op
{
public:
    BoolAndOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        if (!doAnd(value, left->value, right->value)) return 1000;
        return 0;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BoolAndOp* cloned = new BoolAndOp(value, stack);
        CLONE
    }
};

class BoolOrOp : public Op
{
public:
    BoolOrOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        if (!doOr(value, left->value, right->value)) return 1000;
        return 0;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BoolOrOp* cloned = new BoolOrOp(value, stack);
        CLONE
    }
};

class BoolXorOp : public Op
{
public:
    BoolXorOp (MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (left->eval()) return 1000;
        if (right->eval()) return 1000;
        if (!doXor(value, left->value, right->value)) return 1000;
        return 0;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BoolXorOp* cloned = new BoolXorOp(value, stack);
        CLONE
    }
};

class BoolNotOp : public Op
{
public:
    BoolNotOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}
    int eval() {
        if (right->eval()) return 1000;
        if (!doNot(value, right->value)) return 1000;
        return 0;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BoolNotOp* cloned = new BoolNotOp(value, stack);
        CLONE
    }
};

#endif // EXPRESSIONS_H

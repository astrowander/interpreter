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
    MyVariant* value;
    AbstractExpr(MyVariant* m_value) : value (m_value)
    {
        parent = left = right = nullptr;
    }
    virtual ~AbstractExpr() {}
    inline virtual bool eval() = 0;
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

    bool eval()
    {
        return true;
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

    bool assign(MyVariant *rvalue) {
        if (rvalue->getDataType()!=left->value->getDataType() ) {
            reportError("Array element could be assigned same type");
            return false;
        }

        if (!left->value->isArray()) {
            reportError("It isn't' array");
            return false;
        }
        int index = getIndex();
        left->value->setElement(rvalue, index);
        return true;
    }

    bool eval()
    {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (!left->value->isArray()) {
            reportError("It isn't' array");
        }

        int index = getIndex();

        if (index!=-1)
        {
            value = left->value->getElement(index);
            return true;
        }
        else
            return false;
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

    bool eval()
    {
        return true;
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
protected:
    MyCache* stack;
public:
    Op(MyVariant* m_value, MyCache* m_stack) : stack(m_stack), AbstractExpr(m_value) {}
    bool assign(MyVariant* rvalue) {
        return false;
    }

    bool allocateArray(MyVariant* toAlloc, MyVariant* m_v1)
    {
        int mysize = toAlloc->getSize(), v1size = m_v1->getSize();
        if (mysize == v1size && mysize == -1)
            return true;

        if (mysize<v1size) {
            if (mysize==-1) ++mysize;//Костыль. Исправить.
            int dif = v1size - mysize;
            for (int i=0; i<dif; ++i)
            {
                if (!stack->allocate( toAlloc->getElement(i + mysize) )) return false;
            }
            toAlloc->setSize(mysize+dif);
        }

        for (int i=0; i<v1size; ++i)
        {
            if (!allocateArray(toAlloc->getElement(i), m_v1->getElement(i))) return false;
        }
        return true;
    }
};

class SizeOp: public Op
{
public:
    SizeOp(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack) {}

    bool eval()
    {
        if (!right->eval()) return false;
        value->setDataType(INTEGER);
        value->setSize(-1);
        value->setIsArray(false);
        if (!right->value->isArray()) {
            value->setData(0);
            return true;
        }
        value->setData(right->value->getSize());
        return true;
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
    bool eval()
    {
        QList<MyVariant*> parameters;
        foreach (AbstractExpr* expr, arguments)
        {
            if (!expr->eval()) return false;
            parameters.append(expr->value);
        }

        (parentBlock->*pToRun)(value, &parameters);
        return true;
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

    bool eval()
    {
        if(!left->eval()) return false;
        if (left->value->toInt())
        {
            (parentBlock->*pToRunChild)(ifTrueId);
        }
        else {
            (parentBlock->*pToRunChild)(ifFalseId);
        }
        return true;
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

    bool eval()
    {
        forever {
            if (!left->eval()) return false;
            if (!left->value->toInt()) return true;
            (parentBlock->*pToRunChild)(ifTrueId);
        }
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
    bool eval()
    {
        if (!left->eval()) return false;
        int count = left->value->toInt();
        std::cout << count << std::endl;
        while (count--) {
            (parentBlock->*pToRunChild)(ifTrueId);
        }
        return true;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        LoopOp* cloned = new LoopOp(value, stack, parentBlock, pToRunChild, ifTrueId);
        CLONE
    }

};

class InitializerList : public Op
{

public:
    InitializerList(MyVariant* m_value, MyCache* m_stack) : Op(m_value, m_stack)

    {}

    bool eval()
    {
        value->setIsArray(true);
        bool isTypesSame = true;
        for (int i=0; i<arguments.size(); ++i)
        {
            if (!arguments[i]->eval()) return false;
            value->addElement(arguments[i]->value);
            if (i>0)
                isTypesSame &= (arguments[i]->value->getDataType() == arguments[i-1]->value->getDataType());
        }

        if (!isTypesSame) {
            reportError("All the elements of array must belong to the same type");
            value->reset();
            return false;
        }
        if (arguments.size())
            value->setDataType(arguments[0]->value->getDataType());
        else
            value->setDataType(INTEGER);

        return true;
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
    bool eval()
    {
        if (left!=nullptr)
            if (!left->eval())
                return false;
        value->print();
        return true;
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

    bool eval() {
        if (!right->eval()) return false;
        if (right->value->isArray())
            if (!allocateArray(value, right->value)) return false;
        unaryMinus(value, right->value);
        return true;
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
    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        //fitValueSize(std::min(left->value->getSize(), right->value->getSize()));
        if (left->value->isArray())
            if (!allocateArray(value, left->value)) return false;
        if (!add(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (left->value->isArray())
            if (!allocateArray(value, left->value)) return false;
        if (!substract(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (left->value->isArray())
            if (!allocateArray(value, left->value)) return false;
        if (right->value->isArray())
            if (!allocateArray(value, right->value)) return false;
        if (!multiply(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (left->value->isArray())
            if (!allocateArray(value, left->value)) return false;
        if (!divide(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (!power(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!right->eval()) return false;
        if (right->value->isArray())
            if (!allocateArray(value, right->value)) return false;
        if (!mySqrt(value,right->value)) return false;
        return true;
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
    bool eval() {
        if (!right->eval()) return false;
        if (right->value->isArray())
            if (!allocateArray(value, right->value)) return false;
        if (!myLog(value,right->value)) return false;
        return true;
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
    bool eval() {
        if (!right->eval()) return false;
        if (right->value->isArray())
            if (!allocateArray(value, right->value)) return false;
        if (!myAbs(value,right->value)) return false;
        return true;
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

    bool eval() {
        if (!right->eval()) return false;
        value = right->value;
        if (!left->assign(value)) {
            reportError("lvalue of assignment isn't correct");
            value->reset();
            return false;
        }
        return true;
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

    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (left->value->isArray())
            if (!allocateArray(value, left->value)) return false;
        if (!isEqual(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (!isLess(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (!isLessEqual(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (!isMore(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (!isMoreEqual(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (!isNotEqual(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (!doAnd(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (!doOr(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!left->eval()) return false;
        if (!right->eval()) return false;
        if (!doXor(value, left->value, right->value)) return false;
        return true;
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
    bool eval() {
        if (!right->eval()) return false;
        if (!doNot(value, right->value)) return false;
        return true;
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BoolNotOp* cloned = new BoolNotOp(value, stack);
        CLONE
    }
};

#endif // EXPRESSIONS_H

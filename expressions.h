#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include "myvariant.h"

class Block;
typedef void (Block::* PointerToRun) (MyVariant&, const QList<MyVariant>&);
typedef void (Block::* PointerToAdd) (const QString&, DataType, int, real_type*);
typedef void (Block::* PointerToDel) (const QString&);


class AbstractExpr
{    
public:
    AbstractExpr* parent;
    AbstractExpr* left;
    AbstractExpr* right;
    QList<AbstractExpr*> arguments;
    AbstractExpr()
    {
        parent = left = right = nullptr;
    }
    virtual ~AbstractExpr() {}
    virtual MyVariant eval() = 0;
    virtual bool assign(const MyVariant& rvalue) = 0;
    virtual AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const = 0;
};

class TerminalExpr: public AbstractExpr {};

class NonTerminalExpr: public AbstractExpr {};

class Literal: public TerminalExpr
{
private:
    const MyVariant value;
public:
    Literal(const MyVariant& m_value) : value(m_value) {}

    bool assign(const MyVariant& rvalue) {
        return false;
    }

    MyVariant eval()
    {
        return value;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        Literal* cloned = new Literal(value);
        cloned->parent = m_parent;
        return cloned;
    }
};

class ArrayElement: public TerminalExpr
{
private:
    MyVariant& value;
    int index;

public:
    ArrayElement(MyVariant& m_value, int m_index) : value(m_value), index(m_index) {}

    bool assign(const MyVariant &rvalue) {
        if (rvalue.getDataType()!=REAL) {
            reportError("Array element could be assigned only number");
            return false;
        }

        if (value.getDataType()!=REALARRAY) {
            reportError("It isn't' array");
            return false;
        }

        if (index>=value.getSize()) {
            reportError("Out of bounds");
            return false;
        }
        *(value.atPtr(index)) = rvalue.toRealType();
    }

    MyVariant eval()
    {
        if (value.getDataType()!=REALARRAY) {
            reportError("It isn't' array");
            return MyVariant(VOID);
        }
        if (index>=value.getSize()) {

            reportError("Out of bounds");
            return MyVariant(VOID);
        }
        return MyVariant(REAL,value.atPtr(index));
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        ArrayElement* cloned = new ArrayElement(value, index);
        cloned->parent = m_parent;
        return cloned;
    }

};

class Variable: public TerminalExpr
{
private:
    MyVariant& value;
public:
    Variable(MyVariant& m_value) : value(m_value) {}    

    MyVariant eval()
    {
        return value;
    }

    bool assign(const MyVariant &rvalue) {
        value = rvalue;
        //std::cout << value.print().toStdString();
        return true;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        Variable* cloned = new Variable(value);
        cloned->parent = m_parent;
        return cloned;
    }
};

class Op : public NonTerminalExpr
{
public:
    bool assign(const MyVariant& rvalue) {
        return false;
    }
};

class CallFunction : public Op
{

private:
    PointerToRun pToRun;
    Block* parentBlock;

public:
    CallFunction(Block* m_parentBlock, PointerToRun m_pToRun) : parentBlock(m_parentBlock) , pToRun(m_pToRun) {}
    MyVariant eval()
    {
        QList<MyVariant> parameters;        
        foreach (AbstractExpr* expr, arguments)
        {
            parameters.append(expr->eval());
        }

        MyVariant result;       
        (parentBlock->*pToRun)(result, parameters);
        return result;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        CallFunction* cloned = new CallFunction(parentBlock, pToRun);
        cloned->parent = m_parent;
        foreach (AbstractExpr* expr, arguments)
        {
            cloned->arguments.append(expr->clone(cloned));
        }
        return cloned;
    }
};

class InitializerList : public Op
{
private:
    QString name;
public:
    InitializerList(const QString& m_name = "")
        : name(m_name)
    {}

    MyVariant eval()
    {
        int size = arguments.size();
        real_type* array = new real_type[size];
        for (int i=0; i<size; ++i)
            array[i] = arguments[i]->eval().toRealType();
        return MyVariant(REALARRAY,array,size,name);
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        InitializerList* cloned = new InitializerList(name);
        cloned->parent = m_parent;
        foreach (AbstractExpr* expr, arguments)
        {
            cloned->arguments.append(expr->clone(cloned));
        }
        return cloned;
    }
};

class DeleteVariable : public Op
{
private:
    PointerToDel pToDel;
    Block* parentBlock;
    MyVariant& value;
public:
    DeleteVariable(MyVariant& m_value, Block* m_parentBlock, PointerToDel m_pToDel) : value(m_value), parentBlock(m_parentBlock), pToDel(m_pToDel) {}
    MyVariant eval()
    {
        if (right!=nullptr) right->eval();
        (parentBlock->*pToDel)(value.getName());
        return MyVariant(VOID);
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        DeleteVariable* cloned = new DeleteVariable(value, parentBlock, pToDel);
        cloned->parent = m_parent;
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class DisplayVariable : public Op
{
private:
    MyVariant& value;
public:
    DisplayVariable(MyVariant& m_value) : value(m_value) {}
    MyVariant eval()
    {
        if (left!=nullptr) left->eval();
        value.print();
        return MyVariant(VOID);
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        DisplayVariable* cloned = new DisplayVariable(value);
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        return cloned;
    }
};

class AddOp : public Op
{
public:
    MyVariant eval() {
        return left->eval()+right->eval();
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        AddOp* cloned = new AddOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class SubstractOp : public Op
{
    MyVariant eval() {
        return left->eval()-right->eval();
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        SubstractOp* cloned = new SubstractOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class MultiplyOp : public Op
{
    MyVariant eval() {
        return left->eval() * right->eval();
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        MultiplyOp* cloned = new MultiplyOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class DivideOp : public Op
{
    MyVariant eval() {
        return left->eval() / right->eval();
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        DivideOp* cloned = new DivideOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class PowerOp : public Op
{
    MyVariant eval() {
        return left->eval() ^ right->eval();
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        PowerOp* cloned = new PowerOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class SqrtOp : public Op
{
    MyVariant eval() {
        return mySqrt(right->eval());
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        SqrtOp* cloned = new SqrtOp();
        cloned->parent = m_parent;
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class LogOp : public Op
{
    MyVariant eval() {
        return myLog(right->eval());
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        LogOp* cloned = new LogOp();
        cloned->parent = m_parent;
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class AbsOp : public Op
{
    MyVariant eval() {
        return myFabs(right->eval());
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        AbsOp* cloned = new AbsOp();
        cloned->parent = m_parent;
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};


class AssignOp : public Op
{
    MyVariant eval() {
        MyVariant result = right->eval();
        left->assign(result);        
        return result;
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        AssignOp* cloned = new AssignOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

#endif // EXPRESSIONS_H

#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include "myvariant.h"

class Block;
typedef void (Block::* PointerToRun) (MyVariant&, const QList<MyVariant>&);
typedef void (Block::*PointerToRunChild) (int);
typedef void (Block::* PointerToAdd) (const QString&, DataType, int, real_type*);

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

public:
    ArrayElement(MyVariant& m_value) : value(m_value) {}

    int getIndex()
    {
        MyVariant temp( right->eval()) ;
        if (temp.getDataType()==INTEGER && !temp.isArray())
        {
            int result = temp.toInt();
            if (result>=0 && result < value.getSize())
                return result;
            else {
                reportError("Bad index");
                return -1;
            }
        }
        reportError("Index must be an integer number");
        return -1;
    }

    bool assign(const MyVariant &rvalue) {
        if (rvalue.getDataType()!=value.getDataType() ) {
            reportError("Array element could be assigned same type");
            return false;
        }

        if (!value.isArray()) {
            reportError("It isn't' array");
            return false;
        }
        int index = getIndex();
        value.setElement(rvalue, index);
    }

    MyVariant eval()
    {
        if (!value.isArray()) {
            reportError("It isn't' array");
            return MyVariant(VOID);
        }
        int index = getIndex();

        if (index!=-1)
            return MyVariant(value.getElement(index));
        else
            return MyVariant(VOID);
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        ArrayElement* cloned = new ArrayElement(value);
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
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

class IfOp : public Op
{
private:
    PointerToRunChild pToRunChild;
    Block* parentBlock;
    int ifTrueId, ifFalseId;
public:
    IfOp(Block* m_parentBlock, PointerToRunChild m_pToRunChild, int m_IfTrueId, int m_IfFalseId = -1) : parentBlock(m_parentBlock) , pToRunChild(m_pToRunChild), ifTrueId(m_IfTrueId), ifFalseId(m_IfFalseId) {}

    MyVariant eval()
    {
        if (left->eval().toInt())
        {
            (parentBlock->*pToRunChild)(ifTrueId);
        }
        else {
            (parentBlock->*pToRunChild)(ifFalseId);
        }
        return MyVariant();
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IfOp* cloned = new IfOp(parentBlock, pToRunChild, ifTrueId, ifFalseId);
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class WhileOp: public Op
{
private:
    PointerToRunChild pToRunChild;
    Block* parentBlock;
    int ifTrueId;
public:
    WhileOp(Block* m_parentBlock, PointerToRunChild m_pToRunChild, int m_IfTrueId) : parentBlock(m_parentBlock) , pToRunChild(m_pToRunChild), ifTrueId(m_IfTrueId) {}

    MyVariant eval()
    {
        while (left->eval().toInt()) {
            (parentBlock->*pToRunChild)(ifTrueId);
        }
        return MyVariant();
    }

    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        WhileOp* cloned = new WhileOp(parentBlock, pToRunChild, ifTrueId);
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
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
        /*real_type* array = new real_type[size];
        for (int i=0; i<size; ++i)
            array[i] = arguments[i]->eval().toRealType();*/
        QVector<MyVariant*> elements;
        bool isTypesSame = true;
        for (int i=0; i<size; ++i)
        {
            elements.append(new MyVariant(arguments[i]->eval()));
            if (i>0)
                isTypesSame &= (elements[i]->getDataType() == elements[i-1]->getDataType());
        }

        if (!isTypesSame) {
            reportError("All the elements of array must belong to the same type");
            return MyVariant(VOID);
        }

        return MyVariant(elements, name);
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

class UnaryMinus :public Op
{
public:
    MyVariant eval() {
        return -right->eval();
    }
    AbstractExpr* clone(AbstractExpr *m_parent) const override
    {
        UnaryMinus* cloned = new UnaryMinus();
        cloned->parent = m_parent;
        if (right!=nullptr) cloned->right = right->clone(cloned);
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
        return myAbs(right->eval());
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
        if (!left->assign(result)) {
            reportError("lvalue of assignment isn't correct");
            return MyVariant(VOID);
        }
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

class IsEqualOp : public Op
{
    MyVariant eval() {
        return left->eval() == right->eval();
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsEqualOp* cloned = new IsEqualOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class IsLessOp : public Op
{
    MyVariant eval() {
        return left->eval() < right->eval();
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsLessOp* cloned = new IsLessOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class IsLessEqualOp : public Op
{
    MyVariant eval() {
        return left->eval() <= right->eval();
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsLessEqualOp* cloned = new IsLessEqualOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class IsMoreOp : public Op
{
    MyVariant eval() {
        return left->eval() > right->eval();
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsMoreOp* cloned = new IsMoreOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class IsMoreEqualOp : public Op
{
    MyVariant eval() {
        return left->eval() >= right->eval();
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsMoreEqualOp* cloned = new IsMoreEqualOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class IsNotEqualOp : public Op
{
    MyVariant eval() {
        return left->eval() != right->eval();
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        IsEqualOp* cloned = new IsEqualOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class BoolAndOp : public Op
{
    MyVariant eval() {
        return left->eval() && right->eval();
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BoolAndOp* cloned = new BoolAndOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class BoolOrOp : public Op
{
    MyVariant eval() {
        return left->eval() || right->eval();
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BoolOrOp* cloned = new BoolOrOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class BoolXorOp : public Op
{
    MyVariant eval() {
        return doXor(left->eval(), right->eval());
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BoolXorOp* cloned = new BoolXorOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};

class BoolNotOp : public Op
{
    MyVariant eval() {
        return !right->eval();
    }
    AbstractExpr* clone(AbstractExpr* m_parent = nullptr) const override
    {
        BoolXorOp* cloned = new BoolXorOp();
        cloned->parent = m_parent;
        if (left!=nullptr) cloned->left = left->clone(cloned);
        if (right!=nullptr) cloned->right = right->clone(cloned);
        return cloned;
    }
};



#endif // EXPRESSIONS_H

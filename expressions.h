#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include "myvariant.h"

class AbstractExpr
{
public:
    AbstractExpr* parent;
    AbstractExpr* left;
    AbstractExpr* right;
    AbstractExpr(AbstractExpr* m_parent = nullptr) {}
    virtual ~AbstractExpr() {}
    virtual MyVariant eval() = 0;
    virtual bool assign(const MyVariant& rvalue) = 0;
};

class TerminalExpr: public AbstractExpr
{
public:
    //TerminalExpr(AbstractExpr* m_parent = nullptr) : AbstractExpr(m_parent) {}
    //~TerminalExpr();
};
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
        std::cout << value.getName().toStdString() << " = " << rvalue.toRealType() << std::endl;
        return true;
    }
};

class Op : public NonTerminalExpr
{
public:
    bool assign(const MyVariant& rvalue) {
        return false;
    }
};

class AddOp : public Op
{
public:
    MyVariant eval() {
        return left->eval()+right->eval();
    }
};

class SubstractOp : public Op
{
    MyVariant eval() {
        return left->eval()-right->eval();
    }
};

class MultiplyOp : public Op
{
    MyVariant eval() {
        return left->eval() * right->eval();
    }
};

class DivideOp : public Op
{
    MyVariant eval() {
        return left->eval() / right->eval();
    }
};

class PowerOp : public Op
{
    MyVariant eval() {
        return left->eval() ^ right->eval();
    }
};

class AssignOp : public Op
{
    MyVariant eval() {
        MyVariant result = right->eval();
        left->assign(result);        
        return result;
    }
};

#endif // EXPRESSIONS_H

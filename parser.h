#ifndef PARSER_H
#define PARSER_H
#include "block.h"
#include "stringprocessor.h"

class Interpreter;
typedef int(Interpreter::*PointerToGetStringList) (QStringList&);
typedef void(Interpreter::*PointerToAddBuffer) (const QString&);



class Parser
{

private:
    Block* activeBlock;
    Statement* currentStatement;
    StringProcessor sp;
    Interpreter* interpreter;
    PointerToGetStringList pToGet;
    PointerToAddBuffer pToAddBuf;
    bool outputResult;

    const QVector<QString> keywords = {"display", "function", "if", "loop", "var", "while", "end"};
    const QVector<QString> presetFunctions = {"abs", "log", "sqrt"};
    MyCache *stack;

    bool add();
    bool substract();
    bool multiply();
    bool divide();
    bool power();

    bool factor();
    bool signedFactor();
    bool ident();
    //bool doHighPriorityOperations();
    bool term();

    bool assign();
    bool powerFactor();
    bool expression();
    bool relation();
    bool boolExpression();
    bool boolTerm();
    bool notFactor();
public:
    Parser (Block *m_activeBlock, Interpreter* m_interpreter, PointerToGetStringList m_pToGet, PointerToAddBuffer m_pToAddBuf, MyCache *m_stack) : activeBlock(m_activeBlock), interpreter(m_interpreter), pToGet(m_pToGet) , pToAddBuf(m_pToAddBuf), stack(m_stack) {}
    Block* getActiveBlock()
    {
        return activeBlock;
    }

    bool parse(QStringList list);
    bool doOutput() const;

    bool block();
};

#endif // PARSER_H

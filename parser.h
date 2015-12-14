#ifndef PARSER_H
#define PARSER_H
#include "block.h"
#include "stringprocessor.h"

class Parser
{

private:
    Block* activeBlock;
    Statement* currentStatement;
    StringProcessor sp;
    bool outputResult;

    const QVector<QString> keywords = {"delete", "display", "function", "end"};

    bool add();
    bool substract();
    bool multiply();
    bool divide();
    bool power();

    bool factor();
    bool ident();
    bool doHighPriorityOperations();
    bool term();

    bool assign();
public:
    Parser (Block *m_activeBlock) : activeBlock(m_activeBlock) {}
    Block* getActiveBlock()
    {
        return activeBlock;
    }

    bool parse(QStringList list, Statement* m_currentStatement);
    bool doOutput() const;
};

#endif // PARSER_H

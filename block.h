#ifndef BLOCK_H
#define BLOCK_H

#include <QMap>
#include <QString>
#include <QVector>
#include "statement.h"

class Block
{
private:
    static int number;
    int m_id;
    QVector<Block*> blocks;
    QVector<Statement*> statements;
    Statement* activeStatement;
    QMap <QString, int> functionsMap;
    QMap <QString, MyVariant> variablesMap;
public:
    Block()
    {
        m_id = number++;
        variablesMap.insert(QString("result"), MyVariant());
    }

    bool addStatement(Statement* m_statement)
    {
        Statement* statement = new Statement;
        *statement=*m_statement;
        delete m_statement;
        statements.append(statement);
    }
};


#endif // BLOCK_H

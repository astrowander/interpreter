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
        variablesMap.insert("result", MyVariant(NUMBER,new real_type(0),0,"result"));
    }

    bool addStatement(Statement* m_statement)
    {
        Statement* statement = new Statement;
        *statement=*m_statement;

        statements.append(statement);
    }

    bool isVariableDeclared(const QString ss)
    {
        return variablesMap.contains(ss);
    }

    MyVariant& getVariableByValue(const QString& ss)
    {
       return variablesMap[ss];
    }

    void addVariable(const QString& ss, DataType dataType = NUMBER, int m_size = 0, real_type* data = new real_type(0))
    {
        variablesMap.insert(ss, MyVariant(dataType, data, m_size, ss));
    }

    void setVariable(const QString& name, const MyVariant& result) {
        variablesMap[name]=result;
        std::cout << name.toStdString() << " = " << result.toRealType() << std::endl;
    }

};


#endif // BLOCK_H

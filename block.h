#ifndef BLOCK_H
#define BLOCK_H

#include "statement.h"



class Block
{

private:
    static int number;
    int m_id;
    Block* parent;
    QVector<Statement*> statements;
    Statement* activeStatement;

    QList<QString> incomingParameters;
    QMap <int, Block*> childBlocksMap;
    QMap <QString, Block*> functionsMap;
    QMap <QString, MyVariant> variablesMap;

public:
    Block() : parent(nullptr)
    {
        init();
    }

    Block(Block* m_parent) : parent (m_parent)
    {
        m_id = number++;
    }

    Block(Block* m_parent, const QList<QString>& parameters)  : parent(m_parent)
    {
        init();
        foreach (QString name, parameters) {
            addVariable(name);
            addIncomingParameter(name);
        }
    }

    void init()
    {
        m_id = number++;
        variablesMap.insert("result", MyVariant(new int(0), -1, "result"));
    }

    bool addStatement(Statement* m_statement)
    {
        Statement* statement = new Statement;
        *statement=*m_statement;

        statements.append(statement);        
    }

    bool isFunctionDeclared(const QString& ss)
    {
        return functionsMap.contains(ss);
    }

    bool isVariableDeclared(const QString ss)
    {
        return variablesMap.contains(ss);
    }

    Block* getFunctionByName(const QString& name)
    {
        if (!functionsMap.contains(name))
        {
            reportError("Function " + name + " is not declared in this scope");
            return nullptr;
        }
        return functionsMap[name];
    }

    Block* getParent()
    {
        return parent;
    }

    Block* getChildBlockByNumber(int n)
    {
        if (!childBlocksMap.contains(n)) {
            reportError("Bad pointer");
            return nullptr;
        }
        return childBlocksMap[n];
    }

    MyVariant& getVariableByValue(const QString& ss)
    {
       return variablesMap[ss];
    }

    void addFunction(const QString& name, const QList<QString>& parameters)
    {
        functionsMap.insert(name, new Block(this,parameters));
    }

    int addChildBlock()
    {
        childBlocksMap.insert(number-1, new Block(this));
        return number - 1;
    }

    void deleteChildBlock(int number)
    {
        if (!childBlocksMap.contains(number))
        {
            reportError("Bad pointer");
            return;
        }
        delete childBlocksMap[number];
        childBlocksMap.remove(number);
    }

    void deleteFunction(const QString& name)
    {
        if (!functionsMap.contains(name))
        {
            reportError("Function" + name + "is not declared");
            return;
        }
        delete functionsMap[name];
        functionsMap.remove(name);
    }

    void addVariable(const QString& ss, DataType dataType = REAL, int m_size = -1, real_type* data = new real_type(0))
    {
        variablesMap.insert(ss, MyVariant(data, m_size, ss));
    }

    void addIncomingParameter(const QString& ss)
    {
        incomingParameters.append(ss);
    }

    void setVariable(const QString& name, const MyVariant& result) {
        variablesMap[name]=result;
        std::cout << name.toStdString() << " = " << result.toRealType() << std::endl;
    }

    void deleteVariable(const QString& ss) {
        variablesMap.remove(ss);
    }

    void runChildBlock(int id)
    {
        if (id==-1) return;

        if (!childBlocksMap.contains(id)) {
            reportError("Bad pointer");
            return;
        }
        MyVariant temp;
        childBlocksMap[id]->run(temp);
    }

    void run(MyVariant& result, const QList<MyVariant> &parameters = QList<MyVariant>())
    {
        if (parameters.size()!=incomingParameters.size())
        {
            reportError("Invalid number of parameters");
            result = MyVariant();
            return;
        }

        for(int i=0; i<parameters.size(); ++i)
        {
            variablesMap[incomingParameters[i]] = parameters[i];
        }

        foreach (Statement* statement, statements)
        {
            statement->eval();
        }

        result = variablesMap["result"];
    }

    void runLast(MyVariant& result)
    {
        if (!statements.isEmpty())
            result = statements.last()->eval();
    }

    friend int getLevel(Block* block);

    int getId()
    {
        return m_id;
    }

    int howManyStatements()
    {
        return statements.size();
    }
};

#endif // BLOCK_H

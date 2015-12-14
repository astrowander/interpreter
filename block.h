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
    QMap <QString, Block*> functionsMap;
    QMap <QString, MyVariant> variablesMap;

public:
    Block(Block* m_parent = nullptr) : parent(m_parent)
    {
        m_id = number++;
        variablesMap.insert("result", MyVariant(NUMBER,new real_type(0),0,"result"));
    }

    Block(Block* m_parent, const QList<QString>& parameters = QList<QString>())  : parent(m_parent)
    {
        foreach (QString name, parameters) {
            addVariable(name);
            addIncomingParameter(name);
        }
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

    MyVariant& getVariableByValue(const QString& ss)
    {
       return variablesMap[ss];
    }

    void addFunction(const QString& name, const QList<QString>& parameters)
    {
        functionsMap.insert(name, new Block(this,parameters));
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

    void addVariable(const QString& ss, DataType dataType = NUMBER, int m_size = 0, real_type* data = new real_type(0))
    {
        variablesMap.insert(ss, MyVariant(dataType, data, m_size, ss));        
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

    friend int getLevel(Block* block);
};
/*
class Function :public Block
{
    Function(Block* m_parent = nullptr, const QList<QString>& parameters = QList<QString>())  :Block(m_parent)
    {
        foreach (QString name, parameters) {
            addVariable(name);
        }
    }
};
*/

#endif // BLOCK_H

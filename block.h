#ifndef BLOCK_H
#define BLOCK_H

#include "statement.h"



class Block
{

private:
    static int number;
    int m_id;
    Block* parent;
    //QVector<Statement*> statements;
    Statement* statements[1024];
    int nStatements;
    Statement* activeStatement;

    QList<QString> incomingParameters;
    //QMap <int, Block*> childBlocksMap;
    Block* childBlocksMap[1024];    
    QMap <QString, Block*> functionsMap;
    QMap <QString, MyVariant*> variablesMap;
    MyVariant* resultPtr;

    MyCache* stack;

public:
    Block(MyCache* m_stack = nullptr) : parent(nullptr), stack(m_stack)
    {
        init();
    }

    Block(Block* m_parent, MyCache* m_stack = nullptr) : parent (m_parent), stack(m_stack)
    {
        m_id = number++;
        nStatements=0;
        resultPtr = nullptr;
    }

    Block(Block* m_parent, const QList<QString>& parameters, MyCache* m_stack = nullptr)  : parent(m_parent), stack(m_stack)
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
        MyVariant* temp;
        stack->allocate(temp);
        *temp = MyVariant(int(0));
        variablesMap.insert("result", temp);
        resultPtr = variablesMap["result"];
        nStatements=0;
    }

    bool addStatement(Statement* m_statement)
    {
        Statement* statement = new Statement;
        *statement=*m_statement;

        //statements.append(statement);
        statements[nStatements++] = statement;
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
        /*if (!childBlocksMap.contains(n)) {
            reportError("Bad pointer");
            return nullptr;
        }*/
        return childBlocksMap[n];
    }

    MyVariant* getVariableByName(const QString& ss)
    {
       return variablesMap[ss];
    }

    void addFunction(const QString& name, const QList<QString>& parameters)
    {
        functionsMap.insert(name, new Block(this,parameters, stack));
    }

    int addChildBlock()
    {
        //childBlocksMap.insert(number-1, new Block(this));
        //return number - 1;
        childBlocksMap[number-1] = new Block(this, stack);
        return number - 1;
    }

    void deleteChildBlock(int number)
    {
       /* if (!childBlocksMap.contains(number))
        {
            reportError("Bad pointer");
            return;
        }
        delete childBlocksMap[number];
        childBlocksMap.remove(number);*/
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

    void addVariable(const QString& ss, DataType dataType = REAL, int m_size = -1, real_type data = 0)
    {
        MyVariant* temp;
        stack->allocate(temp);
        switch (dataType)
        {
        case REAL:
            *temp = MyVariant(data);
            break;

        case INTEGER:
            *temp = MyVariant(int(data));
            break;

        default:
            *temp = MyVariant(VOID);

        }

        variablesMap.insert(ss, temp);
    }

    void addIncomingParameter(const QString& ss)
    {
        incomingParameters.append(ss);
    }

    void setVariable(const QString& name, MyVariant* result) {
        *variablesMap[name] = *result;
        std::cout << name.toStdString() << " = " << result->toRealType() << std::endl;
    }

    void deleteVariable(const QString& ss) {
        variablesMap.remove(ss);
    }

    void runChildBlock(int id)
    {
        if (id==-1) return;

       /* if (!childBlocksMap.contains(id)) {
            reportError("Bad pointer");
            return;
        }*/
        childBlocksMap[id]->run();
    }

    void run(MyVariant* result = nullptr, QList<MyVariant*> *parameters = nullptr)
    {
        if (resultPtr!=nullptr)
            resultPtr->reset();
        if (parameters != nullptr)
        {
            if (parameters->size()!=incomingParameters.size())
            {
                reportError("Invalid number of parameters");
                *result = MyVariant();
                return;
            }

            for(int i=0; i<parameters->size(); ++i)
            {
                *variablesMap[incomingParameters[i]] = *parameters->at(i);
            }
        }

        for (int i=0; i<nStatements; ++i)
        {
            statements[i]->eval();
        }

        if (resultPtr!=nullptr)
            *result = *resultPtr;
    }

    bool runLast(MyVariant* result)
    {
        if (nStatements!=0)
          return statements[nStatements-1]->eval(result);

        return false;
    }

    friend int getLevel(Block* block);

    int getId()
    {
        return m_id;
    }

    int howManyStatements()
    {
        return nStatements;
    }
};

#endif // BLOCK_H

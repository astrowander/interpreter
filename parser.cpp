#include "parser.h"

bool Parser::parse(QStringList list, Statement *m_currentStatement)
{    
    currentStatement = m_currentStatement;

    switch (keywords.indexOf(list[0]))
    {
    case 0: //delete
        list.removeAt(0);
        sp.setString(list.join(' '));

        while (!sp.endOfString()) {
            QString name = sp.getWord();
            if (activeBlock->isVariableDeclared(name)) {
                currentStatement->createNodeAbove(new DeleteVariable(activeBlock->getVariableByValue(name),activeBlock,&Block::deleteVariable));
                //activeBlock->deleteVariable(name);
                continue;
            }
            reportError("Variable " + name + "is not declared");
            return false;
        }
        break;

    case 1: //display
        list.removeAt(0);
        sp.setString(list.join(' '));

        while (!sp.endOfString()) {
            QString name = sp.getWord();
            if (activeBlock->isVariableDeclared(name)) {
                currentStatement->createNodeAbove(new DisplayVariable(activeBlock->getVariableByValue(name)));
                continue;
            }
            std::cout << name.toStdString() << std::endl;
        }
        break;

    case 2: //function
    {
        list.removeAt(0);
        sp.setString(list.join(' '));

        QString name = sp.getWord();

        if (activeBlock->isFunctionDeclared(name)) {
            reportError ("function " + name + " is already declared");
            return false;
        }

        if (!sp.match('(')) return false;
        QList<QString> parameters;
        while(!sp.lookIs(')')) {
            parameters.append(sp.getWord());
            if (!sp.lookIs(',') && !sp.lookIs(')')) {
                reportExpected("',' or ')'");
                return false;
            }
            if (sp.lookIs(',')) sp.match(',');
        }        

        if (!sp.match(')')) return false;

        activeBlock->addFunction(name,parameters);
        if (sp.lookIs('=')) {

            sp.setString("result" + sp.getCurrentString().mid(sp.getCursor()));
            activeBlock = activeBlock->getFunctionByName(name);
            if (!assign()) {
                activeBlock = activeBlock->getParent();
                activeBlock->deleteFunction(name);
                return false;
            }
            activeBlock = activeBlock->getParent();
            activeBlock->getFunctionByName(name)->addStatement(currentStatement);

            delete currentStatement;
            currentStatement = new Statement;
            return true;
        }

        if (!sp.endOfString())
        {
            reportError("Extra token");
            activeBlock = activeBlock->getParent();
            activeBlock->deleteFunction(name);
            return false;
        }

        activeBlock = activeBlock->getFunctionByName(name);
        break;
    }

    case 3: //end

        if (activeBlock->getParent()==nullptr) {
            reportError("'end' token is unallowed here");
            return false;
        }
        activeBlock=activeBlock->getParent();
        break;

    default: // assign
        sp.setString(list.join(' '));
        if (!assign()) return false;
        break;
    }
    return true;
}

bool Parser::add()
{
    sp.match('+');

    currentStatement->createNodeAbove(new AddOp);
    currentStatement->goUp();
    if (!term()) return false;
    currentStatement->goUp();
    return true;
}

bool Parser::substract()
{
    sp.match('-');
    currentStatement->createNodeAbove(new SubstractOp);
    currentStatement->goUp();
    if (!term()) return false;
    currentStatement->goUp();
    return true;
}

bool Parser::multiply()
{
    if (!sp.match('*')) return false;
    currentStatement->createNodeAbove(new MultiplyOp);
    currentStatement->goUp();
    if (!factor()) return false;
    if (!doHighPriorityOperations()) return false;
    return true;
}

bool Parser::divide()
{
    if (!sp.match('/')) return false;
    currentStatement->createNodeAbove(new DivideOp);
    currentStatement->goUp();
    if (!factor()) return false;
    if (!doHighPriorityOperations()) return false;
    return true;
}

bool Parser::power()
{
    sp.match('^');
    currentStatement->createNodeAbove(new PowerOp);
    currentStatement->goUp();
    if (!factor()) return false;
    currentStatement->goUp();
    return true;
}

bool Parser::ident()
{
    QString name = sp.getWord();

    Block* blockWhereFunctionCalled = activeBlock;
    while(blockWhereFunctionCalled!=nullptr) {
        if (blockWhereFunctionCalled->isFunctionDeclared(name)) {
            if (!sp.match('(')) return false;

            Block* called = blockWhereFunctionCalled->getFunctionByName(name);
            CallFunction* callFunction = new CallFunction(called, &Block::run);
            currentStatement->createRightChild(callFunction);

            while (!sp.lookIs(')'))
            {
                Statement* buf = new Statement;
                *buf = *currentStatement;
                currentStatement->deleteTree();

                if(!assign()) return false;

                buf->getCurrentNode()->arguments.append( currentStatement->getRoot() );

                *currentStatement = *buf;
                delete buf;

                if (sp.lookIs(',')) sp.match(',');
            }
        sp.match(')');
        return true;
        }
        blockWhereFunctionCalled = blockWhereFunctionCalled->getParent();
    }

    if (!activeBlock->isVariableDeclared(name))
    {
        if (sp.lookIs('[')) {

            sp.match('[');

            bool ok;
            int size = sp.getInt(&ok);

            if (!ok || !sp.match(']')) return false;

            activeBlock->addVariable(name,ARRAY,size,new real_type[size]);
        }
        else {
            activeBlock->addVariable(name);
        }

        currentStatement->createRightChild(new Variable(activeBlock->getVariableByValue(name)));
        return true;
    }

    if (!sp.lookIs('[')) {
        currentStatement->createRightChild(new Variable(activeBlock->getVariableByValue(name)));
        return true;
    }

    sp.match('[');

    bool ok;
    int n = sp.getInt(&ok);
    sp.skipSpaces();
    if (!ok)
    {
        reportError("Invalid index");
        return false;
    }
    currentStatement->createRightChild(new ArrayElement(activeBlock->getVariableByValue(name),n));//Variable(MyVariant(NUMBER,activeBlock->getVariableByValue(name).atPtr(n,&ok))));
    sp.match(']');
    return true;
}

bool Parser::factor()
{
    if (sp.lookIs('(')) {
        sp.match('(');
        if (!assign()) return false;
        if (!sp.match(')')) return false;
        return true;
    }

    if (sp.lookIsLetter()) {
        if (!ident()) return false;
        return true;
    }

    bool ok;
    real_type* v = new real_type(sp.getReal(&ok));
    sp.skipSpaces();
    if (!ok) {
        reportError("Invalid number");
        delete v;
        return false;
    }

    currentStatement->createRightChild(new Literal(MyVariant(NUMBER, v)));
    delete v;
    return true;
}

bool Parser::doHighPriorityOperations()
{
    if (sp.lookIs('=')) {
        sp.match('=');
        currentStatement->createNodeAbove(new AssignOp);
        currentStatement->goUp();
        if (!assign()) return false;
        return true;
    }

    if (sp.lookIs('^')) {
        if (!power()) return false;
        return true;
    }
    return true;
}



bool Parser::term()
{
    if (!factor()) return false;
    if (!doHighPriorityOperations()) return false;
    while (sp.lookIs('*') || sp.lookIs('/')) {

        if (sp.lookIs('*')) {
            if (!multiply()) return false;
            currentStatement->goUp();
        }
        else if (sp.lookIs('/')) {
            if (!divide()) return false;
            currentStatement->goUp();
        }
        else {
            reportExpected("Mulop");
            return false;
        }
    }
    return true;
}

bool Parser::assign()
{
    outputResult=true;
    if (sp.lookIsAddop()) {
        currentStatement->createRightChild(new Literal(MyVariant(NUMBER, new real_type(0))));
    }
    else {
        if (!term()) return false;
    }

     while (!sp.lookIs('\0') && !sp.lookIs(')') && !sp.lookIs(',') && !sp.lookIs(';'))
     {
         if (sp.lookIs('+')) {
             if (!add()) return false;
         }
         else if (sp.lookIs('-')) {
             if (!substract()) return false;
         }
         else {
             reportExpected("Addop");
             return false;
         }
     }
     if (sp.lookIs(';')) {
         outputResult = false;
     }
     return true;
}

bool Parser::doOutput() const
{
    return outputResult;
}

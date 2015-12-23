#include "parser.h"

bool Parser::parse(QStringList list)
{    
    currentStatement = new Statement;

    switch (keywords.indexOf(list[0]))
    {
    case 0: //display
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

    case 1: //function
    {
        list.removeAt(0);
        sp.setString(list.join(' '));

        QString name = sp.getWord();

        if (activeBlock->isFunctionDeclared(name)) {
            reportWarning("Function " + name + " has been re-declared");
            activeBlock->deleteFunction(name);
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
    // if
    case 2:
    {
        list.removeAt(0);
        sp.setString(list.join(' '));
        sp.match('(');
        //reading condition
        if (!assign()) return false;
        sp.match(')');
        
        int ifTrue = activeBlock->addChildBlock();
        activeBlock = activeBlock->getChildBlockByNumber(ifTrue);

        Statement* buf = new Statement();
        *buf = *currentStatement;
        delete currentStatement;

        if(!block()) {
            activeBlock = activeBlock->getParent();
            activeBlock->deleteChildBlock(ifTrue);
            return false;
        }

        currentStatement = new Statement;
        *currentStatement = *buf;
        delete buf;

        QStringList temp;
        (interpreter->*pToGet)(temp);
        if (temp.at(0)=="else")
        {
            buf = new Statement();
            *buf = *currentStatement;
            delete currentStatement;

            int ifFalse = activeBlock->addChildBlock();
            activeBlock = activeBlock->getChildBlockByNumber(ifFalse);

            if(!block()) {
                activeBlock = activeBlock->getParent();
                activeBlock->deleteChildBlock(ifFalse);
                return false;
            }


            currentStatement = new Statement;
            *currentStatement = *buf;
            delete buf;

            currentStatement->setCurrentNodeToRoot();
            stack->push(MyVariant());
            currentStatement->createNodeAbove(new IfOp(stack->last(), activeBlock, &Block::runChildBlock, ifTrue, ifFalse));
            activeBlock->addStatement(currentStatement);
        }
        else
        {
            currentStatement->setCurrentNodeToRoot();
            stack->push(MyVariant());
            currentStatement->createNodeAbove(new IfOp(stack->last(), activeBlock, &Block::runChildBlock, ifTrue));
            activeBlock->addStatement(currentStatement);
            delete currentStatement;

            (interpreter->*pToAddBuf)(temp.join(' '));
            return true;
        }
        break;
    }

    case 3: //loop
    {
        list.removeAt(0);
        sp.setString(list.join(' '));
        sp.match('(');
        //reading condition
        if (!assign()) return false;
        sp.match(')');

        int ifTrue = activeBlock->addChildBlock();
        activeBlock = activeBlock->getChildBlockByNumber(ifTrue);

        Statement* buf = new Statement();
        *buf = *currentStatement;
        delete currentStatement;

        if(!block()) {
            activeBlock = activeBlock->getParent();
            activeBlock->deleteChildBlock(ifTrue);
            return false;
        }

        currentStatement = new Statement;
        *currentStatement = *buf;
        delete buf;

        currentStatement->setCurrentNodeToRoot();
        stack->push(MyVariant());
        currentStatement->createNodeAbove(new LoopOp(stack->last(), activeBlock, &Block::runChildBlock, ifTrue));
        activeBlock->addStatement(currentStatement);
        delete currentStatement;

        return true;
    }

    case 4: //var
    {
        list.removeAt(0);
        sp.setString(list.join(' '));
        QString name = sp.getWord();     
        activeBlock->addVariable(name);

        if (sp.lookIs('=')) {
            sp.setString(name+sp.getCurrentString().mid(sp.getCursor()));
            if (!assign()) return false;
            activeBlock->addStatement(currentStatement);
        }
        break;
    }
    //while
    case 5:
    {
        list.removeAt(0);
        sp.setString(list.join(' '));
        sp.match('(');
        //reading condition
        if (!assign()) return false;
        sp.match(')');

        int ifTrue = activeBlock->addChildBlock();
        activeBlock = activeBlock->getChildBlockByNumber(ifTrue);

        Statement* buf = new Statement();
        *buf = *currentStatement;
        delete currentStatement;

        if(!block()) {
            activeBlock = activeBlock->getParent();
            activeBlock->deleteChildBlock(ifTrue);
            return false;
        }

        currentStatement = new Statement;
        *currentStatement = *buf;
        delete buf;

        currentStatement->setCurrentNodeToRoot();
        stack->push(MyVariant());
        currentStatement->createNodeAbove(new WhileOp(stack->last(), activeBlock, &Block::runChildBlock, ifTrue));
        activeBlock->addStatement(currentStatement);
        delete currentStatement;

        return true;
    }

    case 6: //end

        if (activeBlock->getParent()==nullptr) {
            reportError("'end' token is unallowed here");
            return false;
        }
        activeBlock=activeBlock->getParent();
        break;

    default: // assign
        sp.setString(list.join(' '));
        if (!assign()) return false;
        activeBlock->addStatement(currentStatement);
        break;
    }

    delete currentStatement;
    return true;
}

bool Parser::block()
{
    QStringList toParseList;
    Block* oldBlock = activeBlock;

    while (oldBlock==activeBlock)
    {
        if ((interpreter->*pToGet)(toParseList))
            return false;

        if (!parse(toParseList))
            return false;
    }
    return true;
}

bool Parser::assign()
{
    outputResult=true;
    if (!boolExpression()) return false;
    if (sp.lookIs('=')) {
        stack->push(MyVariant(VOID));
        currentStatement->createNodeAbove(new AssignOp(stack, stack->last()));
        currentStatement->goUp();
        sp.match('=');
        if(!boolExpression()) return false;
    }

     if (sp.lookIs(';')) {
         sp.match(';');
         outputResult = false;
     }

     return true;
}

bool Parser::boolExpression()
{
    if (!boolTerm()) return false;
    while (!sp.lookIs('\0') && !sp.lookIs(')') && !sp.lookIs(',') && !sp.lookIs(';') && !sp.lookIs('}') && !sp.lookIs(']') && !sp.lookIs('='))
    {
        if (sp.lookIs('|')) {
            sp.match('|');
            stack->push(MyVariant());
            currentStatement->createNodeAbove(new BoolOrOp(stack->last()));
        }

        else if (sp.lookIs('~')) {
            stack->push(MyVariant());
            currentStatement->createNodeAbove(new BoolXorOp(stack->last()));
        }
        else {
            reportExpected("'|' or '~'");
        }
        currentStatement->goUp();
        if (!boolTerm()) return false;
        currentStatement->goUp();
    }
    return true;
}

bool Parser::boolTerm()
{
    if (!notFactor()) return false;
    while (sp.lookIs('&'))
    {
        sp.match('&');
        stack->push(MyVariant());
        currentStatement->createNodeAbove(new BoolAndOp(stack->last()));
        currentStatement->goUp();
        if (!notFactor()) return false;
        currentStatement->goUp();
    }
    return true;
}

bool Parser::notFactor()
{
    if (sp.lookIs('!'))
    {
        sp.match('!');
        stack->push(MyVariant());
        currentStatement->createNodeAbove(new BoolNotOp(stack->last()));
    }
    if (!relation()) return false;
    return true;
}

bool Parser::relation()
{
    if (!expression()) return false;

    if (sp.lookIsRelop() && !sp.lookIsAssignmentOp())
    {
        if (sp.lookIs('=') && sp.nextIs('=')) {
            sp.match('=');
            sp.match('=');
            stack->push(MyVariant());
            currentStatement->createNodeAbove(new IsEqualOp(stack->last()));
        }

        else if (sp.lookIs('!') && sp.nextIs('=')) {
            sp.match('!');
            sp.match('=');
            stack->push(MyVariant());
            currentStatement->createNodeAbove(new IsNotEqualOp(stack->last()));
        }

        else if (sp.lookIs('<')) {
            sp.match('<');
            if (sp.lookIs('=')) {
                sp.match('=');
                stack->push(MyVariant());
                currentStatement->createNodeAbove(new IsLessEqualOp(stack->last()));
            }
            else {
                stack->push(MyVariant());
                currentStatement->createNodeAbove(new IsLessOp(stack->last()));
            }
        }

        else if (sp.lookIs('>')) {
            sp.match('>');
            if (sp.lookIs('=')) {
                sp.match('=');
                stack->push(MyVariant());
                currentStatement->createNodeAbove(new IsMoreEqualOp(stack->last()));
            }
            else {
                stack->push(MyVariant());
                currentStatement->createNodeAbove(new IsMoreOp(stack->last()));
            }
        }

        currentStatement->goUp();
        if (!expression()) return false;
        currentStatement->goUp();
        return true;
    }
    return true;
}
bool Parser::expression()
{
    if (!term()) return false;
    while (sp.lookIsAddop())
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
        currentStatement->goUp();
        if (!term()) return false;
        currentStatement->goUp();
    }
    return true;
}

bool Parser::add()
{
    sp.match('+');
    stack->push(MyVariant());
    currentStatement->createNodeAbove(new AddOp(stack->last()));
    return true;
}

bool Parser::substract()
{
    sp.match('-');
    stack->push(MyVariant());
    currentStatement->createNodeAbove(new SubstractOp(stack->last()));
    return true;
}

bool Parser::term()
{
    if (!signedFactor()) return false;

    while (sp.lookIsMulop()) {
        if (sp.lookIs('*')) {
            if (!multiply()) return false;
        }
        else if (sp.lookIs('/')) {
            if (!divide()) return false;
        }
        else {
            reportExpected("Mulop");
            return false;
        }
        currentStatement->goUp();
        if (!powerFactor()) return false;
        currentStatement->goUp();
    }
    return true;
}

bool Parser::multiply()
{
    if (!sp.match('*')) return false;
    stack->push(MyVariant());
    currentStatement->createNodeAbove(new MultiplyOp(stack->last()));
    return true;
}

bool Parser::divide()
{
    if (!sp.match('/')) return false;
    stack->push(MyVariant());
    currentStatement->createNodeAbove(new DivideOp(stack->last()));
    return true;
}


bool Parser::powerFactor()
{
    if (!factor()) return false;

    while (sp.lookIs('^')) {
        sp.match('^');
        stack->push(MyVariant());
        currentStatement->createNodeAbove(new PowerOp(stack->last()));
        currentStatement->goUp();
        if (!factor()) return false;
        currentStatement->goUp();
    }
    return true;
}

bool Parser::signedFactor()
{
    if (sp.lookIs('-')) {
            sp.match('-');
            stack->push(MyVariant());
            currentStatement->createRightChild(new UnaryMinus(stack->last()));
    }
    if (!powerFactor()) return false;
    return true;
}

bool Parser::factor()
{
    //if (sp.endOfString()) return true;

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

    if (sp.lookIs('{'))
    {
        sp.match('{');
        stack->push(MyVariant());
        currentStatement->createRightChild(new InitializerList((stack->last())));
        while (!sp.lookIs('}'))
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
        if(!sp.match('}')) return false;
        return true;
    }

    bool ok, isInt;
    real_type tempReal = sp.getReal(&ok, &isInt);

    if (!ok) {
        reportError("Invalid number");
        return false;
    }

    if (!isInt) {
        real_type* v = new real_type(tempReal);
        sp.skipSpaces();
        stack->push(MyVariant(v));
        currentStatement->createRightChild(new Literal(stack->last()));
        delete v;
        return true;
   }
   //else

   int* v = new int(tempReal);
   sp.skipSpaces();
   stack->push((MyVariant(v)));
   currentStatement->createRightChild(new Literal(stack->last()));
   delete v;
   return true;
}

bool Parser::ident()
{
    QString name = sp.getWord();

    if (presetFunctions.contains(name)) {
        switch (presetFunctions.indexOf(name))
        {
        case 0: //abs
            stack->push(MyVariant());
            currentStatement->createRightChild(new AbsOp(stack->last()));
            break;
        case 1: //log
            stack->push(MyVariant());
            currentStatement->createRightChild(new LogOp(stack->last()));
            break;
        case 2: //sqrt
            stack->push(MyVariant());
            currentStatement->createRightChild(new SqrtOp(stack->last()));
            break;
        }

        if(!sp.match('(')) return false;
        if (!assign()) return false;
        if(!sp.match(')')) return false;
        return true;
    }

    Block* seeingBlock = activeBlock;
    while(seeingBlock!=nullptr) {
        if (seeingBlock->isFunctionDeclared(name)) {
            if (!sp.match('(')) return false;

            stack->push(MyVariant());
            Block* called = seeingBlock->getFunctionByName(name);
            CallFunction* callFunction = new CallFunction(called, &Block::run, stack->last());
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

        if (seeingBlock->isVariableDeclared(name)) {
            if (!sp.lookIs('[')) {
                currentStatement->createRightChild(new Variable(seeingBlock->getVariableByValue(name)));
                return true;
            }

            sp.match('[');
            currentStatement->createRightChild(new ArrayElement(seeingBlock->getVariableByValue(name)));

            if (!assign()) return false;
            currentStatement->goUp();
            sp.skipSpaces();

            //Variable(MyVariant(NUMBER,activeBlock->getVariableByValue(name).atPtr(n,&ok))));
            sp.match(']');
            return true;
        }
        seeingBlock = seeingBlock->getParent();
    }
    reportError("Variable or function " + name + " is not declared");

    return false;
}





bool Parser::doOutput() const
{
    return outputResult;
}

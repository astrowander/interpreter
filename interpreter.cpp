#include "interpreter.h"





Interpreter::Interpreter()
{
    activeBlock = &mainBlock;
    forbidOutput = false;
}

void Interpreter::reportError(const QString &ss)
{
    std::cout << "Error:" << ss.toStdString() << "." << std::endl;
}


void Interpreter::reportRuntimeError(const QString &ss)
{
    std::cout << "Runtime error: " << ss.toStdString() << std::endl;
}


void Interpreter::reportWarning(const QString &ss)
{
    std::cout << "Warning: " << ss.toStdString() << "." << std::endl;
}

void Interpreter::reportExpected(const QString &ss)
{
    reportError(ss + " expected");
}

void Interpreter::run()
{
    QTextStream inStream(stdin);
    forever
    {
        std::cout << ">>> ";
        statement = inStream.readLine();
        if (statement=="") continue;
        if (statement=="exit") break;
        if (!parseString()) continue;
        MyVariant result(currentStatement->eval());
        if(!forbidOutput && outputResult)
            //activeBlock->setVariable("result", result);
            std::cout << result.toRealType() << std::endl;
    }
}

void Interpreter::getChar()
{
    ++cursor;
    if (cursor < statement.size()) {
        look = statement[cursor];
        return;
    }
    look = '\0';
}

bool Interpreter::match(QChar x)
{
    if (x == look) {
        getChar();
        skipSpaces();
        return true;
    }
    reportExpected(x);
    return false;
}

bool Interpreter::lookIsAddop()
{
    if (look =='+' or look =='-') return true;
    return false;
}

void Interpreter::skipSpaces()
{
    while (look.isSpace()) {
        getChar();
    }
}

QString Interpreter::getWord()
{
    QString token = "";
    if (!look.isLetter()) reportExpected("Name");
    while (look.isLetterOrNumber() && look!='\0') {
        token+=QString(look);
        getChar();
    }
    skipSpaces();
    return token;
}

QString Interpreter::getNum(bool *ok)
{
    QString value = "";
    if (!look.isDigit()) reportExpected("Integer");
    while ((look.isDigit() || look=='.')&& look!='\0') {
        value+=QString(look);
        getChar();
    }
    value.toDouble(ok);
    skipSpaces();
    return value;
}

bool Interpreter::add()
{
    match('+');

    currentStatement->createNodeAbove(new AddOp);
    currentStatement->goUp();
    if (!term()) return false;
    currentStatement->goUp();
    return true;
}

bool Interpreter::substract()
{
    match('-');
    currentStatement->createNodeAbove(new SubstractOp);
    currentStatement->goUp();
    if (!term()) return false;
    currentStatement->goUp();
    return true;
}

bool Interpreter::multiply()
{
    if (!match('*')) return false;
    currentStatement->createNodeAbove(new MultiplyOp);
    currentStatement->goUp();
    if (!factor()) return false;
    if (!doHighPriorityOperations()) return false;
    return true;
}

bool Interpreter::divide()
{
    if (!match('/')) return false;
    currentStatement->createNodeAbove(new DivideOp);
    currentStatement->goUp();
    if (!factor()) return false;
    if (!doHighPriorityOperations()) return false;
    return true;
}

bool Interpreter::power()
{
    match('^');
    currentStatement->createNodeAbove(new PowerOp);
    currentStatement->goUp();
    if (!factor()) return false;
    currentStatement->goUp();
    return true;
}

bool Interpreter::ident()
{
    QString name = getWord();
    if (!activeBlock->isVariableDeclared(name))
    {
        activeBlock->addVariable(name);
    }

    currentStatement->createRightChild(new Variable(activeBlock->getVariableByValue(name)));
    return true;
}

bool Interpreter::factor()
{
    if (look == '(') {
        match('(');
        assign();
        if (!match(')')) return false;
        return true;
    }

    if (look.isLetter()) {
        if (!ident()) return false;
        return true;
    }
    bool ok;
    real_type v = getNum(&ok).toDouble();
    if (!ok) {
        reportError("Invalid number");
        return false;
    }
    currentStatement->createRightChild(new Literal(MyVariant(NUMBER,new real_type(v))));
    return true;
}

bool Interpreter::doHighPriorityOperations()
{
    if (look=='=') {
        match('=');
        currentStatement->createNodeAbove(new AssignOp);
        currentStatement->goUp();
        if (!assign()) return false;
        return true;
    }

    if (look=='^') {
        if (!power()) return false;
        return true;
    }
    return true;
}



bool Interpreter::term()
{
    if (!factor()) return false;
    if (!doHighPriorityOperations()) return false;
    while (look =='*' or look == '/') {

        if (look == '*') {
            if (!multiply()) return false;
            currentStatement->goUp();
        }
        else if (look == '/') {
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

bool Interpreter::assign()
{
    if (lookIsAddop()) {
        currentStatement->createRightChild(new Literal(MyVariant(NUMBER, new real_type(0))));
    }
    else {
        if (!term()) return false;
    }

     while (look!='\0' && look!=')' && look!=',' && look != ';')
     {
         if (look == '+') {
             if (!add()) return false;
         }
         else if (look =='-') {
             if (!substract()) return false;
         }
         else {
             reportExpected("Addop");
             return false;
         }
     }
    return true;
}

bool Interpreter::parseString()
{
    cursor = -1;
    getChar();
    skipSpaces();

    currentStatement = new Statement();
    outputResult = true;
    //assignment
    bool success = assign();
    if (look==';') outputResult=false;
    return success;
}

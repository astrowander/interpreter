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
        currentString = inStream.readLine();
        if (currentString=="") continue;
        if (currentString=="exit") break;
        //if (!parseString()) continue;
        QList<QString> list = preprocess();
        foreach (QString ss , list) {
            currentString = ss;
            currentStatement = new Statement();
            if (!parseString()) break;
            activeBlock->addStatement(currentStatement);
            MyVariant result(currentStatement->eval());
            if(!forbidOutput && outputResult)
                //activeBlock->setVariable("result", result);
                std::cout << result.print().toStdString() << std::endl;

            delete currentStatement;
        }


    }
}

void Interpreter::getChar()
{
    ++cursor;
    if (cursor < currentString.size()) {
        look = currentString[cursor];
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



real_type Interpreter::getReal(bool *ok)
{
    QString value = "";
    if (!look.isDigit()) reportExpected("Real");
    while ((look.isDigit() || look=='.')&& look!='\0') {
        value+=QString(look);
        getChar();
    }
    //skipSpaces();
    return value.toDouble(ok);
}

int Interpreter::getInt(bool* ok)
{
    QString value = "";
    if (!look.isDigit()) reportExpected("Integer");
    while (look.isDigit()&& look!='\0') {
        value+=QString(look);
        getChar();
    }
    return value.toInt(ok);
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
        if (look=='[') {

            match('[');

            bool ok;
            int size = getInt(&ok);

            if (!ok || !match(']')) return false;

            activeBlock->addVariable(name,ARRAY,size,new real_type[size]);
        }
        else {
            activeBlock->addVariable(name);
        }

        currentStatement->createRightChild(new Variable(activeBlock->getVariableByValue(name)));
        return true;
    }

    if (look!='[') {
        currentStatement->createRightChild(new Variable(activeBlock->getVariableByValue(name)));
        return true;
    }

    match('[');

    bool ok;
    int n = getInt(&ok);
    skipSpaces();
    if (!ok)
    {
        reportError("Invalid index");
        return false;
    }
    currentStatement->createRightChild(new ArrayElement(activeBlock->getVariableByValue(name).atPtr(n,&ok)));//Variable(MyVariant(NUMBER,activeBlock->getVariableByValue(name).atPtr(n,&ok))));
    match(']');
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

  /*  if (look == '{') {
        match('{');
        QVector<real_type> vector;

        while (look!='}') {

            bool ok;
            vector.append(getNum(&ok).toDouble());
            if (!ok) return false;

            if (look!='}' and look!=',') {
                reportExpected("',' or '}'");
                return false;
            }
            if (look==',') match(',');
        }
        match('}');
        currentStatement->createRightChild(new Literal(MyVariant(ARRAY, vector.data(), vector.size())));
        return true;
    }*/

    bool ok;
    real_type* v = new real_type(getReal(&ok));
    skipSpaces();
    if (!ok) {
        reportError("Invalid number");
        delete v;
        return false;
    }

    currentStatement->createRightChild(new Literal(MyVariant(NUMBER, v)));
    delete v;
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

QList<QString> Interpreter::preprocess()
{
    QList<QString> list;
    int ntemp = 0;

    cursor = -1;
    getChar();
    while (look!='\0')
    {
       if (look=='{')
       {
           int start, stop;
           start = cursor;

           QString varName = "temp" + QString::number(ntemp);
           int size = 0;

           match('{');
           while (look!='}') {
               ++size;
               QString ss = varName + "[" + QString::number(size-1) + "]=";
               while (look!=',' && look!='}') {
                   ss+=QString(look);
                   getChar();
               }
               ss+=";";
               list.append(ss);
               if (look==',') match(',');
           }
           stop = cursor;
           match('}');
           int len = stop-start+1;
           currentString.replace(start, len, varName);
           cursor-= len - varName.length();
           list.prepend(varName + "[" + QString::number(size) + "];");
           ntemp++;
       }
       getChar();
    }

    list.append(currentString);
    return list;
}

bool Interpreter::parseString()
{
    cursor = -1;
    getChar();
    skipSpaces();


    outputResult = true;
    //assignment
    bool success = assign();
    if (look==';') outputResult=false;
    return success;
}

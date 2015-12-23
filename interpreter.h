#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <QTime>
#include <QTextStream>
#include "scanner.h"
#include "parser.h"
#include "preprocessor.h"

class Interpreter
{
private:
    Block mainBlock;

    Scanner scanner;
    Parser parser;
    Preprocessor preprocessor;

    QStringList buffer;
    QTextStream inStream;
    MyCache myVariantStack;
    MyVariant result;

public:

    Interpreter() : inStream(stdin), parser(&mainBlock, this, &Interpreter::getStringList, &Interpreter::addToBuffer, &myVariantStack)
    {

    }

    ~Interpreter()
    {

    }

    void run();
    int getStringList(QStringList &stringList);
    void addToBuffer(const QString& ss);
};

#endif // INTERPRETER_H

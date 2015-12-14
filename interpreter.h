#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <QTextStream>
#include "scanner.h"
#include "parser.h"
#include "preprocessor.h"

class Interpreter
{
private:
    Block mainBlock;
    //Block* activeBlock;
    Statement* currentStatement;

    Scanner scanner;
    Parser* parser;
    Preprocessor preprocessor;

public:

    Interpreter()
    {
        parser = new Parser(&mainBlock);
    }

    ~Interpreter()
    {
        delete parser;
    }

    QList<QString> preprocess();

    void run();
};

#endif // INTERPRETER_H

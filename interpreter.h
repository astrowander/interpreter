#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <QTextStream>
#include "block.h"

class Interpreter
{
private:
    Block mainBlock;
    Block* activeBlock;
    Statement* currentStatement;

    QString currentString;
    QChar look;
    int cursor;
    bool outputResult, forbidOutput;

    void getChar();
    bool match(QChar x);
    bool lookIsAddop();
    void skipSpaces();

    void reportError(const QString &ss);
    void reportRuntimeError(const QString& ss);
    void reportWarning(const QString &ss);
    void reportExpected(const QString &ss);

    QString getWord();
    real_type getReal(bool *ok);


    bool add();
    bool substract();
    bool multiply();
    bool divide();
    bool power();

    //bool parseExpression();
    bool factor();
    bool ident();
    bool doHighPriorityOperations();
    bool term();
    int getInt(bool *ok);
public:
    Interpreter();

    bool assign();

    QList<QString> preprocess();

    bool parseString();

    void run();
};

#endif // INTERPRETER_H

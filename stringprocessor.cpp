#include "stringprocessor.h"



void StringProcessor::getChar()
{
    ++cursor;
    if (cursor < currentString.size()) {
        look = currentString[cursor];
        return;
    }
    look = '\0';
}

bool StringProcessor::match(QChar ch)
{
    if (ch == look) {
        getChar();
        skipSpaces();
        return true;
    }
    reportExpected(ch);
    return false;
}

bool StringProcessor::lookIsAddop() const
{
    if (look =='+' or look =='-') return true;
    return false;
}

void StringProcessor::skipSpaces()
{
    while (look.isSpace()) {
        getChar();
    }
}

QString StringProcessor::getWord()
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



real_type StringProcessor::getReal(bool *ok)
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

int StringProcessor::getInt(bool* ok)
{
    QString value = "";
    if (!look.isDigit()) reportExpected("Integer");
    while (look.isDigit()&& look!='\0') {
        value+=QString(look);
        getChar();
    }
    return value.toInt(ok);
}

QString StringProcessor::getLook() const
{
    return QString(look);
}



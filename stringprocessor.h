#ifndef STRINGPROCESSOR_H
#define STRINGPROCESSOR_H
#include "definitions.h"

class StringProcessor
{
private:
    QString currentString;
    QChar look;
    int cursor;
public:
    StringProcessor(const QString& m_string = "")
    {
        setString(m_string);
    }

    void getChar();
    bool match(QChar ch);
    bool lookIsAddop() const;
    bool lookIsMulop() const;
    bool lookIsRelop() const;
    bool lookIsAssignmentOp() const;

    void skipSpaces();

    QString getWord();
    real_type getReal(bool *ok, bool *isInt);
    int getInt(bool *ok);

    bool endOfString () const
    {
        return (look=='\0');
    }

    bool lookIs(QChar ch) const
    {
        return (look==ch);
    }

    bool lookIs(QList<QChar> list) const
    {
        bool result = false;
        foreach (QChar ch , list) {
            result == result || (ch == look);
        }
        return result;
    }

    bool nextIs(QChar ch) const
    {
        if (cursor >= currentString.size()-1)
            return false;

        return currentString.at(cursor+1) == ch;
    }

    bool lookIsLetter()
    {
        return look.isLetter();
    }

    bool lookIsDigit()
    {
        return look.isDigit();
    }

    int getCursor() const
    {
        return cursor;
    }

    void replace(int start, int stop, const QString& ss)
    {
        int len = stop-start+1;
        currentString.replace(start, len, ss);
        cursor-= len - ss.length();
    }

    void setString(const QString& m_string = "")
    {
        currentString = m_string;
        cursor=-1;
        getChar();
        skipSpaces();
    }

    QString getCurrentString() const
    {
        return currentString;
    }
    QChar getLook() const;
};

#endif // STRINGPROCESSOR_H

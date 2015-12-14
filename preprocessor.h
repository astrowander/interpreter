#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H
#include "stringprocessor.h"

class Preprocessor
{
private:
    QString currentString;
public:
    Preprocessor(const QString& ss = "") : currentString(ss) {}

    QList<QString> preprocess(const QString& ss)
    {
        StringProcessor sp(ss);
        QList<QString> list;
        int ntemp = 0;

        while (!sp.endOfString())
        {
            if (sp.lookIs('{')) {
                int start, stop;
                start = sp.getCursor();

                QString varName = "temp" + QString::number(ntemp);
                int size = 0;

                sp.match('{');
                while (!sp.lookIs('}')) {
                    ++size;
                    QString ss = varName + "[" + QString::number(size-1) + "]=";
                    while ( !sp.lookIs(',') && !sp.lookIs('}')) {
                        ss+=sp.getLook();
                        sp.getChar();
                    }
                    ss+=";";
                    list.append(ss);
                    if (sp.lookIs(',')) sp.match(',');
                }
                stop = sp.getCursor();
                sp.match('}');
                sp.replace(start, stop, varName);
                list.prepend(varName + "[" + QString::number(size) + "];");
                ntemp++;
            }
            sp.getChar();
        }
        list.append(sp.getCurrentString());

        for (int i=0; i<ntemp; ++i) {
            list.append("delete temp" + QString::number(i));
        }
        return list;
    }
};

#endif // PREPROCESSOR_H

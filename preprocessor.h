#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H
#include "stringprocessor.h"
#include <QFile>
#include <QTextStream>

class Preprocessor
{
private:
    QString currentString;
public:
    Preprocessor(const QString& ss = "") : currentString(ss) {}

    QStringList preprocess(const QString& ss)
    {
        //StringProcessor sp(ss);
        QStringList list, tempList;
        tempList = ss.split(' ');
        if (tempList[0]=="import")
        {
            tempList.removeAt(0);
            if (tempList.isEmpty()) {
                reportExpected("Filename");
                return QList<QString>();
            }

            QString filename = tempList.join(' ');
            QFile importFile(filename);
            if (!importFile.open(QIODevice::ReadOnly)) {
                reportError("Unable to open file");
                return QList<QString>();
            }

            QTextStream importStream(&importFile);
            while (!importStream.atEnd())
            {
                list.append(preprocess(importStream.readLine()));
            }
            importFile.close();
            /*foreach(QString str, list)
                std::cout<<str.toStdString() << std::endl;*/
            return list;
        }

/*
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
                list.prepend("var " + varName + "[" + QString::number(size) + "];");
                ntemp++;
            }
            sp.getChar();
        }
        list.append(sp.getCurrentString());

        for (int i=0; i<ntemp; ++i) {
            list.append("delete temp" + QString::number(i));
        }
     */
        return QStringList(ss);
    }

};

#endif // PREPROCESSOR_H

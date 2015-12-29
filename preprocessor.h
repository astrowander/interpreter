#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H
#include "stringprocessor.h"
#include <QFile>
#include <QTextStream>

class Preprocessor
{
private:
    QString currentString;
    const QString workdir = "/home/astrowander/libs/";
public:
    Preprocessor(const QString& ss = "") : currentString(ss) {}

    QStringList preprocess(const QString& ss, bool* ok)
    {
        //StringProcessor sp(ss);
        *ok = false;
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
            QFile importFile(workdir+filename);
            if (!importFile.open(QIODevice::ReadOnly)) {
                reportError("Unable to open file");
                return QList<QString>();
            }

            QTextStream importStream(&importFile);
            while (!importStream.atEnd())
            {
                list.append(importStream.readLine());
            }
            importFile.close();
        }

        *ok = true;
        return list;
    }

};

#endif // PREPROCESSOR_H

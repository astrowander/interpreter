#ifndef SCANNER_H
#define SCANNER_H
#include "definitions.h"

class Scanner
{
private:

public:
    Scanner();
    QStringList scan(const QString& ss)
    {
        return ss.split(' ');
    }
};

#endif // SCANNER_H

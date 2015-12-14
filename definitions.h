#ifndef DEFINITIONS
#define DEFINITIONS

#include <iostream>
#include <valarray>

#include <QList>
#include <QMap>
#include <QStack>
#include <QString>
#include <QStringList>
#include <QVector>

typedef long double real_type;

void reportError(const QString &ss);
void reportRuntimeError(const QString& ss);
void reportWarning(const QString &ss);
void reportExpected(const QString &ss);

#endif // DEFINITIONS


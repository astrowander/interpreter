#include "definitions.h"

void reportError(const QString &ss)
{
    std::cout << "Error:" << ss.toStdString() << "." << std::endl;
}

void reportRuntimeError(const QString &ss)
{
    std::cout << "Runtime error: " << ss.toStdString() << std::endl;
}

void reportWarning(const QString &ss)
{
    std::cout << "Warning: " << ss.toStdString() << "." << std::endl;
}

void reportExpected(const QString &ss)
{
    reportError(ss + " expected");
}

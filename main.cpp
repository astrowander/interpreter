#include <QCoreApplication>
#include <QMap>
#include <QString>
#include <QVector>


#include "interpreter.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Interpreter interpreter;
    interpreter.run();
    return 0;
}

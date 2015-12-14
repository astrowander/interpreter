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

/*TODO
 * 1. Решить проблему с копированием массивов
 * 2. Разработать лексический анализатор, который преобразовал бы входную строку в список лексем, которые уже передавались бы синтаксическому анализатору, и на их основе
 * уже строилось дерево или создавался новый блок. Возможно, имеет смысл описать лексический и синтаксический анализатор (и препроцессор туда же), как отдельные классы. В зависимости от первой лексемы
 * интерпретатор решает, какой именно метод СА вызвать либо создать новый блок.
 */


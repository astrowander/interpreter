#include "interpreter.h"

void Interpreter::run()
{
    int nOfStatements=0;

    forever
    {
        QStringList toParseList;

        int code = getStringList(toParseList);

        if (code==1)
            continue;
        else if (code==2)
            break;

        if (!parser.parse(toParseList)) continue;

        if (parser.getActiveBlock()==&mainBlock) {

            if (nOfStatements==mainBlock.howManyStatements()) {
                continue;
            }
            nOfStatements = mainBlock.howManyStatements();

            QTime timer;
            timer.start();

            mainBlock.runLast(&result);
            if(parser.doOutput())
                result.print();            
            std::cout << "Time elapsed: " << timer.elapsed() << std::endl;
        }
    }
}

int Interpreter::getStringList(QStringList& stringList)
{
   QString currentString;

   if (buffer.isEmpty()) {
       std::cout << ">>> ";
       for (int i=0; i < getLevel(parser.getActiveBlock()); ++i)
           std::cout << "    ";
       currentString = inStream.readLine();
   }

   else {
       currentString = buffer.first();
       buffer.pop_front();
   }
   currentString = currentString.trimmed();

   if (currentString=="") return 1;
   if (currentString=="exit") return 2;

   bool ok;
   QStringList list = preprocessor.preprocess(currentString, &ok);
   if (!ok) return 1;

   if (!list.isEmpty()) {
       buffer.append(list);
       return 1;
   }

   stringList = scanner.scan(currentString);
   return 0;
}

void Interpreter::addToBuffer(const QString &ss)
{
    buffer.prepend(ss);
}


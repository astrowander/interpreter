#include "interpreter.h"
/*
Interpreter::Interpreter()
{
    activeBlock = &mainBlock;
    forbidOutput = false;
}
*/


void Interpreter::run()
{
    QTextStream inStream(stdin);
    forever
    {
        std::cout << ">>> ";

        for (int i=0; i < getLevel(parser->getActiveBlock()); ++i)
            std::cout << "    ";

        QString currentString = inStream.readLine();
        if (currentString=="") continue;
        if (currentString=="exit") break;

        QList<QString> list = preprocessor.preprocess(currentString);
        foreach (QString ss , list) {
            currentStatement = new Statement();
            if (!parser->parse(scanner.scan(ss), currentStatement)) continue;
            if (currentStatement->getRoot()!=nullptr) parser->getActiveBlock()->addStatement(currentStatement);

            if (parser->getActiveBlock()==&mainBlock) {
                MyVariant result;

                if (currentStatement->getRoot() != nullptr)
                    result = currentStatement->eval();

                if(parser->doOutput())
                    result.print();
            }
            delete currentStatement;
        }
    }
}

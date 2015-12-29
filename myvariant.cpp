#include "myvariant.h"

MyVariant::MyVariant(real_type m_data)
{
    v_isArray = false;
    dataType = REAL;
    realData = m_data;
    dimension = 0;
    size=-1;
   // std::cout << "Constructor of REAL MyVariant is called" << std::endl;
}

MyVariant::MyVariant(int m_data)
{
    v_isArray = false;
    dataType = INTEGER;
    //elements.clear();
    intData = m_data;
    dimension = 0;
    size=-1;
    // std::cout << "Constructor of Integer MyVariant is called" << std::endl;
}

MyVariant::MyVariant(MyVariant *m_elements[], int m_size) : intData(0), realData(0), size(m_size), v_isArray(true)
{
    if (size!=-1)
        dataType = elements[0]->getDataType();
}

MyVariant::MyVariant(const MyVariant &other)
{
    dataType = other.dataType;
    size = other.size;
    v_isArray = other.v_isArray;
    realData = other.realData;
    intData = other.intData;

    for (int i=0; i<size; ++i) {
        elements[i] = other.elements[i];
    }
    //std::cout << "copy constructor" << std::end;
}

MyVariant& MyVariant::operator=(const MyVariant &other)
{
    dataType = other.dataType;
    size = other.size;
    v_isArray = other.v_isArray;
    realData = other.realData;
    intData = other.intData;

    for (int i=0; i<size; ++i) {
        elements[i] = other.elements[i];
    }
   // std::cout << "assignment operator\n";
}

MyVariant::~MyVariant() {}


std::valarray<real_type> MyVariant::toRealValarray() const
{
    std::valarray<real_type> result(size);
    for (int i=0; i<size; ++i) {
        result[i] = elements[i]->realData;
    }
}

void MyVariant::print() const
{
    QString result = "\n= ";
    makePrintString(result);
    result += "\n\n";
    std::cout << result.toStdString();
}

void MyVariant::makePrintString(QString& result) const
{
    //QString result = "\n= ";
    if (v_isArray) {
        result += "{";
        for (int i=0; i<size; ++i) {
            elements[i]->makePrintString(result);
            result += ", ";
        }
        result.chop(2);
        result += "}";
        return;
    }

    switch (dataType)
    {
    case REAL:
            result += QString::number(realData);
            break;

    case INTEGER:
            result += QString::number(intData);
            break;

    case TYPEERROR:
        result += "Type mismatch error";
        break;

    default: //VOID
        return;
    }
    //result += "\n\n";
    //std::cout << result.toStdString();
}
/*
bool MyVariant::setElement(const MyVariant &element, int n)
{
    if (!v_isArray || n>=size || n<0) {
        return false;
    }
    if (element.dataType != dataType)
    {
        //*elements[n] = element.castTo(dataType);
        return true;
    }
    *elements[n] = element;
    return true;
}
*/

void MyVariant::reset()
{
    dataType = VOID;
    size = -1;
    v_isArray = false;
}

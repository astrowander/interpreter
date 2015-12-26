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

/*
void MyVariant::isEqual(MyVariant &result, const MyVariant &v1, const MyVariant &v2)
{
    result.dataType = INTEGER;
    result.size = -1;
    result.isArray = false;
    if (v1.v_isArray && v2.v_isArray) {

        if (v1.size != v2.size)
        {
            result.dataType = TYPEERROR;
            return;
        }
        bool allEqual = true;
        for (int i=0; i<v1.size; ++i) {
            allEqual &= (*v1.elements[i] == *v2.elements[i]);
        }

        return MyVariant(int(result));
    }

    switch (v1.dataType)
    {
    case REAL:
        return MyVariant(int(v1.realData == v2.realData));

    case INTEGER:
        return MyVariant(int(v1.intData == v2.intData));

    default:
        return MyVariant();
    }
}
*/
MyVariant operator ==(const MyVariant &v1, const MyVariant &v2)
{
    /*        QPair<MyVariant, MyVariant> operands = autoCast(v1,v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;*/

    if (v1.v_isArray && v2.v_isArray) {

        if (v1.size != v2.size)
            return MyVariant(TYPEERROR);

        bool result = true;
        for (int i=0; i<v1.size; ++i) {
            result &= (*v1.elements[i] == *v2.elements[i]).toInt();
        }
        return MyVariant(int(result));
    }

    switch (v1.dataType)
    {
    case REAL:
        return MyVariant(int(v1.realData == v2.realData));

    case INTEGER:
        return MyVariant(int(v1.intData == v2.intData));

    default:
        return MyVariant();
    }
}

MyVariant operator<(const MyVariant &v1, const MyVariant &v2)
{
    /*QPair<MyVariant, MyVariant> operands = autoCast(v1,v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;*/

    if (v1.v_isArray || v2.v_isArray) {
        reportError("Arrays could be compared only by ==");
        return MyVariant(TYPEERROR);
    }

    switch (v1.dataType)
    {
    case REAL:
        return MyVariant(int(v1.realData < v2.realData));

    case INTEGER:
        return MyVariant(int(v1.intData < v2.intData));
    }
}

MyVariant operator>(const MyVariant &v1, const MyVariant &v2)
{
    return v2 < v1;
    /* QPair<MyVariant, MyVariant> operands = autoCast(v1,v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;

    if (v1.v_isArray || v2.v_isArray) {
        reportError("Arrays could be compared only by ==");
        return MyVariant(TYPEERROR);
    }

    switch (v1.dataType)
    {
    case REAL:
        return MyVariant(int(v1.realData > v2.realData));
    case INTEGER:
        return MyVariant(int(v1.intData > v2.intData));
    default:
        return MyVariant();
    }*/
}

MyVariant operator<=(const MyVariant &v1, const MyVariant &v2)
{
    /*QPair<MyVariant, MyVariant> operands = autoCast(v1,v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;*/

    if (v1.v_isArray || v2.v_isArray) {
        reportError("Arrays could be compared only by ==");
        return MyVariant(TYPEERROR);
    }

    switch (v1.dataType)
    {
    case REAL:
        return MyVariant(int(v1.realData <= v2.realData));
    case INTEGER:
        return MyVariant(int(v1.intData <= v2.intData));
    default:
        return MyVariant();
    }
}

MyVariant operator>=(const MyVariant &v1, const MyVariant &v2)
{
    return v2 <= v1;
    /*QPair<MyVariant, MyVariant> operands = autoCast(v1,v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;

    if (v1.v_isArray || v2.v_isArray) {
        reportError("Arrays could be compared only by ==");
        return MyVariant(TYPEERROR);
    }

    switch (v1.dataType)
    {
    case REAL:
        return MyVariant(int(v1.realData >= v2.realData));
    case INTEGER:
        return MyVariant(int(v1.intData >= v2.intData));
    }*/
}

MyVariant operator !(const MyVariant &v1)
{
    if (!v1.dataType==INTEGER || v1.v_isArray) {
        return MyVariant(TYPEERROR);
    }
    return MyVariant(int(!v1.intData));
}

MyVariant operator!=(const MyVariant &v1, const MyVariant &v2)
{
    if (v1.v_isArray && v2.v_isArray) {

        if (v1.size != v2.size)
            return MyVariant(TYPEERROR);

        bool result = false;
        for (int i=0; i<v1.size; ++i) {
            result |= (*v1.elements[i] != *v1.elements[i]).toInt();
        }
        return MyVariant(int(result));
    }

    switch (v1.dataType)
    {
    case REAL:
        return MyVariant(int(v1.realData != v2.realData));
    case INTEGER:
        return MyVariant(int(v1.intData != v2.intData));
    default:
        return MyVariant();
    }
}

MyVariant operator &&(const MyVariant &v1, const MyVariant &v2)
{
    if (!(v1.dataType==INTEGER) || v1.v_isArray || !(v2.dataType==INTEGER) || v2.v_isArray) {
        reportError("Operands must be integer");
        return MyVariant(TYPEERROR);
    }
    return MyVariant(int(v1.intData && v2.intData));
}

MyVariant operator ||(const MyVariant &v1, const MyVariant &v2)
{
    if (!(v1.dataType==INTEGER) || v1.v_isArray || !(v2.dataType==INTEGER) || v2.v_isArray) {
        reportError("Operands must be integer");
        return MyVariant(TYPEERROR);
    }
    return MyVariant(int(v1.intData || v2.intData));
}

MyVariant doXor(const MyVariant &v1, const MyVariant &v2)
{
    if (!(v1.dataType==INTEGER) || v1.v_isArray || !(v2.dataType==INTEGER) || v2.v_isArray) {
        reportError("Operands must be integer");
        return MyVariant(TYPEERROR);
    }
    return MyVariant(int(!v1.intData != !v2.intData));
}
/*
MyVariant& operator+(MyVariant &v1, MyVariant &v2)
{
    //QPair<MyVariant, MyVariant> operands = autoCast(v1,v2);
    //MyVariant& v1 = operands.first;
    //MyVariant& v2 = operands.second;

    if (v1.size!= v2.size)
        dataType = VOID;

    if (v1.v_isArray && v2.v_isArray) {
        MyVariant* m_elements[1024];
        for (int i = 0; i < v1.size; ++i) {
            *m_elements[i] = *v1.elements[i] + *v2.elements[i];
        }
        return MyVariant(m_elements, v1.size);
    }

    if (!v1.v_isArray && !v2.v_isArray) {

        switch (v1.dataType) {

        case REAL:
            return MyVariant(v1.realData + v2.realData);
        case INTEGER:
            return MyVariant(v1.intData + v2.intData);
        }
    }

    return MyVariant(TYPEERROR);
}
*/
MyVariant operator-(const MyVariant &v1)
{
    if (v1.v_isArray)
    {
        MyVariant* m_elements[1024];
        for (int i=0; i<v1.size; ++i) {
            *m_elements[i] = -*v1.elements[i];
        }
        return MyVariant(m_elements, v1.size);
    }

    switch (v1.dataType)
    {
    case REAL:
        return MyVariant(-v1.realData);
    case INTEGER:
        return MyVariant(-v1.intData);
    }

    return MyVariant(TYPEERROR);
}

MyVariant operator-(const MyVariant &v1, const MyVariant &v2)
{
    //QPair<MyVariant, MyVariant> operands = autoCast(v1,v2);
    //MyVariant& v1 = operands.first;
    //MyVariant& v2 = operands.second;

    if (v1.size!= v2.size)
        return MyVariant (TYPEERROR);

    if (v1.v_isArray && v2.v_isArray) {
        MyVariant* m_elements[1024];
        for (int i = 0; i < v1.size; ++i) {
            *m_elements[i]= *v1.elements[i] - *v2.elements[i];
        }
        return MyVariant(m_elements, v1.size);
    }

    if (!v1.v_isArray && !v2.v_isArray) {

        switch (v1.dataType) {
        case REAL:
            return MyVariant(v1.realData - v2.realData);
        case INTEGER:
            return MyVariant(v1.intData - v2.intData);
        }
    }

    return MyVariant(TYPEERROR);
}

MyVariant operator*(const MyVariant &v1, const MyVariant &v2)
{
    //QPair<MyVariant, MyVariant> operands = autoCast(v1,v2);
    //MyVariant& v1 = operands.first;
    //MyVariant& v2 = operands.second;

    if (v1.v_isArray && !v2.v_isArray) {
        MyVariant* m_elements[1024];
        for (int i = 0; i < v1.size; ++i) {
            *m_elements[i]= *v1.elements[i] * v2;
        }
        return MyVariant(m_elements, v1.size);
    }

    if (!v1.v_isArray && v2.v_isArray) {
        MyVariant* m_elements[1024];
        for (int i = 0; i < v2.size; ++i) {
            *m_elements[i]= v1 * *v2.elements[i];
        }
        return MyVariant(m_elements, v2.size);
    }

    if (!v1.v_isArray && !v2.v_isArray) {

        switch (v1.dataType) {
        case REAL:
            return MyVariant(v1.realData * v2.realData);
        case INTEGER:
            return MyVariant(v1.intData * v2.intData);
        }
    }

    return MyVariant(TYPEERROR);
}

MyVariant operator/(const MyVariant &v1, const MyVariant &v2)
{
   /* QPair<MyVariant, MyVariant> operands = autoCast(v1,v2);
    MyVariant& v1 = operands.first;
    MyVariant& v2 = operands.second;*/

    if (v1.v_isArray && !v2.v_isArray) {
        MyVariant* m_elements[1024];
        for (int i = 0; i < v1.size; ++i) {
            *m_elements[i]= *v1.elements[i] * v2;
        }
        return MyVariant(m_elements, v1.size);
    }

    if (!v1.v_isArray && !v2.v_isArray) {

        switch (v1.dataType) {
        case REAL:
            return MyVariant(v1.realData * v2.realData);
        case INTEGER:
            return MyVariant(v1.intData * v2.intData);
        }
    }

    return MyVariant(TYPEERROR);
}

MyVariant operator^(const MyVariant &v1, const MyVariant &v2)
{
    /*QPair<MyVariant, MyVariant> operands = autoCast(v1,v2);
    MyVariant& v1 = operands.first;
    MyVariant& v2 = operands.second;*/

    if (!v1.v_isArray && !v2.v_isArray) {

        switch (v1.dataType) {
        case REAL:
            return MyVariant(pow(v1.realData, v2.realData));
        case INTEGER:
            return MyVariant(pow(v1.intData, v2.intData));
        }
    }
    return MyVariant(TYPEERROR);
}

MyVariant myLog(const MyVariant &v1)
{
    if (v1.dataType==REAL)
    {
        if (!v1.v_isArray)
            return MyVariant(log(v1.realData));
        //else
        real_type* result = new real_type[v1.size];
        for (int i = 0; i < v1.size; ++i) {
            result[i]=log(v1.elements[i]->realData);
        }
        return MyVariant(result[0]);
    }

    if (v1.dataType==INTEGER)
    {
        if (!v1.v_isArray)
            return MyVariant(int(log(v1.intData)));
        //else
        int* result = new int[v1.size];
        for (int i = 0; i < v1.size; ++i) {
            result[i]=log(v1.elements[i]->intData);
        }
        return MyVariant(result[0]);
    }
    return MyVariant(TYPEERROR);
}

MyVariant myAbs(const MyVariant &v1)
{
    if (v1.dataType==REAL)
    {
        if (!v1.v_isArray)
            return MyVariant(fabs(v1.realData));
        //else
        real_type* result = new real_type[v1.size];
        for (int i = 0; i < v1.size; ++i) {
            result[i]=fabs(v1.elements[i]->realData);
        }
        return MyVariant(result[0]);
    }

    if (v1.dataType==INTEGER)
    {
        if (!v1.v_isArray)
            return MyVariant(int(abs(v1.intData)));
        //else
        int* result = new int[v1.size];
        for (int i = 0; i < v1.size; ++i) {
            result[i]=abs(v1.elements[i]->intData);
        }
        return MyVariant(result[0]);
    }
    return MyVariant(TYPEERROR);
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

MyVariant *&MyVariant::getElement(int n)
{
    return elements[n];
}

void MyVariant::reset()
{
    dataType = VOID;
    size = -1;
    v_isArray = false;
}

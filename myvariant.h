#ifndef MYVARIANT_H
#define MYVARIANT_H
#include "definitions.h"

enum DataType {VOID, INTEGER, REAL, TYPEERROR};

class MyVariant
{
private:
    DataType dataType;
    void* data;

    QVector<MyVariant*> elements;
    QString name;
    int size;
    bool v_isArray;

public:
    MyVariant(DataType m_dataType=VOID) : dataType(m_dataType), data(nullptr), name(""), size(-1), v_isArray(false) {}
    MyVariant(real_type* m_data, int m_size=-1, const QString& m_name="");
    MyVariant(int* m_data, int m_size=-1, const QString& m_name="");
    MyVariant(QVector<MyVariant*> &m_elements, const QString& m_name = "") : data(nullptr), dataType(VOID), elements(m_elements) , name(m_name), size(m_elements.size()), v_isArray(true)
    {
        if (!elements.isEmpty())
            dataType = elements.at(0)->getDataType();
    }
    MyVariant (MyVariant const& other);
    MyVariant& operator=(const MyVariant& other);
    ~MyVariant();

    real_type toRealType() const;
    int toInt() const;
    std::valarray<real_type> toRealValarray() const;

    DataType getDataType() const;

    MyVariant castTo(DataType otherDataType) const
    {
        MyVariant result;
        result.size = size;
        result.name = name;
        result.v_isArray = v_isArray;
        result.dataType = otherDataType;

        if (v_isArray)
        {
            foreach(MyVariant* element, elements)
            {
                result.elements.append(new MyVariant(element->castTo(otherDataType)));
            }
            return result;
        }

        switch (otherDataType)
        {
        case REAL:
            result.data = new real_type;

            switch (dataType)
            {
            case INTEGER:
                * (real_type*) result.data = (real_type) *(int*) data;
                return result;
            default:
                reportError("Type casting error");
                delete result.data;
                return MyVariant(TYPEERROR);
            }
        case INTEGER:
            result.data = new int;

            switch (dataType)
            {
            case REAL:
                * (real_type*) result.data = (int) *(real_type*) data;
                return result;
            default:
                reportError("Type casting error");
                delete result.data;
                return MyVariant(TYPEERROR);
            }
        default:
            reportError("Type casting error");
            return MyVariant(TYPEERROR);
        }
    }

    static QPair<MyVariant, MyVariant> autoCast(const MyVariant& v1, const MyVariant& v2)
    {
        if (v1.dataType == REAL && v2.dataType == REAL || v1.dataType ==INTEGER && v2.dataType == INTEGER)
            return QPair<MyVariant, MyVariant>(v1,v2);

        if (v1.dataType == REAL && v2.dataType ==INTEGER)
            return QPair<MyVariant, MyVariant> (v1,v2.castTo(REAL));

        if (v1.dataType ==INTEGER && v2.dataType == REAL)
            return QPair<MyVariant, MyVariant> (v1.castTo(REAL), v2);

        return QPair<MyVariant, MyVariant>();
    }

    friend MyVariant operator== (const MyVariant& m_v1, const MyVariant& m_v2)
    {
        QPair<MyVariant, MyVariant> operands = autoCast(m_v1,m_v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;

        if (m_v1.v_isArray && m_v2.v_isArray) {

            if (m_v1.size != m_v2.size)
                return MyVariant(TYPEERROR);

            bool result = true;
            for (int i=0; i<m_v1.size; ++i) {
                result &= (m_v1.elements.at(i) == m_v1.elements.at(i));
            }
            return MyVariant(new int(result));
        }

        switch (m_v1.dataType)
        {
        case REAL:
            return MyVariant(new int(v1.toRealType() == v2.toRealType()));
        case INTEGER:
            return MyVariant(new int(v1.toInt() == v2.toInt()));
        }
    }

    friend MyVariant operator< (const MyVariant& m_v1, const MyVariant& m_v2)
    {
        QPair<MyVariant, MyVariant> operands = autoCast(m_v1,m_v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;

        if (m_v1.v_isArray || m_v2.v_isArray) {
           reportError("Arrays could be compared only by ==");
           return MyVariant(TYPEERROR);
        }

        switch (m_v1.dataType)
        {
        case REAL:
            return MyVariant(new int(v1.toRealType() < v2.toRealType()));
        case INTEGER:
            return MyVariant(new int(v1.toInt() < v2.toInt()));
        }
    }

    friend MyVariant operator> (const MyVariant& m_v1, const MyVariant& m_v2)
    {
        QPair<MyVariant, MyVariant> operands = autoCast(m_v1,m_v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;

        if (m_v1.v_isArray || m_v2.v_isArray) {
            reportError("Arrays could be compared only by ==");
            return MyVariant(TYPEERROR);
        }

        switch (m_v1.dataType)
        {
        case REAL:
            return MyVariant(new int(v1.toRealType() > v2.toRealType()));
        case INTEGER:
            return MyVariant(new int(v1.toInt() > v2.toInt()));
        }
    }

    friend MyVariant operator<= (const MyVariant& m_v1, const MyVariant& m_v2)
    {
        QPair<MyVariant, MyVariant> operands = autoCast(m_v1,m_v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;

        if (m_v1.v_isArray || m_v2.v_isArray) {
            reportError("Arrays could be compared only by ==");
            return MyVariant(TYPEERROR);
        }

        switch (m_v1.dataType)
        {
        case REAL:
            return MyVariant(new int(v1.toRealType() <= v2.toRealType()));
        case INTEGER:
            return MyVariant(new int(v1.toInt() <= v2.toInt()));
        }
    }

    friend MyVariant operator>= (const MyVariant& m_v1, const MyVariant& m_v2)
    {
        QPair<MyVariant, MyVariant> operands = autoCast(m_v1,m_v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;

        if (m_v1.v_isArray || m_v2.v_isArray) {
            reportError("Arrays could be compared only by ==");
            return MyVariant(TYPEERROR);
        }

        switch (m_v1.dataType)
        {
        case REAL:
            return MyVariant(new int(v1.toRealType() >= v2.toRealType()));
        case INTEGER:
            return MyVariant(new int(v1.toInt() >= v2.toInt()));
        }
    }

    friend MyVariant operator !(const MyVariant& m_v1)
    {
        if (!m_v1.dataType==INTEGER || m_v1.v_isArray) {
            return MyVariant(TYPEERROR);
        }
        return MyVariant(new int(!m_v1.toInt()));
    }

    friend MyVariant operator!= (const MyVariant& m_v1, const MyVariant& m_v2)
    {
        return !(m_v1 == m_v2);
    }

    friend MyVariant operator &&(const MyVariant& m_v1, const MyVariant& m_v2)
    {
        if (!(m_v1.dataType==INTEGER) || m_v1.v_isArray || !(m_v2.dataType==INTEGER) || m_v2.v_isArray) {
            reportError("Operands must be integer");
            return MyVariant(TYPEERROR);
        }
        return MyVariant(new int(m_v1.toInt() && m_v2.toInt()));
    }

    friend MyVariant operator ||(const MyVariant& m_v1, const MyVariant& m_v2)
    {
        if (!(m_v1.dataType==INTEGER) || m_v1.v_isArray || !(m_v2.dataType==INTEGER) || m_v2.v_isArray) {
            reportError("Operands must be integer");
            return MyVariant(TYPEERROR);
        }
        return MyVariant(new int(m_v1.toInt() || m_v2.toInt()));
    }

    //XOR
    friend MyVariant doXor(const MyVariant& m_v1, const MyVariant& m_v2)
    {
        if (!(m_v1.dataType==INTEGER) || m_v1.v_isArray || !(m_v2.dataType==INTEGER) || m_v2.v_isArray) {
            reportError("Operands must be integer");
            return MyVariant(TYPEERROR);
        }
        return MyVariant(new int(!m_v1.toInt() != !m_v2.toInt()));
    }

    friend MyVariant operator+(const MyVariant& v1, const MyVariant& v2)
    {
       //QPair<MyVariant, MyVariant> operands = autoCast(m_v1,m_v2);
       //MyVariant& v1 = operands.first;
       //MyVariant& v2 = operands.second;

       if (v1.dataType==REAL) {

            if (!v1.v_isArray && !v2.v_isArray)
                return MyVariant(new real_type(v1.toRealType() + v2.toRealType()));

            if (v1.v_isArray && v2.v_isArray) {
                real_type* result = new real_type[v1.size];
                for (int i = 0; i < v1.size; ++i) {
                    result[i]= v1.elements.at(i)->toRealType() + v2.elements.at(i)->toRealType();
                }
                return MyVariant(result, v1.size);

            }
        }

        if (v1.dataType==INTEGER) {
            if (!v1.v_isArray && !v2.v_isArray)
                return MyVariant(new int(v1.toInt() + v2.toInt()));

            if (v1.v_isArray && v2.v_isArray) {
                real_type* result = new real_type[v1.size];
                for (int i = 0; i < v1.size; ++i) {
                    result[i]= v1.elements.at(i)->toInt() + v2.elements.at(i)->toInt();
                }
                return MyVariant(result, v1.size);

            }
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant operator-(const MyVariant& m_v1)
    {
        if (m_v1.v_isArray)
        {
            QVector<MyVariant*> tempElements;
            for (int i=0; i<m_v1.size; ++i) {
                tempElements.append(new MyVariant(-(*m_v1.elements.at(i))));
            }
            return MyVariant(tempElements);
        }

        switch (m_v1.dataType)
        {
        case REAL:
            return MyVariant(new real_type( -m_v1.toRealType()));
        case INTEGER:
            return MyVariant(new int(-m_v1.toInt()));
        }

        return MyVariant(TYPEERROR);
    }

    friend MyVariant operator-(const MyVariant& m_v1, const MyVariant& m_v2)
    {
        QPair<MyVariant, MyVariant> operands = autoCast(m_v1,m_v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;

        if (v1.dataType==REAL) {

            if (!v1.v_isArray && !v2.v_isArray)
                return MyVariant(new real_type(v1.toRealType() - v2.toRealType()));

            if (v1.v_isArray && v2.v_isArray) {
                real_type* result = new real_type[v1.size];
                for (int i = 0; i < v1.size; ++i) {
                    result[i]= v1.elements.at(i)->toRealType() - v2.elements.at(i)->toRealType();
                }
                return MyVariant(result, v1.size);

            }
        }

        if (v1.dataType==INTEGER) {
            if (!v1.v_isArray && !v2.v_isArray)
                return MyVariant(new int(v1.toInt() - v2.toInt()));

            if (v1.v_isArray && v2.v_isArray) {
                real_type* result = new real_type[v1.size];
                for (int i = 0; i < v1.size; ++i) {
                    result[i]= v1.elements.at(i)->toInt() - v2.elements.at(i)->toInt();
                }
                return MyVariant(result, v1.size);

            }
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant operator*(const MyVariant& m_v1, const MyVariant& m_v2)
    {
        QPair<MyVariant, MyVariant> operands = autoCast(m_v1,m_v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;

        if (v1.dataType==REAL) {
            if (!v1.v_isArray && !v2.v_isArray)
                return MyVariant(new real_type(v1.toRealType() * v2.toRealType()));

            if (v1.v_isArray && !v2.v_isArray) {
                real_type* result = new real_type[v1.size];
                for (int i = 0; i < v1.size; ++i) {
                    result[i]=v1.elements.at(i)->toRealType() * v2.toRealType();
                }
                return MyVariant(result,v1.size);
            }

            if (!v1.v_isArray && v2.v_isArray) {
                real_type* result = new real_type[v2.size];
                for (int i = 0; i < v2.size; ++i) {
                    result[i]=v1.toRealType() * v2.elements.at(i)->toRealType();
                }
                return MyVariant(result,v2.size);
            }
        }

        if (v1.dataType==INTEGER) {
            if (!v1.v_isArray && !v2.v_isArray)
                return MyVariant(new int(v1.toInt() * v2.toInt()));

            if (v1.v_isArray && !v2.v_isArray) {
                int* result = new int[v1.size];
                for (int i = 0; i < v1.size; ++i) {
                    result[i]=v1.elements.at(i)->toInt() * v2.toInt();
                }
                return MyVariant(result,v1.size);
            }

            if (!v1.v_isArray && v2.v_isArray) {
                int* result = new int[v2.size];
                for (int i = 0; i < v2.size; ++i) {
                    result[i]=v1.toInt() * v2.elements.at(i)->toInt();
                }
                return MyVariant(result,v2.size);
            }
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant operator/(const MyVariant& m_v1, const MyVariant& m_v2)
    {
        QPair<MyVariant, MyVariant> operands = autoCast(m_v1,m_v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;

        if (v1.dataType==REAL) {
            if (!v1.v_isArray && !v2.v_isArray)
                return MyVariant(new real_type(v1.toRealType() / v2.toRealType()));

            if (v1.v_isArray && !v2.v_isArray) {
                real_type* result = new real_type[v1.size];
                for (int i = 0; i < v1.size; ++i) {
                    result[i]=v1.elements.at(i)->toRealType() / v2.toRealType();
                }
                return MyVariant(result,v1.size);
            }
         }

        if (v1.dataType==INTEGER) {
            if (!v1.v_isArray && !v2.v_isArray)
                return MyVariant(new int(v1.toInt() / v2.toInt()));

            if (v1.v_isArray && !v2.v_isArray) {
                int* result = new int[v1.size];
                for (int i = 0; i < v1.size; ++i) {
                    result[i]=v1.elements.at(i)->toInt() / v2.toInt();
                }
                return MyVariant(result,v1.size);
            }
         }
         return MyVariant(TYPEERROR);
    }

    friend MyVariant operator^(const MyVariant& m_v1, const MyVariant& m_v2)
    {
        QPair<MyVariant, MyVariant> operands = autoCast(m_v1,m_v2);
        MyVariant& v1 = operands.first;
        MyVariant& v2 = operands.second;

        if (v1.dataType==REAL) {
            if (!v1.v_isArray && !v2.v_isArray)
                return MyVariant(new real_type(pow(v1.toRealType(),  v2.toRealType())));
        }

        if (v1.dataType==INTEGER) {
            if (!v1.v_isArray && !v2.v_isArray)
                return MyVariant(new int(pow(v1.toInt(),  v2.toInt())));
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant mySqrt(const MyVariant& v1)
    {
        if (v1.dataType==REAL)
        {
            if (!v1.v_isArray)
                return MyVariant(new real_type(sqrt(v1.toRealType())));
            //else
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=sqrt(v1.elements.at(i)->toRealType());
            }
            return MyVariant(result,v1.size);
        }

        if (v1.dataType==INTEGER)
        {
            if (!v1.v_isArray)
                return MyVariant(new int(sqrt(v1.toInt())));
            //else
            int* result = new int[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=sqrt(v1.elements.at(i)->toInt());
            }
            return MyVariant(result,v1.size);
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant myLog(const MyVariant&v1)
    {
        if (v1.dataType==REAL)
        {
            if (!v1.v_isArray)
                return MyVariant(new real_type(log(v1.toRealType())));
            //else
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=log(v1.elements.at(i)->toRealType());
            }
            return MyVariant(result,v1.size);
        }

        if (v1.dataType==INTEGER)
        {
            if (!v1.v_isArray)
                return MyVariant(new int(log(v1.toInt())));
            //else
            int* result = new int[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=log(v1.elements.at(i)->toInt());
            }
            return MyVariant(result,v1.size);
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant myAbs(const MyVariant&v1)
    {
        if (v1.dataType==REAL)
        {
            if (!v1.v_isArray)
                return MyVariant(new real_type(fabs(v1.toRealType())));
            //else
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=fabs(v1.elements.at(i)->toRealType());
            }
            return MyVariant(result,v1.size);
        }

        if (v1.dataType==INTEGER)
        {
            if (!v1.v_isArray)
                return MyVariant(new int(abs(v1.toInt())));
            //else
            int* result = new int[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=abs(v1.elements.at(i)->toInt());
            }
            return MyVariant(result,v1.size);
        }
        return MyVariant(TYPEERROR);
    }

    QString getName() const
    {
        return name;
    }

    void print() const
    {
        QString result = "\n" + name + " = ";

        switch (dataType)
        {
        case REAL:
            if (!v_isArray) {
                result += QString::number(double( *(real_type*)data) );
                break;
            }

            result += "{";
            for (int i=0; i<size; ++i) {
                result += QString::number(double(elements.at(i)->toRealType()));
                result += ", ";
            }
            result.chop(2);
            result += "}";
            break;

        case INTEGER:
            if (!v_isArray) {
                result += QString::number( *(int*) data);
                break;
            }

            result += "{";
            for (int i=0; i<size; ++i) {
                result += QString::number(elements.at(i)->toInt());
                result += ", ";
            }
            result.chop(2);
            result += "}";
            break;

        case TYPEERROR:
            result += "Type mismatch error";
            break;

        default: //VOID
            return;
        }
        result += "\n\n";
        std::cout << result.toStdString();
    }

    int getSize() const
    {
        return size;
    }

    real_type* getRealData()
    {
        return (real_type*) data;
    }

    bool isArray() const{
        return v_isArray;
    }

    bool setElement(const MyVariant& element, int n)
    {
        if (!v_isArray || n>=size || n<0) {
            return false;
        }
        if (element.dataType != dataType)
        {
            *elements[n] = element.castTo(dataType);
            return true;
        }
        *elements[n] = element;
        return true;
    }

    MyVariant getElement(int n) const
    {
        return *elements.at(n);
    }

};

#endif // MYVARIANT_H

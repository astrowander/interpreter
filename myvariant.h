#ifndef MYVARIANT_H
#define MYVARIANT_H
#include "definitions.h"

enum DataType {VOID, INTEGER, REAL, REALARRAY, TYPEERROR};

class MyVariant
{
private:
    DataType dataType;
    real_type* data;    
    QString name;
    int size;   

public:
    MyVariant(DataType m_dataType=VOID, real_type* m_data=nullptr, int m_size=0, const QString& m_name="");
    MyVariant (MyVariant const& other);

    MyVariant& operator=(const MyVariant& other);

    ~MyVariant();

    real_type toRealType() const;

    bool setValue();

    std::valarray<real_type> toValarray() const;

    DataType getDataType() const;

    friend MyVariant operator+(const MyVariant& v1, const MyVariant& v2)
    {
        if (v1.dataType==REAL && v2.dataType==REAL) {
            return MyVariant(REAL, new real_type(v1.toRealType() + v2.toRealType()));
        }
        if (v1.dataType==REALARRAY && v2.dataType==REALARRAY) {
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=v1.data[i]+v2.data[i];
            }
            return MyVariant(REALARRAY, result, v1.size);
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant operator-(const MyVariant& v1, const MyVariant& v2)
    {
        if (v1.dataType==REAL && v2.dataType==REAL) {
            return MyVariant(REAL, new real_type(v1.toRealType() - v2.toRealType()));
        }
        if (v1.dataType==REALARRAY && v2.dataType==REALARRAY) {
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=v1.data[i]-v2.data[i];
            }
            return MyVariant(REALARRAY, result,v1.size);
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant operator*(const MyVariant& v1, const MyVariant& v2)
    {
        if (v1.dataType==REAL && v2.dataType==REAL) {
            return MyVariant(REAL, new real_type(v1.toRealType() * v2.toRealType()));
        }
        if (v1.dataType==REALARRAY && v2.dataType==REAL) {
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=v1.data[i]*(*v2.data);
            }
            return MyVariant(REALARRAY, result,v1.size);
        }
        if (v1.dataType==REAL && v2.dataType==REALARRAY) {
            real_type* result = new real_type[v2.size];
            for (int i = 0; i < v2.size; ++i) {
                result[i]=v2.data[i]*(*v1.data);
            }
            return MyVariant(REALARRAY,result,v2.size);
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant operator/(const MyVariant& v1, const MyVariant& v2)
    {
        if (v1.dataType==REAL && v2.dataType==REAL) {
            return MyVariant(REAL, new real_type(v1.toRealType() / v2.toRealType()));
        }

        if (v1.dataType==REALARRAY && v2.dataType==REAL) {
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=v1.data[i] / (*v2.data);
            }
            return MyVariant(REALARRAY, result,v1.size);
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant operator^(const MyVariant& v1, const MyVariant& v2)
    {
        if (v1.dataType==REAL && v2.dataType==REAL) {
            return MyVariant(REAL, new real_type(pow(v1.toRealType(),  v2.toRealType())));
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant mySqrt(const MyVariant&v1)
    {
        if (v1.dataType==REAL)
        {
            return MyVariant(REAL, new real_type(sqrt(v1.toRealType())));
        }

        if (v1.dataType==REALARRAY)
        {
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=sqrt(v1.data[i]);
            }
            return MyVariant(REALARRAY, result,v1.size);
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant myLog(const MyVariant&v1)
    {
        if (v1.dataType==REAL)
        {
            return MyVariant(REAL, new real_type(log(v1.toRealType())));
        }

        if (v1.dataType==REALARRAY)
        {
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=log(v1.data[i]);
            }
            return MyVariant(REALARRAY, result,v1.size);
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant myFabs(const MyVariant&v1)
    {
        if (v1.dataType==REAL)
        {
            return MyVariant(REAL, new real_type(fabs(v1.toRealType())));
        }

        if (v1.dataType==REALARRAY)
        {
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=fabs(v1.data[i]);
            }
            return MyVariant(REALARRAY, result,v1.size);
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
            result += QString::number(double(*data));
            break;
        case REALARRAY:
            result += "{";
            for (int i=0; i<size; ++i) {
                result += QString::number(double(data[i]));
                result += ", ";
            }
            result.chop(2);
            result += "}";
            break;
        case TYPEERROR:
            result += "Type mismatch error";
            break;
        default:
            return;
        }
        result += "\n\n";
        std::cout << result.toStdString();
    }

    real_type at(int n, bool* ok = nullptr) const
    {
        if (dataType!=REALARRAY || n>=size || n<0) {
            *ok=false;
            return 0.0;
        }
        return data[n];
    }

    real_type* atPtr(int n, bool* ok = nullptr)
    {
        if (dataType!=REALARRAY || n>=size || n<0) {
            if (ok!=nullptr) *ok=false;
            return nullptr;
        }
        return data+n;
    }

    int getSize() const
    {
        return size;
    }
};

#endif // MYVARIANT_H

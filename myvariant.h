#ifndef MYVARIANT_H
#define MYVARIANT_H
#include <iostream>;
#include <valarray>;
#include <QString>


typedef long double real_type;

enum DataType {VOID, NUMBER, ARRAY, TYPEERROR};

class Statement;

class MyVariant
{
private:
    DataType dataType;
    real_type* data;
    Statement* statement;
    QString name;
    int size;
public:
    MyVariant(DataType m_dataType=VOID, real_type* m_data=nullptr, int m_size=0, const QString& m_name="");
    MyVariant(Statement* m_statement);
    MyVariant (MyVariant const& other);

    MyVariant& operator=(const MyVariant& other);

    ~MyVariant();

    real_type toRealType() const;

    bool setValue();

    std::valarray<real_type> toValarray() const;

    DataType getDataType();

    friend MyVariant operator+(const MyVariant& v1, const MyVariant& v2)
    {
        if (v1.dataType==NUMBER && v2.dataType==NUMBER) {
            return MyVariant(NUMBER, new real_type(v1.toRealType() + v2.toRealType()));
        }
        if (v1.dataType==ARRAY && v2.dataType==ARRAY) {
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=v1.data[i]+v2.data[i];
            }
            return MyVariant(ARRAY, result, v1.size);
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant operator-(const MyVariant& v1, const MyVariant& v2)
    {
        if (v1.dataType==NUMBER && v2.dataType==NUMBER) {
            return MyVariant(NUMBER, new real_type(v1.toRealType() - v2.toRealType()));
        }
        if (v1.dataType==ARRAY && v2.dataType==ARRAY) {
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=v1.data[i]-v2.data[i];
            }
            return MyVariant(ARRAY, result,v1.size);
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant operator*(const MyVariant& v1, const MyVariant& v2)
    {
        if (v1.dataType==NUMBER && v2.dataType==NUMBER) {
            return MyVariant(NUMBER, new real_type(v1.toRealType() * v2.toRealType()));
        }
        if (v1.dataType==ARRAY && v2.dataType==NUMBER) {
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=v1.data[i]*(*v2.data);
            }
            return MyVariant(ARRAY, result,v1.size);
        }
        if (v1.dataType==NUMBER && v2.dataType==ARRAY) {
            real_type* result = new real_type[v2.size];
            for (int i = 0; i < v2.size; ++i) {
                result[i]=v2.data[i]*(*v1.data);
            }
            return MyVariant(ARRAY,result,v2.size);
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant operator/(const MyVariant& v1, const MyVariant& v2)
    {
        if (v1.dataType==NUMBER && v2.dataType==NUMBER) {
            return MyVariant(NUMBER, new real_type(v1.toRealType() / v2.toRealType()));
        }

        if (v1.dataType==ARRAY && v2.dataType==NUMBER) {
            real_type* result = new real_type[v1.size];
            for (int i = 0; i < v1.size; ++i) {
                result[i]=v1.data[i] / (*v2.data);
            }
            return MyVariant(ARRAY, result,v1.size);
        }
        return MyVariant(TYPEERROR);
    }

    friend MyVariant operator^(const MyVariant& v1, const MyVariant& v2)
    {
        if (v1.dataType==NUMBER && v2.dataType==NUMBER) {
            return MyVariant(NUMBER, new real_type(pow(v1.toRealType(),  v2.toRealType())));
        }
        return MyVariant(TYPEERROR);
    }

    QString getName()
    {
        return name;
    }

    QString print() const
    {
        QString result = name + " = ";

        switch (dataType)
        {
        case NUMBER:
            result += QString::number(double(*data));
            break;
        case ARRAY:
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
            result += "NULL";
        }
        result += "\n";
        return result;
    }

    real_type at(int n, bool* ok = nullptr)
    {
        if (dataType!=ARRAY || n>=size || n<0) {
            *ok=false;
            return 0.0;
        }
        return data[n];
    }

    real_type* atPtr(int n, bool* ok = nullptr)
    {
        if (dataType!=ARRAY || n>=size || n<0) {
            if (ok!=nullptr) *ok=false;
            return nullptr;
        }
        return data+n;
    }
};

#endif // MYVARIANT_H

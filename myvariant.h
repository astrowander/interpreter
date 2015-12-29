#ifndef MYVARIANT_H
#define MYVARIANT_H
#include "definitions.h"

enum DataType {VOID, INTEGER, REAL, TYPEERROR};

class MyVariant
{
private:
    DataType dataType;
    int intData;
    real_type realData;

    MyVariant* elements[1024];  
    int size;
    bool v_isArray;
    int dimension;

public:
    MyVariant(DataType m_dataType=VOID) : dataType(m_dataType), intData(0), realData(0), size(-1), v_isArray(false) {}
    MyVariant(real_type m_data);
    MyVariant(int m_data);
    MyVariant(MyVariant* m_elements[], int m_size);
    MyVariant (MyVariant const& other);
    MyVariant& operator=(const MyVariant& other);
    ~MyVariant();

    inline real_type toRealType() const
    {
        return realData;
    }
    inline int toInt() const
    {
        return intData;
    }
    std::valarray<real_type> toRealValarray() const;

    inline DataType getDataType() const
    {
        return dataType;
    }

    friend void castTo(MyVariant* m_v1, DataType otherDataType)
    {
        //m_v1->dataType = otherDataType;

        if (m_v1->v_isArray)
        {
            for(int i=0; i<m_v1->size; ++i)
            {
                castTo(m_v1->elements[i], otherDataType);
            }
            return;
        }

        switch (otherDataType)
        {
        case REAL:
            switch (m_v1->dataType)
            {
            case INTEGER:
                m_v1->realData = real_type(m_v1->intData);
                m_v1->dataType = REAL;
                return;
            default:
                reportError("Type casting error");
                m_v1->dataType = TYPEERROR;
                return;
            }
        case INTEGER:
            switch (m_v1->dataType)
            {
            case REAL:
                m_v1->intData = (int) m_v1->realData;
                m_v1->dataType = INTEGER;
                return;
            default:
                reportError("Type casting error");
                m_v1->dataType = TYPEERROR;
                return;
            }
        default:
            reportError("Type casting error");
            m_v1->dataType = TYPEERROR;
            return;
        }
    }

    friend void autoCast(MyVariant* v1, MyVariant* v2)
    {
        if (v1->dataType == REAL && v2->dataType == REAL || v1->dataType ==INTEGER && v2->dataType == INTEGER)
            return;

        if (v1->dataType == REAL && v2->dataType ==INTEGER) {
            castTo(v2, REAL);
            return;
        }
        if (v1->dataType ==INTEGER && v2->dataType == REAL)
        {
            castTo(v1, REAL);
            return;
        }
        v1->dataType = v2->dataType = TYPEERROR;
    }

    friend bool isEqual(MyVariant* result, MyVariant* v1, MyVariant* v2)
    {
        autoCast(v1,v2);
        result->dataType = INTEGER;
        result->v_isArray = false;
        if (!v1->v_isArray && !v2->v_isArray) {
            switch (v1->dataType)
            {
            case REAL:
                result->intData = v1->realData == v2->realData;
                break;
            case INTEGER:
                result->intData = v1->intData == v2->intData;
                break;
            default:
                result->dataType = TYPEERROR;
            }
            return true;
        }

        if (v1->v_isArray && v2->v_isArray) {
            if (v1->size!=v2->size) {
                reportError("Dimensions of array does not match");
                return false;
            }
            bool isAllTrue = true;
            for (int i=0; i< v1->getSize(); ++i) {
                if (!isEqual(result->elements[i], v1->elements[i], v2->elements[i])) return false;
                isAllTrue&= result->elements[i]->intData;
            }
            result->intData = int(isAllTrue);
            return true;
        }
        return false;
    }

    friend bool isLess(MyVariant* result, MyVariant* v1, MyVariant* v2)
    {
        autoCast(v1,v2);
        result->dataType = INTEGER;
        result->v_isArray = false;
        if (!v1->v_isArray && !v2->v_isArray) {
            switch (v1->dataType)
            {
            case REAL:
                result->intData = v1->realData < v2->realData;
                break;
            case INTEGER:
                result->intData = v1->intData < v2->intData;
                break;
            default:
                result->dataType = TYPEERROR;
            }
            return true;
        }
        reportError("Arrays could not be compared, except operators '==' and '!=' ");
        return false;
    }

    friend bool isMore(MyVariant* result, MyVariant* v1, MyVariant* v2)
    {
       if (!isLess(result, v2,v1)) return false;
       return true;
    }

    friend bool isLessEqual(MyVariant* result, MyVariant*v1, MyVariant* v2)
    {
        if (!isMore(result, v1, v2)) return false;
        result->intData = !result->intData;
        return true;
    }

    friend bool isMoreEqual(MyVariant* result, MyVariant*v1, MyVariant* v2)
    {
        if (!isLessEqual(result, v2,v1)) return false;
        return true;
    }

    friend bool isNotEqual(MyVariant* result, MyVariant*v1, MyVariant* v2)
    {
        if (!isEqual(result,v1, v2)) return false;
        result->intData = !result->intData;
        return true;
    }

    friend bool doAnd(MyVariant* result, MyVariant*v1, MyVariant* v2)
    {
        if (v1->v_isArray or v2->v_isArray or v1->dataType!=INTEGER or v2->dataType!=INTEGER) {
            reportError("Logical operations could be applied only for integer numbers (not arrays)");
            return false;
        }
        result->v_isArray = false;
        result->size = -1;
        result->dataType = INTEGER;
        result->intData = v1->intData && v2->intData;
        return true;
    }

    friend bool doOr(MyVariant* result, MyVariant*v1, MyVariant* v2)
    {
        if (v1->v_isArray or v2->v_isArray or v1->dataType!=INTEGER or v2->dataType!=INTEGER) {
            reportError("Logical operations could be applied only for integer numbers (not arrays)");
            return false;
        }
        result->v_isArray = false;
        result->size = -1;
        result->dataType = INTEGER;
        result->intData = v1->intData || v2->intData;
        return true;
    }

    friend bool doXor(MyVariant* result, MyVariant*v1, MyVariant* v2)
    {
        if (v1->v_isArray or v2->v_isArray or v1->dataType!=INTEGER or v2->dataType!=INTEGER) {
            reportError("Logical operations could be applied only for integer numbers (not arrays)");
            return false;
        }
        *result = MyVariant(int(!v1->intData != !v2->intData));
        return true;
    }

    friend bool doNot(MyVariant* result, MyVariant*v1)
    {
        if (v1->v_isArray or v1->dataType!=INTEGER ) {
            reportError("Logical operations could be applied only to integer numbers (not arrays)");
            return false;
        }
        result->v_isArray = false;
        result->size = -1;
        result->dataType = INTEGER;
        result->intData = !v1->intData;
        return true;
    }

    friend bool add(MyVariant* result, MyVariant* v1, MyVariant* v2)
    {
        autoCast(v1,v2);
        result->dataType = v1->dataType;
        if (!v1->v_isArray && !v2->v_isArray) {
            result->v_isArray = false;

            switch (v1->dataType)
            {
            case REAL:
                result->realData = v1->realData + v2->realData;
                break;
            case INTEGER:
                result->intData = v1->intData + v2->intData;
                break;
            default:
                result->dataType = TYPEERROR;
            }
            return true;
        }

        if (v1->v_isArray && v2->v_isArray) {
            result->v_isArray = true;
            if (v1->size!=v2->size) {
                reportError("Dimensions of array does not match");
                return false;
            }
            for (int i=0; i<  i< v1->getSize(); ++i) {
                if (!add(result->elements[i], v1->elements[i], v2->elements[i])) return false;
            }
            return true;
        }
        return false;
    }

    friend bool substract(MyVariant* result, MyVariant* v1, MyVariant* v2)
    {
        autoCast(v1,v2);
        result->dataType = v1->dataType;
        if (!v1->v_isArray && !v2->v_isArray) {
            result->v_isArray = false;
            switch (v1->dataType)
            {
            case REAL:
                result->realData = v1->realData - v2->realData;
                break;
            case INTEGER:
                result->intData = v1->intData - v2->intData;
                break;
            default:
                result->dataType = TYPEERROR;
            }
            return true;
        }

        if (v1->v_isArray && v2->v_isArray) {
            result->v_isArray = true;
            if (v1->size!=v2->size) {
                reportError("Dimensions of array does not match");
                return false;
            }
            for (int i=0; i< v1->getSize(); ++i) {
                if (!substract(result->elements[i], v1->elements[i], v2->elements[i])) return false;
            }
            return true;
        }
        return false;
    }

    friend bool unaryMinus(MyVariant* result, MyVariant* v1)
    {
        result->dataType = v1->dataType;
        if (!v1->v_isArray) {
            result->v_isArray = false;
            switch (v1->dataType)
            {
            case REAL:
                result->realData = -v1->realData;
                break;
            case INTEGER:
                result->intData = -v1->intData;
                break;
            default:
                result->dataType = TYPEERROR;
            }
            return true;
        }
        //else
        result->v_isArray = true;
        for (int i=0; i< v1->getSize(); ++i) {
            if (!unaryMinus(result->elements[i], v1->elements[i])) return false;
        }
    }

    friend bool multiply(MyVariant* result, MyVariant* v1, MyVariant* v2)
    {
        autoCast(v1,v2);
        result->dataType = v1->dataType;
        if (!v1->v_isArray && !v2->v_isArray) {
            result->v_isArray = false;
            result->dataType = v1->dataType;
            switch (v1->dataType)
            {
            case REAL:
                result->realData = v1->realData * v2->realData;
                break;
            case INTEGER:
                result->intData = v1->intData * v2->intData;
                break;
            default:
                result->dataType = TYPEERROR;
            }
            return true;
        }

        if (!v1->v_isArray && v2->v_isArray) {
            result->v_isArray = true;

            for (int i=0; i< v2->size; ++i) {
                if (!multiply(result->elements[i], v1, v2->elements[i])) return false;
            }
            return true;
        }

        if (v1->v_isArray && !v2->v_isArray) {
            result->v_isArray = true;
            for (int i=0; i< v1->size; ++i) {
                if (!multiply(result->elements[i], v1->elements[i], v2)) return false;
            }
            return true;
        }
        reportError("Type mysmatch");
        return false;
    }

    friend bool divide(MyVariant* result, MyVariant* v1, MyVariant* v2)
    {
        autoCast(v1,v2);
        result->dataType = v1->dataType;
        if (!v1->v_isArray && !v2->v_isArray) {
            result->v_isArray = false;

            switch (v1->dataType)
            {
            case REAL:
                result->realData = v1->realData / v2->realData;
                break;
            case INTEGER:
                result->intData = v1->intData / v2->intData;
                break;
            default:
                result->dataType = TYPEERROR;
            }
            return true;
        }

        if (v1->v_isArray && !v2->v_isArray) {
            result->v_isArray = true;
            for (int i=0; i< v1->size; ++i) {
                if (!divide(result->elements[i], v1->elements[i], v2)) return false;
            }
            return true;
        }
        reportError("Type mysmatch");
        return false;
    }

    friend bool power(MyVariant* result, MyVariant* v1, MyVariant* v2)
    {
        result->dataType = v1->dataType;
        autoCast(v1,v2);
        if (!v1->v_isArray && !v2->v_isArray) {
            result->v_isArray = false;
            switch (v1->dataType)
            {
            case REAL:
                result->realData = pow(v1->realData, v2->realData);
                break;
            case INTEGER:
                result->intData = pow(v1->intData, v2->intData);
                break;
            default:
                result->dataType = TYPEERROR;
            }
            return true;
        }
        reportError("Type mysmatch");
        return false;
    }

    friend bool mySqrt(MyVariant* result, MyVariant* v1)
    {
        result->dataType = v1->dataType;
        if (!v1->isArray()) {
            result->v_isArray = false;

            switch (v1->dataType)
            {
            case REAL:
                result->realData = sqrt(v1->realData);
                break;
            case INTEGER:
                result->intData = sqrt(v1->intData);
                break;
            default:
                result->dataType = TYPEERROR;
                reportError("Type mysmatch");
                return false;
            }
            return true;
        }
        //else
        result->v_isArray = true;

        for (int i=0; i<v1->size; ++i) {
            if (!mySqrt(result->elements[i], v1->elements[i])) return false;
        }
        return true;
    }

    friend bool myLog(MyVariant* result, MyVariant* v1)
    {
        result->dataType = v1->dataType;
        if (!v1->isArray()) {
            result->v_isArray = false;

            switch (v1->dataType)
            {
            case REAL:
                result->realData = log(v1->realData);
                break;
            case INTEGER:
                result->intData = log(v1->intData);
                break;
            default:
                result->dataType = TYPEERROR;
                reportError("Type mysmatch");
                return false;
            }
            return true;
        }
        //else
        result->v_isArray = true;

        for (int i=0; i<v1->size; ++i) {
            if (!myLog(result->elements[i], v1->elements[i])) return false;
        }
        return true;
    }

    friend bool myAbs(MyVariant* result, MyVariant* v1)
    {
        result->dataType = v1->dataType;
        if (!v1->isArray()) {
            result->v_isArray = false;

            switch (v1->dataType)
            {
            case REAL:
                result->realData = fabs(v1->realData);
                break;
            case INTEGER:
                result->intData = abs(v1->intData);
                break;
            default:
                result->dataType = TYPEERROR;
                reportError("Type mysmatch");
                return false;
            }
            return true;
        }
        //else
        result->v_isArray = true;

        for (int i=0; i<v1->size; ++i) {
            if (!myAbs(result->elements[i], v1->elements[i])) return false;
        }
        return true;
    }

    void makePrintString(QString &result) const;
    void print() const;

    inline int getSize() const
    {
        return size;
    }

    bool isArray() const
    {
        return v_isArray;
    }

    inline void setSize(int s) {
        size = s;
    }

    inline void setIsArray(bool arg)
    {
            size=0;
            v_isArray = arg;
    }

    MyVariant*& getElement(int n)
    {
        return elements[n];
    }

    void setElement(MyVariant* elementPtr, int index)
    {
        elements[index] = elementPtr;
    }

    void reset();

    void addElement(MyVariant* elementPtr)
    {
        elements[size++] = elementPtr;
    }

    void setDataType(DataType m_dataType)
    {
        dataType = m_dataType;
    }

    void setData(int data)
    {
        intData = data;
    }

    void setData(real_type data)
    {
        realData = data;
    }

};

class MyCache
{
private:
    MyVariant cache[1024];
    int iterator;
public:
    MyCache() : iterator(-1) {}

    bool push(const MyVariant &toPush)
    {
        if (iterator==1023) return false;
        cache[++iterator] = toPush;
        return true;
    }

    MyVariant& last()
    {
        return cache[iterator];
    }

    bool removeLast()
    {
        if (iterator<0) return false;
        cache[iterator--].reset();
        return true;
    }

    bool allocate(MyVariant* &ptr)
    {
        if (iterator==1023) return false;
        ptr = &cache[++iterator];
        return true;
    }
};
#endif // MYVARIANT_H

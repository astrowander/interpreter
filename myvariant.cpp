#include "myvariant.h"

MyVariant::MyVariant(DataType m_dataType, real_type *m_data, int m_size, const QString& m_name) : dataType(m_dataType), size(m_size), name(m_name)
{
    switch (dataType) {
    case REAL:
        data = new real_type(*m_data);
        //delete m_data;
        break;
    case REALARRAY:
        data = new real_type[size];
        std::copy(m_data, m_data+m_size, data);
        //if (m_data!=nullptr) delete[] m_data;
        break;
    case TYPEERROR:
        data = nullptr;
        break;
    }
    //std::cout << "Constructor of variable is called" << std::endl;
}

MyVariant::MyVariant(const MyVariant &other)
{
    dataType= other.dataType;
    size = other.size;
    name=other.name;
    switch (other.dataType) {
    case REAL:
        data = new  real_type;
        *data = *other.data;
        break;
    case REALARRAY:
        data = new real_type[other.size];
        std::copy(other.data, other.data+other.size, data);
        break;
    }
   // std::cout << "copy construction\n";
}

MyVariant &MyVariant::operator=(const MyVariant &other)
{
    dataType= other.dataType;
    size = other.size;
    //

    switch (other.dataType) {
    case REAL:
        data = new  real_type;
        *data = *other.data;
        break;
    case REALARRAY:
        data = new real_type[other.size];
        std::copy(other.data, other.data+other.size, data);
        break;
    }
   // std::cout << "assignment operator\n";
}

MyVariant::~MyVariant() {
    //std::cout << "Destructor of variable is called" << std::endl;
    if (dataType==REAL && data!=nullptr) {
        delete data;
    }
    else if (dataType == REALARRAY) { //dataType == ARRAY
        delete[] data;
    }
}

real_type MyVariant::toRealType() const
{
    return *data;
}

bool MyVariant::setValue()
{
    if (dataType==REAL) {

    }
}

std::valarray<real_type> MyVariant::toValarray() const
{
    return std::valarray<real_type>(data,size);
}

DataType MyVariant::getDataType() const
{
    return dataType;
}


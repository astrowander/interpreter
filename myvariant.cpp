#include "myvariant.h"

MyVariant::MyVariant(real_type *m_data, int m_size) : size(m_size)
{
    v_isArray = !(size==-1);
    dataType = REAL;
    if (!v_isArray) {
        data = new real_type(*m_data);
        return;
    }
    //else
    for (int i=0; i<size; ++i) {
        elements.append(new MyVariant(m_data+i));
    }

   // std::cout << "Constructor of REAL MyVariant is called" << std::endl;
}

MyVariant::MyVariant(int *m_data, int m_size) : size(m_size)
{
    v_isArray = !(size==-1);
    dataType = INTEGER;
    //elements.clear();
    if (!v_isArray) {
        data = new int(*m_data);
        return;
    }
    //else
    for (int i=0; i<size; ++i) {
        elements.append(new MyVariant(m_data+i));
    }

   // std::cout << "Constructor of Integer MyVariant is called" << std::endl;
}

MyVariant::MyVariant(const MyVariant &other)
{

    dataType = other.dataType;
    size = other.size;    
    v_isArray = other.v_isArray;

    switch (other.dataType) {
    case REAL:
        if (!v_isArray) {
            data = new  real_type;
            *(real_type*)data = *(real_type*)other.data;
            break;
        }
        for (int i=0; i<size; ++i) {
            elements.append(new MyVariant((real_type*)other.elements[i]->data));
        }
        break;

    case INTEGER:
        if (!v_isArray) {
            data = new  int;
            *(int*) data = *(int*) other.data;
            break;
        }
        for (int i=0; i<size; ++i) {
            elements.append(new MyVariant((int*)other.elements[i]->data));
        }
        break;

    default:
        data = nullptr;
        //dataType = other.dataType;
       // size = other.size;
    }
   // std::cout << "copy construction\n";
}

MyVariant &MyVariant::operator=(const MyVariant &other)
{
    dataType = other.dataType;
    size = other.size;
    v_isArray = other.v_isArray;
    elements.clear();

    switch (other.dataType) {
    case REAL:
        if (!v_isArray) {
            if (data!=nullptr) data = new  real_type;
            *(real_type*)data = *(real_type*)other.data;
            break;
        }
        for (int i=0; i<size; ++i) {
            elements.append(new MyVariant((real_type*)other.elements[i]->data));
        }
        break;

    case INTEGER:
        if (!v_isArray) {

            if (data==nullptr) data = new  int;
            *(int*) data = *(int*) other.data;
            break;
        }
        for (int i=0; i<size; ++i) {
            elements.append(new MyVariant((int*)other.elements[i]->data));
        }
        break;
    default:
        data = nullptr;

    }

   // std::cout << "assignment operator\n";
}

MyVariant::~MyVariant() {
  //  std::cout << "Destructor of MyVariant is called" << std::endl;
    if (data!=nullptr && !v_isArray) {
        delete data;
    }
    else if (v_isArray) {
        for (int i=0; i<size; ++i)
            delete elements[i];
        elements.clear();
    }
}

real_type MyVariant::toRealType() const
{
    if (data!=nullptr)
        return *(real_type*)data;

    return elements.at(0)->toRealType();
}

int MyVariant::toInt() const
{
    if (data!=nullptr)
        return *(int*)data;

    return elements.at(0)->toInt();
}

std::valarray<real_type> MyVariant::toRealValarray() const
{
    std::valarray<real_type> result(size);
    for (int i=0; i<size; ++i) {
        result[i] = elements.at(i)->toRealType();
    }
}

DataType MyVariant::getDataType() const
{
    return dataType;
}

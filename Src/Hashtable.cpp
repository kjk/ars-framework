
#include "Hashtable.hpp"

using namespace ArsLexis;

Hashtable::Hashtable()
{
    entity_ = NULL;
    value_ = NULL;
    counter_ = 0;
    max_ = 0;
}

#define CACHE_ADD 8
void Hashtable::put(String entity, String value)
{
    int i;
    int size;
    String tempS;
    if(counter_ == max_)
    {
        char_t **tempE;
        char_t **tempV;
        tempE = (char_t **) new (unsigned long[max_ + CACHE_ADD]);
        tempV = (char_t **) new (unsigned long[max_ + CACHE_ADD]);
       
        for(i=0;i<max_;i++)
        {
             tempE[i] = entity_[i];
             tempV[i] = value_[i];
        }
        max_ += CACHE_ADD;

        if(entity_!=NULL)
            delete entity_;
        if(value_!=NULL)
            delete value_;
     
        entity_ = tempE;
        value_ = tempV;

        for(i=i;i<max_;i++)
        {
            entity_[i] = NULL;
            value_[i] = NULL;
        }
    }

    for(i=0;i<counter_;i++)
    {
        tempS = entity_[i];
        if(tempS == entity)
        {
            delete (value_[i]);
            size = value.size();
            value_[i] = new (char_t[size+1]);
            for(int j=0;j<size;j++)
                (value_[i])[j] = value[j];
            (value_[i])[size] = 0;
            return;
        }
    }

    i = counter_;
    counter_++;

    size = value.size();
    value_[i] = new (char_t[size+1]);
    for(int j=0;j<size;j++)
        (value_[i])[j] = value[j];
    (value_[i])[size] = 0;

    size = entity.size();
    entity_[i] = new (char_t[size+1]);
    for(int j=0;j<size;j++)
        (entity_[i])[j] = entity[j];
    (entity_[i])[size] = 0;
}

String Hashtable::get(String code)
{
    String tempS;
    int i;
    for(i=0;i<counter_;i++)
    {
        tempS = entity_[i];
        if(tempS == code)
            return (value_[i]);
    }
    //not found... return null string
    tempS = "";
    return tempS;
}

void Hashtable::Dispose()
{
    int i;
    for(i=0;i<counter_;i++)
    {
        delete (entity_[i]);
        delete (value_[i]);
    }
    delete (entity_);
    delete (value_);
    value_ = NULL;
    entity_ = NULL;
    max_ = 0;
    counter_ = 0;
}
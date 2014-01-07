#include "field.h"

namespace q5250 {

Field::Field(unsigned short len) :
    fieldLength(len)
{
}

unsigned short Field::length() const
{
    return fieldLength;
}

} // namespace q5250

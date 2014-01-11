#include "field.h"

#include "cursor.h"

namespace q5250 {

static const unsigned short BYPASS_FIELD_MASK        = 0x2000;   // Bit 13
static const unsigned short DUPLICATION_ALLOWED_MASK = 0x1000;   // Bit 12
static const unsigned short MODIFIED_MASK            = 0x800;    // Bit 11    1 = Field was modified (MDT)
static const unsigned short FIELD_EDIT_MASK          = 0x700;    // Bit 10-8
static const unsigned short AUTO_ENTER_MASK          = 0x80;     // Bit 7
static const unsigned short FIELD_EXIT_MASK          = 0x40;     // Bit 6
static const unsigned short UPPERCASE_MASK           = 0x20;     // Bit 5     1 = Translate lowercase to uppercase
static const unsigned short MANDATORY_MASK           = 0x8;      // Bit 3
static const unsigned short ADJUSTMENT_MASK          = 0x7;      // Bit 2-0

Field::Field() :
    fieldLength(0),
    fieldFormat(0),
    fieldAttribute(0)
{
}

void Field::setPosition(unsigned int column, unsigned int row)
{
    startPosition.setX(column);
    startPosition.setY(row);
}

void Field::gotoField()
{
    Cursor cursor;
    cursor.setPosition(startPosition.x(), startPosition.y());
}

void Field::setLength(unsigned short length)
{
    fieldLength = length;
}

unsigned short Field::length() const
{
    return fieldLength;
}

void Field::setFieldFormat(unsigned short format)
{
    fieldFormat = format;
}

bool Field::isBypassField() const
{
    return fieldFormat & BYPASS_FIELD_MASK;
}

bool Field::isDuplicationAllowed() const
{
    return fieldFormat & DUPLICATION_ALLOWED_MASK;
}

bool Field::isModified() const
{
    return fieldFormat & MODIFIED_MASK;
}

Field::FieldEdit Field::fieldEdit() const
{
    return (FieldEdit)(fieldFormat & FIELD_EDIT_MASK);
}

bool Field::isAutoEnterEnabled() const
{
    return fieldFormat & AUTO_ENTER_MASK;
}

bool Field::isFieldExitKeyRequired() const
{
    return fieldFormat & FIELD_EXIT_MASK;
}

void Field::setAttribute(unsigned char attribute)
{
    fieldAttribute = attribute;
}

unsigned char Field::attribute() const
{
    return fieldAttribute;
}

} // namespace q5250

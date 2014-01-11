#ifndef Q5250_FIELD_H
#define Q5250_FIELD_H

#include "q5250_global.h"

#include <QMetaType>
#include <QPoint>

namespace q5250 {

class Q5250SHARED_EXPORT Field
{
public:
    enum FieldEdit {
        AlphaShift = 0,
        AlphaOnly,
        NumericShift,
        NumericOnly,
        KatakanaShift,
        DigitsOnly,
        IOFeatureInput,
        SignedNumeric
    };

    Field();

    void setPosition(unsigned int column, unsigned int row);
    void gotoField();

    void setLength(unsigned short length);
    unsigned short length() const;

    void setFieldFormat(unsigned short format);
    bool isBypassField() const;
    bool isDuplicationAllowed() const;
    bool isModified() const;
    FieldEdit fieldEdit() const;
    bool isAutoEnterEnabled() const;
    bool isFieldExitKeyRequired() const;

    void setAttribute(unsigned char attribute);
    unsigned char attribute() const;

private:
    unsigned short fieldLength;
    unsigned short fieldFormat;
    unsigned char fieldAttribute;
    QPoint startPosition;
};

} // namespace q5250

Q_DECLARE_METATYPE(q5250::Field)

#endif // Q5250_FIELD_H

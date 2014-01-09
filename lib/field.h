#ifndef Q5250_FIELD_H
#define Q5250_FIELD_H

#include "q5250_global.h"

#include <QMetaType>

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
};

} // namespace q5250

Q_DECLARE_METATYPE(q5250::Field)

#endif // Q5250_FIELD_H

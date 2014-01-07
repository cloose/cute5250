#ifndef Q5250_FIELD_H
#define Q5250_FIELD_H

namespace q5250 {

class Field
{
public:
    explicit Field(unsigned short len);

    unsigned short length() const;

private:
    unsigned short fieldLength;
};

} // namespace q5250

#endif // Q5250_FIELD_H

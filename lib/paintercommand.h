#ifndef Q5250_PAINTERCOMMAND_H
#define Q5250_PAINTERCOMMAND_H

class QPainter;

namespace q5250 {

class PainterCommand
{
public:
    virtual ~PainterCommand() {}

    virtual void execute(QPainter *p) = 0;
};

} // namespace q5250

#endif // Q5250_PAINTERCOMMAND_H

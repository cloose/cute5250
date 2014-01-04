#ifndef Q5250_PAINTERCOMMAND_H
#define Q5250_PAINTERCOMMAND_H

#include <QRect>
class QPainter;

namespace q5250 {

class PainterCommand
{
public:
    explicit PainterCommand(const QRect &rect) : widgetRect(rect) {}
    virtual ~PainterCommand() {}

    virtual void execute(QPainter *p) = 0;

protected:
    QRect widgetRect;
};

} // namespace q5250

#endif // Q5250_PAINTERCOMMAND_H

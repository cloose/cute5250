#ifndef Q5250_WRITETODISPLAYPARSER_H
#define Q5250_WRITETODISPLAYPARSER_H

#include <QObject>

namespace q5250 {

class GeneralDataStream;

class WriteToDisplayParser : public QObject
{
    Q_OBJECT

public:
    explicit WriteToDisplayParser(QObject *parent = 0);

    void parse(GeneralDataStream &stream);

signals:
    void positionCursor(uint col, uint row);
};

} // namespace q5250

#endif // Q5250_WRITETODISPLAYPARSER_H

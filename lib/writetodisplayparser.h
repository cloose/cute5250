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
    void displayText(const QByteArray &ebcdicText);
    void setDisplayAttribute(const unsigned char attribute);
    void repeatCharacter(uint col, uint row, uchar character);

private:
    bool isDataCharacter(const unsigned char byte);
    bool isDisplayAttribute(const unsigned char byte);
};

} // namespace q5250

#endif // Q5250_WRITETODISPLAYPARSER_H

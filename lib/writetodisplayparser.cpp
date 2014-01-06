#include "writetodisplayparser.h"

#include <QDebug>
#include <QTextCodec>

#include "generaldatastream.h"

namespace q5250 {

// see documentation: http://publibfp.boulder.ibm.com/cgi-bin/bookmgr/BOOKS/co2e2001/CCONTENTS

namespace Order {

enum {
    SOH  = 0x01,    // Start of header
    RA   = 0x02,
    EA   = 0x03,
    TD   = 0x10,
    SBA  = 0x11,
    WEA  = 0x12,
    IC   = 0x13,
    MC   = 0x14,
    WDSF = 0x15,
    SF   = 0x1d
};

}

WriteToDisplayParser::WriteToDisplayParser(QObject *parent) :
    QObject(parent)
{
    codec = QTextCodec::codecForName("IBM500");
}

void WriteToDisplayParser::parse(GeneralDataStream &stream)
{
    // get control characters (see 5494 RCU 15.6.1)
    unsigned char cc1 = stream.readByte();
    unsigned char cc2 = stream.readByte();

    while (!stream.atEnd()) {
        unsigned char byte = stream.readByte();

        if (byte == 0x04 /*ESC*/) {
            stream.seekToPreviousByte();
            return;
        }

        switch (byte) {
        case Order::SOH:
            {
                qDebug() << "SERVER --> [GDS:WTD] -- START OF HEADER --";
                unsigned char orderLength = stream.readByte();
                unsigned char flagByte = stream.readByte();
                stream.readByte();  // reserved
                unsigned char resqField = stream.readByte();
                unsigned char errorRow = stream.readByte();

                for (int i = 4; i < orderLength; ++i) {
                    unsigned char commandKey = stream.readByte();
                }
            }
            break;

        case Order::RA:
            {
                qDebug() << "SERVER --> [GDS:WTD] -- REPEAT TO ADDRESS --";
                unsigned char row = stream.readByte();
                unsigned char column = stream.readByte();
                unsigned char character = stream.readByte();
            }
            break;

        case Order::EA:
            {
                qDebug() << "SERVER --> [GDS:WTD] -- ERASE TO ADDRESS --";

            }
            break;

        case Order::TD:
            {
                qDebug() << "SERVER --> [GDS:WTD] -- TRANSPARENT DATA --";

            }
            break;

        case Order::SBA:
            {
                qDebug() << "SERVER --> [GDS:WTD] -- SET BUFFER ADDRESS --";
                unsigned char row = stream.readByte();
                unsigned char column = stream.readByte();
                emit positionCursor(column, row);
            }
            break;

        case Order::WEA:
            {
                qDebug() << "SERVER --> [GDS:WTD] -- WRITE EXTENDED ATTRIBUTE --";

            }
            break;

        case Order::IC:
            {
                qDebug() << "SERVER --> [GDS:WTD] -- INSERT CURSOR --";

            }
            break;

        case Order::MC:
            {
                qDebug() << "SERVER --> [GDS:WTD] -- MOVE CURSOR --";

            }
            break;

        case Order::WDSF:
            {
                qDebug() << "SERVER --> [GDS:WTD] -- WRITE TO DISPLAY STRUCTURED FIELD --";

            }
            break;

        case Order::SF:
            {
                qDebug() << "SERVER --> [GDS:WTD] -- START OF FIELD --";
                unsigned char ffw0 = stream.readByte();
                qDebug() << "field format 0" << ffw0 << QString::number(ffw0, 16) << QString::number(ffw0, 2);
                if (ffw0 & 0x40) {
                    qDebug() << "Bit 14-15: FFW";
                    qDebug() << "Bit 13: bypass field" << ((ffw0 & 0x20) == 0x20);
                    qDebug() << "Bit 12: Duplication allowed" << ((ffw0 & 0x10) == 0x10);
                    qDebug() << "Bit 11: Field was modified" << ((ffw0 & 0x08) == 0x08);

                    unsigned char ffw1 = stream.readByte();
                    qDebug() << "field format 1" << ffw1 << QString::number(ffw1, 16) << QString::number(ffw1, 2);

                    unsigned char fcw0 = stream.readByte();
                    qDebug() << "field control 0" << fcw0 << QString::number(fcw0, 16) << QString::number(fcw0, 2);

                    if ((fcw0 & 0xe0) == 0x20) {
                        qDebug() << "ATTRIBUTE";
                    } else {
                        unsigned char fcw1 = stream.readByte();
                        qDebug() << "field control 1" << fcw1 << QString::number(fcw1, 16) << QString::number(fcw1, 2);

                        if (fcw0 == 0x80) {
                            qDebug() << "resequence";
                        }
                    }
                } else {
                    qDebug() << "ATTRIBUTE";
                }

                unsigned char fl0 = stream.readByte();
                unsigned char fl1 = stream.readByte();
                unsigned short fieldLength = (fl0 << 8) | fl1;
                qDebug() << "field length" << fieldLength;
            }
            break;

        default:
            QByteArray encodedByte;
            encodedByte.append(byte);

            qDebug() << "SERVER --> [GDS:WTD]" << QString::number(byte, 16) << (char)byte
                        << codec->toUnicode(encodedByte);
            break;
        }
    }
}

} // namespace q5250

#include "writetodisplayparser.h"

#include <QDebug>

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

            }
            break;

        default:
            break;
        }
    }
}

} // namespace q5250

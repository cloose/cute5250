/*
 * Copyright (c) 2013, Christian Loose
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "generaldatastream.h"

#include <QDebug>
#include <QDataStream>

namespace q5250 {

class GeneralDataStream::Private
{
public:
    struct Header
    {
        quint16 recordLength;
        quint16 recordType;
        quint16 reservedBytes;
        quint8  varHdrLen;
        quint16 flags;
        quint8  opcode;
    };

    explicit Private(const QByteArray &data);
    ~Private();

    void readHeader();

    QDataStream *dataStream;
    Header header;
};

GeneralDataStream::Private::Private(const QByteArray &data) :
    dataStream(new QDataStream(data))
{
    readHeader();

    qDebug() << "---GDS HEADER---"
             << QString::number(header.recordLength, 16)
             << QString::number(header.recordType, 16)
             << QString::number(header.varHdrLen, 16)
             << QString::number(header.flags, 16)
             << QString::number(header.opcode, 16);
}

GeneralDataStream::Private::~Private()
{
    delete dataStream;
}

void GeneralDataStream::Private::readHeader()
{
    *dataStream >> header.recordLength
                >> header.recordType
                >> header.reservedBytes
                >> header.varHdrLen
                >> header.flags
                >> header.opcode;
}

GeneralDataStream::GeneralDataStream(const QByteArray &data) :
    d(new Private(data))
{
}

GeneralDataStream::~GeneralDataStream()
{
}

bool GeneralDataStream::atEnd() const
{
    return d->dataStream->atEnd();
}

unsigned char GeneralDataStream::readByte() const
{
    unsigned char byte;
    *d->dataStream >> byte;
    return byte;
}

void GeneralDataStream::seekToPreviousByte()
{
    d->dataStream->device()->seek(d->dataStream->device()->pos()-1);
}

} // namespace q5250

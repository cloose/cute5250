/*
 * Copyright (c) 2013-2014, Christian Loose
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
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

#include <QDataStream>
#include <QScopedPointer>

namespace q5250 {

class GeneralDataStream::Private
{
public:
    struct Header
    {
        quint16 recordLength;
        quint16 recordType;
        quint16 reservedBytes;
        quint8 varHdrLen;
        quint16 flags;
        quint8 opcode;
    };

    QScopedPointer<QDataStream> stream;
    Header header;
    static const quint16 GdsRecordType{0x12a0};

    explicit Private(const QByteArray &data);
    void readHeader();
    bool isValid() const;

    qint64 currentPosition() const;
    void seekToPosition(qint64 pos);
};

GeneralDataStream::Private::Private(const QByteArray &data) :
    stream(new QDataStream(data))
{
    readHeader();
}

void GeneralDataStream::Private::readHeader()
{
    *stream >> header.recordLength
            >> header.recordType
            >> header.reservedBytes
            >> header.varHdrLen
            >> header.flags
            >> header.opcode;
}

bool GeneralDataStream::Private::isValid() const
{
    return header.recordLength == stream->device()->size() &&
           header.recordType == GdsRecordType;
}

qint64 GeneralDataStream::Private::currentPosition() const
{
    return stream->device()->pos();
}

void GeneralDataStream::Private::seekToPosition(qint64 pos)
{
    stream->device()->seek(pos);
}


GeneralDataStream::GeneralDataStream(const QByteArray &data) :
    d(new Private(data))
{
}

GeneralDataStream::~GeneralDataStream()
{
}

bool GeneralDataStream::isValid() const
{
    return d->isValid();
}

bool GeneralDataStream::atEnd() const
{
    return d->stream->atEnd();
}

unsigned char GeneralDataStream::readByte()
{
    unsigned char byte;
    *d->stream >> byte;
    return byte;
}

void GeneralDataStream::seekToPreviousByte()
{
    d->seekToPosition(d->currentPosition()-1);
}

} // namespace q5250

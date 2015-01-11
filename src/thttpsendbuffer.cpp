/* Copyright (c) 013, AOYAMA Kazuharu
 * All rights reserved.
 *
 * This software may be used and distributed according to the terms of
 * the New BSD License, which is incorporated herein by reference.
 */

#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QHostAddress>
#include <TWebApplication>
#include <THttpResponseHeader>
#include <THttpUtility>
#include "thttpsendbuffer.h"
#include "tsystemglobal.h"


THttpSendBuffer::THttpSendBuffer(const QByteArray &header, const QFileInfo &file, bool autoRemove, const TAccessLogger &logger)
    : arrayBuffer(header), bodyFile(0), fileRemove(autoRemove), accesslogger(logger), startPos(0)
{
    if (file.exists() && file.isFile()) {
        bodyFile = new QFile(file.absoluteFilePath());
        if (!bodyFile->open(QIODevice::ReadOnly)) {
            tSystemWarn("file open failed: %s", qPrintable(file.absoluteFilePath()));
            release();
        }
    }
}


THttpSendBuffer::THttpSendBuffer(const QByteArray &header)
    : arrayBuffer(header), bodyFile(0), fileRemove(false), accesslogger(), startPos(0)
{ }


THttpSendBuffer::THttpSendBuffer(int statusCode, const QHostAddress &address, const QByteArray &method)
    : arrayBuffer(), bodyFile(0), fileRemove(false), accesslogger(), startPos(0)
{
    accesslogger.open();
    accesslogger.setStatusCode(statusCode);
    accesslogger.setTimestamp(QDateTime::currentDateTime());
    accesslogger.setRemoteHost(address.toString().toLatin1());
    accesslogger.setRequest(method);

    THttpResponseHeader header;
    header.setStatusLine(statusCode, THttpUtility::getResponseReasonPhrase(statusCode));
    header.setRawHeader("Server", "TreeFrog server");
    header.setCurrentDate();

    arrayBuffer += header.toByteArray();
}


THttpSendBuffer::~THttpSendBuffer()
{
    release();
}


void THttpSendBuffer::release()
{
    if (bodyFile) {
        if (fileRemove) {
            bodyFile->remove();
        }
        delete bodyFile;
        bodyFile = 0;
    }
}

/*
int THttpSendBuffer::read(char *data, int maxSize)
{
    int ret = 0;
    int len;

    // Read the byte array
    len = qMin(arrayBuffer.length() - arraySentSize, maxSize);
    if (len > 0) {
        memcpy(data, arrayBuffer.constData() + arraySentSize, len);
        arraySentSize += len;
        data += len;
        ret += len;
        maxSize -= len;
    }

    // Read the file
    if (maxSize > 0 && !atEnd()) {
        len = bodyFile->read(data, maxSize);
        if (len < 0) {
            tSystemError("file read error: %s", qPrintable(bodyFile->fileName()));
            release();
        } else {
            ret += len;
        }
    }
    return ret;
}
*/

void *THttpSendBuffer::getData(int &size)
{
    if (Q_UNLIKELY(size <= 0)) {
        tSystemError("Invalid data size. [%s:%d]", __FILE__, __LINE__);
        return 0;
    }

    if (!arrayBuffer.isEmpty()) {
        size = qMin(arrayBuffer.length() - startPos, size);
        return arrayBuffer.data() + startPos;
    }

    if (!bodyFile || bodyFile->atEnd()) {
        size = 0;
        return 0;
    }

    arrayBuffer.reserve(size);
    size = bodyFile->read(arrayBuffer.data(), size);
    if (size < 0) {
        tSystemError("file read error: %s", qPrintable(bodyFile->fileName()));
        size = 0;
        release();
        return 0;
    }

    startPos = 0;
    return arrayBuffer.data();
}


bool THttpSendBuffer::seekData(int pos)
{
    if (Q_UNLIKELY(pos < 0)) {
        return false;
    }

    if (startPos + pos >= arrayBuffer.length()) {
        arrayBuffer.truncate(0);
        startPos = 0;
    } else {
        startPos += pos;
    }
    return true;
}


int THttpSendBuffer::prepend(const char *data, int maxSize)
{
    if (startPos > 0) {
        arrayBuffer.remove(0, startPos);
    }
    arrayBuffer.prepend(data, maxSize);
    startPos = 0;
    return maxSize;
}


bool THttpSendBuffer::atEnd() const
{
    return startPos >= arrayBuffer.length() && (!bodyFile || bodyFile->atEnd());
}

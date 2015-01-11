#ifndef TEPOLLSOCKET_H
#define TEPOLLSOCKET_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QHostAddress>
#include <TGlobal>

class QHostAddress;
class THttpSendBuffer;
class THttpHeader;
class TAccessLogger;
class TAbstractRecvBuffer;


class T_CORE_EXPORT TEpollSocket : public QObject
{
    Q_OBJECT
public:
    TEpollSocket(int socketDescriptor, const QHostAddress &address);
    virtual ~TEpollSocket();

    void close();
    int socketDescriptor() const { return sd; }
    const QHostAddress &clientAddress() const { return clientAddr; }
    quint64 objectId() const { return identifier; }

    virtual bool canReadRequest() { return false; }
    virtual void startWorker() { }

    static TEpollSocket *accept(int listeningSocket);
    static TEpollSocket *create(int socketDescriptor, const QHostAddress &address);

protected:
    int send();
    int recv();
    virtual void *getRecvBuffer(int size) = 0;
    virtual bool seekRecvBuffer(int pos) = 0;
    void setSocketDescpriter(int socketDescriptor);

private:
    int sd;
    quint64 identifier;
    QHostAddress clientAddr;
    QQueue<THttpSendBuffer*> sendBuf;

    static void initBuffer(int socketDescriptor);

    friend class TEpoll;
    Q_DISABLE_COPY(TEpollSocket)
};

#endif // TEPOLLSOCKET_H

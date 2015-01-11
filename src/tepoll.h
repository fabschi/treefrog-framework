#ifndef TEPOLL_H
#define TEPOLL_H

#include <QMap>
#include <TGlobal>
#include <TAtomicQueue>

class QIODevice;
class QByteArray;
class TEpollSocket;
class TAccessLogger;
class TSendData;
struct epoll_event;


class T_CORE_EXPORT TEpoll
{
public:
    ~TEpoll();

    int wait(int timeout);
    bool isPolling() const { return polling; }
    TEpollSocket *next();
    bool canReceive() const;
    bool canSend() const;
    int recv(TEpollSocket *socket) const;
    int send(TEpollSocket *socket) const;

    bool addPoll(TEpollSocket *socket, int events);
    bool modifyPoll(TEpollSocket *socket, int events);
    bool deletePoll(TEpollSocket *socket);
    bool waitSendData(int msec);
    void dispatchSendData();
    void releaseAllPollingSockets();

    // For action workers
    void setSendData(quint64 id, const QByteArray &header, QIODevice *body, bool autoRemove, const TAccessLogger &accessLogger);
    void setDisconnect(quint64 id);
    void setSwitchProtocols(quint64 id, const QByteArray &header, TEpollSocket *target);

    static TEpoll *instance();

protected:
    bool modifyPoll(int fd, int events);

private:
    int epollFd;
    int listenSocket;
    struct epoll_event *events;
    volatile bool polling;
    int numEvents;
    int eventIterator;
    QMap<quint64, TEpollSocket*> pollingSockets;
    TAtomicQueue<TSendData *> sendRequests;

    TEpoll();
    Q_DISABLE_COPY(TEpoll);
};

#endif // TEPOLL_H

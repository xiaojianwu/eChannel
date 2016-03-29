#ifndef UTPMANAGER_H
#define UTPMANAGER_H

#include <QObject>
#include <QHash>
#include <QThread>


#include <utp.h>
#include <utp_types.h>

#include "utphandle.h"

#include <QMutex>

class TransportChannel;

class UTPManager : public QObject
{
    Q_OBJECT

protected:
    UTPManager(QObject *parent = NULL);
    ~UTPManager();

public:
    void init();

    static UTPManager* instance();

    int createUtpSocket(QString peerAddress, TransportChannel* channel);

    void close(QString peerAddress, int connectID);

    void udpRecv(QString peerAddr, const QByteArray& buf);

    void reqUtpPack(QString peerAddr, const QByteArray& buf);

private slots:
    void onUtpReqSend(QString peerAddress, const QByteArray& buf);
    void onUtpRecv(int connectID, const QByteArray& buf);
    void onUtpAccept(QString peerAddress);
    void onUtpConnected(int connectID);

    void onUtpError(QString peerAddr, int connectID, QString errMsg);

    //void onUtpDisconnected(QString peerAddr, int connectID);

    void onUtpIdle(const QString& peerAddr);
    
private:
    static UTPManager* _instance;

private:
    int         m_connID; // 为每个传输通道分配一个ID，自增

    QHash<QString, TransportChannel*> m_hashChannels;   // ip:port    -> channel
    QHash<int, TransportChannel*>     m_hashIDChannels; // connectid  -> channel

    UtpHandle*  m_utpHandle;
    QThread     m_utpThread;

    bool        m_isInited;

    QMutex      m_mutexChannels;
  
};

#endif // UTPMANAGER_H

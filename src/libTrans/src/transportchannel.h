#ifndef TRANSPORTCHANNEL_H
#define TRANSPORTCHANNEL_H

#include <QObject>
#include <QMutex>

#include "libTrans.h"

#include "PJICESocket.h"

class TransportChannel : public Channel
{
    Q_OBJECT
public:
    TransportChannel(QObject *parent = NULL);
    ~TransportChannel();


    void init(QString sessionID, bool isControl, TransferInterface* tinterface);
    
public:
    bool isInitialized();
    int write(const QByteArray &data);
    void close();
    QString getPeerAddress();
    bool isRelay();
    bool isConnected();

public:
    QString getLocalSDP();

    void updateRemoteSDP(QString sdp);

    void connectSuccess(QString peerAddr);

    void conncetError(QString errMsg);

    void onUtpIdle();
public:
    // 线程切换
    void utpSend(const QByteArray& data);
    void utpRecv(const QByteArray& data);

    int  getUtpConnID() {return m_utpConnectID;}

    bool isControl() {return m_isControl;}

signals:
    void reqInit();

    void sigUtpSend(const QByteArray& data);

    void sigClose();

    private slots:
        void onInit();

        void onICEInit(bool) ;
        void onICENego(bool, bool , QString address);
        void onICEReadPkt(const QByteArray& rdata,  unsigned compId, const QString& peerAddr);

        void onUtpSend(const QByteArray& data);

private:
    QString		m_sessionId;
    bool        m_isControl;

    QMutex		m_mutex;

    // 协商结果
    bool        m_isInitialized;
    bool        m_iceNego;
    bool        m_isRelay;
    bool        m_isConnected;
    QString                     m_peerAddress;

    QString                     m_localSDP;
    QString                     m_remoteSDP;

    TransferInterface*          m_interface;
    FocusIce::PJICESocket*      m_iceSocket;

    int                         m_utpConnectID;

    
};

#endif // TRANSPORTCHANNEL_H

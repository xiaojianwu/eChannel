#ifndef UTPHANDLE_H
#define UTPHANDLE_H

#include <QObject>

#include <utp.h>
#include <utp_types.h>
#include <utp_utils.h>

#include <QQueue>
#include <QHash>
#include <QTimer>

class UtpHandle : public QObject
{
    Q_OBJECT

public:
    UtpHandle(QObject *parent = NULL);
    ~UtpHandle();

    struct IncomingUtpPack {
        QString peerAddr;
        QByteArray buf;
    };

    struct PackInfo {
        QString peerAddr;
        QByteArray buf;
        int         pos;
        PackInfo ()
        {
            pos = 0;
        }
    };

public:
    void init();

    void createSocket(QString peerAddr, int connectID);

    void closeSocket(QString peerAddr, int connectID);


    // UDP socket接收到数据
    void udpRecv(QString peerAddr, const QByteArray& buf);

    // 业务请求发送数据
    void udpReqSend(QString peerAddr, const QByteArray& buf);

    // accept 的uutp_socket 设置connectid
    void updateConnectID(QString peerAddr, int connectID);

public:
    static uint64 callback_on_firewall(utp_callback_arguments *arg);
    static uint64 callback_on_accept(utp_callback_arguments* arg);
    static uint64 callback_on_error(utp_callback_arguments* arg);
    static uint64 callback_on_read(utp_callback_arguments* arg);
    static uint64 callback_on_state_change(utp_callback_arguments* arg);
    static uint64 callback_sendto(utp_callback_arguments* arg);
    static uint64 callback_log(utp_callback_arguments* arg);


signals:
    // utp封装好数据请求udp socket发送 biz->utp
    void reqUDPSend(QString peerAddr, const QByteArray& buf);

    // utp解析完的数据给业务 utp->biz
    void utpRecv(int connectID, const QByteArray& buf);

    // 新的UTP SOCKET
    void utpAccept(QString peerAddr);

    // utp 连接成功
    void utpConnected(int connectID);

    // utp_socket error
    void utpError(int connectID, QString msg);

    // utp disconnect
    void utpDisconnected(int connectID);

    // utp处于空闲，上层可以继续发送数据
    void utpIdle(const QString& peerAddr);
signals:
    void reqInit();
    // UDP收到的数据给utp解包
    void reqParse(QString peerAddr, const QByteArray& buf);

    // udp请求utp封装数据包
    void reqPack(QString peerAddr, const QByteArray& buf);

    // 在utp线程中创建一个utpsocket连接
    void reqConnectToPeer(QString peerAddr, int connectID);

    // 在utp线程中设置accept的utp socket的ID
    void reqSetAcceptConnID(QString peerAddr, int connectID);

    // 关闭清理utpsocket
    void reqClose(QString peerAddr, int connectID);

private slots:
    void onInit();
    void processLoop();

    void connectToPeer(QString peerAddr, int connectID);
    void onCloseSocket(QString peerAddr, int connectID);
    

    void save2RecvQueue(QString peerAddr, const QByteArray& buf);
    void save2WriteQueue(QString peerAddr, const QByteArray& buf);

    void checkUtp();
    void setAcceptConnID(QString peerAddr, int connectID);

private:
    utp_socket* prepareConnect(const char *ip, unsigned short port, int connectID);

    void processWrite();
    void processRecv();
    bool write_data(PackInfo& pack);
private:

    bool        m_isInited;
    utp_context *ctx;

    QHash<int, utp_socket*> m_utp_sockets;
    QHash<QString, utp_socket*> m_utp_sockets_addr;

    QQueue<IncomingUtpPack>  m_recvQueue;
    QQueue<PackInfo>  m_writeQueue;

    QTimer*     m_timer;

    uint64      m_last_time;
    
};

#endif // UTPHANDLE_H

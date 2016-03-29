#include "utphandle.h"


#include <QDebug>
#include <QThread>

#include <QStringList>
#include <QDateTime>

#include <Logger.h>

#define LOG_HEAD QString("[%1 %2]").arg(QDateTime::currentDateTime().toString("hh:mm:ss-zzz")).arg((int)(QThread::currentThreadId()))



UtpHandle::UtpHandle(QObject *parent)
    : QObject(parent)
{
    ctx = NULL;
    m_isInited = false;

    m_recvQueue.clear();
    m_writeQueue.clear();


    connect(this, SIGNAL(reqInit()), this, SLOT(onInit()));
    connect(this, SIGNAL(reqConnectToPeer(QString, int)), this, SLOT(connectToPeer(QString, int)));
}

UtpHandle::~UtpHandle()
{
    if (ctx)
    {
        utp_destroy(ctx);
    }
   
}



void UtpHandle::init()
{
    if (m_isInited)
    {
        return;
    }
    m_isInited = true;
    emit reqInit();
}

void UtpHandle::onInit()
{

    ctx = utp_init(2);

    Q_ASSERT_X(ctx, "UtpHandle::init", "utp_init error");
    qDebug() << LOG_HEAD << QString("UTP context %1").arg((int)ctx);

    utp_set_callback(ctx, UTP_LOG,				&callback_log);
    utp_set_callback(ctx, UTP_SENDTO,			&callback_sendto);
    utp_set_callback(ctx, UTP_ON_ERROR,			&callback_on_error);
    utp_set_callback(ctx, UTP_ON_STATE_CHANGE,	&callback_on_state_change);
    utp_set_callback(ctx, UTP_ON_READ,			&callback_on_read);
    utp_set_callback(ctx, UTP_ON_FIREWALL,		&callback_on_firewall);
    utp_set_callback(ctx, UTP_ON_ACCEPT,		&callback_on_accept);


    utp_context_set_userdata(ctx, this);

#ifdef _DEBUG
    utp_context_set_option(ctx, UTP_LOG_NORMAL, 1);
    utp_context_set_option(ctx, UTP_LOG_MTU,    1);
    utp_context_set_option(ctx, UTP_LOG_DEBUG,  1);
#endif // _DEBUG

    //connect(this, SIGNAL(reqLoop()), this, SLOT(processLoop()));
    connect(this, SIGNAL(reqParse(QString, const QByteArray&)), this, SLOT(save2RecvQueue(QString, const QByteArray&)));
    connect(this, SIGNAL(reqPack(QString, const QByteArray&)), this, SLOT(save2WriteQueue(QString, const QByteArray&)));
    connect(this, SIGNAL(reqSetAcceptConnID(QString, int)), this, SLOT(setAcceptConnID(QString, int)));
    connect(this, SIGNAL(reqClose(QString, int)), this, SLOT(onCloseSocket(QString, int)));

    m_timer = new QTimer;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkUtp()));

    m_timer->start(10);
    m_last_time = utp_default_get_milliseconds(NULL);
}



void UtpHandle::createSocket(QString peerAddr, int connectID)
{
    emit reqConnectToPeer(peerAddr, connectID);
}

void UtpHandle::closeSocket(QString peerAddr, int connectID)
{
    emit reqClose(peerAddr, connectID);
}

void UtpHandle::onCloseSocket(QString peerAddr, int connectID)
{
    bool isClosing = false;
    if (m_utp_sockets.contains(connectID))
    {
        utp_socket* s = m_utp_sockets[connectID];
        utp_close(s);
        isClosing = true;
    }
    if (m_utp_sockets_addr.contains(peerAddr))
    {
        utp_socket* s = m_utp_sockets_addr[peerAddr];
        if (!isClosing)
        {
            utp_close(s);
        }
    }
}


void UtpHandle::connectToPeer(QString peerAddr, int connectID)
{
    QStringList ipport = peerAddr.split(":");
    std::string ip = ipport[0].toStdString();
    const char *o_remote_address = ip.c_str();
    unsigned short o_remote_port = ipport[1].toInt();

    utp_socket* s = utp_create_socket(ctx);

    Q_ASSERT_X(s, "UtpHandle::prepareConnect", "utp_create_socket error");

    qDebug() << LOG_HEAD << QString("UTP socket %1\n").arg((int)s);

    m_utp_sockets[connectID] = s;
    m_utp_sockets_addr[peerAddr] = s;

    sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(o_remote_address);
    address.sin_port = htons(o_remote_port);

    // 组装connect报文
    utp_connect(s, (sockaddr*)&address, sizeof(sockaddr_in));

    utp_set_userdata(s, (void*)connectID);

    //return s;
}


// UDP socket接收到数据
void UtpHandle::udpRecv(QString peerAddr, const QByteArray& buf)
{
    // 转到utp线程处理
    emit reqParse(peerAddr, buf);
}

// 业务请求发送数据
void UtpHandle::udpReqSend(QString peerAddr, const QByteArray& buf)
{
    emit reqPack(peerAddr, buf);
}


// accept 的utp_socket 设置connectid
void UtpHandle::updateConnectID(QString peerAddr, int connectID)
{
    emit reqSetAcceptConnID(peerAddr, connectID);
}

void UtpHandle::setAcceptConnID(QString peerAddr, int connectID)
{
    qDebug() << LOG_HEAD << QString("UtpHandle::setAcceptConnID peer=%1, connectid=%2").arg(peerAddr).arg(connectID);
    if (m_utp_sockets_addr.contains(peerAddr))
    {
        utp_socket* s = m_utp_sockets_addr[peerAddr];
        utp_set_userdata(s, (void*)connectID);
    }
}


// UTP解析出的原始数据
uint64 UtpHandle::callback_on_read(utp_callback_arguments *a)
{
    // 参数中获取不到地址信息(addr)，所以只能通过保存的connectid抛给上层
    QByteArray buf;
    buf.append((char*)a->buf, a->len);

    utp_read_drained(a->socket);

    utp_socket* s = a->socket;
    int connectId = (int)utp_get_userdata(s);

#ifdef _DEBUG
    qDebug() << LOG_HEAD << QString("[callback_on_read:] size= %1.connectid=%2.").arg(a->len).arg(connectId);
#endif // _DEBUG

    UtpHandle* h = (UtpHandle*)utp_context_get_userdata(a->context);
    emit h->utpRecv(connectId, buf);

    return 0;
}

uint64 UtpHandle::callback_on_firewall(utp_callback_arguments *a)
{
    qDebug() << LOG_HEAD << "callback_on_firewall Firewall allowing inbound connection\n";
    return 0;
}

uint64 UtpHandle::callback_on_accept(utp_callback_arguments *a)
{
    utp_socket* s = a->socket;
    Q_ASSERT_X(s, "callback_on_accept", "socket is null");

    struct sockaddr_in *sin = (struct sockaddr_in *) a->address;

    QString peerIp = inet_ntoa(sin->sin_addr);
    QString peerPort = QString::number(ntohs(sin->sin_port));
    QString peerAddr = QString("%1:%2").arg(peerIp).arg(peerPort);

    qDebug() << LOG_HEAD << QString("[callback_on_accept:] Accepted inbound socket %2.peeraddr=%3.\n").arg((int)s).arg(peerAddr);

    UtpHandle* h = (UtpHandle*)utp_context_get_userdata(a->context);
    h->m_utp_sockets_addr[peerAddr] = s;

    h->m_okChannels << peerAddr;

    emit h->utpAccept(peerAddr);

    h->processWrite();
    return 0;
}

uint64 UtpHandle::callback_on_error(utp_callback_arguments *a)
{
    QString msg = utp_error_code_names[a->error_code];
    qWarning() << LOG_HEAD << QString("[callback_on_error:] %1.").arg(msg);
    utp_socket* s = a->socket;
    UtpHandle* h = (UtpHandle*)utp_context_get_userdata(a->context);
    int connectID = (int)utp_get_userdata(s);

    //utp_close(s);

    QString peerAddr = h->m_utp_sockets_addr.key(s);
    //emit h->utpError(peerAddr, connectID, msg);

    return 0;
}

uint64 UtpHandle::callback_on_state_change(utp_callback_arguments *a)
{
    // 此回调函数中没有address
    utp_socket_stats *stats;
    UtpHandle* h = (UtpHandle*)utp_context_get_userdata(a->context);

    utp_socket* s = a->socket;

    int connectID = (int)utp_get_userdata(s);
#ifdef _DEBUG
    qDebug() << LOG_HEAD << QString("[callback_on_state_change:]state %1: %2 %3").arg(a->state).arg(utp_state_names[a->state]).arg(connectID);
#endif // _DEBUG
    

    switch (a->state) {
    case UTP_STATE_CONNECT:
        {
            QString peerAddr = h->m_utp_sockets_addr.key(s);

            h->m_okChannels << peerAddr;

            emit h->utpConnected(connectID);
        }
    case UTP_STATE_WRITABLE:
        {           
            h->processWrite();
        }

        break;

    case UTP_STATE_EOF:
        {
            qDebug() << "Received EOF from socket; closing\n";
            utp_close(s);
        }
        break;

    case UTP_STATE_DESTROYING:
        {
            qDebug() << "UTP socket is being destroyed; exiting\n";

            stats = utp_get_stats(s);
            if (stats) {
                qDebug() << "Socket Statistics:\n";
                qDebug() << QString("    Bytes sent:          %1").arg(stats->nbytes_xmit);
                qDebug() << QString("    Bytes received:      %1").arg(stats->nbytes_recv);
                qDebug() << QString("    Packets received:    %1").arg(stats->nrecv);
                qDebug() << QString("    Packets sent:        %1").arg(stats->nxmit);
                qDebug() << QString("    Duplicate receives:  %1").arg(stats->nduprecv);
                qDebug() << QString("    Retransmits:         %1").arg(stats->rexmit);
                qDebug() << QString("    Fast Retransmits:    %1").arg(stats->fastrexmit);
                qDebug() << QString("    Best guess at MTU:   %1").arg(stats->mtu_guess);
            }
            else {
                qDebug() << "No socket statistics available\n";
            }
            QString peerAddr = h->m_utp_sockets_addr.key(s);
            h->m_utp_sockets.remove(connectID);
            h->m_utp_sockets_addr.remove(peerAddr);

            emit h->utpError(peerAddr, connectID, "utp socket destroyed.");
        }
        break;
    }

    return 0;
}


uint64 UtpHandle::callback_sendto(utp_callback_arguments *a)
{
    struct sockaddr_in *sin = (struct sockaddr_in *) a->address;

    QString peerAddr;
    if (sin) 
    {
        QString peerIp = inet_ntoa(sin->sin_addr);
        QString peerPort = QString::number(ntohs(sin->sin_port));
        peerAddr = QString("%1:%2").arg(peerIp).arg(peerPort);
    }

#ifdef _DEBUG
    qDebug() << LOG_HEAD << QString("callback_sendto: %1 byte packet to %2 %3.\n").arg(a->address_len).arg(peerAddr).arg((a->flags & UTP_UDP_DONTFRAG) ? "  (DF bit requested, but not yet implemented)" : "");
#endif // _DEBUG

    // utp发送回执时， 参数中没有保存utp_socket
    //utp_socket* s = a->socket;


    QByteArray buf;
    buf.append((char*)a->buf, a->len);


    UtpHandle* handle = (UtpHandle*)utp_context_get_userdata(a->context);
    emit handle->reqUDPSend(peerAddr, buf);

    return 0;
}

uint64 UtpHandle::callback_log(utp_callback_arguments *a)
{
    //qWarning() << LOG_HEAD << QString("log: %1\n").arg((char*)a->buf);
    return 0;
}


void UtpHandle::save2RecvQueue(QString peerAddr, const QByteArray& buf)
{
    IncomingUtpPack pack;
    pack.peerAddr = peerAddr;
    pack.buf = buf;

    m_recvQueue.enqueue(pack);

    processLoop();
}

void UtpHandle::save2WriteQueue(QString peerAddr, const QByteArray& buf)
{
    PackInfo pack;
    pack.peerAddr = peerAddr;
    pack.buf = buf;

    m_writeQueue.enqueue(pack);

    processLoop();
}

void UtpHandle::checkUtp()
{
    if (m_recvQueue.isEmpty())
    {
        utp_issue_deferred_acks(ctx);
    }
    int cur_time = utp_default_get_milliseconds(NULL);
    if (cur_time - m_last_time > 1000)
    {
        m_last_time = cur_time;
        utp_check_timeouts(ctx);
    }
    
}

void UtpHandle::processLoop()
{
    processWrite();
    processRecv();

    int cur_time = utp_default_get_milliseconds(NULL);
    if (cur_time - m_last_time > 1000)
    {
        m_last_time = cur_time;
        utp_check_timeouts(ctx);
    }
}


void UtpHandle::processWrite()
{
    while (m_writeQueue.size() > 0)
    {
        PackInfo& pack =  m_writeQueue.head();

        if (!m_okChannels.contains(pack.peerAddr))
        {
            m_writeQueue.dequeue();
            continue;
        }

        if (write_data(pack))
        {
            m_writeQueue.dequeue();
        }
        else
        {
            // not writeable
            break;
        }
    }
}

void UtpHandle::processRecv()
{
    while (m_recvQueue.size() > 0)
    {
        IncomingUtpPack pack =  m_recvQueue.dequeue();
        QByteArray buf = pack.buf;
        QString peerAddr = pack.peerAddr;

        unsigned char* socket_data = (unsigned char*)buf.data();

        QStringList ipport = peerAddr.split(":");
        std::string strip = ipport[0].toStdString();
        const char *ip = strip.c_str();
        unsigned short port = ipport[1].toInt();

        sockaddr_in src_addr = {0};
        src_addr.sin_family = AF_INET;
        src_addr.sin_addr.s_addr = inet_addr(ip);
        src_addr.sin_port = htons(port);
        socklen_t addrlen = sizeof(src_addr);

        ssize_t len = buf.size();

        if (! utp_process_udp(ctx, socket_data, len, (sockaddr*)&src_addr, addrlen))
            qDebug() << LOG_HEAD << "UDP packet not handled by UTP.  Ignoring.\n";
    }
}


bool UtpHandle::write_data(PackInfo& pack)
{
    QString peerAddr = pack.peerAddr;
    utp_socket* s = NULL;
    if (!m_utp_sockets_addr.contains(peerAddr))
    {
        //goto out;
        return false;
    }
    s = m_utp_sockets_addr[peerAddr];

    int pos = pack.pos;
    int sent = 0;
    int buf_len;
    char *buf, *p;

    buf = (char*)(pack.buf.data());
    p = buf + pos;
    buf_len = pack.buf.size();

#ifdef _DEBUG
    //qDebug() << LOG_HEAD << "write_data size=" << buf_len << "pos=" << pos;
#endif // _DEBUG
    

    while (p < buf+buf_len) {
        size_t sent;

        sent = utp_write(s, p, buf+buf_len-p);
        if (sent == 0) {
            int newpos =  p - buf;
            #ifdef _DEBUG
             qDebug() << LOG_HEAD << "socket no longer writable. pos=" << newpos;
            #endif
            
            pack.pos = newpos;
            return false;
        }

        p += sent;

        if (p == buf+buf_len) {
            #ifdef _DEBUG
            //qDebug() << LOG_HEAD << QString("wrote %1 bytes; buffer now empty").arg(sent);
            #endif
            p = buf;
            buf_len = 0;

            // 用于文件传输，通知上层正处于空闲
            emit utpIdle(peerAddr);
            break;
        }
        else
        {
            #ifdef _DEBUG
            //qDebug() << LOG_HEAD << QString("wrote %1 bytes; %2 bytes left in buffer").arg(sent).arg(buf+buf_len-p);
            #endif
        }
    }

    return true;
}

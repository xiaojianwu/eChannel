#include "transportchannel.h"

#include <QtEndian>

#include <QDebug>

#include <Logger.h>

#include "utpmanager.h"

#include <QUuid>


TransportChannel::TransportChannel(QObject *parent)
{
    m_sessionId = "";
    m_iceNego = false;
    m_isInitialized = false;
    m_isRelay = false;
    m_isConnected = false;

    m_iceSocket = NULL;
}

TransportChannel::~TransportChannel()
{

}


void TransportChannel::init(QString sessionID, bool isControl, TransferInterface* tInterface)
{
    m_sessionId = sessionID;
    m_isControl = isControl;
    m_interface = tInterface;

    // 线程
    //connect(this, SIGNAL(reqInit()), this, SLOT(onInit()));
    //emit reqInit();

    onInit();

}


void TransportChannel::close()
{
    // TODO:
    delete m_iceSocket;
    m_iceSocket = NULL;

    disconnect(this);

    emit sigClose();
}

void TransportChannel::onInit()
{
    m_iceSocket = new FocusIce::PJICESocket(this, m_sessionId);

    int size = 256 * 1024;

    m_iceSocket->setRecvBufSize(size);
    m_iceSocket->setSndBufSize(size);


    connect(m_iceSocket, SIGNAL(cbInitResult(bool)),this,SLOT(onICEInit(bool))) ;
    connect(m_iceSocket, SIGNAL(cbNegoResult(bool, bool, QString)),this,SLOT(onICENego(bool,bool, QString))) ;
    connect(m_iceSocket, SIGNAL(cbReadData(const QByteArray&,  unsigned, const QString&)),this,
        SLOT	(onICEReadPkt(const QByteArray&,  unsigned, const QString&))) ;


    connect(this, SIGNAL(sigUtpSend(const QByteArray&)), this, SLOT(onUtpSend(const QByteArray&)));


    m_iceSocket->startICESocket(m_interface->getTurnServer());

}


void TransportChannel::onICEInit(bool isSuccess ) 
{
#ifdef _DEBUG
    qDebug() << "TransportChannel::onICEInit result=" << isSuccess;
#endif // _DEBUG

    m_isInitialized = isSuccess;

    emit initialized();
}

bool TransportChannel::isInitialized()
{
    return m_isInitialized;
}

bool TransportChannel::isConnected()
{
    return m_isConnected;
}

// ICE协商完成
void TransportChannel::onICENego(bool isSuccess, bool isRelay, QString address)
{
#ifdef _DEBUG
    qDebug() << "TransportChannel::onICENego isSuccess=" << isSuccess << "isRelay= " << isRelay << "address=" << address ;
#endif // _DEBUG

    m_peerAddress = address;

    m_isRelay = isRelay;

    if (!isSuccess)
    {
        emit error(ERROR_CODE_NE_FAILED, "ice nego failed."); 
    }
    else
    {
        m_utpConnectID = UTPManager::instance()->createUtpSocket(address, this);
    }

    m_iceNego = isSuccess ;
}

void TransportChannel::onICEReadPkt(const QByteArray& data,  unsigned compId, const QString& peerAddr)
{
#ifdef _DEBUG
    qDebug() << "TransportChannel::onICEReadPkt buf=" << data.toHex() << "size=" << data.size() << "peeraddr=" << peerAddr;
#endif // _DEBUG
    
    UTPManager::instance()->udpRecv(peerAddr, data);
}

// socket线程中
void TransportChannel::utpSend(const QByteArray& data)
{
#ifdef _DEBUG
    //qDebug() << "TransportChannel::utpSend peeraddr=" << m_peerAddress << "buf=" << data.toHex(); 
#endif // _DEBUG
    // TODO: ICE没有在线程中
    //emit sigUtpSend(data);
    onUtpSend(data);
}

// UTP包装的数据通过ICE socket发送
void TransportChannel::onUtpSend(const QByteArray& data)
{
#ifdef _DEBUG
    qDebug() << "TransportChannel::onUtpSend peeraddr=" << m_peerAddress << "buf=" << data.toHex(); 
#endif // _DEBUG

    m_iceSocket->iceSentTo((void*)data.data(), data.size());
}


// 主线程中,UTP确认收到的完整可靠包
void TransportChannel::utpRecv(const QByteArray& data)
{
#ifdef _DEBUG
    qDebug() << "TransportChannel::utpRecv peeraddr=" << m_peerAddress << "buf=" << data.toHex(); 
#endif // _DEBUG
    emit readyRead(data);
}


/************************************************************************/
/* Channel接口实现部分                                                  */
/************************************************************************/
QString TransportChannel::getLocalSDP()
{
    Q_ASSERT_X(m_iceSocket != NULL, "getLocalSDP", "icesocket not initialed.");
    m_localSDP = m_iceSocket->localSdp(m_isControl);

    qDebug() << "local sdp=" << m_localSDP;

    return m_localSDP;
}


void TransportChannel::updateRemoteSDP(QString sdp)
{
    m_remoteSDP = sdp;
    Q_ASSERT_X(m_iceSocket != NULL, "updateRemoteSDP", "icesocket not initialed.");
    m_iceSocket->negoWith(sdp);
}


QString TransportChannel::getPeerAddress()
{
    return m_peerAddress;
}

// 外部传入需要发送的数据
int TransportChannel::write(const QByteArray &data)
{
#ifdef _DEBUG
    qDebug() << "TransportChannel::write m_peerAddress=" << m_peerAddress;
#endif // _DEBUG
    bool ba = !m_peerAddress.isEmpty();
    Q_ASSERT_X(ba, "TransportChannel::write", QString("bad connection.address=%1").arg(m_peerAddress).toStdString().c_str());

    UTPManager::instance()->reqUtpPack(m_peerAddress, data);
    return 0; // write(data.data(), data.size());
}


bool TransportChannel::isRelay()
{
    return m_isRelay;
}


void TransportChannel::connectSuccess(QString peerAddr)
{
#ifdef _DEBUG
    qDebug() << "TransportChannel::connectSuccess m_peerAddress=" << peerAddr;
#endif // _DEBUG
    if (!peerAddr.isEmpty())
    {
        m_peerAddress = peerAddr;
    }
    m_isConnected = true;
    emit connected(); 
}

void TransportChannel::conncetError(QString errMsg)
{
    emit error(ERROR_CODE_OTHER, errMsg);
}


void TransportChannel::onUtpIdle()
{
    emit idle();
}


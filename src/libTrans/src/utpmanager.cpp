#include "utpmanager.h"

#include <assert.h>

#include <QDebug>
#include <QStringList>

#include <qglobal.h>

#include <qt_windows.h>

#include "transportchannel.h"

UTPManager::UTPManager(QObject *parent)
    : QObject(parent)
{
    m_isInited = false;
}

UTPManager::~UTPManager()
{
    m_utpThread.wait();
    m_utpThread.quit();
}


UTPManager* UTPManager::_instance = NULL;
UTPManager* UTPManager::instance()
{
    if (!_instance)
    {
        _instance = new UTPManager;
    }
    return _instance;
}


void UTPManager::init()
{
    if (m_isInited)
    {
        return;
    }

    m_isInited = true;
    m_connID = 1;

    m_utpHandle = new UtpHandle();

    m_utpThread.setObjectName("UtpHandleThread");
    m_utpHandle->moveToThread(&m_utpThread);


    connect(m_utpHandle, SIGNAL(utpConnected(int)), this, SLOT(onUtpConnected(int)));
    connect(m_utpHandle, SIGNAL(utpAccept(QString)), this, SLOT(onUtpAccept(QString)));
    connect(m_utpHandle, SIGNAL(reqUDPSend(QString, const QByteArray&)), this, SLOT(onUtpReqSend(QString, const QByteArray&)));
    connect(m_utpHandle, SIGNAL(utpRecv(int, const QByteArray&)), this, SLOT(onUtpRecv(int, const QByteArray&)));

    
    connect(m_utpHandle, SIGNAL(utpIdle(const QString&)), this, SLOT(onUtpIdle(const QString&)));
    connect(m_utpHandle, SIGNAL(utpError(int, QString)), this, SLOT(onUtpError(int, QString)));
    

    m_utpThread.start();

    m_utpHandle->init();
}



int UTPManager::createUtpSocket(QString peerAddress, TransportChannel* channel)
{
    qDebug() << QString("createUtpSocket remote address %1").arg(peerAddress);

    m_connID++;
    m_hashChannels[peerAddress] = channel;
    m_hashIDChannels[m_connID] = channel;

    if (channel->isControl())
    {
        m_utpHandle->createSocket(peerAddress, m_connID);
    }

    return m_connID;
}


void UTPManager::close(QString peerAddress, int connectID)
{
    m_utpHandle->closeSocket(peerAddress, connectID);
    m_hashChannels.remove(peerAddress);
    m_hashIDChannels.remove(connectID);
}

void UTPManager::udpRecv(QString peerAddr, const QByteArray& buf)
{
    m_utpHandle->udpRecv(peerAddr, buf);
}

void UTPManager::reqUtpPack(QString peerAddr, const QByteArray& buf)
{
    m_utpHandle->udpReqSend(peerAddr, buf);
}

// utp组装好的数据请求通过udpsocket发送
void UTPManager::onUtpReqSend(QString peerAddress, const QByteArray& buf)
{
    if (m_hashChannels.contains(peerAddress))
    {
        TransportChannel* channel = m_hashChannels[peerAddress];
        if (channel)
        {
            channel->utpSend(buf);
        }
    }
}

// utp解封后的原始数据包
void UTPManager::onUtpRecv(int connectID, const QByteArray& buf)
{

    if (m_hashIDChannels.contains(connectID))
    {
        TransportChannel* channel = m_hashIDChannels[connectID];
        /*emit */
        channel->utpRecv(buf);
    }
}

// 被动端受到connect，给utp_socket设置connectid
void UTPManager::onUtpAccept(QString peerAddress)
{

#ifdef _DEBUG
    qDebug() << "UTPManager::onUtpAccept peerAddress=" << peerAddress;
#endif // _DEBUG
    if (m_hashChannels.contains(peerAddress))
    {
        TransportChannel* channel = m_hashChannels[peerAddress];
        int connID = channel->getUtpConnID();
        m_utpHandle->updateConnectID(peerAddress, connID);
        channel->connectSuccess(peerAddress);
    }
    else
    {
        Q_ASSERT_X(false, "UTPManager::onUtpAccept", peerAddress.toStdString().c_str());
    }
}

// 主动connect成功
void UTPManager::onUtpConnected(int connectID)
{
#ifdef _DEBUG
    qDebug() << "UTPManager::onUtpConnected connectID=" << connectID;
#endif // _DEBUG
    if (m_hashIDChannels.contains(connectID))
    {
        TransportChannel* channel = m_hashIDChannels[connectID];

        QString peeraddr = m_hashChannels.key(channel);

#ifdef _DEBUG
        qDebug() << "UTPManager::onUtpConnected peeraddr=" << peeraddr;
#endif // _DEBUG

        channel->connectSuccess(peeraddr);
    }
}

void UTPManager::onUtpError(int connectID, QString errMsg)
{
    if (m_hashIDChannels.contains(connectID))
    {
        TransportChannel* channel = m_hashIDChannels[connectID];
        channel->conncetError(errMsg);
    }
}

void UTPManager::onUtpIdle(const QString& peerAddr)
{
    if (m_hashChannels.contains(peerAddr))
    {
        TransportChannel* channel = m_hashChannels[peerAddr];
        channel->onUtpIdle();
    }
    else
    {
        //Q_ASSERT_X(false, "UTPManager::onUtpIdle", peerAddress.toStdString().c_str());
    }
}







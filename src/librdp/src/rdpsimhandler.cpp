#include "rdpsimhandler.h"

#include <QProcess>
#include <QStringList>
#include <QFile>

#include <Logger.h>

RDPSimHandler::RDPSimHandler(QObject *parent)
	: m_simSocket(NULL),
	m_channel(NULL),
	m_viewerProcess(NULL),
	m_isClosing(false),
	m_socketDescriptor(0),
    m_isStartFailed(false)
{
}

RDPSimHandler::~RDPSimHandler()
{

}


void RDPSimHandler::init(TransferInterface* pInterface, RDPInterface* rdpinterface, QString sessionId, int simPort)
{
#ifdef _DEBUG
    qDebug()
#else
    LOG_DEBUG()
#endif // _DEBUG
        << "RDPSimHandler::init sessionId=" << sessionId;


    m_pInterface = pInterface;
    m_rdpInterface = rdpinterface;

    m_sessionId = sessionId;
    m_simPort = simPort;
    m_channel = pInterface->getChannel(m_sessionId);

    m_rcvBuf.clear();

    m_viewerArgs << QString("%1:%2").arg("127.0.0.1").arg(m_simPort);

    connect(m_channel, SIGNAL(readyRead(const QByteArray&)), this, SLOT(onRead(const QByteArray&)));
    connect(m_channel, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(m_channel, SIGNAL(error(int, QString)), this, SLOT(onChannelError(int, QString)));
}


void RDPSimHandler::setViewerArgs(QString args)
{
    // TODO:
    if (!args.isEmpty())
    {
        m_viewerArgs.clear();

        m_viewerArgs << QString("%1:%2").arg("127.0.0.1").arg(m_simPort);  
    }
    
}

void RDPSimHandler::start(bool isControl)
{

#ifdef _DEBUG
    qDebug()
#else
    LOG_DEBUG()
#endif // _DEBUG
        << "RDPSimHandler::start isControl=" << isControl;

    m_isControl = isControl;

    if (m_channel->isConnected())
    {
        if (m_isControl)
        {
            startVNCViewer();
        }
        else
        {
            createSimSocket(0);
        }
    }
    else
    {
        m_isStartFailed = true;
    }
}


void RDPSimHandler::startVNCViewer()
{

#ifdef _DEBUG
    qDebug()
#else
    LOG_DEBUG()
#endif // _DEBUG
        << "RDPSimHandler::startVNCViewer";


	QString program = m_rdpInterface->getVncViewerPath();


	if (!QFile::exists(program))
	{
		close();
		return;
	}

	m_viewerProcess = new QProcess(this);

	connect(m_viewerProcess, SIGNAL(started()), this, SLOT(onViewerProcessStarted()));
	connect(m_viewerProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onViewerProcessError(QProcess::ProcessError)));
	connect(m_viewerProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onViewerProcessExit(int, QProcess::ExitStatus)));
	connect(m_viewerProcess, SIGNAL(stateChanged (QProcess::ProcessState)), this, SLOT(onViewerProcessStateChanged(QProcess::ProcessState)));
	m_viewerProcess->start(program, m_viewerArgs);
}


void RDPSimHandler::onViewerProcessStarted()
{
	LOG_DEBUG() << "started";
}

void RDPSimHandler::onViewerProcessExit(int exitCode, QProcess::ExitStatus exitStatus)
{
	LOG_DEBUG() << "RDPSimHandler::onViewerProcessExit exitCode=" << exitCode << "exitStatus=" << exitStatus ;

    // 直接通过与模拟socket的链接断开后，即判断为断开。
	//close();
}

void RDPSimHandler::onViewerProcessStateChanged (QProcess::ProcessState newState)
{
	LOG_DEBUG() << "newState=" << newState ;
}

void RDPSimHandler::onViewerProcessError ( QProcess::ProcessError error )
{
	LOG_DEBUG() << "RDPSimHandler::onViewerProcessError error=" << error ;
	//close();
}

void RDPSimHandler::close()
{
    if (!m_isClosing)
    {
        m_isClosing = true;

        if (m_channel)
        {
            m_channel->close();
        }

        if (m_simSocket)
        {
            m_simSocket->close();
        }

        // 等待模拟线程退出
        m_simSocketThread.quit();
        m_simSocketThread.wait();
    }

}

// viewer连接成功， client端
void RDPSimHandler::onNewSimConnection(int socketDescriptor)
{

    qDebug() << "RDPSimHandler::onNewSimConnection socketDescriptor=" << socketDescriptor;
	if (m_socketDescriptor == 0)
	{
		LOG_DEBUG()  << "sessionId=" << m_sessionId;
		m_socketDescriptor = socketDescriptor;

        createSimSocket(m_socketDescriptor);
	}
}


void RDPSimHandler::onSimConnected()
{

#ifdef _DEBUG
    qDebug()
#else
    LOG_DEBUG()
#endif // _DEBUG
        << "RDPSimHandler::onSimConnected m_rcvBuf=" << m_rcvBuf;

	QMutexLocker locker(&m_rcvBufMutex);
	if (!m_rcvBuf.isEmpty())
	{
		if (m_simSocket)
		{
            m_simSocket->write(m_rcvBuf);
			m_rcvBuf.clear();
		}
	}
	
}

void RDPSimHandler::onSimRead(const QByteArray& data)
{

    qDebug() << "RDPSimHandler::onSimRead " << data;
	m_channel->write(data);
}


void RDPSimHandler::onSimDisconnected()
{
    qDebug() << "onSimDisconnected exit";

    close();
}


// 连接成功
void RDPSimHandler::onConnected()
{
	// 双方均连接成功
#ifdef _DEBUG
    LOG_DEBUG() 
#else
    qDebug()
#endif // _DEBUG
     << "RDPSimHandler::onConnected. " << "sessionId=" << m_sessionId << "address=" << m_channel->getPeerAddress();

    if (m_isStartFailed)
    {
        start(m_isControl);
    }
    
}


void RDPSimHandler::onChannelError(int code, QString errMsg)
{
    qDebug() << QString("%1 onChannelError. code=%2, msg=%3").arg(QDateTime::currentDateTime().toString("hh:mm:ss-zzz")).arg(code).arg(errMsg);

    // 此时channel已经被销毁，不可再使用
    m_channel = NULL;

    emit error(m_sessionId, RDPInterface::RDP_ERROR_SIM_SOCKET_DISCONNECT, QString("channel error:%1").arg(errMsg));
}


void RDPSimHandler::createSimSocket(int socketNotifier)
{
	m_simSocket = new SimSocket();
	m_simSocket->init(m_sessionId, m_isControl, socketNotifier, m_rdpInterface->getVncPort());

    connect(m_simSocket, SIGNAL(sigSimConnected()), this, SLOT(onSimConnected()));
    connect(m_simSocket, SIGNAL(sigSimRead(const QByteArray&)), this, SLOT(onSimRead(const QByteArray&)));
    connect(m_simSocket, SIGNAL(sigSimDisconnected()), this, SLOT(onSimDisconnected()));

	m_simSocket->moveToThread(&m_simSocketThread);

    m_simSocketThread.setObjectName("SimSocketThread");
	m_simSocketThread.start();
	
	m_simSocket->createSocket();
}



void RDPSimHandler::onRead(const QByteArray& buf)
{
    // 转发P2P -> 本地TCP
    QMutexLocker locker(&m_rcvBufMutex);
    m_rcvBuf.append(buf);

#ifdef _DEBUG
    qDebug()
#else
    LOG_DEBUG() 
#endif // _DEBUG
     << "RDPSimHandler::onRead"  <<  "sessionId=" << m_sessionId << "from peer size=" << m_rcvBuf.size() <<  "data=" << m_rcvBuf;

	if (m_simSocket)
	{
		m_simSocket->write(m_rcvBuf);
		m_rcvBuf.clear();
	}
}

void RDPSimHandler::terminate()
{
	LOG_DEBUG()  << "sessionId=" << m_sessionId;
    
    close();   
}

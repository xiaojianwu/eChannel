#include "rdpinstance.h"

#include <QProcessEnvironment>
#include <QDir>

#include <Logger.h>
//#include <RollingFileAppender.h>

#include <qglobal.h>

#include <Windows.h>

#include <QCoreApplication>

RDPInstance::RDPInstance() : m_tcpServer(NULL)
{

}

RDPInstance::~RDPInstance()
{

}



void RDPInstance::init(TransferInterface* pInterface, QString vncViewerPath, int vncPort)
{
	m_vncViewerPath = vncViewerPath;
	m_pInterface = pInterface;

    m_vncPort = vncPort;

	QString appdataPath = QProcessEnvironment::systemEnvironment().value("APPDATA");
	QDir dir;
	QString logpath = QString("%1/FocusChina/FocusTeachService/logs").arg(appdataPath);
	dir.mkpath(logpath);
	QString logfile = QString("%1/rdp_%2.log").arg(logpath).arg(m_pInterface->getCurrentUser());

	//RollingFileAppender* appdender = new RollingFileAppender(logfile);
	//appdender->setDetailsLevel(Logger::Debug);
	//appdender->setDatePattern(RollingFileAppender::DailyRollover);

	//logger->registerAppender(appdender);

	// 启动模拟服务端
	m_tcpServer = new SimTCPServer();
	if (m_tcpServer->listen(QHostAddress("127.0.0.1")))
	{
		int port = m_tcpServer->serverPort();
		LOG_DEBUG()  << "bind port=" << port;
	}

	LOG_DEBUG() << "init ok. viewerpath=" << m_vncViewerPath;
} 

QString RDPInstance::getVncViewerPath()
{
	return m_vncViewerPath;
}

void RDPInstance::initConnection(QString sessionId, bool isControl)
{
	LOG_DEBUG() << "sessionId=" << sessionId << "isControl=" << isControl << "user=" << m_pInterface->getCurrentUser();
    const bool isMainThread = 
        QThread::currentThread() == QCoreApplication::instance()->thread();
    Q_ASSERT_X(isMainThread, "RDPInstance::initConnection", "not in main thread.please check it.");

    if (m_handlers.contains(sessionId))
    {
        RDPSimHandler* handler =  m_handlers[sessionId];
        handler->start(isControl);
    }
}



void RDPInstance::initChannel(QString sessionId, QString viewrArgs)
{
    LOG_DEBUG() << "sessionId=" << sessionId << "user=" << m_pInterface->getCurrentUser();
    const bool isMainThread = 
        QThread::currentThread() == QCoreApplication::instance()->thread();
    Q_ASSERT_X(isMainThread, "RDPInstance::initConnection", "not in main thread.please check it.");

    if (!m_handlers.contains(sessionId))
    {
        RDPSimHandler* handler = new RDPSimHandler(this);

        connect(handler, SIGNAL(error(QString, int, QString)), this, SLOT(onError(QString, int, QString)));

        connect(m_tcpServer, SIGNAL(newConn(int)), handler, SLOT(onNewSimConnection(int)));

        handler->init(m_pInterface, this, sessionId, m_tcpServer->serverPort());
        handler->setViewerArgs(viewrArgs);

        m_handlers[sessionId] = handler;
    }


}


// 发送协助请求
 void RDPInstance::sendRDPAssitant(QString sessionId)
 {
	 LOG_DEBUG() << "sessionId=" << sessionId << "user=" << m_pInterface->getCurrentUser();
	 initConnection(sessionId, false);// 启动一个客户端模拟器
 }

 // 接受协助请求
 void RDPInstance::recvRDPAssitant(QString sessionId)
 {
     LOG_DEBUG() << "sessionId=" << sessionId << "user=" << m_pInterface->getCurrentUser();
     initConnection(sessionId, true);// 启动一个服务端模拟器
 }

// 发送控制请求
 void RDPInstance::sendRDPControl(QString sessionId)
 {
	 LOG_DEBUG() << "sessionId=" << sessionId << "user=" << m_pInterface->getCurrentUser();
	 initConnection(sessionId, true);
 }

// 接受控制请求
 void RDPInstance::recvRDPControl(QString sessionId)
 {
	 LOG_DEBUG() << "sessionId=" << sessionId << "user=" << m_pInterface->getCurrentUser();
	 initConnection(sessionId, false);
 }


 void RDPInstance::onError(QString sessionID, int code, QString errMsg)
 {
     emit error(sessionID, code, errMsg);
 }

// 结束会话
 void RDPInstance::terminate(QString sessionId)
 {
	 LOG_DEBUG() << "sessionId=" << sessionId << "user=" << m_pInterface->getCurrentUser();
	 if (m_handlers.contains(sessionId))
	 {
		 RDPSimHandler* handler = m_handlers[sessionId];

		 disconnect(m_pInterface, 0, handler, 0);
		 handler->terminate();

		 delete handler;

		 m_handlers.remove(sessionId);
	 }
 }

#if QT_VERSION < 0x050000
  Q_EXPORT_PLUGIN2(rdpinstance, RDPInstance);
#endif

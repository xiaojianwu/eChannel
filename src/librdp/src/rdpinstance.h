#ifndef RDP_INSTANCE_H
#define RDP_INSTANCE_H

#include <QObject>
#include <QQueue>

#include <QtPlugin>
#include <QTcpServer>

#include <libTrans.h>
#include <libRDP.h>

#include "rdpsimhandler.h"


// 模拟服务器
class SimTCPServer : public QTcpServer
{
	Q_OBJECT

public:
	SimTCPServer() {};
	~SimTCPServer()
	{

	}
signals:
	void newConn(int handle);

protected:
	virtual void incomingConnection ( int socketDescriptor ) 
	{
		LOG_DEBUG() << "socketDescriptor=" << socketDescriptor;
		qDebug() << "SimTCPServer::incomingConnection socketid=" << socketDescriptor;
		emit newConn(socketDescriptor);
	}
};


class RDPInstance : public RDPInterface
{
	Q_OBJECT
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
		Q_PLUGIN_METADATA(IID "com.focuschina.RDPInterface" FILE "libprdp.json")
#endif

		Q_INTERFACES(RDPInterface)

public:
	RDPInstance();
	~RDPInstance();

public:

	virtual void init(TransferInterface* pInterface, QString vncViewerPath, int vncPort);

	virtual QString getVncViewerPath();

    // 初始化传输通道
    virtual void initChannel(QString sessionId, QString viewrArgs);

	// 发送协助请求
	virtual void sendRDPAssitant(QString sessionId);

	// 接受协助请求
	virtual void recvRDPAssitant(QString sessionId);


	// 发送控制请求
	virtual void sendRDPControl(QString sessionId);

	// 接受控制请求
	virtual void recvRDPControl(QString sessionId);

	// 结束会话
	virtual void terminate(QString sessionId);


    int getVncPort() {return m_vncPort;}

private:
	void initConnection(QString sessionId, bool isControl);


private slots:
    void onError(QString sessionID, int code, QString errMsg);


private:
	TransferInterface* 			    m_pInterface;
	QHash<QString, RDPSimHandler*>	m_handlers;

	QString							m_vncViewerPath;

	SimTCPServer*	m_tcpServer;

    int                             m_vncPort;

};

#endif // RDP_INSTANCE_H

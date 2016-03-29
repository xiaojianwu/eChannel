#ifndef RDP_SIM_HANDLER_H
#define RDP_SIM_HANDLER_H

#include <QThread>

#include <QMutex>


#include <QProcess>

#include <libTrans.h>
#include <libRDP.h>

#include "simsocket.h"

#include <Logger.h>

class RDPSimHandler : public QObject
{
	Q_OBJECT

public:
	RDPSimHandler(QObject *parent);
	~RDPSimHandler();

	void init(TransferInterface* pInterface, RDPInterface* rdpinterface, QString sessionId, int simPort);

    void setViewerArgs(QString args);


    void start(bool isControl);

	void terminate();

	void startVNCViewer();


    void createSimSocket(int socketNotifier); 

public slots:
    // channel信号处理
    void onRead(const QByteArray& data);
    void onConnected();
    void onChannelError(int code, QString msg);


    // 模拟socket信号处理
    void onSimConnected();
    void onSimRead(const QByteArray& data);
    void onSimDisconnected();

	void onNewSimConnection(int socketDescriptor);
	void onViewerProcessStarted();
	void onViewerProcessExit(int exitCode, QProcess::ExitStatus exitStatus);
	void onViewerProcessStateChanged (QProcess::ProcessState newState);
	void onViewerProcessError ( QProcess::ProcessError error );


private:
    void close();

	
signals:
	void error(QString sessionId, int code, QString errMsg);

private:
	QString			m_sessionId;
	bool			m_isControl; // 控制端

    bool            m_isStartFailed; // 由于网络没准备好启动失败

	Channel*		m_channel;	 // P2P传输通道

	
	int				m_socketDescriptor;

	SimSocket*		m_simSocket;

	TransferInterface*	m_pInterface;
	RDPInterface*	m_rdpInterface;

	QByteArray		m_rcvBuf;

	QMutex			m_rcvBufMutex;

	QProcess*		m_viewerProcess;
	QStringList     m_viewerArgs;


	QThread			m_simSocketThread;

	int				m_simPort;


    bool            m_isClosing;
};

#endif // RDP_SIM_HANDLER_H

#ifndef SIMSOCKET_H
#define SIMSOCKET_H

#include <QObject>

#include <QTcpSocket>
#include <QMutex>


//class SimNotify
//{
//public:
//	virtual void onSimConnected() = 0;
//	virtual void onSimRead(const QByteArray& buf) = 0;
//	virtual void onSimError(QString errMsg) = 0;
//	virtual void onSimDisconnected() = 0;
//};

class SimSocket : public QObject
{
	Q_OBJECT

public:
	SimSocket();
	~SimSocket();

	void init(QString sessionId, bool isControl, int handle, int vncPort);
    void createSocket();
    void close();
    void write(const QByteArray& buf);

signals:
    //void onSimRead(const QByteArray& buf);
    //void onSimError(QString errMsg);

    void sigCreateSocket();
    void sigClose();
    void sigWrite(const QByteArray& buf);

    void sigSimConnected();
    void sigSimRead(const QByteArray& buf);
    void sigSimError(QString errMsg);
    void sigSimDisconnected();
	
public slots:
		void onNewSocket();
		void onWrite(const QByteArray& buf);
        void onCloseConnection();

		void onSimConnected();
		void onSimRead();
		void onSimDisconnected();
		void onSimError(QAbstractSocket::SocketError);

		

private:
	QTcpSocket*		m_tcpSocket; // 与RDP程序的通信连接

	QString			m_sessionId;
	bool			m_isControl;
	int				m_socketDescriptor;

	QByteArray		m_buf;
	QMutex			m_bufMutex;

	//SimNotify*		m_notifier;

	bool			m_forceClose;

    int             m_vncPort;
};

#endif // SIMSOCKET_H

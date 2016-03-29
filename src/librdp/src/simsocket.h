#ifndef SIMSOCKET_H
#define SIMSOCKET_H

#include <QObject>

#include <QTcpSocket>
#include <QMutex>

class SimSocket : public QObject
{
	Q_OBJECT

public:
	SimSocket();
	~SimSocket();

	void init(QString sessionId, bool isControl, int socketHandle, int vncPort);
    void createSocket();
    void close();
    void write(const QByteArray& buf);

signals:
    void sigCreateSocket();
    void sigClose();
    void sigWrite(const QByteArray& buf);

    void sigSimConnected();
    void sigSimRead(const QByteArray& buf);
    //void sigSimError(QString errMsg);
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

    int             m_vncPort;
};

#endif // SIMSOCKET_H

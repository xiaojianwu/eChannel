#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>

#include <QRunnable>
#include <QThread>

#include <QTimer>
#include <QTcpSocket>
#include <QMutex>

#include <string>




class Connection :/* public QObject, */public QThread
{
	Q_OBJECT

public:
	Connection(QObject *parent = NULL);
	~Connection();

private:


public:
	void setProxy(QString proxyIp, quint16 port);

	void connectToHost(QString ip, quint16 port);
	void run();

	// 添加发送任务
	void addSendTask(int type, int length, const std::string& data);
	int addSendTask(int type, int length, const char *data);

	// 释放连接
	void release();

	QString ServerIp() const { return m_serverIp; }
	quint16 ServerPort() const { return m_serverPort; }

	public slots:
		// 数据处理
		void onRead();

		// 网络连接错误处理
		void onError(QAbstractSocket::SocketError);


		void onConnected();
		// 网络断开
		void onDisconnected();

signals:
		void error(QAbstractSocket::SocketError, QString);

		void recvData(int type, int length, const QByteArray& payload);

		void connected();

		void disconnected();


private:
	int parseHeader(); // 解析包头
	bool hasEnoughData(); // 判断包是否完整
	void processData(); // 解析处理



private:
	static quint16 byte2Int(const QByteArray& data);
	static QByteArray int2Byte(quint16 num);


private:
	// c++ 11, initialising in headers...
	QTimer*		m_pTimer;
	QTcpSocket* m_pSocket;   

	QString		m_serverIp;

	quint16		m_serverPort;
	

	QMutex		mutex;
	int			m_numBytesForCurrentDataType;
	int			m_currentDataType;

	QByteArray *m_buffer;

	bool		m_quit;

};

#endif // CONNECTION_H

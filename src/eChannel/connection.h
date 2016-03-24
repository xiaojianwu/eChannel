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

	// ��ӷ�������
	void addSendTask(int type, int length, const std::string& data);
	int addSendTask(int type, int length, const char *data);

	// �ͷ�����
	void release();

	QString ServerIp() const { return m_serverIp; }
	quint16 ServerPort() const { return m_serverPort; }

	public slots:
		// ���ݴ���
		void onRead();

		// �������Ӵ�����
		void onError(QAbstractSocket::SocketError);


		void onConnected();
		// ����Ͽ�
		void onDisconnected();

signals:
		void error(QAbstractSocket::SocketError, QString);

		void recvData(int type, int length, const QByteArray& payload);

		void connected();

		void disconnected();


private:
	int parseHeader(); // ������ͷ
	bool hasEnoughData(); // �жϰ��Ƿ�����
	void processData(); // ��������



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

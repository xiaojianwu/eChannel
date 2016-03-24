#include "connection.h"

#include <QtEndian>
#include <QHostAddress>
#include <QBuffer>

const int Timeout = 5 * 1000;

typedef  struct _tagMGWHead
{
	quint16    cmd ;
	quint16    length;
	quint32    unused; 

	void Ntoh()
	{
		cmd = qToBigEndian(cmd);
		length = qToBigEndian(length);
	}
} MGWHead;

Connection::Connection(QObject *parent)
	//: QObject(parent)
{
	m_buffer = new QByteArray;
	m_quit = false;

	m_numBytesForCurrentDataType = -1;
	m_currentDataType = -1;
}

Connection::~Connection()
{
	m_quit = true;

	delete m_buffer;
}

void Connection::connectToHost(QString ip, quint16 port)
{
	m_serverIp = ip;
	m_serverPort = port;
	m_pSocket = new QTcpSocket();

	//! [2] //! [3]
	connect(m_pSocket, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(onRead()));
	//! [2] //! [4]
	connect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)),
		//! [3]
		this, SLOT(onError(QAbstractSocket::SocketError)));
	//! [4]
	QObject::connect(m_pSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

	m_pSocket->connectToHost(m_serverIp, m_serverPort);

	// 连接超时
	if (!m_pSocket->waitForConnected(Timeout)) {
		emit error(m_pSocket->error(), m_pSocket->errorString());
		return;
	}
}



void Connection::run()
{
	//m_pTimer = new QTimer;
	//// 启动定时器发送心跳
	//connect(m_pTimer, SIGNAL(timeout()), this, SLOT(doCheck()), Qt::QueuedConnection);
	//m_pTimer->start(Timeout);


	//exec();

	//m_pTimer->stop();
	//delete m_pTimer;
}

void Connection::release()
{
	qDebug(" Connection::release!");
	m_pSocket->disconnectFromHost();
}


// 添加发送任务
void Connection::addSendTask(int type, int length, const std::string& data)
{

	if (m_pSocket == NULL)
	{
		return;
	}

	QByteArray buf;
	buf.resize(sizeof(MGWHead));

	MGWHead head;
	head.cmd = type;
	head.length = length;
	head.unused = 0;
	head.Ntoh();
	memcpy(buf.data(), &head, sizeof(MGWHead));

	//qDebug() << "buf==" << buf.toHex();
	// 数据部分
	buf.append(data.c_str(), length);

	//qDebug() << "buf==" << buf.toHex();

	m_pSocket->write(buf);

}
// 添加发送任务
int Connection::addSendTask(int type, int length, const char *data)
{

	if (m_pSocket == NULL)
	{
		return -1;
	}
	QByteArray buf;
	buf.resize(sizeof(MGWHead));

	MGWHead head;
	head.cmd = type;
	head.length = length;
	head.unused = 0;
	head.Ntoh();
	memcpy(buf.data(), &head, sizeof(MGWHead));

	//qDebug() << "buf==" << buf.toHex();
	// 数据部分
	buf.append(data, length);

	//qDebug() << "buf==" << buf.toHex();
	return (int)m_pSocket->write(buf);
}



void Connection::onRead()
{
	if (!m_pSocket->isValid()) {
		qDebug() <<  "connection abort reason:" << "Connection::processReadyRead,isValid";
		abort();
		return;
	}

	QByteArray recvBuf = m_pSocket->readAll();
	if (recvBuf.size() <= 0)
	{
		return;
	}

	//m_pTimer->stop();
	mutex.lock();
	m_buffer->append(recvBuf);
	mutex.unlock();
	//m_pTimer->start();

	do {	
		if (!hasEnoughData())
		{
			return;
		}

		processData();
	} while (m_buffer->size() >= 8);
}

void Connection::onError(QAbstractSocket::SocketError error)
{

}


void  Connection::onConnected()
{
	emit connected();
}

void  Connection::onDisconnected()
{
	qDebug() << "Server disconnected!";


	disconnect(m_pSocket, SIGNAL(connected()), this, SLOT(onConnected()));
	disconnect(m_pSocket, SIGNAL(readyRead()), this, SLOT(onRead()));
	//! [2] //! [4]
	disconnect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)),
		//! [3]
		this, SLOT(onError(QAbstractSocket::SocketError)));
	//! [4]
	disconnect(m_pSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

	m_pSocket->deleteLater();

	emit disconnected();
}


// 获得报文长度
int Connection::parseHeader()
{
	if (m_buffer->size() < 8)
	{
		return 0;
	}

	// 前4个字节 commandtype(short:2bytes) + data length(short:2bytes)
	mutex.lock();
	m_currentDataType = byte2Int(m_buffer->left(2));
	m_numBytesForCurrentDataType = byte2Int(m_buffer->mid(2, 2));


	//qDebug() << "parseHeader orgi hex=" << m_buffer->left(4).toHex();
	// 去除头（含保留4字节）
	m_buffer->remove(0, 8);
	mutex.unlock();

	//qDebug() << "m_currentDataType=" << m_currentDataType << ",m_numBytesForCurrentDataType=" << m_numBytesForCurrentDataType;
	return m_numBytesForCurrentDataType;
}

// 判断包是否完整
bool Connection::hasEnoughData()
{
	if (m_numBytesForCurrentDataType < 0)
		parseHeader();

	if (m_buffer->size() < m_numBytesForCurrentDataType
		|| m_numBytesForCurrentDataType < 0) {
			return false;
	}

	return true;
}

// 包处理
void Connection::processData()
{
	mutex.lock();
	QByteArray payload = m_buffer->left(m_numBytesForCurrentDataType);
	m_buffer->remove(0, m_numBytesForCurrentDataType);
	mutex.unlock();

	//qDebug() <<  "Connection::processData: bytes:" << m_numBytesForCurrentDataType << ",type=" << m_currentDataType << ",payload=" << payload.toHex();

	emit recvData(m_currentDataType, m_numBytesForCurrentDataType, payload);

	m_numBytesForCurrentDataType = -1;
	m_currentDataType = -1;
}


quint16 Connection::byte2Int(const QByteArray& data)
{
	const uchar *num = reinterpret_cast<const uchar *>(data.constData());
	quint16 len = qFromBigEndian<quint16>(num);
	return len;
}

QByteArray Connection::int2Byte(quint16 num)
{
	QByteArray result;
	quint16 netNo = qToBigEndian<quint16>(num);

	//qDebug() << "convert no==" << netNo;


	result.append((char*)&netNo, 2);

	//qDebug() << "convert no hex==" << result.toHex();
	return result;
}






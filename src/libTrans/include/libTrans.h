#ifndef TRANSFER_INTERFACE_H
#define TRANSFER_INTERFACE_H

/*!
 * \file libTrans.h
 * \date 2016/03/28 16:41
 *
 * \author wuxiaojian
 * Contact: user@company.com
 *
 * \brief 
 *
 * 传输层接口定义
 *
 * \note
*/
#include <QString>

#include <QObject>

class Channel : public QObject
{
	Q_OBJECT
public:
    enum ERROR_CODE {
        ERROR_CODE_NE_FAILED = 1, // 协商失败
        ERROR_CODE_CONNECT_FAILED // 连接错误（主动断开或被动断开）
    };

	//virtual ~Channel();

    /**
	* @brief  发送业务数据
	* @param data  数据内容
	* @return int 写入大小
	*/
	virtual int write(const QByteArray& data) = 0;

    /**
	* @brief  关闭当前传输通道
	* @return void
	*/
	virtual void close() = 0;

    /**
	* @brief  当前传输通道是否通过中转传输
	* @return bool true:中转 false:p2p直传
	*/
	virtual bool isRelay() = 0;

    /**
	* @brief  获取本地的SDP信息，需要传送给对方
	* @param sessionId  Session的唯一ID
	* @return QString 本地SDP信息序列化信息
	*/
	virtual QString getPeerAddress() = 0;


    /**
	* @brief  获取本地的SDP信息，需要传送给对方
	* @return QString 本地SDP信息序列化信息
	*/
	virtual QString getLocalSDP() = 0;

    /**
	* @brief  设置或更新对方的SDP信息,设置完后即开始进行ICE协商，双方数据收发成功后返回
    * @param remoteSDP  对方SDP信息序列化信息
	* @return void
	*/
	virtual void updateRemoteSDP(QString remoteSDP) = 0;


    /**
	* @brief  判断当前通道是否初始化完成
	* @return bool 是否初始化完成
	*/
    virtual bool isInitialized() = 0;


    /**
	* @brief  判断是否连接成功
	* @return bool
	*/
    virtual bool isConnected() = 0;
	
signals:

    /**
	* @brief  ICE初始化成功，必须收到该信号后才可以调用获取SDP的接口
	* @return void
	*/
    void initialized();

    /**
	* @brief  连接成功
	* @return void
	*/
	void connected();

    /**
	* @brief  接受到的可靠数据
	* @param data  数据内容
	* @return void
	*/
	void readyRead(const QByteArray& data);

    /**
	* @brief  连接发生错误
    * @param code  错误编码
	* @param msg  错误消息内容
	* @return void
	*/
	void error(int code, QString msg);

    /**
	* @brief  通道空闲，可以继续发送数据
	* @return void
	*/
    void idle();
};

//! [0]
class TransferInterface : public QObject
{
	Q_OBJECT

public:
	TransferInterface() {}
	virtual ~TransferInterface() {}

    /**
	* @brief  传输层接口初始化，该接口只调用一次
	* @param turnserver  标准rfc-5766的服务器，示例：turnserver.trademessenger.com; 192.168.17.153；192.168.17.153:80
	* @param logFile  日志文件
    * @param uName turn 的用户名称，default:test
    * @param passwd turn用户的用户秘密，default:test
    * @param reaml turn的realm信息,default:www.vemic.com
	* @return void
	*/
	virtual void init(QString turnserver, QString logFile, const QString &uName = "test",const QString &passwd = "test",const QString &realm = "www.vemic.com") = 0;

    /**
	* @brief  获取当前设置的服务器地址
	* @return QString 服务器地址
	*/
    virtual QString getTurnServer() = 0;

    /**
	* @brief  创建一个Session， 返回sessionID
	* @param sessionId  Session的唯一ID，传空是自动获取，非空时是与对方的传输通道保持一致
	* @return QString 唯一的SessioID
	*/
	virtual QString initSession(QString sessionId = "") = 0;

    /**
	* @brief  获取可用的传输通道
	* @param sessionId  Session的唯一ID
	* @return Channel* 传输通道
	*/
    virtual Channel* getChannel(QString sessionId) = 0;
};


QT_BEGIN_NAMESPACE

#define TransferInterface_iid "com.focuschina.TransInterface"

	Q_DECLARE_INTERFACE(TransferInterface, TransferInterface_iid)
	QT_END_NAMESPACE

	//! [0]
#endif

#ifndef TRANSFER_INSTANCE_H
#define TRANSFER_INSTANCE_H

#include <QObject>
#include <QQueue>

#include <QtPlugin>
#include "libTrans.h"

class TransferInstance : public TransferInterface
{
	Q_OBJECT
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
	Q_PLUGIN_METADATA(IID "com.focuschina.TransInterface" FILE "libTrans.json")
#endif
		
		Q_INTERFACES(TransferInterface)

public:
	TransferInstance();
	~TransferInstance();

public:
	virtual void init(QString turnserver, QString logFile, const QString &uName = "test",const QString &passwd = "test",const QString &realm = "www.vemic.com");

    virtual QString getTurnServer();


	// 创建一个Session， 返回sessionID
	// Session由发送方生成，接受方需要传入发送方生成的SessionId
	virtual QString initSession(QString sessionId = "");

    /**
	* @brief  获取可用的传输通道
	* @param sessionId  Session的唯一ID
	* @return Channel* 传输通道
	*/
    virtual Channel* getChannel(QString sessionId);


private:

    QString         m_turnServer;
};

#endif // TRANSFER_INSTANCE_H

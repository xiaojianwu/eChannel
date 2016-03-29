#ifndef RDP_INTERFACE_H
#define RDP_INTERFACE_H

/*!
 * \file libRDP.h
 * \date 2016/03/28 16:37
 *
 * \author wuxiaojian
 * Contact: wuxiaojian@focusteach.com
 *
 * \brief 
 *
 * remote desktop interface
 *
 * \note
*/
#include <libTrans.h>

//! [0]
class RDPInterface : public QObject
{
	Q_OBJECT

public:
	//RDPInterface(QObject *parent) : QObject(parent) {}
	virtual ~RDPInterface() {}


    enum RDP_ERROR_CODE {
        RDP_ERROR_PROCESS_EXIT = 1, // viewer进程退出
        RDP_ERROR_SIM_SOCKET_DISCONNECT, // 模拟socket异常
    };

    /**
	* @brief  远程桌面库初始化
	* @param pInterface  传输层接口指针
    * @param vncViewerPath  viewer程序的目录
    * @param vncPort  vnc服务的端口
    * @param logFilePath  日志文件路径
	* @return void
	*/
	virtual void init(TransferInterface* pInterface, QString vncViewerPath, int vncPort, QString logFilePath) = 0;


    /**
	* @brief  获取viewer程序的目录
	* @return QString viewer程序的目录
	*/
    virtual QString getVncViewerPath() = 0;

    /**
	* @brief  获取vnc服务的端口
	* @return int vnc服务的端口
	*/
    virtual int getVncPort() = 0;


    /**
	* @brief  初始化通道
	* @param sessionId  Session的唯一ID
    * @param viewerArgs  viewer进程参数
	* @return void
	*/
    virtual void initChannel(QString sessionId, QString viewerArgs) = 0;

    /**
	* @brief  发送协助请求
	* @param sessionId  Session的唯一ID
	* @return void
	*/
	virtual void sendRDPAssitant(QString sessionId) = 0;

    /**
	* @brief  接受协助请求
	* @param sessionId  Session的唯一ID
	* @return void
	*/
    virtual void recvRDPAssitant(QString sessionId) = 0;

    /**
	* @brief  发送控制请求
	* @param sessionId  Session的唯一ID
	* @return void
	*/
	virtual void sendRDPControl(QString sessionId) = 0;

    /**
	* @brief  接受控制请求
	* @param sessionId  Session的唯一ID
	* @return void
	*/
	virtual void recvRDPControl(QString sessionId) = 0;

    /**
	* @brief  主动结束会话（被动不要调用此接口）
	* @param sessionId  Session的唯一ID
	* @return void
	*/
	virtual void terminate(QString sessionId) = 0;

signals:
    /**
	* @brief  远程桌面会话发生错误
	* @param sessionId  Session的唯一ID
    * @param code  错误编码
    * @param errorMsg  错误内容
	* @return void
	*/
	void error(QString sessionId, int code, QString errorMsg);
};


QT_BEGIN_NAMESPACE

#define RDPInterface_iid "com.focuschina.RDPInterface"

	Q_DECLARE_INTERFACE(RDPInterface, RDPInterface_iid)
	QT_END_NAMESPACE

	//! [0]
#endif

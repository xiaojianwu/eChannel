/**
*	@filename	PJICESocket.h
*	@Copyright (C), 2016-2027.
*	@brief	pjsip ice interface file
*	@author	zhuyongwen
*	@version	0.2.1
*	@date	2016/02/17
*	@history
*	YZ	2016/02/17	init
*/

#ifndef PJICESocket_H
#define PJICESocket_H
#include "libtrans_global.h"

#include <QObject>
#include <string>
namespace FocusIce { 

class ICE_Session ;

class PJICESocketAbstract:public QObject {
public:
	 PJICESocketAbstract(QObject *parent):QObject(parent){}
	 virtual ~PJICESocketAbstract()  {} ;
public:
	 virtual void iceInitResult(bool) =0;
	 virtual void iceNegoResult(bool,bool,const std::string& connIpstr ) =0 ;
	 virtual void dataRead(void *pkt,size_t size,unsigned compId,const char* ipstr) =0;
	 virtual ICE_Session* sessionInfo()=0;
};

class PJICESocket : public PJICESocketAbstract
{
	Q_OBJECT
public:

	/**
	* @brief  构造PJICE socket，该socket 需要指定一个socket的名字，方便跟踪；turnsrv 是标准的rfc5766服务器，如果存在端口请参考：
				turnserver.trademessenger.com或者192.168.17.153,或者使用冒号指定端口，如：192.168.17.153:80
	* @param parent 父窗口对象的指针
	* @param Name  socket的名字，任意指定，方便跟踪
	*
	* @return QString sdp描述信息，详细请参考sdp协议。
	*/
	PJICESocket(QObject *parent,const QString &Name );
	~PJICESocket();	

	/**
	* @brief 设置socket的发送使用的缓冲区大小
	* @param size   需要设置的使用本socket的发送缓冲区大小
	* @note	  该函数需要在链接turnsrv以前使用
	*/
	void		setSndBufSize(unsigned size) {_soSndSize = size ;}

	/**
	* @brief 设置socket的接收数据使用的缓冲区大小
	* @param size   需要设置的使用本socket的接收缓冲区大小
	* @note	  该函数需要在链接turnsrv以前使用
	*/
	void    setRecvBufSize(unsigned size) {_soRecvSize=size ;}


	/**
	* @brief 设置socket的发送使用的缓冲区大小
	* @param turnsrv 标准rfc-5766的服务器，示例：turnserver.trademessenger.com；192.168.17.153；192.168.17.153:80
	*/
	void		startICESocket(const QString &turnsrv);


	/**
	* @brief 获取到本地的sdp描述信息。
	* @return QString sdp描述信息，详细请参考sdp协议。
	*/
	QString  localSdp(bool isControl);

	/**
	* @brief ICE Session 开始协商，协商结果由信号带回，注意如果ice框架没有初始化成功，这不会进行协商
	* @param remoteSdp   远端sdp的协商内容。	
	*/
	void		negoWith(const QString &remoteSdp ) ;
	

	/**
	* @brief ICE Session 发送数据
	* @param pkt	指向待发送到数据指针
	* @param pktLen 需要发送到数据长度
	* @comp_id ice协商后会存在多个组成（比如rtp=0，rtcp=1），默认情况下使用0发送数据
	*/
	void iceSentTo(void  *pkt,size_t pktLen,int comp_id=1) ;
	
protected :   
	/** 
	the below function only invote by the pjice core , please do not call them
	*/

	/**
	* @brief ICE Session 初始化结果
	* @param success 初始化是否成功，如果没有成功，后续操作不允许
	*/
	void		iceInitResult(bool success) ;

	/**
	* @brief ICE Session ICE 协商初始化结果通知
	* @param success 协商是否成功，如果协商没有成功，则ICE不通，无法使用ICE发送和接受数据
	*/
	void		iceNegoResult(bool success,bool isRelay,const std::string &ipstr ) ;
	/**
	* @brief 返回当前对象所包含的ICE对象
	* @return	ICE_Session 的对象指针
	*/
	ICE_Session *	sessionInfo() {return _iceInfo ;}

	/**
	* @brief ICE Session ICE 协商初始化结果通知
	* @param success 协商是否成功，如果协商没有成功，怎么ICE不通，无法使用ICE 发送和接受数据
	*/
	void		dataRead(void *pkt,size_t size,unsigned compId,const char* ipstr) ;

signals:	
	/**
	* @brief		ice框架初始化session的结果信号
	* @param success 表示由初始化操作返回，还是非初始化返回，true表示session 初始化成功，反之则失败	
	*/
	void		cbInitResult(bool success ) ;
	/**
	* @brief ice框架Session 和对端协商的结果
	* @param success true表示session和对端协商成功，可以发送数据，反之则协商失败，用户无法发送数据
	* @param isRelay true表示协商通道使用中继服务器，反正使用udp穿透（包括：直连，穿透等）
	* @param connIpstr 对端的ip和port信息，比如：192.178.17.21：5060,(note:如果 success为false，connIpstr可能为空）
	*/
	void		cbNegoResult(bool success,bool isRelay ,const QString& connIpstr);

	/**
	* @brief cbReadData 信号收到数据 
	* @param pktData 收到的元素数据，udp的话可能会是一个完整的数据包，tcp需要分包和封包。
	* @param compId,使用的是rtp还是rtcp接受到的数据，一般使用创建一个，这可以忽略。
	* @param ipstr  发送者的ip和端口信息
	*/
	void		cbReadData(const QByteArray &pktData,  unsigned compId,const QString& ipstr);

public:
	/**
	* @brief pjnat静态函数，用于设置pjnat ice框架的打印日志的指定文件，函数如果未被调用过，dll会在当前目录下创建名字为“PJICE.log”日志输出文件
	* @param QString &logFile	 日志文件的名字和目录
	*/
	static void	setIceLogfile(const QString &logFile) ;


	/**
	* @brief 设置turnserver的鉴权，加密信息
	* @param uName turn 的用户名称。
	* @param passwd turn用户的用户秘密。
	* @param reaml turn的realm信息。
	* @note  本静态函数如果需要单独设置，请在初始化启动的时候设置，如果不设置，pjice将使用焦点默认设置。
	*/
	static void	setIceTurnAuth(const QString &uName,const QString &passwd,const QString &realm ) ;


private:
	QString		  _name;
	ICE_Session 	* _iceInfo  ;
	bool			  _sInit;
	bool			  _sNego;
	unsigned		  _soRecvSize;
	unsigned		  _soSndSize;
};
}

#endif // PJICESocket_H

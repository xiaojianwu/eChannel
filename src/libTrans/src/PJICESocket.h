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
	* @brief  ����PJICE socket����socket ��Ҫָ��һ��socket�����֣�������٣�turnsrv �Ǳ�׼��rfc5766��������������ڶ˿���ο���
				turnserver.trademessenger.com����192.168.17.153,����ʹ��ð��ָ���˿ڣ��磺192.168.17.153:80
	* @param parent �����ڶ����ָ��
	* @param Name  socket�����֣�����ָ�����������
	*
	* @return QString sdp������Ϣ����ϸ��ο�sdpЭ�顣
	*/
	PJICESocket(QObject *parent,const QString &Name );
	~PJICESocket();	

	/**
	* @brief ����socket�ķ���ʹ�õĻ�������С
	* @param size   ��Ҫ���õ�ʹ�ñ�socket�ķ��ͻ�������С
	* @note	  �ú�����Ҫ������turnsrv��ǰʹ��
	*/
	void		setSndBufSize(unsigned size) {_soSndSize = size ;}

	/**
	* @brief ����socket�Ľ�������ʹ�õĻ�������С
	* @param size   ��Ҫ���õ�ʹ�ñ�socket�Ľ��ջ�������С
	* @note	  �ú�����Ҫ������turnsrv��ǰʹ��
	*/
	void    setRecvBufSize(unsigned size) {_soRecvSize=size ;}


	/**
	* @brief ����socket�ķ���ʹ�õĻ�������С
	* @param turnsrv ��׼rfc-5766�ķ�������ʾ����turnserver.trademessenger.com��192.168.17.153��192.168.17.153:80
	*/
	void		startICESocket(const QString &turnsrv);


	/**
	* @brief ��ȡ�����ص�sdp������Ϣ��
	* @return QString sdp������Ϣ����ϸ��ο�sdpЭ�顣
	*/
	QString  localSdp(bool isControl);

	/**
	* @brief ICE Session ��ʼЭ�̣�Э�̽�����źŴ��أ�ע�����ice���û�г�ʼ���ɹ����ⲻ�����Э��
	* @param remoteSdp   Զ��sdp��Э�����ݡ�	
	*/
	void		negoWith(const QString &remoteSdp ) ;
	

	/**
	* @brief ICE Session ��������
	* @param pkt	ָ������͵�����ָ��
	* @param pktLen ��Ҫ���͵����ݳ���
	* @comp_id iceЭ�̺����ڶ����ɣ�����rtp=0��rtcp=1����Ĭ�������ʹ��0��������
	*/
	void iceSentTo(void  *pkt,size_t pktLen,int comp_id=1) ;
	
protected :   
	/** 
	the below function only invote by the pjice core , please do not call them
	*/

	/**
	* @brief ICE Session ��ʼ�����
	* @param success ��ʼ���Ƿ�ɹ������û�гɹ�����������������
	*/
	void		iceInitResult(bool success) ;

	/**
	* @brief ICE Session ICE Э�̳�ʼ�����֪ͨ
	* @param success Э���Ƿ�ɹ������Э��û�гɹ�����ICE��ͨ���޷�ʹ��ICE���ͺͽ�������
	*/
	void		iceNegoResult(bool success,bool isRelay,const std::string &ipstr ) ;
	/**
	* @brief ���ص�ǰ������������ICE����
	* @return	ICE_Session �Ķ���ָ��
	*/
	ICE_Session *	sessionInfo() {return _iceInfo ;}

	/**
	* @brief ICE Session ICE Э�̳�ʼ�����֪ͨ
	* @param success Э���Ƿ�ɹ������Э��û�гɹ�����ôICE��ͨ���޷�ʹ��ICE ���ͺͽ�������
	*/
	void		dataRead(void *pkt,size_t size,unsigned compId,const char* ipstr) ;

signals:	
	/**
	* @brief		ice��ܳ�ʼ��session�Ľ���ź�
	* @param success ��ʾ�ɳ�ʼ���������أ����Ƿǳ�ʼ�����أ�true��ʾsession ��ʼ���ɹ�����֮��ʧ��	
	*/
	void		cbInitResult(bool success ) ;
	/**
	* @brief ice���Session �ͶԶ�Э�̵Ľ��
	* @param success true��ʾsession�ͶԶ�Э�̳ɹ������Է������ݣ���֮��Э��ʧ�ܣ��û��޷���������
	* @param isRelay true��ʾЭ��ͨ��ʹ���м̷�����������ʹ��udp��͸��������ֱ������͸�ȣ�
	* @param connIpstr �Զ˵�ip��port��Ϣ�����磺192.178.17.21��5060,(note:��� successΪfalse��connIpstr����Ϊ�գ�
	*/
	void		cbNegoResult(bool success,bool isRelay ,const QString& connIpstr);

	/**
	* @brief cbReadData �ź��յ����� 
	* @param pktData �յ���Ԫ�����ݣ�udp�Ļ����ܻ���һ�����������ݰ���tcp��Ҫ�ְ��ͷ����
	* @param compId,ʹ�õ���rtp����rtcp���ܵ������ݣ�һ��ʹ�ô���һ��������Ժ��ԡ�
	* @param ipstr  �����ߵ�ip�Ͷ˿���Ϣ
	*/
	void		cbReadData(const QByteArray &pktData,  unsigned compId,const QString& ipstr);

public:
	/**
	* @brief pjnat��̬��������������pjnat ice��ܵĴ�ӡ��־��ָ���ļ����������δ�����ù���dll���ڵ�ǰĿ¼�´�������Ϊ��PJICE.log����־����ļ�
	* @param QString &logFile	 ��־�ļ������ֺ�Ŀ¼
	*/
	static void	setIceLogfile(const QString &logFile) ;


	/**
	* @brief ����turnserver�ļ�Ȩ��������Ϣ
	* @param uName turn ���û����ơ�
	* @param passwd turn�û����û����ܡ�
	* @param reaml turn��realm��Ϣ��
	* @note  ����̬���������Ҫ�������ã����ڳ�ʼ��������ʱ�����ã���������ã�pjice��ʹ�ý���Ĭ�����á�
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

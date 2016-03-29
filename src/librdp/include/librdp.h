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
        RDP_ERROR_PROCESS_EXIT = 1, // viewer�����˳�
        RDP_ERROR_SIM_SOCKET_DISCONNECT, // ģ��socket�쳣
    };

    /**
	* @brief  Զ��������ʼ��
	* @param pInterface  �����ӿ�ָ��
    * @param vncViewerPath  viewer�����Ŀ¼
    * @param vncPort  vnc����Ķ˿�
    * @param logFilePath  ��־�ļ�·��
	* @return void
	*/
	virtual void init(TransferInterface* pInterface, QString vncViewerPath, int vncPort, QString logFilePath) = 0;


    /**
	* @brief  ��ȡviewer�����Ŀ¼
	* @return QString viewer�����Ŀ¼
	*/
    virtual QString getVncViewerPath() = 0;

    /**
	* @brief  ��ȡvnc����Ķ˿�
	* @return int vnc����Ķ˿�
	*/
    virtual int getVncPort() = 0;


    /**
	* @brief  ��ʼ��ͨ��
	* @param sessionId  Session��ΨһID
    * @param viewerArgs  viewer���̲���
	* @return void
	*/
    virtual void initChannel(QString sessionId, QString viewerArgs) = 0;

    /**
	* @brief  ����Э������
	* @param sessionId  Session��ΨһID
	* @return void
	*/
	virtual void sendRDPAssitant(QString sessionId) = 0;

    /**
	* @brief  ����Э������
	* @param sessionId  Session��ΨһID
	* @return void
	*/
    virtual void recvRDPAssitant(QString sessionId) = 0;

    /**
	* @brief  ���Ϳ�������
	* @param sessionId  Session��ΨһID
	* @return void
	*/
	virtual void sendRDPControl(QString sessionId) = 0;

    /**
	* @brief  ���ܿ�������
	* @param sessionId  Session��ΨһID
	* @return void
	*/
	virtual void recvRDPControl(QString sessionId) = 0;

    /**
	* @brief  ���������Ự��������Ҫ���ô˽ӿڣ�
	* @param sessionId  Session��ΨһID
	* @return void
	*/
	virtual void terminate(QString sessionId) = 0;

signals:
    /**
	* @brief  Զ������Ự��������
	* @param sessionId  Session��ΨһID
    * @param code  �������
    * @param errorMsg  ��������
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

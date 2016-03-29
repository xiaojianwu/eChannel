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
 * �����ӿڶ���
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
        ERROR_CODE_NE_FAILED = 1, // Э��ʧ��
        ERROR_CODE_CONNECT_FAILED // ���Ӵ��������Ͽ��򱻶��Ͽ���
    };

	//virtual ~Channel();

    /**
	* @brief  ����ҵ������
	* @param data  ��������
	* @return int д���С
	*/
	virtual int write(const QByteArray& data) = 0;

    /**
	* @brief  �رյ�ǰ����ͨ��
	* @return void
	*/
	virtual void close() = 0;

    /**
	* @brief  ��ǰ����ͨ���Ƿ�ͨ����ת����
	* @return bool true:��ת false:p2pֱ��
	*/
	virtual bool isRelay() = 0;

    /**
	* @brief  ��ȡ���ص�SDP��Ϣ����Ҫ���͸��Է�
	* @param sessionId  Session��ΨһID
	* @return QString ����SDP��Ϣ���л���Ϣ
	*/
	virtual QString getPeerAddress() = 0;


    /**
	* @brief  ��ȡ���ص�SDP��Ϣ����Ҫ���͸��Է�
	* @return QString ����SDP��Ϣ���л���Ϣ
	*/
	virtual QString getLocalSDP() = 0;

    /**
	* @brief  ���û���¶Է���SDP��Ϣ,������󼴿�ʼ����ICEЭ�̣�˫�������շ��ɹ��󷵻�
    * @param remoteSDP  �Է�SDP��Ϣ���л���Ϣ
	* @return void
	*/
	virtual void updateRemoteSDP(QString remoteSDP) = 0;


    /**
	* @brief  �жϵ�ǰͨ���Ƿ��ʼ�����
	* @return bool �Ƿ��ʼ�����
	*/
    virtual bool isInitialized() = 0;


    /**
	* @brief  �ж��Ƿ����ӳɹ�
	* @return bool
	*/
    virtual bool isConnected() = 0;
	
signals:

    /**
	* @brief  ICE��ʼ���ɹ��������յ����źź�ſ��Ե��û�ȡSDP�Ľӿ�
	* @return void
	*/
    void initialized();

    /**
	* @brief  ���ӳɹ�
	* @return void
	*/
	void connected();

    /**
	* @brief  ���ܵ��Ŀɿ�����
	* @param data  ��������
	* @return void
	*/
	void readyRead(const QByteArray& data);

    /**
	* @brief  ���ӷ�������
    * @param code  �������
	* @param msg  ������Ϣ����
	* @return void
	*/
	void error(int code, QString msg);

    /**
	* @brief  ͨ�����У����Լ�����������
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
	* @brief  �����ӿڳ�ʼ�����ýӿ�ֻ����һ��
	* @param turnserver  ��׼rfc-5766�ķ�������ʾ����turnserver.trademessenger.com; 192.168.17.153��192.168.17.153:80
	* @param logFile  ��־�ļ�
    * @param uName turn ���û����ƣ�default:test
    * @param passwd turn�û����û����ܣ�default:test
    * @param reaml turn��realm��Ϣ,default:www.vemic.com
	* @return void
	*/
	virtual void init(QString turnserver, QString logFile, const QString &uName = "test",const QString &passwd = "test",const QString &realm = "www.vemic.com") = 0;

    /**
	* @brief  ��ȡ��ǰ���õķ�������ַ
	* @return QString ��������ַ
	*/
    virtual QString getTurnServer() = 0;

    /**
	* @brief  ����һ��Session�� ����sessionID
	* @param sessionId  Session��ΨһID���������Զ���ȡ���ǿ�ʱ����Է��Ĵ���ͨ������һ��
	* @return QString Ψһ��SessioID
	*/
	virtual QString initSession(QString sessionId = "") = 0;

    /**
	* @brief  ��ȡ���õĴ���ͨ��
	* @param sessionId  Session��ΨһID
	* @return Channel* ����ͨ��
	*/
    virtual Channel* getChannel(QString sessionId) = 0;
};


QT_BEGIN_NAMESPACE

#define TransferInterface_iid "com.focuschina.TransInterface"

	Q_DECLARE_INTERFACE(TransferInterface, TransferInterface_iid)
	QT_END_NAMESPACE

	//! [0]
#endif

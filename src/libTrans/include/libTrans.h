#ifndef TRANSFER_INTERFACE_H
#define TRANSFER_INTERFACE_H

#include <QString>

#include <QObject>

#include <QThread>




class Channel : public QObject
{
	Q_OBJECT
public:
    enum ERROR_CODE {
        ERROR_CODE_NE_FAILED = 1, // Э��ʧ��
        ERROR_CODE_CONNECT_FAILED, // ����ʧ��
        ERROR_CODE_OTHER, // ��������
    };

	//virtual ~Channel();

    /**
	* @brief  ����ҵ������
	* @param data  ��������
	* @return int д���С
	*/
	virtual int write(const QByteArray &data) = 0;

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
	* @brief  ���ܵ��Ŀɿ�����
	* @return void
	*/
    void initialized();

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
	* @brief  ���ӶϿ�
	* @return void
	*/
	void disconnected();

    /**
	* @brief  ͨ�����У����Լ�����������
	* @return void
	*/
    void idle();


    /**
	* @brief  ���ӳɹ�
	* @return void
	*/
	void connected();
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
	* @return void
	*/
	virtual void init(QString turnserver, QString logFile) = 0;

    /**
	* @brief  ��ȡ��ǰ���õķ�������ַ
	* @return QString ��������ַ
	*/
    virtual QString getTurnServer() = 0;


    /**
	* @brief  ���õ�ǰ�û�
    * @param currentUser  ��ǰ�û�
	* @return void
	*/
	virtual void updateCurrentUser(QString currentUser) = 0;


    /**
	* @brief  ����һ��Session�� ����sessionID
	* @param isControl  �Ƿ���ƶ�
	* @param sessionId  Session��ΨһID���������Զ���ȡ���ǿ�ʱ����Է��Ĵ���ͨ������һ��
	* @return QString Ψһ��SessioID
	*/
	virtual QString initSession(bool isControl, QString sessionId = "") = 0;

    /**
	* @brief  ��ȡ���õĴ���ͨ��
	* @param sessionId  Session��ΨһID
	* @return Channel* ����ͨ��
	*/
    virtual Channel* getChannel(QString sessionId) = 0;

    /**
	* @brief  ��ȡ��ǰ�û�
	* @return QString ��ǰ�û�
	*/
	virtual QString getCurrentUser() = 0;
};


QT_BEGIN_NAMESPACE

#define TransferInterface_iid "com.focuschina.TransInterface"

	Q_DECLARE_INTERFACE(TransferInterface, TransferInterface_iid)
	QT_END_NAMESPACE

	//! [0]
#endif

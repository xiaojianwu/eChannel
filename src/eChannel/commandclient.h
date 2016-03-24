#ifndef COMMANDCLIENT_H
#define COMMANDCLIENT_H

#include <QObject>
#include <QTimer>
#include "connection.h"

#include <QHash>
#include <QStringList>

#include "media.pb.h"

#include "cmdefines.h"


enum LoginResult {
	LOGIN_OK = 1,
	LOGIN_CONNECT_ERROR = 2,
	LOGIN_FAILED = 3,
};


enum RequestType {
	REQUEST_TYPE_FILE = 1,
	REQUEST_TYPE_AUDIO,
	REQUEST_TYPE_VIDEO,
};


enum HandleType {
	HANDLE_TYPE_ACCEPT = 1,
	HANDLE_TYPE_REJECT,
	HANDLE_TYPE_CANCEL,
};


class CommandClient : public QObject

{
	Q_OBJECT

public:
	static CommandClient* instance();

	void init();

private:
	static CommandClient* inst;

protected:
	CommandClient(QObject *parent = NULL);
	~CommandClient();


private slots:
	// �յ����Ľ���
	void onRecvData(int type, int length, const QByteArray& payload);

	void onConnected();

	void onDisConnected();

	void onError(QAbstractSocket::SocketError, QString);


	void sendHeartbeat();
	
private:
	// ע�᷵�ؽ��
	void onRecvResiger(const QByteArray& payload);

	// �յ�����
	void onRecvInvite(const QByteArray& payload);

	// �յ����룬������Ӧ�ظ�
	void onRecvInviteResp(const QByteArray& payload);


	// �յ����������Ӧ
	void onRecvBye(const QByteArray& payload);

	// �յ����������Ӧ
	void onRecvByeResp(const QByteArray& payload);
	

	void onRecvTurnIsOK(const QByteArray& payload);

	// �յ�����
	void onRecvRing(const MGC::InviteReq& request);

	// �յ�����
	void onRecvAccept(const MGC::InviteReq& request);

	// �յ��ܾ�
	void onRecvReject(const MGC::InviteReq& request);

public:
	// ��ʼ��
	void login(QString ip, quint16 port, QString username);

	// ��¼ע��
	void login(QString username);

	// ����
	QString invite(QString uuid, QString touser, int mediaType, std::string sdp, QString mediaInfo = "");

	// ��������
	void accept(QString uuid, std::string sdp);

	// �ܾ�����
	void reject(QString uuid);

	// �����Ự
	void byebye(QString uuid);

	// �����Ự
	void byeResp(QString uuid);

	const QString& getCurrentUser() {return  m_currentUser;}

    // ��������
    void ring(QString uuid, std::string sdp);

private:

	void sendInvite(QString uuid, QString to, QString mediainfo, std::string sdp);

	void connectToHost();
	// ͨ��˽Կע��
	void loginWithAuth();



	// ��ȡ����
	//std::string getLocalCandicateString();

	// ����������Ϣ
	void parseRequest(const MGC::InviteReq& request);

	// ��װ���뷵������
	std::string genReponse(const QString& uuid, InviteCodeType code, std::string sdp);

	std::string encode(const std::string &src ) ;

signals:
	// MGC��¼�ɹ�
	void onLoginResult(LoginResult result);

	//void onError(ERRORTYPE errortype, QString errorMsg);

	// �յ�����
	void onInvite(QString fromuser, RequestType type, QString mediainfo, QString uuid, QString sdp);

	// �Է����壬�ȴ���
	void onRing(QString uuid, QString from, QString sdp);

	// �Է�����
	void onAccept(QString uuid, QString from);

	// �Է��ܾ�����
	void onReject(QString uuid, QString from);

	// �Է��ж�
	void onTerminate(QString uuid);

	// ���ӳɹ�
	void onConnected(QString uuid, int transType);

	// ���ؽ���
	void downloadProgress(QString uuid, qint64 bytesReceived, qint64 bytesTotal, int blockSize);

	// �ϴ�����
	void uploadProgress(QString uuid, qint64 bytesSent, qint64 bytesTotal, int blockSize);

	// �ļ��������
	void onFileComplete(QString uuid);

	// ȡ���ж�
	void onCancel(QString uuid);


	// δ����
	void onNotFound(QString uuid);

private:
	Connection*		 m_pConnection;

	std::string		m_privateKey;

	QString			m_serverip;
	quint16			m_port;

	std::string		m_domain;
	QString		m_currentUser;

	std::string		m_from;
	std::string		m_to;
	int m_mediaType;
	std::string    m_uuid;
	std::string    m_mediainfo;
    QString        m_remotesdp;

	QTimer *timer;

	bool			m_isRegisterOK;

	QStringList    m_turnList;
};

#endif // COMMANDCLIENT_H

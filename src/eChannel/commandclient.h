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
	// 收到报文解析
	void onRecvData(int type, int length, const QByteArray& payload);

	void onConnected();

	void onDisConnected();

	void onError(QAbstractSocket::SocketError, QString);


	void sendHeartbeat();
	
private:
	// 注册返回结果
	void onRecvResiger(const QByteArray& payload);

	// 收到邀请
	void onRecvInvite(const QByteArray& payload);

	// 收到邀请，邀请响应回复
	void onRecvInviteResp(const QByteArray& payload);


	// 收到解除连接响应
	void onRecvBye(const QByteArray& payload);

	// 收到解除连接响应
	void onRecvByeResp(const QByteArray& payload);
	

	void onRecvTurnIsOK(const QByteArray& payload);

	// 收到响铃
	void onRecvRing(const MGC::InviteReq& request);

	// 收到接受
	void onRecvAccept(const MGC::InviteReq& request);

	// 收到拒绝
	void onRecvReject(const MGC::InviteReq& request);

public:
	// 初始化
	void login(QString ip, quint16 port, QString username);

	// 登录注册
	void login(QString username);

	// 邀请
	QString invite(QString uuid, QString touser, int mediaType, std::string sdp, QString mediaInfo = "");

	// 接受邀请
	void accept(QString uuid, std::string sdp);

	// 拒绝邀请
	void reject(QString uuid);

	// 结束会话
	void byebye(QString uuid);

	// 结束会话
	void byeResp(QString uuid);

	const QString& getCurrentUser() {return  m_currentUser;}

    // 发送响铃
    void ring(QString uuid, std::string sdp);

private:

	void sendInvite(QString uuid, QString to, QString mediainfo, std::string sdp);

	void connectToHost();
	// 通过私钥注册
	void loginWithAuth();



	// 获取本地
	//std::string getLocalCandicateString();

	// 解析请求信息
	void parseRequest(const MGC::InviteReq& request);

	// 组装邀请返回数据
	std::string genReponse(const QString& uuid, InviteCodeType code, std::string sdp);

	std::string encode(const std::string &src ) ;

signals:
	// MGC登录成功
	void onLoginResult(LoginResult result);

	//void onError(ERRORTYPE errortype, QString errorMsg);

	// 收到邀请
	void onInvite(QString fromuser, RequestType type, QString mediainfo, QString uuid, QString sdp);

	// 对方响铃，等待中
	void onRing(QString uuid, QString from, QString sdp);

	// 对方接受
	void onAccept(QString uuid, QString from);

	// 对方拒绝接受
	void onReject(QString uuid, QString from);

	// 对方中断
	void onTerminate(QString uuid);

	// 连接成功
	void onConnected(QString uuid, int transType);

	// 下载进度
	void downloadProgress(QString uuid, qint64 bytesReceived, qint64 bytesTotal, int blockSize);

	// 上传进度
	void uploadProgress(QString uuid, qint64 bytesSent, qint64 bytesTotal, int blockSize);

	// 文件传输完毕
	void onFileComplete(QString uuid);

	// 取消中断
	void onCancel(QString uuid);


	// 未发现
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

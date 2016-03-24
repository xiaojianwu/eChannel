#include "commandclient.h"
#include <QThreadPool>

#include "cmdefines.h"
#include "errorcode.h"

#include <QUuid>

#include <QFileInfo>


#include "commondefine.h"
#include "base64url.h"


// ������������ĻỰ����
CommandClient::CommandClient(QObject *parent)
	: QObject(parent)
{
	m_isRegisterOK = false;
}

CommandClient::~CommandClient()
{

}



CommandClient *CommandClient::inst = 0;
CommandClient *CommandClient::instance()
{
	if (!inst)
	{
		inst = new CommandClient;
	}
	return inst;
}


void CommandClient::init()
{
}
// ��ʼ��
void CommandClient::login(QString ip, quint16 port, QString username)
{
	//m_inteface = inteface;
	m_serverip = ip;
	m_port = port;
	m_currentUser = username;

	connectToHost();

}

void CommandClient::connectToHost()
{
	m_pConnection = new Connection(this);

	connect(m_pConnection, SIGNAL(recvData(int, int, const QByteArray&)), this, SLOT(onRecvData(int, int, const QByteArray&)));
	connect(m_pConnection, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(m_pConnection, SIGNAL(disconnected()), this, SLOT(onDisConnected()));
	connect(m_pConnection, SIGNAL(error(QAbstractSocket::SocketError, QString)), this, SLOT(onError(QAbstractSocket::SocketError, QString)));

	m_pConnection->connectToHost(m_serverip, m_port);
}


void CommandClient::onError(QAbstractSocket::SocketError errorcode, QString msg)
{
	qDebug() << "CommandClient::onError=" << errorcode << msg;

	m_isRegisterOK = false;
	if (m_pConnection)
	{
		m_pConnection->release();
		//emit onError(ERRORTYPE_NETWORK, msg);
	}
}

void CommandClient::onConnected()
{
	qDebug() << "CommandClient::onConnected";
	// ������ʱ����������
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(sendHeartbeat()));
	timer->start(10000); //10s


	login(m_currentUser);
}

void CommandClient::onDisConnected()
{
	m_isRegisterOK = false;
	timer->stop();
	delete timer;

	delete m_pConnection;
	m_pConnection = NULL;

	connectToHost();
}

// ��������
void CommandClient::sendHeartbeat()
{
	m_pConnection->addSendTask(MGC_COMMAND_HEARTBEAT, 0, "");
}


// �յ����Ľ���
void CommandClient::onRecvData(int type, int length, const QByteArray& payload)
{

	qDebug() << "CommandClient::onRecvData.type=" << type << "length=" << length;
	switch (type)
	{
	case MGC_COMMAND_REGITER_RESP:
		onRecvResiger(payload);
		break;
	case MGC_COMMAND_INVITE:
		onRecvInvite(payload);
		break;
	case MGC_COMMAND_INVITE_RESP:
		onRecvInviteResp(payload);
		break;
	case MGC_COMMAND_BYE_REQ:
		onRecvBye(payload);
		break;
	case MGC_COMMAND_BYE_RESP:
		onRecvByeResp(payload);
		break;
	default:
		break;
	}
}

// ע�᷵�ؽ��
void CommandClient::onRecvResiger(const QByteArray& payload)
{
	qDebug() << "CommandClient::onRecvResiger";
	MGC::RegisterResp info;
	info.ParseFromArray(payload.data(), payload.size());

	switch(info.code())
	{
	case MGC_REGISTER_CODE_OK:
		
		{
			m_isRegisterOK = true;
			emit onLoginResult(LOGIN_OK);
			qDebug() << "MGC_REGISTER_CODE_OK";
		}
		break;
	case MGC_REGISTER_CODE_UNAUTH:
		{
			qDebug() << "MGC_REGISTER_CODE_UNAUTH";
			m_privateKey = info.privatekey();
			loginWithAuth();
		}

		break;
	case MGC_REGISTER_CODE_FAIL:
		{
			qDebug() << "MGC_REGISTER_CODE_FAIL";
		}
		break;
	default:
		break;
	}
}

// �յ�����
void CommandClient::onRecvInvite(const QByteArray& payload)
{
	MGC::InviteReq request;
	request.ParseFromArray(payload, payload.size());

	parseRequest(request);

	// ֪ͨ��������������
	emit onInvite(QString::fromStdString(m_from), REQUEST_TYPE_FILE, QString::fromStdString(m_mediainfo), QString::fromStdString(m_uuid), m_remotesdp);
}



// �յ����룬������
void CommandClient::onRecvInviteResp(const QByteArray& payload)
{
	MGC::InviteResp info;

	info.ParseFromArray(payload, payload.size());

	const MGC::InviteReq& request = info.request();

	switch(info.code())
	{
	case MGC_CODE_NOT_FOUND:
		emit onNotFound(QString::fromStdString(request.suuid()));
		break;
		// ����ȴ���
	case MGC_CODE_RING:
		onRecvRing(request);
		break;
	case MGC_CODE_ACCEPT:
		onRecvAccept(request);
		break;
	case MGC_CODE_REJECT:
		onRecvReject(request);
		break;
	default:
		break;
	}
}

// �յ����������Ӧ
void CommandClient::onRecvBye(const QByteArray& payload)
{
	MGC::ByeReq info;
	info.ParseFromArray(payload, payload.size());

	QString uuid = QString::fromStdString(info.suuid());
	emit onTerminate(uuid);

	qDebug() << "CommandClient::onRecvBye uuid=" << uuid;
}

// �յ����������Ӧ
void CommandClient::onRecvByeResp(const QByteArray& payload)
{
	MGC::ByeResp info;

	info.ParseFromArray(payload, payload.size());

	const MGC::ByeReq& request = info.request();

	QString uuid = QString::fromStdString(request.suuid());

	int code = info.code();
	// TODO:Code����

	emit onTerminate(uuid);

	qDebug() << "CommandClient::onRecvByeResp uuid=" << uuid;
}

// �յ�����
void CommandClient::onRecvRing(const MGC::InviteReq& request)
{
	parseRequest(request);

	// ֪ͨ������������Է����յ�
	emit onRing(QString::fromStdString(m_uuid), QString::fromStdString(m_from), m_remotesdp);
}

// �յ�����
void CommandClient::onRecvAccept(const MGC::InviteReq& request)
{
	parseRequest(request);

	QString uuid = QString::fromStdString(request.suuid()); 

	// ֪ͨ����
	emit onAccept(uuid, QString::fromStdString(request.from()));
}

// �յ��ܾ�
void CommandClient::onRecvReject(const MGC::InviteReq& request)
{
	// ���ͨ��
	QString uuid = QString::fromStdString(request.suuid()); 

	// δ��ʼ����
	//m_p2pinterface->terminate(uuid);
}


// ��¼ע��
void CommandClient::login(QString username)
{
	qDebug() << "CommandClient::login";
	m_currentUser = username;

	MGC::Register regReq;
	regReq.set_from(m_currentUser.toStdString());
	
	std::string buf = regReq.SerializeAsString();

	m_pConnection->addSendTask(MGC_COMMAND_REGISTER, buf.size(), buf);
}

// ����
QString CommandClient::invite(QString uuid, QString touser, int mediaType, std::string sdp, QString mediaInfo)
{
	// ��������
	sendInvite(uuid, touser, mediaInfo, sdp);

	return uuid;

}

void CommandClient::sendInvite(QString uuid, QString to, QString mediainfo, std::string sdp)
{
	m_from = m_currentUser.toStdString();
	m_to = to.toStdString();

	MGC::InviteReq request;
	request.set_from(m_from);
	request.set_to(m_to);
	request.set_media(MEDIA_TYPE_FILE);
	request.set_mediainfo(mediainfo.toStdString());
	request.set_suuid(uuid.toStdString());

	std::string candicateinfo = sdp;

	request.set_mediaaddrs(candicateinfo);

	std::string buf = request.SerializeAsString();

	m_pConnection->addSendTask(MGC_COMMAND_INVITE, buf.size(), buf);
}


// ��������
void CommandClient::ring(QString uuid, std::string sdp)
{

	qDebug() << "CommandClient::ring";
	// ��������
	std::string buf = genReponse(uuid, MGC_CODE_RING, sdp);

	m_pConnection->addSendTask(MGC_COMMAND_INVITE_RESP, buf.size(), buf);
}

// ��������
void CommandClient::accept(QString uuid, std::string sdp)
{
	std::string buf = genReponse(uuid, MGC_CODE_ACCEPT, sdp);

	m_pConnection->addSendTask(MGC_COMMAND_INVITE_RESP, buf.size(), buf);
}

// �ܾ�����
void CommandClient::reject(QString uuid)
{
	std::string buf = genReponse(uuid, MGC_CODE_REJECT, "");

	m_pConnection->addSendTask(MGC_COMMAND_INVITE_RESP, buf.size(), buf);

	//SessionMgr::instance()->removeSession(uuid);
}

// �����Ự
void CommandClient::byebye(QString uuid)
{
}

// �����Ự
void CommandClient::byeResp(QString uuid)
{
}

// ����������Ϣ
void CommandClient::parseRequest(const MGC::InviteReq& request)
{
	m_from = request.from();
	m_to = request.to();
	m_mediaType =request.media();
	m_mediainfo = request.mediainfo();
	m_uuid = request.suuid(); 

    m_remotesdp = QString::fromStdString(request.mediaaddrs());

	m_turnList.clear();
	for(int i =0 ;i<request.turnaddr_size();i++){
		QString turnAddr = QString("%1:%2").arg(QString::fromStdString(request.turnaddr(i).addr())).arg(request.turnaddr(i).port());
		m_turnList.append(turnAddr);
	}	
}


// ��װ���뷵������
std::string CommandClient::genReponse(const QString& uuid, InviteCodeType code, std::string sdp)
{
	MGC::InviteResp resp;

	MGC::InviteReq* request = resp.mutable_request();
	
	request->set_from(m_from);
	request->set_to(m_to);
	request->set_media(m_mediaType);
	request->set_mediainfo(m_mediainfo);
	request->set_suuid(m_uuid);
    request->set_mediaaddrs(sdp);
	

	// �ش�turn��ַ
	for (int i = 0; i < m_turnList.size(); i++)
	{
		MGC::TurnRelay* turnRelay = request->add_turnaddr();
		QStringList turnaddr = m_turnList.at(i).split(":");
		turnRelay->set_addr(turnaddr[0].toStdString());
		turnRelay->set_port(turnaddr[1].toShort());
	}
	
	resp.set_code(code);

	std::string buf = resp.SerializeAsString();

	return buf;
}

std::string CommandClient::encode(const std::string &src ) 
{ 
	char password[] = "val:FocusChinaMediaGateway2014"; 
	int passlen = strlen(password); 
	std::string dests ; 
	dests.resize(src.length()); 
	for(int i=0; i<src.length(); i++) 
	{ 
		dests[i] =( src[i]^password[i%passlen] ); 
	} 
	base64url b64u ; 
	std::string b64str = b64u.encoding(dests.data(),dests.size(),true); 
	return b64str; 
}


// ͨ��˽Կע��
void CommandClient::loginWithAuth()
{
	qDebug() << "CommandClient::loginWithAuth";
	MGC::Register regReq;
	regReq.set_from(m_currentUser.toStdString());

	// ���ܴ���
	regReq.set_auth(encode(m_privateKey));

	std::string buf = regReq.SerializeAsString();

	m_pConnection->addSendTask(MGC_COMMAND_REGISTER, buf.size(), buf);
}


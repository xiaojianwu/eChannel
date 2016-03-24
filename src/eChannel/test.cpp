#include "test.h"

#include <QDir>
#include <QPluginLoader>

#include <QFileDialog>

#include <QMessageBox>

#include <QDateTime>

#include <QDebug>

#include <QUuid>

#include <qt_windows.h>

Test::Test(QWidget *parent)
	: QMainWindow(parent),
	m_p2pinterface(NULL),
	m_rdpinterface(NULL)
{
	ui.setupUi(this);

	connect(ui.pushButtonLogin, SIGNAL(clicked()), this, SLOT(login()));
	connect(ui.pushButton_Send, SIGNAL(clicked()), this, SLOT(send()));
	connect(ui.pushButton_Accept, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui.pushButton_Reject, SIGNAL(clicked()), this, SLOT(reject()));

	connect(ui.pushButton_Cancel, SIGNAL(clicked()), this, SLOT(cancel()));

    connect(ui.pushButton_filesend, SIGNAL(clicked()), this, SLOT(sendFile()));
    connect(ui.pushButton_filerecv, SIGNAL(clicked()), this, SLOT(recvFile()));
    connect(ui.pushButton_fileselect, SIGNAL(clicked()), this, SLOT(selectSendFile()));
    connect(ui.pushButton_filerecv_select, SIGNAL(clicked()), this, SLOT(selectRecvFile()));
    

	m_isTurn = false;

    m_isInvite = false;
    m_isRing = false;
    m_isAccept = false;
    m_isFile = false;

    m_fileRecvHandle = NULL;
    m_fileSend = NULL;

	ui.label_12->clear();
	ui.pushButton_Accept->setEnabled(false);
	ui.pushButton_Reject->setEnabled(false);

	ui.pushButton_Send->setEnabled(true);


	qRegisterMetaType<HandleType>("HandleType");
	qRegisterMetaType<RequestType>("RequestType");
	qRegisterMetaType<LoginResult>("LoginResult");

	connect(CommandClient::instance(), SIGNAL(onLoginResult(LoginResult)), this, SLOT(onLoginResult(LoginResult)));
	connect(CommandClient::instance(), SIGNAL(onInvite(QString, RequestType, QString, QString, QString)), this, SLOT(onInvite(QString, RequestType, QString, QString, QString)));

	connect(CommandClient::instance(), SIGNAL(onRing(QString, QString, QString)), this, SLOT(onRing(QString, QString, QString)));
	connect(CommandClient::instance(), SIGNAL(onAccept(QString, QString)), this, SLOT(onAccept(QString, QString)));
	connect(CommandClient::instance(), SIGNAL(onReject(QString, QString)), this, SLOT(onReject(QString, QString)));

    CommandClient::instance()->init();

	if (loadPlugin())
	{
		m_p2pinterface->init("192.168.25.102", "d:\\ice.log");
        ui.textEdit->append("load plugin success");

        connect(m_rdpinterface, SIGNAL(error(QString, int, QString)), this, SLOT(onRDPError(QString, int, QString)));
	}
    else
    {
        ui.textEdit->append("load plugin failed");
    }
}

Test::~Test()
{

}



void Test::selectSendFile()
{
    QString fileName = QFileDialog::getOpenFileName(this);

    ui.lineEdit_filesend->setText(fileName);
}

void Test::sendFile()
{
    m_isFile = true;

    m_touser = ui.lineEdit_peerNo->text();
    invite();
}


void Test::selectRecvFile()
{
    QString uuid = QDateTime::currentDateTime().toString("yyyy.MM.dd-hh.mm.ss");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
        QString("/home/%1").arg(uuid),
        tr("Files (*.*)"));

    ui.lineEdit_filerecv->setText(fileName);
}



void Test::recvFile()
{
    QString filePath = ui.lineEdit_filerecv->text();

    if (m_fileRecvHandle == NULL)
    {
        m_fileRecvHandle = new FileHandle;

        m_fileRecvThread.setObjectName("fileRecvThread");

        m_fileRecvHandle->moveToThread(&m_fileRecvThread);

        m_fileRecvThread.start();
        m_fileRecvHandle->init(filePath);

    }
    else
    {
        m_fileRecvHandle->setFilePath(filePath);
    }

    accept();
}

void Test::login()
{
	m_currentUser = ui.lineEdit_loginuser->text();


	m_p2pinterface->updateCurrentUser(m_currentUser);

	QString viewer = QString("%1\\%2").arg(qApp->applicationDirPath()).arg("FocusTeachVNCViewer.exe");	


    int vncPort = ui.lineEdit_VNCPort->text().toInt();
	m_rdpinterface->init(m_p2pinterface, viewer, vncPort);


	CommandClient::instance()->login(ui.lineEditIP->text(), ui.lineEditPort->text().toUShort(), m_currentUser);

}

void Test::send()
{
	m_touser = ui.lineEdit_peeruser->text();
    invite();
}

void Test::onRead(const QByteArray& data)
{
    m_fileRecvHandle->write(data);
}

void Test::invite()
{
    bool isControl = true;
    initChannel(isControl);

    if (m_channel->isInitialized())
    {
        std::string sdp = m_channel->getLocalSDP().toStdString();
        CommandClient::instance()->invite(m_uuid, m_touser, REQUEST_TYPE_FILE, sdp, "");
    }
    else
    {
        m_isInvite = true;
    }
}


void Test::onChannelInit()
{

    QString msg = QString("%1 ChannelInit").arg(QDateTime::currentDateTime().toString("hh:mm:ss-zzz"));
    ui.textEdit->append(msg);

    QString localsdp = m_channel->getLocalSDP();
    std::string sdp = localsdp.toStdString();

    ui.textEdit->append("\n=================LOCAL SDP BEGIN=================");
    ui.textEdit->append(localsdp);
    ui.textEdit->append("=================LOCAL SDP   END=================\n");

    if (m_isInvite)
    {
        CommandClient::instance()->invite(m_uuid, m_touser, REQUEST_TYPE_FILE, sdp, "");
        m_isInvite = false;
    }
    if (m_isRing)
    {
        CommandClient::instance()->ring(m_uuid, sdp);
        m_isRing = false;
    }

    if (m_isAccept)
    {
        CommandClient::instance()->accept(m_uuid, sdp);
        m_isAccept = false;
    }

    if (!m_remoteSDP.isEmpty())
    {
        m_channel->updateRemoteSDP(m_remoteSDP);
    }
    
}

void Test::accept()
{

    if (m_channel->isInitialized())
    {
        std::string sdp = m_channel->getLocalSDP().toStdString();
        CommandClient::instance()->accept(m_uuid, sdp);
    }
    else
    {
        m_isAccept = true;
    }

    QString msg = QString("i am accepted. uuid=%1").arg(m_uuid);
    ui.textEdit->append(msg);

    if (!m_isFile)
    {
        m_rdpinterface->recvRDPControl(m_uuid);

        ui.pushButton_Accept->setEnabled(false);
        ui.pushButton_Reject->setEnabled(false);
    }
}

void Test::reject()
{
	CommandClient::instance()->reject(m_uuid);

	m_rdpinterface->terminate(m_uuid);

	QString msg = QString("i am rejeceted. uuid=%1").arg(m_uuid);
	ui.textEdit->append(msg);


}

void Test::cancel()
{
	// 取消 已经在传输的文件
	m_rdpinterface->terminate(m_uuid);
	//m_p2pinterface->terminate(m_uuid);

	QString msg = QString("i am terminate. uuid=%1").arg(m_uuid);
	ui.textEdit->append(msg);
}

// 对方中断
void Test::onTerminate(QString uuid)
{
	QString msg = QString("peer terminate. uuid=%1").arg(m_uuid);
	ui.textEdit->append(msg);

	m_rdpinterface->terminate(uuid);
}


// 收到邀请
void Test::onInvite(QString fromuser, RequestType type, QString mediainfo, QString uuid, QString remotesdp)
{

    if (ui.tabWidget->currentIndex() == 1)
    {
        m_isFile = true;
    }

	ui.pushButton_Accept->setEnabled(true);
	ui.pushButton_Reject->setEnabled(true);

    ui.pushButton_filerecv->setEnabled(true);

	m_uuid = uuid;
    m_remoteSDP = remotesdp;

    QString msg = QString("receive invite. type=%1,id=%2").arg(type).arg(m_uuid);
    ui.textEdit->append(msg);

    ui.textEdit->append("\n=================REMOTE SDP BEGIN=================");
    ui.textEdit->append(m_remoteSDP);
    ui.textEdit->append("=================REMOTE SDP   END=================\n");

    bool isControl = false;
    initChannel(isControl);


    if (m_channel->isInitialized())
    {
        std::string sdp = m_channel->getLocalSDP().toStdString();

        m_channel->updateRemoteSDP(m_remoteSDP);

        // 发送响铃
        CommandClient::instance()->ring(m_uuid, sdp);
    }
    else
    {
        m_isRing = true;
    }
}

void Test::initChannel(bool isControl)
{
    QString msg = QString("%1 initSession. isControl;%2").arg(QDateTime::currentDateTime().toString("hh:mm:ss-zzz")).arg(isControl);
    ui.textEdit->append(msg);

    m_uuid = m_p2pinterface->initSession(isControl, m_uuid);
    m_channel = m_p2pinterface->getChannel(m_uuid);

    m_rdpinterface->initChannel(m_uuid, "");

    connect(m_channel, SIGNAL(initialized()), this, SLOT(onChannelInit()));
    connect(m_channel, SIGNAL(connected()), this, SLOT(onConnected()));

    if (m_isFile)
    {
        connect(m_channel, SIGNAL(idle()), this, SLOT(onIdle()));
        //connect(m_channel, SIGNAL(readyRead(const QByteArray&)), this, SLOT(onRead(const QByteArray&)));
    }
    

    connect(m_channel, SIGNAL(error(int, QString)), this, SLOT(onChannelError(int, QString)));
    
}


// 登录结果
void Test::onLoginResult(LoginResult loginResult)
{
	ui.pushButtonLogin->setDisabled(true);
	ui.label_12->setText(tr("success"));
}

void Test::onError(QString uuid, QString msg)
{
	QString message = QString("Error: %1 %2").arg(uuid).arg(msg);
	ui.label_12->setText(message);
}


// 对方响铃，等待中
void Test::onRing(QString uuid, QString from, QString sdp)
{
	QString msg = QString("Peer is Ringing. uuid=%1").arg(uuid);
	ui.textEdit->append(msg);

    m_remoteSDP = sdp;
    ui.textEdit->append("\n=================REMOTE SDP BEGIN=================");
    ui.textEdit->append(m_remoteSDP);
    ui.textEdit->append("=================REMOTE SDP   END=================\n");

    m_channel->updateRemoteSDP(m_remoteSDP);
}

// 对方接受
void Test::onAccept(QString uuid, QString from)
{
	QString msg = QString("Peer is accepted. uuid=%1").arg(uuid);
	ui.textEdit->append(msg);


    if (m_isFile)
    {      
         QString filePath = ui.lineEdit_filesend->text();

        if (m_fileSend)
        {
            m_fileSend->setFileName(filePath);
        }
        else
        {
            m_fileSend = new QFile(filePath);
        }

        m_fileSend->open(QFile::ReadOnly);


        m_blocksize = ui.lineEdit_blocksize->text().toInt();

        m_buf = m_fileSend->read(1048576);// 1M 缓存大小
        m_pos = 0;

        m_startTime = QDateTime::currentDateTime().toString("hh:mm:ss-zzz");

        onIdle();
    }
    else
    {
        m_rdpinterface->sendRDPControl(uuid);
    }
	
}

// 对方拒绝接受
void Test::onReject(QString uuid, QString from)
{
	QString msg = QString("Peer is rejeceted. uuid=%1").arg(uuid);
	ui.textEdit->append(msg);

	// 拒绝未开始传输的
	m_rdpinterface->terminate(uuid);
}


void Test::onConnected()
{
    QString msg = QString("%1 onConnected. isturn=%2, address=%3").arg(QDateTime::currentDateTime().toString("hh:mm:ss-zzz")).arg(m_channel->isRelay()).arg(m_channel->getPeerAddress());
    ui.textEdit->append(msg);
}


void Test::onIdle()
{
    if (m_channel == NULL)
    {
        return;
    }
    if (!m_buf.isEmpty())
    {
        QByteArray data = m_buf.mid(m_pos, m_blocksize); // 块大小
        m_channel->write(data);

        m_pos = m_pos + data.size();
        //m_buf.remove(0, 1048576);

        //qint64 pos = m_fileSend->pos();
        //QString msg = QString("begin: %1.\n. send pos:%2").arg(m_startTime).arg(pos);
        //ui.textEdit->setPlainText(msg);

        if (m_pos == m_buf.size())
        {
            //m_buf = m_fileSend->read(1048576); // 1M 缓存大小
            m_pos = 0;
        }


    }
    else
    {
        if (m_fileSend && m_fileSend->atEnd())
        {
            QString msg = QString("%1: end.").arg(QDateTime::currentDateTime().toString("hh:mm:ss-zzz"));
            ui.textEdit->append(msg);
        }
    }
}

void Test::onChannelError(int code, QString errMsg)
{
    QString msg = QString("%1 onChannelError. code=%2, msg=%3").arg(QDateTime::currentDateTime().toString("hh:mm:ss-zzz")).arg(code).arg(errMsg);
    ui.textEdit->append(msg);

    m_channel->close();
    m_channel = NULL;
    m_uuid = "";
}

void Test::onRDPError(QString uuid, int code, QString errMsg)
{
    QString msg = QString("%1 onRDPError. code=%2, msg=%3").arg(QDateTime::currentDateTime().toString("hh:mm:ss-zzz")).arg(code).arg(errMsg);
    ui.textEdit->append(msg);

    m_channel->close();

    m_channel = NULL;
    m_uuid = "";

    m_rdpinterface->terminate(uuid);
}

bool Test::loadPlugin()
{
	QDir pluginsDir(qApp->applicationDirPath());
	foreach (QString fileName, pluginsDir.entryList(QStringList() << "*.dll")) {
		QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
		QObject *plugin = pluginLoader.instance();
		if (plugin) 
		{
			if (m_p2pinterface == NULL)
			{
				m_p2pinterface = qobject_cast<TransferInterface *>(plugin);
			}
			if (m_rdpinterface == NULL)
			{
				m_rdpinterface = qobject_cast<RDPInterface *>(plugin);
			}

			if (m_p2pinterface && m_rdpinterface)
				return true;
		}
	}

	return false;
}
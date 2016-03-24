#ifndef Test_H
#define Test_H

#include <QtWidgets/QMainWindow>
#include "ui_test.h"

#include <libTrans.h>

#include <libRDP.h>

#include "commandclient.h"

//#include <QTimer>

#include <QByteArray>

#include "filehandle.h"

class Test : public QMainWindow
{
    Q_OBJECT

public:
    Test(QWidget *parent = 0);
    ~Test();


    bool loadPlugin();

    public slots:
        void login();

        // 远程桌面
        void send();

        void accept();

        void reject();

        void cancel();


        // 文件处理
        void sendFile();
        void recvFile();

        void selectSendFile();
        void selectRecvFile();

        void onRead(const QByteArray& data);

        // 邀请处理
        void onInvite(QString fromuser, RequestType type, QString mediainfo, QString uuid, QString sdp);

        // 对方响铃，等待中
        void onRing(QString uuid, QString from, QString sdp);

        // 对方接受
        void onAccept(QString uuid, QString from);

        // 对方拒绝接受
        void onReject(QString uuid, QString from);

        // 登录结果
        void onLoginResult(LoginResult loginResult);

        void onError(QString uuid, QString msg);

        void onTerminate(QString uuid);

        void onChannelInit();

        void onConnected();

        void onIdle();

        void onChannelError(int code, QString msg);

        void onRDPError(QString uuid, int code, QString msg);

private:
    void invite();
    void initChannel(bool isControl);

private:
    Ui::testRDPClass ui;

    TransferInterface* m_p2pinterface;
    RDPInterface* m_rdpinterface;

    Channel*        m_channel;

    //CommandClient  m_commandClient;
    QString			m_currentUser;

    QString m_touser;

    QString m_uuid;

    bool m_isTurn;

    bool    m_isInvite;
    bool    m_isRing;
    bool    m_isAccept;


    QString m_remoteSDP;


    bool m_isFile;


    // 文件相关
    FileHandle*         m_fileRecvHandle;
    QThread             m_fileRecvThread;


    QFile*              m_fileSend;
    QByteArray          m_buf; // 文件发送缓存
    int                 m_blocksize;
    int                 m_pos;

    QString             m_startTime;
};

#endif // Test_H

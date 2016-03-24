#ifndef SESSION_H
#define SESSION_H


#include <QHash>

#include <QObject>

#include "transportchannel.h"

#include <QMutex>


#include <QThread>

#include "libTrans.h"

#include "PJICESocket.h"

class Session : public QObject
{
    Q_OBJECT
public:
    Session(QObject *parent = NULL);
    ~Session();


public:
    void init(QString sessionId, TransferInterface* inteface, bool isControl);

    TransportChannel* getChannel();

    bool isControl() {return m_isControl;}


private slots:
    void onClose();

signals:
    void sigClose();

private:
    TransportChannel*					m_channel; // 主通道用来发送
    QString								m_sessionId;

    QString                             m_localSDP;
    QString                             m_remoteSDP;

    TransferInterface					*m_transInterface;
    bool                                m_isControl;

    QThread                             m_threadChannel;
};

#endif // SESSION_H

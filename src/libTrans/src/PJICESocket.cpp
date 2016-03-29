#include "PJICESocket.h"
#include "imppjice.h"
#include <QString>
#include "pjnat_inc.h"
#include "imppjice.h"

namespace FocusIce { 

PJICESocket::PJICESocket(QObject *parent,const QString &name )
	: PJICESocketAbstract(parent),/*QObject(parent),*/_name(name),_iceInfo(nullptr),_sInit(false ),_sNego(false),_soRecvSize(0),_soSndSize(0)
{
	_iceInfo = new ICE_Session() ;		
}

PJICESocket::~PJICESocket()
{
	iceAppStopSession(_iceInfo) ;
	check_deletep(_iceInfo) ;
	_sInit = _sNego = false ;
}

void PJICESocket::startICESocket(const QString &turnsrv)
{
	if(!createSession(this,_name.toStdString().c_str(),turnsrv.toStdString().c_str(),_soSndSize,_soRecvSize )){
		cbInitResult(false);
	}
}

QString PJICESocket::localSdp(bool isControl)
{	
	QString sdp ;
	do 
	{
		if (!_sInit){break ;}
		char role=isControl?'o':'n';
		iceAppInitSession(role,_iceInfo );
		char szsdp[4096]={0};
		int sdplen =encodeSession(szsdp,sizeof(szsdp),_iceInfo );
		if(sdplen>0){
			sdp.resize(sdplen);
			sdp=szsdp ;
		}
	} while (false);	
	return sdp ;
}
void PJICESocket::negoWith(const QString &remoteSdp )
{
	if(_sInit){
		if(icedemo_input_remote(_iceInfo,remoteSdp.toStdString()) ){
			iceAppStartNego(_iceInfo ) ;
		}else{
			emit cbNegoResult(false,false,"") ;
		}
	}else{
		emit cbNegoResult(false,false,"");
	}
}

void	PJICESocket::setIceLogfile (const QString &logFile) 
{	
	setLogFile(logFile.toStdString().c_str()) ;
}
void PJICESocket::setIceTurnAuth(const QString &uName,const QString &passwd,const QString &realm )
{
	setTurnAuth(uName.toStdString().c_str(),passwd.toStdString().c_str(),realm.toStdString().c_str() );
}

void	PJICESocket::iceInitResult(bool success)
{
	_sInit = success ;	
	emit cbInitResult(_sInit);
}

void PJICESocket::iceNegoResult(bool success,bool isRelay,const std::string &connip )
{
	_sNego = success ;
	if(!success){
		_iceInfo->icest=NULL;
	}
	emit cbNegoResult(_sNego,isRelay,QString::fromStdString(connip)) ;
}

void PJICESocket::iceSentTo(void  *pkt,size_t pktLen,int comp_id/*=1*/)
{
	//void iceAppSendTo(ICE_Session *pSession,unsigned comp_id, const char *data,pj_size_t length )
	if(_sNego){
		iceAppSendTo(_iceInfo,comp_id,(const char*)pkt,pktLen);
	}
	else{
		print_warning_log("PJICESocket::iceSentTo failed because ice framework still nego failed") ;
	}
}
void PJICESocket::dataRead(void *pkt,size_t size,unsigned compId,const char* ipstr)
{
	if (pkt!=NULL&& size>0){
		QByteArray  rData((const char*)pkt,size);
		emit cbReadData(rData,compId,QString::fromLocal8Bit(ipstr)) ;
	}else{
		print_warning_log("data read but can't inform the app layer,pkt=%p,size=%d,compId=%d",pkt,size,compId);
	}
}

}


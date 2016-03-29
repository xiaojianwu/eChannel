#ifndef IMPPJICE_H
#define IMPPJICE_H
#include <string>
#include "pjnat_inc.h"
namespace FocusIce{
//class PJICESocket ;

#define DEFAULT_USER "test"
#define DEFALTT_PSWD	"test"
#define DEFAULT_REALM	"www.vemic.com"
#define MAX_TURNCHARLEN	16

typedef struct _tagIceCfg{
	struct options
	{			
		char			turn_username[MAX_TURNCHARLEN];
		char			turn_password[MAX_TURNCHARLEN];
		char			turn_reaml[MAX_TURNCHARLEN];		
	} opt;	

	pj_caching_pool		cp;
	pj_pool_t		*	pool;
	pj_thread_t		*	thread;
	pj_bool_t			thread_quit_flag;
	pj_timer_heap_t	*   timer_heap;
	pj_ioqueue_t	*		ioqueue;
	//pj_ice_strans_cfg		ice_cfg;	
	FILE			*		log_fhnd;
	unsigned				ref;
	_tagIceCfg(){
		pool = NULL ,thread = NULL,log_fhnd =NULL,timer_heap=NULL;
		thread_quit_flag= false ;	
		ref=0 ;
		strncpy(opt.turn_username,DEFAULT_USER,MAX_TURNCHARLEN);
		strncpy(opt.turn_password,DEFALTT_PSWD,MAX_TURNCHARLEN);
		strncpy(opt.turn_reaml,DEFAULT_REALM,MAX_TURNCHARLEN);		
	}

}IceAppCfg;



class ICE_Session
{
public:
	ICE_Session():icest(NULL){
		memset(rem.ufrag,0,sizeof(rem.ufrag));
		memset(rem.pwd,0,sizeof(rem.pwd));
		rem.comp_cnt=1;
	}
	~ICE_Session() {};
public:
	struct rem_info
	{
		char				ufrag[80];
		char				pwd[80];
		unsigned			comp_cnt;
		pj_sockaddr		def_addr[PJ_ICE_MAX_COMP];
		unsigned			cand_cnt;
		pj_ice_sess_cand cand[PJ_ICE_ST_MAX_CAND];
	} rem;
	pj_ice_strans	*		icest;
};


void		setLogFile(const char *logfile) ;
void		setTurnAuth(const char* uName,const char* passwd,const char*reaml) ;
bool		iceAppInit(const char *rfc5766 ) ;
bool		createSession(void* pUser,const char*Name,const char* rfc5766,unsigned soSndsize,unsigned soRecvSize) ;
void		iceAppStopSession(ICE_Session *pSession );
int		encodeSession(char buffer[], unsigned maxlen,ICE_Session *pSession);
void		iceAppInitSession(unsigned rolechar,ICE_Session *pUser);
bool		icedemo_input_remote(ICE_Session *pSession,const std::string &sdp) ;
void		iceAppStartNego(ICE_Session *pSession ) ;

void		iceAppSendTo(ICE_Session *pSession,unsigned comp_id, const char *data,pj_size_t length) ;


}

#endif // IMPPJICE_H

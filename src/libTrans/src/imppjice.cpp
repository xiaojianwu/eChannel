#include "imppjice.h"
#include "pjnat_inc.h"
#include "PJICESocket.h"
	
#include "errno.h"
#include <string>
//////////////////////////////////////////////////////////////////////////
//global function
//////////////////////////////////////////////////////////////////////////
namespace FocusIce{


#define		KA_INTERVAL		500

static IceAppCfg iceCfg ;


std::string pjstInfo(pj_status_t t) 
{	
	char errmsg[PJ_ERR_MSG_SIZE] = {0};
	pj_strerror(t, errmsg, PJ_ERR_MSG_SIZE);
	return std::string(errmsg);
}

#define  CHCKE_BREAK(title,status) {\
				if(status!=PJ_SUCCESS)\
				{\
					print_error_log("%s failed,error:%s",title,pjstInfo(status).c_str());\
					break ;\
				}\
			}

#define CHECK_THREAD_REGISTR(tname)	 { \
	if(!pj_thread_is_registered()) {\
		pj_thread_desc adesc ;\
		pj_thread_t *thread ;\
		pj_status_t status= pj_thread_register(tname,adesc,&thread) ;\
	}\
 }

pj_status_t handleEvents(unsigned maxMsec,unsigned *pCnt)
{
	enum{MAX_NET_EVENTS=1};
	pj_time_val  maxTimeOut ={0,0};

	pj_time_val	timeOut={0,0};
	unsigned count = 0,netEventCnt=0;
	
	maxTimeOut.msec = maxMsec;
	/* Poll the timer to run it and also to retrieve the earliest entry. */
	maxTimeOut.sec= timeOut.sec =0;
	int c = pj_timer_heap_poll(iceCfg.timer_heap,&timeOut) ;
	if (c>0) {count+=c;}
	 /* timer_heap_poll should never ever returns negative value, or otherwise
     * ioqueue_poll() will block forever!
     */
	pj_assert(timeOut.sec>=0 && timeOut.msec>=0) ;
	if(timeOut.msec>=1000) timeOut.msec=999;

	/* compare the value with the timeout to wait from timer, and use the 
     * minimum value. 
    */
	if (PJ_TIME_VAL_GT(timeOut, maxTimeOut)){
		timeOut = maxTimeOut;
	}

	/* Poll ioqueue. 
     * Repeat polling the ioqueue while we have immediate events, because
     * timer heap may process more than one events, so if we only process
     * one network events at a time (such as when IOCP backend is used),
     * the ioqueue may have trouble keeping up with the request rate.
     *
     * For example, for each send() request, one network event will be
     *   reported by ioqueue for the send() completion. If we don't poll
     *   the ioqueue often enough, the send() completion will not be
     *   reported in timely manner.
     */
	do 
	{
		c=pj_ioqueue_poll(iceCfg.ioqueue,&timeOut);
		if (c<0){
			pj_status_t errst = pj_get_netos_error() ;
			pj_thread_sleep(PJ_TIME_VAL_MSEC(timeOut)) ;
			if(pCnt){
				*pCnt = count ;
			}
			return errst ;
		}else if(c==0){
			break;
		}else{
			netEventCnt+=c;
			timeOut.sec = timeOut.msec=0;
		}
	} while (c > 0 && netEventCnt < MAX_NET_EVENTS);
	count+=netEventCnt ;
	if(pCnt!=NULL){
		*pCnt = count ;
	}
	return PJ_SUCCESS ;
}


/*
 * This is the worker thread that polls event in the background.
 */
int iceAppWorkThread(void *pUser)
{
	PJ_UNUSED_ARG(pUser) ;
	while(!iceCfg.thread_quit_flag){
		handleEvents(500,NULL) ;
	}
	return 0 ;
}


/* log callback to write to file */
void cbLogWrite(int level,const char *data,int len)
{
	pj_log_write(level,data,len);
	if(iceCfg.log_fhnd != NULL){
		if (fwrite(data, len, 1, iceCfg.log_fhnd) != 1){
			print_error_log("cbLogWrite write to file failed") ;
			return;
		}
		fflush(iceCfg.log_fhnd);
	}
}
/*
 * This is the main application load library function. It is called
 * once (and only once) during application initialization sequence by 
 * and it can't be unload because video and voice may use the library too
 */
static bool  loadLibrary() {
	pj_status_t status ;
	do 
	{		
		status = pj_init() ;	
		CHCKE_BREAK("pj_init", status);
		print_info_log("pj_init success");
		status = pjlib_util_init();		
		CHCKE_BREAK("pjlib_util_init", status);
		status = pjnath_init() ;		
		CHCKE_BREAK("pjnath_init", status );
	} while (false);
	return status == PJ_SUCCESS;
}



void setLogFile(const char * file)
{
	if(iceCfg.log_fhnd==NULL){		
		pj_assert(file!=NULL) ;
		iceCfg.log_fhnd = fopen(file,"w+");
		unsigned decor= PJ_LOG_HAS_YEAR|PJ_LOG_HAS_MONTH|PJ_LOG_HAS_TIME | PJ_LOG_HAS_MICRO_SEC | PJ_LOG_HAS_SENDER| PJ_LOG_HAS_NEWLINE | PJ_LOG_HAS_THREAD_ID
			| PJ_LOG_HAS_LEVEL_TEXT | PJ_LOG_HAS_DAY_OF_MON ; 
		pj_log_set_decor(decor);
		pj_log_set_level(4); //4 :debug; 5:trace
		pj_log_set_log_func(&cbLogWrite) ;
		loadLibrary() ;
	}
}

void  setTurnAuth(const char *uName,const char* passwd,const char* realm)
{
	assert(uName!=NULL) ;
	assert(passwd!=NULL);
	assert(realm!=NULL);
	char szName[64]={0};
	strncpy(iceCfg.opt.turn_username,uName,MAX_TURNCHARLEN);
	strncpy(iceCfg.opt.turn_password,uName,MAX_TURNCHARLEN);
	strncpy(iceCfg.opt.turn_reaml,uName,MAX_TURNCHARLEN);
}

/*
 * This is the main application initialization function. It is called
 * once (and only once) during application initialization sequence by 
 * main().
 */

void initIceStransCfg(pj_ice_strans_cfg	& cfg	,const char* rfc5766 ,unsigned so_snd_size/*=0*/,unsigned so_recv_size/*=0*/){

	pj_ice_strans_cfg_default(&cfg);
	cfg.stun_cfg.pf = &iceCfg.cp.factory ;
	cfg.stun_cfg.timer_heap = iceCfg.timer_heap;
	cfg.stun_cfg.ioqueue = iceCfg.ioqueue;
	cfg.af = pj_AF_INET();	
	
	pj_str_t		rfc5766_srv=pj_str((char*)rfc5766);
	//******************************dns does't need*******************************
	/* -= Start initializing ICE stream transport config =- */

	/* Maximum number of host candidates */
	cfg.stun.max_host_cands= 1;
	/* Nomination strategy */
	cfg.opt.aggressive = PJ_FALSE;
	/* Configure STUN/srflx candidate resolution */
	if (rfc5766_srv.slen) {
		char *pos;
		/* Command line option may contain port number */
		if ((pos=pj_strchr(&rfc5766_srv, ':')) != NULL) {
			cfg.stun.server.ptr = rfc5766_srv.ptr;
			cfg.stun.server.slen = (pos - rfc5766_srv.ptr);
			cfg.stun.port = (pj_uint16_t)atoi(pos+1);
		} else {
			cfg.stun.server = rfc5766_srv;
			cfg.stun.port = PJ_STUN_PORT;
		}

		/* For this demo app, configure longer STUN keep-alive time
			* so that it does't clutter the screen output.
			*/
		/*init for socket snd buffer size and recv buffer size*/
		cfg.stun.cfg.so_sndbuf_size = so_snd_size ;
		cfg.stun.cfg.so_rcvbuf_size = so_recv_size ;

		cfg.stun.cfg.ka_interval = KA_INTERVAL;
	}

	/* Configure TURN candidate */
	if (rfc5766_srv.slen) {
		char *pos;
		/* Command line option may contain port number */
		if ((pos=pj_strchr(&rfc5766_srv, ':')) != NULL) {	
			cfg.turn.server.ptr = rfc5766_srv.ptr;
			cfg.turn.server.slen = (pos - rfc5766_srv.ptr);
			cfg.turn.port = (pj_uint16_t)atoi(pos+1);
		} else {	
			cfg.turn.server =rfc5766_srv;
			cfg.turn.port = PJ_STUN_PORT;
		}

		/* TURN credential */
		cfg.turn.auth_cred.type = PJ_STUN_AUTH_CRED_STATIC;
		cfg.turn.auth_cred.data.static_cred.username = pj_str(iceCfg.opt.turn_username);
		cfg.turn.auth_cred.data.static_cred.data_type = PJ_STUN_PASSWD_HASHED;//PJ_STUN_PASSWD_PLAIN;
		cfg.turn.auth_cred.data.static_cred.realm=pj_str(iceCfg.opt.turn_reaml );/*pj_str("www.vemic.com");*/
		cfg.turn.auth_cred.data.static_cred.data = pj_str(iceCfg.opt.turn_password);

		/* Connection type to TURN server */
		//if (iceCfg.opt.turn_tcp)
		cfg.turn.conn_type = PJ_TURN_TP_TCP;
// 		else
// 			cfg.turn.conn_type = PJ_TURN_TP_UDP;

		/* For this demo app, configure longer keep-alive time
			* so that it does't clutter the screen output.
			*/
		/*init for socket snd buffer size and recv buffer size*/
		cfg.turn.cfg.so_sndbuf_size = so_snd_size ;
		cfg.turn.cfg.so_rcvbuf_size = so_recv_size ;

		cfg.turn.alloc_param.ka_interval = KA_INTERVAL;
	}
}


bool iceAppInit(const char *rfc5766  ) 
{
	CHECK_THREAD_REGISTR("iceAppInit");
	if (iceCfg.pool!=NULL )
		return true;
	iceCfg.thread_quit_flag = PJ_FALSE ;
	if (iceCfg.log_fhnd==NULL) {
		setLogFile("PJICE.log");
	}
	pj_status_t status ;	
	do 
	{	
		/* Must create pool factory, where memory allocations come from */
		pj_caching_pool_init(&iceCfg.cp, NULL, 0);
		
		/* Create application memory pool */
		iceCfg.pool = pj_pool_create(&iceCfg.cp.factory, "iceApp",512, 512, NULL);
		/* Create timer heap for timer stuff */
		status = pj_timer_heap_create(iceCfg.pool,100,&iceCfg.timer_heap);
		CHCKE_BREAK("pj_timer_heap_create",status) ;			

		/* and create ioqueue for network I/O stuff */
		status = pj_ioqueue_create(iceCfg.pool, 16, &iceCfg.ioqueue) ;
		CHCKE_BREAK("pj_ioqueue_create", status );
		/* something must poll the timer heap and ioqueue, 
		* unless we're on Symbian where the timer heap and ioqueue run
		* on themselves.
		*/
		status =  pj_thread_create(iceCfg.pool, "iceAppWorker", &iceAppWorkThread,	NULL, 0, 0, &iceCfg.thread) ;
		CHCKE_BREAK("pj_thread_create",status );		
	} while (false);
	return (status==PJ_SUCCESS) ;
}


void err_exit(const char*title,pj_status_t status)
{
// 	if (status!=PJ_SUCCESS){
// 		print_error_log("title:%s,error:%s",title,pjstInfo(status).c_str()) ;
// 	}
// 	print_info_log("Shutting down...");
// 	if(iceApp.icest!=nullptr){
// 		pj_ice_strans_destroy(iceApp.icest) ;
// 	}
// 	pj_thread_sleep(500) ;
// 	iceApp.thread_quit_flag = PJ_TRUE;
// 	if(iceApp.thread!=nullptr){
// 		pj_thread_join(iceApp.thread);
// 		pj_thread_destroy(iceApp.thread);
// 	}
// 	if(iceApp.ice_cfg.stun_cfg.ioqueue){
// 		pj_ioqueue_destroy(iceApp.ice_cfg.stun_cfg.ioqueue) ;
// 	}
// 	if(iceApp.ice_cfg.stun_cfg.timer_heap){
// 		pj_timer_heap_destroy(iceApp.ice_cfg.stun_cfg.timer_heap) ;
// 	}
// 	pj_caching_pool_destroy(&iceApp.cp);
// 	pj_shutdown() ;
// 
// 	if (iceApp.log_fhnd!=NULL) {		
// 		fclose(iceApp.log_fhnd);
// 		iceApp.log_fhnd = NULL;
// 	}
}

/*
 * This is the callback that is registered to the ICE stream transport to
 * receive notification about incoming data. By "data" it means application
 * data such as RTP/RTCP, and not packets that belong to ICE signaling (such
 * as STUN connectivity checks or TURN signaling).
 */
void cbOnRxData(pj_ice_strans *iceSt,unsigned compId,void *pkt,pj_size_t size,
	const pj_sockaddr_t *srcAddr,unsigned srcAddrlen){
	char ipstr[PJ_INET6_ADDRSTRLEN+10] = {0} ;
	pj_sockaddr_print(srcAddr, ipstr, sizeof(ipstr), 3);
// 	PJ_UNUSED_ARG(iceSt) ;
// 	PJ_UNUSED_ARG(srcAddrlen);
// 	PJ_UNUSED_ARG(pkt) ;
	// Don't do this! It will ruin the packet buffer in case TCP is used!
	//((char*)pkt)[size] = '\0';
// 	print_debug_log("Component %d: received %d bytes data from %s: \"%.*s\"",
// 		compId, size,
// 		pj_sockaddr_print(srcAddr, ipstr, sizeof(ipstr), 3),
// 		(unsigned)size,
// 		(char*)pkt);

	PJICESocketAbstract* appNet = (PJICESocketAbstract*)pj_ice_strans_get_user_data(iceSt)  ;
	if (appNet!=NULL ) {
		appNet->dataRead(pkt,size,compId,ipstr) ;
	}else{
		print_error_log("cbOnRxData appNet  is NULL ") ;
	}
}
/*
 * This is the callback that is registered to the ICE stream transport to
 * receive notification about ICE state progression.
 */
void cbOnIceComplete(pj_ice_strans* iceSt,pj_ice_strans_op op,pj_status_t status)
{	
	const char* opName =(op==PJ_ICE_STRANS_OP_INIT ?"initialization" :
		(op==PJ_ICE_STRANS_OP_NEGOTIATION ? "negotiation" : "unknown_op"));
	print_debug_log("cbOnIceComplete option name=%s",opName) ;
	if(status == PJ_SUCCESS){
		print_info_log("ICE %s successful",opName) ;
	}else{
		print_error_log("cbOnIceComplete %s failed:%s",opName,pjstInfo(status).c_str()) ;
		pj_ice_strans_destroy(iceSt);		
	}
	PJICESocketAbstract *pUser =  (PJICESocketAbstract *)(pj_ice_strans_get_user_data(iceSt)) ;
	if(pUser !=NULL && PJ_ICE_STRANS_OP_INIT==op ){
		pUser->iceInitResult(status==PJ_SUCCESS);
	}else if(pUser!=NULL && PJ_ICE_STRANS_OP_NEGOTIATION ==op){
		std::string conIpstr ="" ;
		unsigned compCnt = pj_ice_strans_get_running_comp_cnt (iceSt) ;	
		bool relay = false ;
		for(int i=1;i<=compCnt;++i){
			const pj_ice_sess_check *vlist=	pj_ice_strans_get_valid_pair (iceSt, i) ;
			if(vlist!=NULL){
				char ipaddr[PJ_INET6_ADDRSTRLEN];
				pj_sockaddr_print((pj_sockaddr_t*)&(vlist->rcand->addr),ipaddr, PJ_INET6_ADDRSTRLEN,3);
				conIpstr=ipaddr;
				char ipaddloc[PJ_INET6_ADDRSTRLEN];
				pj_sockaddr_print((pj_sockaddr_t*)&(vlist->lcand->addr),ipaddloc, PJ_INET6_ADDRSTRLEN,3);
				print_debug_log("cbOnIceComplete negotiation completed remote addr=%s type=%s,local addr=%s,type=%s",
					ipaddr,pj_ice_get_cand_type_name(vlist->rcand->type),ipaddloc,pj_ice_get_cand_type_name(vlist->lcand->type)) ;
				if(vlist->lcand->type==PJ_ICE_CAND_TYPE_RELAYED&& vlist->rcand->type==PJ_ICE_CAND_TYPE_RELAYED){
					relay = true ;
				}
				break;
			}		
		}
		pUser->iceNegoResult(status==PJ_SUCCESS,relay,conIpstr);
	}	
}

void createSession(void * pUser,const char*Name ,const char* rfc5766,unsigned soSndsize,unsigned soRecvSize ){		
	do 
	{
		if (pUser == NULL){
			print_error_log("createSession ,pUser is NULL ") ;
			break;
		}
		CHECK_THREAD_REGISTR("createSess") ;

		PJICESocketAbstract *pNetS = (PJICESocketAbstract*)(pUser ) ;
		ICE_Session *sInfo =  pNetS->sessionInfo() ;
		if (sInfo == NULL) {
			print_error_log("createSession ,sInfo is NULL ") ;
			break ;
		}
		if (sInfo->icest!=NULL){
			print_warning_log("already have ICE instance") ;
			break ;
		}
		if (iceCfg.pool==NULL  ){
			iceAppInit(rfc5766) ;
		}

		pj_ice_strans_cfg  scfg;
		initIceStransCfg(scfg,rfc5766,soSndsize,soRecvSize) ;

		pj_ice_strans_cb icecb ;
		/* init the callback */
		pj_bzero(&icecb, sizeof(icecb));
		icecb.on_rx_data = cbOnRxData ;
		icecb.on_ice_complete = cbOnIceComplete;


		pj_status_t status ;	
		/* create the instance */
		status = pj_ice_strans_create(Name,		/* object name  */
			&scfg,	    /* settings	    */
			scfg.stun.max_host_cands,  /* comp_cnt	    */ //Yongwenz 可以修改的地方
			pUser,			    /* ***************** user data very important********************** */
			&icecb,			    /* callback	    */
			&sInfo->icest);		/* instance ptr */		
		if (status != PJ_SUCCESS){			
			print_error_log("error creating ice %s ",pjstInfo(status).c_str());
		}else{
			iceCfg.ref++;
			print_info_log("ICE instance successfully created");
		}
	} while (false);

}
void  resetRemoteInfo(ICE_Session::rem_info &info ){
	pj_bzero(&info,sizeof(ICE_Session::rem_info));
}

/*
 * Destroy ICE stream transport instance, invoked from the menu.
 */
static void destoryIceApp(pj_ice_strans *icest )
{
	if (--iceCfg.ref==0){
		pj_thread_sleep(500) ;
		iceCfg.thread_quit_flag = PJ_TRUE;
		if(iceCfg.thread!=nullptr){
			pj_thread_join(iceCfg.thread);
			pj_thread_destroy(iceCfg.thread);
		}
		if(iceCfg.ioqueue){
			pj_ioqueue_destroy(iceCfg.ioqueue) ;
		}
		if(iceCfg.timer_heap){
			pj_timer_heap_destroy(iceCfg.timer_heap) ;
		}
		pj_pool_release(iceCfg.pool);
		iceCfg.pool=nullptr ;
		pj_caching_pool_destroy(&iceCfg.cp);
	}
}

/*
 * Create ICE session, invoked from the menu.
 */
void iceAppInitSession(unsigned rolechar,ICE_Session *pInfo)
{
	do 
	{
		pj_status_t status;
		if (pInfo == NULL) {
			print_error_log("iceAppInitSession pUser is NULL") ;
			break;
		}
		pj_ice_sess_role role = (pj_tolower((pj_uint8_t)rolechar)=='o' ? PJ_ICE_SESS_ROLE_CONTROLLING : PJ_ICE_SESS_ROLE_CONTROLLED);

		if (pInfo->icest == NULL) {
			print_error_log("Error: No ICE instance, create it first");
			return;
		}
		
		if (pj_ice_strans_has_sess(pInfo->icest)) {
			print_info_log("Session already created");
			return;
		}

		status = pj_ice_strans_init_ice(pInfo->icest, role, NULL, NULL);
		if (status != PJ_SUCCESS){
			print_error_log("error creating session ,msg=%s",pjstInfo(status).c_str());
		} else{
			print_debug_log("ICE session created") ;
		}
		resetRemoteInfo(pInfo->rem );
	} while (false);  
}

/*
 * Stop/destroy ICE session, invoked from the menu.
 */
void iceAppStopSession(ICE_Session *pSession )
{
	if (pSession==NULL){
		return ;
	}
    pj_status_t status;
    if (pSession->icest == NULL) {
		print_error_log("Error: No ICE instance, create it first") ;	
		return;
    }

    if (!pj_ice_strans_has_sess(pSession->icest)) {
		print_error_log("Error: No ICE session, initialize first");
		return;
    }

    status = pj_ice_strans_stop_ice(pSession->icest);
	if (status != PJ_SUCCESS){
		print_error_log("error stopping session,msg:%s",pjstInfo(status).c_str()) ;
	}   else{
		print_info_log( "ICE session stopped");
	}
	pj_ice_strans_destroy(pSession->icest);        
	print_info_log( "ICE instance destroyed");
	resetRemoteInfo(pSession->rem) ;
	pSession->icest = NULL ;
	destoryIceApp(pSession->icest) ;
}

#define PRINT(...)	    \
	printed = pj_ansi_snprintf(p, maxlen - (p-buffer),  \
	__VA_ARGS__); \
	if (printed <= 0 || printed >= (int)(maxlen - (p-buffer))) \
	return -PJ_ETOOSMALL; \
	p += printed


/* Utility to create a=candidate SDP attribute */
int print_cand(char buffer[], unsigned maxlen,
	const pj_ice_sess_cand *cand)
{
	char ipaddr[PJ_INET6_ADDRSTRLEN];
	char *p = buffer;
	int printed;

	PRINT("a=candidate:%.*s %u UDP %u %s %u typ ",
		(int)cand->foundation.slen,
		cand->foundation.ptr,
		(unsigned)cand->comp_id,
		cand->prio,
		pj_sockaddr_print(&cand->addr, ipaddr, 
		sizeof(ipaddr), 0),
		(unsigned)pj_sockaddr_get_port(&cand->addr));

	PRINT("%s\n",	pj_ice_get_cand_type_name(cand->type));

	if (p == buffer+maxlen)
		return -PJ_ETOOSMALL;

	*p = '\0';

	return (int)(p-buffer);
}

/* 
 * Encode ICE information in SDP.
 */
int encodeSession(char buffer[], unsigned maxlen,ICE_Session *pSession)
{
	
    char *p = buffer;
    unsigned comp;
    int printed;
    pj_str_t local_ufrag, local_pwd;
    pj_status_t status;
	if (pSession==NULL ||pSession->icest == NULL){
		print_error_log("encodeSession pSession is NULL") ;
		return 0 ;
	}
	
    /* Write "dummy" SDP v=, o=, s=, and t= lines */
    PRINT("v=0\no=- 3414953978 3414953978 IN IP4 localhost\ns=ice\nt=0 0\n");

    /* Get ufrag and pwd from current session */
    pj_ice_strans_get_ufrag_pwd(pSession->icest, &local_ufrag, &local_pwd,	NULL, NULL);

	unsigned compCnt =  pj_ice_strans_get_running_comp_cnt(pSession->icest);

    /* Write the a=ice-ufrag and a=ice-pwd attributes */
    PRINT("a=ice-ufrag:%.*s\na=ice-pwd:%.*s\n",
	   (int)local_ufrag.slen,
	   local_ufrag.ptr,
	   (int)local_pwd.slen,
	   local_pwd.ptr);

    /* Write each component */
    for (comp=0; comp<compCnt; ++comp) {
	unsigned j, cand_cnt;
	pj_ice_sess_cand cand[PJ_ICE_ST_MAX_CAND];
	char ipaddr[PJ_INET6_ADDRSTRLEN];

	/* Get default candidate for the component */
	status = pj_ice_strans_get_def_cand(pSession->icest, comp+1, &cand[0]);
	if (status != PJ_SUCCESS)
	    return -status;

	/* Write the default address */
	if (comp==0) {
	    /* For component 1, default address is in m= and c= lines */
	    PRINT("m=audio %d RTP/AVP 0\n"
		  "c=IN IP4 %s\n",
		  (int)pj_sockaddr_get_port(&cand[0].addr),
		  pj_sockaddr_print(&cand[0].addr, ipaddr,
				    sizeof(ipaddr), 0));
	} else if (comp==1) {
	    /* For component 2, default address is in a=rtcp line */
	    PRINT("a=rtcp:%d IN IP4 %s\n",
		  (int)pj_sockaddr_get_port(&cand[0].addr),
		  pj_sockaddr_print(&cand[0].addr, ipaddr,
				    sizeof(ipaddr), 0));
	} else {
	    /* For other components, we'll just invent this.. */
	    PRINT("a=Xice-defcand:%d IN IP4 %s\n",
		  (int)pj_sockaddr_get_port(&cand[0].addr),
		  pj_sockaddr_print(&cand[0].addr, ipaddr,
				    sizeof(ipaddr), 0));
	}

	/* Enumerate all candidates for this component */
	cand_cnt = PJ_ARRAY_SIZE(cand);
	status = pj_ice_strans_enum_cands(pSession->icest, comp+1,
					  &cand_cnt, cand);
	if (status != PJ_SUCCESS)
	    return -status;

	/* And encode the candidates as SDP */
	for (j=0; j<cand_cnt; ++j) {
	    printed = print_cand(p, maxlen - (unsigned)(p-buffer), &cand[j]);
	    if (printed < 0)
		return -PJ_ETOOSMALL;
	    p += printed;
	}
    }

    if (p == buffer+maxlen)
	return -PJ_ETOOSMALL;

    *p = '\0';
    return (int)(p - buffer);
}

/*
 * Input and parse SDP from the remote (containing remote's ICE information) 
 * and save it to global variables.
 */
bool icedemo_input_remote(ICE_Session *pSession,const std::string &sdp)
{	
	if (pSession==NULL)
		return false ;
	ICE_Session::rem_info &rem = pSession->rem ;
	unsigned mediaCnt = 0 ;
	unsigned comp0_port=0;
	char comp0_addr[80]={0 };
	resetRemoteInfo(pSession->rem) ;
	int len=sdp.length(), pos =0 ;
	int s_pos=0;
	while(pos<len){
		while(sdp.at(pos)!='\r'&&sdp.at(pos)!='\n') {
			pos++;
		}
		std::string line = sdp.substr(s_pos,pos-s_pos);
		switch (line.at(0)){
			case 'm':
			{
				int cnt;
				char media[32]={0},portstr[32]={0} ;
				++mediaCnt;
				if(mediaCnt>1){
					print_info_log("media line ignored");
					break;
				}
				cnt = sscanf(line.c_str()+2, "%s %s RTP/", media, portstr);

				if (cnt != 2) {
					print_error_log( "Error parsing media line");
					goto on_error;
				}
				comp0_port = atoi(portstr);
				break;
			}
			case 'c':
			{
				int cnt;
				char c[32], net[32], ip[80];
				cnt = sscanf(line.c_str()+2, "%s %s %s", c, net, ip);
				if (cnt != 3) {
					print_error_log("Error parsing connection line");
					goto on_error;
				}
				strcpy(comp0_addr, ip);
				break;
			}

			case 'a':
			{
				
				char *attr = strtok((char*)line.c_str()+2, ": \t\r\n");
				if (strcmp(attr, "ice-ufrag")==0) {
					strcpy(rem.ufrag, attr+strlen(attr)+1);
				} else if (strcmp(attr, "ice-pwd")==0) {
					strcpy(rem.pwd, attr+strlen(attr)+1);
				} else if (strcmp(attr, "rtcp")==0) {
					char *val = attr+strlen(attr)+1;
					int af, cnt;
					int port;
					char net[32], ip[64];
					pj_str_t tmp_addr;
					pj_status_t status;

					cnt = sscanf(val, "%d IN %s %s", &port, net, ip);
					if (cnt != 3) {
						print_error_log("Error parsing rtcp attribute");
						goto on_error;
					}

					if (strchr(ip, ':'))
						af = pj_AF_INET6();
					else
						af = pj_AF_INET();

					pj_sockaddr_init(af, &rem.def_addr[1], NULL, 0);
					tmp_addr = pj_str(ip);
					status = pj_sockaddr_set_str_addr(af, &rem.def_addr[1],&tmp_addr);
					if (status != PJ_SUCCESS) {
						print_error_log("Invalid IP address");
						goto on_error;
					}
					pj_sockaddr_set_port(&rem.def_addr[1], (pj_uint16_t)port);

				} else if (strcmp(attr, "candidate")==0) {
					char *sdpcand = attr+strlen(attr)+1;
					int af, cnt;
					char foundation[32], transport[12], ipaddr[80], type[32];
					pj_str_t tmpaddr;
					int comp_id, prio, port;
					pj_ice_sess_cand *cand;
					pj_status_t status;

					cnt = sscanf(sdpcand, "%s %d %s %d %s %d typ %s",
						foundation,
						&comp_id,
						transport,
						&prio,
						ipaddr,
						&port,
						type);
					if (cnt != 7) {
						print_error_log("error: Invalid ICE candidate line");
						goto on_error;
					}

					cand = &rem.cand[rem.cand_cnt];
					pj_bzero(cand, sizeof(*cand));

					if (strcmp(type, "host")==0)
						cand->type = PJ_ICE_CAND_TYPE_HOST;
					else if (strcmp(type, "srflx")==0)
						cand->type = PJ_ICE_CAND_TYPE_SRFLX;
					else if (strcmp(type, "relay")==0)
						cand->type = PJ_ICE_CAND_TYPE_RELAYED;
					else {
						print_error_log("Error: invalid candidate type '%s'", type);
						goto on_error;
					}

					cand->comp_id = (pj_uint8_t)comp_id;
					pj_strdup2(iceCfg.pool, &cand->foundation, foundation);
					cand->prio = prio;

					if (strchr(ipaddr, ':'))
						af = pj_AF_INET6();
					else
						af = pj_AF_INET();

					tmpaddr = pj_str(ipaddr);
					pj_sockaddr_init(af, &cand->addr, NULL, 0);
					status = pj_sockaddr_set_str_addr(af, &cand->addr, &tmpaddr);
					if (status != PJ_SUCCESS) {
						print_error_log("Error: invalid IP address '%s'",ipaddr);
						goto on_error;
					}

					pj_sockaddr_set_port(&cand->addr, (pj_uint16_t)port);
					++rem.cand_cnt;

					if (cand->comp_id > rem.comp_cnt){
						rem.comp_cnt = cand->comp_id;
					}						
				}
				break;
			}
			default:
			{
				print_info_log("line[0]=%c",line.at(0));
				break;
			}
			
		}
		s_pos = pos ;		
		while(pos<len && (sdp.at(pos)=='\r'|| sdp.at(pos)=='\n')){
			pos++;
			s_pos++;
		}	
	}
	
	//at last check remote

	if (rem.cand_cnt==0 ||
		rem.ufrag[0]==0 ||
		rem.pwd[0]==0 ||
		rem.comp_cnt == 0)
	{
		print_error_log("Error: not enough info");
		goto on_error;
	}

	if (comp0_port==0 || comp0_addr[0]=='\0') {
		print_error_log("Error: default address for component 0 not found");
		goto on_error;
	} else {
		int af;
		pj_str_t tmp_addr;
		pj_status_t status;

		if (strchr(comp0_addr, ':'))
			af = pj_AF_INET6();
		else
			af = pj_AF_INET();

		pj_sockaddr_init(af, &rem.def_addr[0], NULL, 0);
		tmp_addr = pj_str(comp0_addr);
		status = pj_sockaddr_set_str_addr(af, &rem.def_addr[0],&tmp_addr);
		if (status != PJ_SUCCESS) {
			print_error_log("Invalid IP address in c= line");
			goto on_error;
		}
		pj_sockaddr_set_port(&rem.def_addr[0], (pj_uint16_t)comp0_port);
	}
	print_info_log("Done, %d remote candidate(s) added", rem.cand_cnt);
	return true;

on_error:
	print_error_log("prase remote sdp error") ;
	resetRemoteInfo(rem);
	return false;		
}

void iceAppStartNego(ICE_Session *pSession )
{	
	pj_str_t rufrag, rpwd;
	pj_status_t status;
	if (pSession == NULL ||NULL == pSession->icest ) {
		print_error_log( "Error: No ICE instance, create it first,pSession=%p ,icest=%p",pSession->icest );
		return ;
	}	
	pj_ice_strans *icest = pSession->icest  ;
	ICE_Session::rem_info &rem = pSession->rem ;
	if (!pj_ice_strans_has_sess(icest)) {
		print_error_log( "Error: No ICE session, initialize first");
		return;
	}
	if (pSession->rem.cand_cnt == 0) {
		print_error_log( "Error: No remote info, input remote info first");
		return;
	}

	print_debug_log("Starting ICE negotiation..");

	status = pj_ice_strans_start_ice(icest, 
		pj_cstr(&rufrag, rem.ufrag),
		pj_cstr(&rpwd, rem.pwd),
		rem.cand_cnt,
		rem.cand);
	if (status != PJ_SUCCESS){
		print_error_log( "Error starting ICE msg:%s",pjstInfo(status).c_str());
	}else{
		print_info_log("ICE negotiation started");
	}
}


/*
 * Send application data to remote agent.
 */
void iceAppSendTo(ICE_Session *pSession,unsigned comp_id, const char *data,pj_size_t length )
{

	CHECK_THREAD_REGISTR("iceAppSendTo");

	if (pSession==NULL || pSession->icest == NULL){
		print_error_log( "ERROR iceAppSendTo session not init pSession=%p,icest=%p, ",pSession,pSession->icest );
	}
	pj_ice_strans * icest = pSession->icest ;
    pj_status_t status;  

    if (!pj_ice_strans_has_sess(icest)) {
		print_error_log( "Error: No ICE session, initialize first");
		return;
    }
    
    if (!pj_ice_strans_sess_is_complete(icest)) {
		print_error_log("Error: ICE negotiation has not been started or is in progress") ;
		return;
    }
    ICE_Session::rem_info &rem = pSession->rem ;
	unsigned runingCmp = pj_ice_strans_get_running_comp_cnt(icest ) ;
    if ( comp_id<1 ||comp_id>runingCmp ) {
		print_error_log( "Error: invalid component ID");
		return;
    }

    status = pj_ice_strans_sendto(icest, comp_id, data, length/*strlen(data)*/,
				  &rem.def_addr[comp_id-1],
				  pj_sockaddr_get_len(&rem.def_addr[comp_id-1 ]));
	if (status != PJ_SUCCESS){
		print_error_log( "Error sending data msg:%s",pjstInfo(status).c_str());
	}   else{
		//print_debug_log("Data sent");
	}	
}


//////////////////////////////////////////////////////////////////////////

}
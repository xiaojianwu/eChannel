#ifndef  __PJNAT_INC_H__
#define  __PJNAT_INC_H__
#include <pjlib.h>
#include <pjlib-util.h>
#include <pjnath.h>
#include <stdarg.h>
#include <string>
#define		check_deletep(p)		if(NULL!=p){delete p;p=NULL;}
#define		check_deleteA(p)		if (NULL!=p){	delete[] p ;	p=NULL; 	}
#define		check_free(x)			if (NULL !=x){free(x);x=NULL ;}

#define print_error_log(fmt,...)	{PJ_LOG(1,(__FILE__,fmt,__VA_ARGS__));}

#define print_warning_log(fmt,...)	{PJ_LOG(2,(__FILE__,fmt,__VA_ARGS__));}

#define print_info_log(fmt,...)	{PJ_LOG(3,(__FILE__,fmt,__VA_ARGS__));}

#define print_debug_log(fmt,...)	{PJ_LOG(4,(__FILE__,fmt,__VA_ARGS__));}

#define print_trace_log(fmt,...)	{PJ_LOG(5,(__FILE__,fmt,__VA_ARGS__));}
std::string	pjstInfo(pj_status_t t) ;

#endif//__PJNAT_INC_H__

#ifndef __BASE64URL__H__

#define  __BASE64URL__H__
#include <string>
#include <string.h>
using namespace  std;



class base64url
{
#define  MASK_6BITS  0x3f
#define  MASK_8BITS	 0xff

public:
	base64url(void);
	~base64url(void);
public:
	string encoding(const char *src,const int inLen , bool urlSafe= false);
	string decoding(const char * src, unsigned int inLen, int &outByte );

private:
	inline unsigned char replace(unsigned char src);


};
#endif //__BASE64URL__H__
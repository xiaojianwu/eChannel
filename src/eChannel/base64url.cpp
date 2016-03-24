#include "base64url.h"


static const unsigned char gStandard_table[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char gUrlsafe_table[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static const unsigned char   gDecodeTable[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	62, // '+'
	0, 0, 0,
	63, // '/'
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
	0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
	0, 0, 0, 0, 0, 0,
	26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
};



base64url::base64url(void)
{
}

base64url::~base64url(void)
{
}

//3*8bits=4*6bit=24bits=8byte
string base64url::encoding(const char *src,const int inLen,bool urlSafe/* = false */)
{
	string strEncode;
	const unsigned char *mapTable = urlSafe ?gUrlsafe_table :gStandard_table ;
	if (NULL != src)
	{
		int srcLen = inLen ;
		int lineLen = 0 ;
		int index = 0 ; 
		for (;index+3<=srcLen ; index+=3)
		{
			unsigned char a0  = src[index]; 
			//a0 = (a0<0)?a0+256:a0;
			unsigned char a1 =  src[index + 1];
			//a1 = (a1<0)?a1+256:a1;
			unsigned char a2 =  src[index + 2];
			//a2 = (a2<0)?a2+256:a2;			
			strEncode += mapTable[(a0>>2)&MASK_6BITS];
			strEncode += mapTable[((a0<<4) | (a1>>4))&MASK_6BITS];
			strEncode += mapTable[((a1<<2) | (a2>>6))&MASK_6BITS];
			strEncode += mapTable[a2&MASK_6BITS];
			lineLen += 4;
			if (lineLen%76==0)
			{
				strEncode+="\n\r"; //change to a new line
			}
		}

		//对剩余数据进行编码
		int Mod=srcLen%3;
		if(Mod==1)
		{
			unsigned char a0 = src[index];
			a0 = (a0<0)?a0+256:a0;
			strEncode+= mapTable[(a0>>2)&MASK_6BITS ];
			strEncode+= mapTable[((a0 <<4) & MASK_6BITS )];
			if(! urlSafe){
				strEncode+= "==";
			}
		}
		else if(Mod==2)
		{
			unsigned char a0 = src[index];
			a0 = (a0<0)?a0+256:a0;
			unsigned char a1 = src[index+1];
			a1 = (a1<0)?a1+256:a1;
			strEncode+= mapTable[(a0 >>2 ) & MASK_6BITS ];
			strEncode+= mapTable[((a0<<4) | (a1>>4))&MASK_6BITS];
			strEncode +=mapTable[((a1<<2) | (0 >>6))&MASK_6BITS]; //补零
			if(! urlSafe){
				strEncode+= "=";
			}
		}
	}
	return strEncode;
}

inline unsigned char base64url::replace(unsigned char src)
{
	if (src=='-')
		return '+';
	else if (src == '_')
	{
		return '/';
	}
	return src;
}

string base64url::decoding(const char * src, unsigned int inLen, int &outByte )
{
	//返回值
	string strDecode;
	int nValue;
	int i= 0;
	int DataByte = inLen;
	char bufTmp[128]={0};
	outByte = 0; //set default value
	while (i < DataByte)
	{
		if (*src != '\r' && *src!='\n')
		{
			unsigned char a = replace(*src++);			
			nValue = gDecodeTable[a] << 18;
			a=replace(*src++);
			nValue += gDecodeTable[a] << 12;
			bufTmp[outByte++]= (nValue & 0x00FF0000) >> 16;
			if (*src != '=')
			{
				a = replace(*src++);
				nValue += gDecodeTable[a] << 6;
				bufTmp[outByte++] = (nValue & 0x0000FF00) >> 8;
				if (*src != '=')
				{
					a = replace(*src++);
					nValue += gDecodeTable[a];
					bufTmp[outByte++] = nValue & 0x000000FF;
				}
			}
			i += 4;
		}
		else// 回车换行,跳过
		{
			src++;
			i++;
		}
	}
	//outByte-= 1;
	return (outByte <0) ? "":string(bufTmp,outByte);
}


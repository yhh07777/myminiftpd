#include "str.h"
#include "common.h"

void str_trim_crlf(char *str){
	char *p=&str[strlen(str)-1];
	while(*p=='\r'||*p=='\n'){
		*p--='\0';
	}
}

void str_split(const char *str,char *left,char *right,char c){
	char *p=strchr(str,c);	// 查找该字符串的位置
	if(p==NULL){
		strcpy(left,str);	
	}else{
		strncpy(left,str,p-str);
		strcpy(right,p+1);
	}
	//printf("str_split->left=%s,right=%s\n",left,right);
}

/**
*判断字符串是否都是空白字符
*
*@return 1-所有都是空白字符
*/
int str_all_space(const char *str){
	while(*str){
		if(!isspace(*str)) return 0;
		str++;
	}
	return 1;
}

/**
* 将字符串转换为大写
*/
void str_upper(char *str){
	while(*str){
		*str=toupper(*str);
		str++;
	}
}

long long str_to_longlong(const char *str){
	//return atoll(str);
	long long result=0;
	long long mult=1;
	unsigned int len=strlen(str);
	int i;
	
	// 超过15个字符直接转换成0
	if(len>15){
		return 0;	
	}
	
	/*
	for(i=0;i<len;i++){
		char ch=str[len-(i+1)];
		long long val;
		if(ch<'0'||ch>'9'){
			return 0;	
		}
		val=ch-'0';
		val *=mult;
		result+=val;
		mult*=10;	
	}
	*/
	for(i=len-1;i>=0;i--){
		char ch=str[i];
		long long val;
		if(ch<'0'||ch>'9'){
			return 0;	
		}
		val=ch-'0';
		val *=mult;
		result+=val;
		mult*=10;	
	}
	
	return result;	
}

/**
*将8进制字符串转换为无符号整形
*/
unsigned int str_octal_to_uint(const char *str){
	unsigned int result=0;
	int seen_non_zero_digit=0;	// 第一个字符不为0就将该字符置为1
	
	while(*str){
		int digit=*str;
		if(!isdigit(digit)||digit>'7'){	// 不是字符或者超过7
			break;	
		}
		
		if(digit!='0'){
			seen_non_zero_digit=1;
		}
		
		if(seen_non_zero_digit){
			result<<=3;	//左移3位就是乘以8
			result+=(digit-'0');	
		}
		str++;
	}
	
	return 0;
}

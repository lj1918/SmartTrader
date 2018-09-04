#pragma once
#include "stdafx.h"
#include <string>
#include <ATLComTime.h>
using namespace std;
using namespace boost::python;
/*
static���ε����Ϊ��̬�࣬��̬����ֻ�ܰ�����̬��Ա����static���ε��ֶΡ����ԡ���������
���ܱ�ʵ���������ܱ��̳У��Ǿ�̬�п��԰�����̬��Ա
*/
 static class Tools
{
	//��̬���в��ܴ��ڹ��캯��  
public:
	//static void formatDate(DATE date, char *buf);
	static void formatDate(DATE d, std::string &buf);
	//static COleDateTime myString2Date(string  timeStr);
	static COleDateTime String2Date(char const * timeStr);
	///-------------------------------------------------------------------------------------
	///��Python����C++����ת���õĺ���
	///-------------------------------------------------------------------------------------
	//���ֵ��л�ȡĳ����ֵ��Ӧ������������ֵ������ṹ������ֵ��
	static void getInt(boost::python::dict d, string key, int *value);
	//���ֵ��л�ȡĳ����ֵ��Ӧ�ĸ�����������ֵ������ṹ������ֵ��
	static void getDouble(boost::python::dict d, string key, double *value);
	//���ֵ��л�ȡĳ����ֵ��Ӧ���ַ�������ֵ������ṹ������ֵ��
	static  void getStr(dict d, string key, char *value);
	//���ֵ��л�ȡĳ����ֵ��Ӧ���ַ���������ֵ������ṹ������ֵ��
	static  void getChar(dict d, string key, char *value);
	//��ʱ��ӱ���ʱ��ת��ΪUTCʱ��
	static DATE ConvertLocal2UTC(IO2GSession *pSession, DATE s);
	//��UTCʱ��ת��Ϊ����ʱ��
	static DATE ConvertUTC2Local(IO2GSession *pSession, DATE s);

};

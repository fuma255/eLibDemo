// 忽略的 warning
#pragma warning(disable:4838) // 收缩转换
#pragma warning(disable:4005) // 宏重定义
#pragma warning(disable:4044) //


// 定义一下 _E_MSVC_NAME
#if _MSC_VER == 1200
#define _E_MSVC_NAME "Microsoft Visual C++ 6.0(1998)"
#elif _MSC_VER == 1310
#define _E_MSVC_NAME "Microsoft Visual C++ 7.1(2003)"
#elif _MSC_VER == 1400 
#define _E_MSVC_NAME "Microsoft Visual C++ 8.0(2005)"
#elif _MSC_VER == 1500 
#define _E_MSVC_NAME "Microsoft Visual C++ 9.0(2008)"
#elif _MSC_VER == 1600  
#define _E_MSVC_NAME "Microsoft Visual C++ 10.0(2010)"
#elif _MSC_VER == 1700   
#define _E_MSVC_NAME "Microsoft Visual C++ 11.0(2012)"
#elif _MSC_VER == 1800   
#define _E_MSVC_NAME "Microsoft Visual C++ 12.0(2013)"
#elif _MSC_VER == 1900   
#define _E_MSVC_NAME "Microsoft Visual C++ 14.0(2015)"
#elif _MSC_VER == 1911   
#define _E_MSVC_NAME "Microsoft Visual C++ 14.1(2017)"
#else
#define _E_MSVC_NAME "未知版本 MSVC"
#endif

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
#include <windows.h>

// TODO: 在此处引用程序需要的其他头文件
#include "tchar.h"
#include "wtypes.h"

#include "lib2.h"
#include "lang.h"

//库信息 

#ifndef __E_STATIC_LIB
#define LIB_GUID_STR			"{1A357FCF-855F-41c2-893E-8163685BF81F}" /*guidgen.exe生成,数字签名*/
#define LIB_MajorVersion		1  /*库主版本号*/
#define LIB_MinorVersion		0   /*库次版本号*/
#define LIB_BuildNumber			20180819 /*构建版本号*/
#define LIB_SysMajorVer			3 /*系统主版本号*/
#define LIB_SysMinorVer			0 /*系统次版本号*/
#define LIB_KrnlLibMajorVer		3 /*核心库主版本号*/
#define LIB_KrnlLibMinorVer		0 /*核心库次版本号*/
#define LIB_NAME_STR			"WinSocketForLyp" /*支持库名*/
#define LIB_DESCRIPTION_STR		"封装了客户端IOCP模型的UDP/TCP框架"/*功能描述 原本是NULL*/
#define LIB_Author				"Lyp"/*作者名称*/

#define LIB_ZipCode		NULL
#define LIB_Address		"测试地址"
#define LIB_Phone		"13145205200"
#define LIB_Fax			NULL 
#define LIB_Email		"942664114@qq.com"
#define LIB_HomePage	NULL
#define LIB_Other		NULL

#define LIB_TYPE_COUNT 2 /*命令分类数量*/
#define LIB_TYPE_STR "0000UDPClient\0""0000TCPClient\0""\0" /*命令分类*/
#endif



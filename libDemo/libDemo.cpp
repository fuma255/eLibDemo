#include "stdafx.h"
#include "libDemo.h"
#include "UDPClient.h"
#include "TCPClient.h"

//////////////////////////////////////////////////////////////////////
//////-----↓-----库常量、命令、数据类型定义区-----↓-----//////////
//////////////////////////////////////////////////////////////////////
#ifndef __E_STATIC_LIB
//请在 __E_STATIC_LIB 内部定义
BOOL mConstsIsNull = FALSE;
LIB_CONST_INFO Consts[] =
{

	/* { 中文名称, 英文名称, 常量布局, 常量等级(LVL_), 参数类型(CT_), 文本内容, 数值内容 }   只有两种数据类型*/

	{ _WT("UDP_发送数据"), _WT("UDP_SEND_DATA"), NULL, LVL_HIGH, CT_NUM, NULL, UDP_SEND_DATA },
	{ _WT("UDP_收到数据"), _WT("UDP_RECV_DATA"), NULL, LVL_HIGH, CT_NUM, NULL, UDP_RECV_DATA },
	{ _WT("UDP_其他错误"), _WT("UDP_ERROR_SOCKET"), NULL, LVL_HIGH, CT_NUM, NULL, UDP_ERROR_SOCKET },

	{ _WT("TCP_发送数据"), _WT("TCP_SEND_DATA"), NULL, LVL_HIGH, CT_NUM, NULL, TCP_SEND_DATA },
	{ _WT("TCP_数据到达"), _WT("TCP_RECV_DATA"), NULL, LVL_HIGH, CT_NUM, NULL, TCP_RECV_DATA },
	{ _WT("TCP_客户断开"), _WT("TCP_CLIENT_DISCONNECT"), NULL, LVL_HIGH, CT_NUM, NULL, TCP_CLIENT_DISCONNECT },
	{ _WT("TCP_服务断开"), _WT("TCP_SERVER_DISCONNECT"), NULL, LVL_HIGH, CT_NUM, NULL, TCP_SERVER_DISCONNECT },
	{ _WT("TCP_异常断开"), _WT("TCP_EXCEPTION_DISCONNECT"), NULL, LVL_HIGH, CT_NUM, NULL, TCP_EXCEPTION_DISCONNECT },
	{ _WT("TCP_其他错误"), _WT("TCP_ERROR"), NULL, LVL_HIGH, CT_NUM, NULL, TCP_ERROR },
};

#endif

//相关解释
/*
常量等级有: LVL_SIMPLE  1  初级        LVL_SECONDARY  2  中级             LVL_HIGH  3  高级

参数类型有 : CT_NUM             1    // sample: 3.1415926
CT_BOOL            2    // sample: 1
CT_TEXT            3    // sample: "abc"

文本内容是CT_TEXT用，数值内容是CT_NUM和CT_BOOL用。
*/


/////////////////////////////////////////////////////////////////
//////--------------------定义自定义数据类型----------------//////
////////////////////////////////////////////////////////////////

#ifndef __E_STATIC_LIB	//这中间的静态编译时跳过
//请在 __E_STATIC_LIB 内部定义   
#endif


#ifndef __E_STATIC_LIB
//请在 __E_STATIC_LIB 内部定义
INT DatatypeCommandIndexs[] =
{
	NULL
};
BOOL mDataTypesIsNull = TRUE;
static LIB_DATA_TYPE_INFO DataTypes[] =
{
	NULL
	/* { 中文名称, 英文名称, 数据描述, 索引数量, 命令索引, 对象状态, 图标索引, 事件数量, 事件指针, 属性数量, 属性指针, 界面指针, 元素数量, 元素指针 } */
};

#endif

/////////////////////////////////////////////////////////////////////////////
///////------------------支持库命令的实现内容---------------------////////
/////////////////////////////////////////////////////////////////////////////

/*
函数的实现都需要定义在宏的外面以便静态和动态库都能使用，但ExecuteCommand，Commands则只需定义在宏的里面供动态库使用。
pRetData 输出数据指针。当对应CMD_INFO中m_dtRetType为_SDT_NULL（即定义无返回值）时，pRetData无效；
iArgCount 函数参数个数
pArgInf 函数参数指针
*/

EXTERN_C void sum(PMDATA_INF pRetData, INT iArgCount, PMDATA_INF pArgInf)
{
	pRetData->m_int = pArgInf[0].m_int + pArgInf[1].m_int;
	return;
};

EXTERN_C void InitEnv(PMDATA_INF pRetData, INT iArgCount, PMDATA_INF pArgInf)
{
	pRetData->m_bool = InitWinsock();
	return;
};

EXTERN_C void EndEnv(PMDATA_INF pRetData, INT iArgCount, PMDATA_INF pArgInf)
{
	destory();
	return;
};

EXTERN_C void Start(PMDATA_INF pRetData, INT iArgCount, PMDATA_INF pArgInf)
{
	int port = pArgInf[0].m_int;
	int callback = pArgInf[1].m_int;
	int extra = pArgInf[2].m_int;
	OverlappedP* ptr = start(port, callback, extra);
	pRetData->m_int = (int)ptr;
	return;
};

EXTERN_C void SendTo(PMDATA_INF pRetData, INT iArgCount, PMDATA_INF pArgInf)
{
	OverlappedP* ptr = (OverlappedP*)pArgInf[0].m_int;
	char *ip = pArgInf[1].m_pText;
	int port = pArgInf[2].m_int;
	char* buf = (char*)pArgInf[3].m_int;
	int len = pArgInf[4].m_int;
	pRetData->m_bool = sendpack(ptr, ip, port, buf, len);
	return;
};

EXTERN_C void Stop(PMDATA_INF pRetData, INT iArgCount, PMDATA_INF pArgInf)
{
	OverlappedP* ptr = (OverlappedP*)pArgInf[0].m_int;
	pRetData->m_bool = stop(ptr);
	return;
};

EXTERN_C void InitTCPEnv(PMDATA_INF pRetData, INT iArgCount, PMDATA_INF pArgInf)
{
	pRetData->m_bool = TCPInitWinsock();
	return;
};

EXTERN_C void EndTCPEnv(PMDATA_INF pRetData, INT iArgCount, PMDATA_INF pArgInf)
{
	Destory();
	return;
};

//TCPOverlappedP* Connect(char* serverIp, u_short serverPort, int callback, int Extra);
EXTERN_C void ConnectServer(PMDATA_INF pRetData, INT iArgCount, PMDATA_INF pArgInf)
{
	char *ip = pArgInf[0].m_pText;
	int port = pArgInf[1].m_int;
	int callback = pArgInf[2].m_int;
	int extra = pArgInf[3].m_int;
	
	TCPOverlappedP* ptr = Connect(ip, port, callback, extra);
	pRetData->m_int = (int)ptr;
	return;
};

EXTERN_C void Send(PMDATA_INF pRetData, INT iArgCount, PMDATA_INF pArgInf)
{
	TCPOverlappedP* ptr = (TCPOverlappedP*)pArgInf[0].m_int;
	char* buf = (char*)pArgInf[1].m_int;
	int len = pArgInf[2].m_int;
	pRetData->m_int = Send(ptr, buf, len);
	return;
};

//int Close(TCPOverlappedP* recv);
EXTERN_C void Close(PMDATA_INF pRetData, INT iArgCount, PMDATA_INF pArgInf)
{
	TCPOverlappedP* ptr = (TCPOverlappedP*)pArgInf[0].m_int;
	pRetData->m_int = Close(ptr);
	return;
};

EXTERN_C void GetClientPort(PMDATA_INF pRetData, INT iArgCount, PMDATA_INF pArgInf)
{
	TCPOverlappedP* ptr = (TCPOverlappedP*)pArgInf[0].m_int;
	pRetData->m_int = (int)GetClientPort(ptr);
	return;
};


//==========================================
//==========================================
//==========================================
#ifndef __E_STATIC_LIB
PFN_EXECUTE_CMD ExecuteCommand[] =
{
	// 所有需要库中调用的  函数  都列在这里，用逗号隔开
	InitEnv,
	EndEnv,
	Start,
	SendTo,
	Stop,
	InitTCPEnv,
	EndTCPEnv,
	ConnectServer,
	Send,
	Close,
	GetClientPort
};

static const char* const CommandNames[] =
{
	// 所有需要库中调用的  函数名  都写在这里，用逗号隔开
	"InitEnv",
	"EndEnv",
	"Start",
	"SendTo",
	"Stop", 
	"InitTCPEnv",
	"EndTCPEnv",
	"ConnectServer",
	"Send",
	"Close",
	"GetClientPort"
};

#endif

/////////////////////////////////////////////////////////////////////////////
///////----------------定义支持库命令参数表-----------------------////////
/////////////////////////////////////////////////////////////////////////////
#ifndef __E_STATIC_LIB

ARG_INFO sum_CommandArgs[] =
{
	{ _WT("arg1"), _WT(""), 0, 0, SDT_INT, NULL, NULL },
	{ _WT("arg2"), _WT(""), 0, 0, SDT_INT, NULL, NULL },
};
//启动方法参数参数 OverlappedP* start(int clientport, int callback, int Extra)
//(over->OperationType, (int)over->wsabuf.buf, Transferred, inet_ntoa(over->addr.sin_addr), ntohs(over->addr.sin_port), over->Extra)
/* { 参数名称, 参数描述, 图像索引, 图像数量, 参数类型(参见SDT_), 默认数值, 参数类别(参见AS_) } */
ARG_INFO Start_CommandArgs[] =
{
	{ _WT("绑定端口"), _WT("客户端绑定的本地端口,默认为0并且由系统自动分配一个可用端口"), 0, 0, SDT_INT, NULL, NULL },
	{ _WT("回调地址"), _WT("事件回调函数(无返回值)\r\n\t参数1:事件类型 \r\n\t参数2:数据地址\r\n\t参数3:数据长度\r\n\t参数4:发送者IP\r\n\t参数5:发送者端口\r\n\t参数6:附加数据"), 0, 0, SDT_INT, NULL, NULL },
	{ _WT("附加数据"), _WT("本客户端绑定的数据,可在回调函数中获取进行关联操作."), 0, 0, SDT_INT, NULL, NULL },
};
//发送数据参数   bool sendpack(OverlappedP* recv, char* ip, short port, char* buffer, int buflen)
ARG_INFO Send_CommandArgs[] =
{
	{ _WT("绑定句柄"), _WT("此句柄由启动()函数返回"), 0, 0, SDT_INT, NULL, NULL },
	{ _WT("对方地址"), _WT("对方IP"), 0, 0, SDT_TEXT, NULL, NULL },
	{ _WT("对方端口"), _WT("对方端口"), 0, 0, SDT_INT, NULL, NULL },
	{ _WT("数据指针"), _WT("字节集变量数据地址"), 0, 0, SDT_INT, NULL, NULL },
	{ _WT("数据长度"), _WT("数据长度"), 0, 0, SDT_INT, NULL, NULL },
};

//停止参数   bool stop(OverlappedP *Recv);
ARG_INFO Stop_CommandArgs[] =
{
	{ _WT("绑定句柄"), _WT("此句柄由启动()函数返回"), 0, 0, SDT_INT, NULL, NULL }
};

//Connect参数
ARG_INFO Connect_CommandArgs[] =
{
	{ _WT("服务器IP"), _WT("服务器的IP,点分十进制,例如192.168.0.1,127.0.0.1"), 0, 0, SDT_TEXT, NULL, NULL },
	{ _WT("服务器端口"), _WT("服务器的端口"), 0, 0, SDT_INT, NULL, NULL },
	////事件类型  数据地址 数据长度 服务地址 服务端口 附加数据 错误代码
	{ _WT("回调地址"), _WT("事件回调函数(无返回值),参数都是整数型\r\n\t参数1: 事件类型 整数型\r\n\t参数2: 数据地址\r\n\t参数3: 数据长度\r\n\t参数4:服务器IP\r\n\t参数5:服务器端口\r\n\t参数6:附加数据\r\n\t参数7:错误代码"), 0, 0, SDT_INT, NULL, NULL },
	{ _WT("附加数据"), _WT("本客户端绑定的数据,可在回调函数中获取进行关联操作."), 0, 0, SDT_INT, NULL, NULL },
};

//int Send(TCPOverlappedP* recv, char* buf, int len);
ARG_INFO TCPSend_CommandArgs[] =
{
	{ _WT("绑定句柄"), _WT("此句柄由连接()函数返回"), 0, 0, SDT_INT, NULL, NULL },
	{ _WT("数据指针"), _WT("字节集变量数据地址"), 0, 0, SDT_INT, NULL, NULL },
	{ _WT("数据长度"), _WT("数据长度"), 0, 0, SDT_INT, NULL, NULL },
};

//int Close(TCPOverlappedP* recv);
ARG_INFO Close_CommandArgs[] =
{
	{ _WT("绑定句柄"), _WT("此句柄由连接()函数返回"), 0, 0, SDT_INT, NULL, NULL }
};

//u_short GetClientPort(TCPOverlappedP* ptr);
ARG_INFO GetClientPort_CommandArgs[] =
{
	{ _WT("绑定句柄"), _WT("此句柄由连接()函数返回"), 0, 0, SDT_INT, NULL, NULL }
};
#endif

/////////////////////////////////////////////////////////////////////////////
///////---------------声明要导出的支持库命令----------------------////////
/////////////////////////////////////////////////////////////////////////////
#ifndef __E_STATIC_LIB

static CMD_INFO Commands[] =
{
	/* { 中文名称, 英文名称, 对象描述, 所属类别(-1是数据类型的方法), 命令状态(CT_), 返回类型(SDT_), 此值保留, 对象等级(LVL_), 图像索引, 图像数量, 参数个数, 参数信息 } */
	{
		/*中文名称*/						_WT("InitEnv"),
		/*英文名称*/						_WT("InitEnv"),
		/*对象描述*/						_WT("初始化Socket环境,只调用一次,并且在程序一运行(比如在[_启动窗口_创建完毕]子程序下调用)就要调用,程序结束时调用,EndEnv()"),
		/*所属类别(-1是数据类型的方法)*/	1,
		/*命令状态(CT_)*/				NULL,
		/*返回类型(SDT_)*/				_SDT_NULL,
		/*此值保留*/						0,
		/*对象等级(LVL_)*/				LVL_SIMPLE,
		/*图像索引*/						0,
		/*图像数量*/						0,
		/*参数个数*/						0,
		/*参数信息*/						NULL
	},
	{
		/*中文名称*/						_WT("EndEnv"),
		/*英文名称*/						_WT("EndEnv"),
		/*对象描述*/						_WT("和InitEnv对应"),
		/*所属类别(-1是数据类型的方法)*/	1,
		/*命令状态(CT_)*/				NULL,
		/*返回类型(SDT_)*/				_SDT_NULL,
		/*此值保留*/						0,
		/*对象等级(LVL_)*/				LVL_SIMPLE,
		/*图像索引*/						0,
		/*图像数量*/						0,
		/*参数个数*/						0,
		/*参数信息*/						NULL
	},
	{
		/*中文名称*/						_WT("Start"),
		/*英文名称*/						_WT("Start"),
		/*对象描述*/						_WT("启动Socket,返回句柄,此句柄可用于发送数据,此句柄并非SOCEKT句柄,而是一个重叠结构指针."),
		/*所属类别(-1是数据类型的方法)*/	1,
		/*命令状态(CT_)*/				NULL,
		/*返回类型(SDT_)*/				SDT_INT,
		/*此值保留*/						0,
		/*对象等级(LVL_)*/				LVL_SIMPLE,
		/*图像索引*/						0,
		/*图像数量*/						0,
		/*参数个数*/						sizeof(Start_CommandArgs) / sizeof(Start_CommandArgs[0]),
		/*参数信息*/						Start_CommandArgs
	},
	{
		/*中文名称*/						_WT("SendTo"),
		/*英文名称*/						_WT("SendTo"),
		/*对象描述*/						_WT("发送数据"),
		/*所属类别(-1是数据类型的方法)*/	1,
		/*命令状态(CT_)*/				NULL,
		/*返回类型(SDT_)*/				SDT_BOOL,
		/*此值保留*/						0,
		/*对象等级(LVL_)*/				LVL_SIMPLE,
		/*图像索引*/						0,
		/*图像数量*/						0,
		/*参数个数*/						sizeof(Send_CommandArgs) / sizeof(Send_CommandArgs[0]),
		/*参数信息*/						Send_CommandArgs
	},
	{
		/*中文名称*/						_WT("Stop"),
		/*英文名称*/						_WT("Stop"),
		/*对象描述*/						_WT("关闭这个Socket"),
		/*所属类别(-1是数据类型的方法)*/	1,
		/*命令状态(CT_)*/				NULL,
		/*返回类型(SDT_)*/				SDT_BOOL,
		/*此值保留*/						0,
		/*对象等级(LVL_)*/				LVL_SIMPLE,
		/*图像索引*/						0,
		/*图像数量*/						0,
		/*参数个数*/						sizeof(Stop_CommandArgs) / sizeof(Stop_CommandArgs[0]),
		/*参数信息*/						Stop_CommandArgs
	},
	{
		/*中文名称*/						_WT("InitTCPEnv"),
		/*英文名称*/						_WT("InitTCPEnv"),
		/*对象描述*/						_WT("初始化TCP环境"),
		/*所属类别(-1是数据类型的方法)*/	2,
		/*命令状态(CT_)*/				NULL,
		/*返回类型(SDT_)*/				SDT_BOOL,
		/*此值保留*/						0,
		/*对象等级(LVL_)*/				LVL_SIMPLE,
		/*图像索引*/						0,
		/*图像数量*/						0,
		/*参数个数*/						0,
		/*参数信息*/						0
	},
	{
		/*中文名称*/						_WT("EndTCPEnv"),
		/*英文名称*/						_WT("EndTCPEnv"),
		/*对象描述*/						_WT("清除TCP环境"),
		/*所属类别(-1是数据类型的方法)*/	2,
		/*命令状态(CT_)*/				NULL,
		/*返回类型(SDT_)*/				SDT_BOOL,
		/*此值保留*/						0,
		/*对象等级(LVL_)*/				LVL_SIMPLE,
		/*图像索引*/						0,
		/*图像数量*/						0,
		/*参数个数*/						0,
		/*参数信息*/						0
	},
	{
		/*中文名称*/						_WT("Connect"),
		/*英文名称*/						_WT("ConnectServer"),
		/*对象描述*/						_WT("链接服务器,返回句柄,此句柄可用于发送数据,此句柄并非SOCEKT句柄,而是一个重叠结构指针."),
		/*所属类别(-1是数据类型的方法)*/	2,
		/*命令状态(CT_)*/				NULL,
		/*返回类型(SDT_)*/				SDT_INT,
		/*此值保留*/						0,
		/*对象等级(LVL_)*/				LVL_SIMPLE,
		/*图像索引*/						0,
		/*图像数量*/						0,
		/*参数个数*/						sizeof(Connect_CommandArgs) / sizeof(Connect_CommandArgs[0]),
		/*参数信息*/						Connect_CommandArgs
	},
	{
		/*中文名称*/						_WT("Send"),
		/*英文名称*/						_WT("Send"),
		/*对象描述*/						_WT("发送数据,返回值：0（成功） -1（SOCKET无效） -2 发送失败 可用WSAGetLastError()获取错误代码"),
		/*所属类别(-1是数据类型的方法)*/	2,
		/*命令状态(CT_)*/				NULL,
		/*返回类型(SDT_)*/				SDT_INT,
		/*此值保留*/						0,
		/*对象等级(LVL_)*/				LVL_SIMPLE,
		/*图像索引*/						0,
		/*图像数量*/						0,
		/*参数个数*/						sizeof(TCPSend_CommandArgs) / sizeof(TCPSend_CommandArgs[0]),
		/*参数信息*/						TCPSend_CommandArgs
	},
	{
		/*中文名称*/						_WT("Close"),
		/*英文名称*/						_WT("Close"),
		/*对象描述*/						_WT("关闭这个Socket链接"),
		/*所属类别(-1是数据类型的方法)*/	2,
		/*命令状态(CT_)*/				NULL,
		/*返回类型(SDT_)*/				SDT_INT,
		/*此值保留*/						0,
		/*对象等级(LVL_)*/				LVL_SIMPLE,
		/*图像索引*/						0,
		/*图像数量*/						0,
		/*参数个数*/						sizeof(Close_CommandArgs) / sizeof(Close_CommandArgs[0]),
		/*参数信息*/						Close_CommandArgs
	},
	{
		/*中文名称*/						_WT("GetClientPort"),
		/*英文名称*/						_WT("GetClientPort"),
		/*对象描述*/						_WT("获取客户端端口"),
		/*所属类别(-1是数据类型的方法)*/	2,
		/*命令状态(CT_)*/				NULL,
		/*返回类型(SDT_)*/				SDT_INT,
		/*此值保留*/						0,
		/*对象等级(LVL_)*/				LVL_SIMPLE,
		/*图像索引*/						0,
		/*图像数量*/						0,
		/*参数个数*/						sizeof(GetClientPort_CommandArgs) / sizeof(GetClientPort_CommandArgs[0]),
		/*参数信息*/						GetClientPort_CommandArgs
	},

};
#endif


INT WINAPI ProcessNotifyLib(INT nMsg, DWORD dwParam1, DWORD dwParam2)
{
	INT nRet = NR_OK;
	switch (nMsg)
	{
	case NL_SYS_NOTIFY_FUNCTION:
		//g_fnNotifySys = (PFN_NOTIFY_SYS)dwParam1;
		break;
	case NL_FREE_LIB_DATA:
		break;
	default:
		nRet = NR_ERR;
		break;
	}

	////调用用户代码
	//if (g_fn_OnSysNotify)
	//	nRet = g_fn_OnSysNotify(nMsg, dwParam1, dwParam2);

	return nRet;
}

//////////////////////////////////////////////////////////////////////
//////// -----------------支持库消息处理函数------------------//////
//////////////////////////////////////////////////////////////////////

EXTERN_C INT WINAPI  _ProcessNotifyLib(INT nMsg, DWORD dwParam1, DWORD dwParam2)
{
#ifndef __E_STATIC_LIB
	if (nMsg == NL_GET_CMD_FUNC_NAMES) // 返回所有命令实现函数的的函数名称数组(char*[]), 支持静态编译的动态库必须处理
		return (INT)CommandNames;
	else if (nMsg == NL_GET_NOTIFY_LIB_FUNC_NAME) // 返回处理系统通知的函数名称(PFN_NOTIFY_LIB函数名称), 支持静态编译的动态库必须处理
		return (INT)"_ProcessNotifyLib";
	else if (nMsg == NL_GET_DEPENDENT_LIBS) return (INT)NULL;
	// 返回静态库所依赖的其它静态库文件名列表(格式为\0分隔的文本,结尾两个\0), 支持静态编译的动态库必须处理
	// kernel32.lib user32.lib gdi32.lib 等常用的系统库不需要放在此列表中
	// 返回NULL或NR_ERR表示不指定依赖文件  
#endif
	return ProcessNotifyLib(nMsg, dwParam1, dwParam2);
};


///////////////////////////////////////
////////   定义支持库基本信息   //////
///////////////////////////////////////

#ifndef __E_STATIC_LIB
static LIB_INFO LibInfo =
{
	/* { 库格式号, GUID串号, 主版本号, 次版本号, 构建版本号, 系统主版本号, 系统次版本号, 核心库主版本号, 核心库次版本号,
	支持库名, 支持库语言, 支持库描述, 支持库状态,
	作者姓名, 邮政编码, 通信地址, 电话号码, 传真号码, 电子邮箱, 主页地址, 其它信息,
	类型数量, 类型指针, 类别数量, 命令类别, 命令总数, 命令指针, 命令入口,
	附加功能, 功能描述, 消息指针, 超级模板, 模板描述,
	常量数量, 常量指针, 外部文件} */
	LIB_FORMAT_VER,/*库格式号*/
	_T(LIB_GUID_STR),/*串号*/
	LIB_MajorVersion,/*主版本号*/
	LIB_MinorVersion,/*次版本号*/
	LIB_BuildNumber,/*构建版本号*/
	LIB_SysMajorVer,/*系统主版本号*/
	LIB_SysMinorVer,/*系统次版本号*/
	LIB_KrnlLibMajorVer,/*核心库主版本号*/
	LIB_KrnlLibMinorVer,/*核心库次版本号*/
	_T(LIB_NAME_STR),/*支持库名*/
	__GBK_LANG_VER,/**/
	_WT(LIB_DESCRIPTION_STR),/*支持库描述*/
	_LIB_OS(__OS_WIN),/**/
	_WT(LIB_Author),/*作者姓名*/
	_WT(LIB_ZipCode),/*邮政编码*/
	_WT(LIB_Address),/*通信地址*/
	_WT(LIB_Phone),/*电话号码*/
	_WT(LIB_Fax),/*传真号码*/
	_WT(LIB_Email),/*电子邮箱*/
	_WT(LIB_HomePage),/*主页地址*/
	_WT(LIB_Other),/*其它信息*/
	mDataTypesIsNull ? NULL : sizeof(DataTypes) / sizeof(DataTypes[0]),/*类型数量*/
	mDataTypesIsNull ? NULL : DataTypes,/*类型指针*/
	LIB_TYPE_COUNT,/*类别数量*/
	_WT(LIB_TYPE_STR),/*命令类别*/
	sizeof(Commands) / sizeof(Commands[0]),/*命令总数*/
	Commands,/*命令指针*/
	ExecuteCommand,/*命令入口*/
	NULL,/*附加功能*/
	NULL,/*功能描述*/
	_ProcessNotifyLib,/*消息指针*/
	NULL,/*超级模板*/
	NULL,/*模板描述*/
	mConstsIsNull ? NULL : sizeof(Consts) / sizeof(Consts[0]),/*常量数量*/
	mConstsIsNull ? NULL : Consts,/*常量指针*/
	NULL/*外部文件*/
};


EXTERN_C _declspec(dllexport) PLIB_INFO GetNewInf()
{
	return (&LibInfo);
};

#endif
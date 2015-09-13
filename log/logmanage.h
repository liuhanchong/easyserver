/*
 * logmanage.h
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#ifndef LOG_LOGMANAGE_H_
#define LOG_LOGMANAGE_H_

#include <errno.h>
#include <string.h>
#include <stdio.h>

/*错误级别
 * 1 debug		  调试信息
 * 2 log 		  输出的日志
 * 3 error 		  错误信息
 * 4 system error 系统信息
 * */

#define DEBUG_SERVER /*定义此项为调试模式*/

/*当前服务器定义的运行错误信息*/
#define ERROR_ARGNULL    ("传递的参数不能为空")
#define ERROR_CRETHREAD  ("创建线程失败")
#define ERROR_CLOTHREAD  ("关闭线程失败")
#define ERROR_DETTHREAD  ("分离线程失败")
#define ERROR_TRANTYPE   ("强制转换数据失败")
#define ERROR_SETMODE    ("设置模式失败")
#define ERROR_PROFUNNULL ("线程的处理函数为空")
#define ERROR_OUTQUEUE   ("超出队列的最大长度")
#define ERROR_INSEQUEUE  ("插入元素失败")
#define ERROR_DELQUEUE   ("删除元素失败")
#define ERROR_EDITQUEUE  ("修改数据失败")
#define ERROR_FILEDDB    ("获取结果集字段失败")
#define ERROR_READDINI 	 ("读取配置文件失败")
#define ERROR_INITQUEUE  ("初始化队列失败")
#define ERROR_CREPOOL 	 ("创建池失败")
#define ERROR_DBOPEN 	 ("数据库打开失败")
#define ERROR_CREAIOX 	 ("创建AIO失败")
#define ERROR_RELPOOL    ("释放池失败")
#define ERROR_RELQUEUE   ("释放队列失败")
#define ERROR_REMEVENT   ("删除事件失败")
#define ERROR_DISCONN  	 ("SOCKET连接断开")
#define ERROR_CLOSOCKET  ("关闭SOCKET失败")
#define ERROR_RELAIOX    ("释放异步IO失败")
#define ERROR_ADDEVENT 	 ("增加监听事件失败")
#define ERROR_INITDATA   ("初始化数据处理失败")
#define ERROR_RELEDATA   ("释放数据处理失败")
#define ERROR_MAIN       ("MAIN运行失败")
#define ERROR_ARGNUMBER  ("参数个数不符合规定")
#define ERROR_START      ("启动服务器失败")
#define ERROR_RELSHAMEM  ("释放共享内存失败")
#define ERROR_STOP       ("停止服务器失败")
#define ERROR_ARGVALUE   ("不匹配的参数")
#define ERROR_CRESHAMEM  ("创建共享内存失败")
#define ERROR_SERISRUN   ("服务器已启动")
#define ERROR_SERNORUN   ("服务器未启动")
#define ERROR_GETSHAMEM  ("获取共享内存失败")
#define ERROR_CUTSHAMEM  ("断开共享内存失败")
#define ERROR_CRESOCKET  ("创建SOCKET失败")
#define ERROR_CRELISTEN  ("创建监听失败")
#define ERROR_CREACCEPT  ("创建接收SOCKET失败")
#define ERROR_CRESIGNAL  ("创建信号失败")
#define ERROR_INITLOG    ("初始化LOG失败")
#define ERROR_RELLOG     ("释放LOG失败")
#define ERROR_GETFILE    ("获取文件失败")
#define ERROR_RELINI     ("释放配置文件失败")
#define ERROR_INSOCKET   ("插入SOCKET失败")

void DebugInfor(char *pDebug); /*在调试时，打印的一些运行时信息，当版本发布时候，此函数输出的信息会被屏蔽*/
void LogInfor(char *pLog); /*日常运行情况的记录*/
void ErrorInfor(char *pFunctionName, char *pError); /*根据服务器定义的错误信息，输出相应的信息*/
void SystemErrorInfor(char *pFunctionName, int nErrorNo);	/*针对于Linux系统直接输出errno码所对应的信息*/

#endif /* LOG_LOGMANAGE_H_ */

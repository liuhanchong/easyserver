/*
 * ini.h
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#ifndef CONFIG_CONFIG_H_
#define CONFIG_CONFIG_H_

#include <stdlib.h>
#include <memory.h>
#include "../log/logmanage.h"

typedef struct Ini
{
	FILE *pFile;
	char *pPath;
	int nRowMaxLength;
} Ini;

/*接口*/
int InitIni(Ini *pIni, char *chPath, int nRowMaxLength);
char *GetString(Ini *pIni, char *pSection, char *pKey, char *pDef);/*此处获取的指针需要释放*/
int GetInt(Ini *pIni, char *pSection, char *pKey, int nDefault);
double GetDouble(Ini *pIni, char *pSection, char *pKey, double dDefault);
int ReleaseIni(Ini *pIni);

/*私有*/
int FindValue(Ini *pIni, char *pSection, char *pKey, char *pValue, int nSize);
int FindSection(char *pSection,  char *pValue);
int FindKey(char *pKey, char *pValue);
char *GetLine(char *pText, int nSize, FILE *pFile);
int ClearSpace(char *pText, int nSize);
int FindChar(char ch, char *pText);

#endif /* CONFIG_CONFIG_H_ */

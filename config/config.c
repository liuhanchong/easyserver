/*
 * ini.c
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#include "config.h"

int InitIni(Ini *pIni, char *chPath, int nRowMaxLength)
{
	if (!chPath || !pIni || nRowMaxLength < 1)
	{
		printf("%s-参数不能为空!\n", "InitIni");
		return 0;
	}

	pIni->pFile = fopen(chPath, "r+");
	if (!pIni->pFile)
	{
		printf("%s-%s!\n", "InitIni", strerror(errno));
		return 0;
	}

	pIni->pPath = (char *)malloc(strlen(chPath));
	if (!pIni->pPath)
	{
		printf("%s-%s!\n", "InitIni", strerror(errno));
		fclose(pIni->pFile);
		return 0;
	}

	pIni->nRowMaxLength = nRowMaxLength;
	memcpy(pIni->pPath, chPath, strlen(chPath));

	return 1;
}

char *GetString(Ini *pIni, char *pSection, char *pKey, char *pDef)
{
	if (!pIni || !pSection || !pKey || !pDef)
	{
		ErrorInfor("GetString", ERROR_ARGNULL);
		return NULL;
	}

	char *pValue = (char *)malloc(pIni->nRowMaxLength);
	if (!pValue)
	{
		SystemErrorInfor("GetString", errno);
		return NULL;
	}

	if (!FindValue(pIni, pSection, pKey, pValue, pIni->nRowMaxLength))
	{
		/*判定默认值是否超过当前数组最大值*/
		int nSize = strlen(pDef);
		if (nSize >= pIni->nRowMaxLength)
		{
			nSize = pIni->nRowMaxLength - 1;
		}
		memcpy(pValue, pDef, nSize);
		pValue[nSize] = '\0';
	}

	return pValue;
}

int GetInt(Ini *pIni, char *pSection, char *pKey, int nDefault)
{
	if (!pIni || !pSection || !pKey)
	{
		ErrorInfor("GetInt", ERROR_ARGNULL);
		return nDefault;
	}

	char *pValue = GetString(pIni, pSection, pKey, "");
	int nValue = (strlen(pValue) == 0) ? nDefault : atoi(pValue);
	free(pValue);
	pValue = NULL;

	return nValue;
}

double GetDouble(Ini *pIni, char *pSection, char *pKey, double dDefault)
{
	if (!pIni || !pSection || !pKey)
	{
		ErrorInfor("GetInt", ERROR_ARGNULL);
		return dDefault;
	}

	char *pValue = GetString(pIni, pSection, pKey, "");
	double dValue = (strlen(pValue) == 0) ? dDefault : atof(pValue);
	free(pValue);
	pValue = NULL;

	return dValue;
}

int ReleaseIni(Ini *pIni)
{
	if (!pIni)
	{
		printf("%s-参数不能为空!\n", "ReleaseIni");
		return 0;
	}

	fclose(pIni->pFile);
	free(pIni->pPath);

	return 1;
}

int FindValue(Ini *pIni, char *pSection, char *pKey, char *pValue, int nSize)
{
	if (!pIni || !pSection || !pKey || !pValue || nSize <= 0 || !pIni->pFile)
	{
		ErrorInfor("FindValue", ERROR_ARGNULL);
		return 0;
	}

	if (fseek(pIni->pFile, 0, SEEK_SET) != 0)
	{
		SystemErrorInfor("FindValue-1", errno);
		return 0;
	}

	char *pText = (char *)malloc(pIni->nRowMaxLength);
	if (!pText)
	{
		SystemErrorInfor("FindValue-2", errno);
		return 0;
	}

	while (GetLine(pText, pIni->nRowMaxLength, pIni->pFile))
	{
		//找到段域
		if (FindSection(pSection, pText))
		{
			while (GetLine(pText, pIni->nRowMaxLength, pIni->pFile))
			{
				/*找到下一个段，证明在当前段没找到*/
				if ((FindChar('[', pText) >= 0) && (FindChar(']', pText) >= 0))
				{
					break;
				}

				//找到值
				if (FindKey(pKey, pText))
				{
					/*分析数值*/
					int nIndex = FindChar('=', pText) + 1;
					memcpy(pValue, (pText + nIndex), (strlen(pText) - nIndex));
					pValue[strlen(pText) - nIndex] = '\0';

					free(pText);
					pText = NULL;

					return 1;
				}
			}
		}
	}

	free(pText);
	pText = NULL;

	return 0;
}

int FindSection(char *pSection, char *pValue)
{
	if (!pSection || !pValue)
	{
		ErrorInfor("FindSection", ERROR_ARGNULL);
		return 0;
	}

	int nBegin = -1;
	int nEnd = -1;
	nBegin = FindChar('[', pValue) + 1;
	nEnd = FindChar(']', pValue);

	int nSize = nEnd - nBegin;
	if (nBegin >= 0 && nEnd >= nBegin && nSize == strlen(pSection))
	{
		for (int i = 0; i < nSize; i++)
		{
			if (pValue[i + nBegin] != pSection[i])
			{
				return 0;
			}
		}

		return 1;
	}

	return 0;
}

int FindKey(char *pKey, char *pValue)
{
	if (!pKey || !pValue)
	{
		ErrorInfor("FindKey", ERROR_ARGNULL);
		return 0;
	}

	int nIndex = FindChar('=', pValue);
	if (nIndex == strlen(pKey))
	{
		for (int i = 0; i < nIndex; i++)
		{
			if (pValue[i] != pKey[i])
			{
				return 0;
			}
		}

		return 1;
	}

	return 0;
}

char *GetLine(char *pText, int nSize, FILE *pFile)
{
	if (!pText || nSize <= 0 || !pFile)
	{
		ErrorInfor("GetLine", ERROR_ARGNULL);
		return NULL;
	}

	char *pRet = fgets(pText, nSize, pFile);
	if (!pRet)
	{
		return NULL;
	}

	//删除\r\n
	int nIndex = FindChar('\r', pText);
	if (nIndex >= 0)
	{
		pText[nIndex] = '\0';
	}

	nIndex = FindChar('\n', pText);
	if (nIndex >= 0)
	{
		pText[nIndex] = '\0';
	}

	//找到;号标志
	nIndex = FindChar(';', pText);
	if (nIndex >= 0)
	{
		pText[nIndex] = '\0';
	}

	//清楚左右的空格
	int nLen = strlen(pText);
	ClearSpace(pText, nLen);

	//清除等号左右的空格
	nIndex = FindChar('=', pText);
	if (nIndex >= 0 && nIndex < nLen)
	{
		pText[nIndex] = ' ';

		ClearSpace(pText, nIndex + 1);

		nLen = strlen(pText);
		pText[nLen] = '=';

		ClearSpace(pText + nLen + 1, nSize - nLen);
	}

	return pRet;
}

int ClearSpace(char *pText, int nSize)
{
	if (!pText)
	{
		ErrorInfor("ClearSpace", ERROR_ARGNULL);
		return 0;
	}

	int nBegin = -1;
	int nEnd = -1;
	for (int i = 0; i < nSize; i++)
	{
		if (pText[i] != ' ')
		{
			if (nBegin == -1)
			{
				nBegin = i;
			}

			nEnd = i;
		}
	}

	if (nBegin >= 0 && nEnd >= nBegin)
	{
		int nSize = nEnd - nBegin + 1;
		for (int i = 0; i < nSize; i++)
		{
			pText[i] = pText[i + nBegin];
		}

		pText[nEnd - nBegin + 1] = '\0';
	}
	else
	{
		pText[0] = '\0';
	}

	return 1;
}

int FindChar(char ch, char *pText)
{
	if (!pText)
	{
		ErrorInfor("FindChar", ERROR_ARGNULL);
		return -1;
	}

	char *pIndex = pText;
	while (*pIndex != '\0' && *(pIndex) != ch)
	{
		pIndex++;
	}

	return (*pIndex == ch) ? (pIndex - pText) : -1;
}

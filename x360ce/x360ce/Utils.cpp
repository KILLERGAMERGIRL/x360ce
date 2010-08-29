/*  x360ce - XBOX360 Controler Emulator
 *  Copyright (C) 2002-2010 ToCA Edit
 *
 *  x360ce is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  x360ce is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with x360ce.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include "globals.h"
#include "DirectInput.h"
#include "Utils.h"

BOOL writelog = 0;
LPWSTR lpLogFileName;

DWORD ReadStringFromFile(LPCWSTR strFileSection, LPCWSTR strKey, LPWSTR strOutput)
{
	return ReadStringFromFile(strFileSection, strKey, strOutput, NULL);
}

DWORD ReadStringFromFile(LPCWSTR strFileSection, LPCWSTR strKey, LPWSTR strOutput, LPWSTR strDefault)
{
	if(lpConfigFile) {
		DWORD ret;
		LPTSTR next_token;
		ret = GetPrivateProfileString(strFileSection, strKey, strDefault, strOutput, MAX_PATH, lpConfigFile);
		if(ret) wcstok_s (strOutput,L" ",&next_token);
		return ret;
	}
	return 0;
}

LPCWSTR ModuleFileName()
{
	LPWSTR pStr;
	static WCHAR strPath[MAX_PATH];
	GetModuleFileName (NULL, strPath, MAX_PATH);
	pStr = wcsrchr(strPath, L'\\') +1;
	return pStr;
}

UINT ReadUINTFromFile(LPCTSTR strFileSection, LPCTSTR strKey)
{
	return ReadUINTFromFile(strFileSection, strKey, NULL);
}

UINT ReadUINTFromFile(LPCTSTR strFileSection, LPCTSTR strKey ,UINT uDefault)
{
	if (lpConfigFile) return GetPrivateProfileInt(strFileSection,strKey,uDefault,lpConfigFile);
	return 0;
}

VOID CreateLog()
{
	if (writelog) {

		SYSTEMTIME systime;
		GetLocalTime(&systime);

		lpLogFileName = new TCHAR[MAX_PATH];
		swprintf_s(lpLogFileName,MAX_PATH,L"x360ce\\x360ce %u%02u%02u-%02u%02u%02u.log",
			systime.wYear,systime.wMonth,systime.wDay,systime.wHour,systime.wMinute,systime.wSecond);

		if( (GetFileAttributes(L"x360ce") == INVALID_FILE_ATTRIBUTES) ) CreateDirectory(L"x360ce", NULL);
	}
}

BOOL WriteLog(LPTSTR str,...)
{
	if (writelog) {
		SYSTEMTIME systime;
		GetLocalTime(&systime);

		FILE * fp;
		_wfopen_s(&fp, lpLogFileName, L"a");

		//fp is null, file is not open.
		if (fp==NULL)
			return -1;
		fwprintf(fp, L"%02u:%02u:%02u.%03u:: ", systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
		va_list arglist;
		va_start(arglist,str);
		vfwprintf(fp,str,arglist);
		va_end(arglist);
		fwprintf(fp, L" \n");
		fclose(fp);
		return 1;
	}
	return 0;
}

LONG clamp(LONG val, LONG min, LONG max)
{
	if (val < min) return min;
	if (val > max) return max;
	return val;
}
LONG deadzone(LONG val, LONG min, LONG max, LONG lowerDZ, LONG upperDZ)
{
	if (val < lowerDZ) return min;
	if (val > upperDZ) return max;
	return val;
}

inline static WORD flipShort(WORD s) 
{
	return (s>>8) | (s<<8);
}

inline static DWORD flipLong(DWORD l) 
{
	return (((DWORD)flipShort((WORD)l))<<16) | flipShort((WORD)(l>>16));
}

void GUIDtoString(LPWSTR data, const GUID *pg) 
{
	swprintf_s(data, 50,
		_T("%08X-%04X-%04X-%04X-%04X%08X"),
		pg->Data1, (DWORD)pg->Data2, (DWORD)pg->Data3,
		flipShort(((WORD*)pg->Data4)[0]), 
		flipShort(((WORD*)pg->Data4)[1]),
		flipLong(((DWORD*)pg->Data4)[1]));
}

BOOL StringToGUID(GUID *pg, TCHAR *dataw) 
{
	char data[100];
	if (_tcslen(dataw) > 50) return 0;
	int w = 0;
	while (dataw[w]) {
		data[w] = (char) dataw[w];
		w++;
	}
	data[w] = 0;
	DWORD temp[5];
	sscanf_s(data, "%08X-%04X-%04X-%04X-%04X%08X",
		&pg->Data1, temp, temp+1,
		temp+2, temp+3, temp+4);
	pg->Data2 = (WORD) temp[0];
	pg->Data3 = (WORD) temp[1];
	((WORD*)pg->Data4)[0] = flipShort((WORD)temp[2]);
	((WORD*)pg->Data4)[1] = flipShort((WORD)temp[3]);
	((DWORD*)pg->Data4)[1] = flipLong(temp[4]);
	return 1;
}

LPTSTR const DXErrStr(HRESULT dierr) 
{
	if (dierr == DIERR_ACQUIRED) return _T("DIERR_ACQUIRED");
	if (dierr == DI_BUFFEROVERFLOW) return _T("DI_BUFFEROVERFLOW");
	if (dierr == DI_DOWNLOADSKIPPED) return _T("DI_DOWNLOADSKIPPED");
	if (dierr == DI_EFFECTRESTARTED) return _T("DI_EFFECTRESTARTED");
	if (dierr == DI_NOEFFECT) return _T("DI_NOEFFECT");
	if (dierr == DI_NOTATTACHED) return _T("DI_NOTATTACHED");
	if (dierr == DI_OK) return _T("DI_OK");
	if (dierr == DI_POLLEDDEVICE) return _T("DI_POLLEDDEVICE");
	if (dierr == DI_PROPNOEFFECT) return _T("DI_PROPNOEFFECT");
	if (dierr == DI_SETTINGSNOTSAVED) return _T("DI_SETTINGSNOTSAVED");
	if (dierr == DI_TRUNCATED) return _T("DI_TRUNCATED");
	if (dierr == DI_TRUNCATEDANDRESTARTED) return _T("DI_TRUNCATEDANDRESTARTED");
	if (dierr == DI_WRITEPROTECT) return _T("DI_WRITEPROTECT");
	if (dierr == DIERR_ACQUIRED) return _T("DIERR_ACQUIRED");
	if (dierr == DIERR_ALREADYINITIALIZED) return _T("DIERR_ALREADYINITIALIZED");
	if (dierr == DIERR_BADDRIVERVER) return _T("DIERR_BADDRIVERVER");
	if (dierr == DIERR_BETADIRECTINPUTVERSION) return _T("DIERR_BETADIRECTINPUTVERSION");
	if (dierr == DIERR_DEVICEFULL) return _T("DIERR_DEVICEFULL");
	if (dierr == DIERR_DEVICENOTREG) return _T("DIERR_DEVICENOTREG");
	if (dierr == DIERR_EFFECTPLAYING) return _T("DIERR_EFFECTPLAYING");
	if (dierr == DIERR_GENERIC) return _T("DIERR_GENERIC");
	if (dierr == DIERR_HANDLEEXISTS) return _T("DIERR_HANDLEEXISTS");
	if (dierr == DIERR_HASEFFECTS) return _T("DIERR_HASEFFECTS");
	if (dierr == DIERR_INCOMPLETEEFFECT) return _T("DIERR_INCOMPLETEEFFECT");
	if (dierr == DIERR_INPUTLOST) return _T("DIERR_INPUTLOST");
	if (dierr == DIERR_INVALIDPARAM) return _T("DIERR_INVALIDPARAM");
	if (dierr == DIERR_MAPFILEFAIL) return _T("DIERR_MAPFILEFAIL");
	if (dierr == DIERR_MOREDATA) return _T("DIERR_MOREDATA");
	if (dierr == DIERR_NOAGGREGATION) return _T("DIERR_NOAGGREGATION");
	if (dierr == DIERR_NOINTERFACE) return _T("DIERR_NOINTERFACE");
	if (dierr == DIERR_NOTACQUIRED) return _T("DIERR_NOTACQUIRED");
	if (dierr == DIERR_NOTBUFFERED) return _T("DIERR_NOTBUFFERED");
	if (dierr == DIERR_NOTDOWNLOADED) return _T("DIERR_NOTDOWNLOADED");
	if (dierr == DIERR_NOTEXCLUSIVEACQUIRED) return _T("DIERR_NOTEXCLUSIVEACQUIRED");
	if (dierr == DIERR_NOTFOUND) return _T("DIERR_NOTFOUND");
	if (dierr == DIERR_NOTINITIALIZED) return _T("DIERR_NOTINITIALIZED");
	if (dierr == DIERR_OBJECTNOTFOUND) return _T("DIERR_OBJECTNOTFOUND");
	if (dierr == DIERR_OLDDIRECTINPUTVERSION) return _T("DIERR_OLDDIRECTINPUTVERSION");
	if (dierr == DIERR_OTHERAPPHASPRIO) return _T("DIERR_OTHERAPPHASPRIO");
	if (dierr == DIERR_OUTOFMEMORY) return _T("DIERR_OUTOFMEMORY");
	if (dierr == DIERR_READONLY) return _T("DIERR_READONLY");
	if (dierr == DIERR_REPORTFULL) return _T("DIERR_REPORTFULL");
	if (dierr == DIERR_UNPLUGGED) return _T("DIERR_UNPLUGGED");
	if (dierr == DIERR_UNSUPPORTED) return _T("DIERR_UNSUPPORTED");
	if (dierr == E_HANDLE) return _T("E_HANDLE");
	if (dierr == E_PENDING) return _T("E_PENDING");
	if (dierr == E_POINTER) return _T("E_POINTER");
	
	TCHAR *buffer = NULL;
	_itot_s(dierr,buffer,arrayof(buffer),16);	//as hex
	return buffer;	//return value of HRESULT

}
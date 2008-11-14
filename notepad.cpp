/*
 * Copyright (C) 2005 Stephane Chambrin <superstepho@yahoo.fr>
 * Copyright (C) 2008 Przemyslaw Pawelczyk <przemoc@gmail.com>
 *
 * This software is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2.
 * See <http://www.gnu.org/licenses/gpl-2.0.txt>.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/*
 * Notepad++ launcher (for Windows notepad.exe replacement)
 * version 0.1
 */

/*
 * Copy built notepad.exe to (in given order):
 * - C:\WINDOWS\SERVICEPACKFILES\i386\
 * - C:\WINDOWS\SYSTEM32\dllcache\
 * - C:\WINDOWS\SYSTEM32\
 * - C:\WINDOWS\
 *
 * How-to build (using MinGW):
 *   windres -i notepad.rc --input-format=rc -o notepad.res -O coff
 *   mingw32-gcc -O2 notepad.cpp notepad.res -o notepad.exe -mwindows -s
 */


#include <windows.h>
#include <winreg.h>

int WINAPI WinMain(HINSTANCE hThisInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpszArgument,
                   int nFunsterStil)

{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	HKEY hKey;
	DWORD exitcode = 1,
	      arglen = strlen(lpszArgument);
	char cmd[1024] = "\"C:\\Program Files\\Notepad++\\notepad++.exe\"";

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	/* Getting path to Notepad++ from registry */
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Notepad++", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD iType,
		      iDataSize = 1024;
		unsigned char sData[1024];

		if (RegQueryValueEx(hKey, "", NULL, &iType, sData, &iDataSize) == ERROR_SUCCESS)
		{
			strcpy(cmd, "\"");
			strcat(cmd, (char*) sData);
			strcat(cmd, "\\notepad++.exe\"");
		}

		RegCloseKey(hKey);
	}

	/* Command-line construction */
	if (arglen)
	{
		strcat(cmd, " \"");
		if (lpszArgument[0] != '\"')
		{
			_fullpath(cmd + strlen(cmd), lpszArgument, 1000);
		}
		else
		{
			if (lpszArgument[arglen - 1] == '\"')
			{
				lpszArgument[arglen - 1] = '\0';
			}
			_fullpath(cmd + strlen(cmd), lpszArgument + 1, 1000);
		}
		strcat(cmd, "\"");
	}

	/* Notepad++ launching */
	if (CreateProcess(
		NULL,	/* No module name (use command line) */
		cmd,	/* Command line */
		NULL,	/* Process handle not inheritable */
		NULL,	/* Thread handle not inheritable */
		FALSE,	/* Set handle inheritance to FALSE */
		0,	/* No creation flags */
		NULL,	/* Use parent's environment block */
		NULL,	/* Use parent's starting directory */
		&si,	/* Pointer to STARTUPINFO structure */
		&pi	/* Pointer to PROCESS_INFORMATION structure */
	)) {

		/* Przemoc's note:
		   Without any WaitFor* functions this launcher was useless,
		   when it was used in conjuction with file archivers (e.g. 7-Zip)
		   or other applications that provides files temporarily. */
#if WAIT_FOR_EXIT == 0
		/* Wait until Notepad++ has finished its initialization */
		WaitForInputIdle(pi.hProcess, INFINITE);
		exitcode = 0;
#else
		/* Wait until Notepad++ exits. */
		WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &exitcode);
#endif

		/* Close process and thread handles */
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	return exitcode;
}

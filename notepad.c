/*
 * Copyright (C) 2005 Stephane Chambrin <superstepho@yahoo.fr>
 * Copyright (C) 2008-2009,2017 Przemyslaw Pawelczyk <przemoc@gmail.com>
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
 * version 0.6
 */


#define _ISOC99_SOURCE

#ifndef WAIT_FOR_EXIT
#define WAIT_FOR_EXIT 1
#endif

#include <windows.h>
#include <winreg.h>
#include <io.h>

#ifndef __x86_64__
#define __x86_64__ 0
#endif


static long
query_regkey(HKEY hkey, char *subkey, ULONG rights,
             char *buf, unsigned long *siz)
{
	LONG res;
	HKEY hsubkey;

	res = RegOpenKeyEx(hkey, subkey, 0, KEY_QUERY_VALUE | rights, &hsubkey);
	if (res != ERROR_SUCCESS)
		return res;

	res = RegQueryValueEx(hsubkey, "", NULL, /* type */ NULL, (LPBYTE)buf, siz);

	RegCloseKey(hsubkey);

	return res;
}


static long
query_envvar(char *envvar, char *buf, unsigned long *siz)
{
	LONG res;

	res = GetEnvironmentVariable(envvar, buf, *siz);
	if (res)
		*siz = res;

	return !res;
}


int WINAPI
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{
	(void)hInstance;
	(void)hPrevInstance;
	(void)nCmdShow;

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	HWND hWnd;
	BOOL bIsWow64 = FALSE;
	DWORD dwProcessId;
	DWORD dwExitCode = 1;
	DWORD dwCmdLineLen = strlen(lpCmdLine);
	TCHAR cmd[1024];
	TCHAR szNotepadKey[] = "SOFTWARE\\Notepad++";
	TCHAR szProgFilesVar[] = "ProgramFiles";
#if __x86_64__
	TCHAR szProgFilesAltVar[] = "ProgramFiles(x86)";
#else
	TCHAR szProgFilesAltVar[] = "ProgramW6432";
#endif
	unsigned long siz;
	long res;
	int found = 0;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	IsWow64Process(GetCurrentProcess(), &bIsWow64);

	cmd[0] = '"';

	/* Getting path to Notepad++ from registry. */
	if (!found) {
		siz = sizeof(cmd) - 3;
		res = query_regkey(HKEY_LOCAL_MACHINE, szNotepadKey,
		                   0,
		                   &cmd[1], &siz);
		if (!res) {
			siz--; /* terminating null */
			strncpy(&cmd[1 + siz],
			        "\\notepad++.exe", sizeof(cmd) - 2 - siz);
			found += !access(&cmd[1], 00);
			cmd[1 + siz + sizeof("\\notepad++.exe") - 1] = '"';
		}
	}

	/* Getting path to Notepad++ from registry - alternative way. */
	if (!found && (bIsWow64 || __x86_64__)) {
		siz = sizeof(cmd) - 3;
		res = query_regkey(HKEY_LOCAL_MACHINE, szNotepadKey,
		                   bIsWow64 ? KEY_WOW64_64KEY : KEY_WOW64_32KEY,
		                   &cmd[1], &siz);
		if (!res) {
			siz--; /* terminating null */
			strncpy(&cmd[1 + siz],
			        "\\notepad++.exe", sizeof(cmd) - 2 - siz);
			found += !access(&cmd[1], 00);
			cmd[1 + siz + sizeof("\\notepad++.exe") - 1] = '"';
		}
	}

	/* Use the default path if no info in registry. */
	if (!found) {
		siz = sizeof(cmd) - 3;
		res = query_envvar(szProgFilesVar, &cmd[1], &siz);

		if (!res) {
			strncpy(&cmd[1 + siz],
			        "\\Notepad++\\notepad++.exe", sizeof(cmd) - 2 - siz);
			found += !access(&cmd[1], 00);
			cmd[1 + siz + sizeof("\\Notepad++\\notepad++.exe") - 1] = '"';
		}
	}

	/* Use the default path if no info in registry - alternative way. */
	if (!found && (bIsWow64 || __x86_64__)) {
		siz = sizeof(cmd) - 3;
		res = query_envvar(szProgFilesAltVar, &cmd[1], &siz);

		if (!res) {
			strncpy(&cmd[1 + siz],
			        "\\Notepad++\\notepad++.exe", sizeof(cmd) - 2 - siz);
			found += !access(&cmd[1], 00);
			cmd[1 + siz + sizeof("\\Notepad++\\notepad++.exe") - 1] = '"';
		}
	}

	if (!found)
		return 111;

	/* Command-line construction. */
	if (dwCmdLineLen) {
		strncat(cmd, " \"", sizeof(cmd) - strlen(cmd));
		if (lpCmdLine[0] != '\"')
			_fullpath(cmd + strlen(cmd), lpCmdLine,
			          sizeof(cmd) - strlen(cmd));
		else {
			if (lpCmdLine[dwCmdLineLen - 1] == '\"')
				lpCmdLine[dwCmdLineLen - 1] = '\0';
			_fullpath(cmd + strlen(cmd), lpCmdLine + 1,
			          sizeof(cmd) - strlen(cmd));
		}
		strncat(cmd, "\"", sizeof(cmd) - strlen(cmd));
	}

	/* Notepad++ launching. */
	if (CreateProcess(
	                  NULL,  /* No module name (use command line) */
	                  cmd,   /* Command line */
	                  NULL,  /* Process handle not inheritable */
	                  NULL,  /* Thread handle not inheritable */
	                  FALSE, /* Set handle inheritance to FALSE */
	                  0,     /* No creation flags */
	                  NULL,  /* Use parent's environment block */
	                  NULL,  /* Use parent's starting directory */
	                  &si,   /* Pointer to STARTUPINFO structure */
	                  &pi    /* Pointer to PROCESS_INFORMATION structure */
	    )
	   ) {

		/* Przemoc's note:
		   Without any WaitFor* functions this launcher was useless,
		   when it was used in conjuction with file archivers (e.g. 7z)
		   or other applications that provides files temporarily. */

		/* Wait until Notepad++ has finished its initialization */
		WaitForInputIdle(pi.hProcess, INFINITE);

		/* Find Notepad++ window */
		hWnd = GetTopWindow(0);
		while (hWnd) {
			GetWindowThreadProcessId(hWnd, &dwProcessId);
			if (dwProcessId == pi.dwProcessId) {
				/* And move it to the top. */
				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0,
				             SWP_NOMOVE | SWP_NOSIZE);
				break;
			}
			hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
		}

#if ! WAIT_FOR_EXIT > 0
		dwExitCode = 0;
#else
		/* Wait until Notepad++ exits. */
		WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &dwExitCode);
#endif

		/* Close process and thread handles. */
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	return dwExitCode;
}

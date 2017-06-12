/*
 * Copyright (C) 2005 Stephane Chambrin <superstepho@yahoo.fr>
 * Copyright (C) 2008-2009 Przemyslaw Pawelczyk <przemoc@gmail.com>
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
 * version 0.5
 */


#define _ISOC99_SOURCE

#include <windows.h>
#include <winreg.h>


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	HKEY hKey;
	HWND hWnd;
	DWORD dwProcessId,
	      dwExitCode = 1,
	      dwCmdLineLen = strlen(lpCmdLine);
	TCHAR cmd[1024],
#if __x86_64__
	      szNotepadKey[] = "SOFTWARE\\Wow6432Node\\Notepad++",
	      szProgFilesVar[] = "ProgramFiles(x86)";
#else
	      szNotepadKey[] = "SOFTWARE\\Notepad++",
	      szProgFilesVar[] = "ProgramFiles";
#endif

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	/* Getting path to Notepad++ from registry. */
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szNotepadKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
		DWORD iType,
		      iDataSize = 1024;
		unsigned char sData[1024];

		if (RegQueryValueEx(hKey, "", NULL, &iType, sData, &iDataSize) == ERROR_SUCCESS) {
			strcpy(cmd, "\"");
			strncat(cmd, (TCHAR*) sData, sizeof(cmd) - 1);
			strncat(cmd, "\\notepad++.exe\"", sizeof(cmd) - 1 - iDataSize);
		}

		RegCloseKey(hKey);
	}
	/* Use the default path if no info in registry. */
	else {
		DWORD iVarSize;
		strcpy(cmd, "\"");
		iVarSize = GetEnvironmentVariable(szProgFilesVar, &cmd[1], MAX_PATH - 1);
		strncat(cmd, "\\Notepad++\\notepad++.exe\"", sizeof(cmd) - 1 - iVarSize);
	}

	/* Command-line construction. */
	if (dwCmdLineLen) {
		strncat(cmd, " \"", sizeof(cmd) - strlen(cmd));
		if (lpCmdLine[0] != '\"')
			_fullpath(cmd + strlen(cmd), lpCmdLine, sizeof(cmd) - strlen(cmd));
		else {
			if (lpCmdLine[dwCmdLineLen - 1] == '\"')
				lpCmdLine[dwCmdLineLen - 1] = '\0';
			_fullpath(cmd + strlen(cmd), lpCmdLine + 1, sizeof(cmd) - strlen(cmd));
		}
		strncat(cmd, "\"", sizeof(cmd) - strlen(cmd));
	}

	/* Notepad++ launching. */
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

		/* Wait until Notepad++ has finished its initialization */
		WaitForInputIdle(pi.hProcess, INFINITE);

		/* Find Notepad++ window */
		hWnd = GetTopWindow(0);
		while (hWnd) {
			GetWindowThreadProcessId(hWnd, &dwProcessId);
			if (dwProcessId == pi.dwProcessId) {
				/* And move it to the top. */
				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
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

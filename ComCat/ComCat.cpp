#include "stdafx.h"
#include "windows.h"
#include "Strsafe.h"

static const _TCHAR * version = TEXT("0.1");

void ReadLoop(HANDLE hComm)
{
	DWORD dwCommEvent;
	DWORD dwRead;
	char chRead;
	BOOL ret;

	while (TRUE)
	{
		ret = ReadFile(hComm, &chRead, sizeof(chRead), &dwRead, NULL);
		if (ret && dwRead == 0) /* EOF */
		{
			break;
		}
		if (ret)
		{
			_tprintf(TEXT("%c"), chRead);
		}
	}
}

void Usage(_TCHAR* arg0)
{
	_tprintf(TEXT("Usage: %s <COM Port>\n"), arg0);
}

#define MAX_PORT_NAME   10

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2)
	{
		Usage(argv[0]);
		ExitProcess(2);
	}

	_tprintf(TEXT("ComCat Version %s\n"), version);
//	_TCHAR * portName = TEXT("\\\\.\\COM12");//argv[1];
	/* The port name will be in the range \\.\COM0 - \\.\COM99 */
	TCHAR portName[MAX_PORT_NAME];

	HRESULT hr = StringCbPrintf(portName, sizeof(portName), TEXT("\\\\.\\%s"), argv[1]);
	if (FAILED(hr))
	{
		_tprintf(TEXT("Error parsing com port argument: %s\n"), argv[1]);
		ExitProcess(2);
	}

	HANDLE hComm;
	hComm = CreateFile(
		portName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0, /* flags can be used to do overlapped io */
		NULL);

	if (hComm == INVALID_HANDLE_VALUE)
	{
		_tprintf(TEXT("Error opening port: %s\n"), portName);
		ExitProcess(2);
	}

	ReadLoop(hComm);

	CloseHandle(hComm);
	return 0;
}


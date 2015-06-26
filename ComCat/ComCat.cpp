#include "stdafx.h"
#include "windows.h"
#include "Strsafe.h"

static const char * version = "0.1";

typedef enum Command {
	CMD_READ,
	CMD_DUMP,
	CMD_CONFIG
} Command_t;

struct Arguments {
	const char * portName;
	Command_t command;
};

void ComDump(HANDLE hComm)
{
	DCB dcb = { 0 };

	if (!GetCommState(hComm, &dcb))
	{
		// Error getting current DCB settings
	}
	else
	{

	}

	printf("BaudRate: %d\n", dcb.BaudRate);
	printf("ByteSize: %d\n", dcb.ByteSize);
	printf("Parity: ");
	switch (dcb.Parity)
	{
		case NOPARITY:   printf("None"); break;
		case ODDPARITY:  printf("Odd"); break;
		case EVENPARITY: printf("Event"); break;
		default:         printf("Unknown (%d)", dcb.Parity); break;
	}
	printf("\n");
	printf("StopBits: ");
	switch (dcb.Parity)
	{
		case ONESTOPBIT:   printf("1"); break;
		case ONE5STOPBITS: printf("1.5"); break;
		case TWOSTOPBITS:  printf("2"); break;
		default:           printf("Unknown (%d)", dcb.Parity); break;
	}
	printf("\n");
}

void ComRead(HANDLE hComm)
{
	DWORD dwRead;
	char chRead;
	BOOL ret;

	while (TRUE)
	{
		ret = ReadFile(hComm, &chRead, sizeof(chRead), &dwRead, NULL);
		if (ret && dwRead == 0) /* EOF */
		{
			printf("EOF");
//			break;
		}
		if (ret)
		{
			printf("%c", chRead);
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
	if (argc < 2)
	{
		Usage(argv[0]);
		ExitProcess(2);
	}

	Arguments args = { 0 };
	args.command = CMD_READ;
	
	/* The port name will be in the range \\.\COM0 - \\.\COM99 */
	TCHAR portName[MAX_PORT_NAME];

	HRESULT hr = StringCbPrintf(portName, sizeof(portName), TEXT("\\\\.\\%s"), argv[1]);
	if (FAILED(hr))
	{
		_tprintf(TEXT("Error parsing com port argument: %s\n"), argv[1]);
		ExitProcess(2);
	}

	HANDLE hComm = CreateFile(
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

	if (args.command == CMD_DUMP)
	{
		ComDump(hComm);
	}
	if (args.command == CMD_READ)
	{
		ComRead(hComm);
	}

	CloseHandle(hComm);
	return 0;
}

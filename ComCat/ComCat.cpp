#include "stdafx.h"
#include "windows.h"
#include "Strsafe.h"
#include <stdint.h>

static const char * version = "0.1";
#define READ_BUFFER_SIZE 32

typedef enum Command {
	CMD_READ,
	CMD_DUMP,
	CMD_CONFIG
} Command_t;

struct Arguments {
	const char * portName;
	Command_t command;
};

void ComConfig(HANDLE hComm, uint32_t bauderate)
{
	DCB dcb = { 0 };

	if (!GetCommState(hComm, &dcb))
	{
		fprintf(stderr, "Error getting current com port settings\n");
		return;
	}

	dcb.BaudRate = bauderate;
	
	if (!SetCommState(hComm, &dcb))
	{
		fprintf(stderr, "Error when configuring com port\n");
	}
}

void ComDump(HANDLE hComm)
{
	DCB dcb = { 0 };

	if (!GetCommState(hComm, &dcb))
	{
		fprintf(stderr, "Failed to get the current com port settings\n");
		return;
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
	DWORD dwCommEvent;
	DWORD dwRead;
	char buffer[READ_BUFFER_SIZE];

	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		fprintf(stderr, "Error setting EV_RXCHAR mask\n");
		return;
	}

	while (TRUE)
	{
		if (WaitCommEvent(hComm, &dwCommEvent, NULL))
		{
			do {
				if (ReadFile(hComm, &buffer[0], READ_BUFFER_SIZE, &dwRead, NULL))
				{
					fwrite(&buffer[0], 1, dwRead, stdout);
				}
				else
				{
					fprintf(stderr, "Error reading com port\n");
					break;
				}
			} while (dwRead);
		}
		else
		{
			fprintf(stderr, "Error in WaitCommEvent\n");
			break;
		}
	}
}

void Usage(char * arg0)
{
	_tprintf(TEXT("Usage: %s <COM Port>\n"), arg0);
}

#define MAX_PORT_NAME   10

int main(int argc, char * argv[])
{
	if (argc < 2)
	{
		Usage(argv[0]);
		ExitProcess(2);
	}

	Arguments args = { 0 };
	args.command = CMD_READ;
	
	/* The port name will be in the range \\.\COM0 - \\.\COM99 */
	char portName[MAX_PORT_NAME];

	if (sprintf_s(portName, sizeof(portName), "\\\\.\\%s", argv[1]) == -1)
	{
		printf("Error parsing com port argument: %s\n", argv[1]);
		ExitProcess(2);
	}

	HANDLE hComm = CreateFileA(
		portName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0, /* flags can be used to do overlapped io */
		NULL);

	if (hComm == INVALID_HANDLE_VALUE)
	{
		printf("Error opening port: %s\n", portName);
		ExitProcess(2);
	}

	ComConfig(hComm, 9600);

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

#include "windows.h"
#include "wingetopt.h"
#include <stdint.h>
#include <stdio.h>

static const char * version = "0.1";
#define READ_BUFFER_SIZE 32

typedef enum Command {
	CMD_READ,
	CMD_DUMP,
	CMD_CONFIG
} Command_t;

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

void usage(char * arg0)
{
	printf("Usage: %s -p <COM Port> -s <baudrate>\n", arg0);
}

#define MAX_PORT_NAME   10

int main(int argc, char * argv[])
{
	int c;
	char * device = NULL;
	int baudrate = -1;
	Command_t command = CMD_READ;

	while ((c = getopt(argc, argv, "p:s:d")) != -1)
	{
		switch (c)
		{
		case 'p': 
			device = optarg;
			break;
		case 's':
			baudrate = atoi(optarg);
			break;
		case 'd':
			command = CMD_DUMP;
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (device == NULL)
	{
		usage(argv[0]);
		return 1;
	}

	/* The port name will be in the range \\.\COM0 - \\.\COM99 */
	char portName[MAX_PORT_NAME];

	if (sprintf_s(portName, sizeof(portName), "\\\\.\\%s", device) == -1)
	{
		printf("Error parsing com port argument: %s\n", device);
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

	if (baudrate > 0)
	{
		ComConfig(hComm, baudrate);
	}

	if (command == CMD_DUMP)
	{
		ComDump(hComm);
	}
	else if (command == CMD_READ)
	{
		ComRead(hComm);
	}

	CloseHandle(hComm);
	return 0;
}

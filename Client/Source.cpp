#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")

#include <Winsock2.h>
#include <iostream>

#define SERVICE_PORT 1500 

int send_string(SOCKET s, const char* sString)
{
	return send(s, sString, strlen(sString), 0);
}

int main(void)
{
	setlocale(0, "");
	SOCKET  S;

	sockaddr_in serv_addr;
	WSADATA wsadata;

	// Инициализируем библиотеку сокетов
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	// Создаем сокет
	// Для TCP-сокета указываем параметр SOCK_STREAM
	// Для UDP - SOCK_DGRAM 
	if ((S = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		fprintf(stderr, "Can't create socket\n");
		exit(1);
	}

	// Заполняем структуру адресов 
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons((u_short)SERVICE_PORT);


	if (SOCKET_ERROR == (connect(S, (sockaddr*)&serv_addr, sizeof(serv_addr))))
	{
		fprintf(stderr, "Can't connect\n");
		exit(1);
	}

	char    sReceiveBuffer[1024] = { 0 };
	char    sSendBuffer[1024] = { 0 };

	while (true)
	{
		int nReaded;
		do
		{
			nReaded = recv(S, sReceiveBuffer, sizeof(sReceiveBuffer) - 1, 0);

			// В случае ошибки (например, отсоединения клиента) выходим
			if (nReaded <= 0) break;
			// Мы получаем поток байт, поэтому нужно самостоятельно 
			// добавить завержающий 0 для ASCII строки 
			sReceiveBuffer[nReaded] = 0;
			// Пропускаем пустые строки
			if (sReceiveBuffer[0] == 0) continue;

			printf("%s", sReceiveBuffer);
		} while (sReceiveBuffer[nReaded - 1] != '\n');

		if (strcmp(sSendBuffer, "exit") == 0 || (strcmp(sSendBuffer, "shutdown") == 0))
			break;

		std::cin >> sSendBuffer;

		send_string(S, sSendBuffer);


	}

	// Закрываем сокет
	closesocket(S);
	// освобождаем ресурсы библиотеки сокетов
	WSACleanup();
	system("pause");
}
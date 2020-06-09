#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")

#include <Winsock2.h>
#include <iostream>
#include <ctime>
#include <fstream>
#include <list>
#include <string>
#include <sstream>
#include <iterator>

// TCP-порт сервера
#define SERVICE_PORT 1500 

int send_string(SOCKET s, const char* sString);

int main()
{
	setlocale(0, "");
	SOCKET  S;  //дескриптор прослушивающего сокета
	SOCKET  NS; //дескриптор присоединенного сокета

	sockaddr_in serv_addr;
	WSADATA     wsadata;
	char        sName[128];
	bool        bTerminate = false;

	// Инициализируем библиотеку сокетов
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	// Пытаемся получить имя текущей машины
	gethostname(sName, sizeof(sName));
	printf("\nServer host: %s\n", sName);

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
	// Разрешаем работу на всех доступных сетевых интерфейсах,
	// в частности на localhost
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	// обратите внимание на преобразование порядка байт
	serv_addr.sin_port = htons((u_short)SERVICE_PORT);

	// Связываем сокет с заданным сетевым интерфесом и портом
	if (bind(S, (sockaddr*)&serv_addr, sizeof(serv_addr)) == INVALID_SOCKET)
	{
		fprintf(stderr, "Can't bind\n");
		exit(1);
	}

	// Переводим сокет в режим прослушивания заданного порта
	// с максимальным количеством ожидания запросов на соединение 5
	if (listen(S, 5) == INVALID_SOCKET)
	{
		fprintf(stderr, "Can't listen\n");
		exit(1);
	}

	printf("Server listen on %s:%d\n",
		inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));

	// Основной цикл обработки подключения клиентов 
	while (!bTerminate)
	{
		printf("Wait for connections.....\n");

		sockaddr_in clnt_addr;
		int addrlen = sizeof(clnt_addr);
		memset(&clnt_addr, 0, sizeof(clnt_addr));

		// Переводим сервис в режим ожидания запроса на соединение.
		// Вызов синхронный, т.е. возвращает управление только при 
		// подключении клиента или ошибке 
		NS = accept(S, (sockaddr*)&clnt_addr, &addrlen);
		if (NS == INVALID_SOCKET)
		{
			fprintf(stderr, "Can't accept connection\n");
			break;
		}
		// Получаем параметры присоединенного сокета NS и
		// информацию о клиенте
		addrlen = sizeof(serv_addr);
		getsockname(NS, (sockaddr*)&serv_addr, &addrlen);
		// Функция inet_ntoa возвращает указатель на глобальный буффер, 
		// поэтому использовать ее в одном вызове printf не получится
		printf("Accepted connection on %s:%d ",
			inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
		printf("from client %s:%d\n",
			inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

		// Отсылаем вводную информацию о сервере
		send_string(NS, "* * * Welcome to simple UPCASE TCP-server * * *\r\n");
		//
		char    sReceiveBuffer[1024] = { 0 };
		// Получаем и обрабатываем данные от клиента
		while (true)
		{
			int nReaded = recv(NS, sReceiveBuffer, sizeof(sReceiveBuffer) - 1, 0);
			// В случае ошибки (например, отсоединения клиента) выходим
			if (nReaded <= 0) break;
			// Мы получаем поток байт, поэтому нужно самостоятельно 
			// добавить завержающий 0 для ASCII строки 
			sReceiveBuffer[nReaded] = 0;

			// Отбрасываем символы превода строк
			for (char* pPtr = sReceiveBuffer; *pPtr != 0; pPtr++)
			{
				if (*pPtr == '\n' || *pPtr == '\r')
				{
					*pPtr = 0;
					break;
				}
			}
			// Пропускаем пустые строки
			if (sReceiveBuffer[0] == 0) continue;

			printf("Received data: %s\n", sReceiveBuffer);

			// Анализируем полученные команды или преобразуем текст в верхний регистр
			if (strcmp(sReceiveBuffer, "Info") == 0)
			{
				send_string(NS, "Test TCP-server.\nDeveloper: Hurrinecane\r\n");
			}
			else if (strcmp(sReceiveBuffer, "Task") == 0)
			{
				send_string(NS, "Task: \"Добавить в сервис поддержку дополнительной команды, для игры в города.\nКлиент отправляет на сервер некоторое символьное имя.\nСервер ищет в файле город, который начинается на букву, которой заканчивается город клиента.\nЕсли в файле информация не найдена, клиенту возвращается соответствующее сообщение.\"\r\n");
			}
			else if (strcmp(sReceiveBuffer, "Exit") == 0)
			{
				send_string(NS, "Bye...\r\n");
				printf("Client initialize disconnection.\r\n");
				break;
			}
			else if (strcmp(sReceiveBuffer, "Time") == 0)
			{
				time_t now = time(0);
				char* dt = ctime(&now);
				send_string(NS, "The current date and time: ");
				send_string(NS, dt);
			}
			else if (strcmp(sReceiveBuffer, "Shutdown") == 0)
			{
				send_string(NS, "Server go to shutdown.\r\n");
				Sleep(200);
				bTerminate = true;
				break;
			}
			else if (strcmp(sReceiveBuffer, "Cities") == 0)
			{

				std::ifstream file;

				std::string str;

				std::string cityList[1000];

				file.open("Города.txt");
				if (!file)
				{
					std::cout << "Файл не открыт\n";
					send_string(NS, "Игра в города недоступна...\n");
				}
				else
				{
					int cityCounter;
					for (cityCounter = 0; std::getline(file, str); cityCounter++)
						cityList[cityCounter] = str;

					file.close();

					send_string(NS, "Начинается игра в города. Для остановки введите \"Stop\"\nВведите название города:\n");

					do
					{
						int nReaded = recv(NS, sReceiveBuffer, sizeof(sReceiveBuffer) - 1, 0);

						if (nReaded <= 0) break;

						sReceiveBuffer[nReaded] = 0;

						if (sReceiveBuffer[0] == 0) continue;

						printf("Received data: %s\n", sReceiveBuffer);

						if (strcmp(sReceiveBuffer, "Stop") == 0)
						{
							send_string(NS, "Игра в города прервана.\n");
							break;
						}

						char* find = _strupr(&sReceiveBuffer[nReaded - 1]);

						printf("last letter: %s\n", find);

						bool cheсk = false;
						char sSendBuffer[1024]{};

						for (int i = 0; i < cityCounter; i++)
							if (cityList[i][0] == *find)
							{
								cheсk = true;
								for (int j = 0; j < cityList[i].length(); j++)
									sSendBuffer[j] = cityList[i][j];
								cityList[i].clear();
								break;
							}
						if (!cheсk)
						{
							_snprintf(sSendBuffer, sizeof(sSendBuffer), "Города закончились. Вы выйграли!\nКонец игры.\r\n");
							send_string(NS, sSendBuffer);
							break;
						}
						_snprintf(sSendBuffer, sizeof(sSendBuffer), "%s\r\n", sSendBuffer);
						send_string(NS, sSendBuffer);
					} while (true);
				}
			}
			else
			{
				// Преобразовываем строку в верхний регистр
				char sSendBuffer[1024];
				_snprintf(sSendBuffer, sizeof(sSendBuffer), "Server reply: %s\r\n", _strupr(sReceiveBuffer));
				send_string(NS, sSendBuffer);
			}
		}
		// закрываем присоединенный сокет
		closesocket(NS);
		printf("Client disconnected.\n");
	}
	// Закрываем серверный сокет
	closesocket(S);
	// освобождаем ресурсы библиотеки сокетов
	WSACleanup();
	return 0;
}

// Функция отсылки текстовой ascii строки клиенту
int send_string(SOCKET s, const char* sString)
{
	return send(s, sString, strlen(sString), 0);
}
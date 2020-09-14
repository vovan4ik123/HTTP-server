#include <iostream>
#include <sstream>
#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
#elif __linux__
    #include <sys/socket.h>
#endif

// Чтоб линковалась DLL библиотека для работы с сокетом
#pragma comment(lib, "Ws2_32.lib")

enum class ResponceCodes
{
    _100, _200, _300, _400, _500
};

ResponceCodes handleRequest(const char* buf, int bufSize, std::stringstream& answerBody);
ResponceCodes handleGet(std::stringstream& answerBody);
ResponceCodes handlePut(std::stringstream& answerBody);
ResponceCodes handlePost(std::stringstream& answerBody);
ResponceCodes handleDelete(std::stringstream& answerBody);
void sendAnswer(ResponceCodes code, int clientSocket, std::stringstream& answerBody);

int main()
{
    WSADATA wsaData;

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0)
    {
        std::cout << "WSAStartup() error" << std::endl;
        return result;
    }

    // структура слушающего сокета с инфо о IP адресе
    addrinfo* addr = nullptr;
    
    // создаем и заполняем шаблон для инициализации структуры
    // "структура слушающего сокета с инфо о IP адресе"
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // инициализировать структура с помощью шаблона
    result = getaddrinfo("127.0.0.1", "80", &hints, &addr);
    if (result != 0)
    {
        std::cout << "getaddrinfo() error" << std::endl;
        WSACleanup();
        return result;
    }

    // ask OS for provide socket to us
    int listenSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cout << "socket() error" << std::endl;
        freeaddrinfo(addr);
        WSACleanup();
        return result;
    }

    result = bind(listenSocket, addr->ai_addr, addr->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        std::cout << "bind() error" << std::endl;
        closesocket(listenSocket);
        freeaddrinfo(addr);
        WSACleanup();
        return result;
    }

    result = listen(listenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR)
    {
        std::cout << "listen() error" << std::endl;
        closesocket(listenSocket);
        freeaddrinfo(addr);
        WSACleanup();
        return result;
    }

    int clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cout << "accept() error" << std::endl;
        closesocket(listenSocket);
        freeaddrinfo(addr);
        WSACleanup();
        return result;
    }

    const int maxRequestLength = 1024;
    char requestBuffer[maxRequestLength];
    result = recv(clientSocket, requestBuffer, maxRequestLength, 0); // result will store nubmer of byte read
    if (result == SOCKET_ERROR)
    {
        std::cout << "recv() error" << std::endl;
        closesocket(listenSocket);
        freeaddrinfo(addr);
        WSACleanup();
    }
    else if (result == 0)
    {
        std::cout << "Connection was closed by client" << std::endl;
    }
    else if (result > 0)
    {
        requestBuffer[result] = '\0';
        std::cout << requestBuffer << std::endl;

        std::stringstream answerBody;
        ResponceCodes respCode = handleRequest(requestBuffer, result, answerBody);

        sendAnswer(respCode, clientSocket, answerBody);
    }


    closesocket(listenSocket);
    freeaddrinfo(addr);
    WSACleanup();

    system("PAUSE");
}

ResponceCodes handleRequest(const char* buf, int bufSize, std::stringstream& answerBody)
{
    const int methodNameLength = 6;
    std::string methodName(buf, methodNameLength);

    if (methodName.find("POST") != std::string::npos)
    {
        // POST ask change in state or side effects on the server.
        return handlePost(answerBody);
    }
    else if (methodName.find("PUT") != std::string::npos)
    {
        // PUT ask replace all current representations of the target resource.
        return handlePut(answerBody);
    }
    else if (methodName.find("DELETE") != std::string::npos)
    {
        // DELETE ask delete the specified resource.
        return handleDelete(answerBody);
    }
    else if (methodName.find("GET") != std::string::npos)
    {
        // GET ask for representation of the specified resource. Only retrieve data.
        return handleGet(answerBody);
    }

    return ResponceCodes::_500;
}

ResponceCodes handleGet(std::stringstream& answerBody)
{
    answerBody  << "<html>"

                << "<head><title>done</title></head>"

                << "<body>"
                << "<font color=\"green\" face=\"arial\" size=\"6\">"
                << "You sent GET request to fetch data from server but server has not data to return =)"
                << "</font>"
                << "</body>"

                << "</html>";

    return ResponceCodes::_100;
}

ResponceCodes handlePut(std::stringstream& answerBody)
{
    answerBody << "<html>"

        << "<head><title>done</title></head>"

        << "<body>"
        << "<font color=\"green\" face=\"arial\" size=\"6\">"
        << "You sent PUT request to replace data on server but there nothing to replace =)"
        << "</font>"
        << "</body>"

        << "</html>";

    return ResponceCodes::_200;
}

ResponceCodes handlePost(std::stringstream& answerBody)
{
    answerBody << "<html>"

        << "<head><title>done</title></head>"

        << "<body>"
        << "<font color=\"green\" face=\"arial\" size=\"6\">"
        << "You sent POST request to change data on server but server has not data =)"
        << "</font>"
        << "</body>"

        << "</html>";

    return ResponceCodes::_200;
}

ResponceCodes handleDelete(std::stringstream& answerBody)
{
    answerBody << "<html>"

        << "<head><title>done</title></head>"

        << "<body>"
        << "<font color=\"green\" face=\"arial\" size=\"6\">"
        << "You sent DELETE request to delete data on server but server has not data =)"
        << "</font>"
        << "</body>"

        << "</html>";

    return ResponceCodes::_200;
}

void sendAnswer(ResponceCodes code, int clientSocket, std::stringstream& answerBody)
{
    std::stringstream totalAnswer;

    if (code == ResponceCodes::_100)
    {
        totalAnswer << "HTTP/1.1 100 Continue\r\n"
            << "Version: HTTP/1.1\r\n"
            << "Server: random example from github\r\n"
            << "Content-Type: text/html; charset=utf-8\r\n"
            << "Content-Length: " << 12
            << "\r\n\r\n"
            << "100 Continue";
    }
    else if (code == ResponceCodes::_200)
    {
        totalAnswer << "HTTP/1.1 200 OK\r\n"
                    << "Version: HTTP/1.1\r\n"
                    << "Server: random example from github\r\n"
                    << "Content-Type: text/html; charset=utf-8\r\n"
                    << "Content-Length: " << answerBody.str().length()
                    << "\r\n\r\n"
                    << answerBody.str();
    }
    else if (code == ResponceCodes::_300)
    {
        totalAnswer << "HTTP/1.1 300 Multiple choises\r\n"
            << "Version: HTTP/1.1\r\n"
            << "Server: random example from github\r\n"
            << "Content-Type: text/html; charset=utf-8\r\n"
            << "Content-Length: " << 20
            << "\r\n\r\n"
            << "300 Multiple choises";
    }
    else if (code == ResponceCodes::_400)
    {
        totalAnswer << "HTTP/1.1 400 Bad request\r\n"
            << "Version: HTTP/1.1\r\n"
            << "Server: random example from github\r\n"
            << "Content-Type: text/html; charset=utf-8\r\n"
            << "Content-Length: " << 15
            << "\r\n\r\n"
            << "400 Bad request";
    }
    else if (code == ResponceCodes::_500)
    {
        totalAnswer << "HTTP/1.1 500 Server error\r\n"
            << "Version: HTTP/1.1\r\n"
            << "Server: random example from github\r\n"
            << "Content-Type: text/html; charset=utf-8\r\n"
            << "Content-Length: " << 16
            << "\r\n\r\n"
            << "500 Server error";
    }

    send(clientSocket, totalAnswer.str().c_str(), totalAnswer.str().length(), 0);
}
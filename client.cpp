//客户端
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<winsock2.h>
#pragma comment(lib,"Ws2_32.lib")
//#include<string.h>

int main()
{
	WORD wdVersion = MAKEWORD(2, 2);	//使用网络库的版本
	WSADATA wdSockMsg;					//系统通过这个参数给我们一些配置信息
	int nRes = WSAStartup(wdVersion, &wdSockMsg);

	if (0 != nRes)
	{
		switch (nRes)
		{
		case WSASYSNOTREADY:
			printf("可以重启电脑，或检查网络库");
			break;
		case WSAVERNOTSUPPORTED:
			printf("请更新网络库");
			break;
		case WSAEINPROGRESS:
			printf("Please reboot this software");
			break;
		case WSAEPROCLIM:
			printf("请关闭不必要的软件，以为当前网络提供充足资源");
			break;
		case WSAEFAULT:
			printf("参数错误");
			break;
		}
		return 0;
	}
	//版本校验
	if (2 != HIBYTE(wdSockMsg.wVersion) || 2 != LOBYTE(wdSockMsg.wVersion))
	{
		//版本打开错误
		WSACleanup();   //关闭网络库
		return 0;
	}

	//服务器的socket
	SOCKET socketSever = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//这三个参数分别为地址类型（IPV4），套接字类型和协议类型（TCP)
	//如果执行失败则返回INVALID_SOCKET
	if (INVALID_SOCKET == socketSever)
	{
		int a = WSAGetLastError();				//如果socket调用失败，返回错误码（工具 -> 错误查找）
		WSACleanup();							//关闭网络库
		return 0;
	}

	struct sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_port = htons(12345);
	si.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int connect_a = connect(socketSever, (const struct sockaddr*)&si, sizeof(si));
	if (SOCKET_ERROR == connect_a)
	{
		int a = WSAGetLastError();				//如果socket调用失败，返回错误码（工具 -> 错误查找）
		closesocket(socketSever);				//关闭socket
		WSACleanup();							//关闭网络库
		return 0;
	}
	send(socketSever, "连接成功", strlen("连接成功"), 0);//如果连接成功，向服务端发送“连接成功”

	while (1)
	{
		/*char buf[1500] = { 0 };
		int res = recv(socketSever, buf, 1499, 0);
		if (0 == res)
		{
			printf("连接中断，客户端下线\n");
		}
		else if (SOCKET_ERROR == res)
		{
			printf("错误码:%d\n", WSAGetLastError());
		}
		else
		{
			printf("%d,%s\n", res, buf);
		}*/

		//发送函数
		char buf[1500] = { 0 };
		scanf("%s", buf);
		if ('0' == buf[0])//输入0时，退出循环，客户端下线
		{
			break;
		}
		int send_a = send(socketSever, buf, strlen(buf), 0);
		if (SOCKET_ERROR == send_a)
		{
			//出现错误
			int a = WSAGetLastError();
		}
	}
	//关闭socket
	closesocket(socketSever);
	//清理网络库
	WSACleanup();
	return 0;
}

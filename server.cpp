//服务端
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include<stdio.h>
#include<string.h>
#include<stdbool.h>

fd_set all_Sockets;

BOOL WINAPI over(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
		//释放所有socket
		for (u_int i = 0; i < all_Sockets.fd_count; i++)
		{
			closesocket(all_Sockets.fd_array[i]);
		}
		//清理网络库
		WSACleanup();
	}
	return 0;
}

int main()
{
	SetConsoleCtrlHandler(over, TRUE);//这个函数的作用是当点击运行框右上角叉号关闭时，执行上面的over函数

	WORD wdVersion = MAKEWORD(2, 2);//使用网络库的版本

	WSADATA wdSockMsg;				//系统通过这个参数给我们一些配置信息
	int nRes = WSAStartup(wdVersion, &wdSockMsg);//打开/启动网络库，只有启动了库，这个库里的函数才能使用

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
	}
	//版本校验
	if (2 != HIBYTE(wdSockMsg.wVersion) || 2 != LOBYTE(wdSockMsg.wVersion))
	{
		//版本打开错误
		WSACleanup();		//关闭网络库
		return 0;
	}
	SOCKET socketSever = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//socket函数三个参数分别为地址类型（IPV4），套接字类型（）和协议类型（TCP） 
	//如果执行失败则返回INVALID_SOCKET

	if (INVALID_SOCKET == socketSever)
	{
		int a = WSAGetLastError();	//如果socket调用失败，返回错误码（工具 -> 错误查找 可以查询具体错误）
		WSACleanup();		//关闭网络库
		return 0;
	}
	struct sockaddr_in si;
	si.sin_family = AF_INET;							//地址类型
	si.sin_port = htons(12345);							//端口号
	si.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");	//IP地址

	int bres = bind(socketSever, (const struct sockaddr*)&si, sizeof(si));
	/*参数1：前面创建的socket
	  参数2：是一个结构体sockaddr(包含地址类型、端口号和IP地址)地址，官方给出结构体sockaddr不方便赋值，因此我们定义sockaddr_in
			分别赋值地址类型、端口号和IP地址后，强制类型转换为sockaddr
	  参数3：参数2类型的大小          */
	if (SOCKET_ERROR == bres)
	{
		//bind函数出错
		int a = WSAGetLastError();		//返回错误码
		closesocket(socketSever);		//关闭socket
		WSACleanup();					//关闭网络库
		return 0;
	}
	//开始监听
	int a = listen(socketSever, SOMAXCONN);
	
	printf("开始监听");
	if (SOCKET_ERROR == a)
	{
		//listen 函数出错
		int a = WSAGetLastError();		//返回错误码
		closesocket(socketSever);		//关闭socket
		WSACleanup();					//关闭网络库
		return 0;
	}

	FD_ZERO(&all_Sockets);	//清零
	FD_SET(socketSever, &all_Sockets);//添加服务器socket

	while (1)
	{
		fd_set readSockets = all_Sockets;
		fd_set writeSockets = all_Sockets;
		fd_set errorSockets = all_Sockets;
		//时间段
		struct timeval timeval_a;//给参数5赋值等待时间
		timeval_a.tv_sec = 3;
		timeval_a.tv_usec = 0;
		int select_a = select(0, &readSockets, &writeSockets, &errorSockets, &timeval_a);
		//第二个参数测试recv和accept，第三个参数测试send，第四个参数测试错误
		if (0 == select_a)
		{
			//没有响应
			continue;
		}
		else if (select_a > 0)//有响应
		{
			//遍历参数4，查看select函数是否有错误返回
			for (u_int i = 0; i < errorSockets.fd_count; i++)
			{
				char str[100] = { 0 };
				int len = 99;
				if (SOCKET_ERROR == getsockopt(errorSockets.fd_array[i], SOL_SOCKET, SO_ERROR, str, &len))//调用getsockopt函数获取错误信息
					//参数1：我们要操作的socket，参数2：socket上的情况，参数4：代表一段空间，返回的错误信息装在里面，参数5：参数4的长度
				{
					printf("无法得到错误信息\n");
				}
				printf("%s\n", str);
			}
			//遍历参数3，寻找找出可以给哪些客户端socket发消息
			for (u_int i = 0; i < writeSockets.fd_count; i++)
			{
				//printf("服务器：%d，%d可写\n", socketSever, writeSockets.fd_array[i]);
				if (SOCKET_ERROR == send(writeSockets.fd_array[i], "OK", 2, 0))
				{
					int a = WSAGetLastError();
				}
			}
			for (u_int i = 0; i < readSockets.fd_count; i++)
			{
				//遍历参数2中（有响应）的socket，在这里响应的socket只可能是服务器socket和客户端socket两种可能
				if (readSockets.fd_array[i] == socketSever)//如果有响应的socket是服务器socket，则是客户端请求连接，需要调用accept函数
				{
					//accept
					SOCKET socketClient = accept(socketSever, NULL, NULL);
					if (INVALID_SOCKET == socketClient)
					{
						continue;
					}
					FD_SET(socketClient, &all_Sockets); // 将刚返回的socket添加到socket数组中

				}
				else      //如果是客户端socket则需要接收消息
				{
					char buf[1500] = { 0 };
					int recv_a = recv(readSockets.fd_array[i], buf, 1500, 0);
					if (0 == recv_a)
					{
						printf("客户端下线\n");
						SOCKET socket_temp = readSockets.fd_array[i];
						//从集合中拿掉
						FD_CLR(readSockets.fd_array[i], &all_Sockets);
						//释放
						closesocket(socket_temp);
					}
					else if (0 < recv_a)
					{
						//接收成功
						printf("%s\n", buf);
					}
					else
					{
						//recv函数出错
						int a = WSAGetLastError();
					}
				}
			}
		}
		else
		{
			printf("错误码2：%d\n", WSAGetLastError());
		}
	}
	//释放所有socket
	for (u_int i = 0; i < all_Sockets.fd_count; i++)
	{
		closesocket(all_Sockets.fd_array[i]);
	}
	WSACleanup();				//关闭网络库
	return 0;
}

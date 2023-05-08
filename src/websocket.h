#pragma once

#include <stdint.h>
#include <map>
#include "libwebsockets.h"
#include "common-def.h"

struct SocketfdManager
{
	bool Exist(lws_sockfd_type fd)
	{
		for (int i = 0; i < SocketfdArrayIndex; i++)
		{
			if (SocketfdArray[i].fd == fd)
			{
				return true;
			}
		}

		return false;
	}

	void Add(lws_pollargs* args)
	{
		SocketfdArray[SocketfdArrayIndex++] = *args;
	}

	void Remove(lws_sockfd_type fd)
	{
		for (int i = 0; i < SocketfdArrayIndex; i++)
		{
			if (SocketfdArray[i].fd == fd)
			{
				memcpy(&SocketfdArray[i], &SocketfdArray[i + 1], sizeof(lws_pollargs) * (SocketfdArrayIndex - i - 1));
				SocketfdArrayIndex--;
				break;
			}
		}
	}

	int GetNum() { return SocketfdArrayIndex; }

	const int GetMaxIndex() { return SocketfdArrayIndex - 1; }

	lws_pollargs* GetMaxArgs()
	{
		if (GetNum() > 0)
		{
			return &SocketfdArray[SocketfdArrayIndex - 1];
		}
		
		return nullptr;
	}

	lws_pollargs* GetArgsByFd(lws_sockfd_type fd)
	{
		for (int i = 0; i < SocketfdArrayIndex; i++)
		{
			if (SocketfdArray[i].fd == fd)
			{
				return &SocketfdArray[i];
			}
		}

		return nullptr;
	}

	int SocketfdArrayIndex = 0;
	lws_pollargs SocketfdArray[10];
};

class WebSocketConnection
{
public:
	static int StaticLwsCallback(lws* Connection, lws_callback_reasons Reason, void* UserData, void* Data, size_t Length);

	static void LwsLog(int Level, const char* LogLine);

	static WebSocketConnection* WSConn;
public:
	WebSocketConnection();
	~WebSocketConnection();

public:
	void Connect(char* ConnectionUrl);

	void Run();

	bool IsRunning() { return bRunningFlag; }

	int LwsCallback(lws* Connection, lws_callback_reasons Reason, void* UserData, void* Data, size_t Length);

	void WriteBuffer(uint8* Buffer, int Length);

	String GetCertificateBuffer();

	void SetXConnectionID(String NewID) { XConnectionID = NewID; }
	String GetXConnectionID() { return XConnectionID; }
private:
	SSL_CTX* CreateSSLContext();
private:
	lws_context* LwsContext;

	lws* LwsConnection;

	bool bRunningFlag = false;

	SocketfdManager SocketfdMrg;

	String XConnectionID;

	int SendParamFlag = 0;

	char* pSoundFile;
	int nSoundFileLength;

	// SSL
	String m_strCaFilePath;
	String m_strCertFilePath;
	String m_strPrivateKeyFilePath;
};
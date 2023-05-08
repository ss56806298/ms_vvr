#pragma warning(disable:4244 4996 4267)

//#include "csv.h"
#include "lws_config.h"

#include "websocket.h"
#include "ssml-creator.h"
#include "utilities.h"
#include "log.h"
#include "ms-vvr.h"
#include "common-def.h"

#include <iostream>
#include <fstream>


String PathMgr::m_strProjectPath = "";
String PathMgr::m_strOutputPath = "";

int GenerateAudio(void* pData, int nLen)
{
	if (!LogModule::GetInstance()->StartModule()) return 1;
	vvr_log("Write Test1.");
	if (SSMLCreatorModule::GetInstance()->GenerateSSML(pData, nLen)) return 1;

	std::string UUID = uuid::generate_uuid_v4();

	vvr_log("Write Test4.");

	// Create websocket connection.
	WebSocketConnection* WSConn = new WebSocketConnection;

	const char* UuidPtr = UUID.c_str();
	for (int i = 0; i < UUID.size(); i++)
	{
		if (UUID[i] == '-')
		{
			UUID.replace(i, 1, "");
			i--;
		}
		else
		{
			UUID[i] = toupper(UUID[i]);
		}
	}

	char ConnectionUrl[1000];

	std::sprintf(&ConnectionUrl[0], "wss://eastus.api.speech.microsoft.com/cognitiveservices/websocket/v1?TrafficType=AzureDemo&Authorization=bearer%%20undefined&X-ConnectionId=%s", UUID.c_str());
	//std::sprintf(&ConnectionUrl[0], "wss://127.0.0.1:12888");
	WSConn->SetXConnectionID(UUID);
	WSConn->Connect(&ConnectionUrl[0]);

	// Release.
	delete WSConn;

	LogModule::GetInstance()->EndModule(); // If you want log module work normally, should end log module last.

	return 0;
}

int SetProjectPath(char* pNewPath)
{
	PathMgr::m_strProjectPath = String(pNewPath);
	return 0;
}

int SetOutputPath(char* pNewPath)
{
	PathMgr::m_strOutputPath = String(pNewPath);
	return 0;
}
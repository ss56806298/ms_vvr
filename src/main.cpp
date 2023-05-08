// AIVoice.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#pragma warning(disable:4244 4996 4267)

#include "lws_config.h"

#include "websocket.h"
#include "ssml-creator.h"
#include "log.h"
#include "utilities.h"
#include "ms-vvr.h"

#include <random>
#include <sstream>
#include <iostream>
#include <fstream>

String PathMgr::m_strProjectPath = "";

int main()
{
    if (!LogModule::GetInstance()->StartModule()) return 0;
    if (!SSMLCreatorModule::GetInstance()->StartModule()) return 0;

    // -----------------------------Test begin.-----------------------------
 //   char Data[5000];
 //   int CurrentSize = 0;

 //   // First Audio.
 //   char chAudioContent[] = "I just want to test a voice. Tell me what to say and how to say.";

 //   AudioHeader* pAudioHeader = (AudioHeader*)&Data[CurrentSize];
 //   pAudioHeader->eVoiceType = EVoiceType::LittleGirl;
 //   pAudioHeader->nRate = 0;
 //   pAudioHeader->nPitch = 0;
 //   pAudioHeader->nContourNum = 0;
 //   pAudioHeader->nLeadingSilenceValue = 0;
 //   pAudioHeader->nSentenceBoundarySilenceValue = 0;
 //   pAudioHeader->nTailingSilenceValue = 0;
 //   pAudioHeader->eExpressStyle = EExpressStyle::chat;
 //   pAudioHeader->nExpressStyleDegree = 0;
 //   pAudioHeader->eExpressStyleRole = EExpressStyleRole::None;
 //   pAudioHeader->nContentLength = sizeof(chAudioContent);
 //   CurrentSize += sizeof(AudioHeader);

 //   memcpy(&Data[CurrentSize], &chAudioContent[0], sizeof(chAudioContent));
 //   CurrentSize += pAudioHeader->nContentLength;

	//// Second Audio.
	//char chAudioContent2[] = "Where is my pencil? I will be late for school.";

	//AudioHeader* pAudioHeader2 = (AudioHeader*)&Data[CurrentSize];
 //   pAudioHeader2->eVoiceType = EVoiceType::YoungMale;
 //   pAudioHeader2->nRate = 0;
 //   pAudioHeader2->nPitch = 0;
 //   pAudioHeader2->nContourNum = 2;
 //   pAudioHeader2->chContour[0][0] = 0;
 //   pAudioHeader2->chContour[0][1] = 60;
	//pAudioHeader2->chContour[1][0] = 60;
	//pAudioHeader2->chContour[1][1] = -60;
 //   pAudioHeader2->nLeadingSilenceValue = 1000;
 //   pAudioHeader2->nSentenceBoundarySilenceValue = 1000;
 //   pAudioHeader2->nTailingSilenceValue = 1000;
 //   pAudioHeader2->eExpressStyle = EExpressStyle::sad;
 //   pAudioHeader2->nExpressStyleDegree = 200;
 //   pAudioHeader2->eExpressStyleRole = EExpressStyleRole::Girl;
 //   pAudioHeader2->nContentLength = sizeof(chAudioContent2);
	//CurrentSize += sizeof(AudioHeader);

	//memcpy(&Data[CurrentSize], &chAudioContent2[0], sizeof(chAudioContent));
	//CurrentSize += pAudioHeader2->nContentLength;

 //   SSMLCreatorModule::GetInstance()->GenerateSSML(&Data[0], CurrentSize);
    // -----------------------------Test end.-----------------------------

	std::string UUID = uuid::generate_uuid_v4();

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

    SSMLCreatorModule::GetInstance()->EndModule();
    LogModule::GetInstance()->EndModule(); // If you want log module work normally, should end log module last.

    return 0;
}

int GenerateAudio(void* pData, int nLen)
{
    return 0;
}

void SetProjectPath(std::string strNewPath)
{
	PathMgr::m_strProjectPath = strNewPath;
}

#include "ssml-creator.h"

#include "log.h"
#include "utilities.h"
#include "ms-vvr.h"

#include "tinyxml2.h"
#include <fstream>
#include "Windows.h"


using namespace tinyxml2;

bool SSMLCreatorModule::StartModule()
{
#ifndef DLLExport
	std::ifstream inFile;
	inFile.open(INPUT_PATH + "ssml.txt", std::ios::in | std::ios::binary);

	if (!inFile.is_open())
	{
		vvr_log("Open ssml file failed, read failed.");
		return false;
	}
	inFile.seekg(0, inFile.end);
	long long nFileLength = inFile.tellg();
	inFile.seekg(0, inFile.beg);

	m_pSSMLFile = new char[nFileLength + 1];

	inFile.read(m_pSSMLFile, nFileLength);

	m_pSSMLFile[nFileLength] = '\0';

	vvr_log("Read ssml file success, FileLength: %d", nFileLength);
#endif
	return true;
}

void SSMLCreatorModule::EndModule()
{
	if (m_pSSMLFile) delete m_pSSMLFile;
}

bool SSMLCreatorModule::GenerateSSML(void* pData, int nLen)
{
	if (!pData || nLen < 1) return false;

	const char* xmlPath = "D:\\test.xml";

	tinyxml2::XMLDocument doc;

	// Fixed ssml.
	XMLElement* pSpeakElement = doc.NewElement("speak");
	pSpeakElement->SetAttribute("xmlns", "http://www.w3.org/2001/10/synthesis");
	pSpeakElement->SetAttribute("xmlns:mstts", "http://www.w3.org/2001/mstts");
	pSpeakElement->SetAttribute("xmlns:emo", "http://www.w3.org/2009/10/emotionml");
	pSpeakElement->SetAttribute("version", "1.0");
	pSpeakElement->SetAttribute("xml:lang", "en-US");
	doc.InsertEndChild(pSpeakElement);

	int nCurrentLen = 0;
	// Real voice ssml.
	while (nCurrentLen < nLen)
	{
		AudioHeader* pHeader = (AudioHeader*)((char*)pData + nCurrentLen);
		nCurrentLen += sizeof(AudioHeader);

		// Voice
		XMLElement* pVoiceElement = pSpeakElement->InsertNewChildElement("voice");
#define VOICE_NAME_ATTRIBUTE(eType, strName)\
case EVoiceType::eType:\
{\
	pVoiceElement->SetAttribute("name", "zh-CN-"#strName"Neural");\
	break;\
}
		switch (pHeader->eVoiceType)
		{
			VOICE_NAME_ATTRIBUTE(YoungFemale, Xiaoxiao);
			VOICE_NAME_ATTRIBUTE(AdultMale, Yunyang);
			VOICE_NAME_ATTRIBUTE(YoungFemaleBitNeutral, Xiaochen);
			VOICE_NAME_ATTRIBUTE(AdultFemale, Xiaohan);
			VOICE_NAME_ATTRIBUTE(YoungFemale2, Xiaomeng);
			VOICE_NAME_ATTRIBUTE(AdultFemaleBitGental, Xiaomo);
			VOICE_NAME_ATTRIBUTE(AdultFemaleBitOld, Xiaoqiu);
			VOICE_NAME_ATTRIBUTE(AdultFemaleLikeASoldier, Xiaorui);
			VOICE_NAME_ATTRIBUTE(LittleGirl, Xiaoshuang);
			VOICE_NAME_ATTRIBUTE(AdultFemaleBitGental2, Xiaoxuan);
			VOICE_NAME_ATTRIBUTE(YoungFemale3, Xiaoyan);
			VOICE_NAME_ATTRIBUTE(HighSchoolGirl, Xiaoyi);
			VOICE_NAME_ATTRIBUTE(GoodLittleGirl, Xiaoyou);
			VOICE_NAME_ATTRIBUTE(YoungFemaleBitOld, Xiaozhen);
			VOICE_NAME_ATTRIBUTE(AdultMaleBitSerious, Yunfeng);
			VOICE_NAME_ATTRIBUTE(AdultMaleBitGental, Yunhao);
			VOICE_NAME_ATTRIBUTE(AdultMaleWithPower, Yunjian);
			VOICE_NAME_ATTRIBUTE(LittleBoy, Yunxia);
			VOICE_NAME_ATTRIBUTE(YoungMale, Yunxi);
			VOICE_NAME_ATTRIBUTE(AdultMaleLikeANewsReporter, Yunye);
			VOICE_NAME_ATTRIBUTE(AdultMaleBitOld, Yunze);
		}
#undef VOICE_NAME_ATTRIBUTE

		// Prosody.
		XMLElement* pProsodyElement = pVoiceElement->InsertNewChildElement("prosody");
		pProsodyElement->SetAttribute("rate", (std::to_string(pHeader->nRate) + '%').c_str());
		pProsodyElement->SetAttribute("pitch", (std::to_string(pHeader->nPitch) + '%').c_str());
		if (pHeader->nContourNum > 0)
		{
			std::string strContour;
			for (int i = 0; i < pHeader->nContourNum; i++)
			{
				if (pHeader->chContour[i][1] > 0)
				{
					strContour += StringHelper::Printf("(%d%%,+%d%%))", (int)pHeader->chContour[i][0], (int)pHeader->chContour[i][1]);
				}
				else
				{
					strContour += StringHelper::Printf("(%d%%,%d%%))", (int)pHeader->chContour[i][0], (int)pHeader->chContour[i][1]);
				}
			}
			pProsodyElement->SetAttribute("contour", strContour.c_str());
		}

		XMLElement* pCurrentElement = pProsodyElement;
		// Express-as
#define EXPRESS_AS_STYLE(style)\
case EExpressStyle::style:\
{\
	XMLElement* pExpressAsElement = pCurrentElement->InsertNewChildElement("mstts:express-as");\
	pCurrentElement = pExpressAsElement;\
	pCurrentElement->SetAttribute("style", #style);\
	if (pHeader->nExpressStyleDegree > 0)\
	{\
		float fDegree = (float)pHeader->nExpressStyleDegree / 100;\
		pCurrentElement->SetAttribute("styledegree", std::to_string(fDegree).c_str());\
	}\
	break;\
}
#define EXPRESS_AS_STYLE_WITH_NAME(style, name)\
case EExpressStyle::style:\
{\
	XMLElement* pExpressAsElement = pCurrentElement->InsertNewChildElement("mstts:express-as");\
	pCurrentElement = pExpressAsElement;\
	pCurrentElement->SetAttribute("style", #name);\
	if (pHeader->nExpressStyleDegree > 0)\
	{\
		float fDegree = (float)pHeader->nExpressStyleDegree / 100;\
		pCurrentElement->SetAttribute("styledegree", std::to_string(fDegree).c_str());\
	}\
	break;\
}
		bool bNotGeneralFlag = true;
		switch (pHeader->eExpressStyle)
		{
		case EExpressStyle::general:
		{
			bNotGeneralFlag = false;
			break;
		}
		EXPRESS_AS_STYLE(advertisement_upbeat);
		EXPRESS_AS_STYLE(affectionate);
		EXPRESS_AS_STYLE(angry);
		EXPRESS_AS_STYLE(assistant);
		EXPRESS_AS_STYLE(calm);
		EXPRESS_AS_STYLE(chat);
		EXPRESS_AS_STYLE(cheerful);
		EXPRESS_AS_STYLE(customerservice);
		EXPRESS_AS_STYLE(depressed);
		EXPRESS_AS_STYLE(disgruntled);
		EXPRESS_AS_STYLE_WITH_NAME(documentary_narration, documentary-narration);
		EXPRESS_AS_STYLE(embarrassed);
		EXPRESS_AS_STYLE(empathetic);
		EXPRESS_AS_STYLE(envious);
		EXPRESS_AS_STYLE(excited);
		EXPRESS_AS_STYLE(fearful);
		EXPRESS_AS_STYLE(friendly);
		EXPRESS_AS_STYLE(gentle);
		EXPRESS_AS_STYLE(hopeful);
		EXPRESS_AS_STYLE(lyrical);
		EXPRESS_AS_STYLE_WITH_NAME(narration_professional, narration-professional);
		EXPRESS_AS_STYLE_WITH_NAME(narration_relaxed, narration-relaxed);
		EXPRESS_AS_STYLE(newscast);
		EXPRESS_AS_STYLE_WITH_NAME(newscast_casual, newscast-casual);
		EXPRESS_AS_STYLE_WITH_NAME(newscast_formal, newscast-formal);
		EXPRESS_AS_STYLE_WITH_NAME(poetry_reading, poetry-reading);
		EXPRESS_AS_STYLE(sad);
		EXPRESS_AS_STYLE(serious);
		EXPRESS_AS_STYLE(shouting);
		EXPRESS_AS_STYLE(sports_commentary);
		EXPRESS_AS_STYLE(sports_commentary_excited);
		EXPRESS_AS_STYLE(whispering);
		EXPRESS_AS_STYLE(terrified);
		EXPRESS_AS_STYLE(unfriendly);
		}
#undef EXPRESS_AS_STYLE
#undef EXPRESS_AS_STYLE_WITH_NAME
#define EXPRESS_STYLE_ROLE(role)\
case EExpressStyleRole::role:\
{\
	if (!bNotGeneralFlag)\
	{\
		XMLElement* pExpressAsElement = pCurrentElement->InsertNewChildElement("mstts:express-as");\
		pCurrentElement = pExpressAsElement;\
	}\
	pCurrentElement->SetAttribute("role", #role);\
	break;\
}
		switch (pHeader->eExpressStyleRole)
		{
		case EExpressStyleRole::None:
		{
			break;
		}
		EXPRESS_STYLE_ROLE(Girl);
		EXPRESS_STYLE_ROLE(Boy);
		EXPRESS_STYLE_ROLE(YoungAdultFemale);
		EXPRESS_STYLE_ROLE(YoungAdultMale);
		EXPRESS_STYLE_ROLE(OlderAdultFemale);
		EXPRESS_STYLE_ROLE(OlderAdultMale);
		EXPRESS_STYLE_ROLE(SeniorFemale);
		EXPRESS_STYLE_ROLE(SeniorMale);
		}
#undef EXPRESS_STYLE_ROLE

		// Silence.
#define SILENCE_TYPE_VALUE(type, value)\
	XMLElement* pElement = pCurrentElement->InsertNewChildElement("mstts:silence");\
	pElement->SetAttribute("type", #type);\
	pElement->SetAttribute("value", (std::to_string(value) + "ms").c_str());

		if (pHeader->nLeadingSilenceValue > 0)
		{
			SILENCE_TYPE_VALUE(Leading-exact, pHeader->nLeadingSilenceValue);
		}
		if (pHeader->nSentenceBoundarySilenceValue > 0)
		{
			SILENCE_TYPE_VALUE(Sentenceboundary-exact, pHeader->nSentenceBoundarySilenceValue);
		}
		if (pHeader->nTailingSilenceValue > 0)
		{
			SILENCE_TYPE_VALUE(Tailing-exact, pHeader->nTailingSilenceValue);
		}
#undef SILENCE_TYPE_VALUE
	
		// Utf8 format content.
		pCurrentElement->InsertNewText((char*)pData + nCurrentLen);
		nCurrentLen += pHeader->nContentLength;
	}
	vvr_log("Write Test2.");

	time_t currentTime = std::time(0);
	tm date;
	localtime_s(&date, &currentTime);
	char chFileName[LOG_FILE_NAME_MAX_LENGTH];
	int nFileNameLen = sprintf_s(chFileName, "%d.%d.%d-%d.%d.%d.ssml", date.tm_year + 1900, date.tm_mon + 1, date.tm_mday, date.tm_hour, date.tm_min, date.tm_sec);
	chFileName[nFileNameLen] = '\0';

	// Save ssml to file.
	FILE* SSMLFile;
	fopen_s(&SSMLFile, (OUTPUT_PATH + chFileName).c_str(), "w+");
	doc.SaveFile(SSMLFile);

	// Get file size.
	fseek(SSMLFile, 0, SEEK_END);
	int64 fileSize = ftell(SSMLFile);
	fseek(SSMLFile, 0, SEEK_SET);

	vvr_log("Write Test3.");

	// Get file content.
	if (m_pSSMLFile) delete m_pSSMLFile;
	m_pSSMLFile = new char[fileSize + 1];
	fread_s(m_pSSMLFile, fileSize, fileSize, 1, SSMLFile);
	vvr_log("Write Test3.1.");
	for (int64 i = fileSize; i > 1; i--)
	{
		if (m_pSSMLFile[i - 1] == '\n')
		{
			m_pSSMLFile[i] = '\0';
			break;
		}
	}
	vvr_log("Write Test3.2.");
	fclose(SSMLFile);

	return 0;
}
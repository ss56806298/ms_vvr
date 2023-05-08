
#pragma once

#ifdef DLLExport
#define MS_VVR_REQ_API __declspec(dllexport)
#else
#define MS_VVR_REQ_API __declspec(dllimport)
#endif

#define MAX_CONTENT_SIZE 16384

#define PROSODY_COUTOUR_MAX_NUM 10

enum class EErrorCode
{
	ERR_NONE = 0,
	ERR_EMPTY_DATA,

};

enum class EVoiceType
{
	YoungFemale = 0,				 // Xiaoxiao
	AdultMale = 1,					 // Yunyang
	YoungFemaleBitNeutral = 2,		 // Xiaochen
	AdultFemale = 3,				 // Xiaohan
	YoungFemale2 = 4,				 // Xiaomeng
	AdultFemaleBitGental = 5,		 // Xiaomo
	AdultFemaleBitOld = 6,			 // Xiaoqiu
	AdultFemaleLikeASoldier = 7,	 // Xiaorui
	LittleGirl = 8,					 // Xiaoshuang
	AdultFemaleBitGental2 = 9,		 // Xiaoxuan
	YoungFemale3 = 10,				 // Xiaoyan
	HighSchoolGirl = 11,	 	     // Xiaoyi
	GoodLittleGirl = 12,			 // Xiaoyou
	YoungFemaleBitOld = 13,			 // Xiaozhen
	AdultMaleBitSerious = 14,		 // Yunfeng
	AdultMaleBitGental = 15,		 // Yunhao
	AdultMaleWithPower = 16,		 // Yunjian
	LittleBoy = 17,					 // Yunxia
	YoungMale = 18,					 // Yunxi
	AdultMaleLikeANewsReporter = 19, // Yunye
	AdultMaleBitOld = 20,			 // Yunze
};

enum class EExpressStyle
{
	general = 0,
	advertisement_upbeat,
	affectionate,
	angry,
	assistant,
	calm,
	chat,
	cheerful,
	customerservice,
	depressed,
	disgruntled,
	documentary_narration,
	embarrassed,
	empathetic,
	envious,
	excited,
	fearful,
	friendly,
	gentle,
	hopeful,
	lyrical,
	narration_professional,
	narration_relaxed,
	newscast,
	newscast_casual,
	newscast_formal,
	poetry_reading,
	sad,
	serious,
	shouting,
	sports_commentary,
	sports_commentary_excited,
	whispering,
	terrified,
	unfriendly
};

enum class EExpressStyleRole
{
	None = 0,
	Girl,
	Boy,
	YoungAdultFemale,
	YoungAdultMale,
	OlderAdultFemale,
	OlderAdultMale,
	SeniorFemale,
	SeniorMale
};

struct AudioHeader
{
	EVoiceType eVoiceType;
	int nRate;
	int nPitch;
	int nContourNum;
	char chContour[PROSODY_COUTOUR_MAX_NUM][2];
	int nLeadingSilenceValue;
	int nSentenceBoundarySilenceValue;
	int nTailingSilenceValue;
	EExpressStyle eExpressStyle;
	int nExpressStyleDegree;
	EExpressStyleRole eExpressStyleRole;
	int nContentLength;
};

/* Used to generate audio.
 * pData = AudioHeader + Content, several of them.
 * nContentLenth: Length of content.
 * Content: UTF8 form.
 * 
 * @return: 0:Success
 */
extern "C" MS_VVR_REQ_API int GenerateAudio(void* pData, int nLen);

/* Set the path to the path you used, default current executable path. 
i.e, if you used as unreal plugin lib, you should set this variable as the plugin root directory.
e.g, YourPath/
*/
extern "C" MS_VVR_REQ_API int SetProjectPath(char* pNewPath);

extern "C" MS_VVR_REQ_API int SetOutputPath(char* pNewPath);
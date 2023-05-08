
#include "log.h"
#include <fstream>
#include <direct.h>


bool LogModule::StartModule()
{
	_mkdir((PROJECT_PATH + "logs").c_str());
	_mkdir((INPUT_PATH).c_str());
	_mkdir(PathMgr::m_strOutputPath.c_str());
	_mkdir((OUTPUT_PATH).c_str());

	time_t currentTime = std::time(0);
	tm date;
	localtime_s(&date, &currentTime);
	char chFileName[LOG_FILE_NAME_MAX_LENGTH];
	sprintf_s(chFileName, "%d.%d.%d-%d.%d.%d.log", date.tm_year + 1900, date.tm_mon + 1, date.tm_mday, date.tm_hour, date.tm_min, date.tm_sec);
	m_szFileName.append(chFileName);
	m_logFile.open(PROJECT_PATH + "logs/" + m_szFileName, std::ios::out | std::ios::app);

	if (!m_logFile.is_open()) return false;

	SetRunningFlag(true);

	return true;
}

void LogModule::EndModule()
{
	SetRunningFlag(false);

	m_szFileName.clear();

	m_logFile.close();
}
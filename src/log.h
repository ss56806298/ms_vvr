#pragma once

#include "common-def.h"
#include "module-singleton.h"
#include <fstream>
#include <iostream>

#define vvr_log(format, ...)\
LogModule::GetInstance()->ExecuteLog(format, ##__VA_ARGS__);\

class LogModule : public ModuleSingleton<LogModule>
{
public:
	virtual bool StartModule() override;
	virtual void EndModule() override;

	void ExecuteLog(const char* format)
	{
		if (!IsRunning()) return;

		char chLine[LOG_FILE_LINE_MAX_LENGTH];
		int nLen = sprintf_s(chLine, format);

		chLine[nLen++] = 0x0a; // \n

		m_logFile.write(&chLine[0], nLen);
		m_logFile.flush();

		chLine[nLen++] = '\0';
		std::cout << chLine;
	};

	template <typename... Types>
	void ExecuteLog(const char* format, Types... Args)
	{
		if (!IsRunning()) return;

		char chLine[LOG_FILE_LINE_MAX_LENGTH];
		int nLen = sprintf_s(chLine, format, Args...);

		chLine[nLen++] = 0x0a; // \n

		m_logFile.write(&chLine[0], nLen);
		m_logFile.flush();

		chLine[nLen++] = '\0';
		std::cout << chLine;
	};
private:
	std::ofstream m_logFile;
	String m_szFileName;
};
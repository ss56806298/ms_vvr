#pragma once

#include <string>

typedef std::string String;
typedef unsigned long uint64;
typedef unsigned char uint8;
typedef long int64;

class PathMgr
{
public:
	static String m_strProjectPath;

	static String m_strOutputPath;
};

#ifdef _DEBUG
#define PROJECT_PATH String("../")
#else
#define PROJECT_PATH PathMgr::m_strProjectPath + String("")
#endif

#define INPUT_PATH\
	PROJECT_PATH + "inputs/"

#define OUTPUT_PATH\
	PathMgr::m_strOutputPath + "/outputs/"

#define LOG_FILE_NAME_MAX_LENGTH 64

#define LOG_FILE_LINE_MAX_LENGTH 1024

#define DEFAULT_OUTPUT_SOUND_FILE_LENGTH 1024 * 1024
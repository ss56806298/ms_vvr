#pragma once

#include "common-def.h"
#include "module-singleton.h"

class SSMLCreatorModule : public ModuleSingleton<SSMLCreatorModule>
{
public:
	virtual bool StartModule() override;
	virtual void EndModule() override;

	const char* GetSSMLFile() { return m_pSSMLFile; }
	bool GenerateSSML(void* pData, int nLen);
private:
	char* m_pSSMLFile;
};
#pragma once

template<typename T>
class ModuleSingleton
{
public:
	static T* GetInstance() 
	{ 
		if (!m_pInstance) 
		{
			m_pInstance = new T();
		}

		return m_pInstance; 
	}
private:
	inline static T* m_pInstance = nullptr;
public:
	ModuleSingleton() {}
	virtual ~ModuleSingleton() {}

	virtual bool StartModule() { return false; }
	virtual void EndModule() {}

	const bool IsRunning() { return bIsRunning; }
	void SetRunningFlag(bool bNewFlag) { bIsRunning = bNewFlag; }
private:
	bool bIsRunning : 1;
};
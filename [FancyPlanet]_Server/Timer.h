#pragma once
#include<windows.h>
#define DECLARE_SINGLETON(ClassName)			\
private:										\
	static ClassName*		m_pInstance;		\
public:											\
	static ClassName*		GetInstance(void)	\
	{											\
		if(m_pInstance == NULL)					\
			m_pInstance = new ClassName;		\
		return m_pInstance;						\
	}											\
	void	DestroyInstance(void)				\
	{											\
		if(m_pInstance != NULL)					\
		{										\
			delete m_pInstance;					\
			m_pInstance = NULL;					\
		}										\
	}											\


#define  IMPLEMENT_SINGLETON(ClassName)			\
ClassName*	ClassName::m_pInstance = NULL;		\

#define GETTIME			CTimeMgr::GetInstance()->GetTime()

class CTimeMgr
{
	DECLARE_SINGLETON(CTimeMgr)
private:
	CTimeMgr();
	~CTimeMgr();
public:
	float GetTime(void);

public:
	void InitTime(void);
	void SetTime(void);

private:
	LARGE_INTEGER		m_FrameTime;
	LARGE_INTEGER		m_FixTime;
	LARGE_INTEGER		m_LastTime;
	LARGE_INTEGER		m_CpuTick;

	float		m_fTime;
};


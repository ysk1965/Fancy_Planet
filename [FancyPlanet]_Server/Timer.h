<<<<<<< HEAD:FancyPlanet_프레임워크/Timer.h
#pragma once
//-----------------------------------------------------------------------------
// File: CGameTimer.h
//-----------------------------------------------------------------------------

const ULONG MAX_SAMPLE_COUNT = 50; // Maximum frame time sample count

class CGameTimer
{
public:
	CGameTimer();
	virtual ~CGameTimer();

	void Tick(float fLockFPS = 0.0f);
	void Start();
	void Stop();
	void Reset();

	unsigned long GetFrameRate(LPTSTR lpszString = NULL, int nCharacters = 0);
	float GetTimeElapsed();
	float GetTotalTime();
private:
	double							m_fTimeScale;
	float							m_fTimeElapsed;

	__int64							m_nBasePerformanceCounter;
	__int64							m_nPausedPerformanceCounter;
	__int64							m_nStopPerformanceCounter;
	__int64							m_nCurrentPerformanceCounter;
	__int64							m_nLastPerformanceCounter;

	__int64							m_nPerformanceFrequencyPerSec;

	float							m_fFrameTime[MAX_SAMPLE_COUNT];
	ULONG							m_nSampleCount;

	unsigned long					m_nCurrentFrameRate;
	unsigned long					m_nFramesPerSecond;
	float							m_fFPSTimeElapsed;

	bool							m_bStopped;
};
=======
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

>>>>>>> f82fe2d5df01e2f881f95d532e5bd8573dd0a8b9:[FancyPlanet]_Server/Timer.h

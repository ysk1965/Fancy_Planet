<<<<<<< HEAD:FancyPlanet_í”„ë ˆìž„ì›Œí¬/Timer.cpp
//-----------------------------------------------------------------------------
// File: CGameTimer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Timer.h"

CGameTimer::CGameTimer()
{
	::QueryPerformanceFrequency((LARGE_INTEGER *)&m_nPerformanceFrequencyPerSec);
	::QueryPerformanceCounter((LARGE_INTEGER *)&m_nLastPerformanceCounter);
	m_fTimeScale = 1.0 / (double)m_nPerformanceFrequencyPerSec;

	m_nBasePerformanceCounter = m_nLastPerformanceCounter;
	m_nPausedPerformanceCounter = 0;
	m_nStopPerformanceCounter = 0;

	m_nSampleCount = 0;
	m_nCurrentFrameRate = 0;
	m_nFramesPerSecond = 0;
	m_fFPSTimeElapsed = 0.0f;
}

CGameTimer::~CGameTimer()
{
}

void CGameTimer::Tick(float fLockFPS)
{
	if (m_bStopped)
	{
		m_fTimeElapsed = 0.0f;
		return;
	}
	float fTimeElapsed;

	::QueryPerformanceCounter((LARGE_INTEGER *)&m_nCurrentPerformanceCounter);
	fTimeElapsed = float((m_nCurrentPerformanceCounter - m_nLastPerformanceCounter) * m_fTimeScale);

	if (fLockFPS > 0.0f)
	{
		while (fTimeElapsed < (1.0f / fLockFPS))
		{
			::QueryPerformanceCounter((LARGE_INTEGER *)&m_nCurrentPerformanceCounter);
			fTimeElapsed = float((m_nCurrentPerformanceCounter - m_nLastPerformanceCounter) * m_fTimeScale);
		}
	}

	m_nLastPerformanceCounter = m_nCurrentPerformanceCounter;

	if (fabsf(fTimeElapsed - m_fTimeElapsed) < 1.0f)
	{
		::memmove(&m_fFrameTime[1], m_fFrameTime, (MAX_SAMPLE_COUNT - 1) * sizeof(float));
		m_fFrameTime[0] = fTimeElapsed;
		if (m_nSampleCount < MAX_SAMPLE_COUNT) m_nSampleCount++;
	}

	m_nFramesPerSecond++;
	m_fFPSTimeElapsed += fTimeElapsed;
	if (m_fFPSTimeElapsed > 1.0f)
	{
		m_nCurrentFrameRate = m_nFramesPerSecond;
		m_nFramesPerSecond = 0;
		m_fFPSTimeElapsed = 0.0f;
	}

	m_fTimeElapsed = 0.0f;
	for (ULONG i = 0; i < m_nSampleCount; i++) m_fTimeElapsed += m_fFrameTime[i];
	if (m_nSampleCount > 0) m_fTimeElapsed /= m_nSampleCount;
}

unsigned long CGameTimer::GetFrameRate(LPTSTR lpszString, int nCharacters)
{
	if (lpszString)
	{
		_itow_s(m_nCurrentFrameRate, lpszString, nCharacters, 10);
		wcscat_s(lpszString, nCharacters, _T(" FPS)"));
	}

	return(m_nCurrentFrameRate);
}

float CGameTimer::GetTimeElapsed()
{
	return(m_fTimeElapsed);
}

float CGameTimer::GetTotalTime()
{
	if (m_bStopped) return(float(((m_nStopPerformanceCounter - m_nPausedPerformanceCounter) - m_nBasePerformanceCounter) * m_fTimeScale));
	return(float(((m_nCurrentPerformanceCounter - m_nPausedPerformanceCounter) - m_nBasePerformanceCounter) * m_fTimeScale));
}

void CGameTimer::Reset()
{
	__int64 nPerformanceCounter;
	::QueryPerformanceCounter((LARGE_INTEGER*)&nPerformanceCounter);

	m_nBasePerformanceCounter = nPerformanceCounter;
	m_nLastPerformanceCounter = nPerformanceCounter;
	m_nStopPerformanceCounter = 0;
	m_bStopped = false;
}

void CGameTimer::Start()
{
	__int64 nPerformanceCounter;
	::QueryPerformanceCounter((LARGE_INTEGER *)&nPerformanceCounter);
	if (m_bStopped)
	{
		m_nPausedPerformanceCounter += (nPerformanceCounter - m_nStopPerformanceCounter);
		m_nLastPerformanceCounter = nPerformanceCounter;
		m_nStopPerformanceCounter = 0;
		m_bStopped = false;
	}
}

void CGameTimer::Stop()
{
	if (!m_bStopped)
	{
		::QueryPerformanceCounter((LARGE_INTEGER *)&m_nStopPerformanceCounter);
		m_bStopped = true;
	}
}
=======
#include "Timer.h"
IMPLEMENT_SINGLETON(CTimeMgr)
CTimeMgr::CTimeMgr()
	: m_fTime(0.f)
{
}


CTimeMgr::~CTimeMgr()
{
}

float CTimeMgr::GetTime(void)
{
	return m_fTime;
}

void CTimeMgr::InitTime(void)
{
	QueryPerformanceCounter(&m_FrameTime);//½Ã°£À» °³¼ö´ÜÀ§·Î ÃøÁ¤
	QueryPerformanceCounter(&m_FixTime);
	QueryPerformanceCounter(&m_LastTime);

	QueryPerformanceFrequency(&m_CpuTick);
}
void CTimeMgr::SetTime(void)
{
	QueryPerformanceCounter(&m_FrameTime);

	if (m_FrameTime.QuadPart - m_LastTime.QuadPart > m_CpuTick.QuadPart)
	{
		QueryPerformanceFrequency(&m_CpuTick);
		m_LastTime.QuadPart = m_FrameTime.QuadPart;
	}

	m_fTime = float((m_FrameTime.QuadPart - m_FixTime.QuadPart) /float(m_CpuTick.QuadPart))/30.f;

	m_FixTime = m_FrameTime;
}
>>>>>>> f82fe2d5df01e2f881f95d532e5bd8573dd0a8b9:[FancyPlanet]_Server/Timer.cpp

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
	QueryPerformanceCounter(&m_FrameTime);//시간을 개수단위로 측정
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

	m_fTime = float((m_FrameTime.QuadPart - m_FixTime.QuadPart) / float(m_CpuTick.QuadPart)) / 30.f;

	m_FixTime = m_FrameTime;
}

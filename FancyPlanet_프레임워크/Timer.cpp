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
	QueryPerformanceCounter(&m_FrameTime);//�ð��� ���������� ����
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

	m_fTime = float(m_FrameTime.QuadPart - m_FixTime.QuadPart) / m_CpuTick.QuadPart;

	m_FixTime = m_FrameTime;
}

#include "stdafx.h"
#include "FmodSound.h"

CFmodSound::CFmodSound(void)
{
	FMOD_System_Create(&m_pSystem);
	FMOD_System_Init(m_pSystem, 32, FMOD_INIT_NORMAL, NULL);
}


CFmodSound::~CFmodSound(void)
{
	FMOD_System_Close(m_pSystem);
	FMOD_System_Release(m_pSystem);
}


void CFmodSound::CreateBGSound(int nCount, string *SoundFileName)
{
	// ȿ�� ���� ����
	m_nBGSoundCount = nCount;
	m_ppBGSound = new FMOD_SOUND*[nCount];
	m_ppBGChannel = new FMOD_CHANNEL*[nCount];

	for (int i = 0; i < nCount; i++)
		FMOD_System_CreateSound(m_pSystem, SoundFileName[i].data(), FMOD_LOOP_NORMAL, 0, &m_ppBGSound[i]);
}



void CFmodSound::CreateEffectSound(int nCount, string *SoundFileName)
{
	// �� �׶��� ���� ����
	m_nEFSoundCount = nCount;
	m_ppEFFSound = new FMOD_SOUND*[nCount];

	for (int i = 0; i < nCount; i++)
		FMOD_System_CreateSound(m_pSystem, SoundFileName[i].data(), FMOD_DEFAULT, 0, &m_ppEFFSound[i]);
}

void CFmodSound::PlaySoundEffect(int nIndex)
{
	if (nIndex < m_nEFSoundCount)
	{
		FMOD_CHANNEL *pChannel;
		FMOD_System_PlaySound(m_pSystem, m_ppEFFSound[nIndex], m_pChannelGroup, 0, &pChannel);



		//FMOD_RESULT F_API FMOD_System_PlaySound(FMOD_SYSTEM *system, FMOD_SOUND *sound, FMOD_CHANNELGROUP *channelgroup, FMOD_BOOL paused, FMOD_CHANNEL **channel);
		//FMOD_RESULT F_API FMOD_System_PlaySound(FMOD_SYSTEM *system, FMOD_CHANNELINDEX channelid, FMOD_SOUND *sound, FMOD_BOOL paused, FMOD_CHANNEL **channel);
	}
}

void CFmodSound::PlaySoundBG(int nIndex)
{
	if (nIndex < m_nBGSoundCount)
	{
		FMOD_CHANNELGROUP* a = NULL;
		FMOD_System_PlaySound(m_pSystem, m_ppBGSound[nIndex], m_pChannelGroup, 0, &m_ppBGChannel[nIndex]);
	}
}

void CFmodSound::StopSoundBG(int nIndex)
{
	if (nIndex < m_nBGSoundCount)
	{
		FMOD_Channel_Stop(m_ppBGChannel[nIndex]);
	}
}

void CFmodSound::ReleaseSound()
{
	int i;

	delete[] m_ppBGChannel;

	for (i = 0; i < m_nBGSoundCount; i++)
		FMOD_Sound_Release(m_ppBGSound[i]);
	delete[] m_ppBGSound;
	for (i = 0; i < m_nEFSoundCount; i++)
		FMOD_Sound_Release(m_ppEFFSound[i]);
	delete[] m_ppEFFSound;
}

void CFmodSound::Update()
{
	if (!m_pSystem)
		FMOD_System_Update(m_pSystem);
}
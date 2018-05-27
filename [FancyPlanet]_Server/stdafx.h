// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#define MSG_BOX(_message)	MessageBox(NULL, _message, L"System Message", MB_OK)

#define FAILED(hr)      (((HRESULT)(hr)) < 0)

#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
//피직스================================================//
#include <PxPhysicsAPI.h>
using namespace physx;
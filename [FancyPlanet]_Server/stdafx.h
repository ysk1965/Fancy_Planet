// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#define MSG_BOX(_message)	MessageBox(NULL, _message, L"System Message", MB_OK)

#define FAILED(hr)      (((HRESULT)(hr)) < 0)

#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
//������================================================//
#include <PxPhysicsAPI.h>
using namespace physx;
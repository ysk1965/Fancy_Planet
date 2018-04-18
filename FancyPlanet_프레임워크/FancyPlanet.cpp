#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console" )
#include "stdafx.h"
#include "FancyPlanet.h"
#include "GameFramework.h"
#include "protocol.h"
#define MAX_LOADSTRING 100
HINSTANCE						ghAppInstance;
TCHAR							szTitle[MAX_LOADSTRING];
TCHAR							szWindowClass[MAX_LOADSTRING];

CGameFramework					gGameFramework;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

//서버 작업을 위한 전역변수들을 선언합니다.
SOCKET g_mysocket;
WSABUF	send_wsabuf;
char 	send_buffer[BUF_SIZE];
WSABUF	recv_wsabuf;
char	recv_buffer[BUF_SIZE];
char	packet_buffer[BUF_SIZE];
DWORD		in_packet_size = 0;
int		saved_packet_size = 0;
int		g_myid; // 내 아이디
HWND main_window_handle = NULL; // save the window handle
HINSTANCE main_instance = NULL;

//모든 클라이언트를 저장할 배열입니다.
PLAYER_INFO g_my_info;
array <PLAYER_INFO, MAX_USER> g_player_info;

//서버 통신 중 오류가 발생하면 프로그램을 종료합니다.
void clienterror()
{
	exit(-1);
}


//패킷의 종류에 따른 처리를 합니다.
void ProcessPacket(char *ptr)
{

	static bool first_time = true;

	switch (ptr[1])//패킷의 1번 인덱스에는 패킷의 종류에 대한 정보가 들어있습니다.
	{
	case SC_PUT_PLAYER://클라이언트 정보에 대한 패킷.
	{
		sc_packet_put_player *my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
		int id = my_packet->id;

		if (first_time) //패킷을 처음 주고 받았을 때 == 내가 접속했을 때
		{
			first_time = false; //이젠 처음이 아니므로 false로 전환
			g_myid = id; //그렇다면 내 아이디는 패킷의 아이디와 같겠죠.
		}
		if (id == g_myid) { // 내 아이디와 패킷의 아이디가 같다면? == 위의 조건문을 돌았다 == 내 정보
			g_my_info.pos = my_packet->m_pos; // 내 플레이어 구조체에 패킷의 좌표값을 저장합니다.

		}
		else {
			g_player_info[id].m_isconnected = true;
			g_player_info[id].pos = my_packet->m_pos;
			printf("Other Client [%d] is Connect to Server", id);
		}
		break;
	}
	case SC_POS://좌표값에대한 패킷.
	{
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		int id = my_packet->id;
		if (id == g_myid) {//내 캐릭터에 대한 패킷이라면

		}
		else //다른 캐릭터의 패킷이라면
			break;
	}

	case SC_REMOVE_PLAYER://접속 종료에 따른 패킷
	{
		sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(ptr);
		int id = my_packet->id;
		if (id == g_myid) {//내 캐릭터의 종료라면??

		}
		else {//다른 캐릭터의 종료라면??

			g_player_info[id].m_isconnected = false;
		}
		break;
	}
	case SC_READY://레디상태에 대한 체크
	{
		sc_packet_ready *my_packet = reinterpret_cast<sc_packet_ready *>(ptr);
		int id = my_packet->id;
		if (id == g_myid)
			{
				g_my_info.m_isready = my_packet->state;
				if (true == g_my_info.m_isready)
					printf("My Client is Ready\n");
				else
					printf("My Client is Not Ready\n");
			}
		else
		{//다른 캐릭터의 준비 패킷이라면?
			g_player_info[id].m_isready = my_packet->state;
			{
				if (true == g_player_info[id].m_isready)
					printf("Client [%d] is Ready\n", id);
				else
					printf("Client [%d] is Not Ready\n", id);
			}
		}
			break;
	}
	
	
	/*case SC_CHAT:
	{
	sc_packet_chat *my_packet = reinterpret_cast<sc_packet_chat *>(ptr);
	int other_id = my_packet->id;
	if (other_id == g_myid) {
	wcsncpy_s(player.message, my_packet->message, 256);
	player.message_time = GetTickCount();
	}
	else if (other_id < NPC_START) {
	wcsncpy_s(skelaton[other_id].message, my_packet->message, 256);
	skelaton[other_id].message_time = GetTickCount();
	}
	else {
	wcsncpy_s(npc[other_id - NPC_START].message, my_packet->message, 256);
	npc[other_id - NPC_START].message_time = GetTickCount();
	}
	break;

	}*/
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

//서버에서 패킷을 전달받을 때 이 함수를 사용합니다.
void ReadPacket(SOCKET sock)
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(sock, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret) {
		int err_code = WSAGetLastError();
		printf("Recv Error [%d]\n", err_code);
	}

	BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);

	while (0 != iobyte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (iobyte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			iobyte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
}




int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{

	bool islogin

		UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	HWND	 hwnd = nullptr;
	MSG msg;
	HACCEL hAccelTable;
	string s_ip;
	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_FANCYPLANET, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
		return(FALSE);

	hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FANCYPLANET));


	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	//원속함수들을 세팅합니다.
	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	printf("Server IP INPUT : ");
	scanf("%s", s_ip);


	g_mysocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int Result = WSAConnect(g_mysocket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

	int retval = WSAAsyncSelect(g_mysocket, main_window_handle, WM_SOCKET, FD_CLOSE | FD_READ);
	if (retval) {
		int err_code = WSAGetLastError();
		printf("AsyncSelect Error [%d]\n", err_code);
	}
	printf("retval : %d\n", retval);
	
	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;
	
	while (1)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		else
		{
			gGameFramework.FrameAdvance();
			XMFLOAT4X4 r = gGameFramework.GetPlayerMatrix();
		}
	}
	gGameFramework.OnDestroy();

	return((int)msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDC_FANCYPLANET));
	wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCE(IDC_LABPROJECT0891);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return ::RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	ghAppInstance = hInstance;

	RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
	AdjustWindowRect(&rc, dwStyle, FALSE);
	HWND hMainWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	main_window_handle = hMainWnd;//핸들 저장

	if (!hMainWnd)
		return(FALSE);

	gGameFramework.OnCreate(hInstance, hMainWnd);

	::ShowWindow(hMainWnd, nCmdShow);
	::UpdateWindow(hMainWnd);

	return(TRUE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	unsigned char packet = 0;
	switch (message)
	{
	case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	case WM_CHAR:
	{
		if (wParam == 'r')
		{
			cs_packet_ready * my_packet = reinterpret_cast<cs_packet_ready *>(send_buffer);
			my_packet->size = sizeof(my_packet);
			send_wsabuf.len = sizeof(my_packet);
			my_packet->type = CS_READY;
			my_packet->state = g_my_info.m_isready;
			DWORD iobyte;
			printf("packet : READY  \n");
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
		}
	
	}
	case WM_KEYUP:
	{
		cs_packet_pos * my_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
		my_packet->size = sizeof(my_packet);
		send_wsabuf.len = sizeof(my_packet);
		my_packet->type = CS_POS;
		gGameFramework.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
		DWORD iobyte;
		//printf("packet: %d\n", my_packet->size);
		//WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

		break;
	}
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			::DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			::DestroyWindow(hWnd);
			break;
		default:
			return(::DefWindowProc(hWnd, message, wParam, lParam));
		}
		break;
	case WM_PAINT:
		hdc = ::BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam)) {
			closesocket((SOCKET)wParam);
			clienterror();
			break;
		}
		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_READ:
			ReadPacket((SOCKET)wParam);
			break;
		case FD_CLOSE:
			closesocket((SOCKET)wParam);
			clienterror();
			break;
		}
	}
	default:
		return(::DefWindowProc(hWnd, message, wParam, lParam));
	}
	return 0;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return((INT_PTR)TRUE);
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			::EndDialog(hDlg, LOWORD(wParam));
			return((INT_PTR)TRUE);
		}
		break;
	}
	return((INT_PTR)FALSE);
}

#include"DBManager.h"

DBMANAGER::DBMANAGER()
{
	setlocale(LC_ALL, "korean");//이 곳에 해도 되나?
}

/////////////////////////////////오류 출력///////////////////////////
void DBMANAGER::HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT *)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}
bool DBMANAGER::DBAccessForLogin(wchar_t* ID,wchar_t* password)
{
	int xPos, yPos = 0;
	int nCHAR_LEVEL, nID;
	bool isSuccessLogin = false;
	SQLLEN cbName = 0, cbxPos = 0, cbyPos = 0, cbLV = 0, cbID = 0;
	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2018_GAME_WT_유재용", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				// Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					wchar_t wc[58] = L"UPDATE dbo.MyDB SET [dummy] = 1 WHERE name = '";
					wcscat(wc, ID);
					wcscat(wc, L"'");

					retcode = SQLExecDirect(hstmt, (SQLWCHAR*)wc, SQL_NTS);
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
					{
						isSuccessLogin = true;
						//로그인 처리
					}
					else
					{
						isSuccessLogin = false;
						HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
						//로그인 실패
					}
					// Process data  

				}
				else
				{
					HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
				}
				// Process data  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
				{
					SQLCancel(hstmt);
					SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				}
				SQLDisconnect(hdbc);
			}

			SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		}
	}
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
	return isSuccessLogin;
}


void DBMANAGER::DBAccessForSaveResult(PLAYER_INFO a,bool isGameResult)
{
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2018_GAME_WT_유재용", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				// Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
					char c[150];
					/*sprintf(c, "UPDATE dbo.MyDB SET xPos = %d, yPos = %d WHERE name = '%ls'",a[1].x, g_clients[cl].y, g_clients[cl].w_name);

					printf("%s\n", c);
					int nlength = MultiByteToWideChar(CP_ACP, 0, c, -1, NULL, NULL);
					wchar_t* wc = new wchar_t[nlength];
					MultiByteToWideChar(CP_ACP, 0, c, strlen(c) + 1, wc, nlength);
					retcode = SQLExecDirect(hstmt, (SQLWCHAR*)wc, SQL_NTS);*/
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

						printf("DB저장 성공 \n");
					}
					else
					{
						HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
					}
					// Process data  
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
					{
						SQLCancel(hstmt);
						SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					}
					SQLDisconnect(hdbc);
				}

				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
}
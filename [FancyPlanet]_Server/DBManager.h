#pragma once
#include "stdafx.h"
#include "protocol.h"

class DBMANAGER
{
	DBMANAGER();
	bool DBAccessForLogin(wchar_t* ID, wchar_t* password);
	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
	void DBAccessForSaveResult(PLAYER_INFO a, bool isGameResult);
	~DBMANAGER();
	

private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	SQLWCHAR szName[NAME_LEN];
};
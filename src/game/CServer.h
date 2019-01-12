/**
* @file CServer.h
*
*/

#ifndef _INC_CSERVER_H
#define _INC_CSERVER_H

#include "../network/CSocket.h"
#include "../common/CSFileObj.h"
#include "../common/CTextConsole.h"
#include "../common/CDataBase.h"
#include "../common/sqlite/SQLite.h"
#include "../sphere/ConsoleInterface.h"
#include "../game/uo_files/CUOMapList.h"
#include "clients/CChat.h"
#include "CServerDef.h"
#include <atomic>


#define GetMapList CUOMapList &g_Serv.GetUOMapList();

class CItemShip;

enum SERVMODE_TYPE
{
	SERVMODE_RestockAll,		// Major event, potentially slow.
	SERVMODE_GarbageCollection,	// Executing the garbage collection, potentially demanding.
	SERVMODE_Saving,			// Forced save freezes the system.
	SERVMODE_Run,				// Game is up and running.
	SERVMODE_ResyncPause,		// Server paused during resync.

    SERVMODE_PreLoadingINI,     // Initial (first) parsing of the sphere.ini
	SERVMODE_Loading,			// Initial load.
	SERVMODE_ResyncLoad,		// Loading after resync.
	SERVMODE_Exiting			// Closing down.
};


extern class CServer : public CServerDef, public CTextConsole
{
	static lpctstr const sm_szVerbKeys[];

public:
	static const char *m_sClassName;
	std::atomic<SERVMODE_TYPE> m_iModeCode;	// Just some error code to return to system.
	std::atomic_int m_iExitFlag;			// identifies who caused the exit. <0 = error
	bool m_fResyncPause;		// Server is temporarily halted so files can be updated.
	CTextConsole * m_fResyncRequested;		// A resync pause has been requested by this source.

	CSocket m_SocketMain;	// This is the incoming monitor socket.(might be multiple ports?)
	CSocket m_SocketGod;	// This is for god clients.

							// admin console.
	int m_iAdminClients;		// how many of my clients are admin consoles ?
	CSString m_sConsoleText;
	bool m_fConsoleTextReadyFlag;	// interlocking flag for moving between tasks.

	int64 m_timeShutdown;	// When to perform the shutdowm (g_World.clock)
	CChat m_Chats;	// keep all the active chats

	char	m_PacketFilter[255][32];	// list of packet filtering functions
	char	m_OutPacketFilter[255][32];	// list of outgoing packet filtering functions

	CSFileObj	fhFile;			//	file script object
	CDataBase	m_hdb;			//	SQL data base
	CSQLite		m_hldb;			//	Local database
    CUOMapList _pMapList;

private:
	void ProfileDump( CTextConsole * pSrc, bool bDump = false );

public:
	CServer();
	virtual ~CServer();

private:
	CServer(const CServer& copy);
	CServer& operator=(const CServer& other);

public:
    CUOMapList &GetUOMapList();
    SERVMODE_TYPE GetServerMode() const;
	void SetServerMode( SERVMODE_TYPE mode );
    bool IsValidBusy() const;
    int GetExitFlag() const;
    void SetExitFlag(int iFlag);
    bool IsLoading() const;
	void Shutdown( int64 iMinutes );
	void SetSignals( bool fMsg = true );
    bool SetProcessPriority(int iPriorityLevel);

	bool SocketsInit(); // Initialize sockets
	bool SocketsInit( CSocket & socket );
	void SocketsClose();

	bool Load();

	void SysMessage( lpctstr pMsg ) const;
    void SysMessage(ConsoleOutput *pMsg) const;
	void PrintTelnet( lpctstr pszMsg ) const;
    void PrintStr(lpctstr pMsg) const;
    void PrintStr(ConsoleTextColor iColor, lpctstr pMsg) const;
    void PrintOutput(ConsoleOutput *pOutput) const;
	ssize_t PrintPercent( ssize_t iCount, ssize_t iTotal );

	virtual bool r_GetRef( lpctstr & pszKey, CScriptObj * & pRef ) override;
	virtual bool r_WriteVal( lpctstr pszKey, CSString & sVal, CTextConsole * pSrc = nullptr ) override;
	virtual bool r_LoadVal( CScript & s ) override;
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc ) override;

	lpctstr GetStatusString( byte iIndex = 0 ) const;
	int64 GetAgeHours() const;

	bool OnConsoleCmd( CSString & sText, CTextConsole * pSrc );

	void OnTick();

public:
	void ListClients( CTextConsole * pClient ) const;
	void SetResyncPause( bool fPause, CTextConsole * pSrc, bool bMessage = false );
	bool CommandLine( int argc, tchar * argv[] );

	lpctstr GetName() const { return( CServerDef::GetName()); }
	PLEVEL_TYPE GetPrivLevel() const;
} g_Serv;	// current state stuff not saved.


#endif // _INC_CSERVER_H

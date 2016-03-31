
#include "../common/CException.h"
#include "../common/graycom.h"
#include "../sphere/ProfileTask.h"
#include "../sphere/UnixTerminal.h"
#include "CLog.h"
#include "CServer.h"

///////////////////////////////////////////////////////////////
// -CLog

CLog::CLog()
{
	m_fLockOpen = false;
	m_pScriptContext = NULL;
	m_pObjectContext = NULL;
	m_dwMsgMask = LOGL_ERROR |
				  LOGM_INIT | LOGM_CLIENTS_LOG | LOGM_GM_PAGE;
	SetFilePath( GRAY_FILE "log.log" );	// default name to go to.
}

const CScript * CLog::SetScriptContext( const CScript * pScriptContext )
{
	const CScript * pOldScript = m_pScriptContext;
	m_pScriptContext = pScriptContext;
	return( pOldScript );
}

const CScriptObj * CLog::SetObjectContext( const CScriptObj * pObjectContext )
{
	const CScriptObj * pOldObject = m_pObjectContext;
	m_pObjectContext = pObjectContext;
	return( pOldObject );
}
bool CLog::SetFilePath( lpctstr pszName )
{
	ASSERT( ! IsFileOpen());
	return CFileText::SetFilePath( pszName );
}

lpctstr CLog::GetLogDir() const
{
	return( m_sBaseDir );
}

dword CLog::GetLogMask() const
{
	return( m_dwMsgMask &~ 0x0f ) ;
}

void CLog::SetLogMask( dword dwMask )
{
	m_dwMsgMask = GetLogLevel() | ( dwMask &~ 0x0f );
}

bool CLog::IsLoggedMask( dword dwMask ) const
{
	return( ((dwMask &~ (0x0f | LOGM_NOCONTEXT | LOGM_DEBUG)) == 0) ||
			(( GetLogMask() & ( dwMask &~ 0x0f )) != 0) );
}

LOGL_TYPE CLog::GetLogLevel() const
{
	return static_cast<LOGL_TYPE>(m_dwMsgMask & 0x0f);
}

void CLog::SetLogLevel( LOGL_TYPE level )
{
	m_dwMsgMask = GetLogMask() | ( level & 0x0f );
}

bool CLog::IsLoggedLevel( LOGL_TYPE level ) const
{
	return ( ((level & 0x0f) != 0) &&
			 (GetLogLevel() >= ( level & 0x0f ) ) );
}

bool CLog::IsLogged( dword wMask ) const
{
	return IsLoggedMask(wMask) || IsLoggedLevel(static_cast<LOGL_TYPE>(wMask));
}

bool CLog::OpenLog( lpctstr pszBaseDirName )	// name set previously.
{
	if ( m_fLockOpen )	// the log is already locked open
		return( false );

	if ( m_sBaseDir == NULL )
		return false;

	if ( pszBaseDirName != NULL )
	{
		if ( pszBaseDirName[0] && pszBaseDirName[1] == '\0' )
		{
			if ( *pszBaseDirName == '0' )
			{
				Close();
				return false;
			}
		}
		else
		{
			m_sBaseDir = pszBaseDirName;
		}
	}

	// Get the new name based on date.
	m_dateStamp = CGTime::GetCurrentTime();
	tchar *pszTemp = Str_GetTemp();
	sprintf(pszTemp, GRAY_FILE "%d-%02d-%02d.log",
		m_dateStamp.GetYear(), m_dateStamp.GetMonth(), m_dateStamp.GetDay());
	CGString sFileName = GetMergedFileName(m_sBaseDir, pszTemp);

	// Use the OF_READWRITE to append to an existing file.
	if ( CFileText::Open( sFileName, OF_SHARE_DENY_NONE|OF_READWRITE|OF_TEXT ) )
	{
		setvbuf(m_pStream, NULL, _IONBF, 0);
		return true;
	}
	return false;
}

void CLog::SetColor(Color color)
{
	switch (color)
	{
#ifdef _WIN32
        case YELLOW:
            NTWindow_PostMsgColor(RGB(127,127,0));
            break;
        case RED:
            NTWindow_PostMsgColor(RGB(255,0,0));
            break;
        case CYAN:
            NTWindow_PostMsgColor(RGB(0,127,255));
            break;
        default:
            NTWindow_PostMsgColor(0);
#else
		case YELLOW:
			g_UnixTerminal.setColor(UnixTerminal::COL_YELLOW);
			break;
		case RED:
			g_UnixTerminal.setColor(UnixTerminal::COL_RED);
			break;
		case CYAN:
			g_UnixTerminal.setColor(UnixTerminal::COL_CYAN);
			break;
		default:
			g_UnixTerminal.setColor(UnixTerminal::COL_DEFAULT);
#endif
	}
}

int CLog::EventStr( dword wMask, lpctstr pszMsg )
{
	// NOTE: This could be called in odd interrupt context so don't use dynamic stuff
	if ( !IsLogged(wMask) )	// I don't care about these.
		return 0;
	else if ( !pszMsg || !*pszMsg )
		return 0;

	int iRet = 0;
	m_mutex.lock();

	try
	{

		// Put up the date/time.
		CGTime datetime = CGTime::GetCurrentTime();	// last real time stamp.

		if ( datetime.GetDaysTotal() != m_dateStamp.GetDaysTotal())
		{
			// it's a new day, open with new day name.
			Close();	// LINUX should alrady be closed.

			OpenLog( NULL );
			Printf( "%s", datetime.Format(NULL));
		}
		else
		{
#ifndef _WIN32
			uint	mode = OF_READWRITE|OF_TEXT;
			mode |= OF_SHARE_DENY_WRITE;
			Open(NULL, mode);	// LINUX needs to close and re-open for each log line !
#endif
		}

		tchar szTime[32];
		sprintf(szTime, "%02d:%02d:", datetime.GetHour(), datetime.GetMinute());
		m_dateStamp = datetime;

		lpctstr pszLabel = NULL;

		switch (wMask & 0x07)
		{
			case LOGL_FATAL:	// fatal error !
				pszLabel = "FATAL:";
				break;
			case LOGL_CRIT:	// critical.
				pszLabel = "CRITICAL:";
				break;
			case LOGL_ERROR:	// non-fatal errors.
				pszLabel = "ERROR:";
				break;
			case LOGL_WARN:
				pszLabel = "WARNING:";
				break;
		}
		if ( !pszLabel && ( wMask & LOGM_DEBUG ) && !( wMask & LOGM_INIT ))
			pszLabel = "DEBUG:";

		// Get the script context. (if there is one)
		tchar szScriptContext[ _MAX_PATH + 16 ];
		if ( !( wMask&LOGM_NOCONTEXT ) && m_pScriptContext )
		{
			CScriptLineContext LineContext = m_pScriptContext->GetContext();
			sprintf( szScriptContext, "(%s,%d)", m_pScriptContext->GetFileTitle(), LineContext.m_iLineNum );
		}
		else
		{
			szScriptContext[0] = '\0';
		}

		// Print to screen.
		if ( ! ( wMask & LOGM_INIT ) && ! g_Serv.IsLoading())
		{
			SetColor(YELLOW);
			g_Serv.PrintStr( szTime );
			SetColor(DEFAULT);
		}

		if ( pszLabel )	// some sort of error
		{
			SetColor(RED);
			g_Serv.PrintStr( pszLabel );
			SetColor(DEFAULT);
		}

		if ( szScriptContext[0] )
		{
			SetColor(CYAN);
			g_Serv.PrintStr( szScriptContext );
			SetColor(DEFAULT);
		}
		g_Serv.PrintStr( pszMsg );

		// Back to normal color.
		SetColor(DEFAULT);

		// Print to log file.
		WriteString( szTime );
		if ( pszLabel )	WriteString( pszLabel );
		if ( szScriptContext[0] )
		{
			WriteString( szScriptContext );
		}
		WriteString( pszMsg );

		iRet = 1;

#ifndef _WIN32
		Close();
#endif
	}
	catch (...)
	{
		// Not much we can do about this
		iRet = 0;
		CurrentProfileData.Count(PROFILE_STAT_FAULTS, 1);
	}

	m_mutex.unlock();

	return( iRet );
}

CGTime CLog::sm_prevCatchTick;

void _cdecl CLog::CatchEvent( const CSphereError * pErr, lpctstr pszCatchContext, ... )
{
	CGTime timeCurrent = CGTime::GetCurrentTime();
	if ( sm_prevCatchTick.GetTime() == timeCurrent.GetTime() )	// prevent message floods.
		return;
	// Keep a record of what we catch.
	try
	{
		tchar szMsg[512];
		LOGL_TYPE eSeverity;
		int iLen = 0;
		if ( pErr != NULL )
		{
			eSeverity = pErr->m_eSeverity;
			const CAssert * pAssertErr = dynamic_cast<const CAssert*>(pErr);
			if ( pAssertErr )
				pAssertErr->GetErrorMessage(szMsg, sizeof(szMsg), 0);
			else
				pErr->GetErrorMessage(szMsg, sizeof(szMsg));
			iLen = strlen(szMsg);
		}
		else
		{
			eSeverity = LOGL_CRIT;
			strcpy(szMsg, "Exception");
			iLen = strlen(szMsg);
		}

		iLen += sprintf( szMsg+iLen, ", in " );

		va_list vargs;
		va_start(vargs, pszCatchContext);

		iLen += vsprintf(szMsg+iLen, pszCatchContext, vargs);
		iLen += sprintf(szMsg+iLen, "\n");

		EventStr(eSeverity, szMsg);
		va_end(vargs);
	}
	catch(...)
	{
		// Not much we can do about this.
		CurrentProfileData.Count(PROFILE_STAT_FAULTS, 1);
	}
	sm_prevCatchTick = timeCurrent;
}
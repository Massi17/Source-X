/**
* @file CSector.h
*
*/

#ifndef _INC_CSECTOR_H
#define _INC_CSECTOR_H

#include "../common/CSectorTemplate.h"
#include "../common/CScriptObj.h"
#include "../../game/components/CCTimedObject.h"
#include "CSectorEnviron.h"


class CChar;
class CItemStone;
class CUOMap;

class CSector : public CScriptObj, public CSectorBase, public CCTimedObject	// square region of the world.
{
	// A square region of the world. ex: MAP0.MUL Dungeon Sectors are 256 by 256 meters
#define SECTOR_TICK_PERIOD (TICKS_PER_SEC / 2) // after how much ticks do we start a pulse.
    friend class CUOMap;
public:
	static const char *m_sClassName;
	static lpctstr const sm_szVerbKeys[];
	static lpctstr const sm_szLoadKeys[];

private:
	bool   m_fSaveParity;		// has the sector been saved relative to the char entering it ?
	CSectorEnviron m_Env;		// Current Environment

	byte m_RainChance;		// 0 to 100%
	byte m_ColdChance;		// Will be snow if rain chance success.
	byte m_ListenItems;		// Items on the ground that listen ?

    short _iPointX;
    short _iPointY;
protected:
    std::map<DIR_TYPE, CSector*> _mAdjacentSectors;
    /*
    * @brief Asign it's adjacent's sectors
    */
    void SetAdjacentSectors();
    CSector *GetAdjacentSector(DIR_TYPE dir) const;

private:
	WEATHER_TYPE GetWeatherCalc() const;
	byte GetLightCalc( bool fQuickSet ) const;
	void SetLightNow( bool fFlash = false );
	bool IsMoonVisible( uint iPhase, int iLocalTime ) const;
	void SetDefaultWeatherChance();

public:
    CSector(CUOMap * pMap, short x, short y, int index);
	~CSector();
    void init();
	CSector(const CSector& copy);
	CSector& operator=(const CSector& other);

public:
	virtual bool OnTick();
    inline virtual bool IsDeleted() const override
    {
        return false;   // Sectors should never be deleted in runtime.
    }
    short GetPointX() const;
    short GetPointY() const;

	// Time
	int GetLocalTime() const;
	lpctstr GetLocalGameTime() const;

	// Season
	SEASON_TYPE GetSeason() const;
	void SetSeason( SEASON_TYPE season );

	// Weather
	WEATHER_TYPE GetWeather() const;
	bool IsRainOverriden() const;
	byte GetRainChance() const;
	bool IsColdOverriden() const;
	byte GetColdChance() const;
	void SetWeather( WEATHER_TYPE w );
	void SetWeatherChance( bool fRain, int iChance );

	// Light
	bool IsLightOverriden() const;
	byte GetLight() const;
	bool IsDark() const;
	bool IsNight() const;
	void LightFlash();
	void SetLight( int light );

	// Items in the sector
	size_t GetItemComplexity() const;
	bool IsItemInSector( const CItem * pItem ) const;
	void MoveItemToSector( CItem * pItem, bool fActive );

	void AddListenItem();
	void RemoveListenItem();
	bool HasListenItems() const;
	void OnHearItem( CChar * pChar, lpctstr pszText );

	// Chars in the sector.
	bool IsCharActiveIn( const CChar * pChar );
	bool IsCharDisconnectedIn( const CChar * pChar );
	size_t GetCharComplexity() const;
	size_t GetInactiveChars() const;
	size_t HasClients() const;
	int64 GetLastClientTime() const;
	bool CanSleep(bool fCheckAdjacents) const;
	void SetSectorWakeStatus();	// Ships may enter a sector before it's riders !
	void ClientAttach( CChar * pChar );
	void ClientDetach( CChar * pChar );
	bool MoveCharToSector( CChar * pChar );
    void AddChar(CChar *pChar);
    void DelChar(CChar *pChar);

	// CCTimedObject
private:
    virtual void GoSleep() override;
    virtual void GoAwake() override;

    // General.
public:
	virtual bool r_LoadVal( CScript & s ) override;
	virtual bool r_WriteVal( lpctstr pszKey, CSString & sVal, CTextConsole * pSrc ) override;
	virtual void r_Write();
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc ) override;

	// AllThings Verbs
	bool v_AllChars( CScript & s, CTextConsole * pSrc );
	bool v_AllCharsIdle( CScript & s, CTextConsole * pSrc );
	bool v_AllItems( CScript & s, CTextConsole * pSrc );
	bool v_AllClients( CScript & s, CTextConsole * pSrc );

	// Other resources.
	void Restock();
	void RespawnDeadNPCs();

	void Close();
	lpctstr GetName() const { return "Sector"; }
};


#endif // _INC_CSECTOR_H

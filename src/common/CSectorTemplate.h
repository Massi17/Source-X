/**
* @file CSectorTemplate.h
*
*/

#ifndef _INC_CSECTORTEMPLATE_H
#define _INC_CSECTORTEMPLATE_H

#include "../game/CServer.h"
#include "../game/CRegion.h"


class CItem;
class CServerMapBlock;
class CSector;

class CCharsDisconnectList : public CSObjList
{
public:
	static const char *m_sClassName;
public:
	CCharsDisconnectList();
private:
	CCharsDisconnectList(const CCharsDisconnectList& copy);
	CCharsDisconnectList& operator=(const CCharsDisconnectList& other);
};

class CCharsActiveList : public CSObjList
{
private:
	size_t m_iClients; // How many clients in this sector now?
    CSector *_pLinkedSector;
public:
	static const char *m_sClassName;
	int64 m_timeLastClient;	// age the sector based on last client here.

protected:
	void OnRemoveObj( CSObjListRec* pObRec );	// Override this = called when removed from list.

public:
	size_t HasClients() const;
	void ClientAttach();
	void ClientDetach();
	void AddCharToSector( CChar * pChar );
    void init(CSector *pLink);
public:
	CCharsActiveList();

private:
	CCharsActiveList(const CCharsActiveList& copy);
	CCharsActiveList& operator=(const CCharsActiveList& other);
};

class CItemsList : public CSObjList
{
	// Top level list of items.
public:
	static bool sm_fNotAMove;	// hack flag to prevent items from bouncing around too much.

protected:
	void OnRemoveObj( CSObjListRec* pObRec );	// Override this = called when removed from list.

public:
	static const char *m_sClassName;
	void AddItemToSector( CItem * pItem );

public:
	CItemsList();

private:
	CItemsList(const CItemsList& copy);
	CItemsList& operator=(const CItemsList& other);
};

class CObjPointSortArray : public CSObjSortArray< CPointSort*, int >
{
public:
	static const char *m_sClassName;
	// Find a point fast.
	int CompareKey( int id, CPointSort* pBase, bool fNoSpaces ) const;
public:
	CObjPointSortArray();
private:
	CObjPointSortArray(const CObjPointSortArray& copy);
	CObjPointSortArray& operator=(const CObjPointSortArray& other);
};

class CSectorBase		// world sector
{
protected:
	int	m_index;		// sector index
	int m_map;			// sector map
private:
	typedef std::map<int, CServerMapBlock*>	MapBlockCache;
	MapBlockCache							m_MapBlockCache;
public:
	static const char *m_sClassName;
	CObjPointSortArray	m_Teleports;		//	CTeleport array
	CRegionLinks		m_RegionLinks;		//	CRegion(s) in this CSector
	dword			m_dwFlags;
public:
	CCharsActiveList		m_Chars_Active;		// CChar(s) activte in this CSector.
	CCharsDisconnectList	m_Chars_Disconnect;	// dead NPCs, etc
	CItemsList m_Items_Timer;				// CItem(s) in this CSector that need timers.
	CItemsList m_Items_Inert;				// CItem(s) in this CSector. (no timer required)
protected:
    std::map<DIR_TYPE, CSector*> _mAdjacentSectors;
public:
    /*
    * @brief Asign it's adjacent's sectors
    */
    void SetAdjacentSectors();
    CSector *GetAdjacentSector(DIR_TYPE dir) const;
public:
	CSectorBase();
	virtual ~CSectorBase();

private:
	CSectorBase(const CSectorBase& copy);
	CSectorBase& operator=(const CSectorBase& other);

public:
	void Init(int index, int newmap);

	// Location map units.
	int GetIndex() const { return m_index; }
	int GetMap() const { return m_map; }
	CPointMap GetBasePoint() const;
	CRectMap GetRect() const;
	bool IsInDungeon() const;

	bool static CheckMapBlockTime( const MapBlockCache::value_type& Elem );
	void ClearMapBlockCache();
	void CheckMapBlockCache();
	static int64 m_iMapBlockCacheTime;
	const CServerMapBlock * GetMapBlock( const CPointMap & pt );

	// CRegion
	CRegion * GetRegion( const CPointBase & pt, dword dwType ) const;
	size_t GetRegions( const CPointBase & pt, dword dwType, CRegionLinks & rlist ) const;

	bool UnLinkRegion( CRegion * pRegionOld );
	bool LinkRegion( CRegion * pRegionNew );

	// CTeleport(s) in the region.
	CTeleport * GetTeleport( const CPointMap & pt ) const;
	bool AddTeleport( CTeleport * pTeleport );

	bool IsFlagSet( dword dwFlag ) const;

#define SECF_NoSleep	0x00000001
#define SECF_InstaSleep	0x00000002
};

#endif // _INC_CSECTORTEMPLATE_H

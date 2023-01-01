/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
/* Based on Race mod stuff and tweaked by GreYFoX@GTi and others to fit our DDRace needs. */
/* CSqlScore Class by Sushi Tee*/
#ifndef GAME_SERVER_SCORE_SQL_SCORE_H
#define GAME_SERVER_SCORE_SQL_SCORE_H

#include <exception>

#include <base/system.h>
#include <engine/console.h>
#include <engine/server/sql_connector.h>
#include <engine/server/sql_string_helpers.h>

#include "../score.h"


class CGameContextError : public std::runtime_error
{
public:
	CGameContextError(const char* pMsg) : std::runtime_error(pMsg) {}
};


// generic implementation to provide gameserver and server
struct CSqlData
{
	CSqlData() : m_Map(ms_pMap), m_GameUuid(ms_pGameUuid)
	{
		m_Instance = ms_Instance;
	}

	virtual ~CSqlData() {}

	bool isGameContextVaild() const
	{
		return m_Instance == ms_Instance && ms_GameContextAvailable;
	}

	CGameContext* GameServer() const { return isGameContextVaild() ? ms_pGameServer : throw CGameContextError("[CSqlData]: GameServer() unavailable."); }
	IServer* Server() const { return isGameContextVaild() ? ms_pServer : throw CGameContextError("[CSqlData]: Server() unavailable."); }
	CPlayerData* PlayerData(int ID) const { return isGameContextVaild() ? &ms_pPlayerData[ID] : throw CGameContextError("[CSqlData]: PlayerData() unavailable."); }

	sqlstr::CSqlString<128> m_Map;
	sqlstr::CSqlString<UUID_MAXSTRSIZE> m_GameUuid;

	// counter to keep track to which instance of GameServer this object belongs to.
	int m_Instance;

	static CGameContext *ms_pGameServer;
	static IServer *ms_pServer;
	static CPlayerData *ms_pPlayerData;
	static const char *ms_pMap;
	static const char *ms_pGameUuid;

	static bool ms_GameContextAvailable;
	// contains the instancecount of the current GameServer
	static int ms_Instance;
};

struct CSqlExecData
{
	CSqlExecData(bool (*pFuncPtr) (CSqlServer*, const CSqlData *, bool), CSqlData *pSqlData, bool ReadOnly = true) :
		m_pFuncPtr(pFuncPtr),
		m_pSqlData(pSqlData),
		m_ReadOnly(ReadOnly)
	{
		++ms_InstanceCount;
	}
	~CSqlExecData()
	{
		--ms_InstanceCount;
	}

	bool (*m_pFuncPtr) (CSqlServer*, const CSqlData *, bool);
	CSqlData *m_pSqlData;
	bool m_ReadOnly;

	// keeps track of score-threads
	volatile static int ms_InstanceCount;
};

struct CSqlPlayerData : CSqlData
{
	int m_ClientID;
	sqlstr::CSqlString<MAX_NAME_LENGTH> m_Name;
};

// used for mapinfo
struct CSqlMapData : CSqlData
{
	int m_ClientID;

	sqlstr::CSqlString<128> m_RequestedMap;
	char m_aFuzzyMap[128];
	sqlstr::CSqlString<MAX_NAME_LENGTH> m_Name;
};

// used for mapvote
struct CSqlMapVoteData : CSqlMapData
{
	std::shared_ptr<CMapVoteResult> m_pResult;
};

struct CSqlScoreData : CSqlData
{
	int m_ClientID;

	sqlstr::CSqlString<MAX_NAME_LENGTH> m_Name;

	bool m_NotEligible;
	float m_Time;
	char m_aTimestamp[TIMESTAMP_STR_LENGTH];
	float m_aCpCurrent[NUM_CHECKPOINTS];
	int m_Num;
	bool m_Search;
	char m_aRequestingPlayer[MAX_NAME_LENGTH];
};

struct CSqlTeamScoreData : CSqlData
{
	bool m_NotEligible;
	float m_Time;
	char m_aTimestamp[TIMESTAMP_STR_LENGTH];
	unsigned int m_Size;
	int m_aClientIDs[MAX_CLIENTS];
	sqlstr::CSqlString<MAX_NAME_LENGTH> m_aNames[MAX_CLIENTS];
};

struct CSqlTeamSave : CSqlData
{
	virtual ~CSqlTeamSave();

	int m_Team;
	int m_ClientID;
	sqlstr::CSqlString<128> m_Code;
	char m_Server[5];
};

struct CSqlTeamLoad : CSqlData
{
	sqlstr::CSqlString<128> m_Code;
	int m_ClientID;
};

struct CSqlRandomMap : CSqlScoreData
{
	std::shared_ptr<CRandomMapResult> m_pResult;
};

class CSqlScore: public IScore
{
	CGameContext *GameServer() { return m_pGameServer; }
	IServer *Server() { return m_pServer; }

	CGameContext *m_pGameServer;
	IServer *m_pServer;

	static void ExecSqlFunc(void *pUser);

	static bool Init(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure);

	char m_aMap[64];
	char m_aGameUuid[UUID_MAXSTRSIZE];

	static LOCK ms_FailureFileLock;

	static bool CheckBirthdayThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool MapInfoThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool MapVoteThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool LoadScoreThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool SaveScoreThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool SaveTeamScoreThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool ShowRankThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool ShowTop5Thread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool ShowTeamRankThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool ShowTeamTop5Thread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool ShowTimesThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool ShowPointsThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool ShowTopPointsThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool RandomMapThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool RandomUnfinishedMapThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool SaveTeamThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);
	static bool LoadTeamThread(CSqlServer* pSqlServer, const CSqlData *pGameData, bool HandleFailure = false);

public:

	CSqlScore(CGameContext *pGameServer);
	~CSqlScore();

	virtual void CheckBirthday(int ClientID);
	virtual void LoadScore(int ClientID);
	virtual void MapInfo(int ClientID, const char* MapName);
	virtual void MapVote(std::shared_ptr<CMapVoteResult> *ppResult, int ClientID, const char* MapName);
	virtual void SaveScore(int ClientID, float Time, const char *pTimestamp,
			float CpTime[NUM_CHECKPOINTS], bool NotEligible);
	virtual void SaveTeamScore(int* aClientIDs, unsigned int Size, float Time, const char *pTimestamp);
	virtual void ShowRank(int ClientID, const char* pName, bool Search = false);
	virtual void ShowTeamRank(int ClientID, const char* pName, bool Search = false);
	virtual void ShowTimes(int ClientID, const char* pName, int Debut = 1);
	virtual void ShowTimes(int ClientID, int Debut = 1);
	virtual void ShowTop5(IConsole::IResult *pResult, int ClientID,
			void *pUserData, int Debut = 1);
	virtual void ShowTeamTop5(IConsole::IResult *pResult, int ClientID,
			void *pUserData, int Debut = 1);
	virtual void ShowPoints(int ClientID, const char* pName, bool Search = false);
	virtual void ShowTopPoints(IConsole::IResult *pResult, int ClientID,
			void *pUserData, int Debut = 1);
	virtual void RandomMap(std::shared_ptr<CRandomMapResult> *ppResult, int ClientID, int stars);
	virtual void RandomUnfinishedMap(std::shared_ptr<CRandomMapResult> *ppResult, int ClientID, int stars);
	virtual void SaveTeam(int Team, const char* Code, int ClientID, const char* Server);
	virtual void LoadTeam(const char* Code, int ClientID);

	virtual void OnShutdown();
};

#endif // GAME_SERVER_SCORE_SQL_SCORE_H

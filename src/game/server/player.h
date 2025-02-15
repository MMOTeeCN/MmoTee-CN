/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_PLAYER_H
#define GAME_SERVER_PLAYER_H

// this include should perhaps be removed
#include "entities/character.h"
#include "gamecontext.h"
#include <game/server/classes.h>
#include "cmds.h"

#include "playerdata.h"

class CPlayer
{
	MACRO_ALLOC_POOL_ID()

public:
	// Основное
	// 主要的
	CPlayer(CGameContext *pGameServer, int ClientID, int Team);
	~CPlayer();

	void Init(int CID);
	int GetCID() const { return m_ClientID; };
	int GetMapID() const { return IsBot() ? m_MapID : Server()->GetClientMapID(GetCID()); };

	void TryRespawn();
	void Respawn();
	void SetTeam(int Team, bool DoChatMsg=true);
	int GetTeam() const { return m_Team; };

	void Tick();
	void PostTick();
	void Snap(int SnappingClient);
	void FakeSnap(int SnappingClient);
	
	const char* TitleGot();

	void OnDirectInput(CNetObj_PlayerInput *NewInput);
	void OnPredictedInput(CNetObj_PlayerInput *NewInput);
	void OnDisconnect(int Type, const char *pReason);

	void KillCharacter(int Weapon = WEAPON_GAME);
	CCharacter *GetCharacter();
	class CCmd *m_pChatCmd;

	vec2 m_ViewPos;
	int m_PlayerFlags;
	int m_aActLatency[MAX_PLAYERS];
	int m_SpectatorID;

	bool m_IsReady;
	bool m_IsInGame;

	bool GetShop();
	bool GetWork();
	bool GetBoss();

	bool IsBot() const { return m_Bot; }
	int GetBotType() const { return m_BotType; }
	int GetBotSubType() const { return m_BotSubType; }
	void SetBotType(int BotType) { m_BotType = BotType; }
	void SetBotSubType(int BotSubType) { m_BotSubType = BotSubType; }
	bool IsBoss();

	// Пользовальские переменные
	// 自定义变量
	int m_HealthStart;
	int m_Health;

	int m_MoneyAdd;
	int m_ExperienceAdd;
	int m_OpenBox;
	int m_OpenBoxType;
	int m_OpenBoxAmount;
	bool m_BigBot;
	int m_aInviteClanID;
	bool m_ActiveChair;
	bool m_AntiPvpSmall;
	bool m_InBossed;
	bool m_InArea;
	int m_LoginSync;

	int m_UpdateMenu;
	int m_SortedSelectTop;
	int m_SortedSelectCraft;
	bool m_Search;
	int m_JailTick;
	
	char m_aSelectPlayer[64];
	int m_SelectItem;
	int m_SelectItemType;
	int m_SelectArmor;
	int m_SelectQuest;
	int m_SelectSubQuest;

	int m_LastVotelist;

	int m_ManaTick;
	int m_Mana;

	int m_Vote;
	int m_VotePos;

	int m_LastVoteCall;
	int m_LastVoteTry;
	int m_LastChat;
	int m_LastSetSpectatorMode;
	int m_LastChangeInfo;
	int m_LastEmote;
	int m_LastKill;
	int m_AngryWroth;

	// TODO: clean this up
	struct
	{
		char m_aSkinName[64];
		bool m_UseCustomColor;
		int m_ColorBody;
		int m_ColorFeet;
	} m_TeeInfos;

	int m_RespawnTick;
	int m_DieTick;
	int m_ScoreStartTick;
	int m_LastActionTick;
	int m_SpecTick;

	bool m_StolenSkin;	
	struct
	{
		int m_TargetX;
		int m_TargetY;
	} m_LatestActivity;

	// network latency calculations
	struct
	{
		int m_Accum;
		int m_AccumMin;
		int m_AccumMax;
		int m_Avg;
		int m_Min;
		int m_Max;
	} m_Latency;

	int m_Authed;
	
	int GetClass(); 
	void SetClassSkin(int newClass, int State = 0);
	void SetClass(int newClass);
	void RandomBoxTick();
	void BasicAuthedTick(); 
	
	const char* GetLanguage();
	const char* GetClassName();
	void SetLanguage(const char* pLanguage);
	
	int GetNeedMana() {	return 100+(AccUpgrade()->m_Mana*5); };
	int GetNeedForUp();
	int GetNeedForUpgClan(Clan Type);
	int m_MapMenuItem;
	 
	CTuningParams m_PrevTuningParams;
	CTuningParams m_NextTuningParams;
	
	void HandleTuningParams();
	
	bool InscoreBoard() { return m_PlayerFlags & PLAYERFLAG_SCOREBOARD; };
	int MapMenu() { return (m_Team != TEAM_SPECTATORS) ? m_MapMenu : 0; };
	void OpenMapMenu(int Menu);
	void CloseMapMenu();
	bool MapMenuClickable();
	
	void ExpAdd(unsigned long int Size, bool Bonus = true);
	void MoneyAdd(int Size, bool ClanBonus = false, bool MoneyDouble = false);
	
	void ResetUpgrade(int ClientID);
	void ResetSkill(int ClientID);

	void InitSnap();
	void UpdateSnap();

	bool GetSnap(int EntityID);
	void SetSnap(int EntityID, bool Snap);

	SAccData *AccData() { return Server()->GetAccData(GetCID()); };
	SAccUpgrade *AccUpgrade() { return Server()->GetAccUpgrade(GetCID()); };

	int m_MapID;

	void GiveUpPoint(int Num);

private:
	CCharacter *m_pCharacter;
	CGameContext *m_pGameServer;

	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const;

	bool m_Spawning;
	int m_ClientID;
	int m_Team;

	char m_aTitle[64];
	bool m_ShowClan;

	// Пользовальские переменные
	// 自定义变量
	char m_aLanguage[16];
	
	int m_MapMenu;
	int m_MapMenuTick;
	
	bool m_Bot;
	int m_BotType;
	int m_BotSubType;

	bool m_aShouldSnap[NUM_ENTTYPES];
};

enum
{
	PLAYERITER_ALL=0x0,
	
	PLAYERITER_COND_READY=0x1,
	PLAYERITER_COND_SPEC=0x2,
	PLAYERITER_COND_NOSPEC=0x4,
	
	PLAYERITER_INGAME = PLAYERITER_COND_READY | PLAYERITER_COND_NOSPEC,
	PLAYERITER_SPECTATORS = PLAYERITER_COND_READY | PLAYERITER_COND_SPEC,
};

template<int FLAGS>
class CPlayerIterator
{
private:
	CPlayer** m_ppPlayers;
	int m_ClientID;
	
public:
	
	CPlayerIterator(CPlayer** ppPlayers) :
		m_ppPlayers(ppPlayers)
	{
		Reset();
	}
	
	inline bool Next()
	{
		for(m_ClientID = m_ClientID+1; m_ClientID<MAX_CLIENTS; m_ClientID++)
		{
			CPlayer* pPlayer = Player();
			
			if(!pPlayer) continue;
			if((FLAGS & PLAYERITER_COND_READY) && (!pPlayer->m_IsInGame)) continue;
			if((FLAGS & PLAYERITER_COND_NOSPEC) && (pPlayer->GetTeam() == TEAM_SPECTATORS)) continue;
			if((FLAGS & PLAYERITER_COND_SPEC) && (pPlayer->GetTeam() != TEAM_SPECTATORS)) continue;
			
			return true;
		}
		
		return false;
	}
	
	inline void Reset() { m_ClientID = -1; }
	
	inline CPlayer* Player() { return m_ppPlayers[m_ClientID]; }
	inline int ClientID() { return m_ClientID; }
};

#endif

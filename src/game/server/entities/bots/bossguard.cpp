/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/mapitems.h>
#include "bossguard.h"
#include "../character.h"
#include "../projectile.h"

MACRO_ALLOC_POOL_ID_IMPL(CBossGuard, MAX_CLIENTS)

CBossGuard::CBossGuard(CGameWorld *pWorld)
	: CCharacter(pWorld)
{
	m_BotDir = 1;
	m_BotLastPos = m_Pos;
	m_BotLastStuckTime = 0.0f;
	m_BotStuckCount = 0;
	m_BotTimePlayerFound = Server()->Tick();
	m_BotTimeGrounded = Server()->Tick();
	m_BotTimeLastOption = Server()->Tick();
	m_BotTimeLastDamage = 0.0f;
	m_BotClientIDFix = -1;
	m_BotTimeLastSound = Server()->Tick();
	m_BotTimeLastChat = Server()->Tick();
	m_BotJumpTry = false;
}

void CBossGuard::Tick()
{
	TickBotAI();
	CCharacter::Tick();
}

bool CBossGuard::TakeDamage(vec2 Force, int Dmg, int From, int Weapon)
{
	if (CCharacter::TakeDamage(Force, Dmg, From, Weapon, 0))
	{
		m_BotTimeLastDamage = Server()->Tick();
		return true;
	}

	return false;
}

void CBossGuard::RunAction()
{
	if (m_BotClientIDFix != -1 && GameServer()->m_apPlayers[m_BotClientIDFix])
	{
		CCharacter *pChar = GameServer()->m_apPlayers[m_BotClientIDFix]->GetCharacter();
		if (!pChar)
		{
			m_BotClientIDFix = -1;
			return;
		}

		if (Server()->Tick() - m_BotTimePlayerFound > Server()->TickSpeed() * 0.01f)
		{
			m_LatestInput.m_Fire = m_Input.m_Fire = 1;
			m_BotTimePlayerFound = Server()->Tick();
		}

		m_BotClientIDFix = -1;
	}
}

void CBossGuard::TickBotAI()
{
	m_ActiveWeapon = WEAPON_HAMMER;

	// ЗВУКИ
	if (Server()->Tick() - m_BotTimeLastSound > Server()->TickSpeed() * 5.0f && random_prob(0.02f))
	{
		// PlaySound();
		m_BotTimeLastSound = Server()->Tick();
	}

	EmoteNormal = EMOTE_HAPPY;

	// ОЧИСТКА ДЕЙСТВИЙ
	m_Input.m_Hook = 0;
	m_Input.m_Fire = 0;
	m_Input.m_Jump = 0;

	// ВЫПОЛНЕНИЕ СЦЕНАРИЯ
	RunAction();

	// ИНТЕРАКТ С ИГРОКАМИ
	bool PlayerClose = false;
	bool PlayerFound = false;
	bool PlayerNFound = false;

	m_BotClientIDFix = -1;
	for (int i = 0; i < g_Config.m_SvMaxClients; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if (!pPlayer || !pPlayer->GetCharacter() || pPlayer->IsBot() || pPlayer->IsBoss() || !pPlayer->m_InBossed)
			continue;

		int Collide = GameServer()->Collision()->IntersectLine(pPlayer->GetCharacter()->m_Pos, m_Pos, 0, 0);
		int Dist = distance(pPlayer->GetCharacter()->m_Pos, m_Pos);

		if (Dist > 100.0f)
		{
			vec2 DirPlayer = normalize(pPlayer->GetCharacter()->m_Pos - m_Pos);
			if (DirPlayer.x < 0)
				m_BotDir = -1;
			else
				m_BotDir = 1;

			if (!Collide)
				m_Input.m_Hook = 1;
		}
		else
		{
			PlayerClose = true;

			m_BotDir = 0;
			m_BotClientIDFix = pPlayer->GetCID();
		}

		m_Input.m_TargetX = static_cast<int>(pPlayer->GetCharacter()->m_Pos.x - m_Pos.x);
		m_Input.m_TargetY = static_cast<int>(pPlayer->GetCharacter()->m_Pos.y - m_Pos.y);

		m_Input.m_Fire = 1;

		PlayerFound = true;
	}

	// Fix target
	if (!PlayerFound && !PlayerNFound)
	{
		m_Input.m_TargetX = m_BotDir;
		m_Input.m_TargetY = 0;
	}
	else if (m_BotClientIDFix != -1)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[m_BotClientIDFix];
		if (pPlayer && pPlayer->GetCharacter() && m_Pos.y > pPlayer->GetCharacter()->m_Pos.y) // Jump to player
			m_Input.m_Jump = 1;
	}

	// Random Actions
	if (!PlayerFound)
	{
		if (Server()->Tick() - m_BotTimeLastOption > Server()->TickSpeed() * 10.0f)
		{
			int Action = random_int(0, 3);
			if (Action == 0)
				m_BotDir = -1;
			else if (Action == 1)
				m_BotDir = 1;
			else if (Action == 2)
				m_BotDir = 0;

			m_BotTimeLastOption = Server()->Tick();
		}
	}

	// Interact with the envirionment
	float radiusZ = ms_PhysSize / 2.0f;
	if (distance(m_Pos, m_BotLastPos) < radiusZ || abs(m_Pos.x - m_BotLastPos.x) < radiusZ)
	{
		if (Server()->Tick() - m_BotLastStuckTime > Server()->TickSpeed() * 0.5f)
		{
			m_BotStuckCount++;
			if (m_BotStuckCount == 15)
			{
				if (!m_BotJumpTry)
				{
					m_Input.m_Jump = 1;
					m_BotJumpTry = true;
				}
				else
				{
					m_BotDir = random_prob(0.3333333333f) ? 1 : -1;
					m_BotJumpTry = false;
				}

				m_BotStuckCount = 0;
				m_BotLastStuckTime = Server()->Tick();
			}
		}
	}

	// Fix Stuck
	if (IsGrounded())
		m_BotTimeGrounded = Server()->Tick();

	// Falls
	if (m_Core.m_Vel.y > GameServer()->Tuning()->m_Gravity)
	{
		if (m_BotClientIDFix != -1)
		{
			CPlayer *pPlayer = GameServer()->m_apPlayers[m_BotClientIDFix];
			if (pPlayer && pPlayer->GetCharacter() && m_Pos.y > pPlayer->GetCharacter()->m_Pos.y)
				m_Input.m_Jump = 1;
			else
				m_Input.m_Jump = 0;
		}
		else
			m_Input.m_Jump = 1;
	}

	// Limits
	int tx = m_Pos.x + m_BotDir * 45.0f;
	if (tx < 0)
		m_BotDir = 1;
	else if (tx >= GameServer()->Collision()->GetWidth() * 32.0f)
		m_BotDir = -1;

	// Delay of actions
	if (!PlayerClose)
		m_BotTimePlayerFound = Server()->Tick();

	// Set data
	m_Input.m_Direction = m_BotDir;
	m_Input.m_PlayerFlags = PLAYERFLAG_PLAYING;
	// Check for legacy input
	if (m_LatestPrevInput.m_Fire && m_Input.m_Fire)
		m_Input.m_Fire = 0;
	if (m_LatestInput.m_Jump && m_Input.m_Jump)
		m_Input.m_Jump = 0;
	// Ceck Double Jump
	if (m_Input.m_Jump && (m_Jumped & 1) && !(m_Jumped & 2) && m_Core.m_Vel.y < GameServer()->Tuning()->m_Gravity)
		m_Input.m_Jump = 0;

	m_LatestPrevInput = m_LatestInput;
	m_LatestInput = m_Input;
	m_BotLastPos = m_Pos;
}

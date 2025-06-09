#include "Glow.h"

#include "../Materials/Materials.h"
#include "../FakeAngle/FakeAngle.h"
#include "../../Backtrack/Backtrack.h"
#include "../../Players/PlayerUtils.h"

static inline bool GetPlayerGlow(CBaseEntity* pPlayer, CBaseEntity* pEntity, CTFPlayer* pLocal, Glow_t* pGlow, Color_t* pColor, bool bEnemy, bool bTeam)
{
	if (Vars::Glow::Player::Local.Value && pPlayer == pLocal
		|| Vars::Glow::Player::Priority.Value && F::PlayerUtils.IsPrioritized(pPlayer->entindex())
		|| Vars::Glow::Player::Friend.Value && H::Entities.IsFriend(pPlayer->entindex())
		|| Vars::Glow::Player::Party.Value && H::Entities.InParty(pPlayer->entindex())
		|| Vars::Glow::Player::Target.Value && pEntity->entindex() == G::AimTarget.m_iEntIndex)
	{
		*pGlow = Glow_t(Vars::Glow::Player::Stencil.Value, Vars::Glow::Player::Blur.Value);
		*pColor = H::Color.GetEntityDrawColor(pLocal, pPlayer, Vars::Colors::Relative.Value, pEntity);
		return true;
	}

	*pColor = H::Color.GetEntityDrawColor(pLocal, pPlayer, Vars::Colors::Relative.Value, pEntity);
	if (pEntity->m_iTeamNum() != pLocal->m_iTeamNum())
	{
		*pGlow = Glow_t(Vars::Glow::Enemy::Stencil.Value, Vars::Glow::Enemy::Blur.Value);
		return bEnemy;
	}
	else
	{
		*pGlow = Glow_t(Vars::Glow::Team::Stencil.Value, Vars::Glow::Team::Blur.Value);
		return bTeam;
	}
}

bool CGlow::GetGlow(CTFPlayer* pLocal, CBaseEntity* pEntity, Glow_t* pGlow, Color_t* pColor)
{
	if (pEntity->IsDormant() || !pEntity->ShouldDraw())
		return false;

	switch (pEntity->GetClassID())
	{
	// player glow
	case ETFClassID::CTFPlayer:
		return GetPlayerGlow(pEntity, pEntity, pLocal, pGlow, pColor, Vars::Glow::Enemy::Players.Value, Vars::Glow::Team::Players.Value);
	// building glow
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
	{
		auto pOwner = pEntity->As<CBaseObject>()->m_hBuilder().Get();
		if (!pOwner) pOwner = pEntity;

		return GetPlayerGlow(pOwner, pEntity, pLocal, pGlow, pColor, Vars::Glow::Enemy::Buildings.Value, Vars::Glow::Team::Buildings.Value);
	}
	// projectile glow
	case ETFClassID::CBaseProjectile:
	case ETFClassID::CBaseGrenade:
	case ETFClassID::CTFWeaponBaseGrenadeProj:
	case ETFClassID::CTFWeaponBaseMerasmusGrenade:
	case ETFClassID::CTFGrenadePipebombProjectile:
	case ETFClassID::CTFStunBall:
	case ETFClassID::CTFBall_Ornament:
	case ETFClassID::CTFProjectile_Jar:
	case ETFClassID::CTFProjectile_Cleaver:
	case ETFClassID::CTFProjectile_JarGas:
	case ETFClassID::CTFProjectile_JarMilk:
	case ETFClassID::CTFProjectile_SpellBats:
	case ETFClassID::CTFProjectile_SpellKartBats:
	case ETFClassID::CTFProjectile_SpellMeteorShower:
	case ETFClassID::CTFProjectile_SpellMirv:
	case ETFClassID::CTFProjectile_SpellPumpkin:
	case ETFClassID::CTFProjectile_SpellSpawnBoss:
	case ETFClassID::CTFProjectile_SpellSpawnHorde:
	case ETFClassID::CTFProjectile_SpellSpawnZombie:
	case ETFClassID::CTFProjectile_SpellTransposeTeleport:
	case ETFClassID::CTFProjectile_Throwable:
	case ETFClassID::CTFProjectile_ThrowableBreadMonster:
	case ETFClassID::CTFProjectile_ThrowableBrick:
	case ETFClassID::CTFProjectile_ThrowableRepel:
	{
		auto pOwner = pEntity->As<CTFWeaponBaseGrenadeProj>()->m_hThrower().Get();
		if (!pOwner) pOwner = pEntity;

		return GetPlayerGlow(pOwner, pEntity, pLocal, pGlow, pColor, Vars::Glow::Enemy::Projectiles.Value, Vars::Glow::Team::Projectiles.Value);
	}
	case ETFClassID::CTFBaseRocket:
	case ETFClassID::CTFFlameRocket:
	case ETFClassID::CTFProjectile_Arrow:
	case ETFClassID::CTFProjectile_GrapplingHook:
	case ETFClassID::CTFProjectile_HealingBolt:
	case ETFClassID::CTFProjectile_Rocket:
	//case ETFClassID::CTFProjectile_BallOfFire: // lifetime too short
	case ETFClassID::CTFProjectile_MechanicalArmOrb:
	case ETFClassID::CTFProjectile_SentryRocket:
	case ETFClassID::CTFProjectile_SpellFireball:
	case ETFClassID::CTFProjectile_SpellLightningOrb:
	case ETFClassID::CTFProjectile_SpellKartOrb:
	case ETFClassID::CTFProjectile_EnergyBall:
	case ETFClassID::CTFProjectile_Flare:
	{
		auto pWeapon = pEntity->As<CTFBaseRocket>()->m_hLauncher().Get();
		auto pOwner = pWeapon ? pWeapon->As<CTFWeaponBase>()->m_hOwner().Get() : pEntity;
		if (!pOwner) pOwner = pEntity;

		return GetPlayerGlow(pOwner, pEntity, pLocal, pGlow, pColor, Vars::Glow::Enemy::Projectiles.Value, Vars::Glow::Team::Projectiles.Value);
	}
	case ETFClassID::CTFBaseProjectile:
	case ETFClassID::CTFProjectile_EnergyRing: // not drawn, shoulddraw check, small anyways
	//case ETFClassID::CTFProjectile_Syringe: // not drawn
	{
		auto pWeapon = pEntity->As<CTFBaseProjectile>()->m_hLauncher().Get();
		auto pOwner = pWeapon ? pWeapon->As<CTFWeaponBase>()->m_hOwner().Get() : pEntity;
		if (!pOwner) pOwner = pEntity;

		return GetPlayerGlow(pOwner, pEntity, pLocal, pGlow, pColor, Vars::Glow::Enemy::Projectiles.Value, Vars::Glow::Team::Projectiles.Value);
	}
	// ragdoll glow
	case ETFClassID::CTFRagdoll:
	case ETFClassID::CRagdollProp:
	case ETFClassID::CRagdollPropAttached:
	{
		auto pOwner = pEntity->As<CTFRagdoll>()->m_hPlayer().Get();
		if (!pOwner) pOwner = pEntity;

		return GetPlayerGlow(pOwner, pEntity, pLocal, pGlow, pColor, Vars::Glow::Enemy::Ragdolls.Value, Vars::Glow::Team::Ragdolls.Value);
	}
	// objective glow
	case ETFClassID::CCaptureFlag:
		*pGlow = Glow_t(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value);
		*pColor = H::Color.GetEntityDrawColor(pLocal, pEntity, Vars::Colors::Relative.Value);
		return Vars::Glow::World::Objective.Value;
	// npc glow
	case ETFClassID::CEyeballBoss:
	case ETFClassID::CHeadlessHatman:
	case ETFClassID::CMerasmus:
	case ETFClassID::CTFBaseBoss:
	case ETFClassID::CTFTankBoss:
	case ETFClassID::CZombie:
		if (pEntity->IsInValidTeam())
		{
			if (auto pOwner = pEntity->m_hOwnerEntity().Get())
				return GetPlayerGlow(pOwner, pEntity, pLocal, pGlow, pColor, Vars::Glow::World::NPCs.Value, Vars::Glow::World::NPCs.Value);
			else
				*pColor = H::Color.GetEntityDrawColor(pLocal, pEntity, Vars::Colors::Relative.Value, pEntity);
		}
		else
			*pColor = Vars::Colors::NPC.Value;

		*pGlow = Glow_t(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value);
		return Vars::Glow::World::NPCs.Value;
	// pickup glow
	case ETFClassID::CTFAmmoPack:
		*pGlow = Glow_t(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value);
		*pColor = Vars::Colors::Ammo.Value;
		return Vars::Glow::World::Pickups.Value;
	case ETFClassID::CCurrencyPack:
		*pGlow = Glow_t(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value);
		*pColor = Vars::Colors::Money.Value;
		return Vars::Glow::World::Pickups.Value;
	case ETFClassID::CHalloweenGiftPickup:
		*pGlow = Glow_t(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value);
		*pColor = Vars::Colors::Halloween.Value;
		return Vars::Glow::World::Halloween.Value;
	case ETFClassID::CBaseAnimating:
	{
		if (H::Entities.IsAmmo(H::Entities.GetModel(pEntity->entindex())))
		{
			*pGlow = Glow_t(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value);
			*pColor = Vars::Colors::Ammo.Value;
			return Vars::Glow::World::Pickups.Value;
		}
		else if (H::Entities.IsHealth(H::Entities.GetModel(pEntity->entindex())))
		{
			*pGlow = Glow_t(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value);
			*pColor = Vars::Colors::Health.Value;
			return Vars::Glow::World::Pickups.Value;
		}
		else if (H::Entities.IsPowerup(H::Entities.GetModel(pEntity->entindex())))
		{
			*pGlow = Glow_t(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value);
			*pColor = Vars::Colors::Powerup.Value;
			return Vars::Glow::World::Powerups.Value;
		}
		else if (H::Entities.IsSpellbook(H::Entities.GetModel(pEntity->entindex())))
		{
			*pGlow = Glow_t(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value);
			*pColor = Vars::Colors::Halloween.Value;
			return Vars::Glow::World::Halloween.Value;
		}
		break;
	}
	// bomb glow
	case ETFClassID::CTFPumpkinBomb:
	case ETFClassID::CTFGenericBomb:
		*pGlow = Glow_t(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value);
		*pColor = Vars::Colors::Halloween.Value;
		return Vars::Glow::World::Bombs.Value;
	case ETFClassID::CTFMedigunShield:
		return false;
	}

	// player glow
	auto pOwner = pEntity->m_hOwnerEntity().Get();
	if (pOwner && pOwner->IsPlayer())
		return GetPlayerGlow(pOwner, pOwner, pLocal, pGlow, pColor, Vars::Glow::Enemy::Players.Value, Vars::Glow::Team::Players.Value);

	return false;
}

void CGlow::StencilBegin(IMatRenderContext* pRenderContext)
{
	pRenderContext->ClearBuffers(false, false, false);
	pRenderContext->SetStencilEnable(true);
	pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_ALWAYS);
	pRenderContext->SetStencilPassOperation(STENCILOPERATION_REPLACE);
	pRenderContext->SetStencilFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilZFailOperation(STENCILOPERATION_REPLACE);
	pRenderContext->SetStencilReferenceValue(1);
	pRenderContext->SetStencilWriteMask(0xFF);
	pRenderContext->SetStencilTestMask(0x0);
}
void CGlow::StencilPreDraw(IMatRenderContext* pRenderContext)
{
	pRenderContext->SetStencilEnable(true);
	pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_EQUAL);
	pRenderContext->SetStencilPassOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilZFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilReferenceValue(0);
	pRenderContext->SetStencilWriteMask(0x0);
	pRenderContext->SetStencilTestMask(0xFF);
}
void CGlow::StencilEnd(IMatRenderContext* pRenderContext)
{
	pRenderContext->SetStencilEnable(false);
}

void CGlow::SetupBegin(Glow_t glow, IMatRenderContext* pRenderContext, IMaterial* m_pMatGlowColor, IMaterial* m_pMatBlurY)
{
	bool bFound; auto $bloomamount = m_pMatBlurY->FindVar("$bloomamount", &bFound, false);
	if (bFound && $bloomamount)
		$bloomamount->SetFloatValue(glow.Blur);

	StencilBegin(pRenderContext);

	m_flSavedBlend = I::RenderView->GetBlend();
	I::RenderView->SetBlend(0.f);
	I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
	I::ModelRender->ForcedMaterialOverride(m_pMatGlowColor);
}
void CGlow::SetupMid(IMatRenderContext* pRenderContext, int w, int h)
{
	pRenderContext->PushRenderTargetAndViewport();
	pRenderContext->SetRenderTarget(m_pRenderBuffer1);
	pRenderContext->Viewport(0, 0, w, h);
	pRenderContext->ClearColor4ub(0, 0, 0, 0);
	pRenderContext->ClearBuffers(true, false, false);
}
void CGlow::SetupEnd(Glow_t glow, IMatRenderContext* pRenderContext, IMaterial* m_pMatBlurX, IMaterial* m_pMatBlurY, IMaterial* m_pMatHaloAddToScreen, int w, int h)
{
	StencilEnd(pRenderContext);
	pRenderContext->PopRenderTargetAndViewport();
	if (glow.Blur)
	{
		pRenderContext->PushRenderTargetAndViewport();
		{
			pRenderContext->Viewport(0, 0, w, h);
			pRenderContext->SetRenderTarget(m_pRenderBuffer2);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatBlurX, 0, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
			pRenderContext->SetRenderTarget(m_pRenderBuffer1);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatBlurY, 0, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		}
		pRenderContext->PopRenderTargetAndViewport();
	}

	StencilPreDraw(pRenderContext);
	if (glow.Stencil)
	{
		int side = float(glow.Stencil + 1) / 2;
		int corner = float(glow.Stencil) / 2;
		if (corner)
		{
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, -corner, -corner, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, corner, corner, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, corner, -corner, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, -corner, corner, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		}
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, -side, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, 0, -side, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, side, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, 0, side, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
	}
	if (glow.Blur)
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, 0, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);

	StencilEnd(pRenderContext);

	I::RenderView->SetBlend(m_flSavedBlend);
	I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
	I::ModelRender->ForcedMaterialOverride(nullptr);
}

void CGlow::DrawModel(CBaseEntity* pEntity)
{
	m_bRendering = true;

	/*
	if (pEntity->IsPlayer())
	{
		auto pPlayer = pEntity->As<CTFPlayer>();

		float flOldInvisibility = pPlayer->m_flInvisibility();
		pPlayer->m_flInvisibility() = 0.f;

		pEntity->DrawModel(STUDIO_RENDER | STUDIO_NOSHADOWS);
		
		pPlayer->m_flInvisibility() = flOldInvisibility;
	}
	else*/
		pEntity->DrawModel(STUDIO_RENDER | STUDIO_NOSHADOWS);

	m_bRendering = false;
}



void CGlow::Store(CTFPlayer* pLocal)
{
	m_mEntities.clear();
	if (!pLocal)
		return;

	for (int n = 1; n <= I::ClientEntityList->GetHighestEntityIndex(); n++)
	{
		auto pEntity = I::ClientEntityList->GetClientEntity(n)->As<CBaseEntity>();
		if (!pEntity)
			continue;

		Glow_t tGlow = {}; Color_t tColor = {};
		if (GetGlow(pLocal, pEntity, &tGlow, &tColor)
			&& SDK::IsOnScreen(pEntity, !H::Entities.IsProjectile(pEntity) /*&& pEntity->GetClassID() != ETFClassID::CTFMedigunShield*/))
			m_mEntities[tGlow].emplace_back(pEntity, tColor);

		if (pEntity->IsPlayer() && !pEntity->IsDormant())
		{
			// backtrack
			if (Vars::Glow::Backtrack::Enabled.Value && (Vars::Glow::Backtrack::Stencil.Value || Vars::Glow::Backtrack::Blur.Value) && pEntity != pLocal)
			{
				auto pWeapon = H::Entities.GetWeapon();
				if (pWeapon && (G::PrimaryWeaponType != EWeaponType::PROJECTILE || Vars::Glow::Backtrack::Draw.Value & Vars::Glow::Backtrack::DrawEnum::Always))
				{
					bool bShowFriendly = false, bShowEnemy = true;
					if (!(Vars::Glow::Backtrack::Draw.Value & Vars::Glow::Backtrack::DrawEnum::IgnoreTeam))
					{
						if (G::PrimaryWeaponType == EWeaponType::MELEE && SDK::AttribHookValue(0, "speed_buff_ally", pWeapon) > 0)
							bShowFriendly = true;
						else if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
							bShowFriendly = true, bShowEnemy = false;
					}
					else if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
						bShowEnemy = false;

					if (bShowEnemy && pEntity->m_iTeamNum() != pLocal->m_iTeamNum() || bShowFriendly && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
					{
						tGlow = Glow_t(Vars::Glow::Backtrack::Stencil.Value, Vars::Glow::Backtrack::Blur.Value);
						m_mEntities[tGlow].emplace_back(pEntity, Vars::Colors::Backtrack.Value.a ? Vars::Colors::Backtrack.Value : tColor, true);
					}
				}
			}

			// fakeangle
			if (Vars::Glow::FakeAngle::Enabled.Value && (Vars::Glow::FakeAngle::Stencil.Value || Vars::Glow::FakeAngle::Blur.Value) && pEntity == pLocal && F::FakeAngle.bDrawChams && F::FakeAngle.bBonesSetup)
			{
				tGlow = Glow_t(Vars::Glow::FakeAngle::Stencil.Value, Vars::Glow::FakeAngle::Blur.Value);
				m_mEntities[tGlow].emplace_back(pEntity, Vars::Colors::FakeAngle.Value.a ? Vars::Colors::FakeAngle.Value : tColor, true);
			}
		}
	}
}

void CGlow::RenderMain()
{
	const int w = H::Draw.m_nScreenW, h = H::Draw.m_nScreenH;
	if (w < 1 || h < 1 || w > 4096 || h > 2160)
		return;

	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext || !m_pMatGlowColor || !m_pMatBlurX || !m_pMatBlurY || !m_pMatHaloAddToScreen)
		return F::Materials.ReloadMaterials();

	for (auto& [tGlow, vInfo] : m_mEntities)
	{
		SetupBegin(tGlow, pRenderContext, m_pMatGlowColor, m_pMatBlurY);
		for (auto& tInfo : vInfo)
		{
			m_bExtra = tInfo.m_bExtra;
			DrawModel(tInfo.m_pEntity);
			m_bExtra = false;
		}

		SetupMid(pRenderContext, w, h);
		for (auto& tInfo : vInfo)
		{
			I::RenderView->SetColorModulation(tInfo.m_cColor.r / 255.f, tInfo.m_cColor.g / 255.f, tInfo.m_cColor.b / 255.f);
			I::RenderView->SetBlend(tInfo.m_cColor.a / 255.f);
			m_bExtra = tInfo.m_bExtra;
			DrawModel(tInfo.m_pEntity);
			m_bExtra = false;
		}

		SetupEnd(tGlow, pRenderContext, m_pMatBlurX, m_pMatBlurY, m_pMatHaloAddToScreen, w, h);
	}
}

void CGlow::RenderBacktrack(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo)
{
	static auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
	if (!ModelRender_DrawModelExecute)
		return;

	auto pEntity = I::ClientEntityList->GetClientEntity(pInfo.entity_index)->As<CTFPlayer>();
	if (!pEntity || !pEntity->IsPlayer())
		return;



	auto drawModel = [&](Vec3& vOrigin, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld, float flBlend)
		{
			if (!SDK::IsOnScreen(pEntity, vOrigin))
				return;

			//float flOriginalBlend = I::RenderView->GetBlend();
			//I::RenderView->SetBlend(flBlend * flOriginalBlend);
			ModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);
			//I::RenderView->SetBlend(flOriginalBlend);
		};

	std::vector<TickRecord*> vRecords = {};
	if (!F::Backtrack.GetRecords(pEntity, vRecords))
		return;
	vRecords = F::Backtrack.GetValidRecords(vRecords);
	if (!vRecords.size())
		return;

	bool bDrawLast = Vars::Glow::Backtrack::Draw.Value & Vars::Glow::Backtrack::DrawEnum::Last;
	bool bDrawFirst = Vars::Glow::Backtrack::Draw.Value & Vars::Glow::Backtrack::DrawEnum::First;

	if (!bDrawLast && !bDrawFirst)
	{
		for (auto pRecord : vRecords)
		{
			if (float flBlend = Math::RemapVal(pEntity->GetAbsOrigin().DistTo(pRecord->m_vOrigin), 1.f, 24.f, 0.f, 1.f))
				drawModel(pRecord->m_vOrigin, pState, pInfo, pRecord->m_BoneMatrix.m_aBones, flBlend);
		}
	}
	else
	{
		if (bDrawLast)
		{
			auto pRecord = vRecords.back();
			if (float flBlend = Math::RemapVal(pEntity->GetAbsOrigin().DistTo(pRecord->m_vOrigin), 1.f, 24.f, 0.f, 1.f))
				drawModel(pRecord->m_vOrigin, pState, pInfo, pRecord->m_BoneMatrix.m_aBones, flBlend);
		}
		if (bDrawFirst)
		{
			auto pRecord = vRecords.front();
			if (float flBlend = Math::RemapVal(pEntity->GetAbsOrigin().DistTo(pRecord->m_vOrigin), 1.f, 24.f, 0.f, 1.f))
				drawModel(pRecord->m_vOrigin, pState, pInfo, pRecord->m_BoneMatrix.m_aBones, flBlend);
		}
	}
}
void CGlow::RenderFakeAngle(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo)
{
	static auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
	if (!ModelRender_DrawModelExecute)
		return;



	ModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, F::FakeAngle.aBones);
}
void CGlow::RenderHandler(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (!m_bExtra)
	{
		static auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
		if (ModelRender_DrawModelExecute)
			ModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);
	}
	else
	{
		if (pInfo.entity_index != I::EngineClient->GetLocalPlayer())
			RenderBacktrack(pState, pInfo);
		else
			RenderFakeAngle(pState, pInfo);
	}
}

void CGlow::RenderViewmodel(void* ecx, int flags)
{
	if (!Vars::Glow::Viewmodel::Weapon.Value || !Vars::Glow::Viewmodel::Stencil.Value && !Vars::Glow::Viewmodel::Blur.Value)
		return;

	const int w = H::Draw.m_nScreenW, h = H::Draw.m_nScreenH;
	if (w < 1 || h < 1 || w > 4096 || h > 2160)
		return;

	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext || !m_pMatGlowColor || !m_pMatBlurX || !m_pMatBlurY || !m_pMatHaloAddToScreen)
		return F::Materials.ReloadMaterials();

	static auto CBaseAnimating_InternalDrawModel = U::Hooks.m_mHooks["CBaseAnimating_InternalDrawModel"];
	if (!CBaseAnimating_InternalDrawModel)
		return;



	pRenderContext->CullMode(MATERIAL_CULLMODE_CCW); // glow won't work properly with MATERIAL_CULLMODE_CW

	auto glow = Glow_t(Vars::Glow::Viewmodel::Stencil.Value, Vars::Glow::Viewmodel::Blur.Value);

	SetupBegin(glow, pRenderContext, m_pMatGlowColor, m_pMatBlurY);
	CBaseAnimating_InternalDrawModel->Call<int>(ecx, flags);

	SetupMid(pRenderContext, w, h);
	auto& color = Vars::Colors::Local.Value;
	I::RenderView->SetColorModulation(float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f);
	I::RenderView->SetBlend(float(color.a) / 255.f);
	CBaseAnimating_InternalDrawModel->Call<int>(ecx, flags);

	SetupEnd(glow, pRenderContext, m_pMatBlurX, m_pMatBlurY, m_pMatHaloAddToScreen, w, h);

	pRenderContext->CullMode(G::FlipViewmodels ? MATERIAL_CULLMODE_CW : MATERIAL_CULLMODE_CCW);
}
void CGlow::RenderViewmodel(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (!Vars::Glow::Viewmodel::Hands.Value || !Vars::Glow::Viewmodel::Stencil.Value && !Vars::Glow::Viewmodel::Blur.Value)
		return;

	const int w = H::Draw.m_nScreenW, h = H::Draw.m_nScreenH;
	if (w < 1 || h < 1 || w > 4096 || h > 2160)
		return;

	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext || !m_pMatGlowColor || !m_pMatBlurX || !m_pMatBlurY || !m_pMatHaloAddToScreen)
		return F::Materials.ReloadMaterials();

	static auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
	if (!ModelRender_DrawModelExecute)
		return;



	pRenderContext->CullMode(MATERIAL_CULLMODE_CCW); // glow won't work properly with MATERIAL_CULLMODE_CW

	auto glow = Glow_t(Vars::Glow::Viewmodel::Stencil.Value, Vars::Glow::Viewmodel::Blur.Value);

	SetupBegin(glow, pRenderContext, m_pMatGlowColor, m_pMatBlurY);
	ModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);

	SetupMid(pRenderContext, w, h);
	auto& color = Vars::Colors::Local.Value;
	I::RenderView->SetColorModulation(float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f);
	I::RenderView->SetBlend(float(color.a) / 255.f);
	ModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);

	SetupEnd(glow, pRenderContext, m_pMatBlurX, m_pMatBlurY, m_pMatHaloAddToScreen, w, h);

	pRenderContext->CullMode(G::FlipViewmodels ? MATERIAL_CULLMODE_CW : MATERIAL_CULLMODE_CCW);
}



void CGlow::Initialize()
{
	int nWidth, nHeight; I::MatSystemSurface->GetScreenSize(nWidth, nHeight);

	if (!m_pMatGlowColor)
	{
		m_pMatGlowColor = I::MaterialSystem->FindMaterial("dev/glow_color", TEXTURE_GROUP_OTHER);
		m_pMatGlowColor->IncrementReferenceCount();
		F::Materials.m_mMatList[m_pMatGlowColor] = true;
	}

	if (!m_pRenderBuffer1)
	{
		m_pRenderBuffer1 = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
			"RenderBuffer1",
			nWidth, nHeight,
			RT_SIZE_LITERAL,
			IMAGE_FORMAT_RGB888,
			MATERIAL_RT_DEPTH_SHARED,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA,
			CREATERENDERTARGETFLAGS_HDR
		);
		m_pRenderBuffer1->IncrementReferenceCount();
	}

	if (!m_pRenderBuffer2)
	{
		m_pRenderBuffer2 = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
			"RenderBuffer2",
			nWidth, nHeight,
			RT_SIZE_LITERAL,
			IMAGE_FORMAT_RGB888,
			MATERIAL_RT_DEPTH_SHARED,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA,
			CREATERENDERTARGETFLAGS_HDR
		);
		m_pRenderBuffer2->IncrementReferenceCount();
	}

	if (!m_pMatHaloAddToScreen)
	{
		KeyValues* kv = new KeyValues("UnlitGeneric");
		kv->SetString("$basetexture", "RenderBuffer1");
		kv->SetString("$additive", "1");
		m_pMatHaloAddToScreen = F::Materials.Create("MatHaloAddToScreen", kv);
	}

	if (!m_pMatBlurX)
	{
		KeyValues* kv = new KeyValues("BlurFilterX");
		kv->SetString("$basetexture", "RenderBuffer1");
		m_pMatBlurX = F::Materials.Create("MatBlurX", kv);
	}

	if (!m_pMatBlurY)
	{
		KeyValues* kv = new KeyValues("BlurFilterY");
		kv->SetString("$basetexture", "RenderBuffer2");
		m_pMatBlurY = F::Materials.Create("MatBlurY", kv);
	}
}

void CGlow::Unload()
{
	if (m_pMatGlowColor)
	{
		m_pMatGlowColor->DecrementReferenceCount();
		m_pMatGlowColor->DeleteIfUnreferenced();
		m_pMatGlowColor = nullptr;
	}

	if (m_pMatBlurX)
	{
		m_pMatBlurX->DecrementReferenceCount();
		m_pMatBlurX->DeleteIfUnreferenced();
		m_pMatBlurX = nullptr;
	}

	if (m_pMatBlurY)
	{
		m_pMatBlurY->DecrementReferenceCount();
		m_pMatBlurY->DeleteIfUnreferenced();
		m_pMatBlurY = nullptr;
	}

	if (m_pMatHaloAddToScreen)
	{
		m_pMatHaloAddToScreen->DecrementReferenceCount();
		m_pMatHaloAddToScreen->DeleteIfUnreferenced();
		m_pMatHaloAddToScreen = nullptr;
	}

	if (m_pRenderBuffer1)
	{
		m_pRenderBuffer1->DecrementReferenceCount();
		m_pRenderBuffer1->DeleteIfUnreferenced();
		m_pRenderBuffer1 = nullptr;
	}

	if (m_pRenderBuffer2)
	{
		m_pRenderBuffer2->DecrementReferenceCount();
		m_pRenderBuffer2->DeleteIfUnreferenced();
		m_pRenderBuffer2 = nullptr;
	}
}
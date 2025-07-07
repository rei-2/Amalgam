#include "Chams.h"

#include "../Materials/Materials.h"
#include "../FakeAngle/FakeAngle.h"
#include "../../Backtrack/Backtrack.h"
#include "../../Players/PlayerUtils.h"
#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"

static inline bool GetPlayerChams(CBaseEntity* pPlayer, CBaseEntity* pEntity, CTFPlayer* pLocal, Chams_t* pChams, bool bEnemy, bool bTeam)
{
	if (Vars::Chams::Player::Local.Value && pPlayer == pLocal
		|| Vars::Chams::Player::Priority.Value && F::PlayerUtils.IsPrioritized(pPlayer->entindex())
		|| Vars::Chams::Player::Friend.Value && H::Entities.IsFriend(pPlayer->entindex())
		|| Vars::Chams::Player::Party.Value && H::Entities.InParty(pPlayer->entindex())
		|| Vars::Chams::Player::Target.Value && pEntity->entindex() == G::AimTarget.m_iEntIndex)
	{
		*pChams = Chams_t(Vars::Chams::Player::Visible.Value, Vars::Chams::Player::Occluded.Value);
		return true;
	}

	if (!Vars::Chams::Relative.Value)
	{
		if (pEntity->m_iTeamNum() != pLocal->m_iTeamNum() ? !Vars::Chams::EnemyChams.Value : !Vars::Chams::TeamChams.Value)
			return false;

		switch (pEntity->m_iTeamNum())
		{
		case TF_TEAM_BLUE:
			*pChams = Chams_t(Vars::Chams::Enemy::Visible.Value, Vars::Chams::Enemy::Occluded.Value);
			return bEnemy;
		case TF_TEAM_RED:
			*pChams = Chams_t(Vars::Chams::Team::Visible.Value, Vars::Chams::Team::Occluded.Value);
			return bTeam;
		}
	}
	else
	{
		if (pEntity->m_iTeamNum() != pLocal->m_iTeamNum())
		{
			*pChams = Chams_t(Vars::Chams::Enemy::Visible.Value, Vars::Chams::Enemy::Occluded.Value);
			return bEnemy;
		}
		else
		{
			*pChams = Chams_t(Vars::Chams::Team::Visible.Value, Vars::Chams::Team::Occluded.Value);
			return bTeam;
		}
	}
	return false;
}

bool CChams::GetChams(CTFPlayer* pLocal, CBaseEntity* pEntity, Chams_t* pChams)
{
	if (pEntity->IsDormant() || !pEntity->ShouldDraw())
		return false;

	switch (pEntity->GetClassID())
	{
	// player chams
	case ETFClassID::CTFPlayer:
		return GetPlayerChams(pEntity, pEntity, pLocal, pChams, Vars::Chams::Enemy::Players.Value, Vars::Chams::Team::Players.Value);
	// building chams
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
	{
		auto pOwner = pEntity->As<CBaseObject>()->m_hBuilder().Get();
		if (!pOwner) pOwner = pEntity;
		return GetPlayerChams(pOwner, pEntity, pLocal, pChams, Vars::Chams::Enemy::Buildings.Value, Vars::Chams::Team::Buildings.Value);
	}
	// projectile chams
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
	case ETFClassID::CTFBaseRocket:
	case ETFClassID::CTFFlameRocket:
	case ETFClassID::CTFProjectile_Arrow:
	case ETFClassID::CTFProjectile_GrapplingHook:
	case ETFClassID::CTFProjectile_HealingBolt:
	case ETFClassID::CTFProjectile_Rocket:
	case ETFClassID::CTFProjectile_BallOfFire:
	case ETFClassID::CTFProjectile_MechanicalArmOrb:
	case ETFClassID::CTFProjectile_SentryRocket:
	case ETFClassID::CTFProjectile_SpellFireball:
	case ETFClassID::CTFProjectile_SpellLightningOrb:
	case ETFClassID::CTFProjectile_SpellKartOrb:
	case ETFClassID::CTFProjectile_EnergyBall:
	case ETFClassID::CTFProjectile_Flare:
	case ETFClassID::CTFBaseProjectile:
	case ETFClassID::CTFProjectile_EnergyRing:
	//case ETFClassID::CTFProjectile_Syringe:
	{
		auto pOwner = F::ProjSim.GetEntities(pEntity).second->As<CBaseEntity>();
		if (!pOwner) pOwner = pEntity;
		return GetPlayerChams(pOwner, pEntity, pLocal, pChams, Vars::Chams::Enemy::Projectiles.Value, Vars::Chams::Team::Projectiles.Value);
	}
	// ragdoll chams
	case ETFClassID::CTFRagdoll:
	case ETFClassID::CRagdollProp:
	case ETFClassID::CRagdollPropAttached:
	{
		auto pOwner = pEntity->As<CTFRagdoll>()->m_hPlayer().Get();
		if (!pOwner) pOwner = pEntity;
		return GetPlayerChams(pOwner, pEntity, pLocal, pChams, Vars::Chams::Enemy::Ragdolls.Value, Vars::Chams::Team::Ragdolls.Value);
	}
	// objective chams
	case ETFClassID::CCaptureFlag:
		*pChams = Chams_t(Vars::Chams::World::Visible.Value, Vars::Chams::World::Occluded.Value);
		return Vars::Chams::World::Objective.Value;
	// npc chams
	case ETFClassID::CEyeballBoss:
	case ETFClassID::CHeadlessHatman:
	case ETFClassID::CMerasmus:
	case ETFClassID::CTFBaseBoss:
	case ETFClassID::CTFTankBoss:
	case ETFClassID::CZombie:
		*pChams = Chams_t(Vars::Chams::World::Visible.Value, Vars::Chams::World::Occluded.Value);
		return Vars::Chams::World::NPCs.Value;
	// pickup chams
	case ETFClassID::CTFAmmoPack:
	case ETFClassID::CCurrencyPack:
	case ETFClassID::CHalloweenGiftPickup:
		*pChams = Chams_t(Vars::Chams::World::Visible.Value, Vars::Chams::World::Occluded.Value);
		return Vars::Chams::World::Pickups.Value;
	case ETFClassID::CBaseAnimating:
	{
		if (H::Entities.IsAmmo(H::Entities.GetModel(pEntity->entindex())) || H::Entities.IsHealth(H::Entities.GetModel(pEntity->entindex())))
		{
			*pChams = Chams_t(Vars::Chams::World::Visible.Value, Vars::Chams::World::Occluded.Value);
			return Vars::Chams::World::Pickups.Value;
		}
		else if (H::Entities.IsPowerup(H::Entities.GetModel(pEntity->entindex())))
		{
			*pChams = Chams_t(Vars::Chams::World::Visible.Value, Vars::Chams::World::Occluded.Value);
			return Vars::Chams::World::Powerups.Value;
		}
		else if (H::Entities.IsSpellbook(H::Entities.GetModel(pEntity->entindex())))
		{
			*pChams = Chams_t(Vars::Chams::World::Visible.Value, Vars::Chams::World::Occluded.Value);
			return Vars::Chams::World::Halloween.Value;
		}
		break;
	}
	// bomb chams
	case ETFClassID::CTFPumpkinBomb:
	case ETFClassID::CTFGenericBomb:
		*pChams = Chams_t(Vars::Chams::World::Visible.Value, Vars::Chams::World::Occluded.Value);
		return Vars::Chams::World::Bombs.Value;
	case ETFClassID::CTFMedigunShield:
		return false;
	}

	// player chams
	auto pOwner = pEntity->m_hOwnerEntity().Get();
	if (pOwner && pOwner->IsPlayer())
		return GetPlayerChams(pOwner, pOwner, pLocal, pChams, Vars::Chams::Enemy::Players.Value, Vars::Chams::Team::Players.Value);

	return false;
}

void CChams::StencilBegin(IMatRenderContext* pRenderContext, bool bTwoModels)
{
	if (!bTwoModels)
		return;
	
	pRenderContext->SetStencilEnable(true);
}
void CChams::StencilVisible(IMatRenderContext* pRenderContext, bool bTwoModels)
{
	if (!bTwoModels)
		return;

	pRenderContext->ClearBuffers(false, false, false);
	pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_ALWAYS);
	pRenderContext->SetStencilPassOperation(STENCILOPERATION_REPLACE);
	pRenderContext->SetStencilFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilZFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilReferenceValue(1);
	pRenderContext->SetStencilWriteMask(0xFF);
	pRenderContext->SetStencilTestMask(0x0);
}
void CChams::StencilOccluded(IMatRenderContext* pRenderContext)
{
	pRenderContext->ClearBuffers(false, false, false);
	pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_EQUAL);
	pRenderContext->SetStencilPassOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilZFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilReferenceValue(0);
	pRenderContext->SetStencilWriteMask(0x0);
	pRenderContext->SetStencilTestMask(0xFF);
	pRenderContext->DepthRange(0.f, 0.2f);
}
void CChams::StencilEnd(IMatRenderContext* pRenderContext, bool bTwoModels)
{
	if (!bTwoModels)
		return;

	pRenderContext->SetStencilEnable(false);
	pRenderContext->DepthRange(0.f, 1.f);
}

void CChams::DrawModel(CBaseEntity* pEntity, Chams_t& tChams, IMatRenderContext* pRenderContext, bool bExtra)
{
	auto vVisibleMaterials = tChams.Visible.size() ? tChams.Visible : std::vector<std::pair<std::string, Color_t>> { { "None", {} } };
	auto vOccludedMaterials = tChams.Occluded.size() ? tChams.Occluded : std::vector<std::pair<std::string, Color_t>> { { "None", {} } };

	StencilBegin(pRenderContext, !bExtra);

	StencilVisible(pRenderContext, !bExtra);
	for (auto& [sName, tColor] : vVisibleMaterials)
	{
		auto pMaterial = F::Materials.GetMaterial(FNV1A::Hash32(sName.c_str()));

		F::Materials.SetColor(pMaterial, tColor);
		I::ModelRender->ForcedMaterialOverride(pMaterial ? pMaterial->m_pMaterial : nullptr);

		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(MATERIAL_CULLMODE_CW);

		m_bRendering = true;
		pEntity->DrawModel(STUDIO_RENDER);
		m_bRendering = false;

		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(MATERIAL_CULLMODE_CCW);
	}
	if (!bExtra)
	{
		StencilOccluded(pRenderContext);
		for (auto& [sName, tColor] : vOccludedMaterials)
		{
			auto pMaterial = F::Materials.GetMaterial(FNV1A::Hash32(sName.c_str()));

			F::Materials.SetColor(pMaterial, tColor);
			I::ModelRender->ForcedMaterialOverride(pMaterial ? pMaterial->m_pMaterial : nullptr);

			if (pMaterial && pMaterial->m_bInvertCull)
				pRenderContext->CullMode(MATERIAL_CULLMODE_CW);

			m_bRendering = true;
			pEntity->DrawModel(STUDIO_RENDER);
			m_bRendering = false;

			if (pMaterial && pMaterial->m_bInvertCull)
				pRenderContext->CullMode(MATERIAL_CULLMODE_CCW);
		}
	}

	StencilEnd(pRenderContext, !bExtra);
	I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
	I::RenderView->SetBlend(1.f);
	I::ModelRender->ForcedMaterialOverride(nullptr);

	if (!bExtra)
		m_mEntities[pEntity->entindex()] = true;
}



void CChams::Store(CTFPlayer* pLocal)
{
	m_vEntities.clear();
	if (!pLocal)
		return;

	for (int n = 1; n <= I::ClientEntityList->GetHighestEntityIndex(); n++)
	{
		auto pEntity = I::ClientEntityList->GetClientEntity(n)->As<CBaseEntity>();
		if (!pEntity)
			continue;

		Chams_t tChams = {};
		if (GetChams(pLocal, pEntity, &tChams)
			&& SDK::IsOnScreen(pEntity, !pEntity->IsProjectile() /*&& pEntity->GetClassID() != ETFClassID::CTFMedigunShield*/))
			m_vEntities.emplace_back(pEntity, tChams);

		if (pEntity->IsPlayer() && !pEntity->IsDormant())
		{
			// backtrack
			if (Vars::Chams::Backtrack::Enabled.Value && pEntity != pLocal
				&& (F::Backtrack.GetFakeLatency() || F::Backtrack.GetFakeInterp() > G::Lerp || F::Backtrack.GetWindow()))
			{
				auto pWeapon = H::Entities.GetWeapon();
				if (pWeapon && (G::PrimaryWeaponType != EWeaponType::PROJECTILE || Vars::Chams::Backtrack::Draw.Value & Vars::Chams::Backtrack::DrawEnum::Always))
				{
					bool bShowFriendly = false, bShowEnemy = true;
					if (!(Vars::Chams::Backtrack::Draw.Value & Vars::Chams::Backtrack::DrawEnum::IgnoreTeam))
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
						tChams = Chams_t(Vars::Chams::Backtrack::Visible.Value, {});
						m_vEntities.emplace_back(pEntity, tChams, true);
					}
				}
			}

			// fakeangle
			if (Vars::Chams::FakeAngle::Enabled.Value && pEntity == pLocal && F::FakeAngle.bDrawChams && F::FakeAngle.bBonesSetup)
			{
				tChams = Chams_t(Vars::Chams::FakeAngle::Visible.Value, {});
				m_vEntities.emplace_back(pEntity, tChams, true);
			}
		}
	}
}

void CChams::RenderMain()
{
	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext)
		return;

	for (auto& tInfo : m_vEntities)
	{
		if (!tInfo.m_bExtra)
			DrawModel(tInfo.m_pEntity, tInfo.m_tChams, pRenderContext, tInfo.m_bExtra);
		else
		{
			m_bExtra = true;

			//auto pPlayer = tInfo.m_pEntity->As<CTFPlayer>();
			//const float flOldInvisibility = pPlayer->m_flInvisibility();
			//pPlayer->m_flInvisibility() = 0.f;
			DrawModel(tInfo.m_pEntity, tInfo.m_tChams, pRenderContext, true);
			//pPlayer->m_flInvisibility() = flOldInvisibility;

			m_bExtra = false;
		}
	}
}

void CChams::RenderBacktrack(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo)
{
	static auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!ModelRender_DrawModelExecute || !pRenderContext)
		return;

	auto pEntity = I::ClientEntityList->GetClientEntity(pInfo.entity_index)->As<CTFPlayer>();
	if (!pEntity || !pEntity->IsPlayer())
		return;



	pRenderContext->DepthRange(0.f, Vars::Chams::Backtrack::IgnoreZ.Value ? 0.2f : 1.f);

	auto drawModel = [&](Vec3& vOrigin, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld, float flBlend)
		{
			if (!SDK::IsOnScreen(pEntity, vOrigin))
				return;

			float flOriginalBlend = I::RenderView->GetBlend();
			I::RenderView->SetBlend(flBlend * flOriginalBlend);
			ModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);
			I::RenderView->SetBlend(flOriginalBlend);
		};

	std::vector<TickRecord*> vRecords = {};
	if (!F::Backtrack.GetRecords(pEntity, vRecords))
		return;
	vRecords = F::Backtrack.GetValidRecords(vRecords);
	if (!vRecords.size())
		return;

	bool bDrawLast = Vars::Chams::Backtrack::Draw.Value & Vars::Chams::Backtrack::DrawEnum::Last;
	bool bDrawFirst = Vars::Chams::Backtrack::Draw.Value & Vars::Chams::Backtrack::DrawEnum::First;

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

	pRenderContext->DepthRange(0.f, 1.f);
}
void CChams::RenderFakeAngle(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo)
{
	static auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!ModelRender_DrawModelExecute || !pRenderContext)
		return;



	pRenderContext->DepthRange(0.f, Vars::Chams::FakeAngle::IgnoreZ.Value ? 0.2f : 1.f);

	ModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, F::FakeAngle.aBones);

	pRenderContext->DepthRange(0.f, 1.f);
}
void CChams::RenderHandler(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
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

bool CChams::RenderViewmodel(void* ecx, int flags, int* iReturn)
{
	if (!Vars::Chams::Viewmodel::Weapon.Value)
		return false;

	static auto CBaseAnimating_InternalDrawModel = U::Hooks.m_mHooks["CBaseAnimating_InternalDrawModel"];
	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!CBaseAnimating_InternalDrawModel || !pRenderContext)
		return false;



	auto& vMaterials = Vars::Chams::Viewmodel::WeaponMaterial.Value;

	for (auto& [sName, tColor] : vMaterials)
	{
		auto pMaterial = F::Materials.GetMaterial(FNV1A::Hash32(sName.c_str()));

		F::Materials.SetColor(pMaterial, tColor);
		I::ModelRender->ForcedMaterialOverride(pMaterial ? pMaterial->m_pMaterial : nullptr);

		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(G::FlipViewmodels ? MATERIAL_CULLMODE_CCW : MATERIAL_CULLMODE_CW);

		*iReturn = CBaseAnimating_InternalDrawModel->Call<int>(ecx, flags);

		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(G::FlipViewmodels ? MATERIAL_CULLMODE_CW : MATERIAL_CULLMODE_CCW);
	}

	I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
	I::RenderView->SetBlend(1.f);
	I::ModelRender->ForcedMaterialOverride(nullptr);

	return true;
}
bool CChams::RenderViewmodel(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (!Vars::Chams::Viewmodel::Hands.Value)
		return false;

	static auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!ModelRender_DrawModelExecute || !pRenderContext)
		return false;



	auto& vMaterials = Vars::Chams::Viewmodel::HandsMaterial.Value;

	for (auto& [sName, tColor] : vMaterials)
	{
		auto pMaterial = F::Materials.GetMaterial(FNV1A::Hash32(sName.c_str()));

		F::Materials.SetColor(pMaterial, tColor);
		I::ModelRender->ForcedMaterialOverride(pMaterial ? pMaterial->m_pMaterial : nullptr);

		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(G::FlipViewmodels ? MATERIAL_CULLMODE_CCW : MATERIAL_CULLMODE_CW);

		ModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);

		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(G::FlipViewmodels ? MATERIAL_CULLMODE_CW : MATERIAL_CULLMODE_CCW);
	}

	I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
	I::RenderView->SetBlend(1.f);
	I::ModelRender->ForcedMaterialOverride(nullptr);

	return true;
}
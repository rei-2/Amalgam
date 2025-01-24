#include "Chams.h"

#include "../Materials/Materials.h"
#include "../FakeAngle/FakeAngle.h"
#include "../../Backtrack/Backtrack.h"
#include "../../Players/PlayerUtils.h"

static inline bool GetPlayerChams(CBaseEntity* pEntity, CTFPlayer* pLocal, Chams_t* pChams, bool bFriendly, bool bEnemy)
{
	if (Vars::Chams::Player::Local.Value && pEntity == pLocal
		|| Vars::Chams::Player::Priority.Value && F::PlayerUtils.IsPrioritized(pEntity->entindex())
		|| Vars::Chams::Player::Friend.Value && H::Entities.IsFriend(pEntity->entindex())
		|| Vars::Chams::Player::Party.Value && H::Entities.InParty(pEntity->entindex())
		|| Vars::Chams::Player::Target.Value && pEntity->entindex() == G::Target.first)
	{
		*pChams = Chams_t(Vars::Chams::Player::Visible.Value, Vars::Chams::Player::Occluded.Value);
		return true;
	}

	const bool bTeam = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();
	*pChams = bTeam
		? Chams_t(Vars::Chams::Friendly::Visible.Value, Vars::Chams::Friendly::Occluded.Value)
		: Chams_t(Vars::Chams::Enemy::Visible.Value, Vars::Chams::Enemy::Occluded.Value);
	return bTeam ? bFriendly : bEnemy;
}

bool CChams::GetChams(CTFPlayer* pLocal, CBaseEntity* pEntity, Chams_t* pChams)
{
	if (pEntity->IsDormant() || !pEntity->ShouldDraw())
		return false;

	switch (pEntity->GetClassID())
	{
	// player chams
	case ETFClassID::CTFPlayer:
		return GetPlayerChams(pEntity, pLocal, pChams, Vars::Chams::Friendly::Players.Value, Vars::Chams::Enemy::Players.Value);
	// building chams
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
	{
		auto pOwner = pEntity->As<CBaseObject>()->m_hBuilder().Get();
		if (!pOwner) pOwner = pEntity;

		return GetPlayerChams(pOwner, pLocal, pChams, Vars::Chams::Friendly::Buildings.Value, Vars::Chams::Enemy::Buildings.Value);
	}
	// ragdoll chams
	case ETFClassID::CTFRagdoll:
	case ETFClassID::CRagdollProp:
	case ETFClassID::CRagdollPropAttached:
	{
		/*
		// don't interfere with ragdolls
		if (Vars::Visuals::Ragdolls::Type.Value)
		{
			if (Vars::Visuals::Ragdolls::EnemyOnly.Value && pEntity && pLocal && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
				return false;
			else
				return false;
		}
		*/
		auto pOwner = pEntity->As<CTFRagdoll>()->m_hPlayer().Get();
		if (!pOwner) pOwner = pEntity;

		return GetPlayerChams(pOwner, pLocal, pChams, Vars::Chams::Friendly::Ragdolls.Value, Vars::Chams::Enemy::Ragdolls.Value);
	}
	// projectile chams
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

		return GetPlayerChams(pOwner, pLocal, pChams, Vars::Chams::Friendly::Projectiles.Value, Vars::Chams::Enemy::Projectiles.Value);
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

		return GetPlayerChams(pOwner, pLocal, pChams, Vars::Chams::Friendly::Projectiles.Value, Vars::Chams::Enemy::Projectiles.Value);
	}
	case ETFClassID::CTFBaseProjectile:
	case ETFClassID::CTFProjectile_EnergyRing: // not drawn, shoulddraw check, small anyways
	//case ETFClassID::CTFProjectile_Syringe: // not drawn
	{
		auto pWeapon = pEntity->As<CTFBaseProjectile>()->m_hLauncher().Get();
		auto pOwner = pWeapon ? pWeapon->As<CTFWeaponBase>()->m_hOwner().Get() : pEntity;
		if (!pOwner) pOwner = pEntity;

		return GetPlayerChams(pOwner, pLocal, pChams, Vars::Chams::Friendly::Projectiles.Value, Vars::Chams::Enemy::Projectiles.Value);
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
	}

	// player chams
	auto pOwner = pEntity->m_hOwnerEntity().Get();
	if (pOwner && pOwner->IsPlayer())
		return GetPlayerChams(pOwner, pLocal, pChams, Vars::Chams::Friendly::Players.Value, Vars::Chams::Enemy::Players.Value);

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

void CChams::DrawModel(CBaseEntity* pEntity, Chams_t chams, IMatRenderContext* pRenderContext, bool bExtra)
{
	auto visibleMaterials = chams.Visible.size() ? chams.Visible : std::vector<std::pair<std::string, Color_t>> { { "None", {} } };
	auto occludedMaterials = chams.Occluded.size() ? chams.Occluded : std::vector<std::pair<std::string, Color_t>> { { "None", {} } };

	StencilBegin(pRenderContext, !bExtra);

	StencilVisible(pRenderContext, !bExtra);
	for (auto it = visibleMaterials.begin(); it != visibleMaterials.end(); it++)
	{
		auto pMaterial = F::Materials.GetMaterial(FNV1A::Hash32(it->first.c_str()));

		F::Materials.SetColor(pMaterial, it->second);
		I::ModelRender->ForcedMaterialOverride(pMaterial ? pMaterial->m_pMaterial : nullptr);

		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(MATERIAL_CULLMODE_CW);

		bRendering = true;
		pEntity->DrawModel(STUDIO_RENDER);
		bRendering = false;

		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(MATERIAL_CULLMODE_CCW);
	}
	if (!bExtra)
	{
		StencilOccluded(pRenderContext);
		for (auto it = occludedMaterials.begin(); it != occludedMaterials.end(); it++)
		{
			auto pMaterial = F::Materials.GetMaterial(FNV1A::Hash32(it->first.c_str()));

			F::Materials.SetColor(pMaterial, it->second);
			I::ModelRender->ForcedMaterialOverride(pMaterial ? pMaterial->m_pMaterial : nullptr);

			if (pMaterial && pMaterial->m_bInvertCull)
				pRenderContext->CullMode(MATERIAL_CULLMODE_CW);

			bRendering = true;
			pEntity->DrawModel(STUDIO_RENDER);
			bRendering = false;

			if (pMaterial && pMaterial->m_bInvertCull)
				pRenderContext->CullMode(MATERIAL_CULLMODE_CCW);
		}
	}

	StencilEnd(pRenderContext, !bExtra);
	I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
	I::RenderView->SetBlend(1.f);
	I::ModelRender->ForcedMaterialOverride(nullptr);

	if (!bExtra)
		mEntities[pEntity->entindex()] = true;
}



void CChams::Store(CTFPlayer* pLocal)
{
	vEntities.clear();
	if (!pLocal)
		return;

	for (int n = 1; n <= I::ClientEntityList->GetHighestEntityIndex(); n++)
	{
		auto pEntity = I::ClientEntityList->GetClientEntity(n)->As<CBaseEntity>();
		if (!pEntity)
			continue;

		Chams_t tChams = {};
		if (GetChams(pLocal, pEntity, &tChams) && SDK::IsOnScreen(pEntity))
			vEntities.push_back({ pEntity, tChams });

		if (pEntity->IsPlayer() && !pEntity->IsDormant())
		{
			// backtrack
			if (Vars::Backtrack::Enabled.Value && Vars::Chams::Backtrack::Enabled.Value && pEntity != pLocal)
			{
				auto pWeapon = H::Entities.GetWeapon();
				if (pWeapon && G::PrimaryWeaponType != EWeaponType::PROJECTILE)
				{
					bool bShowFriendly = false, bShowEnemy = true;
					if (pWeapon->m_iItemDefinitionIndex() == Soldier_t_TheDisciplinaryAction)
						bShowFriendly = true;
					if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
						bShowFriendly = true, bShowEnemy = false;

					if (bShowFriendly && pEntity->m_iTeamNum() == pLocal->m_iTeamNum() || bShowEnemy && pEntity->m_iTeamNum() != pLocal->m_iTeamNum())
					{
						tChams = Chams_t(Vars::Chams::Backtrack::Visible.Value, {});
						vEntities.push_back({ pEntity, tChams, true });
					}
				}
			}

			// fakeangle
			if (Vars::Chams::FakeAngle::Enabled.Value && pEntity == pLocal && F::FakeAngle.bDrawChams && F::FakeAngle.bBonesSetup)
			{
				tChams = Chams_t(Vars::Chams::FakeAngle::Visible.Value, {});
				vEntities.push_back({ pEntity, tChams, true });
			}
		}
	}
}

void CChams::RenderMain()
{
	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext)
		return;

	for (auto& tInfo : vEntities)
	{
		if (!tInfo.m_bExtra)
			DrawModel(tInfo.m_pEntity, tInfo.m_tChams, pRenderContext, tInfo.m_bExtra);
		else
		{
			bExtra = true;

			auto pPlayer = tInfo.m_pEntity->As<CTFPlayer>();

			const float flOldInvisibility = pPlayer->m_flInvisibility();
			pPlayer->m_flInvisibility() = 0.f;

			DrawModel(tInfo.m_pEntity, tInfo.m_tChams, pRenderContext, true);

			pPlayer->m_flInvisibility() = flOldInvisibility;

			bExtra = false;
		}
	}
}

void CChams::RenderBacktrack(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo)
{
	if (!Vars::Backtrack::Enabled.Value || !Vars::Chams::Backtrack::Enabled.Value)
		return;

	static auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
	if (!ModelRender_DrawModelExecute)
		return;

	auto pEntity = I::ClientEntityList->GetClientEntity(pInfo.entity_index)->As<CTFPlayer>();
	if (!pEntity || !pEntity->IsPlayer())
		return;



	auto drawModel = [&](Vec3& vOrigin, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
		{
			if (!SDK::IsOnScreen(pEntity, vOrigin))
				return;

			ModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);
		};

	auto pRecords = F::Backtrack.GetRecords(pEntity);
	auto vRecords = F::Backtrack.GetValidRecords(pRecords);
	if (!vRecords.size())
		return;

	switch (Vars::Chams::Backtrack::Draw.Value)
	{
	case Vars::Chams::Backtrack::DrawEnum::Last: // last
	{
		auto vLastRec = vRecords.end() - 1;
		if (vLastRec != vRecords.end() && pEntity->GetAbsOrigin().DistTo(vLastRec->m_vOrigin) > 0.1f)
			drawModel(vLastRec->m_vOrigin, pState, pInfo, vLastRec->m_BoneMatrix.m_aBones);
		break;
	}
	case Vars::Chams::Backtrack::DrawEnum::LastFirst: // last + first
	{
		auto vFirstRec = vRecords.begin();
		if (vFirstRec != vRecords.end() && pEntity->GetAbsOrigin().DistTo(vFirstRec->m_vOrigin) > 0.1f)
			drawModel(vFirstRec->m_vOrigin, pState, pInfo, vFirstRec->m_BoneMatrix.m_aBones);
		auto vLastRec = vRecords.end() - 1;
		if (vLastRec != vRecords.end() && pEntity->GetAbsOrigin().DistTo(vLastRec->m_vOrigin) > 0.1f)
			drawModel(vLastRec->m_vOrigin, pState, pInfo, vLastRec->m_BoneMatrix.m_aBones);
		break;
	}
	case Vars::Chams::Backtrack::DrawEnum::All: // all
	{
		for (auto& record : vRecords)
		{
			if (pEntity->GetAbsOrigin().DistTo(record.m_vOrigin) < 0.1f)
				continue;

			drawModel(record.m_vOrigin, pState, pInfo, record.m_BoneMatrix.m_aBones);
		}
	}
	}
}
void CChams::RenderFakeAngle(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo)
{
	if (!Vars::Chams::FakeAngle::Enabled.Value || pInfo.entity_index != I::EngineClient->GetLocalPlayer() || !F::FakeAngle.bDrawChams || !F::FakeAngle.bBonesSetup)
		return;

	static auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
	if (!ModelRender_DrawModelExecute)
		return;



	ModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, F::FakeAngle.aBones);
}
void CChams::RenderHandler(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (!bExtra)
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



	auto& vMaterials = Vars::Chams::Viewmodel::WeaponVisible.Value;

	for (auto it = vMaterials.begin(); it != vMaterials.end(); it++)
	{
		auto pMaterial = F::Materials.GetMaterial(FNV1A::Hash32(it->first.c_str()));

		F::Materials.SetColor(pMaterial, it->second);
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



	auto& vMaterials = Vars::Chams::Viewmodel::HandsVisible.Value;

	for (auto it = vMaterials.begin(); it != vMaterials.end(); it++)
	{
		auto pMaterial = F::Materials.GetMaterial(FNV1A::Hash32(it->first.c_str()));

		F::Materials.SetColor(pMaterial, it->second);
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
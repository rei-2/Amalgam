#include "Chams.h"

#include "../Materials/Materials.h"
#include "../FakeAngle/FakeAngle.h"
#include "../../Backtrack/Backtrack.h"
#include "../../Players/PlayerUtils.h"

Chams_t CChams::GetStruct(std::vector<std::string> VisibleMaterial, std::vector<std::string> OccludedMaterial, Color_t VisibleColor, Color_t OccludedColor)
{
	return Chams_t{
		VisibleMaterial,
		OccludedMaterial,
		VisibleColor,
		OccludedColor
	};
}

bool CChams::GetPlayerChams(CTFPlayer* pEntity, CTFPlayer* pLocal, Chams_t* pChams, bool bFriendly, bool bEnemy)
{
	if (Vars::Chams::Player::Local.Value && pEntity == pLocal
		|| Vars::Chams::Player::Friend.Value && H::Entities.IsFriend(pEntity->entindex())
		|| Vars::Chams::Player::Priority.Value && F::PlayerUtils.GetPriority(pEntity->entindex()) > F::PlayerUtils.mTags["Default"].Priority)
	{
		*pChams = GetStruct(Vars::Chams::Player::VisibleMaterial.Value, Vars::Chams::Player::OccludedMaterial.Value, Vars::Chams::Player::VisibleColor.Value, Vars::Chams::Player::OccludedColor.Value);
		return true;
	}

	const bool bTeam = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();
	*pChams = bTeam
		? GetStruct(Vars::Chams::Friendly::VisibleMaterial.Value, Vars::Chams::Friendly::OccludedMaterial.Value, Vars::Chams::Friendly::VisibleColor.Value, Vars::Chams::Friendly::OccludedColor.Value)
		: GetStruct(Vars::Chams::Enemy::VisibleMaterial.Value, Vars::Chams::Enemy::OccludedMaterial.Value, Vars::Chams::Enemy::VisibleColor.Value, Vars::Chams::Enemy::OccludedColor.Value);
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
		return GetPlayerChams(pEntity->As<CTFPlayer>(), pLocal, pChams, Vars::Chams::Friendly::Players.Value, Vars::Chams::Enemy::Players.Value);
	case ETFClassID::CTFWeaponBase:
		SDK::Output("Chams", "ETFClassID::CTFWeaponBase");
		break;
	case ETFClassID::CTFWearable:
	{
		auto pOwner = pEntity->m_hOwnerEntity().Get();
		if (!pOwner)
			return false;

		return GetPlayerChams(pOwner->As<CTFPlayer>(), pLocal, pChams, Vars::Chams::Friendly::Players.Value, Vars::Chams::Enemy::Players.Value);
	}
	// building chams
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
	{
		auto pOwner = pEntity->As<CBaseObject>()->m_hBuilder().Get();
		if (!pOwner)
			return false;

		return GetPlayerChams(pOwner->As<CTFPlayer>(), pLocal, pChams, Vars::Chams::Friendly::Buildings.Value, Vars::Chams::Enemy::Buildings.Value);
	}
	// ragdoll chams
	case ETFClassID::CRagdollPropAttached:
	case ETFClassID::CRagdollProp:
	case ETFClassID::CTFRagdoll:
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
		if (!pOwner)
			return false;

		return GetPlayerChams(pOwner->As<CTFPlayer>(), pLocal, pChams, Vars::Chams::Friendly::Ragdolls.Value, Vars::Chams::Enemy::Ragdolls.Value);
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
	{
		auto pOwner = pEntity->As<CBaseGrenade>()->m_hThrower().Get();
		if (!pOwner)
			return false;

		return GetPlayerChams(pOwner->As<CTFPlayer>(), pLocal, pChams, Vars::Chams::Friendly::Projectiles.Value, Vars::Chams::Enemy::Projectiles.Value);
	}
	case ETFClassID::CTFBaseRocket:
	case ETFClassID::CTFFlameRocket:
	case ETFClassID::CTFProjectile_Arrow:
	case ETFClassID::CTFProjectile_GrapplingHook:
	case ETFClassID::CTFProjectile_HealingBolt:
	case ETFClassID::CTFProjectile_Rocket:
	//case ETFClassID::CTFProjectile_BallOfFire: // lifetime too short
	case ETFClassID::CTFProjectile_MechanicalArmOrb:
	case ETFClassID::CTFProjectile_SentryRocket: // not drawn, no weapon
	case ETFClassID::CTFProjectile_SpellFireball:
	case ETFClassID::CTFProjectile_SpellLightningOrb:
	case ETFClassID::CTFProjectile_SpellKartOrb:
	case ETFClassID::CTFProjectile_EnergyBall:
	case ETFClassID::CTFProjectile_Flare:
	{
		auto pWeapon = pEntity->As<CTFBaseRocket>()->m_hLauncher().Get();
		if (!pWeapon)
			return false;

		auto pOwner = pWeapon->As<CTFWeaponBase>()->m_hOwner().Get();
		if (!pOwner)
			return false;

		return GetPlayerChams(pOwner->As<CTFPlayer>(), pLocal, pChams, Vars::Chams::Friendly::Projectiles.Value, Vars::Chams::Enemy::Projectiles.Value);
	}
	case ETFClassID::CTFBaseProjectile:
	case ETFClassID::CTFProjectile_EnergyRing: // not drawn, shoulddraw check, small anyways
	//case ETFClassID::CTFProjectile_Syringe: // not drawn
	{
		auto pWeapon = pEntity->As<CTFBaseRocket>()->m_hLauncher().Get();
		if (!pWeapon)
			return false;

		auto pOwner = pWeapon->As<CTFWeaponBase>()->m_hOwner().Get();
		if (!pOwner)
			return false;

		return GetPlayerChams(pOwner->As<CTFPlayer>(), pLocal, pChams, Vars::Chams::Friendly::Projectiles.Value, Vars::Chams::Enemy::Projectiles.Value);
	}
	// npc chams
	case ETFClassID::CHeadlessHatman:
	case ETFClassID::CTFTankBoss:
	case ETFClassID::CMerasmus:
	case ETFClassID::CZombie:
	case ETFClassID::CEyeballBoss:
		*pChams = GetStruct(Vars::Chams::World::VisibleMaterial.Value, Vars::Chams::World::OccludedMaterial.Value, Vars::Chams::World::VisibleColor.Value, Vars::Chams::World::OccludedColor.Value);
		return Vars::Chams::World::NPCs.Value;
	// pickup chams
	case ETFClassID::CTFAmmoPack:
	case ETFClassID::CCurrencyPack:
	case ETFClassID::CHalloweenGiftPickup:
		*pChams = GetStruct(Vars::Chams::World::VisibleMaterial.Value, Vars::Chams::World::OccludedMaterial.Value, Vars::Chams::World::VisibleColor.Value, Vars::Chams::World::OccludedColor.Value);
		return Vars::Chams::World::Pickups.Value;
	case ETFClassID::CBaseAnimating:
	{
		if (H::Entities.IsAmmo(pEntity) || H::Entities.IsHealth(pEntity) || H::Entities.IsSpellbook(pEntity))
		{
			*pChams = GetStruct(Vars::Chams::World::VisibleMaterial.Value, Vars::Chams::World::OccludedMaterial.Value, Vars::Chams::World::VisibleColor.Value, Vars::Chams::World::OccludedColor.Value);
			return Vars::Chams::World::Pickups.Value;
		}
		break;
	}
	// bomb chams
	case ETFClassID::CTFPumpkinBomb:
	case ETFClassID::CTFGenericBomb:
		*pChams = GetStruct(Vars::Chams::World::VisibleMaterial.Value, Vars::Chams::World::OccludedMaterial.Value, Vars::Chams::World::VisibleColor.Value, Vars::Chams::World::OccludedColor.Value);
		return Vars::Chams::World::Bombs.Value;
	}

	// player chams
	if (auto pWeapon = pEntity->As<CTFWeaponBase>())
	{
		auto pOwner = pWeapon->m_hOwnerEntity().Get();
		if (!pOwner)
			return false;

		return GetPlayerChams(pOwner->As<CTFPlayer>(), pLocal, pChams, Vars::Chams::Friendly::Players.Value, Vars::Chams::Enemy::Players.Value);
	}
	
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

void CChams::DrawModel(CBaseEntity* pEntity, Chams_t chams, IMatRenderContext* pRenderContext, bool bTwoModels)
{
	mEntities[pEntity->entindex()] = true;
	bRendering = true;

	auto visibleMaterials = chams.VisibleMaterial.size() ? chams.VisibleMaterial : std::vector<std::string>{ "None" };
	auto occludedMaterials = chams.OccludedMaterial.size() ? chams.OccludedMaterial : std::vector<std::string>{ "None" };

	StencilBegin(pRenderContext, bTwoModels);

	StencilVisible(pRenderContext, bTwoModels);
	for (auto it = visibleMaterials.begin(); it != visibleMaterials.end(); it++)
	{
		auto material = F::Materials.GetMaterial(*it);

		F::Materials.SetColor(material, chams.VisibleColor, it + 1 == visibleMaterials.end()); // only apply color to last material
		I::ModelRender->ForcedMaterialOverride(material);
		pEntity->DrawModel(STUDIO_RENDER);
	}
	if (bTwoModels)
	{
		StencilOccluded(pRenderContext);
		for (auto it = occludedMaterials.begin(); it != occludedMaterials.end(); it++)
		{
			auto material = F::Materials.GetMaterial(*it);

			F::Materials.SetColor(material, chams.OccludedColor, it + 1 == occludedMaterials.end());
			I::ModelRender->ForcedMaterialOverride(material);
			pEntity->DrawModel(STUDIO_RENDER);
		}
	}

	StencilEnd(pRenderContext, bTwoModels);
	I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
	I::RenderView->SetBlend(1.f);
	I::ModelRender->ForcedMaterialOverride(nullptr);

	bRendering = false;
}



void CChams::RenderMain(CTFPlayer* pLocal)
{
	const auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pLocal || !pRenderContext)
		return;

	// main
	for (int n = 1; n < I::ClientEntityList->GetHighestEntityIndex(); n++)
	{
		auto pEntity = I::ClientEntityList->GetClientEntity(n)->As<CBaseEntity>();
		if (!pEntity)
			continue;

		Chams_t chams = {};
		if (GetChams(pLocal, pEntity, &chams) && SDK::IsOnScreen(pEntity))
			DrawModel(pEntity, chams, pRenderContext);
	}

	// backtrack / fakeangle
	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (!pPlayer->ShouldDraw() || pPlayer->IsDormant())
			continue;

		bRendering = bExtra = true;

		const float flOldInvisibility = pPlayer->m_flInvisibility();
		if (flOldInvisibility > 0.999f)
			pPlayer->m_flInvisibility() = 0.f;

		pPlayer->DrawModel(STUDIO_RENDER);

		if (flOldInvisibility > 0.999f)
			pPlayer->m_flInvisibility() = flOldInvisibility;

		bRendering = bExtra = false;
	}
}

void CChams::RenderBacktrack(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (!Vars::Backtrack::Enabled.Value || !Vars::Chams::Backtrack::Enabled.Value)
		return;

	const auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["ModelRender_DrawModelExecute"];
	if (!ModelRender_DrawModelExecute)
		return;

	auto pEntity = I::ClientEntityList->GetClientEntity(pInfo.entity_index)->As<CTFPlayer>();
	if (!pEntity || !pEntity->IsPlayer() || !pEntity->IsAlive())
		return;

	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();
	if (!pLocal || !pWeapon || G::WeaponType == EWeaponType::PROJECTILE)
		return;
	if (pEntity == pLocal ||
		pWeapon->m_iItemDefinitionIndex() != Soldier_t_TheDisciplinaryAction && pWeapon->m_iWeaponID() != TF_WEAPON_MEDIGUN && pEntity->m_iTeamNum() == pLocal->m_iTeamNum() ||
		pWeapon->m_iWeaponID() == TF_WEAPON_MEDIGUN && pEntity->m_iTeamNum() != pLocal->m_iTeamNum())
		return;



	auto drawModel = [ModelRender_DrawModelExecute, pEntity](Vec3& vOrigin, std::vector<std::string> materials, Color_t color, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
		{
			if (!SDK::IsOnScreen(pEntity, vOrigin))
				return;

			for (auto it = materials.begin(); it != materials.end(); it++)
			{
				auto material = F::Materials.GetMaterial(*it);

				F::Materials.SetColor(material, color, it + 1 == materials.end());
				I::ModelRender->ForcedMaterialOverride(material ? material : nullptr);
				ModelRender_DrawModelExecute->Original<void(__thiscall*)(IVModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4*)>()(I::ModelRender, pState, pInfo, pBoneToWorld);
			}

			I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
			I::RenderView->SetBlend(1.f);
			I::ModelRender->ForcedMaterialOverride(nullptr);
		};



	auto& vMaterials = Vars::Chams::Backtrack::VisibleMaterial.Value;
	auto& sColor = Vars::Chams::Backtrack::VisibleColor.Value;

	auto pRecords = F::Backtrack.GetRecords(pEntity);
	auto vRecords = F::Backtrack.GetValidRecords(pRecords);
	if (!vRecords.size())
		return;

	switch (Vars::Chams::Backtrack::Draw.Value)
	{
	case 0: // last
	{
		auto vLastRec = vRecords.end() - 1;
		if (vLastRec != vRecords.end() && pEntity->GetAbsOrigin().DistTo(vLastRec->vOrigin) > 0.1f)
			drawModel(vLastRec->vOrigin, vMaterials, sColor, pState, pInfo, reinterpret_cast<matrix3x4*>(&vLastRec->BoneMatrix));
		break;
	}
	case 1: // last + first
	{
		auto vFirstRec = vRecords.begin();
		if (vFirstRec != vRecords.end() && pEntity->GetAbsOrigin().DistTo(vFirstRec->vOrigin) > 0.1f)
			drawModel(vFirstRec->vOrigin, vMaterials, sColor, pState, pInfo, reinterpret_cast<matrix3x4*>(&vFirstRec->BoneMatrix));
		auto vLastRec = vRecords.end() - 1;
		if (vLastRec != vRecords.end() && pEntity->GetAbsOrigin().DistTo(vLastRec->vOrigin) > 0.1f)
			drawModel(vLastRec->vOrigin, vMaterials, sColor, pState, pInfo, reinterpret_cast<matrix3x4*>(&vLastRec->BoneMatrix));
		break;
	}
	case 2: // all
	{
		for (auto& record : vRecords)
		{
			if (pEntity->GetAbsOrigin().DistTo(record.vOrigin) < 0.1f)
				continue;

			drawModel(record.vOrigin, vMaterials, sColor, pState, pInfo, reinterpret_cast<matrix3x4*>(&record.BoneMatrix));
		}
	}
	}
}
void CChams::RenderFakeAngle(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (!Vars::Chams::FakeAngle::Enabled.Value || pInfo.entity_index != I::EngineClient->GetLocalPlayer() || !F::FakeAngle.DrawChams || !F::FakeAngle.BonesSetup)
		return;

	const auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["ModelRender_DrawModelExecute"];
	if (!ModelRender_DrawModelExecute)
		return;



	auto& vMaterials = Vars::Chams::FakeAngle::VisibleMaterial.Value;
	auto& sColor = Vars::Chams::FakeAngle::VisibleColor.Value;

	for (auto it = vMaterials.begin(); it != vMaterials.end(); it++)
	{
		auto material = F::Materials.GetMaterial(*it);

		F::Materials.SetColor(material, sColor, it + 1 == vMaterials.end());
		I::ModelRender->ForcedMaterialOverride(material ? material : nullptr);
		ModelRender_DrawModelExecute->Original<void(__thiscall*)(IVModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4*)>()(I::ModelRender, pState, pInfo, F::FakeAngle.BoneMatrix);
	}

	I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
	I::RenderView->SetBlend(1.f);
	I::ModelRender->ForcedMaterialOverride(nullptr);
}
void CChams::RenderHandler(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (!bExtra)
	{
		if (const auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["ModelRender_DrawModelExecute"])
			ModelRender_DrawModelExecute->Original<void(__thiscall*)(IVModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4*)>()(I::ModelRender, pState, pInfo, pBoneToWorld);
	}
	else
	{
		RenderBacktrack(pState, pInfo, pBoneToWorld);
		RenderFakeAngle(pState, pInfo, pBoneToWorld);
	}
}

bool CChams::RenderViewmodel(void* ecx, int flags, int* iReturn)
{
	if (!Vars::Chams::Viewmodel::Weapon.Value)
		return false;

	const auto CBaseAnimating_DrawModel = U::Hooks.m_mHooks["CBaseAnimating_DrawModel"];
	if (!CBaseAnimating_DrawModel)
		return false;



	auto& vMaterials = Vars::Chams::Viewmodel::VisibleMaterial.Value;
	auto& sColor = Vars::Chams::Viewmodel::VisibleColor.Value;

	for (auto it = vMaterials.begin(); it != vMaterials.end(); it++)
	{
		auto material = F::Materials.GetMaterial(*it);

		F::Materials.SetColor(material, sColor, it + 1 == vMaterials.end());
		I::ModelRender->ForcedMaterialOverride(material ? material : nullptr);
		*iReturn = CBaseAnimating_DrawModel->Original<int(__thiscall*)(void*, int)>()(ecx, flags);
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

	const auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["ModelRender_DrawModelExecute"];
	if (!ModelRender_DrawModelExecute)
		return false;



	auto& vMaterials = Vars::Chams::Viewmodel::VisibleMaterial.Value;
	auto& sColor = Vars::Chams::Viewmodel::VisibleColor.Value;

	for (auto it = vMaterials.begin(); it != vMaterials.end(); it++)
	{
		auto material = F::Materials.GetMaterial(*it);

		F::Materials.SetColor(material, sColor, it + 1 == vMaterials.end());
		I::ModelRender->ForcedMaterialOverride(material ? material : nullptr);
		ModelRender_DrawModelExecute->Original<void(__thiscall*)(IVModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4*)>()(I::ModelRender, pState, pInfo, pBoneToWorld);
	}

	I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
	I::RenderView->SetBlend(1.f);
	I::ModelRender->ForcedMaterialOverride(nullptr);

	return true;
}
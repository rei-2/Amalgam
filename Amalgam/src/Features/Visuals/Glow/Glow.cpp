#include "Glow.h"

#include "../Materials/Materials.h"
#include "../FakeAngle/FakeAngle.h"
#include "../../Backtrack/Backtrack.h"
#include "../../Players/PlayerUtils.h"

Glow_t CGlow::GetStruct(bool Stencil, bool Blur, int StencilScale, int BlurScale)
{
	return Glow_t{
		Stencil,
		Blur,
		StencilScale,
		BlurScale
	};
}

bool CGlow::GetPlayerGlow(CTFPlayer* pEntity, CTFPlayer* pLocal, Glow_t* pGlow, Color_t* pColor, bool bFriendly, bool bEnemy)
{
	if (Vars::Glow::Player::Local.Value && pEntity == pLocal
		|| Vars::Glow::Player::Friend.Value && H::Entities.IsFriend(pEntity->entindex())
		|| Vars::Glow::Player::Priority.Value && F::PlayerUtils.GetPriority(pEntity->entindex()) > F::PlayerUtils.mTags["Default"].Priority)
	{
		*pGlow = GetStruct(Vars::Glow::Player::Stencil.Value, Vars::Glow::Player::Blur.Value, Vars::Glow::Player::StencilScale.Value, Vars::Glow::Player::BlurScale.Value);
		*pColor = H::Color.GetEntityDrawColor(pLocal, pEntity, Vars::Colors::Relative.Value);
		return true;
	}

	const bool bTeam = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();
	*pGlow = bTeam
		? GetStruct(Vars::Glow::Friendly::Stencil.Value, Vars::Glow::Friendly::Blur.Value, Vars::Glow::Friendly::StencilScale.Value, Vars::Glow::Friendly::BlurScale.Value)
		: GetStruct(Vars::Glow::Enemy::Stencil.Value, Vars::Glow::Enemy::Blur.Value, Vars::Glow::Enemy::StencilScale.Value, Vars::Glow::Enemy::BlurScale.Value);
	*pColor = H::Color.GetEntityDrawColor(pLocal, pEntity, Vars::Colors::Relative.Value);
	return bTeam ? bFriendly : bEnemy;
}

bool CGlow::GetGlow(CTFPlayer* pLocal, CBaseEntity* pEntity, Glow_t* pGlow, Color_t* pColor)
{
	if (pEntity->IsDormant() || !pEntity->ShouldDraw())
		return false;

	switch (pEntity->GetClassID())
	{
	// player glow
	case ETFClassID::CTFPlayer:
		return GetPlayerGlow(pEntity->As<CTFPlayer>(), pLocal, pGlow, pColor, Vars::Glow::Friendly::Players.Value, Vars::Glow::Enemy::Players.Value);
	case ETFClassID::CTFWearable:
	{
		auto pOwner = pEntity->m_hOwnerEntity().Get();
		if (!pOwner)
			return false;

		return GetPlayerGlow(pOwner->As<CTFPlayer>(), pLocal, pGlow, pColor, Vars::Glow::Friendly::Players.Value, Vars::Glow::Enemy::Players.Value);
	}
	// building glow
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
	{
		auto pOwner = pEntity->As<CBaseObject>()->m_hBuilder().Get();
		if (!pOwner)
			return false;

		return GetPlayerGlow(pOwner->As<CTFPlayer>(), pLocal, pGlow, pColor, Vars::Glow::Friendly::Buildings.Value, Vars::Glow::Enemy::Buildings.Value);
	}
	// ragdoll glow
	case ETFClassID::CRagdollPropAttached:
	case ETFClassID::CRagdollProp:
	case ETFClassID::CTFRagdoll:
	{
		auto pOwner = pEntity->As<CTFRagdoll>()->m_hPlayer().Get();
		if (!pOwner)
			return false;

		return GetPlayerGlow(pOwner->As<CTFPlayer>(), pLocal, pGlow, pColor, Vars::Glow::Friendly::Ragdolls.Value, Vars::Glow::Enemy::Ragdolls.Value);
	}
	// projectile glow
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

		return GetPlayerGlow(pOwner->As<CTFPlayer>(), pLocal, pGlow, pColor, Vars::Glow::Friendly::Projectiles.Value, Vars::Glow::Enemy::Projectiles.Value);
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

		return GetPlayerGlow(pOwner->As<CTFPlayer>(), pLocal, pGlow, pColor, Vars::Glow::Friendly::Projectiles.Value, Vars::Glow::Enemy::Projectiles.Value);
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

		return GetPlayerGlow(pOwner->As<CTFPlayer>(), pLocal, pGlow, pColor, Vars::Glow::Friendly::Projectiles.Value, Vars::Glow::Enemy::Projectiles.Value);
	}
	// npc glow
	case ETFClassID::CHeadlessHatman:
	case ETFClassID::CTFTankBoss:
	case ETFClassID::CMerasmus:
	case ETFClassID::CZombie:
	case ETFClassID::CEyeballBoss:
		*pGlow = GetStruct(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value, Vars::Glow::World::StencilScale.Value, Vars::Glow::World::BlurScale.Value);
		*pColor = Vars::Colors::NPC.Value;
		return Vars::Glow::World::NPCs.Value;
	// pickup glow
	case ETFClassID::CTFAmmoPack:
		*pGlow = GetStruct(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value, Vars::Glow::World::StencilScale.Value, Vars::Glow::World::BlurScale.Value);
		*pColor = Vars::Colors::Ammo.Value;
		return Vars::Glow::World::Pickups.Value;
	case ETFClassID::CCurrencyPack:
		*pGlow = GetStruct(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value, Vars::Glow::World::StencilScale.Value, Vars::Glow::World::BlurScale.Value);
		*pColor = Vars::Colors::Money.Value;
		return Vars::Glow::World::Pickups.Value;
	case ETFClassID::CHalloweenGiftPickup:
		*pGlow = GetStruct(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value, Vars::Glow::World::StencilScale.Value, Vars::Glow::World::BlurScale.Value);
		*pColor = Vars::Colors::Halloween.Value;
		return Vars::Glow::World::Halloween.Value;
	case ETFClassID::CBaseAnimating:
	{
		if (H::Entities.IsAmmo(pEntity))
		{
			*pGlow = GetStruct(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value, Vars::Glow::World::StencilScale.Value, Vars::Glow::World::BlurScale.Value);
			*pColor = Vars::Colors::Ammo.Value;
			return Vars::Glow::World::Pickups.Value;
		}
		if (H::Entities.IsHealth(pEntity))
		{
			*pGlow = GetStruct(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value, Vars::Glow::World::StencilScale.Value, Vars::Glow::World::BlurScale.Value);
			*pColor = Vars::Colors::Health.Value;
			return Vars::Glow::World::Pickups.Value;
		}
		if (H::Entities.IsSpellbook(pEntity))
		{
			*pGlow = GetStruct(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value, Vars::Glow::World::StencilScale.Value, Vars::Glow::World::BlurScale.Value);
			*pColor = Vars::Colors::Halloween.Value;
			return Vars::Glow::World::Halloween.Value;
		}
		break;
	}
	// bomb glow
	case ETFClassID::CTFPumpkinBomb:
	case ETFClassID::CTFGenericBomb:
		*pGlow = GetStruct(Vars::Glow::World::Stencil.Value, Vars::Glow::World::Blur.Value, Vars::Glow::World::StencilScale.Value, Vars::Glow::World::BlurScale.Value);
		*pColor = Vars::Colors::Bomb.Value;
		return Vars::Glow::World::Bombs.Value;
	}

	// player glow
	if (auto pWeapon = pEntity->As<CTFWeaponBase>())
	{
		auto pOwner = pWeapon->m_hOwnerEntity().Get();
		if (!pOwner || !pOwner->IsPlayer())
			return false;

		return GetPlayerGlow(pOwner->As<CTFPlayer>(), pLocal, pGlow, pColor, Vars::Glow::Friendly::Players.Value, Vars::Glow::Enemy::Players.Value);
	}

	return false;
}

void CGlow::SetupBegin(Glow_t glow, IMatRenderContext* pRenderContext, IMaterial* m_pMatBlurY)
{
	if (IMaterialVar* pVar = m_pMatBlurY->FindVar("$bloomamount", nullptr))
		pVar->SetFloatValue(glow.BlurScale);

	StencilBegin(pRenderContext);
	I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
	I::RenderView->SetBlend(1.f);
}
void CGlow::SetupMid(IMatRenderContext* pRenderContext, IMaterial* m_pMatGlowColor, int w, int h)
{
	I::ModelRender->ForcedMaterialOverride(m_pMatGlowColor);

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
		int side = float(glow.StencilScale + 1) / 2;
		int corner = float(glow.StencilScale) / 2;
		if (corner)
		{
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, -corner, -corner, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, corner, corner, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, corner, -corner, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, -corner, corner, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		}
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, -side, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, 0, -side, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, 0, side, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, side, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
	}
	if (glow.Blur)
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, 0, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);

	StencilEnd(pRenderContext);

	I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
	I::RenderView->SetBlend(1.f);
	I::ModelRender->ForcedMaterialOverride(nullptr);
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

void CGlow::DrawModel(CBaseEntity* pEntity, bool bModel)
{
	bRendering = true;

	if (bModel)
		I::RenderView->SetBlend(0.f);
	const float flOldInvisibility = pEntity->IsPlayer() ? pEntity->As<CTFPlayer>()->m_flInvisibility() : -1.f;
	if (flOldInvisibility > 0.999f)
		pEntity->As<CTFPlayer>()->m_flInvisibility() = 0.f;

	pEntity->DrawModel(bModel ? STUDIO_RENDER : (STUDIO_RENDER | STUDIO_NOSHADOWS));

	if (flOldInvisibility > 0.999f)
		pEntity->As<CTFPlayer>()->m_flInvisibility() = flOldInvisibility;
	if (bModel)
		I::RenderView->SetBlend(1.f);

	bRendering = false;
}



void CGlow::RenderMain(CTFPlayer* pLocal)
{
	mEntities.clear();
	const int w = H::Draw.m_nScreenW, h = H::Draw.m_nScreenH;
	if (!pLocal || w < 1 || h < 1 || w > 4096 || h > 2160)
		return;

	const auto pRenderContext = I::MaterialSystem->GetRenderContext();
	auto m_pMatGlowColor = F::Materials.mGlowMaterials["GlowColor"].pMaterial;
	auto m_pMatBlurX = F::Materials.mGlowMaterials["BlurX"].pMaterial;
	auto m_pMatBlurY = F::Materials.mGlowMaterials["BlurY"].pMaterial;
	auto m_pMatHaloAddToScreen = F::Materials.mGlowMaterials["HaloAddToScreen"].pMaterial;
	if (!pRenderContext || !m_pMatGlowColor || !m_pMatBlurX || !m_pMatBlurY || !m_pMatHaloAddToScreen)
		return F::Materials.ReloadMaterials();



	for (int n = 1; n < I::ClientEntityList->GetHighestEntityIndex(); n++)
	{
		auto pEntity = I::ClientEntityList->GetClientEntity(n)->As<CBaseEntity>();
		if (!pEntity)
			continue;

		Glow_t glow = {}; Color_t color = {};
		if (GetGlow(pLocal, pEntity, &glow, &color) && SDK::IsOnScreen(pEntity))
			mEntities[glow].push_back({ pEntity, color });
	}

	// main
	for (const auto& [glow, entities] : mEntities)
	{
		SetupBegin(glow, pRenderContext, m_pMatBlurY);
		for (auto& info : entities)
			DrawModel(info.m_pEntity, true);

		StencilEnd(pRenderContext);

		SetupMid(pRenderContext, m_pMatGlowColor, w, h);
		for (auto& info : entities)
		{
			I::RenderView->SetColorModulation(float(info.m_Color.r) / 255.f, float(info.m_Color.g) / 255.f, float(info.m_Color.b) / 255.f);
			I::RenderView->SetBlend(float(info.m_Color.a) / 255.f);
			DrawModel(info.m_pEntity, false);
		}

		SetupEnd(glow, pRenderContext, m_pMatBlurX, m_pMatBlurY, m_pMatHaloAddToScreen, w, h);
	}

	// backtrack / fakeangle
	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
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

void CGlow::RenderBacktrack(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld,
	int w, int h, IMatRenderContext* pRenderContext, IMaterial* m_pMatGlowColor, IMaterial* m_pMatBlurX, IMaterial* m_pMatBlurY, IMaterial* m_pMatHaloAddToScreen)
{
	if (!Vars::Backtrack::Enabled.Value || !Vars::Glow::Backtrack::Enabled.Value || !Vars::Glow::Backtrack::Stencil.Value && !Vars::Glow::Backtrack::Blur.Value)
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



	auto drawModel = [ModelRender_DrawModelExecute, pEntity](Vec3& vOrigin, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
		{
			if (!SDK::IsOnScreen(pEntity, vOrigin))
				return;

			ModelRender_DrawModelExecute->Original<void(__thiscall*)(IVModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4*)>()(I::ModelRender, pState, pInfo, pBoneToWorld);
		};
	auto drawModels = [drawModel, pEntity, pState, pInfo](bool bModel)
		{
			if (bModel)
				I::RenderView->SetBlend(0.f);

			const auto& pRecords = F::Backtrack.GetRecords(pEntity);
			auto vRecords = F::Backtrack.GetValidRecords(pRecords);
			if (!vRecords.size())
				return;

			switch (Vars::Glow::Backtrack::Draw.Value)
			{
			case 0: // last
			{
				auto vLastRec = vRecords.end() - 1;
				if (vLastRec != vRecords.end() && pEntity->GetAbsOrigin().DistTo(vLastRec->vOrigin) > 0.1f)
					drawModel(vLastRec->vOrigin, pState, pInfo, reinterpret_cast<matrix3x4*>(&vLastRec->BoneMatrix));
				break;
			}
			case 1: // last + first
			{
				auto vFirstRec = vRecords.begin();
				if (vFirstRec != vRecords.end() && pEntity->GetAbsOrigin().DistTo(vFirstRec->vOrigin) > 0.1f)
					drawModel(vFirstRec->vOrigin, pState, pInfo, reinterpret_cast<matrix3x4*>(&vFirstRec->BoneMatrix));
				auto vLastRec = vRecords.end() - 1;
				if (vLastRec != vRecords.end() && pEntity->GetAbsOrigin().DistTo(vLastRec->vOrigin) > 0.1f)
					drawModel(vLastRec->vOrigin, pState, pInfo, reinterpret_cast<matrix3x4*>(&vLastRec->BoneMatrix));
				break;
			}
			case 2: // all
			{
				for (auto& record : vRecords)
				{
					if (pEntity->GetAbsOrigin().DistTo(record.vOrigin) < 0.1f)
						continue;

					drawModel(record.vOrigin, pState, pInfo, reinterpret_cast<matrix3x4*>(&record.BoneMatrix));
				}
			}
			}

			if (bModel)
				I::RenderView->SetBlend(1.f);
		};



	auto glow = GetStruct(Vars::Glow::Backtrack::Stencil.Value, Vars::Glow::Backtrack::Blur.Value, Vars::Glow::Backtrack::StencilScale.Value, Vars::Glow::Backtrack::BlurScale.Value);

	SetupBegin(glow, pRenderContext, m_pMatBlurY);
	drawModels(true);

	StencilEnd(pRenderContext);

	SetupMid(pRenderContext, m_pMatGlowColor, w, h);
	auto color = H::Color.GetEntityDrawColor(pLocal, pEntity, Vars::Colors::Relative.Value);
	I::RenderView->SetColorModulation(float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f);
	I::RenderView->SetBlend(float(color.a) / 255.f);
	drawModels(false);

	SetupEnd(glow, pRenderContext, m_pMatBlurX, m_pMatBlurY, m_pMatHaloAddToScreen, w, h);
}
void CGlow::RenderFakeAngle(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld,
	int w, int h, IMatRenderContext* pRenderContext, IMaterial* m_pMatGlowColor, IMaterial* m_pMatBlurX, IMaterial* m_pMatBlurY, IMaterial* m_pMatHaloAddToScreen)
{
	if (!Vars::Glow::FakeAngle::Enabled.Value || !Vars::Glow::FakeAngle::Stencil.Value && !Vars::Glow::FakeAngle::Blur.Value || pInfo.entity_index != I::EngineClient->GetLocalPlayer() || !F::FakeAngle.DrawChams || !F::FakeAngle.BonesSetup)
		return;

	const auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["ModelRender_DrawModelExecute"];
	if (!ModelRender_DrawModelExecute)
		return;



	auto drawModel = [ModelRender_DrawModelExecute, pState, pInfo](bool bModel)
		{
			if (bModel)
				I::RenderView->SetBlend(0.f);

			ModelRender_DrawModelExecute->Original<void(__thiscall*)(IVModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4*)>()(I::ModelRender, pState, pInfo, F::FakeAngle.BoneMatrix);

			if (bModel)
				I::RenderView->SetBlend(1.f);
		};



	auto glow = GetStruct(Vars::Glow::FakeAngle::Stencil.Value, Vars::Glow::FakeAngle::Blur.Value, Vars::Glow::FakeAngle::StencilScale.Value, Vars::Glow::FakeAngle::BlurScale.Value);

	SetupBegin(glow, pRenderContext, m_pMatBlurY);
	drawModel(true);

	StencilEnd(pRenderContext);

	SetupMid(pRenderContext, m_pMatGlowColor, w, h);
	auto& color = Vars::Colors::Local.Value;
	I::RenderView->SetColorModulation(float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f);
	I::RenderView->SetBlend(float(color.a) / 255.f);
	drawModel(false);

	SetupEnd(glow, pRenderContext, m_pMatBlurX, m_pMatBlurY, m_pMatHaloAddToScreen, w, h);
}
void CGlow::RenderHandler(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (!bExtra)
	{
		if (const auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["ModelRender_DrawModelExecute"])
			ModelRender_DrawModelExecute->Original<void(__thiscall*)(IVModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4*)>()(I::ModelRender, pState, pInfo, pBoneToWorld);
	}
	else
	{
		const int w = H::Draw.m_nScreenW, h = H::Draw.m_nScreenH;
		if (w < 1 || h < 1 || w > 4096 || h > 2160)
			return;

		const auto pRenderContext = I::MaterialSystem->GetRenderContext();
		auto m_pMatGlowColor = F::Materials.mGlowMaterials["GlowColor"].pMaterial;
		auto m_pMatBlurX = F::Materials.mGlowMaterials["BlurX"].pMaterial;
		auto m_pMatBlurY = F::Materials.mGlowMaterials["BlurY"].pMaterial;
		auto m_pMatHaloAddToScreen = F::Materials.mGlowMaterials["HaloAddToScreen"].pMaterial;
		if (!pRenderContext || !m_pMatGlowColor || !m_pMatBlurX || !m_pMatBlurY || !m_pMatHaloAddToScreen)
			return F::Materials.ReloadMaterials();

		RenderBacktrack(pState, pInfo, pBoneToWorld, w, h, pRenderContext, m_pMatGlowColor, m_pMatBlurX, m_pMatBlurY, m_pMatHaloAddToScreen);
		RenderFakeAngle(pState, pInfo, pBoneToWorld, w, h, pRenderContext, m_pMatGlowColor, m_pMatBlurX, m_pMatBlurY, m_pMatHaloAddToScreen);
	}
}

void CGlow::RenderViewmodel(void* ecx, int flags)
{
	if (!Vars::Glow::Viewmodel::Weapon.Value || !Vars::Glow::Viewmodel::Stencil.Value && !Vars::Glow::Viewmodel::Blur.Value)
		return;

	const int w = H::Draw.m_nScreenW, h = H::Draw.m_nScreenH;
	if (w < 1 || h < 1 || w > 4096 || h > 2160)
		return;

	const auto pRenderContext = I::MaterialSystem->GetRenderContext();
	auto m_pMatGlowColor = F::Materials.mGlowMaterials["GlowColor"].pMaterial;
	auto m_pMatBlurX = F::Materials.mGlowMaterials["BlurX"].pMaterial;
	auto m_pMatBlurY = F::Materials.mGlowMaterials["BlurY"].pMaterial;
	auto m_pMatHaloAddToScreen = F::Materials.mGlowMaterials["HaloAddToScreen"].pMaterial;
	if (!pRenderContext || !m_pMatGlowColor || !m_pMatBlurX || !m_pMatBlurY || !m_pMatHaloAddToScreen)
		return F::Materials.ReloadMaterials();

	const auto CBaseAnimating_DrawModel = U::Hooks.m_mHooks["CBaseAnimating_DrawModel"];
	if (!CBaseAnimating_DrawModel)
		return;



	auto drawModel = [CBaseAnimating_DrawModel, ecx, flags](bool bModel)
		{
			if (bModel)
				I::RenderView->SetBlend(0.f);

			CBaseAnimating_DrawModel->Original<int(__thiscall*)(void*, int)>()(ecx, flags);

			if (bModel)
				I::RenderView->SetBlend(1.f);
		};



	auto glow = GetStruct(Vars::Glow::Viewmodel::Stencil.Value, Vars::Glow::Viewmodel::Blur.Value, Vars::Glow::Viewmodel::StencilScale.Value, Vars::Glow::Viewmodel::BlurScale.Value);

	SetupBegin(glow, pRenderContext, m_pMatBlurY);
	drawModel(true);

	StencilEnd(pRenderContext);

	SetupMid(pRenderContext, m_pMatGlowColor, w, h);
	auto& color = Vars::Colors::Local.Value;
	I::RenderView->SetColorModulation(float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f);
	I::RenderView->SetBlend(float(color.a) / 255.f);
	drawModel(false);

	SetupEnd(glow, pRenderContext, m_pMatBlurX, m_pMatBlurY, m_pMatHaloAddToScreen, w, h);
}
void CGlow::RenderViewmodel(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (!Vars::Glow::Viewmodel::Hands.Value || !Vars::Glow::Viewmodel::Stencil.Value && !Vars::Glow::Viewmodel::Blur.Value)
		return;

	const int w = H::Draw.m_nScreenW, h = H::Draw.m_nScreenH;
	if (w < 1 || h < 1 || w > 4096 || h > 2160)
		return;

	const auto pRenderContext = I::MaterialSystem->GetRenderContext();
	auto m_pMatGlowColor = F::Materials.mGlowMaterials["GlowColor"].pMaterial;
	auto m_pMatBlurX = F::Materials.mGlowMaterials["BlurX"].pMaterial;
	auto m_pMatBlurY = F::Materials.mGlowMaterials["BlurY"].pMaterial;
	auto m_pMatHaloAddToScreen = F::Materials.mGlowMaterials["HaloAddToScreen"].pMaterial;
	if (!pRenderContext || !m_pMatGlowColor || !m_pMatBlurX || !m_pMatBlurY || !m_pMatHaloAddToScreen)
		return F::Materials.ReloadMaterials();

	const auto ModelRender_DrawModelExecute = U::Hooks.m_mHooks["ModelRender_DrawModelExecute"];
	if (!ModelRender_DrawModelExecute)
		return;



	auto drawModel = [ModelRender_DrawModelExecute, pState, pInfo, pBoneToWorld](bool bModel)
		{
			if (bModel)
				I::RenderView->SetBlend(0.f);

			ModelRender_DrawModelExecute->Original<void(__thiscall*)(IVModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4*)>()(I::ModelRender, pState, pInfo, pBoneToWorld);

			if (bModel)
				I::RenderView->SetBlend(1.f);
		};



	auto glow = GetStruct(Vars::Glow::Viewmodel::Stencil.Value, Vars::Glow::Viewmodel::Blur.Value, Vars::Glow::Viewmodel::StencilScale.Value, Vars::Glow::Viewmodel::BlurScale.Value);

	SetupBegin(glow, pRenderContext, m_pMatBlurY);
	drawModel(true);

	StencilEnd(pRenderContext);

	SetupMid(pRenderContext, m_pMatGlowColor, w, h);
	auto& color = Vars::Colors::Local.Value;
	I::RenderView->SetColorModulation(float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f);
	I::RenderView->SetBlend(float(color.a) / 255.f);
	drawModel(false);

	SetupEnd(glow, pRenderContext, m_pMatBlurX, m_pMatBlurY, m_pMatHaloAddToScreen, w, h);
}



void CGlow::Init()
{
	m_pRtFullFrame = I::MaterialSystem->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);
	m_pRtFullFrame->IncrementReferenceCount();

	m_pRenderBuffer1 = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
		"glow_buffer_1",
		m_pRtFullFrame->GetActualWidth(),
		m_pRtFullFrame->GetActualHeight(),
		RT_SIZE_LITERAL,
		IMAGE_FORMAT_RGB888,
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA,
		CREATERENDERTARGETFLAGS_HDR
	);
	m_pRenderBuffer1->IncrementReferenceCount();

	m_pRenderBuffer2 = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
		"glow_buffer_2",
		m_pRtFullFrame->GetActualWidth(),
		m_pRtFullFrame->GetActualHeight(),
		RT_SIZE_LITERAL,
		IMAGE_FORMAT_RGB888,
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA,
		CREATERENDERTARGETFLAGS_HDR
	);
	m_pRenderBuffer2->IncrementReferenceCount();
}
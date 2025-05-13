#include "Materials.h"

#include "../Glow/Glow.h"
#include "../CameraWindow/CameraWindow.h"
#include "../../Configs/Configs.h"
#include "../../Binds/Binds.h"
#include <filesystem>
#include <fstream>

IMaterial* CMaterials::Create(char const* szName, KeyValues* pKV)
{
	IMaterial* pMaterial = I::MaterialSystem->CreateMaterial(szName, pKV);
	m_mMatList[pMaterial] = true;
	return pMaterial;
}

void CMaterials::Remove(IMaterial* pMaterial)
{
	if (!pMaterial)
		return;

	if (m_mMatList.contains(pMaterial))
		m_mMatList.erase(pMaterial);

	pMaterial->DecrementReferenceCount();
	pMaterial->DeleteIfUnreferenced();
	pMaterial = nullptr;
}



void CMaterials::StoreStruct(std::string sName, std::string sVMT, bool bLocked)
{
	Material_t tMaterial = {};
	tMaterial.m_sName = sName;
	tMaterial.m_sVMT = sVMT;
	tMaterial.m_bLocked = bLocked;

	m_mMaterials[FNV1A::Hash32(sName.c_str())] = tMaterial;
}

static inline void StoreVars(Material_t& tMaterial)
{
	if (tMaterial.m_bStored || !tMaterial.m_pMaterial)
		return;

	tMaterial.m_bStored = true;

	bool bFound; auto $phongtint = tMaterial.m_pMaterial->FindVar("$phongtint", &bFound, false);
	if (bFound)
		tMaterial.m_phongtint = $phongtint;
	
	auto $envmaptint = tMaterial.m_pMaterial->FindVar("$envmaptint", &bFound, false);
	if (bFound)
		tMaterial.m_envmaptint = $envmaptint;
	
	auto $invertcull = tMaterial.m_pMaterial->FindVar("$invertcull", &bFound, false);
	if (bFound && $invertcull && $invertcull->GetIntValueInternal())
		tMaterial.m_bInvertCull = true;
}

static inline void RemoveVars(Material_t& tMaterial)
{
	tMaterial.m_bStored = false;
	tMaterial.m_phongtint = nullptr;
	tMaterial.m_envmaptint = nullptr;
	tMaterial.m_bInvertCull = false;
}

static inline void ModifyKeyValues(KeyValues* pKV)
{
	pKV->SetString("$model", "1"); // prevent wacko chams

	if (!pKV->FindKey("$cloakfactor"))
	{
		pKV->SetString("$cloakpassenabled", "1");
		auto sName = pKV->GetName();
		if (sName && FNV1A::Hash32(sName) == FNV1A::Hash32Const("VertexLitGeneric"))
		{
			if (auto pProxies = pKV->FindKey("Proxies", true))
				pProxies->FindKey("invis", true);
		}
	}
}



void CMaterials::LoadMaterials()
{
	// default materials
	StoreStruct( // hacky
		"None",
			"\"UnlitGeneric\""
			"\n{"
			"\n\t$color2 \"[0 0 0]\""
			"\n\t$additive \"1\""
			"\n}",
		true);
	StoreStruct(
		"Flat",
			"\"UnlitGeneric\""
			"\n{"
			"\n\t$basetexture \"white\""
			"\n}",
		true);
	StoreStruct(
		"Shaded",
			"\"VertexLitGeneric\""
			"\n{"
			"\n\t$basetexture \"white\""
			"\n}",
		true);
	StoreStruct(
		"Wireframe",
			"\"UnlitGeneric\""
			"\n{"
			"\n\t$basetexture \"white\""
			"\n\t$wireframe \"1\""
			"\n}",
		true);
	StoreStruct(
		"Fresnel",
			"\"VertexLitGeneric\""
			"\n{"
			"\n\t$basetexture \"white\""
			"\n\t$bumpmap \"models/player/shared/shared_normal\""
			"\n\t$color2 \"[0 0 0]\""
			"\n\t$additive \"1\""
			"\n\t$phong \"1\""
			"\n\t$phongfresnelranges \"[0 0.5 1]\""
			"\n\t$envmap \"skybox/sky_dustbowl_01\""
			"\n\t$envmapfresnel \"1\""
			"\n}",
		true);
	StoreStruct(
		"Shine",
			"\"VertexLitGeneric\""
			"\n{"
			"\n\t$additive \"1\""
			"\n\t$envmap \"cubemaps/cubemap_sheen002.hdr\""
			"\n\t$envmaptint \"[1 1 1]\""
			"\n}",
		true);
	StoreStruct(
		"Tint",
			"\"VertexLitGeneric\""
			"\n{"
			"\n\t$basetexture \"models/player/shared/ice_player\""
			"\n\t$bumpmap \"models/player/shared/shared_normal\""
			"\n\t$additive \"1\""
			"\n\t$phong \"1\""
			"\n\t$phongfresnelranges \"[0 0.001 0.001]\""
			"\n\t$envmap \"skybox/sky_dustbowl_01\""
			"\n\t$envmapfresnel \"1\""
			"\n\t$selfillum \"1\""
			"\n\t$selfillumtint \"[0 0 0]\""
			"\n}",
		true);
	// user materials
	for (auto& entry : std::filesystem::directory_iterator(F::Configs.m_sMaterialsPath))
	{
		// Ignore all non-material files
		if (!entry.is_regular_file() || entry.path().extension() != std::string(".vmt"))
			continue;

		std::ifstream inStream(entry.path());
		if (!inStream.good())
			continue;

		std::string sName = entry.path().filename().string();
		sName.erase(sName.end() - 4, sName.end());
		const std::string sVMT((std::istreambuf_iterator(inStream)), std::istreambuf_iterator<char>());

		auto uHash = FNV1A::Hash32(sName.c_str());
		if (uHash == FNV1A::Hash32Const("Original") || m_mMaterials.contains(uHash))
			continue;

		StoreStruct(sName, sVMT);
	}
	// create materials
	for (auto& [_, tMaterial] : m_mMaterials)
	{
		KeyValues* kv = new KeyValues(tMaterial.m_sName.c_str());
		bool bLoad = kv->LoadFromBuffer(tMaterial.m_sName.c_str(), tMaterial.m_sVMT.c_str());
		ModifyKeyValues(kv);
			
		tMaterial.m_pMaterial = Create(tMaterial.m_sName.c_str(), kv);
		//StoreVars(tMaterial);
	}

	F::Glow.Initialize();
	F::CameraWindow.Initialize();

	S::InitializeStandardMaterials.Call<void>();
	auto pMaterial = *reinterpret_cast<IMaterial**>(U::Memory.RelToAbs(S::Wireframe()));
	pMaterial->SetMaterialVarFlag(MATERIAL_VAR_VERTEXALPHA, true);

	m_bLoaded = true;
}

void CMaterials::UnloadMaterials()
{
	m_bLoaded = false;

	for (auto& [_, mat] : m_mMaterials)
		Remove(mat.m_pMaterial);
	m_mMaterials.clear();
	m_mMatList.clear();

	F::Glow.Unload();
	F::CameraWindow.Unload();
}

void CMaterials::ReloadMaterials()
{
	UnloadMaterials();

	LoadMaterials();
}



void CMaterials::SetColor(Material_t* pMaterial, Color_t tColor)
{
	float r = float(tColor.r) / 255.f;
	float g = float(tColor.g) / 255.f;
	float b = float(tColor.b) / 255.f;
	float a = float(tColor.a) / 255.f;

	I::RenderView->SetColorModulation(r, g, b);
	I::RenderView->SetBlend(a);

	if (pMaterial)
	{
		StoreVars(*pMaterial);
		if (pMaterial->m_phongtint)
			pMaterial->m_phongtint->SetVecValue(r, g, b);
		if (pMaterial->m_envmaptint)
			pMaterial->m_envmaptint->SetVecValue(r, g, b);
	}
}



Material_t* CMaterials::GetMaterial(uint32_t uHash)
{
	if (uHash == FNV1A::Hash32Const("Original"))
		return nullptr;

	return m_mMaterials.contains(uHash) ? &m_mMaterials[uHash] : nullptr;
}

std::string CMaterials::GetVMT(uint32_t uHash)
{
	if (m_mMaterials.contains(uHash))
		return m_mMaterials[uHash].m_sVMT;

	return "";
}

void CMaterials::AddMaterial(const char* sName)
{
	auto uHash = FNV1A::Hash32(sName);
	if (uHash == FNV1A::Hash32Const("Original") || std::filesystem::exists(F::Configs.m_sMaterialsPath + sName + ".vmt") || m_mMaterials.contains(uHash))
		return;

	StoreStruct(
		sName,
			"\"VertexLitGeneric\""
			"\n{"
			"\n\t"
			"\n}"
		);
	auto& tMaterial = m_mMaterials[uHash];

	KeyValues* kv = new KeyValues(sName);
	kv->LoadFromBuffer(sName, tMaterial.m_sVMT.c_str());
	ModifyKeyValues(kv);

	tMaterial.m_pMaterial = Create(sName, kv);
	//StoreVars(tMaterial);

	std::ofstream outStream(F::Configs.m_sMaterialsPath + sName + ".vmt");
	outStream << tMaterial.m_sVMT;
	outStream.close();
}

void CMaterials::EditMaterial(const char* sName, const char* sVMT)
{
	if (!std::filesystem::exists(F::Configs.m_sMaterialsPath + sName + ".vmt"))
		return;

	m_bLoaded = false;

	auto uHash = FNV1A::Hash32(sName);
	if (m_mMaterials.contains(uHash) && !m_mMaterials[uHash].m_bLocked)
	{
		auto& tMaterial = m_mMaterials[uHash];

		Remove(tMaterial.m_pMaterial);
		RemoveVars(tMaterial);
		tMaterial.m_sVMT = sVMT;

		KeyValues* kv = new KeyValues(sName);
		kv->LoadFromBuffer(sName, sVMT);
		ModifyKeyValues(kv);

		tMaterial.m_pMaterial = Create(sName, kv);
		//StoreVars(tMaterial);

		std::ofstream outStream(F::Configs.m_sMaterialsPath + sName + ".vmt");
		outStream << sVMT;
		outStream.close();
	}

	m_bLoaded = true;
}

void CMaterials::RemoveMaterial(const char* sName)
{
	if (!std::filesystem::exists(F::Configs.m_sMaterialsPath + sName + ".vmt"))
		return;

	m_bLoaded = false;

	auto uHash = FNV1A::Hash32(sName);
	if (m_mMaterials.contains(uHash) && !m_mMaterials[uHash].m_bLocked)
	{
		Remove(m_mMaterials[uHash].m_pMaterial);
		m_mMaterials.erase(uHash);

		std::filesystem::remove(F::Configs.m_sMaterialsPath + sName + ".vmt");

		auto removeFromVar = [&](ConfigVar<std::vector<std::pair<std::string, Color_t>>>& var)
			{
				for (auto& [iBind, vVal] : var.Map)
				{
					for (auto it = vVal.begin(); it != vVal.end();)
					{
						if (FNV1A::Hash32(it->first.c_str()) == uHash)
							it = vVal.erase(it);
						else
							++it;
					}
				}
			};
		removeFromVar(Vars::Chams::Team::Visible);
		removeFromVar(Vars::Chams::Team::Occluded);
		removeFromVar(Vars::Chams::Enemy::Visible);
		removeFromVar(Vars::Chams::Enemy::Occluded);
		removeFromVar(Vars::Chams::World::Visible);
		removeFromVar(Vars::Chams::World::Occluded);
		removeFromVar(Vars::Chams::Backtrack::Visible);
		removeFromVar(Vars::Chams::FakeAngle::Visible);
		removeFromVar(Vars::Chams::Viewmodel::WeaponMaterial);
		removeFromVar(Vars::Chams::Viewmodel::HandsMaterial);
	}

	m_bLoaded = true;
}
#include "Materials.h"

#include "../Glow/Glow.h"
#include "../../CameraWindow/CameraWindow.h"
#include "../../Configs/Configs.h"
#include <filesystem>
#include <fstream>

IMaterial* CMaterials::Create(char const* szName, KeyValues* pKV)
{
		SDK::Output("LoadMaterials", "CreateMaterial", {}, false, false, false, true);
	IMaterial* pMaterial = I::MaterialSystem->CreateMaterial(szName, pKV);
	m_mMatList[pMaterial] = true;
	return pMaterial;
}

void CMaterials::Remove(IMaterial* pMaterial)
{
	if (!pMaterial)
		return;

	auto it = m_mMatList.find(pMaterial);
	if (it != m_mMatList.end())
		m_mMatList.erase(it);

	pMaterial->DecrementReferenceCount();
	pMaterial->DeleteIfUnreferenced();
	pMaterial = nullptr;
}



void CMaterials::StoreStruct(std::string sName, std::string sVMT, bool bLocked)
{
		SDK::Output("LoadMaterials", "StoreStruct", {}, false, false, false, true);
	Material_t tMaterial = {};
	tMaterial.m_sName = sName;
	tMaterial.m_sVMT = sVMT;
	tMaterial.m_bLocked = bLocked;

	m_mMaterials[FNV1A::Hash32(sName.c_str())] = tMaterial;
}

void CMaterials::StoreVars(Material_t& tMaterial)
{
		SDK::Output("LoadMaterials", "StoreVars", {}, false, false, false, true);
	if (!tMaterial.m_pMaterial)
		return;

	bool bFound; auto $phongtint = tMaterial.m_pMaterial->FindVar("$phongtint", &bFound, false);
	if (bFound)
		tMaterial.m_phongtint = $phongtint;

	auto $envmaptint = tMaterial.m_pMaterial->FindVar("$envmaptint", &bFound, false);
	if (bFound)
		tMaterial.m_envmaptint = $envmaptint;

	auto $invertcull = tMaterial.m_pMaterial->FindVar("$invertcull", &bFound, false);
	if (bFound && $invertcull && $invertcull->GetIntValueInternal())
		tMaterial.m_bInvertCull = true;

		SDK::Output("LoadMaterials", "StoreVars 2", {}, false, false, false, true);
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
	for (auto& entry : std::filesystem::directory_iterator(MaterialFolder))
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

		if (FNV1A::Hash32(sName.c_str()) == FNV1A::Hash32Const("Original"))
			continue;

		StoreStruct(sName, sVMT);
	}
	// create materials
	for (auto& [_, tMaterial] : m_mMaterials)
	{
			SDK::Output("LoadMaterials", tMaterial.m_sName.c_str(), {}, false, false, false, true);
		KeyValues* kv = new KeyValues(tMaterial.m_sName.c_str());
		kv->LoadFromBuffer(tMaterial.m_sName.c_str(), tMaterial.m_sVMT.c_str());
		ModifyKeyValues(kv);
			
		tMaterial.m_pMaterial = Create(tMaterial.m_sName.c_str(), kv);
	}
	for (auto& [_, tMaterial] : m_mMaterials)
		StoreVars(tMaterial);

		SDK::Output("LoadMaterials", "Glow.Initialize();", {}, false, false, false, true);
	F::Glow.Initialize();
		SDK::Output("LoadMaterials", "CameraWindow.Initialize();", {}, false, false, false, true);
	F::CameraWindow.Initialize();

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
	if (!pMaterial)
		return;

	float r = float(tColor.r) / 255.f;
	float g = float(tColor.g) / 255.f;
	float b = float(tColor.b) / 255.f;
	float a = float(tColor.a) / 255.f;

	if (pMaterial->m_phongtint)
		pMaterial->m_phongtint->SetVecValue(r, g, b);
	if (pMaterial->m_envmaptint)
		pMaterial->m_envmaptint->SetVecValue(r, g, b);
	I::RenderView->SetColorModulation(r, g, b);
	I::RenderView->SetBlend(a);
}



Material_t* CMaterials::GetMaterial(uint32_t uHash)
{
	if (uHash == FNV1A::Hash32Const("Original"))
		return nullptr;

	return m_mMaterials.contains(uHash) ? &m_mMaterials[uHash] : nullptr;
}

std::string CMaterials::GetVMT(uint32_t uHash)
{
	const auto cham = m_mMaterials.find(uHash);
	if (cham != m_mMaterials.end())
		return cham->second.m_sVMT;

	return "";
}

void CMaterials::AddMaterial(const char* sName)
{
	auto uHash = FNV1A::Hash32(sName);
	if (uHash == FNV1A::Hash32Const("Original") || std::filesystem::exists(std::format("{}\\{}.vmt", MaterialFolder, sName)) || m_mMaterials.contains(uHash))
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
	StoreVars(tMaterial);

	std::ofstream outStream(std::format("{}\\{}.vmt", MaterialFolder, sName));
	outStream << tMaterial.m_sVMT;
	outStream.close();
}

void CMaterials::EditMaterial(const char* sName, const char* sVMT)
{
	if (!std::filesystem::exists(std::format("{}\\{}.vmt", MaterialFolder, sName)))
		return;

	m_bLoaded = false;

	const auto cham = m_mMaterials.find(FNV1A::Hash32(sName));
	if (cham != m_mMaterials.end() && !cham->second.m_bLocked)
	{
		auto& tMaterial = cham->second;

		Remove(tMaterial.m_pMaterial);
		tMaterial.m_sVMT = sVMT;

		KeyValues* kv = new KeyValues(sName);
		kv->LoadFromBuffer(sName, sVMT);
		ModifyKeyValues(kv);

		tMaterial.m_pMaterial = Create(sName, kv);
		StoreVars(tMaterial);

		std::ofstream outStream(std::format("{}\\{}.vmt", MaterialFolder, sName));
		outStream << sVMT;
		outStream.close();
	}

	m_bLoaded = true;
}

void CMaterials::RemoveMaterial(const char* sName)
{
	if (!std::filesystem::exists(std::format("{}\\{}.vmt", MaterialFolder, sName)))
		return;

	m_bLoaded = false;

	auto uHash = FNV1A::Hash32(sName);
	const auto cham = m_mMaterials.find(uHash);
	if (cham != m_mMaterials.end() && !cham->second.m_bLocked)
	{
		Remove(cham->second.m_pMaterial);

		m_mMaterials.erase(cham);

		std::filesystem::remove(MaterialFolder + "\\" + sName + ".vmt");

		auto removeFromVar = [&](ConfigVar<std::vector<std::pair<std::string, Color_t>>>& var)
			{
				for (auto& [_, val] : var.Map)
				{
					for (auto it = val.begin(); it != val.end();)
					{
						if (FNV1A::Hash32(it->first.c_str()) == uHash)
							it = val.erase(it);
						else
							++it;
					}
				}
			};
		removeFromVar(Vars::Chams::Friendly::Visible);
		removeFromVar(Vars::Chams::Friendly::Occluded);
		removeFromVar(Vars::Chams::Enemy::Visible);
		removeFromVar(Vars::Chams::Enemy::Occluded);
		removeFromVar(Vars::Chams::World::Visible);
		removeFromVar(Vars::Chams::World::Occluded);
		removeFromVar(Vars::Chams::Backtrack::Visible);
		removeFromVar(Vars::Chams::FakeAngle::Visible);
		removeFromVar(Vars::Chams::Viewmodel::WeaponVisible);
		removeFromVar(Vars::Chams::Viewmodel::HandsVisible);
	}

	m_bLoaded = true;
}
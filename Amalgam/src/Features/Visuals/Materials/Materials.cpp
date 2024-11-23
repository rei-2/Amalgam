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

void CMaterials::LoadMaterials()
{
	// default materials
	{
		// hacky
		Material_t mat = {};
		mat.m_sName = "None";

		mat.m_sVMT = "\"UnlitGeneric\"";
		mat.m_sVMT += "\n{";
		mat.m_sVMT += "\n\t$color2 \"[0 0 0]\"";
		mat.m_sVMT += "\n\t$additive \"1\"";
		mat.m_sVMT += "\n}";

		mat.m_bLocked = true;
		m_mMaterials[FNV1A::Hash32Const("None")] = mat;
	}
	{
		Material_t mat = {};
		mat.m_sName = "Flat";

		mat.m_sVMT = "\"UnlitGeneric\"";
		mat.m_sVMT += "\n{";
		mat.m_sVMT += "\n\t$basetexture \"white\"";
		mat.m_sVMT += "\n}";

		mat.m_bLocked = true;
		m_mMaterials[FNV1A::Hash32Const("Flat")] = mat;
	}
	{
		Material_t mat = {};
		mat.m_sName = "Shaded";

		mat.m_sVMT = "\"VertexLitGeneric\"";
		mat.m_sVMT += "\n{";
		mat.m_sVMT += "\n\t$basetexture \"white\"";
		mat.m_sVMT += "\n}";

		mat.m_bLocked = true;
		m_mMaterials[FNV1A::Hash32Const("Shaded")] = mat;
	}
	{
		Material_t mat = {};
		mat.m_sName = "Wireframe";

		mat.m_sVMT = "\"UnlitGeneric\"";
		mat.m_sVMT += "\n{";
		mat.m_sVMT += "\n\t$basetexture \"white\"";
		mat.m_sVMT += "\n\t$wireframe \"1\"";
		mat.m_sVMT += "\n}";

		mat.m_bLocked = true;
		m_mMaterials[FNV1A::Hash32Const("Wireframe")] = mat;
	}
	{
		Material_t mat = {};
		mat.m_sName = "Fresnel";

		mat.m_sVMT = "\"VertexLitGeneric\"";
		mat.m_sVMT += "\n{";
		mat.m_sVMT += "\n\t$basetexture \"white\"";
		mat.m_sVMT += "\n\t$bumpmap \"models/player/shared/shared_normal\"";
		mat.m_sVMT += "\n\t$color2 \"[0 0 0]\"";
		mat.m_sVMT += "\n\t$additive \"1\"";
		mat.m_sVMT += "\n\t$phong \"1\"";
		mat.m_sVMT += "\n\t$phongfresnelranges \"[0 0.5 1]\"";
		mat.m_sVMT += "\n\t$envmap \"skybox/sky_dustbowl_01\"";
		mat.m_sVMT += "\n\t$envmapfresnel \"1\"";
		mat.m_sVMT += "\n}";

		mat.m_bLocked = true;
		m_mMaterials[FNV1A::Hash32Const("Fresnel")] = mat;
	}
	{
		Material_t mat = {};
		mat.m_sName = "Shine";

		mat.m_sVMT = "\"VertexLitGeneric\"";
		mat.m_sVMT += "\n{";
		mat.m_sVMT += "\n\t$additive \"1\"";
		mat.m_sVMT += "\n\t$envmap \"cubemaps/cubemap_sheen002.hdr\"";
		mat.m_sVMT += "\n\t$envmaptint \"[1 1 1]\"";
		mat.m_sVMT += "\n}";

		mat.m_bLocked = true;
		m_mMaterials[FNV1A::Hash32Const("Shine")] = mat;
	}
	{
		Material_t mat = {};
		mat.m_sName = "Tint";

		mat.m_sVMT = "\"VertexLitGeneric\"";
		mat.m_sVMT += "\n{";
		mat.m_sVMT += "\n\t$basetexture \"models/player/shared/ice_player\"";
		mat.m_sVMT += "\n\t$bumpmap \"models/player/shared/shared_normal\"";
		mat.m_sVMT += "\n\t$additive \"1\"";
		mat.m_sVMT += "\n\t$phong \"1\"";
		mat.m_sVMT += "\n\t$phongfresnelranges \"[0 0.001 0.001]\"";
		mat.m_sVMT += "\n\t$envmap \"skybox/sky_dustbowl_01\"";
		mat.m_sVMT += "\n\t$envmapfresnel \"1\"";
		mat.m_sVMT += "\n\t$selfillum \"1\"";
		mat.m_sVMT += "\n\t$selfillumtint \"[0 0 0]\"";
		mat.m_sVMT += "\n}";

		mat.m_bLocked = true;
		m_mMaterials[FNV1A::Hash32Const("Tint")] = mat;
	}
	// user materials
	for (auto& entry : std::filesystem::directory_iterator(MaterialFolder))
	{
		// Ignore all non-Material files
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

		Material_t mat = {};
		mat.m_sName = sName;
		mat.m_sVMT = sVMT;

		m_mMaterials[FNV1A::Hash32(sName.c_str())] = mat;
	}
	// create materials
	for (auto& [_, mat] : m_mMaterials)
	{
			SDK::Output("LoadMaterials", mat.m_sName.c_str(), {}, false, false, false, true);
		KeyValues* kv = new KeyValues(mat.m_sName.c_str());
		kv->LoadFromBuffer(mat.m_sName.c_str(), mat.m_sVMT.c_str());
		kv->SetString("$model", "1"); // prevent wacko chams
		mat.m_pMaterial = Create(std::format("m_pMat%s", mat.m_sName).c_str(), kv);
	}

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

void CMaterials::SetColor(IMaterial* material, Color_t color)
{
	if (material)
	{
		bool bFound; auto $phongtint = material->FindVar("$phongtint", &bFound, false);
		if (bFound && $phongtint)
			$phongtint->SetVecValue(float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f);

		auto $envmaptint = material->FindVar("$envmaptint", &bFound, false);
		if (bFound && $envmaptint)
			$envmaptint->SetVecValue(float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f);
	}
	I::RenderView->SetColorModulation(float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f);
	I::RenderView->SetBlend(float(color.a) / 255.f);
}



IMaterial* CMaterials::GetMaterial(uint32_t uHash)
{
	if (uHash == FNV1A::Hash32Const("Original"))
		return nullptr;

	return m_mMaterials.contains(uHash) ? m_mMaterials[uHash].m_pMaterial : nullptr;
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

	Material_t mat = {};
	mat.m_sVMT = "\"VertexLitGeneric\"\n{\n\t\n}";

	KeyValues* kv = new KeyValues(sName);
	kv->LoadFromBuffer(sName, mat.m_sVMT.c_str());
	kv->SetString("$model", "1");
	mat.m_pMaterial = Create(std::format("m_pMat%s", sName).c_str(), kv);

	m_mMaterials[uHash] = mat;

	std::ofstream outStream(std::format("{}\\{}.vmt", MaterialFolder, sName));
	outStream << mat.m_sVMT;
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
		Remove(cham->second.m_pMaterial);

		cham->second.m_sVMT = sVMT;

		KeyValues* kv = new KeyValues(sName);
		kv->LoadFromBuffer(sName, sVMT);
		kv->SetString("$model", "1");
		cham->second.m_pMaterial = Create(std::format("m_pMat%s", sName).c_str(), kv);

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
#include "Materials.h"

#include "../../Configs/Configs.h"
#include <filesystem>
#include <fstream>

IMaterial* CMaterials::CreateNRef(char const* szName, KeyValues* pKV)
{
	IMaterial* returnMaterial = I::MaterialSystem->CreateMaterial(szName, pKV);
	returnMaterial->IncrementReferenceCount();
	mMatList[returnMaterial] = true;
	return returnMaterial;
}

void CMaterials::LoadMaterials()
{
	// cham materials

	// default materials
	{	// hacky
		Material_t mat = {}; // None

		mat.sVMT = "\"UnlitGeneric\"";
		mat.sVMT += "\n{";
		mat.sVMT += "\n	$color2 \"[0 0 0]\"";
		mat.sVMT += "\n	$additive \"1\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		mChamMaterials["None"] = mat;
	}
	{
		Material_t mat = {}; // Flat

		mat.sVMT = "\"UnlitGeneric\"";
		mat.sVMT += "\n{";
		mat.sVMT += "\n	$basetexture \"vgui/white_additive\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		mChamMaterials["Flat"] = mat;
	}
	{
		Material_t mat = {}; // Shaded

		mat.sVMT = "\"VertexLitGeneric\"";
		mat.sVMT += "\n{";
		mat.sVMT += "\n	$basetexture \"vgui/white_additive\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		mChamMaterials["Shaded"] = mat;
	}
	{
		Material_t mat = {}; // Wireframe

		mat.sVMT = "\"UnlitGeneric\"";
		mat.sVMT += "\n{";
		mat.sVMT += "\n	$basetexture \"vgui/white_additive\"";
		mat.sVMT += "\n	$wireframe \"1\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		mChamMaterials["Wireframe"] = mat;
	}
	{
		Material_t mat = {}; // Fresnel

		mat.sVMT = "\"VertexLitGeneric\"";
		mat.sVMT += "\n{";
		mat.sVMT += "\n	$basetexture \"vgui/white_additive\"";
		mat.sVMT += "\n	$bumpmap \"models/player/shared/shared_normal\"";
		mat.sVMT += "\n	$color2 \"[0 0 0]\"";
		mat.sVMT += "\n	$additive \"1\"";
		mat.sVMT += "\n	$phong \"1\"";
		mat.sVMT += "\n	$phongfresnelranges \"[0 0.5 1]\"";
		mat.sVMT += "\n	$envmap \"skybox/sky_dustbowl_01\"";
		mat.sVMT += "\n	$envmapfresnel \"1\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		mChamMaterials["Fresnel"] = mat;
	}
	{
		Material_t mat = {}; // Shine

		mat.sVMT = "\"VertexLitGeneric\"";
		mat.sVMT += "\n{";
		mat.sVMT += "\n	$additive \"1\"";
		mat.sVMT += "\n	$envmap \"cubemaps/cubemap_sheen002.hdr\"";
		mat.sVMT += "\n	$envmaptint \"[1 1 1]\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		mChamMaterials["Shine"] = mat;
	}
	{
		Material_t mat = {}; // Tint

		mat.sVMT = "\"VertexLitGeneric\"";
		mat.sVMT += "\n{";
		mat.sVMT += "\n	$basetexture \"models/player/shared/ice_player\"";
		mat.sVMT += "\n	$bumpmap \"models/player/shared/shared_normal\"";
		mat.sVMT += "\n	$additive \"1\"";
		mat.sVMT += "\n	$phong \"1\"";
		mat.sVMT += "\n	$phongfresnelranges \"[0 0.001 0.001]\"";
		mat.sVMT += "\n	$envmap \"skybox/sky_dustbowl_01\"";
		mat.sVMT += "\n	$envmapfresnel \"1\"";
		mat.sVMT += "\n	$selfillum \"1\"";
		mat.sVMT += "\n	$selfillumtint \"[0 0 0]\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		mChamMaterials["Tint"] = mat;
	}
	// user materials
	for (const auto& entry : std::filesystem::directory_iterator(MaterialFolder))
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

		if (FNV1A::Hash(sName.c_str()) == FNV1A::HashConst("Original"))
			continue;

		Material_t mat = {};
		mat.sVMT = sVMT;

		mChamMaterials[sName] = mat;
	}
	// create materials
	for (auto& [sName, mat] : mChamMaterials)
	{
		const auto kv = new KeyValues(sName.c_str());
		kv->LoadFromBuffer(sName.c_str(), mat.sVMT.c_str());
		mat.pMaterial = CreateNRef(std::format("m_pMat%s", sName).c_str(), kv);
	}

	// glow materials

	{
		Material_t mat = {}; // GlowColor

		mat.pMaterial = I::MaterialSystem->FindMaterial("dev/glow_color", TEXTURE_GROUP_OTHER);
		mat.pMaterial->IncrementReferenceCount();

		mGlowMaterials["GlowColor"] = mat;
	}
	{
		Material_t mat = {}; // BlurX

		KeyValues* m_pMatBlurXKV = new KeyValues("BlurFilterX");
		m_pMatBlurXKV->SetString("$basetexture", "glow_buffer_1");
		mat.pMaterial = CreateNRef("m_pMatBlurX", m_pMatBlurXKV);

		mGlowMaterials["BlurX"] = mat;
	}
	{
		Material_t mat = {}; // BlurY

		KeyValues* m_pMatBlurYKV = new KeyValues("BlurFilterY");
		m_pMatBlurYKV->SetString("$basetexture", "glow_buffer_2");
		mat.pMaterial = CreateNRef("m_pMatBlurY", m_pMatBlurYKV);

		mGlowMaterials["BlurY"] = mat;
	}
	{
		Material_t mat = {}; // HaloAddToScreen

		KeyValues* m_pMatHaloAddToScreenKV = new KeyValues("UnlitGeneric");
		m_pMatHaloAddToScreenKV->SetString("$basetexture", "glow_buffer_1");
		m_pMatHaloAddToScreenKV->SetString("$additive", "1");
		mat.pMaterial = CreateNRef("m_pMatHaloAddToScreen", m_pMatHaloAddToScreenKV);

		mGlowMaterials["HaloAddToScreen"] = mat;
	}
}

void CMaterials::UnloadMaterials()
{
	for (auto& [_, mat] : mChamMaterials)
	{
		if (!mat.pMaterial)
			continue;

		mat.pMaterial->DecrementReferenceCount();
		mat.pMaterial->DeleteIfUnreferenced();
		mat.pMaterial = nullptr;
	}
	for (auto& [_, mat] : mGlowMaterials)
	{
		if (!mat.pMaterial)
			continue;

		mat.pMaterial->DecrementReferenceCount();
		mat.pMaterial->DeleteIfUnreferenced();
		mat.pMaterial = nullptr;
	}
	mChamMaterials.clear();
	mGlowMaterials.clear();
	mMatList.clear();
}

void CMaterials::ReloadMaterials()
{
	UnloadMaterials();

	LoadMaterials();
}

void CMaterials::SetColor(IMaterial* material, Color_t color, bool bSetColor)
{
	if (bSetColor)
	{
		if (material)
		{
			if (auto $phongtint = material->FindVar("$phongtint", nullptr, false))
				$phongtint->SetVecValue(float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f);
			if (auto $envmaptint = material->FindVar("$envmaptint", nullptr, false))
				$envmaptint->SetVecValue(float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f);
		}
		I::RenderView->SetColorModulation(float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f);
		I::RenderView->SetBlend(float(color.a) / 255.f);
	}
	else
	{
		if (material)
		{
			if (auto $phongtint = material->FindVar("$phongtint", nullptr, false))
				$phongtint->SetVecValue(1.f, 1.f, 1.f);
			if (auto $envmaptint = material->FindVar("$envmaptint", nullptr, false))
				$envmaptint->SetVecValue(1.f, 1.f, 1.f);
		}
		I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
		I::RenderView->SetBlend(1.f);
	}
}



IMaterial* CMaterials::GetMaterial(std::string sName)
{
	if (FNV1A::Hash(sName.c_str()) == FNV1A::HashConst("Original"))
		return nullptr;

	return mChamMaterials.contains(sName) ? mChamMaterials[sName].pMaterial : nullptr;
}

std::string CMaterials::GetVMT(std::string sName)
{
	const auto cham = mChamMaterials.find(sName);
	if (cham != mChamMaterials.end())
		return cham->second.sVMT;

	return "";
}

void CMaterials::AddMaterial(std::string sName)
{
	if (FNV1A::Hash(sName.c_str()) == FNV1A::HashConst("Original") || std::filesystem::exists(MaterialFolder + "\\" + sName + ".vmt") || mChamMaterials.contains(sName))
		return;

	Material_t mat = {};
	mat.sVMT = "\"VertexLitGeneric\"\n{\n\t\n}";

	const auto kv = new KeyValues(sName.c_str());
	kv->LoadFromBuffer(sName.c_str(), mat.sVMT.c_str());
	mat.pMaterial = CreateNRef(std::format("m_pMat%s", sName).c_str(), kv);

	mChamMaterials[sName] = mat;

	std::ofstream outStream(MaterialFolder + "\\" + sName + ".vmt");
	outStream << mat.sVMT;
	outStream.close();
}

void CMaterials::EditMaterial(std::string sName, std::string sVMT)
{
	if (!std::filesystem::exists(MaterialFolder + "\\" + sName + ".vmt"))
		return;

	const auto cham = mChamMaterials.find(sName);
	if (cham != mChamMaterials.end())
	{
		if (cham->second.bLocked)
			return;

		cham->second.sVMT = sVMT;

		const auto kv = new KeyValues(sName.c_str());
		kv->LoadFromBuffer(sName.c_str(), sVMT.c_str());
		cham->second.pMaterial = CreateNRef(std::format("m_pMat%s", sName).c_str(), kv);

		std::ofstream outStream(MaterialFolder + "\\" + sName + ".vmt");
		outStream << sVMT;
		outStream.close();
	}
}

void CMaterials::RemoveMaterial(std::string sName)
{
	if (!std::filesystem::exists(MaterialFolder + "\\" + sName + ".vmt"))
		return;

	const auto cham = mChamMaterials.find(sName);
	if (cham != mChamMaterials.end())
	{
		if (cham->second.bLocked)
			return;

		if (cham->second.pMaterial)
		{
			cham->second.pMaterial->DecrementReferenceCount();
			cham->second.pMaterial->DeleteIfUnreferenced();
			cham->second.pMaterial = nullptr;
		}

		mChamMaterials.erase(cham);
	}

	std::filesystem::remove(MaterialFolder + "\\" + sName + ".vmt");

	auto removeFromVar = [sName](ConfigVar<std::vector<std::string>>& var)
		{
			for (auto& [_, val] : var.Map)
			{
				for (auto it = val.begin(); it != val.end();)
				{
					if (*it == sName)
						it = val.erase(it);
					else
						++it;
				}
			}
		};
	removeFromVar(Vars::Chams::Friendly::VisibleMaterial);
	removeFromVar(Vars::Chams::Friendly::OccludedMaterial);
	removeFromVar(Vars::Chams::Enemy::VisibleMaterial);
	removeFromVar(Vars::Chams::Enemy::OccludedMaterial);
	removeFromVar(Vars::Chams::World::VisibleMaterial);
	removeFromVar(Vars::Chams::World::OccludedMaterial);
	removeFromVar(Vars::Chams::Backtrack::VisibleMaterial);
	removeFromVar(Vars::Chams::FakeAngle::VisibleMaterial);
	removeFromVar(Vars::Chams::Viewmodel::VisibleMaterial);
}
#include "Binds.h"

#include "../ImGui/Menu/Menu.h"
#include <functional>

#define IsType(type) var->m_iType == typeid(type).hash_code()
#define SetType(type, cond)\
{\
	if (var->As<type>()->Map.contains(cond))\
		var->As<type>()->Value = var->As<type>()->Map[cond];\
}
#define SetT(type, cond) if (IsType(type)) SetType(type, cond)

void CBinds::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (G::Unload)
		return;

	auto setVars = [](int iBind)
		{
			const bool bDefault = iBind == DEFAULT_BIND;
			for (auto var : g_Vars)
			{
				if (var->m_iFlags & (NOSAVE | NOBIND) && !bDefault)
					continue;

				SetT(bool, iBind)
				else SetT(int, iBind)
				else SetT(float, iBind)
				else SetT(IntRange_t, iBind)
				else SetT(FloatRange_t, iBind)
				else SetT(std::string, iBind)
				else SetT(VA_LIST(std::vector<std::pair<std::string, Color_t>>), iBind)
				else SetT(Color_t, iBind)
				else SetT(Gradient_t, iBind)
				else SetT(Vec3, iBind)
				else SetT(DragBox_t, iBind)
				else SetT(WindowBox_t, iBind)
			}
		};
	setVars(DEFAULT_BIND);

	std::function<void(int)> getBinds = [&](int iParent)
		{
			for (auto it = vBinds.rbegin(); it != vBinds.rend(); it++) // reverse so that higher binds have priority over vars
			{
				int iBind = std::distance(vBinds.begin(), it.base()) - 1;
				auto& tBind = *it;
				if (iParent != tBind.Parent)
					continue;

				switch (tBind.Type)
				{
				// key
				case 0:
				{
					bool bKey = false;
					switch (tBind.Info)
					{
					case 0: bKey = U::KeyHandler.Down(tBind.Key, true, &tBind.Storage); break;
					case 1: bKey = U::KeyHandler.Pressed(tBind.Key, true, &tBind.Storage); break;
					case 2: bKey = U::KeyHandler.Double(tBind.Key, true, &tBind.Storage); break;
					}
					const bool bShouldUse = !I::EngineVGui->IsGameUIVisible() && (!I::MatSystemSurface->IsCursorVisible() || I::EngineClient->IsPlayingDemo())
											|| F::Menu.IsOpen && !ImGui::GetIO().WantTextInput && !F::Menu.InKeybind; // allow in menu
					bKey = bShouldUse && bKey;

					switch (tBind.Info)
					{
					case 0:
						if (tBind.Not)
							bKey = !bKey;
						tBind.Active = bKey;
						break;
					case 1:
					case 2:
						if (bKey)
							tBind.Active = !tBind.Active;
					}
					break;
				}
				// class
				case 1:
				{
					const int iClass = pLocal ? pLocal->m_iClass() : 0;
					switch (tBind.Info)
					{
					case 0: { tBind.Active = iClass == 1; break; }
					case 1: { tBind.Active = iClass == 3; break; }
					case 2: { tBind.Active = iClass == 7; break; }
					case 3: { tBind.Active = iClass == 4; break; }
					case 4: { tBind.Active = iClass == 6; break; }
					case 5: { tBind.Active = iClass == 9; break; }
					case 6: { tBind.Active = iClass == 5; break; }
					case 7: { tBind.Active = iClass == 2; break; }
					case 8: { tBind.Active = iClass == 8; break; }
					}
					if (tBind.Not)
						tBind.Active = !tBind.Active;
					break;
				}
				// weapon type
				case 2:
				{
					tBind.Active = tBind.Info + 1 == int(SDK::GetWeaponType(pWeapon));
					if (tBind.Not)
						tBind.Active = !tBind.Active;
				}
				}

				if (tBind.Active)
				{
					setVars(iBind);
					getBinds(iBind);
				}
			}
		};
	getBinds(DEFAULT_BIND);
}



bool CBinds::GetBind(int iID, Bind_t* pBind)
{
	if (iID > DEFAULT_BIND && iID < vBinds.size())
	{
		*pBind = vBinds[iID];
		return true;
	}

	return false;
}

bool CBinds::HasChildren(int iBind)
{
	auto it = std::ranges::find_if(vBinds, [&](const auto& tBind) { return iBind == tBind.Parent; });
	return it != vBinds.end();
}

int CBinds::GetParent(int iBind)
{
	if (iBind > DEFAULT_BIND && iBind < vBinds.size())
		return vBinds[iBind].Parent;
	return DEFAULT_BIND;
}

void CBinds::AddBind(int iBind, Bind_t tBind)
{
	if (iBind == DEFAULT_BIND || iBind >= vBinds.size())
		vBinds.push_back(tBind);
	else
		vBinds[iBind] = tBind;
}

#define HasType(type, bind) IsType(type) && var->As<type>()->Map.find(bind) != var->As<type>()->Map.end()

#define RemoveType(type, bind)\
{\
	std::unordered_map<int, type> mMap = {};\
	for (auto it = var->As<type>()->Map.begin(); it != var->As<type>()->Map.end(); it++)\
	{\
		int iKey = it->first;\
		auto tVal = it->second;\
		if (bind == iKey)\
			continue;\
		else if (bind < iKey)\
			iKey--;\
		mMap[iKey] = tVal;\
	}\
	var->As<type>()->Map = mMap;\
}
#define RemoveT(type, bind) if (IsType(type)) RemoveType(type, bind)

void CBinds::RemoveBind(int iBind, bool bForce)
{
	if (!bForce)
	{
		for (auto var : g_Vars)
		{
			if (HasType(bool, iBind)
				|| HasType(int, iBind)
				|| HasType(float, iBind)
				|| HasType(IntRange_t, iBind)
				|| HasType(FloatRange_t, iBind)
				|| HasType(std::string, iBind)
				|| HasType(VA_LIST(std::vector<std::pair<std::string, Color_t>>), iBind)
				|| HasType(Color_t, iBind)
				|| HasType(Gradient_t, iBind)
				|| HasType(Vec3, iBind)
				|| HasType(DragBox_t, iBind)
				|| HasType(WindowBox_t, iBind))
				return;
		}
		for (auto& tBind : F::Binds.vBinds)
		{
			if (tBind.Parent == iBind)
				return;
		}
	}

	auto removeBind = [&](int iIndex)
		{
			auto it = vBinds.begin() + iIndex;
			if (it != vBinds.end())
				vBinds.erase(it);
			for (auto& tBind : vBinds)
			{
				if (tBind.Parent != DEFAULT_BIND && tBind.Parent > iIndex)
					tBind.Parent--;
			}

			for (auto var : g_Vars)
			{
				RemoveT(bool, iIndex)
				else RemoveT(int, iIndex)
				else RemoveT(float, iIndex)
				else RemoveT(IntRange_t, iIndex)
				else RemoveT(FloatRange_t, iIndex)
				else RemoveT(std::string, iIndex)
				else RemoveT(VA_LIST(std::vector<std::pair<std::string, Color_t>>), iIndex)
				else RemoveT(Color_t, iIndex)
				else RemoveT(Gradient_t, iIndex)
				else RemoveT(Vec3, iIndex)
				else RemoveT(DragBox_t, iIndex)
				else RemoveT(WindowBox_t, iIndex)
			}
		};

	std::function<void(int)> searchBinds = [&](int iIndex)
		{
			for (auto it = vBinds.begin(); it != vBinds.end(); it++)
			{
				if (iIndex == it->Parent)
					searchBinds(std::distance(vBinds.begin(), it));
			}
			removeBind(iIndex);
		};
	searchBinds(iBind);
}
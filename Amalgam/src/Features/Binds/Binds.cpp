#include "Binds.h"

#include "../ImGui/Menu/Menu.h"
#include "../Configs/Configs.h"
#include <functional>

#define IsType(type) pVar->m_iType == typeid(type).hash_code()
#define SetType(type, cond)\
{\
	if (pVar->As<type>()->Map.contains(cond))\
		pVar->As<type>()->Value = pVar->As<type>()->Map[cond];\
}
#define SetT(type, cond) if (IsType(type)) SetType(type, cond)

static inline void LoopVars(int iBind, std::vector<BaseVar*>& vVars = G::Vars)
{
	const bool bDefault = iBind == DEFAULT_BIND;
	for (auto pVar : vVars)
	{
		if (pVar->m_iFlags & (NOSAVE | NOBIND) && !bDefault)
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
}

static inline void GetBinds(int iParent, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, std::vector<Bind_t>& vBinds, bool bManage = true)
{
	if (vBinds.empty())
		return;

	for (int i = int(vBinds.size() - 1); i >= 0; i--) // reverse so higher binds have priority over vars
	{
		auto& tBind = vBinds[i];
		if (iParent != tBind.m_iParent || !tBind.m_bEnabled)
			continue;

		if (bManage)
		{
			switch (tBind.m_iType)
			{
			case BindEnum::Key:
			{
				bool bKey = false;
				switch (tBind.m_iInfo)
				{
				case BindEnum::KeyEnum::Hold: bKey = U::KeyHandler.Down(tBind.m_iKey, false, &tBind.m_tKeyStorage); break;
				case BindEnum::KeyEnum::Toggle: bKey = U::KeyHandler.Pressed(tBind.m_iKey, false, &tBind.m_tKeyStorage); break;
				case BindEnum::KeyEnum::DoubleClick: bKey = U::KeyHandler.Double(tBind.m_iKey, false, &tBind.m_tKeyStorage); break;
				}
				const bool bShouldUse = !I::EngineVGui->IsGameUIVisible() && (!I::MatSystemSurface->IsCursorVisible() || I::EngineClient->IsPlayingDemo())
					|| F::Menu.m_bIsOpen && !ImGui::GetIO().WantTextInput && !F::Menu.m_bInKeybind && (!F::Menu.m_bWindowHovered || tBind.m_iKey != VK_LBUTTON && tBind.m_iKey != VK_RBUTTON); // allow in menu
				bKey = bShouldUse && bKey;

				switch (tBind.m_iInfo)
				{
				case BindEnum::KeyEnum::Hold:
					if (tBind.m_bNot)
						bKey = !bKey;
					tBind.m_bActive = bKey;
					break;
				case BindEnum::KeyEnum::Toggle:
				case BindEnum::KeyEnum::DoubleClick:
					if (bKey)
						tBind.m_bActive = !tBind.m_bActive;
				}
				break;
			}
			case BindEnum::Class:
			{
				const int iClass = pLocal ? pLocal->m_iClass() : TF_CLASS_UNDEFINED;
				switch (tBind.m_iInfo)
				{
				case BindEnum::ClassEnum::Scout: { tBind.m_bActive = iClass == TF_CLASS_SCOUT; break; }
				case BindEnum::ClassEnum::Soldier: { tBind.m_bActive = iClass == TF_CLASS_SOLDIER; break; }
				case BindEnum::ClassEnum::Pyro: { tBind.m_bActive = iClass == TF_CLASS_PYRO; break; }
				case BindEnum::ClassEnum::Demoman: { tBind.m_bActive = iClass == TF_CLASS_DEMOMAN; break; }
				case BindEnum::ClassEnum::Heavy: { tBind.m_bActive = iClass == TF_CLASS_HEAVY; break; }
				case BindEnum::ClassEnum::Engineer: { tBind.m_bActive = iClass == TF_CLASS_ENGINEER; break; }
				case BindEnum::ClassEnum::Medic: { tBind.m_bActive = iClass == TF_CLASS_MEDIC; break; }
				case BindEnum::ClassEnum::Sniper: { tBind.m_bActive = iClass == TF_CLASS_SNIPER; break; }
				case BindEnum::ClassEnum::Spy: { tBind.m_bActive = iClass == TF_CLASS_SPY; break; }
				}
				if (tBind.m_bNot)
					tBind.m_bActive = !tBind.m_bActive;
				break;
			}
			case BindEnum::WeaponType:
			{
				if (tBind.m_iInfo != BindEnum::WeaponTypeEnum::Throwable)
					tBind.m_bActive = tBind.m_iInfo + 1 == int(SDK::GetWeaponType(pWeapon));
				else
					tBind.m_bActive = G::Throwing;
				if (tBind.m_bNot)
					tBind.m_bActive = !tBind.m_bActive;
				break;
			}
			case BindEnum::ItemSlot:
			{
				tBind.m_bActive = tBind.m_iInfo == (pWeapon ? pWeapon->GetSlot() : -1);
				if (tBind.m_bNot)
					tBind.m_bActive = !tBind.m_bActive;
				break;
			}
			}
		}

		if (tBind.m_bActive)
		{
			LoopVars(i, tBind.m_vVars);
			GetBinds(i, pLocal, pWeapon, vBinds, bManage);
		}
	}
}

void CBinds::SetVars(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, bool bManage)
{
	LoopVars(DEFAULT_BIND);
	GetBinds(DEFAULT_BIND, pLocal, pWeapon, m_vBinds, bManage);
}

void CBinds::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (G::Unload)
		return;

	for (auto it = m_vBinds.begin(); it < m_vBinds.end(); it++)
	{	// don't delay inputs for binds
		auto& tBind = *it;
		if (tBind.m_iType != BindEnum::Key)
			continue;

		auto& tKey = tBind.m_tKeyStorage;

		bool bOldIsDown = tKey.m_bIsDown;
		bool bOldIsPressed = tKey.m_bIsPressed;
		bool bOldIsDouble = tKey.m_bIsDouble;
		bool bOldIsReleased = tKey.m_bIsReleased;

		U::KeyHandler.StoreKey(tBind.m_iKey, &tKey);

		tKey.m_bIsDown = tKey.m_bIsDown || bOldIsDown;
		tKey.m_bIsPressed = tKey.m_bIsPressed || bOldIsPressed;
		tKey.m_bIsDouble = tKey.m_bIsDouble || bOldIsDouble;
		tKey.m_bIsReleased = tKey.m_bIsReleased || bOldIsReleased;
	}

	SetVars(pLocal, pWeapon);

	for (auto it = m_vBinds.begin(); it < m_vBinds.end(); it++)
	{	// clear inputs for binds
		auto& tBind = *it;
		if (tBind.m_iType != BindEnum::Key)
			continue;

		auto& tKey = tBind.m_tKeyStorage;

		U::KeyHandler.StoreKey(tBind.m_iKey, &tKey);
	}
}



bool CBinds::GetBind(int iID, Bind_t* pBind)
{
	if (iID > DEFAULT_BIND && iID < m_vBinds.size())
	{
		*pBind = m_vBinds[iID];
		return true;
	}

	return false;
}

void CBinds::AddBind(int iBind, Bind_t& tBind)
{
	if (iBind == DEFAULT_BIND || iBind >= m_vBinds.size())
		m_vBinds.push_back(tBind);
	else
		m_vBinds[iBind] = tBind;
}

#define HasType(type, bind) IsType(type) && pVar->As<type>()->contains(bind)

#define RemoveType(type, bind)\
{\
	std::unordered_map<int, type> mMap = {};\
	for (auto it = pVar->As<type>()->Map.begin(); it != pVar->As<type>()->Map.end(); it++)\
	{\
		int iKey = it->first;\
		auto tVal = it->second;\
		if (bind == iKey)\
			continue;\
		else if (bind < iKey)\
			iKey--;\
		mMap[iKey] = tVal;\
	}\
	pVar->As<type>()->Map = mMap;\
}
#define RemoveT(type, bind) if (IsType(type)) RemoveType(type, bind)

void CBinds::RemoveBind(int iBind, bool bForce)
{
	if (!bForce)
	{
		for (auto pVar : G::Vars)
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
		for (auto& tBind : F::Binds.m_vBinds)
		{
			if (tBind.m_iParent == iBind)
				return;
		}
	}

	std::vector<int> vErases = {};
	std::function<void(int)> searchBinds = [&](int iIndex)
		{
			for (auto it = m_vBinds.begin(); it < m_vBinds.end(); it++)
			{
				int iIndex2 = std::distance(m_vBinds.begin(), it);
				if (iIndex == it->m_iParent && iIndex != iIndex2)
					searchBinds(iIndex2);
			}
			vErases.push_back(iIndex);
		};
	auto removeBind = [&](int iIndex)
		{
			if (iIndex < m_vBinds.size())
				m_vBinds.erase(std::next(m_vBinds.begin(), iIndex));
			for (auto& tBind : m_vBinds)
			{
				if (tBind.m_iParent != DEFAULT_BIND && tBind.m_iParent > iIndex)
					tBind.m_iParent--;
			}

			for (auto pVar : G::Vars)
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
	searchBinds(iBind);
	std::sort(vErases.begin(), vErases.end(), [&](const int a, const int b) -> bool
		{
			return a > b;
		});
	for (auto iIndex : vErases)
		removeBind(iIndex);
}

int CBinds::GetParent(int iBind)
{
	if (iBind > DEFAULT_BIND && iBind < m_vBinds.size())
		return m_vBinds[iBind].m_iParent;
	return DEFAULT_BIND;
}

bool CBinds::HasChildren(int iBind)
{
	auto it = std::ranges::find_if(m_vBinds, [&](const auto& tBind) { return iBind == tBind.m_iParent; });
	return it != m_vBinds.end();
}

bool CBinds::WillBeEnabled(int iBind)
{
	Bind_t tBind;
	while (GetBind(iBind, &tBind))
	{
		if (!tBind.m_bEnabled)
			return false;
		iBind = tBind.m_iParent;
	}
	return true;
}
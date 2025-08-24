#include "Binds.h"

#include "../ImGui/Menu/Menu.h"
#include "../Configs/Configs.h"
#include <functional>

#define IsType(t) pBase->m_iType == typeid(t).hash_code()

template <class T>
static inline void SetMain(BaseVar*& pBase, int iBind)
{
	auto pVar = pBase->As<T>();

	if (pVar->Map.contains(iBind))
		pVar->Value = pVar->Map[iBind];
}
#define Set(t, b) if (IsType(t)) SetMain<t>(pBase, b);

static inline void LoopVars(int iBind, std::vector<BaseVar*>& vVars = G::Vars)
{
	const bool bDefault = iBind == DEFAULT_BIND;
	for (auto pBase : vVars)
	{
		if (pBase->m_iFlags & (NOSAVE | NOBIND) && !bDefault)
			continue;

		Set(bool, iBind)
		else Set(int, iBind)
		else Set(float, iBind)
		else Set(IntRange_t, iBind)
		else Set(FloatRange_t, iBind)
		else Set(std::string, iBind)
		else Set(VA_LIST(std::vector<std::pair<std::string, Color_t>>), iBind)
		else Set(Color_t, iBind)
		else Set(Gradient_t, iBind)
		else Set(Vec3, iBind)
		else Set(DragBox_t, iBind)
		else Set(WindowBox_t, iBind)
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

#define HasType(t, b) IsType(t) && pBase->As<t>()->contains(b)

template <class T>
static inline void RemoveMain(BaseVar*& pBase, int iBind)
{
	auto pVar = pBase->As<T>();

	std::unordered_map<int, T> mMap = {};
	for (auto it = pVar->Map.begin(); it != pVar->Map.end(); it++)
	{
		int iKey = it->first;
		auto tVal = it->second;
		if (iBind == iKey)
			continue;
		else if (iBind < iKey)
			iKey--;
		mMap[iKey] = tVal;
	}
	pVar->Map = mMap;
}
#define Remove(t, b) if (IsType(t)) RemoveMain<t>(pBase, b);

void CBinds::RemoveBind(int iBind, bool bForce)
{
	if (!bForce)
	{
		for (auto& pBase : G::Vars)
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

			for (auto& pBase : G::Vars)
			{
				Remove(bool, iIndex)
				else Remove(int, iIndex)
				else Remove(float, iIndex)
				else Remove(IntRange_t, iIndex)
				else Remove(FloatRange_t, iIndex)
				else Remove(std::string, iIndex)
				else Remove(VA_LIST(std::vector<std::pair<std::string, Color_t>>), iIndex)
				else Remove(Color_t, iIndex)
				else Remove(Gradient_t, iIndex)
				else Remove(Vec3, iIndex)
				else Remove(DragBox_t, iIndex)
				else Remove(WindowBox_t, iIndex)
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

template <class T>
static inline void SwapMain(BaseVar*& pBase, int iBind1, int iBind2)
{
	auto pVar = pBase->As<T>();

	bool bHas1 = pVar->contains(iBind1), bHas2 = pVar->contains(iBind2);
	if (bHas1 && bHas2)
	{
		auto& tVal1 = pVar->Map[iBind1];
		auto& tVal2 = pVar->Map[iBind2];
		auto tTemp = tVal1;
		tVal1 = tVal2;
		tVal2 = tTemp;
	}
	else if (bHas1)
	{
		pVar->Map[iBind2] = pVar->Map[iBind1];
		pVar->Map.erase(iBind1);
	}
	else if (bHas2)
	{
		pVar->Map[iBind1] = pVar->Map[iBind2];
		pVar->Map.erase(iBind2);
	}
}
#define Swap(t, i1, i2) if (IsType(t)) SwapMain<t>(pBase, i1, i2);

void CBinds::Move(int i1, int i2)
{
	auto& tBind1 = m_vBinds[i1];
	auto& tBind2 = m_vBinds[i2];
	auto tTemp = tBind1;
	tBind1 = tBind2;
	tBind2 = tTemp;

	std::vector<Bind_t*> vBinds1, vBinds2;
	for (auto& tBind : m_vBinds)
	{
		if (tBind.m_iParent == i1)
			vBinds1.push_back(&tBind);
		else if (tBind.m_iParent == i2)
			vBinds2.push_back(&tBind);
	}
	std::for_each(vBinds1.begin(), vBinds1.end(), [&](auto pBind) { pBind->m_iParent = i2; });
	std::for_each(vBinds2.begin(), vBinds2.end(), [&](auto pBind) { pBind->m_iParent = i1; });

	for (auto& pBase : G::Vars)
	{
		Swap(bool, i1, i2)
		else Swap(int, i1, i2)
		else Swap(float, i1, i2)
		else Swap(IntRange_t, i1, i2)
		else Swap(FloatRange_t, i1, i2)
		else Swap(std::string, i1, i2)
		else Swap(VA_LIST(std::vector<std::pair<std::string, Color_t>>), i1, i2)
		else Swap(Color_t, i1, i2)
		else Swap(Gradient_t, i1, i2)
		else Swap(Vec3, i1, i2)
		else Swap(DragBox_t, i1, i2)
		else Swap(WindowBox_t, i1, i2)
	}
}
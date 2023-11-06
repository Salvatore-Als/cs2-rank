#pragma once
#include <platform.h>
#include <../gameconfig.h>

class CGameEntitySystem;

class CGameResourceService
{
public:
	CGameEntitySystem *GetGameEntitySystem()
	{
		static int offset = g_CGameConfig->GetOffset("GameEntitySystem");
		return *reinterpret_cast<CGameEntitySystem **>((uintptr_t)(this) + offset);
	}
};
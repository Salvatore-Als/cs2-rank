#pragma once

#include "../../schema.h"
#include "mathlib/vector.h"
#include "baseentity.h"
#include "ehandle.h"

class CBaseEntity;

class CBaseEntity2 : public CBaseEntity
{
public:
  typedef CBaseEntity2 ThisClass;
  static constexpr const char *ThisClassName = "CBaseEntity";
  static constexpr bool IsStruct = false;

  SCHEMA_FIELD(CBitVec<64>, m_isSteadyState)
  SCHEMA_FIELD(float, m_lastNetworkChange)
  SCHEMA_FIELD_POINTER(void, m_NetworkTransmitComponent)
  SCHEMA_FIELD(int, m_iTeamNum)

  SCHEMA_FIELD(MoveType_t, m_MoveType)
  
  CHandle<CBaseEntity> GetHandle() { return m_pEntity->m_EHandle; }
  int entindex() { return m_pEntity->m_EHandle.GetEntryIndex(); }
  const char *GetClassname() { return m_pEntity->m_designerName.String(); }

  static CBaseEntity2 *EntityFromHandle(CHandle<CBaseEntity> handle)
  {
    if (!handle.IsValid())
    {
      return nullptr;
    }

    auto entity = handle.Get();

    if (entity && entity->m_pEntity->m_EHandle == handle)
    {
      return (CBaseEntity2 *)entity;
    }

    return nullptr;
  }
};
#pragma once

#include "../../schema.h"
#include "mathlib/vector.h"
#include "ehandle.h"
#include "cbaseentity.h"

class CTakeDamageInfo;

class CTakeDamageInfo2
{
public:
    typedef CTakeDamageInfo2 ThisClass;
    static constexpr const char *ThisClassName = "CTakeDamageInfo";
    static constexpr bool IsStruct = false;

    SCHEMA_FIELD(CHandle<CBaseEntity>, m_hInflictor)
    SCHEMA_FIELD(CHandle<CBaseEntity>, m_hAttacker)
    SCHEMA_FIELD(float, m_flDamage);
    SCHEMA_FIELD(int, m_bitsDamageType);

    CBaseEntity *GetAttacker() { return m_hAttacker.Get(); }
    CBaseEntity *GetInflictor() { return m_hInflictor.Get(); }
};
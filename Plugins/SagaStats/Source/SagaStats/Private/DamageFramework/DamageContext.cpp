// DamageContext.cpp — 统一存储：所有 Effect 以 UScriptStruct* 为 key
#include "DamageFramework/DamageContext.h"
#include "SagaStatsLog.h"

// ============================================================================
// C++ 内部 API
// ============================================================================

void UDamageContext::SetEffectByType(const FInstancedStruct& Value)
{
	if (Value.IsValid() && Value.GetScriptStruct())
	{
		DamageEffects.Add(const_cast<UScriptStruct*>(Value.GetScriptStruct()), Value);
	}
}

FInstancedStruct UDamageContext::GetEffectByType(UScriptStruct* EffectType) const
{
	if (EffectType)
	{
		if (const FInstancedStruct* Found = DamageEffects.Find(EffectType))
		{
			return *Found;
		}
	}
	return FInstancedStruct();
}

bool UDamageContext::HasEffectByType(UScriptStruct* EffectType) const
{
	return EffectType && DamageEffects.Contains(EffectType);
}

// ============================================================================
// 生命周期 / 调试
// ============================================================================

void UDamageContext::Reset()
{
	DamageEffects.Empty();
}

FString UDamageContext::DumpToString() const
{
	FString Result = TEXT("DamageContext Effects:\n");
	for (const auto& Pair : DamageEffects)
	{
		FString TypeName = Pair.Key ? Pair.Key->GetName() : TEXT("null");
		Result += FString::Printf(TEXT("  [%s]\n"), *TypeName);
	}
	return Result;
}

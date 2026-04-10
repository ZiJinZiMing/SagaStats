// DamageContext.h — UDamageContext: 单次伤害事件的共享上下文
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamageContext.generated.h"

/**
 * UDamageContext
 * 单次伤害事件的共享上下文。
 *
 * 统一存储：TMap<UScriptStruct*, FInstancedStruct>
 * EffectType 作为万能连接件：DamageRule:Effect 1:1 → 类型唯一确定生产者。
 * 攻击上下文和 DamageRule 产出共用同一存储——都是 typed struct。
 *
 * 蓝图 API（SetEffect/GetEffect CustomThunk）在 UDamageContextLibrary 中提供。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UDamageContext : public UObject
{
	GENERATED_BODY()

public:
	// =====================================================================
	// C++ 模板 API（编译时类型安全）
	// =====================================================================

	template<typename T>
	void SetEffect(const T& Value)
	{
		DamageEffects.Add(T::StaticStruct(), FInstancedStruct::Make<T>(Value));
	}

	template<typename T>
	const T* GetEffect() const
	{
		if (const FInstancedStruct* Found = DamageEffects.Find(T::StaticStruct()))
		{
			return Found->GetPtr<T>();
		}
		return nullptr;
	}

	template<typename T>
	bool HasEffect() const
	{
		return DamageEffects.Contains(T::StaticStruct());
	}

	// =====================================================================
	// C++ 内部 API（运行时 UScriptStruct*，供框架使用）
	// =====================================================================

	void SetEffectByType(const FInstancedStruct& Value);
	FInstancedStruct GetEffectByType(UScriptStruct* EffectType) const;
	bool HasEffectByType(UScriptStruct* EffectType) const;

	const TMap<UScriptStruct*, FInstancedStruct>& GetAllDamageEffects() const { return DamageEffects; }

	// =====================================================================
	// 生命周期 / 调试
	// =====================================================================

	UFUNCTION(BlueprintCallable, Category = "DamageContext")
	void Reset();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	FString DumpToString() const;

private:
	/** UScriptStruct* 来自编译时 USTRUCT，生命周期永久，不需要 GC 追踪 */
	TMap<UScriptStruct*, FInstancedStruct> DamageEffects;
};

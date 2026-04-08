// DamageContext.h — UDamageContext: 单次伤害事件的共享上下文 (Context)
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "GameplayTagContainer.h"
#include "DamageContext.generated.h"

// ============================================================================
// 标量兼容包装器 —— SetBool/GetFloat 等通过这些 USTRUCT 存入 FInstancedStruct
// ============================================================================

USTRUCT() struct FDCFact_Bool  { GENERATED_BODY() UPROPERTY() bool Value = false; };
USTRUCT() struct FDCFact_Int   { GENERATED_BODY() UPROPERTY() int32 Value = 0; };
USTRUCT() struct FDCFact_Float { GENERATED_BODY() UPROPERTY() float Value = 0.f; };
USTRUCT() struct FDCFact_Name  { GENERATED_BODY() UPROPERTY() FName Value = NAME_None; };
USTRUCT() struct FDCFact_Tag   { GENERATED_BODY() UPROPERTY() FGameplayTag Value; };
USTRUCT() struct FDCFact_Vector{ GENERATED_BODY() UPROPERTY() FVector Value = FVector::ZeroVector; };

/**
 * UDamageContext (Context)
 * 单次伤害事件的共享上下文。
 *
 * v4.8: 双存储架构
 * - ContextFacts (FName key)：事件上下文，标量兼容接口
 * - DamageEffects (UScriptStruct* key)：DamageRule 产出的 Effect，类型即 key
 *
 * EffectType 作为万能连接件：DamageRule:Effect 1:1 → 类型唯一确定生产者。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UDamageContext : public UObject
{
	GENERATED_BODY()

public:
	// =====================================================================
	// 事件上下文（FInstancedStruct —— 管线启动前一次性填入）
	// =====================================================================

	UFUNCTION(BlueprintCallable, Category = "DamageContext|EventContext")
	void SetEventContext(const FInstancedStruct& InContext);

	UFUNCTION(BlueprintPure, Category = "DamageContext|EventContext")
	const FInstancedStruct& GetEventContext() const { return EventContext; }

	template<typename T>
	const T* GetContext() const { return EventContext.GetPtr<T>(); }

	// =====================================================================
	// DamageRule Effect 存储（v4.8: UScriptStruct* key —— 类型即 key）
	// =====================================================================

	/** C++ 模板：类型安全地写入 DamageEffect（类型即 key） */
	template<typename T>
	void SetEffect(const T& Value)
	{
		DamageEffects.Add(T::StaticStruct(), FInstancedStruct::Make<T>(Value));
	}

	/** C++ 模板：类型安全地读取 DamageEffect（类型即 key） */
	template<typename T>
	const T* GetEffect() const
	{
		if (const FInstancedStruct* Found = DamageEffects.Find(T::StaticStruct()))
		{
			return Found->GetPtr<T>();
		}
		return nullptr;
	}

	/** C++ 模板：检查 DamageEffect 是否存在 */
	template<typename T>
	bool HasEffect() const
	{
		return DamageEffects.Contains(T::StaticStruct());
	}

	/** 通用 API（蓝图可调用）—— 按 EffectType 写入 */
	UFUNCTION(BlueprintCallable, Category = "DamageContext|Facts")
	void SetEffectByType(UScriptStruct* EffectType, const FInstancedStruct& Value);

	/** 通用 API（蓝图可调用）—— 按 EffectType 读取 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext|Facts")
	FInstancedStruct GetEffectByType(UScriptStruct* EffectType) const;

	/** 通用 API（蓝图可调用）—— 按 EffectType 检查存在 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext|Facts")
	bool HasEffectByType(UScriptStruct* EffectType) const;

	/** 获取所有 DamageEffect */
	const TMap<UScriptStruct*, FInstancedStruct>& GetAllDamageEffects() const { return DamageEffects; }

	// =====================================================================
	// 事件上下文标量接口（FName key，保留兼容）
	// =====================================================================

	UFUNCTION(BlueprintCallable, Category = "DamageContext")
	void SetBool(FName Key, bool Value);

	UFUNCTION(BlueprintCallable, Category = "DamageContext")
	void SetFloat(FName Key, float Value);

	UFUNCTION(BlueprintCallable, Category = "DamageContext")
	void SetInt(FName Key, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "DamageContext")
	void SetName(FName Key, FName Value);

	UFUNCTION(BlueprintCallable, Category = "DamageContext")
	void SetVector(FName Key, const FVector& Value);

	UFUNCTION(BlueprintCallable, Category = "DamageContext")
	void SetTag(FName Key, FGameplayTag Value);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	bool GetBool(FName Key) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	float GetFloat(FName Key) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	int32 GetInt(FName Key) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	FGameplayTag GetTag(FName Key) const;

	// =====================================================================
	// 查询 / 生命周期 / 调试
	// =====================================================================

	/** 事件上下文中是否存在指定 key */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	bool Contains(FName Key) const;

	const TSet<FName>& GetContextFieldNames() const { return ContextFieldNames; }

	/** 获取所有事件上下文 Effect（FName key） */
	const TMap<FName, FInstancedStruct>& GetAllContextFacts() const { return ContextFacts; }

	UFUNCTION(BlueprintCallable, Category = "DamageContext")
	void Reset();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	FString DumpToString() const;

private:
	/** 事件上下文存储（FName key） */
	UPROPERTY()
	TMap<FName, FInstancedStruct> ContextFacts;

	/** DamageRule 产出的 Effect 存储（UScriptStruct* key —— 类型即 key） */
	TMap<UScriptStruct*, FInstancedStruct> DamageEffects;

	/** 原始事件上下文结构体 */
	UPROPERTY()
	FInstancedStruct EventContext;

	/** 由 SetEventContext 自动展开的字段名集合 */
	TSet<FName> ContextFieldNames;

	/** 内部：将 FInstancedStruct 的 UPROPERTY 字段展开到 ContextFacts */
	void ExpandStructToFields(const FInstancedStruct& Struct);

	/** 内部标量存取辅助 */
	template<typename T>
	void SetContextValue(FName Key, const T& Value)
	{
		ContextFacts.Add(Key, FInstancedStruct::Make<T>(Value));
	}

	static float ExtractFloatFromStruct(const FInstancedStruct& Struct, FName FieldName);
};

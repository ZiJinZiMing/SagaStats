// DamageContext.h — UDamageContext: 单次伤害事件的共享上下文 (DC)
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
 * UDamageContext (DC)
 * 单次伤害事件的共享上下文。存储事件上下文信息和 DPU 产出的 Fact。
 * 生命周期：事件开始时创建，事件结束时销毁。
 * R1: DC 是 DPU 之间唯一的通信媒介。
 * R3: Fact 缺失时视为 false。
 *
 * v4.4: 内部存储升级为 TMap<FName, FInstancedStruct>（Fact 模型）。
 * SetBool/GetFloat 等保留为兼容接口，内部通过包装器 USTRUCT 存取。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UDamageContext : public UObject
{
	GENERATED_BODY()

public:
	// =====================================================================
	// 事件上下文（FInstancedStruct —— 管线启动前一次性填入）
	// =====================================================================

	/** 设置攻击事件上下文。自动将结构体字段展开到 Facts TMap 中。 */
	UFUNCTION(BlueprintCallable, Category = "DamageContext|EventContext")
	void SetEventContext(const FInstancedStruct& InContext);

	const FInstancedStruct& GetEventContext() const { return EventContext; }

	template<typename T>
	const T* GetContext() const { return EventContext.GetPtr<T>(); }

	// =====================================================================
	// Fact 存储（v4.4 核心 API）
	// =====================================================================

	/** C++ 模板路径：类型安全地写入 Fact */
	template<typename T>
	void SetFact(FName Key, const T& Value)
	{
		Facts.Add(Key, FInstancedStruct::Make<T>(Value));
	}

	/** C++ 模板路径：类型安全地读取 Fact。类型不匹配或不存在返回 nullptr。 */
	template<typename T>
	const T* GetFact(FName Key) const
	{
		if (const FInstancedStruct* Found = Facts.Find(Key))
		{
			return Found->GetPtr<T>();
		}
		return nullptr;
	}

	/** 通用 FInstancedStruct 路径（蓝图可调用） */
	UFUNCTION(BlueprintCallable, Category = "DamageContext|Facts")
	void SetFactGeneric(FName Key, const FInstancedStruct& Value);

	/** 通用读取。不存在返回空的 FInstancedStruct。 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext|Facts")
	FInstancedStruct GetFactGeneric(FName Key) const;

	/** Fact 是否存在 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext|Facts")
	bool HasFact(FName Key) const;

	/**
	 * 调用 FactMethodRegistry 评估 Fact 领域方法。
	 * Fact 不存在 → 返回 false（R3）。
	 * MethodName 为 None → 信号 Fact 检查（存在即 true）。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext|Facts")
	bool EvaluateFactMethod(FName FactKey, FName MethodName) const;

	/**
	 * 通过属性反射读取 Fact 内部字段的 float 值。
	 * 用于 ConditionNode_Compare 节点。
	 * Fact 不存在或字段不存在 → 返回 0.f。
	 * FieldName 为空 → 尝试对包装器 Fact 取标量值。
	 */
	float GetFactFieldAsFloat(FName FactKey, FName FieldName) const;

	/** 获取所有 Fact（包含上下文展开的和 DPU 产出的） */
	const TMap<FName, FInstancedStruct>& GetAllFacts() const { return Facts; }

	// =====================================================================
	// 标量兼容接口（保留，内部通过包装器 USTRUCT 存取）
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

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	bool Contains(FName Key) const;

	/** 获取由 SetEventContext 自动展开的上下文字段名列表 */
	const TSet<FName>& GetContextFieldNames() const { return ContextFieldNames; }

	UFUNCTION(BlueprintCallable, Category = "DamageContext")
	void Reset();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	FString DumpToString() const;

private:
	/** Fact 存储：Fact Key → FInstancedStruct（包含标量包装器和复杂 Fact） */
	UPROPERTY()
	TMap<FName, FInstancedStruct> Facts;

	/** 原始事件上下文结构体 */
	UPROPERTY()
	FInstancedStruct EventContext;

	/** 由 SetEventContext 自动展开的字段名集合（区分"上下文"和"DPU产出"） */
	TSet<FName> ContextFieldNames;

	/** 内部：将 FInstancedStruct 的 UPROPERTY 字段展开到 Facts TMap */
	void ExpandStructToFields(const FInstancedStruct& Struct);

	/** 内部：从 FInstancedStruct 提取标量 float 值 */
	static float ExtractFloatFromStruct(const FInstancedStruct& Struct, FName FieldName);
};

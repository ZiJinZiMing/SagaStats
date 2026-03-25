// DamageContext.h — UDamageContext: 单次伤害事件的共享上下文 (DC)
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "GameplayTagContainer.h"
#include "DPUFramework/DCValue.h"
#include "DamageContext.generated.h"

/**
 * UDamageContext (DC)
 * 单次伤害事件的共享上下文。保存上下文信息和 DPU 输出结果。
 * 生命周期：事件开始时创建，事件结束时销毁。
 * R1: DC 是 DPU 之间唯一的通信媒介。
 * R3: 缺失字段返回 None (AsBool() == false)。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UDamageContext : public UObject
{
	GENERATED_BODY()

public:
	// =====================================================================
	// 事件上下文（FInstancedStruct —— 管线启动前一次性填入）
	// =====================================================================

	/**
	 * 设置攻击事件上下文（FInstancedStruct）。
	 * 自动将结构体的所有 UPROPERTY 字段展开到 DC 的 Fields TMap 中，
	 * 使得 Condition 表达式和 DPU 可以通过 GetBool/GetFloat 等直接读取。
	 *
	 * C++ DPU 也可通过 GetContext<T>() 获取类型安全的结构体指针。
	 */
	UFUNCTION(BlueprintCallable, Category = "DamageContext|EventContext")
	void SetEventContext(const FInstancedStruct& InContext);

	/** 获取原始 FInstancedStruct（用于 C++ 模板化访问） */
	const FInstancedStruct& GetEventContext() const { return EventContext; }

	/** C++ 便捷方法：类型安全地获取事件上下文指针。类型不匹配返回 nullptr。 */
	template<typename T>
	const T* GetContext() const { return EventContext.GetPtr<T>(); }

	// =====================================================================
	// DPU 产出字段（TMap —— 管线执行中动态写入）
	// =====================================================================

	// --- 写入 (DPU 调用) ---
	UFUNCTION(BlueprintCallable, Category = "DamageContext")
	void Set(FName Key, const FDCValue& Value);

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

	// --- 读取 (DPU 和 Condition 调用) ---
	// R3: 缺失字段返回 None 类型的 FDCValue
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	FDCValue Get(FName Key) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	bool GetBool(FName Key) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	float GetFloat(FName Key) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	int32 GetInt(FName Key) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	FGameplayTag GetTag(FName Key) const;

	// --- 查询 ---
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	bool Contains(FName Key) const;

	const TMap<FName, FDCValue>& GetAllFields() const { return Fields; }

	/** 获取由 SetEventContext 自动展开的上下文字段名列表（用于 Mermaid 导出区分颜色） */
	const TSet<FName>& GetContextFieldNames() const { return ContextFieldNames; }

	// --- 生命周期 ---
	UFUNCTION(BlueprintCallable, Category = "DamageContext")
	void Reset();

	// --- 调试 ---
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	FString DumpToString() const;

private:
	/** DPU 产出 + 自动展开的上下文字段（统一存储，Condition 透明读取） */
	UPROPERTY()
	TMap<FName, FDCValue> Fields;

	/** 原始事件上下文结构体（C++ GetContext<T>() 访问） */
	UPROPERTY()
	FInstancedStruct EventContext;

	/** 由 SetEventContext 自动展开的字段名集合（区分"上下文"和"DPU产出"） */
	TSet<FName> ContextFieldNames;

	/** 内部：将 FInstancedStruct 的 UPROPERTY 字段展开到 Fields TMap */
	void ExpandStructToFields(const FInstancedStruct& Struct);
};

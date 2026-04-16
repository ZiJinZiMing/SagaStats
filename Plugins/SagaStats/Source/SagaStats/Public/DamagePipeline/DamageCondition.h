/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageCondition.h — 条件原子（子类即域方法，无 MethodName）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamageCondition.generated.h"

class UDamageContext;

// ============================================================================
// UDamageCondition — 条件原子基类
// ============================================================================

/**
 * 选子类 = 选域方法。每个子类 IS 一个域检查。
 * 框架自动从 DC 按 EffectType 取出 Effect 传给 Evaluate。
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable, CollapseCategories)
class SAGASTATS_API UDamageCondition : public UObject
{
	GENERATED_BODY()

public:
	/** 公共入口——自动从 DC 按 EffectType 取 Effect 传给 Evaluate */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageCondition")
	bool EvaluateCondition(const UDamageContext* Context) const;

	/**
	 * 子类重写——BlueprintNativeEvent。
	 * @param Context         共享上下文
	 * @param InEffect  框架自动取出的 Effect（按 EffectType 从 DC 获取）
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "DamageCondition")
	bool Evaluate(const UDamageContext* Context, const FInstancedStruct& InEffect) const;
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& InEffect) const { return false; }

	// ---- EffectType 连接件 ----
	virtual UScriptStruct* GetEffectType() const { return EffectType; }

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "DamageCondition")
	FString GetDisplayString() const;

	virtual FString GetDisplayString_Implementation() const
	{
		FString Name = GetClass()->GetName();
		Name.RemoveFromEnd(TEXT("_C"));
		return Name;
	}

protected:
	/**
	 * 蓝图子类在 Blueprint Class Defaults 中设置；C++ 子类 override GetEffectType()。
	 * 非 CDO 实例上通过 IsClassDefaultContext() + EditConditionHides 完全隐藏，防止在 DamageRule 内误改。
	 */
	UPROPERTY(EditAnywhere, Category = "DamageCondition", meta = (EditCondition = "IsClassDefaultContext", EditConditionHides))
	UScriptStruct* EffectType = nullptr;

private:
	/** EditCondition 驱动函数：CDO/Archetype 返回 true → EffectType 可见可编辑；普通实例返回 false → 隐藏 */
	UFUNCTION()
	bool IsClassDefaultContext() const { return HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject); }
};


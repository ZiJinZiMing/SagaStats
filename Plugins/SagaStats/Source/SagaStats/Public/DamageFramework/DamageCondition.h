// DamageCondition.h — 条件原子（v4.8: 子类即域方法，无 MethodName）
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
 * 框架自动从 DC 取出 ConsumedEffect 传给 Evaluate。
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable, CollapseCategories)
class SAGASTATS_API UDamageCondition : public UObject
{
	GENERATED_BODY()

public:
	/** 公共入口——自动从 DC 取 ConsumedEffect 传给 Evaluate */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageCondition")
	bool EvaluateCondition(const UDamageContext* Context) const;

	/**
	 * 子类重写——BlueprintNativeEvent。
	 * @param Context         共享上下文
	 * @param OutEffect  框架自动取出的 Effect（按 ConsumedEffectType 从 DC 获取）
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "DamageCondition")
	bool Evaluate(const UDamageContext* Context, const FInstancedStruct& OutEffect) const;
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& OutEffect) const { return false; }

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
	/** 蓝图子类在类默认值中设置；C++ 子类 override GetConsumedEffectType()。实例上不可见。 */
	UPROPERTY(EditDefaultsOnly, Category = "DamageCondition", meta = (HideInDetailPanel))
	UScriptStruct* EffectType = nullptr;
};


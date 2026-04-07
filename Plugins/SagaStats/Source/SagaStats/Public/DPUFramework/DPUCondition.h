// DPUCondition.h — 条件原子（v4.8: 子类即域方法，无 MethodName）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPUCondition.generated.h"

class UDamageContext;

// ============================================================================
// UDPUCondition — 条件原子基类
// ============================================================================

/**
 * 选子类 = 选域方法。每个子类 IS 一个域检查。
 * 框架自动从 DC 取出 ConsumedFact 传给 Evaluate。
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable, CollapseCategories)
class SAGASTATS_API UDPUCondition : public UObject
{
	GENERATED_BODY()

public:
	/** 公共入口——自动从 DC 取 ConsumedFact 传给 Evaluate */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Condition")
	bool EvaluateCondition(const UDamageContext* DC) const;

	/**
	 * 子类重写——BlueprintNativeEvent。
	 * @param DC             共享上下文
	 * @param ConsumedFact   框架自动取出的 Fact（按 ConsumedFactType 从 DC 获取）
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Condition")
	bool Evaluate(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const;
	virtual bool Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const { return false; }

	// ---- FactType 连接件 ----

	/** 蓝图子类在类默认值中设置；C++ 子类 override GetConsumedFactType()。实例上不可见。 */
	UPROPERTY(EditDefaultsOnly, Category = "Condition", meta = (DisplayName = "Consumed Fact Type", HideInDetailPanel))
	UScriptStruct* ConsumedFactType = nullptr;

	virtual UScriptStruct* GetConsumedFactType() const { return ConsumedFactType; }

	TArray<UScriptStruct*> GetConsumedFactTypes() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Condition")
	virtual FString GetDisplayString() const;
};

// ============================================================================
// UDPUCondition_ContextCheck — 事件上下文字段检查（不消费 DPU Fact）
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "Context Check"))
class SAGASTATS_API UDPUCondition_ContextCheck : public UDPUCondition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Condition")
	FName ContextKey;

	virtual bool Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const override;
	virtual FString GetDisplayString() const override;
};

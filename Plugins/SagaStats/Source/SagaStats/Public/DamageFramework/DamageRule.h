// DamageRuleDefinition.h — UDamageRule: DamageRule 定义（DataAsset 形式）
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DamageFramework/DamageOperationBase.h"
#include "DamageFramework/DamagePredicate.h"
#include "DamageRule.generated.h"

/**
 * UDamageRule — DamageRule 的静态定义。
 *
 * v4.7: ProducesEffectType 从 OperationClass CDO 获取，不再在 Definition 上声明。
 * Condition 类型改为 UDamagePredicate（谓词容器）。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UDamageRule : public UDataAsset
{
	GENERATED_BODY()

public:
	/** DamageRule 身份标识——同时用于 DC 中 Effect 索引和依赖引用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RuleName;

	/** 人类可读的描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	/** Condition 谓词容器（v4.7: Predicate/Condition 双层，可为空 = 始终执行） */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite)
	TObjectPtr<UDamagePredicate> Condition;

	/** 实现机制的逻辑类（UDamageOperationBase 的子类） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDamageOperationBase> OperationClass;

	/** 表现层选择的基础优先级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasePriority = 0.f;

	/** 从 OperationClass CDO 获取此 DamageRule 产出的 Effect 类型 */
	UScriptStruct* GetProducesEffectType() const;

	/** 从 Condition 谓词树中提取依赖的 EffectType 列表（用于拓扑排序） */
	TArray<UScriptStruct*> GetConsumedEffectTypes() const;
};

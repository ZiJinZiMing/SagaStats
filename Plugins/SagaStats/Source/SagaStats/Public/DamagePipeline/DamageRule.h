/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DamagePipeline/DamageOperationBase.h"
#include "DamagePipeline/DamagePredicate.h"
#include "DamageRule.generated.h"

/**
 * 伤害管线中的一个处理规则，包含生效条件和操作逻辑。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UDamageRule : public UDataAsset
{
	GENERATED_BODY()

public:

	/** 从 OperationClass CDO 获取此 DamageRule 产出的 Effect 类型 */
	UScriptStruct* GetProducesEffectType() const;

	/** 从 Condition 谓词树中提取依赖的 EffectType 列表（用于拓扑排序） */
	TArray<UScriptStruct*> GetConsumedEffectTypes() const;
	
	/** Condition 谓词容器（Predicate/Condition 双层，可为空 = 始终执行） */
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly)
	TObjectPtr<UDamagePredicate> Condition;
	
	/** 实现机制的逻辑类（UDamageOperationBase 的子类） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UDamageOperationBase> OperationClass;
	
	/** 可读的机制描述（多行 FText，用于在 Graph 节点中显示 Rule 的行为意图） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DamageRule", meta = (MultiLine = true))
	FText Description;
};

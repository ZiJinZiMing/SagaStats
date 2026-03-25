// ConditionExpression.h — UConditionExpression DPU 门控条件基类
#pragma once

#include "CoreMinimal.h"
#include "ConditionExpression.generated.h"

class UDamageContext;

/**
 * UConditionExpression — Condition（DPU 门控）基类。
 * R3: 读取 DC，输出布尔值。缺失字段 = false。
 * R4: Condition 装配到 DPU 上，而非 DPU 的一部分。
 * 继承 UStringConditionExpression（表达式字符串）或在 C++ 中重写。
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class SAGASTATS_API UConditionExpression : public UObject
{
	GENERATED_BODY()

public:
	/** 根据给定 DC 评估条件。返回 true 表示 DPU 应当执行。 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DPU Condition")
	virtual bool Evaluate(const UDamageContext* DC) const PURE_VIRTUAL(UConditionExpression::Evaluate, return true;);

	/** 提取消费的 DC 字段名（用于拓扑排序依赖边）。 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DPU Condition")
	virtual TArray<FName> GetConsumedFields() const PURE_VIRTUAL(UConditionExpression::GetConsumedFields, return {};);
};

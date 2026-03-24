// StringConditionExpression.h — 基于字符串的条件表达式，使用递归下降解析器
#pragma once

#include "CoreMinimal.h"
#include "DPUFramework/ConditionExpression.h"
#include "StringConditionExpression.generated.h"

/**
 * UStringConditionExpression
 * 解析并求值布尔表达式，如 "Guard && !IsLightningInAir"。
 *
 * 文法:
 *   Expr      -> OrExpr
 *   OrExpr    -> AndExpr ( "||" AndExpr )*
 *   AndExpr   -> UnaryExpr ( "&&" UnaryExpr )*
 *   UnaryExpr -> "!" UnaryExpr | Primary
 *   Primary   -> "(" Expr ")" | Identifier ( ("==" | "!=") Value )? | "true" | "false"
 *   Value     -> Identifier | "true" | "false" | Number
 *
 * 标识符通过 DC->Get(FName).AsBool() 解析为 DC 字段名。
 * 对于 == / != 比较，两侧均被解析后进行比较。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UStringConditionExpression : public UConditionExpression
{
	GENERATED_BODY()

public:
	/** 条件表达式字符串，例如 "Guard && !IsLightningInAir" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Expression;

	virtual bool Evaluate(const UDamageContext* DC) const override;
	virtual TArray<FName> GetConsumedFields() const override;

	/** 辅助方法：一次调用完成创建和初始化（用于测试） */
	static UStringConditionExpression* Create(UObject* Outer, const FString& InExpression);
};

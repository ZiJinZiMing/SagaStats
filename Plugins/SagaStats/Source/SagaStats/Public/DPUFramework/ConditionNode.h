// ConditionNode.h — 条件树节点层级（复用 AIToken Predicate 模式）
#pragma once

#include "CoreMinimal.h"
#include "ConditionNode.generated.h"

class UDamageContext;

// ============================================================================
// 比较运算符枚举
// ============================================================================

UENUM(BlueprintType)
enum class ECompareOp : uint8
{
	Equal,
	NotEqual,
	Less,
	LessEqual,
	Greater,
	GreaterEqual
};

// ============================================================================
// UConditionNode 基类
// ============================================================================

/**
 * UConditionNode — 条件树节点基类。
 *
 * 模式来源：AIToken 插件的 Predicate 系统。
 * DefaultToInstanced + EditInlineNew 实现属性面板内联嵌套编辑。
 * bReverse 取反开关在基类统一处理，子类只需实现 Evaluate()。
 *
 * 子类：And / Or / FactQuery / Compare
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable, CollapseCategories)
class SAGASTATS_API UConditionNode : public UObject
{
	GENERATED_BODY()

public:
	/** 取反开关：true 时 EvaluateCondition 返回 !Evaluate() 的结果 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	bool bReverse = false;

	/**
	 * 公开入口——调用 Evaluate() 并应用 bReverse。
	 * PipelineAsset 和 DPUDefinition 应调用此方法，而非直接调用 Evaluate()。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Condition")
	bool EvaluateCondition(const UDamageContext* DC) const;

	/** 子类实现：评估条件，返回原始结果（不含 bReverse） */
	virtual bool Evaluate(const UDamageContext* DC) const
		PURE_VIRTUAL(UConditionNode::Evaluate, return true;);

	/** 递归收集本节点引用的所有 Fact Key（用于拓扑排序的产销匹配） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Condition")
	virtual TArray<FName> GetConsumedFacts() const { return {}; }

	/** 人类可读的条件描述（Mermaid 导出用） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Condition")
	virtual FString GetDisplayString() const { return TEXT("(base)"); }
};

// ============================================================================
// UConditionNode_And — 所有子节点为 true 才 true
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "AND"))
class SAGASTATS_API UConditionNode_And : public UConditionNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "Condition")
	TArray<TObjectPtr<UConditionNode>> Children;

	virtual bool Evaluate(const UDamageContext* DC) const override;
	virtual TArray<FName> GetConsumedFacts() const override;
	virtual FString GetDisplayString() const override;
};

// ============================================================================
// UConditionNode_Or — 任一子节点为 true 即 true
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "OR"))
class SAGASTATS_API UConditionNode_Or : public UConditionNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "Condition")
	TArray<TObjectPtr<UConditionNode>> Children;

	virtual bool Evaluate(const UDamageContext* DC) const override;
	virtual TArray<FName> GetConsumedFacts() const override;
	virtual FString GetDisplayString() const override;
};

// ============================================================================
// UConditionNode_FactQuery — 通过 FactMethodRegistry 调用领域方法
// ============================================================================

/**
 * 引用 Fact 的领域方法。
 * FactKey 对应 DC 中的 Fact Key。
 * MethodName 对应 FactMethodRegistry 中注册的方法名。
 * MethodName 为 None 时视为信号 Fact 检查（存在即 true）。
 */
UCLASS(BlueprintType, meta = (DisplayName = "Fact Query"))
class SAGASTATS_API UConditionNode_FactQuery : public UConditionNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (GetOptions = "GetFactKeyOptions"))
	FName FactKey;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (GetOptions = "GetMethodOptions"))
	FName MethodName;

	virtual bool Evaluate(const UDamageContext* DC) const override;
	virtual TArray<FName> GetConsumedFacts() const override;
	virtual FString GetDisplayString() const override;

	/** 编辑器下拉：返回所有已注册的 Fact Key */
	UFUNCTION()
	TArray<FString> GetFactKeyOptions() const;

	/** 编辑器联动下拉：返回当前 FactKey 对应类型的已注册领域方法 */
	UFUNCTION()
	TArray<FString> GetMethodOptions() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

// ============================================================================
// UConditionNode_Compare — Fact 数据字段数值比较
// ============================================================================

/**
 * 对 Fact 内部字段做数值比较。
 * 通过 UE5 属性反射访问 FieldName 对应的字段值。
 * FieldName 为空时对整个 Fact 做布尔求值（HasFact）。
 */
UCLASS(BlueprintType, meta = (DisplayName = "Compare"))
class SAGASTATS_API UConditionNode_Compare : public UConditionNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (GetOptions = "GetFactKeyOptions"))
	FName FactKey;

	/** Fact 内部的字段名。空 = 对 Fact 整体布尔求值。 */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (GetOptions = "GetFieldOptions"))
	FName FieldName;

	UPROPERTY(EditAnywhere, Category = "Condition")
	ECompareOp Operator = ECompareOp::Equal;

	UPROPERTY(EditAnywhere, Category = "Condition")
	float Value = 0.f;

	virtual bool Evaluate(const UDamageContext* DC) const override;
	virtual TArray<FName> GetConsumedFacts() const override;
	virtual FString GetDisplayString() const override;

	/** 编辑器下拉：返回所有已注册的 Fact Key */
	UFUNCTION()
	TArray<FString> GetFactKeyOptions() const;

	/** 编辑器联动下拉：返回当前 FactKey 对应 Fact 类型的数值字段 */
	UFUNCTION()
	TArray<FString> GetFieldOptions() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

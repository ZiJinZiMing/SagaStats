// ConditionNode.h — 条件树框架核心（v4.6: 基类 + And/Or/DPUBase/ContextCheck）
// Per-DPU 子类定义在 Sekiro/ 子目录的各 DPU 文件中。
#pragma once

#include "CoreMinimal.h"
#include "ConditionNode.generated.h"

class UDamageContext;
class UDPUDefinition;

// ============================================================================
// UConditionNode 基类
// ============================================================================

UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable, CollapseCategories)
class SAGASTATS_API UConditionNode : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	bool bReverse = false;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Condition")
	bool EvaluateCondition(const UDamageContext* DC) const;

	virtual bool Evaluate(const UDamageContext* DC) const
		PURE_VIRTUAL(UConditionNode::Evaluate, return true;);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Condition")
	virtual TArray<FName> GetConsumedDPUs() const { return {}; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Condition")
	virtual FString GetDisplayString() const { return TEXT("(base)"); }

	/** 子类绑定的 Fact 类型（DPU 和 Fact 一一对应，FactType 唯一确定生产者 DPU） */
	virtual UScriptStruct* GetConsumedFactType() const { return nullptr; }

	/** Build 时由 PipelineAsset 解析——通过 FactType→DPUName 映射自动填充 */
	FName ResolvedProducerDPU;
	virtual void ResolveProducer(const TMap<UScriptStruct*, FName>& FactTypeToProducerMap);
};

// ============================================================================
// UConditionNode_And
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "AND"))
class SAGASTATS_API UConditionNode_And : public UConditionNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "Condition")
	TArray<TObjectPtr<UConditionNode>> Children;

	virtual bool Evaluate(const UDamageContext* DC) const override;
	virtual TArray<FName> GetConsumedDPUs() const override;
	virtual FString GetDisplayString() const override;
	virtual void ResolveProducer(const TMap<UScriptStruct*, FName>& FactTypeToProducerMap) override;
};

// ============================================================================
// UConditionNode_Or
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "OR"))
class SAGASTATS_API UConditionNode_Or : public UConditionNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "Condition")
	TArray<TObjectPtr<UConditionNode>> Children;

	virtual bool Evaluate(const UDamageContext* DC) const override;
	virtual TArray<FName> GetConsumedDPUs() const override;
	virtual FString GetDisplayString() const override;
	virtual void ResolveProducer(const TMap<UScriptStruct*, FName>& FactTypeToProducerMap) override;
};

// ============================================================================
// UConditionNode_ContextCheck — 事件上下文字段检查（非 DPU 依赖）
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "Context Check"))
class SAGASTATS_API UConditionNode_ContextCheck : public UConditionNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Condition")
	FName ContextKey;

	virtual bool Evaluate(const UDamageContext* DC) const override;
	virtual TArray<FName> GetConsumedDPUs() const override { return {}; }
	virtual FString GetDisplayString() const override;
};

// ============================================================================
// UConditionNode_DPUBase — 按 DPU 分的条件节点中间基类
// ============================================================================

/**
 * v4.6: 每个 DPU 对应一个子类，领域方法为 UFUNCTION。
 * 领域方法签名：bool MethodName(const UDamageContext* DC) const
 * MethodName 为 None 时退化为信号检查（Fact 存在即 true）。
 */
UCLASS(Abstract, BlueprintType)
class SAGASTATS_API UConditionNode_DPUBase : public UConditionNode
{
	GENERATED_BODY()

public:
	/** 消费的 Fact 类型（蓝图子类在类默认值中设置；C++ 子类可直接 override GetConsumedFactType） */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (DisplayName = "Consumed Fact Type"))
	UScriptStruct* ConsumedFactType = nullptr;

	virtual UScriptStruct* GetConsumedFactType() const override { return ConsumedFactType; }

	/** 领域方法名（None = 信号检查，存在即 true） */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (GetOptions = "GetMethodOptions"))
	FName MethodName;

	virtual bool Evaluate(const UDamageContext* DC) const override;
	virtual TArray<FName> GetConsumedDPUs() const override;
	virtual FString GetDisplayString() const override;

	/** UE 反射自动发现子类上的领域方法，填充下拉 */
	UFUNCTION()
	TArray<FString> GetMethodOptions() const;

protected:
	/** 通过 ProcessEvent 调用命名的领域方法（bool 返回类型） */
	bool CallDomainMethod(const UDamageContext* DC) const;

	/** 判断 UFunction 是否是合法的领域方法候选 */
	static bool IsValidDomainMethod(const UFunction* Func);
};

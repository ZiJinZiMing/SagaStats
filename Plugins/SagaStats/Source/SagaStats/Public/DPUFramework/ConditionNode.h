// ConditionNode.h — 条件树节点层级（v4.6: 按 DPU 分子类，领域方法为 UFUNCTION）
#pragma once

#include "CoreMinimal.h"
#include "DPUFramework/SekiroFacts.h"
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

	/** v4.6: 递归收集依赖的 DPU 名列表 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Condition")
	virtual TArray<FName> GetConsumedDPUs() const { return {}; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Condition")
	virtual FString GetDisplayString() const { return TEXT("(base)"); }

	/** v4.6: 子类绑定的 Fact 类型（叶子节点返回具体类型，复合节点返回 nullptr） */
	virtual UScriptStruct* GetConsumedFactType() const { return nullptr; }

	/** v4.6: 子类硬编码的生产者 DPU 名（叶子节点返回具体名，复合节点返回 None） */
	virtual FName GetExpectedProducerDPUName() const { return NAME_None; }

	/** Build 时由 PipelineAsset 调用，解析并缓存生产者 DPU 名 */
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
 * v4.6: 每个 DPU 对应一个 ConditionNode 子类。
 * 领域方法直接定义为 UFUNCTION，通过 UE 反射自动发现。
 * MethodName 属性选择调用哪个方法，None = 信号 Fact 检查（存在即 true）。
 */
UCLASS(Abstract, BlueprintType)
class SAGASTATS_API UConditionNode_DPUBase : public UConditionNode
{
	GENERATED_BODY()

public:
	/** 领域方法名（None = 信号检查，存在即 true） */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (GetOptions = "GetMethodOptions"))
	FName MethodName;

	virtual bool Evaluate(const UDamageContext* DC) const override;
	virtual TArray<FName> GetConsumedDPUs() const override;
	virtual FString GetDisplayString() const override;

	/** UE 反射自动发现子类上的领域方法（UFUNCTION），填充下拉 */
	UFUNCTION()
	TArray<FString> GetMethodOptions() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	/** 通过 ProcessEvent 调用命名的领域方法 */
	bool CallDomainMethod(const UDamageContext* DC) const;

	/** 判断一个 UFunction 是否是合法的领域方法候选 */
	static bool IsValidDomainMethod(const UFunction* Func, const UClass* LeafClass);
};

// ============================================================================
// Per-DPU ConditionNode 子类 — 复杂 Fact（带领域方法）
// ============================================================================

// --- Mixup ---
UCLASS(BlueprintType, meta = (DisplayName = "Mixup"))
class SAGASTATS_API UConditionNode_Mixup : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FMixupResult::StaticStruct(); }
	virtual FName GetExpectedProducerDPUName() const override { return FName("Mixup"); }

	UFUNCTION() bool IsGuard(const UDamageContext* DC) const;
	UFUNCTION() bool IsJustGuard(const UDamageContext* DC) const;
	UFUNCTION() bool IsGuardSuccess(const UDamageContext* DC) const;
};

// --- Guard ---
UCLASS(BlueprintType, meta = (DisplayName = "Guard"))
class SAGASTATS_API UConditionNode_Guard : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FGuardResult::StaticStruct(); }
	virtual FName GetExpectedProducerDPUName() const override { return FName("Guard"); }

	UFUNCTION() bool IsGuardSuccess(const UDamageContext* DC) const;
	UFUNCTION() bool IsJustGuard(const UDamageContext* DC) const;
};

// --- Hurt ---
UCLASS(BlueprintType, meta = (DisplayName = "Hurt"))
class SAGASTATS_API UConditionNode_Hurt : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FHurtResult::StaticStruct(); }
	virtual FName GetExpectedProducerDPUName() const override { return FName("Hurt"); }

	UFUNCTION() bool IsHurt(const UDamageContext* DC) const;
};

// --- Death ---
UCLASS(BlueprintType, meta = (DisplayName = "Death"))
class SAGASTATS_API UConditionNode_Death : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FDeathResult::StaticStruct(); }
	virtual FName GetExpectedProducerDPUName() const override { return FName("Death"); }

	UFUNCTION() bool IsDead(const UDamageContext* DC) const;
};

// --- Collapse ---
UCLASS(BlueprintType, meta = (DisplayName = "Collapse"))
class SAGASTATS_API UConditionNode_Collapse : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FCollapseResult::StaticStruct(); }
	virtual FName GetExpectedProducerDPUName() const override { return FName("Collapse"); }

	UFUNCTION() bool IsCollapse(const UDamageContext* DC) const;
};

// --- CollapseGuard（和 Collapse 共享 FCollapseResult，但绑定不同 DPU） ---
UCLASS(BlueprintType, meta = (DisplayName = "CollapseGuard"))
class SAGASTATS_API UConditionNode_CollapseGuard : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FCollapseResult::StaticStruct(); }
	virtual FName GetExpectedProducerDPUName() const override { return FName("CollapseGuard"); }

	UFUNCTION() bool IsCollapse(const UDamageContext* DC) const;
};

// --- Poison ---
UCLASS(BlueprintType, meta = (DisplayName = "Poison"))
class SAGASTATS_API UConditionNode_Poison : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FPoisonResult::StaticStruct(); }
	virtual FName GetExpectedProducerDPUName() const override { return FName("Poison"); }

	UFUNCTION() bool IsPoisoned(const UDamageContext* DC) const;
};

// --- AttackerBound ---
UCLASS(BlueprintType, meta = (DisplayName = "AttackerBound"))
class SAGASTATS_API UConditionNode_AttackerBound : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FAttackerBoundResult::StaticStruct(); }
	virtual FName GetExpectedProducerDPUName() const override { return FName("AttackerBound"); }

	UFUNCTION() bool IsBound(const UDamageContext* DC) const;
};

// ============================================================================
// Per-DPU ConditionNode 子类 — 信号 Fact（无领域方法，存在即 true）
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "LightningInAir"))
class SAGASTATS_API UConditionNode_LightningInAir : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FLightningInAirSignal::StaticStruct(); }
	virtual FName GetExpectedProducerDPUName() const override { return FName("LightningInAir"); }
};

UCLASS(BlueprintType, meta = (DisplayName = "LightningOnGround"))
class SAGASTATS_API UConditionNode_LightningOnGround : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FLightningOnGroundSignal::StaticStruct(); }
	virtual FName GetExpectedProducerDPUName() const override { return FName("LightningOnGround"); }
};

UCLASS(BlueprintType, meta = (DisplayName = "Toughness"))
class SAGASTATS_API UConditionNode_Toughness : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FToughnessSignal::StaticStruct(); }
	virtual FName GetExpectedProducerDPUName() const override { return FName("Toughness"); }
};

UCLASS(BlueprintType, meta = (DisplayName = "SuperArmor"))
class SAGASTATS_API UConditionNode_SuperArmor : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FSuperArmorSignal::StaticStruct(); }
	virtual FName GetExpectedProducerDPUName() const override { return FName("SuperArmor"); }
};

UCLASS(BlueprintType, meta = (DisplayName = "CollapseJustGuard"))
class SAGASTATS_API UConditionNode_CollapseJustGuard : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FCollapseJustGuardSignal::StaticStruct(); }
	virtual FName GetExpectedProducerDPUName() const override { return FName("CollapseJustGuard"); }
};

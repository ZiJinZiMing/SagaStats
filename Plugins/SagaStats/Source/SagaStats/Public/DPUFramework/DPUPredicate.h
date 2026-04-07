// DPUPredicate.h — 谓词容器（v4.8: 无 ResolveProducer，依赖通过 FactType 直接解析）
#pragma once

#include "CoreMinimal.h"
#include "DPUFramework/DPUCondition.h"
#include "DPUPredicate.generated.h"

class UDamageContext;

// ============================================================================
// UDPUPredicate — 谓词容器基类
// ============================================================================

UCLASS(Abstract, DefaultToInstanced, EditInlineNew, BlueprintType, CollapseCategories)
class SAGASTATS_API UDPUPredicate : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Predicate")
	bool bReverse = false;

	bool EvaluatePredicate(const UDamageContext* DC) const;

	virtual bool Evaluate(const UDamageContext* DC) const
		PURE_VIRTUAL(UDPUPredicate::Evaluate, return false;);

	virtual TArray<UScriptStruct*> GetConsumedFactTypes() const { return {}; }
	virtual FString GetDisplayString() const { return TEXT("(base)"); }
};

// ============================================================================
// UDPUPredicate_Single
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "Single"))
class SAGASTATS_API UDPUPredicate_Single : public UDPUPredicate
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "Predicate")
	TObjectPtr<UDPUCondition> Condition;

	virtual bool Evaluate(const UDamageContext* DC) const override;
	virtual TArray<UScriptStruct*> GetConsumedFactTypes() const override;
	virtual FString GetDisplayString() const override;
};

// ============================================================================
// UDPUPredicate_And
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "AND"))
class SAGASTATS_API UDPUPredicate_And : public UDPUPredicate
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "Predicate")
	TArray<TObjectPtr<UDPUPredicate>> Predicates;

	virtual bool Evaluate(const UDamageContext* DC) const override;
	virtual TArray<UScriptStruct*> GetConsumedFactTypes() const override;
	virtual FString GetDisplayString() const override;
};

// ============================================================================
// UDPUPredicate_Or
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "OR"))
class SAGASTATS_API UDPUPredicate_Or : public UDPUPredicate
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "Predicate")
	TArray<TObjectPtr<UDPUPredicate>> Predicates;

	virtual bool Evaluate(const UDamageContext* DC) const override;
	virtual TArray<UScriptStruct*> GetConsumedFactTypes() const override;
	virtual FString GetDisplayString() const override;
};

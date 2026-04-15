// DamagePredicate.h — 谓词容器（v4.8: 无 ResolveProducer，依赖通过 EffectType 直接解析）
#pragma once

#include "CoreMinimal.h"
#include "DamageFramework/DamageCondition.h"
#include "DamagePredicate.generated.h"

class UDamageContext;

// ============================================================================
// UDamagePredicate — 谓词容器基类
// ============================================================================

UCLASS(Abstract, DefaultToInstanced, EditInlineNew, BlueprintType, CollapseCategories)
class SAGASTATS_API UDamagePredicate : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "DamagePredicate")
	bool bReverse = false;

	bool EvaluatePredicate(const UDamageContext* Context) const;

	virtual bool Evaluate(const UDamageContext* Context) const
		PURE_VIRTUAL(UDamagePredicate::Evaluate, return false;);

	virtual TArray<UScriptStruct*> GetDependencyEffectTypes() const { return {}; }
	virtual FString GetDisplayString() const { return TEXT("(base)"); }
};

// ============================================================================
// UDamagePredicate_Single
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "Single"))
class SAGASTATS_API UDamagePredicate_Single : public UDamagePredicate
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "DamagePredicate")
	TObjectPtr<UDamageCondition> Condition;

	virtual bool Evaluate(const UDamageContext* Context) const override;
	virtual TArray<UScriptStruct*> GetDependencyEffectTypes() const override;
	virtual FString GetDisplayString() const override;
};

// ============================================================================
// UDamagePredicate_And
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "AND"))
class SAGASTATS_API UDamagePredicate_And : public UDamagePredicate
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "DamagePredicate")
	TArray<TObjectPtr<UDamagePredicate>> Predicates;

	virtual bool Evaluate(const UDamageContext* Context) const override;
	virtual TArray<UScriptStruct*> GetDependencyEffectTypes() const override;
	virtual FString GetDisplayString() const override;
};

// ============================================================================
// UDamagePredicate_Or
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "OR"))
class SAGASTATS_API UDamagePredicate_Or : public UDamagePredicate
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "DamagePredicate")
	TArray<TObjectPtr<UDamagePredicate>> Predicates;

	virtual bool Evaluate(const UDamageContext* Context) const override;
	virtual TArray<UScriptStruct*> GetDependencyEffectTypes() const override;
	virtual FString GetDisplayString() const override;
};

// DamagePredicate.h — 谓词容器（v4.8: 无 ResolveProducer，依赖通过 EffectType 直接解析）
#pragma once

#include "CoreMinimal.h"
#include "DamageFramework/DamageCondition.h"
#include "DamagePredicate.generated.h"

class UDamageContext;

/**
 * Condition 树显示单行 —— SVerticalBox 装配所需的结构化数据
 *
 * 每行要么是 Leaf（Single 原子，含一个 EffectType 依赖），要么是 Compound 的 header
 * （AND/OR 标题，EffectType = nullptr）。Slate 侧按行构造 widget，leaf 行右侧画色块。
 */
struct FConditionDisplayLine
{
	FString Text;
	UScriptStruct* EffectType = nullptr; // nullptr = 结构行（AND/OR 标题）
};

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

	/**
	 * ASCII 树格式递归收集每一行的（文本 + 依赖 EffectType）。
	 * @param FirstLinePrefix    本节点首行前缀（父节点传入，如 "├─ " / "└─ " / ""）
	 * @param ContinuationPrefix 本节点后续行的缩进前缀（如 "│  " / "   " / ""）
	 * @param OutLines           追加到这个数组
	 *
	 * Leaf（Single 原子）贡献一行，EffectType = Condition->GetEffectType()。
	 * Compound（And/Or）贡献 header 行（EffectType = nullptr）+ 递归所有孩子。
	 */
	virtual void CollectDisplayLines(
		const FString& FirstLinePrefix,
		const FString& ContinuationPrefix,
		TArray<FConditionDisplayLine>& OutLines) const {}

	/**
	 * 非虚 convenience：把 CollectDisplayLines 的所有行拼成多行字符串。
	 * 给 Tooltip 等需要纯字符串的 caller 使用。
	 */
	FString GetDisplayString(
		const FString& FirstLinePrefix = TEXT(""),
		const FString& ContinuationPrefix = TEXT("")) const
	{
		TArray<FConditionDisplayLine> Lines;
		CollectDisplayLines(FirstLinePrefix, ContinuationPrefix, Lines);
		TArray<FString> Texts;
		Texts.Reserve(Lines.Num());
		for (const FConditionDisplayLine& L : Lines)
		{
			Texts.Add(L.Text);
		}
		return FString::Join(Texts, TEXT("\n"));
	}
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
	virtual void CollectDisplayLines(
		const FString& FirstLinePrefix,
		const FString& ContinuationPrefix,
		TArray<FConditionDisplayLine>& OutLines) const override;
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
	virtual void CollectDisplayLines(
		const FString& FirstLinePrefix,
		const FString& ContinuationPrefix,
		TArray<FConditionDisplayLine>& OutLines) const override;
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
	virtual void CollectDisplayLines(
		const FString& FirstLinePrefix,
		const FString& ContinuationPrefix,
		TArray<FConditionDisplayLine>& OutLines) const override;
};

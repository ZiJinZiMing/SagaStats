// DamagePredicate.cpp — 谓词容器实现（v4.8: 无 ResolveProducer）
#include "DamageFramework/DamagePredicate.h"
#include "DamageFramework/DamageContext.h"

// ============================================================================
// UDamagePredicate
// ============================================================================

bool UDamagePredicate::EvaluatePredicate(const UDamageContext* Context) const
{
	bool Result = Evaluate(Context);
	return bReverse ? !Result : Result;
}

// ============================================================================
// ASCII 树格式辅助
// ============================================================================

namespace
{
	// Unicode box-drawing 字符（\u 转义避免源码编码依赖）
	const FString BranchMid  = TEXT("\u251C\u2500 ");  // ├─
	const FString BranchLast = TEXT("\u2514\u2500 ");  // └─
	const FString BarCont    = TEXT("\u2502  ");       // │
	const FString SpaceCont  = TEXT("   ");            // (3 空格)

	/** And/Or 共享的行收集：header 行 + 每个孩子递归 */
	void CollectCompoundLines(
		const TArray<TObjectPtr<UDamagePredicate>>& Predicates,
		const FString& FirstLinePrefix,
		const FString& ContinuationPrefix,
		const TCHAR* Keyword,   // "AND" 或 "OR"
		bool bReverseSelf,
		TArray<FConditionDisplayLine>& OutLines)
	{
		// 收集非 null 孩子
		TArray<UDamagePredicate*> Valid;
		Valid.Reserve(Predicates.Num());
		for (const auto& P : Predicates)
		{
			if (P) Valid.Add(P.Get());
		}

		// Header 文本：前缀 + (NOT if bReverse) + 运算符
		FString HeaderText = FirstLinePrefix;
		if (bReverseSelf)
		{
			HeaderText += TEXT("NOT ");
		}
		HeaderText += Keyword;

		if (Valid.Num() == 0)
		{
			// 空集合：单行 "... AND (empty)"
			FConditionDisplayLine EmptyLine;
			EmptyLine.Text = HeaderText + TEXT(" (empty)");
			OutLines.Add(EmptyLine);
			return;
		}

		// Header 结构行（EffectType = nullptr）
		FConditionDisplayLine HeaderLine;
		HeaderLine.Text = HeaderText;
		OutLines.Add(HeaderLine);

		// 递归孩子
		for (int32 i = 0; i < Valid.Num(); ++i)
		{
			const bool bLast = (i == Valid.Num() - 1);
			const FString ChildFirstPrefix = ContinuationPrefix + (bLast ? BranchLast : BranchMid);
			const FString ChildContPrefix  = ContinuationPrefix + (bLast ? SpaceCont : BarCont);
			Valid[i]->CollectDisplayLines(ChildFirstPrefix, ChildContPrefix, OutLines);
		}
	}
}

// ============================================================================
// UDamagePredicate_Single
// ============================================================================

bool UDamagePredicate_Single::Evaluate(const UDamageContext* Context) const
{
	if (!Condition) return false;
	return Condition->EvaluateCondition(Context);
}

TArray<UScriptStruct*> UDamagePredicate_Single::GetDependencyEffectTypes() const
{
	if (Condition) return {Condition->GetEffectType()};
	return {};
}

void UDamagePredicate_Single::CollectDisplayLines(
	const FString& FirstLinePrefix,
	const FString& ContinuationPrefix,
	TArray<FConditionDisplayLine>& OutLines) const
{
	const FString Body = Condition ? Condition->GetDisplayString() : TEXT("(empty)");

	FConditionDisplayLine Line;
	Line.Text = FirstLinePrefix + (bReverse ? TEXT("NOT ") : TEXT("")) + Body;
	Line.EffectType = Condition ? Condition->GetEffectType() : nullptr;
	OutLines.Add(Line);
}

// ============================================================================
// UDamagePredicate_And
// ============================================================================

bool UDamagePredicate_And::Evaluate(const UDamageContext* Context) const
{
	if (Predicates.Num() == 0) return false;
	for (const auto& P : Predicates)
	{
		if (!P) continue;
		if (!P->EvaluatePredicate(Context)) return false;
	}
	return true;
}

TArray<UScriptStruct*> UDamagePredicate_And::GetDependencyEffectTypes() const
{
	TArray<UScriptStruct*> Result;
	for (const auto& P : Predicates)
	{
		if (!P) continue;
		for (UScriptStruct* Type : P->GetDependencyEffectTypes())
			Result.AddUnique(Type);
	}
	return Result;
}

void UDamagePredicate_And::CollectDisplayLines(
	const FString& FirstLinePrefix,
	const FString& ContinuationPrefix,
	TArray<FConditionDisplayLine>& OutLines) const
{
	CollectCompoundLines(Predicates, FirstLinePrefix, ContinuationPrefix, TEXT("AND"), bReverse, OutLines);
}

// ============================================================================
// UDamagePredicate_Or
// ============================================================================

bool UDamagePredicate_Or::Evaluate(const UDamageContext* Context) const
{
	if (Predicates.Num() == 0) return false;
	for (const auto& P : Predicates)
	{
		if (!P) continue;
		if (P->EvaluatePredicate(Context)) return true;
	}
	return false;
}

TArray<UScriptStruct*> UDamagePredicate_Or::GetDependencyEffectTypes() const
{
	TArray<UScriptStruct*> Result;
	for (const auto& P : Predicates)
	{
		if (!P) continue;
		for (UScriptStruct* Type : P->GetDependencyEffectTypes())
			Result.AddUnique(Type);
	}
	return Result;
}

void UDamagePredicate_Or::CollectDisplayLines(
	const FString& FirstLinePrefix,
	const FString& ContinuationPrefix,
	TArray<FConditionDisplayLine>& OutLines) const
{
	CollectCompoundLines(Predicates, FirstLinePrefix, ContinuationPrefix, TEXT("OR"), bReverse, OutLines);
}

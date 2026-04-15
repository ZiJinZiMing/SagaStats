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

FString UDamagePredicate_Single::GetDisplayString() const
{
	if (!Condition) return TEXT("(empty)");
	return Condition->GetDisplayString();
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

FString UDamagePredicate_And::GetDisplayString() const
{
	TArray<FString> Parts;
	for (const auto& P : Predicates)
	{
		if (!P) continue;
		FString S = P->GetDisplayString();
		if (P->bReverse) S = FString::Printf(TEXT("!(%s)"), *S);
		Parts.Add(S);
	}
	return FString::Join(Parts, TEXT(" AND "));
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

FString UDamagePredicate_Or::GetDisplayString() const
{
	TArray<FString> Parts;
	for (const auto& P : Predicates)
	{
		if (!P) continue;
		FString S = P->GetDisplayString();
		if (P->bReverse) S = FString::Printf(TEXT("!(%s)"), *S);
		Parts.Add(S);
	}
	return FString::Join(Parts, TEXT(" OR "));
}

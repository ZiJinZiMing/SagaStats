// DPUPredicate.cpp — 谓词容器实现（v4.8: 无 ResolveProducer）
#include "DPUFramework/DPUPredicate.h"
#include "DPUFramework/DamageContext.h"

// ============================================================================
// UDPUPredicate
// ============================================================================

bool UDPUPredicate::EvaluatePredicate(const UDamageContext* DC) const
{
	bool Result = Evaluate(DC);
	return bReverse ? !Result : Result;
}

// ============================================================================
// UDPUPredicate_Single
// ============================================================================

bool UDPUPredicate_Single::Evaluate(const UDamageContext* DC) const
{
	if (!Condition) return false;
	return Condition->EvaluateCondition(DC);
}

TArray<UScriptStruct*> UDPUPredicate_Single::GetConsumedFactTypes() const
{
	if (Condition) return Condition->GetConsumedFactTypes();
	return {};
}

FString UDPUPredicate_Single::GetDisplayString() const
{
	if (!Condition) return TEXT("(empty)");
	return Condition->GetDisplayString();
}

// ============================================================================
// UDPUPredicate_And
// ============================================================================

bool UDPUPredicate_And::Evaluate(const UDamageContext* DC) const
{
	if (Predicates.Num() == 0) return false;
	for (const auto& P : Predicates)
	{
		if (!P) continue;
		if (!P->EvaluatePredicate(DC)) return false;
	}
	return true;
}

TArray<UScriptStruct*> UDPUPredicate_And::GetConsumedFactTypes() const
{
	TArray<UScriptStruct*> Result;
	for (const auto& P : Predicates)
	{
		if (!P) continue;
		for (UScriptStruct* Type : P->GetConsumedFactTypes())
			Result.AddUnique(Type);
	}
	return Result;
}

FString UDPUPredicate_And::GetDisplayString() const
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
// UDPUPredicate_Or
// ============================================================================

bool UDPUPredicate_Or::Evaluate(const UDamageContext* DC) const
{
	if (Predicates.Num() == 0) return false;
	for (const auto& P : Predicates)
	{
		if (!P) continue;
		if (P->EvaluatePredicate(DC)) return true;
	}
	return false;
}

TArray<UScriptStruct*> UDPUPredicate_Or::GetConsumedFactTypes() const
{
	TArray<UScriptStruct*> Result;
	for (const auto& P : Predicates)
	{
		if (!P) continue;
		for (UScriptStruct* Type : P->GetConsumedFactTypes())
			Result.AddUnique(Type);
	}
	return Result;
}

FString UDPUPredicate_Or::GetDisplayString() const
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

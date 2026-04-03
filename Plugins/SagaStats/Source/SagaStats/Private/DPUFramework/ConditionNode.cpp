// ConditionNode.cpp — 条件树框架核心实现
// Per-DPU 子类的领域方法实现在 Sekiro/ 子目录的各 DPU .cpp 文件中。
#include "DPUFramework/ConditionNode.h"
#include "DPUFramework/DamageContext.h"
#include "SagaStatsLog.h"

// ============================================================================
// UConditionNode 基类
// ============================================================================

bool UConditionNode::EvaluateCondition(const UDamageContext* DC) const
{
	bool Result = Evaluate(DC);
	return bReverse ? !Result : Result;
}

void UConditionNode::ResolveProducer(const TMap<UScriptStruct*, FName>& FactTypeToProducerMap)
{
	UScriptStruct* FactType = GetConsumedFactType();
	if (FactType)
	{
		if (const FName* Found = FactTypeToProducerMap.Find(FactType))
		{
			ResolvedProducerDPU = *Found;
		}
	}
}

// ============================================================================
// UConditionNode_And
// ============================================================================

bool UConditionNode_And::Evaluate(const UDamageContext* DC) const
{
	if (Children.Num() == 0) return false;
	for (const auto& Child : Children)
	{
		if (!Child) continue;
		if (!Child->EvaluateCondition(DC)) return false;
	}
	return true;
}

TArray<FName> UConditionNode_And::GetConsumedDPUs() const
{
	TArray<FName> Result;
	for (const auto& Child : Children)
	{
		if (!Child) continue;
		for (const FName& Name : Child->GetConsumedDPUs())
			Result.AddUnique(Name);
	}
	return Result;
}

FString UConditionNode_And::GetDisplayString() const
{
	TArray<FString> Parts;
	for (const auto& Child : Children)
	{
		if (!Child) continue;
		FString S = Child->GetDisplayString();
		if (Child->bReverse) S = FString::Printf(TEXT("!(%s)"), *S);
		Parts.Add(S);
	}
	return FString::Join(Parts, TEXT(" AND "));
}

void UConditionNode_And::ResolveProducer(const TMap<UScriptStruct*, FName>& Map)
{
	for (auto& Child : Children)
		if (Child) Child->ResolveProducer(Map);
}

// ============================================================================
// UConditionNode_Or
// ============================================================================

bool UConditionNode_Or::Evaluate(const UDamageContext* DC) const
{
	if (Children.Num() == 0) return false;
	for (const auto& Child : Children)
	{
		if (!Child) continue;
		if (Child->EvaluateCondition(DC)) return true;
	}
	return false;
}

TArray<FName> UConditionNode_Or::GetConsumedDPUs() const
{
	TArray<FName> Result;
	for (const auto& Child : Children)
	{
		if (!Child) continue;
		for (const FName& Name : Child->GetConsumedDPUs())
			Result.AddUnique(Name);
	}
	return Result;
}

FString UConditionNode_Or::GetDisplayString() const
{
	TArray<FString> Parts;
	for (const auto& Child : Children)
	{
		if (!Child) continue;
		FString S = Child->GetDisplayString();
		if (Child->bReverse) S = FString::Printf(TEXT("!(%s)"), *S);
		Parts.Add(S);
	}
	return FString::Join(Parts, TEXT(" OR "));
}

void UConditionNode_Or::ResolveProducer(const TMap<UScriptStruct*, FName>& Map)
{
	for (auto& Child : Children)
		if (Child) Child->ResolveProducer(Map);
}

// ============================================================================
// UConditionNode_ContextCheck
// ============================================================================

bool UConditionNode_ContextCheck::Evaluate(const UDamageContext* DC) const
{
	if (!DC || ContextKey.IsNone()) return false;
	return DC->HasFact(ContextKey) && DC->GetBool(ContextKey);
}

FString UConditionNode_ContextCheck::GetDisplayString() const
{
	return FString::Printf(TEXT("Ctx:%s"), *ContextKey.ToString());
}

// ============================================================================
// UConditionNode_DPUBase — 共享实现
// ============================================================================

bool UConditionNode_DPUBase::Evaluate(const UDamageContext* DC) const
{
	if (!DC || ResolvedProducerDPU.IsNone()) return false;

	if (MethodName.IsNone())
	{
		return DC->HasFact(ResolvedProducerDPU);
	}

	if (!DC->HasFact(ResolvedProducerDPU)) return false;

	return CallDomainMethod(DC);
}

TArray<FName> UConditionNode_DPUBase::GetConsumedDPUs() const
{
	if (!ResolvedProducerDPU.IsNone()) return {ResolvedProducerDPU};
	return {};
}

FString UConditionNode_DPUBase::GetDisplayString() const
{
	FString DPUStr = ResolvedProducerDPU.IsNone() ? GetClass()->GetName() : ResolvedProducerDPU.ToString();
	if (MethodName.IsNone()) return DPUStr;
	return FString::Printf(TEXT("%s.%s"), *DPUStr, *MethodName.ToString());
}

bool UConditionNode_DPUBase::CallDomainMethod(const UDamageContext* DC) const
{
	UFunction* Func = FindFunction(MethodName);
	if (!Func)
	{
		UE_LOG(LogSagaStats, Warning, TEXT("ConditionNode %s: 未找到领域方法 %s"),
			*GetClass()->GetName(), *MethodName.ToString());
		return false;
	}

	struct FBoolParams { const UDamageContext* DC; bool ReturnValue; };
	FBoolParams Params{DC, false};
	const_cast<UConditionNode_DPUBase*>(this)->ProcessEvent(Func, &Params);
	return Params.ReturnValue;
}

bool UConditionNode_DPUBase::IsValidDomainMethod(const UFunction* Func)
{
	if (!Func) return false;

	// 领域方法必须定义在 DPUBase 的具体子类上（排除 DPUBase 本身及其父类的所有函数）
	const UClass* Owner = Func->GetOwnerClass();
	if (!Owner || !Owner->IsChildOf(UConditionNode_DPUBase::StaticClass())
		|| Owner == UConditionNode_DPUBase::StaticClass())
	{
		return false;
	}

	// 排除 Blueprint 内部函数
	if (Func->GetName().StartsWith(TEXT("ExecuteUbergraph"))) return false;

	return true;
}

TArray<FString> UConditionNode_DPUBase::GetMethodOptions() const
{
	TArray<FString> Result;
	Result.Add(TEXT("None"));

	const UClass* ClassToScan = GetClass();
	for (TFieldIterator<UFunction> It(ClassToScan); It; ++It)
	{
		if (IsValidDomainMethod(*It))
		{
			Result.Add(It->GetName());
		}
	}
	return Result;
}

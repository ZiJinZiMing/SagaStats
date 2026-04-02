// ConditionNode.cpp — 条件树节点实现（v4.6: 按 DPU 分子类，领域方法为 UFUNCTION）
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
	FName Expected = GetExpectedProducerDPUName();
	if (!Expected.IsNone())
	{
		ResolvedProducerDPU = Expected;
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
	{
		if (Child) Child->ResolveProducer(Map);
	}
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
	{
		if (Child) Child->ResolveProducer(Map);
	}
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

	// 信号检查：MethodName 为 None → 存在即 true
	if (MethodName.IsNone())
	{
		return DC->HasFact(ResolvedProducerDPU);
	}

	// 生产者的 Fact 是否存在
	if (!DC->HasFact(ResolvedProducerDPU)) return false;

	// 调用命名的领域方法
	return CallDomainMethod(DC);
}

TArray<FName> UConditionNode_DPUBase::GetConsumedDPUs() const
{
	FName Producer = GetExpectedProducerDPUName();
	if (!Producer.IsNone()) return {Producer};
	return {};
}

FString UConditionNode_DPUBase::GetDisplayString() const
{
	FString DPUStr = GetExpectedProducerDPUName().ToString();
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

	// 领域方法签名：bool MethodName(const UDamageContext*) const
	struct FParams
	{
		const UDamageContext* DC;
		bool ReturnValue;
	};
	FParams Params{DC, false};
	const_cast<UConditionNode_DPUBase*>(this)->ProcessEvent(Func, &Params);
	return Params.ReturnValue;
}

bool UConditionNode_DPUBase::IsValidDomainMethod(const UFunction* Func, const UClass* LeafClass)
{
	if (!Func || !LeafClass) return false;

	// 必须定义在叶子类上（不是 DPUBase 或更上层）
	if (Func->GetOwnerClass() != LeafClass) return false;

	// 必须是 UFUNCTION
	if (!Func->HasAnyFunctionFlags(FUNC_Native | FUNC_BlueprintCallable | FUNC_Final)) return false;

	// 排除 UE 内部函数
	if (Func->GetName().StartsWith(TEXT("ExecuteUbergraph"))) return false;

	return true;
}

TArray<FString> UConditionNode_DPUBase::GetMethodOptions() const
{
	TArray<FString> Result;
	Result.Add(TEXT("None")); // 信号检查

	const UClass* LeafClass = GetClass();
	for (TFieldIterator<UFunction> It(LeafClass); It; ++It)
	{
		if (IsValidDomainMethod(*It, LeafClass))
		{
			Result.Add(It->GetName());
		}
	}
	return Result;
}

#if WITH_EDITOR
void UConditionNode_DPUBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	// SourceDPU 概念在 v4.6 中不存在——子类本身就是绑定
	// 这里暂无需重置 MethodName（切换子类类型时 UE 会重新创建对象）
}
#endif

// ============================================================================
// Per-DPU 领域方法实现
// ============================================================================

// --- Mixup ---
bool UConditionNode_Mixup::IsGuard(const UDamageContext* DC) const
{
	const FMixupResult* F = DC->GetFact<FMixupResult>(ResolvedProducerDPU);
	return F ? F->bIsGuard : false;
}
bool UConditionNode_Mixup::IsJustGuard(const UDamageContext* DC) const
{
	const FMixupResult* F = DC->GetFact<FMixupResult>(ResolvedProducerDPU);
	return F ? F->bIsJustGuard : false;
}
bool UConditionNode_Mixup::IsGuardSuccess(const UDamageContext* DC) const
{
	const FMixupResult* F = DC->GetFact<FMixupResult>(ResolvedProducerDPU);
	return F ? F->bIsGuard : false; // 格挡成功 = 格挡触发
}

// --- Guard ---
bool UConditionNode_Guard::IsGuardSuccess(const UDamageContext* DC) const
{
	const FGuardResult* F = DC->GetFact<FGuardResult>(ResolvedProducerDPU);
	return F ? F->bGuardSuccess : false;
}
bool UConditionNode_Guard::IsJustGuard(const UDamageContext* DC) const
{
	const FGuardResult* F = DC->GetFact<FGuardResult>(ResolvedProducerDPU);
	return F ? F->bIsJustGuard : false;
}

// --- Hurt ---
bool UConditionNode_Hurt::IsHurt(const UDamageContext* DC) const
{
	const FHurtResult* F = DC->GetFact<FHurtResult>(ResolvedProducerDPU);
	return F ? F->bIsHurt : false;
}

// --- Death ---
bool UConditionNode_Death::IsDead(const UDamageContext* DC) const
{
	const FDeathResult* F = DC->GetFact<FDeathResult>(ResolvedProducerDPU);
	return F ? F->bIsDead : false;
}

// --- Collapse ---
bool UConditionNode_Collapse::IsCollapse(const UDamageContext* DC) const
{
	const FCollapseResult* F = DC->GetFact<FCollapseResult>(ResolvedProducerDPU);
	return F ? F->bIsCollapse : false;
}

// --- CollapseGuard ---
bool UConditionNode_CollapseGuard::IsCollapse(const UDamageContext* DC) const
{
	const FCollapseResult* F = DC->GetFact<FCollapseResult>(ResolvedProducerDPU);
	return F ? F->bIsCollapse : false;
}

// --- Poison ---
bool UConditionNode_Poison::IsPoisoned(const UDamageContext* DC) const
{
	const FPoisonResult* F = DC->GetFact<FPoisonResult>(ResolvedProducerDPU);
	return F ? F->bIsPoisoned : false;
}

// --- AttackerBound ---
bool UConditionNode_AttackerBound::IsBound(const UDamageContext* DC) const
{
	const FAttackerBoundResult* F = DC->GetFact<FAttackerBoundResult>(ResolvedProducerDPU);
	return F ? F->bIsBound : false;
}

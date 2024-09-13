/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "Meter/SSIncreaseMeter.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

USSIncreaseMeter::USSIncreaseMeter(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	InitAccumulate(0.f);
	InitDegeneration(0.f);
	InitDegenerationCooldown(0.f);
}

void USSIncreaseMeter::OnAccumulate_Implementation(const FSSAttributeSetExecutionData& Data)
{
	float OldCurrent = GetCurrent();
	SetAttributeValue(GetCurrentAttribute(), GetAccumulate() + OldCurrent);
	SetImpactedAccumulate(GetCurrent() - OldCurrent);

	if (GetImpactedAccumulate() > 0 && GetDegenerationCooldown() > 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(DegenerationCooldownTimer);
		GetWorld()->GetTimerManager().SetTimer(DegenerationCooldownTimer, FTimerDelegate::CreateUObject(this, &ThisClass::OnDegenerationCooldownTimerFinish), GetDegeneration(), false);
	}
}

void USSIncreaseMeter::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetAccumulateAttribute())
	{
		const FSSAttributeSetExecutionData ExecutionData(Data);
		OnAccumulate(ExecutionData);
		SetAccumulate(0);
	}
}

void USSIncreaseMeter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CanRegen())
	{
		if (FillStatus != EMeterFillStatus::Emptied)
		{
			float NewValue = GetCurrent() - GetDegeneration() * DeltaTime;
			SetAttributeValue(GetCurrentAttribute(), NewValue);
		}
	}
}

void USSIncreaseMeter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	DOREPLIFETIME_CONDITION_NOTIFY(USSIncreaseMeter, Degeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USSIncreaseMeter, DegenerationCooldown, COND_None, REPNOTIFY_Always);
}

bool USSIncreaseMeter::CanRegen() const
{
	return GetDegeneration() > 0 && !DegenerationCooldownTimer.IsValid();
}

void USSIncreaseMeter::OnDegenerationCooldownTimerFinish()
{
	DegenerationCooldownTimer.Invalidate();

	if (GetDegeneration() <= 0.f)
	{
		SetAttributeValue(GetCurrentAttribute(),METER_MINIMUM);
	}
}

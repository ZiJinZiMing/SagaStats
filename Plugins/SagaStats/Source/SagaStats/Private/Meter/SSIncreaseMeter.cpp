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
	InitDegeneration(0.f);
	InitDegenerationCooldown(0.f);
}

void USSIncreaseMeter::InitFromMetaDataTable(const UDataTable* DataTable)
{
	Super::InitFromMetaDataTable(DataTable);
}

void USSIncreaseMeter::OnAccumulate_Implementation(const FSSAttributeSetExecutionData& Data)
{
	Super::OnAccumulate_Implementation(Data);
	
	if (GetImpactedAccumulate() > 0 && GetDegenerationCooldown() > 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(DegenerationCooldownTimer);
		GetWorld()->GetTimerManager().SetTimer(DegenerationCooldownTimer, FTimerDelegate::CreateUObject(this, &ThisClass::OnDegenerationCooldownTimerFinish), GetDegenerationCooldown(), false);
	}
}


void USSIncreaseMeter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CanDegeneration())
	{
		if (!IsEmptied())
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

bool USSIncreaseMeter::CanDegeneration() const
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

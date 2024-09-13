/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "Meter/SSDecreaseMeter.h"

#include "Net/UnrealNetwork.h"

DEFINE_ENUM_TO_STRING(EMeterState, "/Script/SagaStats")


USSDecreaseMeter::USSDecreaseMeter(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	Current.MinValue.ClampType = ESSClampingType::AttributeBased;
	Current.MinValue.Attribute = GetMinimumClampAttribute();
	Current.MaxValue.ClampType = ESSClampingType::AttributeBased;
	Current.MaxValue.Attribute = GetMaximumAttribute();
	
}

void USSDecreaseMeter::OnRep_MeterState(const EMeterState& OldValue)
{
}

void USSDecreaseMeter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Regeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, RegenerationCooldown, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, LockDuration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, ResetRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, ImmuneThreshold, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MeterState, COND_None, REPNOTIFY_OnChanged);

	
}

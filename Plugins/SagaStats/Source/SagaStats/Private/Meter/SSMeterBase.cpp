/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "Meter/SSMeterBase.h"
#include "Net/UnrealNetwork.h"


USSMeterBase::USSMeterBase(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	Current.MinValue.ClampType = ESSClampingType::Float;
	Current.MinValue.Value = METER_MINIMUM;
	Current.MaxValue.ClampType = ESSClampingType::AttributeBased;
	FGameplayAttribute MaximumAttribute = GetMaximumAttribute();
	Current.MaxValue.Attribute = MaximumAttribute;

	InitMaximum(1.f);
}


bool USSMeterBase::IsFilled() const
{
	return GreaterOrNearlyEqual(GetCurrent(), GetMaximum());
}

bool USSMeterBase::IsEmptied() const
{
	return LessOrNearlyEqual(GetCurrent(),METER_MINIMUM);
}

void USSMeterBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaximumAttribute())
	{
		OnMaximumChanged(OldValue, NewValue);
	}
	else if (Attribute == GetCurrentAttribute())
	{
		OnCurrentChanged(OldValue, NewValue);
	}
}

void USSMeterBase::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void USSMeterBase::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	if (Attribute == GetCurrentAttribute())
	{
		PerformClampingForAttribute(Attribute, NewValue);
	}
}

float USSMeterBase::GetPercent() const
{
	if (GetMaximum() == 0) return 0.f;
	return GetCurrent() / GetMaximum();
}

void USSMeterBase::OnEmptied_Implementation()
{
}

void USSMeterBase::OnFilled_Implementation()
{
}

void USSMeterBase::OnMaximumChanged(float OldValue, float NewValue)
{
	//Current is only clamped by the Maximum value and does not increase with the Maximum value
	float NewCurrent = GetCurrent();
	if (PerformClampingForAttribute(GetCurrentAttribute(), NewCurrent))
	{
		bool ClampedFilled = GetCurrent() < OldValue && GetCurrent() >= NewValue;
		SetAttributeValue(GetCurrentAttribute(), NewCurrent);
		if (ClampedFilled)
		{
			OnFilled();
		}
	}
}

void USSMeterBase::OnCurrentChanged(float OldValue, float NewValue)
{
	if (IsEmptied())
	{
		if (!LessOrNearlyEqual(OldValue,METER_MINIMUM))
		{
			OnEmptied();
		}
	}
	else if (IsFilled())
	{
		if (!GreaterOrNearlyEqual(OldValue, GetMaximum()))
		{
			OnFilled();
		}
	}
}

bool USSMeterBase::ShouldTick() const
{
	return GetOwningActor()->HasAuthority();
}

void USSMeterBase::Tick(float DeltaTime)
{
}

void USSMeterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USSMeterBase, Current, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USSMeterBase, Maximum, COND_None, REPNOTIFY_Always);
};


bool USSMeterBase::GreaterOrNearlyEqual(float A, float B)
{
	return A >= B || FMath::IsNearlyEqual(A, B, KINDA_SMALL_NUMBER);
}

bool USSMeterBase::LessOrNearlyEqual(float A, float B)
{
	return A <= B || FMath::IsNearlyEqual(A, B, KINDA_SMALL_NUMBER);
}

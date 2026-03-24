// DamageContext.cpp
#include "DPUFramework/DamageContext.h"

void UDamageContext::Set(FName Key, const FDCValue& Value)
{
	Fields.Add(Key, Value);
}

void UDamageContext::SetBool(FName Key, bool Value)
{
	Fields.Add(Key, FDCValue::FromBool(Value));
}

void UDamageContext::SetFloat(FName Key, float Value)
{
	Fields.Add(Key, FDCValue::FromFloat(Value));
}

void UDamageContext::SetInt(FName Key, int32 Value)
{
	Fields.Add(Key, FDCValue::FromInt(Value));
}

void UDamageContext::SetName(FName Key, FName Value)
{
	Fields.Add(Key, FDCValue::FromName(Value));
}

void UDamageContext::SetVector(FName Key, const FVector& Value)
{
	Fields.Add(Key, FDCValue::FromVector(Value));
}

FDCValue UDamageContext::Get(FName Key) const
{
	if (const FDCValue* Found = Fields.Find(Key))
	{
		return *Found;
	}
	return FDCValue(); // None 类型 — AsBool() == false (R3)
}

bool UDamageContext::GetBool(FName Key) const
{
	return Get(Key).AsBool();
}

float UDamageContext::GetFloat(FName Key) const
{
	return Get(Key).AsFloat();
}

int32 UDamageContext::GetInt(FName Key) const
{
	return Get(Key).AsInt();
}

bool UDamageContext::Contains(FName Key) const
{
	return Fields.Contains(Key);
}

void UDamageContext::Reset()
{
	Fields.Empty();
}

FString UDamageContext::DumpToString() const
{
	FString Result = TEXT("DC Fields:\n");
	for (const auto& Pair : Fields)
	{
		Result += FString::Printf(TEXT("  %s = %s\n"), *Pair.Key.ToString(), *Pair.Value.ToString());
	}
	return Result;
}

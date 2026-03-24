// DCValue.cpp
#include "DPUFramework/DCValue.h"

FDCValue FDCValue::FromBool(bool V)
{
	FDCValue Result;
	Result.Type = EDCValueType::Bool;
	Result.BoolValue = V;
	return Result;
}

FDCValue FDCValue::FromInt(int32 V)
{
	FDCValue Result;
	Result.Type = EDCValueType::Int;
	Result.IntValue = V;
	return Result;
}

FDCValue FDCValue::FromFloat(float V)
{
	FDCValue Result;
	Result.Type = EDCValueType::Float;
	Result.FloatValue = V;
	return Result;
}

FDCValue FDCValue::FromName(FName V)
{
	FDCValue Result;
	Result.Type = EDCValueType::Name;
	Result.NameValue = V;
	return Result;
}

FDCValue FDCValue::FromVector(const FVector& V)
{
	FDCValue Result;
	Result.Type = EDCValueType::Vector;
	Result.VectorValue = V;
	return Result;
}

bool FDCValue::AsBool() const
{
	switch (Type)
	{
	case EDCValueType::None:   return false;
	case EDCValueType::Bool:   return BoolValue;
	case EDCValueType::Int:    return IntValue != 0;
	case EDCValueType::Float:  return FloatValue != 0.f;
	case EDCValueType::Name:   return NameValue != NAME_None;
	case EDCValueType::Vector: return !VectorValue.IsZero();
	default:                   return false;
	}
}

float FDCValue::AsFloat() const
{
	switch (Type)
	{
	case EDCValueType::None:  return 0.f;
	case EDCValueType::Bool:  return BoolValue ? 1.f : 0.f;
	case EDCValueType::Int:   return static_cast<float>(IntValue);
	case EDCValueType::Float: return FloatValue;
	default:                  return 0.f;
	}
}

int32 FDCValue::AsInt() const
{
	switch (Type)
	{
	case EDCValueType::None:  return 0;
	case EDCValueType::Bool:  return BoolValue ? 1 : 0;
	case EDCValueType::Int:   return IntValue;
	case EDCValueType::Float: return static_cast<int32>(FloatValue);
	default:                  return 0;
	}
}

FName FDCValue::AsName() const
{
	if (Type == EDCValueType::Name)
	{
		return NameValue;
	}
	return NAME_None;
}

FString FDCValue::ToString() const
{
	switch (Type)
	{
	case EDCValueType::None:   return TEXT("None");
	case EDCValueType::Bool:   return BoolValue ? TEXT("true") : TEXT("false");
	case EDCValueType::Int:    return FString::FromInt(IntValue);
	case EDCValueType::Float:  return FString::SanitizeFloat(FloatValue);
	case EDCValueType::Name:   return NameValue.ToString();
	case EDCValueType::Vector: return VectorValue.ToString();
	default:                   return TEXT("Unknown");
	}
}

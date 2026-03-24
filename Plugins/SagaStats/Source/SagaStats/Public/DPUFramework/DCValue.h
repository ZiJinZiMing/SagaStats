// DCValue.h — FDCValue 变体类型，用于 DamageContext 字段
#pragma once

#include "CoreMinimal.h"
#include "DCValue.generated.h"

UENUM(BlueprintType)
enum class EDCValueType : uint8
{
	None,
	Bool,
	Int,
	Float,
	Name,
	Vector
};

USTRUCT(BlueprintType)
struct SAGASTATS_API FDCValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDCValueType Type = EDCValueType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool BoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FloatValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NameValue = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector VectorValue = FVector::ZeroVector;

	// 工厂方法
	static FDCValue FromBool(bool V);
	static FDCValue FromInt(int32 V);
	static FDCValue FromFloat(float V);
	static FDCValue FromName(FName V);
	static FDCValue FromVector(const FVector& V);

	// 类型转换 (R3: None -> false/0)
	bool AsBool() const;
	float AsFloat() const;
	int32 AsInt() const;
	FName AsName() const;

	bool IsNone() const { return Type == EDCValueType::None; }

	FString ToString() const;
};

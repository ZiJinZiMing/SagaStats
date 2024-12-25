// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SagaStatsType.generated.h"


/*State of Meter*/
UENUM(BlueprintType)
enum class EMeterState : uint8
{
	Normal,
	Lock,
	Reset,
};
DECLARE_ENUM_TO_STRING(EMeterState);

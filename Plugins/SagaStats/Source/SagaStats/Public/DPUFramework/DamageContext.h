// DamageContext.h — UDamageContext: 单次伤害事件的共享上下文 (DC)
#pragma once

#include "CoreMinimal.h"
#include "DPUFramework/DCValue.h"
#include "DamageContext.generated.h"

/**
 * UDamageContext (DC)
 * 单次伤害事件的共享上下文。保存上下文信息和 DPU 输出结果。
 * 生命周期：事件开始时创建，事件结束时销毁。
 * R1: DC 是 DPU 之间唯一的通信媒介。
 * R3: 缺失字段返回 None (AsBool() == false)。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UDamageContext : public UObject
{
	GENERATED_BODY()

public:
	// --- 写入 (DPU 调用) ---
	void Set(FName Key, const FDCValue& Value);
	void SetBool(FName Key, bool Value);
	void SetFloat(FName Key, float Value);
	void SetInt(FName Key, int32 Value);
	void SetName(FName Key, FName Value);
	void SetVector(FName Key, const FVector& Value);

	// --- 读取 (DPU 和 Condition 调用) ---
	// R3: 缺失字段返回 None 类型的 FDCValue
	FDCValue Get(FName Key) const;
	bool GetBool(FName Key) const;
	float GetFloat(FName Key) const;
	int32 GetInt(FName Key) const;

	// --- 查询 ---
	bool Contains(FName Key) const;
	const TMap<FName, FDCValue>& GetAllFields() const { return Fields; }

	// --- 生命周期 ---
	void Reset();

	// --- 调试 ---
	FString DumpToString() const;

private:
	UPROPERTY()
	TMap<FName, FDCValue> Fields;
};

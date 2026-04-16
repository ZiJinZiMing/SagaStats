/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageContextLibrary.h — DamageContext 蓝图 CustomThunk 接口（static Library）
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintInstancedStructLibrary.h"
#include "DamageContextLibrary.generated.h"

class UDamageContext;

/**
 * UDamageContextLibrary
 *
 * DamageContext 的蓝图接口。使用 CustomThunk + CustomStructureParam 实现通配结构体引脚。
 * 参照引擎 UBlueprintInstancedStructLibrary 的模式：static 函数 + CustomThunk。
 */
UCLASS()
class SAGASTATS_API UDamageContextLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 写入 Effect 到 DamageContext（类型由连线的结构体引脚自动推断）。
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "DamageContext", meta = (CustomStructureParam = "Value"))
	static void SetEffect(UDamageContext* Context, const int32& Value);

	/**
	 * 从 DamageContext 读取 Effect（类型由连线的结构体引脚自动推断）。
	 * Valid/NotValid 执行引脚指示 Effect 是否存在。
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "DamageContext", meta = (CustomStructureParam = "OutValue", ExpandEnumAsExecs = "ExecResult"))
	static void GetEffect(UDamageContext* Context, EStructUtilsResult& ExecResult, int32& OutValue);

private:
	DECLARE_FUNCTION(execSetEffect);
	DECLARE_FUNCTION(execGetEffect);
};

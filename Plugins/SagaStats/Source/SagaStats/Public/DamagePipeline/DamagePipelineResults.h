/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamagePipelineResults.h — Game 侧读写 DamageContext 的唯一外部 API 入口
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintInstancedStructLibrary.h"
#include "StructUtils/InstancedStruct.h"
#include "DamagePipeline/DamageContext.h"
#include "DamagePipelineResults.generated.h"

/**
 * UDamagePipelineResults
 *
 * Game 侧与 DamageContext 交互的唯一 API 入口（语义上标记"我是 Game 消费者，
 * 不是 DSL 原语内部"）。
 *
 * 设计动机：
 * - UDamageContext 的 Effect 读写 API 全部 protected 以防止 Condition/Operation
 *   内部越界访问（保障 R5 契约）
 * - 但 Game 侧有合法需求：Execute 前预填攻击上下文、Execute 后读管线结果、调试遍历
 * - 本类作为 friend 授权的 Library，把"合法外部访问"与"非法内部绕过"在类型层面区分
 *
 * 使用场景：
 * - WriteEffect：Execute 前在 Context 里填入攻击上下文或其他外部输入
 * - ReadEffect：Execute 后读取 Rule 产出，驱动表现/游戏逻辑
 * - HasEffect：判断某类型产出是否存在
 * - GetAllEffects：调试输出（生产代码慎用）
 *
 * 蓝图路径：通过 CustomThunk 提供通配结构体引脚，语义自解释（"Read Pipeline Result"）。
 */
UCLASS()
class SAGASTATS_API UDamagePipelineResults : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// =====================================================================
	// C++ 模板 API（编译时类型安全）
	// =====================================================================

	/** Execute 前预填一个 Effect 到 Context（攻击上下文 / 游戏侧外部输入） */
	template<typename T>
	static void WriteEffect(UDamageContext* Context, const T& Value)
	{
		if (Context) Context->SetEffect<T>(Value);
	}

	/** Execute 后读取一个 Effect（驱动表现/游戏逻辑）；不存在返回 nullptr */
	template<typename T>
	static const T* ReadEffect(const UDamageContext* Context)
	{
		return Context ? Context->GetEffect<T>() : nullptr;
	}

	/** 判断某 Effect 类型是否在 Context 中存在 */
	template<typename T>
	static bool HasEffect(const UDamageContext* Context)
	{
		return Context ? Context->HasEffect<T>() : false;
	}

	/** 遍历所有 Effect（Game 侧调试用；生产代码应走 ReadEffect<T>） */
	static const TMap<TObjectPtr<UScriptStruct>, FInstancedStruct>& GetAllEffects(const UDamageContext* Context);

	// =====================================================================
	// 蓝图 API（CustomThunk 通配结构体引脚）
	// =====================================================================

	/**
	 * 蓝图：Execute 前预填 Effect（类型由连线的结构体引脚自动推断）。
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "DamagePipeline|Results",
		meta = (DisplayName = "Write Pipeline Input", CustomStructureParam = "Value"))
	static void WriteEffect(UDamageContext* Context, const int32& Value);

	/**
	 * 蓝图：Execute 后读取 Effect（类型由连线的结构体引脚自动推断）。
	 * Valid/NotValid 执行引脚指示 Effect 是否存在。
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "DamagePipeline|Results",
		meta = (DisplayName = "Read Pipeline Result", CustomStructureParam = "OutValue", ExpandEnumAsExecs = "ExecResult"))
	static void ReadEffect(const UDamageContext* Context, EStructUtilsResult& ExecResult, int32& OutValue);

private:
	DECLARE_FUNCTION(execWriteEffect);
	DECLARE_FUNCTION(execReadEffect);
};

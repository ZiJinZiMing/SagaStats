/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageContext.h — UDamageContext: 单次伤害事件的共享上下文
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamageContext.generated.h"

// Forward declarations for friend classes（访问分层，详见下方注释）
class UDamagePipeline;
class UDamagePipelineResults;
class UDamageCondition_Effect;
class UDamageOperationBase;

/**
 * UDamageContext
 * 单次伤害事件的共享上下文。
 *
 * 统一存储：TMap<UScriptStruct*, FInstancedStruct>
 * EffectType 作为万能连接件：DamageRule:Effect 1:1 → 类型唯一确定生产者。
 * 攻击上下文和 DamageRule 产出共用同一存储——都是 typed struct。
 *
 * ============================================================================
 * 访问分层（R5 设计契约的编译期强制）
 * ============================================================================
 *
 * Effect 的读写 API（SetEffect/GetEffect/HasEffect/SetEffectByType/GetEffectByType/...）
 * 全部为 protected。违反 R5（越界读其他 Effect）在编译期被拒绝。
 *
 * 合法访问路径：
 * - DSL 框架内部（UDamagePipeline::Build/Execute、UDamageCondition_Effect 预取）
 *   → 通过 friend class 授权
 * - Game 侧消费（Execute 前填充输入 / Execute 后读取产出 / 调试遍历）
 *   → 通过 UDamagePipelineResults 包装 API（C++ 模板 + 蓝图 CustomThunk）
 *
 * 违反契约的场景（编译期拒绝）：
 * - Condition/Operation 子类的 Evaluate/Execute 内部调 Context->GetEffect<T>() → protected 访问错误
 * - 蓝图 Condition/Operation 想绕过：没有通用读写蓝图节点（v4.7 删除了旧的 UDamageContextLibrary）
 *
 * Game 扩展字段（通过 Cast<UDamageContextSubclass>(Ctx)->GameField 访问）不受影响——
 * 子类自己加的 public 字段是 Game 项目的合法扩展面。
 *
 * Blueprintable：Game 侧可蓝图继承添加上下文便利字段。
 */
UCLASS(BlueprintType, Blueprintable)
class SAGASTATS_API UDamageContext : public UObject
{
	GENERATED_BODY()

	// ------------------------------------------------------------------------
	// Friend 列表 —— DSL 内部授权访问
	// ------------------------------------------------------------------------
	friend class UDamagePipeline;              // Build/Execute 内部读写
	friend class UDamagePipelineResults;       // Game 侧读写包装（唯一的外部 API 入口）
	friend class UDamageCondition_Effect;      // 预取自己声明的 EffectType 对应 Effect
	friend class UDamageOperationBase;         // Operation 通过基类 ReadEffect 读上游 Effect（R5 强制留下轮）

public:
	// =====================================================================
	// 公开 API（所有人可用）
	// =====================================================================

	UFUNCTION(BlueprintCallable, Category = "DamageContext")
	void Reset();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageContext")
	FString DumpToString() const;

protected:
	// =====================================================================
	// Effect 读写 API（protected —— 只对 friend 开放）
	// =====================================================================

	// ---- C++ 模板 API（编译时类型安全）----

	template<typename T>
	void SetEffect(const T& Value)
	{
		DamageEffects.Add(T::StaticStruct(), FInstancedStruct::Make<T>(Value));
	}

	template<typename T>
	const T* GetEffect() const
	{
		if (const FInstancedStruct* Found = DamageEffects.Find(T::StaticStruct()))
		{
			return Found->GetPtr<T>();
		}
		return nullptr;
	}

	template<typename T>
	bool HasEffect() const
	{
		return DamageEffects.Contains(T::StaticStruct());
	}

	// ---- C++ 运行时 API（UScriptStruct*，供框架使用）----

	void SetEffectByType(const FInstancedStruct& Value);
	FInstancedStruct GetEffectByType(UScriptStruct* EffectType) const;
	bool HasEffectByType(UScriptStruct* EffectType) const;

	const TMap<TObjectPtr<UScriptStruct>, FInstancedStruct>& GetAllDamageEffects() const { return DamageEffects; }

private:
	/** DamageEffect 存储（UScriptStruct* key —— 类型即 key） */
	UPROPERTY()
	TMap<TObjectPtr<UScriptStruct>, FInstancedStruct> DamageEffects;
};

// FactMethodRegistry.h — Fact 领域方法全局注册表
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "StructUtils/InstancedStruct.h"
#include "FactMethodRegistry.generated.h"

/**
 * Fact 领域方法的调用委托。
 * 输入：FInstancedStruct（Fact 实例）
 * 输出：bool
 */
DECLARE_DELEGATE_RetVal_OneParam(bool, FFactMethodDelegate, const FInstancedStruct&);

/**
 * UFactMethodRegistry — 全局 Fact 方法注册表。
 *
 * 映射 (UScriptStruct*, MethodName) → 查询函数。
 * 支持两条注册路径：
 *   - C++ 路径：Register<T>(MethodName, lambda)，编译时类型安全
 *   - 蓝图路径：RegisterBP(UScriptStruct*, MethodName, UObject*, FunctionName)，运行时反射
 *
 * 两条路径注册的方法在 Registry 中统一存储。
 * CallMethod 对调用者完全透明——不区分注册方式。
 */
UCLASS()
class SAGASTATS_API UFactMethodRegistry : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 获取全局单例 */
	static UFactMethodRegistry* Get();

	// =====================================================================
	// C++ 注册路径（模板 + lambda，编译时类型安全）
	// =====================================================================

	/**
	 * 注册一个类型安全的领域方法。
	 * @param MethodName 方法名（如 "IsParry"）
	 * @param Func 查询函数，输入为具体 Fact 类型的引用，返回 bool
	 */
	template<typename T>
	void Register(FName MethodName, TFunction<bool(const T&)> Func)
	{
		FFactMethodDelegate Delegate = FFactMethodDelegate::CreateLambda(
			[Func](const FInstancedStruct& S) -> bool
			{
				const T* Ptr = S.GetPtr<T>();
				return Ptr ? Func(*Ptr) : false;
			});
		RegisterMethod(T::StaticStruct(), MethodName, MoveTemp(Delegate));
	}

	// =====================================================================
	// 通用注册路径
	// =====================================================================

	/** 注册一个方法（底层接口，C++ 模板和蓝图路径最终都调用此方法） */
	void RegisterMethod(UScriptStruct* Struct, FName MethodName, FFactMethodDelegate Delegate);

	// =====================================================================
	// 蓝图注册路径
	// =====================================================================

	/**
	 * 蓝图注册路径：绑定一个 UFunction 作为领域方法。
	 * UFunction 签名要求：输入 FInstancedStruct，输出 bool。
	 */
	UFUNCTION(BlueprintCallable, Category = "FactMethodRegistry")
	void RegisterBP(UScriptStruct* Struct, FName MethodName, UObject* Target, FName FunctionName);

	// =====================================================================
	// 调用
	// =====================================================================

	/**
	 * 调用已注册的领域方法。
	 * @param Fact Fact 实例（FInstancedStruct）
	 * @param MethodName 方法名
	 * @return 方法返回值。Fact 无效或方法未注册时返回 false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FactMethodRegistry")
	bool CallMethod(const FInstancedStruct& Fact, FName MethodName) const;

	// =====================================================================
	// 内省（属性面板联动下拉用）
	// =====================================================================

	/** 返回指定 Fact 类型已注册的所有方法名列表 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FactMethodRegistry")
	TArray<FName> GetMethodsForStruct(UScriptStruct* Struct) const;

	/** 返回所有已注册的 Fact 类型 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FactMethodRegistry")
	TArray<UScriptStruct*> GetRegisteredStructs() const;

	// =====================================================================
	// Fact Key → Type 映射（编辑器联动下拉用）
	// =====================================================================

	/**
	 * 注册 Fact Key → UScriptStruct* 映射。
	 * Register<T>() 调用时自动建立（通过方法注册顺带）。
	 * 信号 Fact（无领域方法）需要单独调用此方法注册。
	 */
	void RegisterFactType(FName FactKey, UScriptStruct* Struct);

	/** C++ 模板便捷方法 */
	template<typename T>
	void RegisterFactType(FName FactKey)
	{
		RegisterFactType(FactKey, T::StaticStruct());
	}

	/** 通过 Fact Key 查找 UScriptStruct*，未注册返回 nullptr */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FactMethodRegistry")
	UScriptStruct* GetStructForFactKey(FName FactKey) const;

	/** 返回所有已注册的 Fact Key 列表 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FactMethodRegistry")
	TArray<FName> GetRegisteredFactKeys() const;

	/**
	 * 通过 Fact Key 获取该 Fact 类型已注册的所有方法名。
	 * 联动下拉的核心 API：选了 FactKey → 查到 Struct → 查到方法列表。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FactMethodRegistry")
	TArray<FName> GetMethodsForFactKey(FName FactKey) const;

private:
	/** 注册表的 Key：(Struct 类型, 方法名) */
	struct FMethodKey
	{
		UScriptStruct* Struct = nullptr;
		FName MethodName;

		bool operator==(const FMethodKey& Other) const
		{
			return Struct == Other.Struct && MethodName == Other.MethodName;
		}

		friend uint32 GetTypeHash(const FMethodKey& Key)
		{
			return HashCombine(GetTypeHash(Key.Struct), GetTypeHash(Key.MethodName));
		}
	};

	TMap<FMethodKey, FFactMethodDelegate> Methods;

	/** Fact Key → UScriptStruct* 映射 */
	TMap<FName, UScriptStruct*> FactKeyToStruct;
};

// FactMethodRegistry.cpp — Fact 领域方法全局注册表实现
#include "DPUFramework/FactMethodRegistry.h"
#include "DPUFramework/SekiroFacts.h"
#include "SagaStatsLog.h"

void UFactMethodRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 注册只狼 MVP 的 Fact 领域方法
	RegisterSekiroFactMethods(this);

	UE_LOG(LogSagaStats, Log, TEXT("FactMethodRegistry 已初始化"));
}

void UFactMethodRegistry::Deinitialize()
{
	Methods.Empty();
	Super::Deinitialize();
}

UFactMethodRegistry* UFactMethodRegistry::Get()
{
	return GEngine ? GEngine->GetEngineSubsystem<UFactMethodRegistry>() : nullptr;
}

void UFactMethodRegistry::RegisterMethod(UScriptStruct* Struct, FName MethodName, FFactMethodDelegate Delegate)
{
	if (!Struct)
	{
		UE_LOG(LogSagaStats, Warning, TEXT("RegisterMethod: Struct 为空，跳过注册 %s"), *MethodName.ToString());
		return;
	}

	FMethodKey Key{Struct, MethodName};
	if (Methods.Contains(Key))
	{
		UE_LOG(LogSagaStats, Warning, TEXT("RegisterMethod: (%s, %s) 已注册，将覆盖"),
			*Struct->GetName(), *MethodName.ToString());
	}

	Methods.Add(Key, MoveTemp(Delegate));
	UE_LOG(LogSagaStats, Verbose, TEXT("RegisterMethod: (%s, %s) 注册成功"),
		*Struct->GetName(), *MethodName.ToString());
}

void UFactMethodRegistry::RegisterBP(UScriptStruct* Struct, FName MethodName, UObject* Target, FName FunctionName)
{
	if (!Struct || !Target)
	{
		UE_LOG(LogSagaStats, Warning, TEXT("RegisterBP: 参数无效，跳过注册"));
		return;
	}

	UFunction* Func = Target->FindFunction(FunctionName);
	if (!Func)
	{
		UE_LOG(LogSagaStats, Warning, TEXT("RegisterBP: 在 %s 上未找到函数 %s"),
			*Target->GetName(), *FunctionName.ToString());
		return;
	}

	// 捕获 Target 和 FunctionName，通过 ProcessEvent 调用蓝图函数
	TWeakObjectPtr<UObject> WeakTarget = Target;
	FName CapturedFuncName = FunctionName;

	FFactMethodDelegate Delegate = FFactMethodDelegate::CreateLambda(
		[WeakTarget, CapturedFuncName](const FInstancedStruct& Fact) -> bool
		{
			UObject* Obj = WeakTarget.Get();
			if (!Obj) return false;

			UFunction* Fn = Obj->FindFunction(CapturedFuncName);
			if (!Fn) return false;

			// 蓝图函数签名：(FInstancedStruct) -> bool
			struct FParams
			{
				FInstancedStruct Input;
				bool ReturnValue = false;
			};
			FParams Params;
			Params.Input = Fact;
			Obj->ProcessEvent(Fn, &Params);
			return Params.ReturnValue;
		});

	RegisterMethod(Struct, MethodName, MoveTemp(Delegate));
}

bool UFactMethodRegistry::CallMethod(const FInstancedStruct& Fact, FName MethodName) const
{
	if (!Fact.IsValid())
	{
		return false;
	}

	const UScriptStruct* Struct = Fact.GetScriptStruct();
	FMethodKey Key{const_cast<UScriptStruct*>(Struct), MethodName};

	if (const FFactMethodDelegate* Delegate = Methods.Find(Key))
	{
		return Delegate->Execute(Fact);
	}

	// 方法未注册——可能是信号 Fact（无领域方法），MethodName 为空时返回 true（存在即 true）
	if (MethodName.IsNone() || MethodName == NAME_None)
	{
		return true; // 信号 Fact：存在即 true
	}

	UE_LOG(LogSagaStats, Warning, TEXT("CallMethod: (%s, %s) 未注册"),
		*Struct->GetName(), *MethodName.ToString());
	return false;
}

TArray<FName> UFactMethodRegistry::GetMethodsForStruct(UScriptStruct* Struct) const
{
	TArray<FName> Result;
	if (!Struct) return Result;

	for (const auto& Pair : Methods)
	{
		if (Pair.Key.Struct == Struct)
		{
			Result.Add(Pair.Key.MethodName);
		}
	}
	return Result;
}

TArray<UScriptStruct*> UFactMethodRegistry::GetRegisteredStructs() const
{
	TArray<UScriptStruct*> Result;
	for (const auto& Pair : Methods)
	{
		Result.AddUnique(Pair.Key.Struct);
	}
	return Result;
}

// ============================================================================
// Fact Key → Type 映射
// ============================================================================

void UFactMethodRegistry::RegisterFactType(FName FactKey, UScriptStruct* Struct)
{
	if (FactKey.IsNone() || !Struct) return;

	FactKeyToStruct.Add(FactKey, Struct);
}

UScriptStruct* UFactMethodRegistry::GetStructForFactKey(FName FactKey) const
{
	if (UScriptStruct* const* Found = FactKeyToStruct.Find(FactKey))
	{
		return *Found;
	}
	return nullptr;
}

TArray<FName> UFactMethodRegistry::GetRegisteredFactKeys() const
{
	TArray<FName> Result;
	FactKeyToStruct.GetKeys(Result);
	return Result;
}

TArray<FName> UFactMethodRegistry::GetMethodsForFactKey(FName FactKey) const
{
	UScriptStruct* Struct = GetStructForFactKey(FactKey);
	if (!Struct) return {};
	return GetMethodsForStruct(Struct);
}

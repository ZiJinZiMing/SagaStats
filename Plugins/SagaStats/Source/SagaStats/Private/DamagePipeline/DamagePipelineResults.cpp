/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamagePipelineResults.cpp — Game 侧读写 DamageContext 的唯一外部 API 入口实现
#include "DamagePipeline/DamagePipelineResults.h"
#include "Blueprint/BlueprintExceptionInfo.h"

#define LOCTEXT_NAMESPACE "UDamagePipelineResults"

// ============================================================================
// C++ 模板的非模板补充
// ============================================================================

const TMap<TObjectPtr<UScriptStruct>, FInstancedStruct>& UDamagePipelineResults::GetAllEffects(const UDamageContext* Context)
{
	static const TMap<TObjectPtr<UScriptStruct>, FInstancedStruct> Empty;
	return Context ? Context->GetAllDamageEffects() : Empty;
}

// ============================================================================
// 蓝图桩函数（CustomThunk 走 exec 版本，这些永远不会被调用）
// ============================================================================

void UDamagePipelineResults::WriteEffect(UDamageContext* Context, const int32& Value)
{
	checkNoEntry();
}

void UDamagePipelineResults::ReadEffect(const UDamageContext* Context, EStructUtilsResult& ExecResult, int32& OutValue)
{
	checkNoEntry();
}

// ============================================================================
// CustomThunk: WriteEffect（Execute 前预填攻击上下文或外部输入）
// ============================================================================

DEFINE_FUNCTION(UDamagePipelineResults::execWriteEffect)
{
	P_GET_OBJECT(UDamageContext, ContextObj);
	UDamageContext* DC = Cast<UDamageContext>(ContextObj);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const FStructProperty* ValueProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	const void* ValuePtr = Stack.MostRecentPropertyAddress;

	P_FINISH;

	if (!DC)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AccessViolation,
			LOCTEXT("WriteEffect_NullContext", "WriteEffect called on null DamageContext")
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
	else if (!ValueProp || !ValuePtr)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AbortExecution,
			LOCTEXT("WriteEffect_InvalidValue", "Failed to resolve the input struct type for WriteEffect")
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
	else
	{
		P_NATIVE_BEGIN;
		FInstancedStruct NewEffect;
		NewEffect.InitializeAs(ValueProp->Struct, (const uint8*)ValuePtr);
		DC->SetEffectByType(NewEffect);
		P_NATIVE_END;
	}
}

// ============================================================================
// CustomThunk: ReadEffect（Execute 后读取 Rule 产出）
// ============================================================================

DEFINE_FUNCTION(UDamagePipelineResults::execReadEffect)
{
	P_GET_OBJECT(UDamageContext, ContextObj);
	const UDamageContext* DC = Cast<const UDamageContext>(ContextObj);

	P_GET_ENUM_REF(EStructUtilsResult, ExecResult);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const FStructProperty* ValueProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	void* ValuePtr = Stack.MostRecentPropertyAddress;

	P_FINISH;

	ExecResult = EStructUtilsResult::NotValid;

	if (!DC)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AccessViolation,
			LOCTEXT("ReadEffect_NullContext", "ReadEffect called on null DamageContext")
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
	else if (!ValueProp || !ValuePtr)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AbortExecution,
			LOCTEXT("ReadEffect_InvalidValue", "Failed to resolve the output struct type for ReadEffect")
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
	else
	{
		P_NATIVE_BEGIN;
		FInstancedStruct Found = DC->GetEffectByType(ValueProp->Struct);
		if (Found.IsValid() && Found.GetScriptStruct()->IsChildOf(ValueProp->Struct))
		{
			ValueProp->Struct->CopyScriptStruct(ValuePtr, Found.GetMemory());
			ExecResult = EStructUtilsResult::Valid;
		}
		P_NATIVE_END;
	}
}

#undef LOCTEXT_NAMESPACE

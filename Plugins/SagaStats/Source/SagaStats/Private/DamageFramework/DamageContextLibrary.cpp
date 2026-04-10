// DamageContextLibrary.cpp — DamageContext 蓝图 CustomThunk 实现
#include "DamageFramework/DamageContextLibrary.h"
#include "DamageFramework/DamageContext.h"
#include "Blueprint/BlueprintExceptionInfo.h"

#define LOCTEXT_NAMESPACE "UDamageContextLibrary"

// ============================================================================
// 蓝图桩函数（CustomThunk 走 exec 版本，这些永远不会被调用）
// ============================================================================

void UDamageContextLibrary::SetEffect(UDamageContext* Context, const int32& Value)
{
	checkNoEntry();
}

void UDamageContextLibrary::GetEffect(UDamageContext* Context, EStructUtilsResult& ExecResult, int32& OutValue)
{
	checkNoEntry();
}

// ============================================================================
// CustomThunk: SetEffect
// ============================================================================

DEFINE_FUNCTION(UDamageContextLibrary::execSetEffect)
{
	// 参数1: UDamageContext*
	P_GET_OBJECT(UDamageContext, ContextObj);
	UDamageContext* DC = Cast<UDamageContext>(ContextObj);

	// 参数2: 通配结构体
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
			LOCTEXT("SetEffect_NullContext", "SetEffect called on null DamageContext")
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
	else if (!ValueProp || !ValuePtr)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AbortExecution,
			LOCTEXT("SetEffect_InvalidValue", "Failed to resolve the input struct type for SetEffect")
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
// CustomThunk: GetEffect
// ============================================================================

DEFINE_FUNCTION(UDamageContextLibrary::execGetEffect)
{
	// 参数1: UDamageContext*
	P_GET_OBJECT(UDamageContext, ContextObj);
	UDamageContext* DC = Cast<UDamageContext>(ContextObj);

	// 参数2: ExecResult
	P_GET_ENUM_REF(EStructUtilsResult, ExecResult);

	// 参数3: 通配结构体
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
			LOCTEXT("GetEffect_NullContext", "GetEffect called on null DamageContext")
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
	else if (!ValueProp || !ValuePtr)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AbortExecution,
			LOCTEXT("GetEffect_InvalidValue", "Failed to resolve the output struct type for GetEffect")
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

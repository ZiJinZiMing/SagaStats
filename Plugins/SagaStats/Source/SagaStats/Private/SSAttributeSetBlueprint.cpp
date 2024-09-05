// Copyright 2022-2024 Mickael Daniel. All Rights Reserved.

#pragma once

#include "SSAttributeSetBlueprint.h"

#include "SSDelegates.h"
#include "SSLog.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/UObjectGlobals.h"

#if WITH_EDITOR
#if UE_VERSION_NEWER_THAN(5, 1, -1)
#include "RigVMDeveloperTypeUtils.h"
#else
#include "RigVMTypeUtils.h"
#endif
#endif

USSAttributeSetBlueprint::~USSAttributeSetBlueprint()
{
	SS_LOG(VeryVerbose, TEXT("USSAttributeSetBlueprint::~USSAttributeSetBlueprint - Destructor"))
#if WITH_EDITOR
	FCoreUObjectDelegates::OnObjectModified.RemoveAll(this);
	OnChanged().RemoveAll(this);
	OnCompiled().RemoveAll(this);
#endif
}

void USSAttributeSetBlueprint::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITOR
	RegisterDelegates();
#endif
}

#if WITH_EDITOR

void USSAttributeSetBlueprint::RegisterDelegates()
{
	if (IsTemplate())
	{
		return;
	}
	
	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::RegisterDelegates - Setup delegates for %s"), *GetName())

	FCoreUObjectDelegates::OnObjectModified.RemoveAll(this);
	OnChanged().RemoveAll(this);
	OnCompiled().RemoveAll(this);

	FCoreUObjectDelegates::OnObjectModified.AddUObject(this, &USSAttributeSetBlueprint::OnPreVariableChange);
	OnChanged().AddUObject(this, &USSAttributeSetBlueprint::OnPostVariableChange);
	OnCompiled().AddUObject(this, &USSAttributeSetBlueprint::OnPostCompiled);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void USSAttributeSetBlueprint::OnPreVariableChange(UObject* InObject)
{
	if (InObject != this)
	{
		return;
	}

	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::OnPreVariableChange %s"), *GetNameSafe(InObject))

	LastNewVariables = NewVariables;
}

void USSAttributeSetBlueprint::HandleVariableChanges(const UBlueprint* InBlueprint)
{
	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::OnPostVariableChange %s"), *GetNameSafe(InBlueprint))

	TMap<FGuid, int32> NewVariablesByGuid;
	for (int32 VarIndex = 0; VarIndex < NewVariables.Num(); VarIndex++)
	{
		NewVariablesByGuid.Add(NewVariables[VarIndex].VarGuid, VarIndex);
	}

	TMap<FGuid, int32> OldVariablesByGuid;
	for (int32 VarIndex = 0; VarIndex < LastNewVariables.Num(); VarIndex++)
	{
		OldVariablesByGuid.Add(LastNewVariables[VarIndex].VarGuid, VarIndex);
	}

	for (const FBPVariableDescription& OldVariable : LastNewVariables)
	{
		if (!NewVariablesByGuid.Contains(OldVariable.VarGuid))
		{
			OnVariableRemoved(OldVariable.VarName);
		}
	}

	for (const FBPVariableDescription& NewVariable : NewVariables)
	{
		if (!OldVariablesByGuid.Contains(NewVariable.VarGuid))
		{
			OnVariableAdded(NewVariable.VarName);
			continue;
		}

		const int32 OldVarIndex = OldVariablesByGuid.FindChecked(NewVariable.VarGuid);
		const FBPVariableDescription& OldVariable = LastNewVariables[OldVarIndex];
		if (OldVariable.VarName != NewVariable.VarName)
		{
			OnVariableRenamed(OldVariable.VarName, NewVariable.VarName);
		}

		if (OldVariable.VarType != NewVariable.VarType)
		{
			OnVariableTypeChanged(NewVariable.VarName, OldVariable.VarType, NewVariable.VarType);
		}
	}

	LastNewVariables = NewVariables;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void USSAttributeSetBlueprint::OnPostVariableChange(UBlueprint* InBlueprint) const
{
	if (InBlueprint != this)
	{
		return;
	}

	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::OnPostVariableChange - %s"), *GetNameSafe(InBlueprint))
	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::OnPostVariableChange - IsPossiblyDirty: %s"), IsPossiblyDirty() ? TEXT("true") : TEXT("false"))
	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::OnPostVariableChange - IsUpToDate: %s"), IsUpToDate() ? TEXT("true") : TEXT("false"))
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void USSAttributeSetBlueprint::OnPostCompiled(UBlueprint* InBlueprint)
{
	if (InBlueprint != this)
	{
		return;
	}


	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::OnPostCompiled - %s"), *GetNameSafe(InBlueprint))
	HandleVariableChanges(InBlueprint);

	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::OnPostCompiled - IsPossiblyDirty: %s"), IsPossiblyDirty() ? TEXT("true") : TEXT("false"))
	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::OnPostCompiled - IsUpToDate: %s"), IsUpToDate() ? TEXT("true") : TEXT("false"))

	if (const UPackage* Package = GetPackage())
	{
		FSSDelegates::OnPostCompile.Broadcast(Package->GetFName());
	}
}

void USSAttributeSetBlueprint::OnVariableAdded(const FName& Name)
{
	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::OnVariableAdded Name: %s"), *Name.ToString())
	FSSDelegates::OnVariableAdded.Broadcast(Name);
}

void USSAttributeSetBlueprint::OnVariableRemoved(const FName& Name)
{
	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::OnVariableRemoved Name: %s"), *Name.ToString())
	FSSDelegates::OnVariableRemoved.Broadcast(Name);
}

void USSAttributeSetBlueprint::OnVariableRenamed(const FName& InOldVarName, const FName& InNewVarName) const
{
	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::OnVariableRenamed InOldVarName: %s, InNewVarName: %s"), *InOldVarName.ToString(), *InNewVarName.ToString())

	const UPackage* Package = GetPackage();
	if (!Package)
	{
		SS_LOG(Warning, TEXT("USSAttributeSetBlueprint::OnVariableRenamed SelfPackage nullptr"))
		return;
	}

	// Broadcast renamed event, the subsystem will handle the rest
	FSSDelegates::OnVariableRenamed.Broadcast(Package->GetFName(), InOldVarName, InNewVarName);
}

void USSAttributeSetBlueprint::OnVariableTypeChanged(const FName& InVarName, const FEdGraphPinType& InOldPinType, const FEdGraphPinType& InNewPinType) const
{
	const FString OldVarTypeStr = FString::Printf(TEXT("Category: %s, SubCategory: %s"), *InOldPinType.PinCategory.ToString(), *InOldPinType.PinSubCategory.ToString());
	const FString NewVarTypeStr = FString::Printf(TEXT("Category: %s, SubCategory: %s"), *InNewPinType.PinCategory.ToString(), *InNewPinType.PinSubCategory.ToString());

	FString CPPType;
	UObject* CPPTypeObject = nullptr;
	RigVMTypeUtils::CPPTypeFromPinType(InNewPinType, CPPType, &CPPTypeObject);

	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::OnVariableTypeChanged InVarName: %s || InOldPinType: %s || InNewPinType: %s"), *InVarName.ToString(), *OldVarTypeStr, *NewVarTypeStr)
	SS_LOG(Verbose, TEXT("USSAttributeSetBlueprint::OnVariableTypeChanged ==> CPPType: %s || CPPTypeObject: %s"), *CPPType, *GetNameSafe(CPPTypeObject))

	FSSDelegates::OnVariableTypeChanged.Broadcast(InVarName, CPPType, CPPTypeObject);
}

#endif
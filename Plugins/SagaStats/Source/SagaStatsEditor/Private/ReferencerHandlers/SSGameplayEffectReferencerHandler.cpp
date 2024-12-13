﻿/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "SSGameplayEffectReferencerHandler.h"

#include "SSEditorLog.h"
#include "GameplayEffect.h"
#include "Engine/Blueprint.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/UObjectToken.h"
#include "SagaEditorSubsystem.h"

#if UE_VERSION_NEWER_THAN(5, 3, -1)
#include "GameplayEffectComponents/ImmunityGameplayEffectComponent.h"
#include "GameplayEffectComponents/RemoveOtherGameplayEffectComponent.h"
#endif

#define LOCTEXT_NAMESPACE "SSGameplayEffectReferencerHandler"

TSharedPtr<ISSAttributeReferencerHandler> FSSGameplayEffectReferencerHandler::Create()
{
	return MakeShared<FSSGameplayEffectReferencerHandler>();
}

FSSGameplayEffectReferencerHandler::~FSSGameplayEffectReferencerHandler()
{
	AttributesCacheMap.Reset();
}

void FSSGameplayEffectReferencerHandler::OnPreCompile(const FString& InPackageName)
{
	AttributesCacheMap.Reset();
}

void FSSGameplayEffectReferencerHandler::OnPostCompile(const FString& InPackageName)
{
}

bool FSSGameplayEffectReferencerHandler::HandlePreCompile(const FAssetIdentifier& InAssetIdentifier, const FSSAttributeReferencerPayload& InPayload)
{
	SS_EDITOR_NS_LOG(Verbose, TEXT("InAssetIdentifier: %s, InPayload: %s"), *InAssetIdentifier.ToString(), *InPayload.ToString())
	
	UGameplayEffect* EffectCDO = Cast<UGameplayEffect>(InPayload.DefaultObject);
	if (!EffectCDO)
	{
		return false;
	}
	
	TArray<FAttributeReference> AttributesCache;
	AttributesCache.Reserve(EffectCDO->Modifiers.Num());

	// Duration
	FAttributeReference DurationAttributeReference;
	if (BuildModifierMagnitudeAttributeReference(EffectCDO->DurationMagnitude, DurationAttributeReference))
	{
		DurationAttributeReference.Index = 0;
		DurationAttributeReference.Type = EAttributeReferenceType::DurationMagnitude;
		AttributesCache.Add(DurationAttributeReference);
	}

	// Modifiers
	{
		int32 CurrentIndex = 0;
		for (const FGameplayModifierInfo& Modifier : EffectCDO->Modifiers)
		{
			CurrentIndex++;

			// Modifier attribute
			{
				FAttributeReference AttributeReference;
				if (BuildModifierInfoAttributeReference(Modifier, AttributeReference))
				{
					AttributeReference.Index = CurrentIndex - 1;
					AttributeReference.Type = EAttributeReferenceType::ModifierInfo_Attribute;
					AttributesCache.Add(AttributeReference);
				}
			}

			// Modifier attribute based magnitude
			{
				FAttributeReference AttributeReference;
				if (BuildModifierMagnitudeAttributeReference(Modifier.ModifierMagnitude, AttributeReference))
				{
					AttributeReference.Index = CurrentIndex - 1;
					AttributeReference.Type = EAttributeReferenceType::ModifierInfo_BackingAttribute_AttributeToCapture;
					AttributesCache.Add(AttributeReference);
				}
			}
		}
	}
	
	// Gameplay Cues > Magnitude Attribute
	{
		int32 CurrentIndex = 0;
		for (const FGameplayEffectCue& GameplayCue : EffectCDO->GameplayCues)
		{
			CurrentIndex++;

			FAttributeReference AttributeReference;
			if (BuildEffectCueMagnitudeAttributeReference(GameplayCue, AttributeReference))
			{
				AttributeReference.Index = CurrentIndex - 1;
				AttributeReference.Type = EAttributeReferenceType::GameplayCue_MagnitudeAttribute;
				AttributesCache.Add(AttributeReference);
			}
		}
	}

	// Tags > RemoveGameplayEffectQuery
	{

		// TODO: Check everything working fine in 5.3 and GEComponents
		
#if UE_VERSION_NEWER_THAN(5, 3, -1)
		if (const URemoveOtherGameplayEffectComponent* GameplayEffectComponent = EffectCDO->FindComponent<URemoveOtherGameplayEffectComponent>())
		{
			int32 CurrentIndex = 0;
			for (const FGameplayEffectQuery& Query : GameplayEffectComponent->RemoveGameplayEffectQueries)
			{
				CurrentIndex++;
				
				FAttributeReference AttributeReference;
				if (BuildEffectQueryAttributeReference(Query, AttributeReference))
				{
					AttributeReference.Index = CurrentIndex - 1;
					AttributeReference.Type = EAttributeReferenceType::RemoveGameplayEffectQuery_ModifyingAttribute;
					AttributesCache.Add(AttributeReference);
				}
			}
		}
#else
		FAttributeReference AttributeReference;
		if (BuildEffectQueryAttributeReference(EffectCDO->RemoveGameplayEffectQuery, AttributeReference))
		{
			AttributeReference.Index = 0;
			AttributeReference.Type = EAttributeReferenceType::RemoveGameplayEffectQuery_ModifyingAttribute;
			AttributesCache.Add(AttributeReference);
		}
#endif
	}

	// Immunity > GrantedApplicationImmunityQuery
	{
		// TODO: Check everything working fine in 5.3 and GEComponents
		
#if UE_VERSION_NEWER_THAN(5, 3, -1)
		if (const UImmunityGameplayEffectComponent* GameplayEffectComponent = EffectCDO->FindComponent<UImmunityGameplayEffectComponent>())
		{
			int32 CurrentIndex = 0;
			for (const FGameplayEffectQuery& Query : GameplayEffectComponent->ImmunityQueries)
			{
				CurrentIndex++;
				
				FAttributeReference AttributeReference;
				if (BuildEffectQueryAttributeReference(Query, AttributeReference))
				{
					AttributeReference.Index = CurrentIndex - 1;
					AttributeReference.Type = EAttributeReferenceType::GrantedApplicationImmunityQuery_ModifyingAttribute;
					AttributesCache.Add(AttributeReference);
				}
			}
		}
#else
		FAttributeReference AttributeReference;
		if (BuildEffectQueryAttributeReference(EffectCDO->GrantedApplicationImmunityQuery, AttributeReference))
		{
			AttributeReference.Index = 0;
			AttributeReference.Type = EAttributeReferenceType::GrantedApplicationImmunityQuery_ModifyingAttribute;
			AttributesCache.Add(AttributeReference);
		}
#endif
	}

	AttributesCacheMap.Add(InAssetIdentifier, AttributesCache);
	return true;
}

bool FSSGameplayEffectReferencerHandler::HandleAttributeRename(const FAssetIdentifier& InAssetIdentifier, const FSSAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SS_EDITOR_NS_LOG(Verbose, TEXT("InAssetIdentifier: %s, InPayload: %s"), *InAssetIdentifier.ToString(), *InPayload.ToString())

	UGameplayEffect* EffectCDO = Cast<UGameplayEffect>(InPayload.DefaultObject);
	if (!EffectCDO)
	{
		return false;
	}

	const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *InPayload.PackageName);
	if (!Blueprint)
	{
		SS_EDITOR_NS_LOG(Warning, TEXT("Failed to update modifiers because of invalid Blueprint for %s"), *InPayload.PackageName)
		return false;
	}

	bool bModified = UpdateModifiers(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateModifiersBackingAttribute(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateDuration(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateCues(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateRemoveGameplayEffectQuery(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateImmunityEffectQuery(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	
	return bModified;
}

bool FSSGameplayEffectReferencerHandler::BuildModifierInfoAttributeReference(const FGameplayModifierInfo& InModifier, FAttributeReference& OutAttributeReference)
{
	if (!InModifier.Attribute.IsValid())
	{
		return false;
	}

	FString FinalValue;
	FGameplayAttribute::StaticStruct()->ExportText(FinalValue, &InModifier.Attribute, &InModifier.Attribute, nullptr, PPF_SerializedAsImportText, nullptr);
	if (FinalValue.IsEmpty())
	{
		return false;
	}

	FAttributeReference Reference;
	if (!USagaEditorSubsystem::ParseAttributeFromDefaultValue(FinalValue, Reference.PackageNameOwner, Reference.AttributeName))
	{
		return false;
	}
	//Feature Begin Attribute In subclass of AttributeSet
	Reference.AttributeOwnerClass = InModifier.Attribute.GetAttributeSetClass();
	//Feature End
	
	OutAttributeReference = MoveTemp(Reference);
	return true;
}

bool FSSGameplayEffectReferencerHandler::BuildModifierMagnitudeAttributeReference(const FGameplayEffectModifierMagnitude& InMagnitude, FAttributeReference& OutAttributeReference)
{
	const EGameplayEffectMagnitudeCalculation CalculationType = InMagnitude.GetMagnitudeCalculationType();
	
	TArray<FGameplayEffectAttributeCaptureDefinition> Definitions;
	InMagnitude.GetAttributeCaptureDefinitions(Definitions);

	TArray<FAttributeReference> AttributeReferences;
	AttributeReferences.Reserve(Definitions.Num());
	
	if (CalculationType == EGameplayEffectMagnitudeCalculation::AttributeBased && Definitions.Num() == 1 && Definitions.IsValidIndex(0))
	{
		const FGameplayEffectAttributeCaptureDefinition CaptureDefinition = Definitions[0];

		const FGameplayAttribute AttributeToCapture = CaptureDefinition.AttributeToCapture;

		FString FinalValue;
		FGameplayAttribute::StaticStruct()->ExportText(FinalValue, &AttributeToCapture, &AttributeToCapture, nullptr, PPF_SerializedAsImportText, nullptr);
		if (FinalValue.IsEmpty())
		{
			return false;
		}

		FAttributeReference Reference;
		if (!USagaEditorSubsystem::ParseAttributeFromDefaultValue(FinalValue, Reference.PackageNameOwner, Reference.AttributeName))
		{
			return false;
		}
		//Feature Begin Attribute In subclass of AttributeSet
		Reference.AttributeOwnerClass = AttributeToCapture.GetAttributeSetClass();
		//Feature End

		OutAttributeReference = MoveTemp(Reference);
		return true;
	}
	
	return false;
}

bool FSSGameplayEffectReferencerHandler::BuildEffectCueMagnitudeAttributeReference(const FGameplayEffectCue& InEffectCue, FAttributeReference& OutAttributeReference)
{
	FString FinalValue;
	FGameplayAttribute::StaticStruct()->ExportText(FinalValue, &InEffectCue.MagnitudeAttribute, &InEffectCue.MagnitudeAttribute, nullptr, PPF_SerializedAsImportText, nullptr);
	if (FinalValue.IsEmpty())
	{
		return false;
	}

	FAttributeReference AttributeReference;
	if (!USagaEditorSubsystem::ParseAttributeFromDefaultValue(FinalValue, AttributeReference.PackageNameOwner, AttributeReference.AttributeName))
	{
		return false;
	}
	//Feature Begin Attribute In subclass of AttributeSet
	AttributeReference.AttributeOwnerClass = InEffectCue.MagnitudeAttribute.GetAttributeSetClass();
	//Feature End
	
	OutAttributeReference = MoveTemp(AttributeReference);
	return true;
}

bool FSSGameplayEffectReferencerHandler::BuildEffectQueryAttributeReference(const FGameplayEffectQuery& InEffectQuery, FAttributeReference& OutAttributeReference)
{
	FString FinalValue;
	FGameplayAttribute::StaticStruct()->ExportText(FinalValue, &InEffectQuery.ModifyingAttribute, &InEffectQuery.ModifyingAttribute, nullptr, PPF_SerializedAsImportText, nullptr);
	if (FinalValue.IsEmpty())
	{
		return false;
	}

	FAttributeReference AttributeReference;
	if (!USagaEditorSubsystem::ParseAttributeFromDefaultValue(FinalValue, AttributeReference.PackageNameOwner, AttributeReference.AttributeName))
	{
		return false;
	}
	//Feature Begin Attribute In subclass of AttributeSet
	AttributeReference.AttributeOwnerClass = InEffectQuery.ModifyingAttribute.GetAttributeSetClass();
	//Feature End
	
	OutAttributeReference = MoveTemp(AttributeReference);
	return true;
}

bool FSSGameplayEffectReferencerHandler::UpdateModifiers(UGameplayEffect* InEffectCDO, const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, const FSSAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SS_EDITOR_NS_LOG(Display, TEXT("InEffectCDO: %s, InBlueprint: %s, InOldPropertyName: %s, InNewPropertyName: %s"),
		*GetNameSafe(InEffectCDO),
		*GetNameSafe(InBlueprint),
		*InPayload.OldPropertyName,
		*InPayload.NewPropertyName
	)

	check(InEffectCDO);
	check(InBlueprint);

	SS_EDITOR_LOG(VeryVerbose, TEXT("USagaEditorSubsystem::UpdateGameplayEffectModifiers Blueprint: %s"), *GetNameSafe(InBlueprint))
	int32 CurrentIndex = 0;
	TArray<FAttributeModifierToReplace> ModifiersToReplace;
	for (const FGameplayModifierInfo& Modifier : InEffectCDO->Modifiers)
	{
		FAttributeReference CachedModifier;
		const bool bHasCachedAttribute = GetCachedAttributeByPredicate(InAssetIdentifier, CachedModifier, [CurrentIndex](const FAttributeReference& Item)
		{
			return Item.Type == EAttributeReferenceType::ModifierInfo_Attribute && Item.Index == CurrentIndex;
		});

		if (!bHasCachedAttribute)
		{
			CurrentIndex++;
			continue;
		}
		
		if (!Modifier.Attribute.IsValid() && CachedModifier.AttributeName == InPayload.OldPropertyName)
		{
			if (!InBlueprint->GeneratedClass)
			{
				CurrentIndex++;
				continue;
			}

			if (FProperty* Prop = FindFProperty<FProperty>(InBlueprint->GeneratedClass, FName(*InPayload.NewPropertyName)))
			{
				//Feature Begin Attribute In subclass of AttributeSet
				ModifiersToReplace.Add(FAttributeModifierToReplace(CurrentIndex, Prop,CachedModifier.AttributeOwnerClass));
				//Feature End
			}
		}

		CurrentIndex++;
	}

	// Now that we have the list of modifiers that needs an update, update modifiers
	SS_EDITOR_LOG(Verbose, TEXT("USagaEditorSubsystem::UpdateGameplayEffectModifiers Update CDO modifiers now from gathered props to replace: %d"), ModifiersToReplace.Num())
	for (const FAttributeModifierToReplace& ModifierToReplace : ModifiersToReplace)
	{
		const int32 Index = ModifierToReplace.Index;

		if (!InEffectCDO->Modifiers.IsValidIndex(Index))
		{
			SS_EDITOR_LOG(Verbose, TEXT("Invalid index %d for CDO modifiers"), Index)
			continue;
		}

		FGameplayModifierInfo& ModifierInfo = InEffectCDO->Modifiers[Index];
		//Feature Begin Attribute In subclass of AttributeSet
		ModifierInfo.Attribute = FGameplayAttribute(ModifierToReplace.Property.Get(),ModifierToReplace.AttributeOwnerClass);
		//Feature End

		TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);

		Message->AddToken(FTextToken::Create(LOCTEXT("ChangedEffect", "Gameplay Effect: ")));
		if (InPayload.ReferencerBlueprint.IsValid())
		{
			Message->AddToken(
				FUObjectToken::Create(InPayload.ReferencerBlueprint.Get(), FText::FromString(InPayload.ReferencerBlueprint->GetName()))
				->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&USagaEditorSubsystem::HandleMessageLogLinkActivated))
			);
		}
		
		Message->AddToken(FTextToken::Create(FText::Format(
			LOCTEXT("ChangedModifierFromTo", "Updated Gameplay Effect > Modifiers at Index {0} from {1} to {2}"),
			FText::AsNumber(Index),
			FText::FromString(InPayload.OldPropertyName),
			FText::FromString(ModifierInfo.Attribute.GetName())
		)));
		
		OutMessages.Add(Message);
	}

	return !ModifiersToReplace.IsEmpty();
}

bool FSSGameplayEffectReferencerHandler::UpdateModifiersBackingAttribute(UGameplayEffect* InEffectCDO, const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, const FSSAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SS_EDITOR_NS_LOG(Display, TEXT("InEffectCDO: %s, InBlueprint: %s, InOldPropertyName: %s, InNewPropertyName: %s"),
		*GetNameSafe(InEffectCDO),
		*GetNameSafe(InBlueprint),
		*InPayload.OldPropertyName,
		*InPayload.NewPropertyName
	)

	check(InEffectCDO);
	check(InBlueprint);

	bool bWasModified = false;

	int32 CurrentIndex = 0;
	for (FGameplayModifierInfo& Modifier : InEffectCDO->Modifiers)
	{
		CurrentIndex++;
		
		FAttributeReference CachedAttribute;
		const bool bHasCachedAttribute = GetCachedAttributeByPredicate(InAssetIdentifier, CachedAttribute, [CurrentIndex](const FAttributeReference& Item)
		{
			return Item.Type == EAttributeReferenceType::ModifierInfo_BackingAttribute_AttributeToCapture && Item.Index == CurrentIndex - 1;
		});
		
		if (!bHasCachedAttribute)
		{
			continue;
		}

		const EGameplayEffectMagnitudeCalculation CalculationType = Modifier.ModifierMagnitude.GetMagnitudeCalculationType();
		
		TArray<FGameplayEffectAttributeCaptureDefinition> Definitions;
		Modifier.ModifierMagnitude.GetAttributeCaptureDefinitions(Definitions);
		
		if (CalculationType == EGameplayEffectMagnitudeCalculation::AttributeBased && Definitions.IsValidIndex(0))
		{
			const FGameplayAttribute Attribute = Definitions[0].AttributeToCapture;
			if (Attribute.IsValid())
			{
				continue;
			}

			if (CachedAttribute.PackageNameOwner == InPayload.PackageName && CachedAttribute.AttributeName == InPayload.OldPropertyName)
			{
				FProperty* NewAttributeProp = FindFProperty<FProperty>(InBlueprint->GeneratedClass, FName(*InPayload.NewPropertyName));
				if (!NewAttributeProp)
				{
					continue;
				}
				
				SS_EDITOR_NS_LOG(VeryVerbose, TEXT("Replacing modifier magnitude backing attribute with %s"), *GetNameSafe(NewAttributeProp))

				//Feature Begin Attribute In subclass of AttributeSet
				const FGameplayAttribute NewAttribute = FGameplayAttribute(NewAttributeProp,CachedAttribute.AttributeOwnerClass);
				//Feature End

				FAttributeBasedFloat AttributeBasedFloat = FAttributeBasedFloat();
				if (const FAttributeBasedFloat* PrevAttributeBased = GetAttributeBasedFloatFromContainer(&Modifier.ModifierMagnitude))
				{
					AttributeBasedFloat.Coefficient = PrevAttributeBased->Coefficient;
					AttributeBasedFloat.PreMultiplyAdditiveValue = PrevAttributeBased->PreMultiplyAdditiveValue;
					AttributeBasedFloat.PostMultiplyAdditiveValue = PrevAttributeBased->PostMultiplyAdditiveValue;
					AttributeBasedFloat.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(
						NewAttribute,
						PrevAttributeBased->BackingAttribute.AttributeSource,
						PrevAttributeBased->BackingAttribute.bSnapshot
					);
					AttributeBasedFloat.AttributeCurve = PrevAttributeBased->AttributeCurve;
					AttributeBasedFloat.AttributeCalculationType = PrevAttributeBased->AttributeCalculationType;
					AttributeBasedFloat.FinalChannel = PrevAttributeBased->FinalChannel;
					AttributeBasedFloat.SourceTagFilter = PrevAttributeBased->SourceTagFilter;
					AttributeBasedFloat.TargetTagFilter = PrevAttributeBased->TargetTagFilter;
				}
				else
				{
					AttributeBasedFloat.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(NewAttribute, EGameplayEffectAttributeCaptureSource::Source, false);
				}
				
				Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(AttributeBasedFloat);
				bWasModified = true;

				const TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);
				Message->AddToken(FTextToken::Create(LOCTEXT("ChangedEffect", "Gameplay Effect: ")));
				if (InPayload.ReferencerBlueprint.IsValid())
				{
					Message->AddToken(
						FUObjectToken::Create(InPayload.ReferencerBlueprint.Get(), FText::FromString(InPayload.ReferencerBlueprint->GetName()))
						->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&USagaEditorSubsystem::HandleMessageLogLinkActivated))
					);
				}
				
				Message->AddToken(FTextToken::Create(FText::Format(
					LOCTEXT("ChangedModifiersAttributeBaseFromTo", "Updated Gameplay Effect > Modifiers > Attribute Based Magnitude at Index {0} from {1} to {2}"),
					FText::AsNumber(CurrentIndex - 1),
					FText::FromString(InPayload.OldPropertyName),
					FText::FromString(InPayload.NewPropertyName)
				)));

				OutMessages.Add(Message);
			}
		}
	}

	return bWasModified;
}

bool FSSGameplayEffectReferencerHandler::UpdateDuration(UGameplayEffect* InEffectCDO, const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, const FSSAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SS_EDITOR_NS_LOG(
		Verbose,
		TEXT("InEffectCDO: %s, InAssetIdentifier: %s, InPackageName: %s, InOldPropertyName: %s, InNewPropertyName: %s"),
		*GetNameSafe(InEffectCDO),
		*InAssetIdentifier.ToString(),
		*InPayload.PackageName,
		*InPayload.OldPropertyName,
		*InPayload.NewPropertyName
	)
	
	check(InEffectCDO);
	check(InBlueprint);

	FAttributeReference CachedAttribute;
	const bool bHasCachedAttribute = GetCachedAttributeByPredicate(InAssetIdentifier, CachedAttribute, [](const FAttributeReference& Item)
	{
		return Item.Type == EAttributeReferenceType::DurationMagnitude;
	});

	if (!bHasCachedAttribute)
	{
		return false;
	}

	const FGameplayEffectModifierMagnitude DurationMagnitude = InEffectCDO->DurationMagnitude;

	TArray<FGameplayEffectAttributeCaptureDefinition> Definitions;
	DurationMagnitude.GetAttributeCaptureDefinitions(Definitions);

	if (DurationMagnitude.GetMagnitudeCalculationType() == EGameplayEffectMagnitudeCalculation::AttributeBased && Definitions.IsValidIndex(0))
	{
		const FGameplayAttribute Attribute = Definitions[0].AttributeToCapture;
		if (Attribute.IsValid())
		{
			return false;
		}

		if (CachedAttribute.PackageNameOwner == InPayload.PackageName && CachedAttribute.AttributeName == InPayload.OldPropertyName)
		{
			FProperty* NewAttributeProp = FindFProperty<FProperty>(InBlueprint->GeneratedClass, FName(*InPayload.NewPropertyName));
			if (!NewAttributeProp)
			{
				return false;
			}

			SS_EDITOR_NS_LOG(VeryVerbose, TEXT("Replacing duration magnitude attribute with %s"), *GetNameSafe(NewAttributeProp))

			//Feature Begin Attribute In subclass of AttributeSet
			const FGameplayAttribute NewAttribute = FGameplayAttribute(NewAttributeProp,CachedAttribute.AttributeOwnerClass);
			//Feature End

			FAttributeBasedFloat AttributeBasedFloat = FAttributeBasedFloat();
			if (const FAttributeBasedFloat* PrevAttributeBased = GetAttributeBasedFloatFromContainer(&InEffectCDO->DurationMagnitude))
			{
				AttributeBasedFloat.Coefficient = PrevAttributeBased->Coefficient;
				AttributeBasedFloat.PreMultiplyAdditiveValue = PrevAttributeBased->PreMultiplyAdditiveValue;
				AttributeBasedFloat.PostMultiplyAdditiveValue = PrevAttributeBased->PostMultiplyAdditiveValue;
				AttributeBasedFloat.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(
					NewAttribute,
					PrevAttributeBased->BackingAttribute.AttributeSource,
					PrevAttributeBased->BackingAttribute.bSnapshot
				);
				AttributeBasedFloat.AttributeCurve = PrevAttributeBased->AttributeCurve;
				AttributeBasedFloat.AttributeCalculationType = PrevAttributeBased->AttributeCalculationType;
				AttributeBasedFloat.FinalChannel = PrevAttributeBased->FinalChannel;
				AttributeBasedFloat.SourceTagFilter = PrevAttributeBased->SourceTagFilter;
				AttributeBasedFloat.TargetTagFilter = PrevAttributeBased->TargetTagFilter;
			}
			else
			{
				AttributeBasedFloat.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(NewAttribute, EGameplayEffectAttributeCaptureSource::Source, false);
			}

			InEffectCDO->DurationMagnitude = FGameplayEffectModifierMagnitude(AttributeBasedFloat);

			const TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);
			Message->AddToken(FTextToken::Create(LOCTEXT("ChangedEffect", "Gameplay Effect: ")));

			if (InPayload.ReferencerBlueprint.IsValid())
			{
				Message->AddToken(
					FUObjectToken::Create(InPayload.ReferencerBlueprint.Get(), FText::FromString(InPayload.ReferencerBlueprint->GetName()))
					->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&USagaEditorSubsystem::HandleMessageLogLinkActivated))
				);
			}
			
			Message->AddToken(FTextToken::Create(FText::Format(
			LOCTEXT("ChangedDurationFromTo", "Updated Gameplay Effect > Duration Magnitude > Attribute Based Magnitude > Backing Attribute from {0} to {1}"),
				FText::FromString(InPayload.OldPropertyName),
				FText::FromString(InPayload.NewPropertyName)
			)));

			OutMessages.Add(Message);
			return true;
		}
	}
	
	return false;
}

bool FSSGameplayEffectReferencerHandler::UpdateCues(UGameplayEffect* InEffectCDO, const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, const FSSAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SS_EDITOR_NS_LOG(
		Verbose,
		TEXT("InEffectCDO: %s, InBlueprint: %s, InAssetIdentifier: %s, InPayload: %s"),
		*GetNameSafe(InEffectCDO),
		*GetNameSafe(InBlueprint),
		*InAssetIdentifier.ToString(),
		*InPayload.ToString()
	)

	check(InEffectCDO);
	check(InBlueprint);
	
	bool bWasModified = false;
	
	int32 CurrentIndex = 0;
	for (FGameplayEffectCue& GameplayCue : InEffectCDO->GameplayCues)
	{
		CurrentIndex++;
		
		// Attribute still valid, no need to replace
		if (GameplayCue.MagnitudeAttribute.IsValid())
		{
			continue;	
		}
		
		FAttributeReference CachedAttribute;
		const bool bHasCachedAttribute = GetCachedAttributeByPredicate(InAssetIdentifier, CachedAttribute, [CurrentIndex](const FAttributeReference& Item)
		{
			return Item.Type == EAttributeReferenceType::GameplayCue_MagnitudeAttribute && Item.Index == CurrentIndex - 1;
		});

		if (!bHasCachedAttribute)
		{
			continue;
		}

		// Cached attribute for this index with invalid attribute is matching the old name, replace
		if (CachedAttribute.AttributeName == InPayload.OldPropertyName)
		{
			if (FProperty* NewAttributeProp = FindFProperty<FProperty>(InBlueprint->GeneratedClass, FName(*InPayload.NewPropertyName)))
			{
				//Feature Begin Attribute In subclass of AttributeSet
				GameplayCue.MagnitudeAttribute = FGameplayAttribute(NewAttributeProp,CachedAttribute.AttributeOwnerClass);
				//Feature End
				
				bWasModified = true;

				const TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);
				Message->AddToken(FTextToken::Create(LOCTEXT("ChangedEffect", "Gameplay Effect: ")));

				if (InPayload.ReferencerBlueprint.IsValid())
				{
					Message->AddToken(
						FUObjectToken::Create(InPayload.ReferencerBlueprint.Get(), FText::FromString(InPayload.ReferencerBlueprint->GetName()))
						->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&USagaEditorSubsystem::HandleMessageLogLinkActivated))
					);
				}

				Message->AddToken(FTextToken::Create(FText::Format(
					LOCTEXT("ChangedGameplayCuesFromToWithIndex", "Updated Display > Gameplay Cues > Magnitude Attribute at Index {0} from {1} to {2}"),
					FText::AsNumber(CurrentIndex - 1),
					FText::FromString(InPayload.OldPropertyName),
					FText::FromString(InPayload.NewPropertyName)
				)));

				OutMessages.Add(Message);
			}	
		}
	}
	
	return bWasModified;
}

bool FSSGameplayEffectReferencerHandler::UpdateRemoveGameplayEffectQuery(UGameplayEffect* InEffectCDO, const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, const FSSAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
#if UE_VERSION_NEWER_THAN(5, 3, -1)

	bool bModified = false;
	if (URemoveOtherGameplayEffectComponent* GameplayEffectComponent = const_cast<URemoveOtherGameplayEffectComponent*>(InEffectCDO->FindComponent<URemoveOtherGameplayEffectComponent>()))
	{
		int32 CurrentIndex = 0;
		for (FGameplayEffectQuery& Query : GameplayEffectComponent->RemoveGameplayEffectQueries)
		{
			const FText MessageText = FText::Format(
				LOCTEXT("RemoveGameplayEffectQuery_ModifyingAttribute_WithIndex", "Tags > Remove Gameplay Effect Query [{0}] > ModifyingAttribute"),
				CurrentIndex
			);
			
			bModified |= UpdateEffectQuery(
				InBlueprint,
				InAssetIdentifier,
				EAttributeReferenceType::RemoveGameplayEffectQuery_ModifyingAttribute,
				InPayload,
				MessageText,
				Query.ModifyingAttribute,
				OutMessages
			);
			
			CurrentIndex++;
		}
	}

	return bModified;
	
#else
	const FText MessageText = LOCTEXT("RemoveGameplayEffectQuery_ModifyingAttribute", "Tags > Remove Gameplay Effect Query > ModifyingAttribute");
	return UpdateEffectQuery(
		InBlueprint,
		InAssetIdentifier,
		EAttributeReferenceType::RemoveGameplayEffectQuery_ModifyingAttribute,
		InPayload,
		MessageText,
		InEffectCDO->RemoveGameplayEffectQuery.ModifyingAttribute,
		OutMessages
	);
#endif
}

bool FSSGameplayEffectReferencerHandler::UpdateImmunityEffectQuery(UGameplayEffect* InEffectCDO, const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, const FSSAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
#if UE_VERSION_NEWER_THAN(5, 3, -1)

	bool bModified = false;
	if (UImmunityGameplayEffectComponent* GameplayEffectComponent = const_cast<UImmunityGameplayEffectComponent*>(InEffectCDO->FindComponent<UImmunityGameplayEffectComponent>()))
	{
		int32 CurrentIndex = 0;
		for (FGameplayEffectQuery& Query : GameplayEffectComponent->ImmunityQueries)
		{
			const FText MessageText = FText::Format(
				LOCTEXT("GrantedApplicationImmunityQuery_ModifyingAttribute_WithIndex", "Immunity > Granted Application Immunity Query [{0}] > ModifyingAttribute"),
				CurrentIndex
			);
			
			bModified |= UpdateEffectQuery(
				InBlueprint,
				InAssetIdentifier,
				EAttributeReferenceType::GrantedApplicationImmunityQuery_ModifyingAttribute,
				InPayload,
				MessageText,
				Query.ModifyingAttribute,
				OutMessages
			);
			
			CurrentIndex++;
		}
	}

	return bModified;
	
#else
	const FText MessageText = LOCTEXT("GrantedApplicationImmunityQuery_ModifyingAttribute", "Immunity > Granted Application Immunity Query > ModifyingAttribute");
	return UpdateEffectQuery(
		InBlueprint,
		InAssetIdentifier,
		EAttributeReferenceType::GrantedApplicationImmunityQuery_ModifyingAttribute,
		InPayload,
		MessageText,
		InEffectCDO->GrantedApplicationImmunityQuery.ModifyingAttribute,
		OutMessages
	);
#endif
}

bool FSSGameplayEffectReferencerHandler::UpdateEffectQuery(const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, EAttributeReferenceType InReferenceType, const FSSAttributeReferencerPayload& InPayload, const FText& InMessageText, FGameplayAttribute& OutModifyingAttribute, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SS_EDITOR_NS_LOG(Verbose, TEXT("InBlueprint: %s, InAssetIdentifier: %s, InPayload: %s"), *GetNameSafe(InBlueprint), *InAssetIdentifier.ToString(), *InPayload.ToString())

	check(InBlueprint);

	FAttributeReference CachedAttribute;
	const bool bHasCachedAttribute = GetCachedAttributeByPredicate(InAssetIdentifier, CachedAttribute, [InReferenceType](const FAttributeReference& Item)
	{
		return Item.Type == InReferenceType;
	});

	if (!bHasCachedAttribute)
	{
		return false;
	}

	if (CachedAttribute.PackageNameOwner == InPayload.PackageName && CachedAttribute.AttributeName == InPayload.OldPropertyName)
	{
		FProperty* NewAttributeProp = FindFProperty<FProperty>(InBlueprint->GeneratedClass, FName(*InPayload.NewPropertyName));
		if (!NewAttributeProp)
		{
			return false;
		}

		SS_EDITOR_NS_LOG(VeryVerbose, TEXT("Replacing %s with %s"), *InMessageText.ToString(), *GetNameSafe(NewAttributeProp))

		//Feature Begin Attribute In subclass of AttributeSet
		// Update it to the new attribute
		OutModifyingAttribute = FGameplayAttribute(NewAttributeProp,CachedAttribute.AttributeOwnerClass);
		//Feature End

		// Build up a message for message log
		const TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);
		Message->AddToken(FTextToken::Create(FText::Format(LOCTEXT("ChangedEffectQuery_ModifyingAttribute", "Gameplay Effect: "), InMessageText)));

		if (InPayload.ReferencerBlueprint.IsValid())
		{
			Message->AddToken(
				FUObjectToken::Create(InPayload.ReferencerBlueprint.Get(), FText::FromString(InPayload.ReferencerBlueprint->GetName()))
				->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&USagaEditorSubsystem::HandleMessageLogLinkActivated))
			);
		}
		
		Message->AddToken(FTextToken::Create(FText::Format(
			LOCTEXT("ChangedFromTo", "Updated {0} from {1} to {2}"),
			InMessageText,
			FText::FromString(InPayload.OldPropertyName),
			FText::FromString(InPayload.NewPropertyName)
		)));

		OutMessages.Add(Message);
		
		return true;
	}

	return false;
}

FAttributeBasedFloat* FSSGameplayEffectReferencerHandler::GetAttributeBasedFloatFromContainer(FGameplayEffectModifierMagnitude* InModifierMagnitude) const
{
	if (const FProperty* AttributeBasedMagnitudeProp = FindFProperty<FProperty>(FGameplayEffectModifierMagnitude::StaticStruct(), TEXT("AttributeBasedMagnitude")))
	{
		uint8* ClassDefaults = reinterpret_cast<uint8*>(InModifierMagnitude);
		return AttributeBasedMagnitudeProp->ContainerPtrToValuePtr<FAttributeBasedFloat>(ClassDefaults);
	}

	return nullptr;
}

FString FSSGameplayEffectReferencerHandler::GetClassDefaultName(const FString& InName)
{
	FString Name = InName;
	Name.RemoveFromStart(TEXT("Default__"));
	Name.RemoveFromEnd(TEXT("_C"));
	return Name;
}

#undef LOCTEXT_NAMESPACE

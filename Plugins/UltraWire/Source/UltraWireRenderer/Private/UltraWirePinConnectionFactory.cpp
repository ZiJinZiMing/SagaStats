// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWirePinConnectionFactory.h"
#include "UltraWireDrawingPolicy.h"
#include "UltraWireSettings.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphSchema.h"
#include "Layout/SlateRect.h"

// Schema class names used for graph-type identification.
// We use FName comparisons rather than static_cast<> to avoid
// compile-time dependencies on every schema header (which would require
// adding a long tail of private module dependencies to the Build.cs).
namespace UltraWireSchemaNames
{
	static const FName BlueprintSchema      (TEXT("EdGraphSchema_K2"));
	static const FName MaterialSchema       (TEXT("MaterialGraphSchema"));
	static const FName AnimGraphSchema      (TEXT("AnimationGraphSchema"));
	static const FName BehaviorTreeSchema   (TEXT("EdGraphSchema_BehaviorTree"));
	static const FName ControlRigSchema     (TEXT("ControlRigGraphSchema"));
	static const FName PCGSchema            (TEXT("PCGEditorGraphSchema"));
	static const FName SoundCueSchema       (TEXT("SoundCueGraphSchema"));
	static const FName MetaSoundSchema      (TEXT("MetasoundEditorGraphSchema"));
	static const FName EnvQuerySchema       (TEXT("EnvironmentQuerySchema"));
	static const FName NiagaraSchema        (TEXT("NiagaraEdGraphSchema"));
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

FUltraWirePinConnectionFactory::FUltraWirePinConnectionFactory(EUltraWireGraphType InGraphType)
	: GraphType(InGraphType)
{
}

// ---------------------------------------------------------------------------
// SchemaToGraphType
// ---------------------------------------------------------------------------

EUltraWireGraphType FUltraWirePinConnectionFactory::SchemaToGraphType(const UEdGraphSchema* Schema)
{
	if (!Schema)
	{
		return EUltraWireGraphType::Other;
	}

	const FName ClassName = Schema->GetClass()->GetFName();

	if (ClassName == UltraWireSchemaNames::BlueprintSchema)   { return EUltraWireGraphType::Blueprint; }
	if (ClassName == UltraWireSchemaNames::MaterialSchema)    { return EUltraWireGraphType::Material; }
	if (ClassName == UltraWireSchemaNames::AnimGraphSchema)   { return EUltraWireGraphType::AnimationBlueprint; }
	if (ClassName == UltraWireSchemaNames::BehaviorTreeSchema){ return EUltraWireGraphType::BehaviorTree; }
	if (ClassName == UltraWireSchemaNames::ControlRigSchema)  { return EUltraWireGraphType::ControlRig; }
	if (ClassName == UltraWireSchemaNames::PCGSchema)         { return EUltraWireGraphType::PCG; }
	if (ClassName == UltraWireSchemaNames::SoundCueSchema)    { return EUltraWireGraphType::SoundCue; }
	if (ClassName == UltraWireSchemaNames::MetaSoundSchema)   { return EUltraWireGraphType::MetaSound; }
	if (ClassName == UltraWireSchemaNames::EnvQuerySchema)    { return EUltraWireGraphType::EnvironmentQuery; }
	if (ClassName == UltraWireSchemaNames::NiagaraSchema)     { return EUltraWireGraphType::Niagara; }

	return EUltraWireGraphType::Other;
}

// ---------------------------------------------------------------------------
// CreateConnectionPolicy
// ---------------------------------------------------------------------------

FConnectionDrawingPolicy* FUltraWirePinConnectionFactory::CreateConnectionPolicy(
	const UEdGraphSchema* Schema,
	int32 InBackLayerID,
	int32 InFrontLayerID,
	float ZoomFactor,
	const FSlateRect& InClippingRect,
	FSlateWindowElementList& InDrawElements,
	UEdGraph* InGraphObj) const
{
	// Bail out early if the plugin is globally disabled.
	const UUltraWireSettings* Settings = UUltraWireSettings::Get();
	if (!Settings || !Settings->bEnabled)
	{
		// Returning nullptr causes the engine to fall through to the next
		// registered factory (or its built-in default).
		return nullptr;
	}

	// Identify the graph type from the schema so the policy picks up the
	// correct settings profile.
	const EUltraWireGraphType DetectedType = SchemaToGraphType(Schema);

	return new FUltraWireDrawingPolicy(
		InBackLayerID,
		InFrontLayerID,
		ZoomFactor,
		InClippingRect,
		InDrawElements,
		InGraphObj,
		DetectedType);
}

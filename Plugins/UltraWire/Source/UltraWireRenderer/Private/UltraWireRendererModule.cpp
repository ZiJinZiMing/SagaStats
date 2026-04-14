// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireRendererModule.h"
#include "UltraWirePinConnectionFactory.h"
#include "UltraWireTypes.h"

#include "EdGraphUtilities.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FUltraWireRendererModule"

// ---------------------------------------------------------------------------
// StartupModule
// ---------------------------------------------------------------------------

void FUltraWireRendererModule::StartupModule()
{
	// Create one factory per supported graph type and register it with the
	// global FEdGraphUtilities registry.  Order matters: factories registered
	// later are queried first, so we register the most specific types last
	// (Blueprint is the most-used, so it is registered last and queried first).

	static const EUltraWireGraphType GraphTypesToRegister[] =
	{
		EUltraWireGraphType::Other,
		EUltraWireGraphType::SoundCue,
		EUltraWireGraphType::MetaSound,
		EUltraWireGraphType::EnvironmentQuery,
		EUltraWireGraphType::BehaviorTree,
		EUltraWireGraphType::PCG,
		EUltraWireGraphType::ControlRig,
		EUltraWireGraphType::Niagara,
		EUltraWireGraphType::AnimationBlueprint,
		EUltraWireGraphType::Material,
		EUltraWireGraphType::Blueprint,   // queried first (registered last)
	};

	RegisteredFactories.Reserve(UE_ARRAY_COUNT(GraphTypesToRegister));

	for (EUltraWireGraphType GraphType : GraphTypesToRegister)
	{
		TSharedPtr<FUltraWirePinConnectionFactory> Factory =
			MakeShared<FUltraWirePinConnectionFactory>(GraphType);

		// FEdGraphUtilities::RegisterVisualPinConnectionFactory takes a raw
		// TSharedPtr<FGraphPanelPinConnectionFactory> and stores it internally.
		FEdGraphUtilities::RegisterVisualPinConnectionFactory(Factory);

		RegisteredFactories.Add(MoveTemp(Factory));
	}

	UE_LOG(LogTemp, Log,
		TEXT("UltraWireRenderer: Registered %d pin connection factories."),
		RegisteredFactories.Num());
}

// ---------------------------------------------------------------------------
// ShutdownModule
// ---------------------------------------------------------------------------

void FUltraWireRendererModule::ShutdownModule()
{
	// Unregister all factories in reverse registration order.
	for (int32 i = RegisteredFactories.Num() - 1; i >= 0; --i)
	{
		if (RegisteredFactories[i].IsValid())
		{
			FEdGraphUtilities::UnregisterVisualPinConnectionFactory(RegisteredFactories[i]);
		}
	}

	RegisteredFactories.Empty();

	UE_LOG(LogTemp, Log, TEXT("UltraWireRenderer: Unregistered all pin connection factories."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUltraWireRendererModule, UltraWireRenderer)

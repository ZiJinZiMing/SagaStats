// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UltraWireTypes.generated.h"

UENUM(BlueprintType)
enum class EUltraWireStyle : uint8
{
	Default		UMETA(DisplayName = "Default (Bezier)"),
	Manhattan	UMETA(DisplayName = "Manhattan (90°)"),
	Subway		UMETA(DisplayName = "Subway (45°)"),
	Freeform	UMETA(DisplayName = "Freeform (Custom Angle)")
};

UENUM(BlueprintType)
enum class EUltraWireCrossingStyle : uint8
{
	None	UMETA(DisplayName = "None"),
	Gap		UMETA(DisplayName = "Gap"),
	Arc		UMETA(DisplayName = "Arc"),
	Circle	UMETA(DisplayName = "Circle")
};

UENUM(BlueprintType)
enum class EUltraWireCornerStyle : uint8
{
	Sharp		UMETA(DisplayName = "Sharp"),
	Rounded		UMETA(DisplayName = "Rounded"),
	Chamfered	UMETA(DisplayName = "Chamfered")
};

UENUM(BlueprintType)
enum class EUltraWirePinShape : uint8
{
	Circle		UMETA(DisplayName = "Circle"),
	Diamond		UMETA(DisplayName = "Diamond"),
	Square		UMETA(DisplayName = "Square"),
	Arrow		UMETA(DisplayName = "Arrow")
};

UENUM(BlueprintType)
enum class EUltraWireGraphType : uint8
{
	Blueprint			UMETA(DisplayName = "Blueprint"),
	Material			UMETA(DisplayName = "Material"),
	Niagara				UMETA(DisplayName = "Niagara"),
	AnimationBlueprint	UMETA(DisplayName = "Animation Blueprint"),
	BehaviorTree		UMETA(DisplayName = "Behavior Tree"),
	ControlRig			UMETA(DisplayName = "Control Rig"),
	PCG					UMETA(DisplayName = "PCG"),
	SoundCue			UMETA(DisplayName = "Sound Cue"),
	MetaSound			UMETA(DisplayName = "MetaSound"),
	EnvironmentQuery	UMETA(DisplayName = "Environment Query"),
	GameplayAbility		UMETA(DisplayName = "Gameplay Ability"),
	Other				UMETA(DisplayName = "Other")
};

UENUM(BlueprintType)
enum class EUltraWireMinimapPosition : uint8
{
	TopLeft			UMETA(DisplayName = "Top Left"),
	TopRight		UMETA(DisplayName = "Top Right"),
	BottomLeft		UMETA(DisplayName = "Bottom Left"),
	BottomRight		UMETA(DisplayName = "Bottom Right")
};

USTRUCT(BlueprintType)
struct ULTRAWIRECORE_API FUltraWireProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Wire Style")
	EUltraWireStyle WireStyle = EUltraWireStyle::Manhattan;

	UPROPERTY(EditAnywhere, Category = "Wire Style", meta = (ClampMin = "5.0", ClampMax = "85.0", EditCondition = "WireStyle == EUltraWireStyle::Freeform"))
	float FreeformAngle = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Wire Style")
	EUltraWireCornerStyle CornerStyle = EUltraWireCornerStyle::Rounded;

	UPROPERTY(EditAnywhere, Category = "Wire Style", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float CornerRadius = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Wire Style", meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float WireThickness = 1.5f;

	// Ribbon / stacking
	UPROPERTY(EditAnywhere, Category = "Ribbons")
	bool bEnableRibbons = false;

	UPROPERTY(EditAnywhere, Category = "Ribbons", meta = (ClampMin = "1.0", ClampMax = "20.0", EditCondition = "bEnableRibbons"))
	float RibbonOffset = 4.0f;

	// Crossings
	UPROPERTY(EditAnywhere, Category = "Crossings")
	EUltraWireCrossingStyle CrossingStyle = EUltraWireCrossingStyle::None;

	UPROPERTY(EditAnywhere, Category = "Crossings", meta = (ClampMin = "3.0", ClampMax = "20.0", EditCondition = "CrossingStyle != EUltraWireCrossingStyle::None"))
	float CrossingSize = 8.0f;

	// Bubbles
	UPROPERTY(EditAnywhere, Category = "Bubbles")
	bool bEnableBubbles = false;

	UPROPERTY(EditAnywhere, Category = "Bubbles", meta = (ClampMin = "10.0", ClampMax = "500.0", EditCondition = "bEnableBubbles"))
	float BubbleSpeed = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Bubbles", meta = (ClampMin = "1.0", ClampMax = "10.0", EditCondition = "bEnableBubbles"))
	float BubbleSize = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Bubbles", meta = (ClampMin = "10.0", ClampMax = "200.0", EditCondition = "bEnableBubbles"))
	float BubbleSpacing = 50.0f;

	// Glow
	UPROPERTY(EditAnywhere, Category = "Glow")
	bool bEnableGlow = false;

	UPROPERTY(EditAnywhere, Category = "Glow", meta = (ClampMin = "0.0", ClampMax = "5.0", EditCondition = "bEnableGlow"))
	float GlowIntensity = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Glow", meta = (ClampMin = "1.0", ClampMax = "20.0", EditCondition = "bEnableGlow"))
	float GlowWidth = 6.0f;

	UPROPERTY(EditAnywhere, Category = "Glow", meta = (EditCondition = "bEnableGlow"))
	bool bGlowPulse = false;

	UPROPERTY(EditAnywhere, Category = "Glow", meta = (ClampMin = "0.1", ClampMax = "5.0", EditCondition = "bEnableGlow && bGlowPulse"))
	float GlowPulseSpeed = 1.0f;

	// Smart routing
	UPROPERTY(EditAnywhere, Category = "Smart Routing")
	bool bEnableSmartRouting = false;

	UPROPERTY(EditAnywhere, Category = "Smart Routing", meta = (ClampMin = "5", ClampMax = "50", EditCondition = "bEnableSmartRouting"))
	int32 RoutingGridSize = 20;

	UPROPERTY(EditAnywhere, Category = "Smart Routing", meta = (ClampMin = "0.0", ClampMax = "50.0", EditCondition = "bEnableSmartRouting"))
	float RoutingNodePadding = 10.0f;

	// Heatmap
	UPROPERTY(EditAnywhere, Category = "Heatmap")
	bool bEnableHeatmap = false;

	UPROPERTY(EditAnywhere, Category = "Heatmap", meta = (EditCondition = "bEnableHeatmap"))
	FLinearColor HeatmapColdColor = FLinearColor(0.0f, 0.2f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Heatmap", meta = (EditCondition = "bEnableHeatmap"))
	FLinearColor HeatmapHotColor = FLinearColor(1.0f, 0.1f, 0.0f, 1.0f);

	// Node theming
	UPROPERTY(EditAnywhere, Category = "Node Theme")
	bool bEnableNodeTheming = false;

	UPROPERTY(EditAnywhere, Category = "Node Theme", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableNodeTheming"))
	float NodeBodyOpacity = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Node Theme", meta = (EditCondition = "bEnableNodeTheming"))
	FLinearColor NodeHeaderTintColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Node Theme", meta = (ClampMin = "0.0", ClampMax = "16.0", EditCondition = "bEnableNodeTheming"))
	float NodeCornerRadius = 4.0f;

	UPROPERTY(EditAnywhere, Category = "Node Theme", meta = (EditCondition = "bEnableNodeTheming"))
	EUltraWirePinShape PinShape = EUltraWirePinShape::Circle;

	// Wire labels
	UPROPERTY(EditAnywhere, Category = "Labels")
	bool bEnableAutoLabels = false;

	// Minimap
	UPROPERTY(EditAnywhere, Category = "Minimap")
	bool bEnableMinimap = false;

	UPROPERTY(EditAnywhere, Category = "Minimap", meta = (EditCondition = "bEnableMinimap"))
	EUltraWireMinimapPosition MinimapPosition = EUltraWireMinimapPosition::BottomRight;

	UPROPERTY(EditAnywhere, Category = "Minimap", meta = (ClampMin = "100.0", ClampMax = "500.0", EditCondition = "bEnableMinimap"))
	float MinimapSize = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Minimap", meta = (ClampMin = "0.1", ClampMax = "1.0", EditCondition = "bEnableMinimap"))
	float MinimapOpacity = 0.8f;
};

USTRUCT(BlueprintType)
struct ULTRAWIRECORE_API FUltraWirePreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Preset")
	FString PresetName;

	UPROPERTY(EditAnywhere, Category = "Preset")
	FString Description;

	UPROPERTY(EditAnywhere, Category = "Preset")
	FString Author;

	UPROPERTY(EditAnywhere, Category = "Preset")
	FUltraWireProfile Profile;
};

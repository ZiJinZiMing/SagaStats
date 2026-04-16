/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamagePipelineGraphSchema.cpp
#include "Graph/DamagePipelineGraphSchema.h"
#include "DamagePipelineConnectionDrawingPolicy.h"

const FPinConnectionResponse UDamagePipelineGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Read-only graph"));
}

FLinearColor UDamagePipelineGraphSchema::GetColorForEffectType(const UScriptStruct* EffectType)
{
	// 独立 12 色调色板（按 EffectType 类名哈希取模分配）：#e6194b #3cb44b #4363d8 #f58231 #911eb4 #46f0f0 #f032e6 #bcf60c #fabebe #008080 #e6beff #9a6324
	static const FLinearColor Palette[] = {
		FLinearColor::FromSRGBColor(FColor(0xe6, 0x19, 0x4b)),
		FLinearColor::FromSRGBColor(FColor(0x3c, 0xb4, 0x4b)),
		FLinearColor::FromSRGBColor(FColor(0x43, 0x63, 0xd8)),
		FLinearColor::FromSRGBColor(FColor(0xf5, 0x82, 0x31)),
		FLinearColor::FromSRGBColor(FColor(0x91, 0x1e, 0xb4)),
		FLinearColor::FromSRGBColor(FColor(0x46, 0xf0, 0xf0)),
		FLinearColor::FromSRGBColor(FColor(0xf0, 0x32, 0xe6)),
		FLinearColor::FromSRGBColor(FColor(0xbc, 0xf6, 0x0c)),
		FLinearColor::FromSRGBColor(FColor(0xfa, 0xbe, 0xbe)),
		FLinearColor::FromSRGBColor(FColor(0x00, 0x80, 0x80)),
		FLinearColor::FromSRGBColor(FColor(0xe6, 0xbe, 0xff)),
		FLinearColor::FromSRGBColor(FColor(0x9a, 0x63, 0x24)),
	};
	constexpr int32 PaletteSize = UE_ARRAY_COUNT(Palette);

	if (EffectType)
	{
		// 用类型名字符串哈希，跨会话稳定
		const uint32 Hash = GetTypeHash(EffectType->GetName());
		return Palette[Hash % PaletteSize];
	}
	return FLinearColor(0.42f, 0.56f, 0.75f);
}

FLinearColor UDamagePipelineGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	if (PinType.PinCategory == TEXT("DamageEffect"))
	{
		const UObject* TypeObj = PinType.PinSubCategoryObject.Get();
		return GetColorForEffectType(Cast<UScriptStruct>(TypeObj));
	}
	return Super::GetPinTypeColor(PinType);
}

FConnectionDrawingPolicy* UDamagePipelineGraphSchema::CreateConnectionDrawingPolicy(
	int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor,
	const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements,
	UEdGraph* InGraphObj) const
{
	return new FDamagePipelineConnectionDrawingPolicy(
		InBackLayerID, InFrontLayerID, InZoomFactor,
		InClippingRect, InDrawElements, this);
}

/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "SSSPositiveActionButton.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Styling/StyleColors.h"

void SSSPositiveActionButton::Construct(const FArguments& InArgs)
{
	check(InArgs._Icon.IsSet());

	TAttribute<FText> Text = InArgs._Text;

	const TSharedRef<SHorizontalBox> ButtonContent = SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SImage)
			.Image(InArgs._Icon)
			.ColorAndOpacity(FStyleColors::AccentGreen)
		]
		+SHorizontalBox::Slot()
		.Padding(FMargin(3, 0, 0, 0))
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(STextBlock)
			.TextStyle(FAppStyle::Get(), "SmallButtonText")
			.Text(InArgs._Text)
			.Visibility_Lambda([Text]
			{
				return Text.Get(FText::GetEmpty()).IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
			})
		];

	ChildSlot
	[
		SAssignNew(ComboButton, SComboButton)
		.HasDownArrow(false)
		.ContentPadding(FMargin(2.0f, 3.0f))
		.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
		.ForegroundColor(FSlateColor::UseStyle())
		.IsEnabled(InArgs._IsEnabled)
		.ToolTipText(InArgs._ToolTipText)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.MenuPlacement(InArgs._MenuPlacement)
		.ButtonContent()
		[
			ButtonContent
		]
		.MenuContent()
		[
			InArgs._MenuContent.Widget
		]
		.OnGetMenuContent(InArgs._OnGetMenuContent)
		.OnMenuOpenChanged(InArgs._OnMenuOpenChanged)
		.OnComboBoxOpened(InArgs._OnComboBoxOpened)
	];
}

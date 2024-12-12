/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "Framework/SlateDelegates.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboButton.h"

class SButton;
class SComboButton;

/**
 * A Button that is used to call out/highlight a positive option (Add, Save etc). It can also be used to open a menu.
 *
 * Based off SPositiveActionButton in ToolsWidget.
 *
 * Forked mainly to allow custom menu placement with a sensible default for our use case (eg. MenuPlacement_ComboBoxRight)
 */
class SSSPositiveActionButton : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SSSPositiveActionButton)
		: _Icon(FAppStyle::Get().GetBrush("Icons.Plus"))
		, _MenuPlacement(MenuPlacement_ComboBoxRight)
		{}

		/** The text to display in the button. */
		SLATE_ATTRIBUTE(FText, Text)

		SLATE_ATTRIBUTE(const FSlateBrush*, Icon)

		/** The static menu content widget. */
		SLATE_NAMED_SLOT(FArguments, MenuContent)

		SLATE_EVENT(FOnGetContent, OnGetMenuContent)
		SLATE_EVENT(FOnComboBoxOpened, OnComboBoxOpened)
		SLATE_EVENT(FOnIsOpenChanged, OnMenuOpenChanged)

		SLATE_ATTRIBUTE(EMenuPlacement, MenuPlacement)

	SLATE_END_ARGS()

	SSSPositiveActionButton() {}

	void Construct(const FArguments& InArgs);

private:

	TSharedPtr<SComboButton> ComboButton;
	TSharedPtr<SButton> Button;
};
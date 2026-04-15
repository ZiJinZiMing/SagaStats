// DamagePipelineEditorSettings.cpp
#include "DamagePipelineEditorSettings.h"

UDamagePipelineEditorSettings::UDamagePipelineEditorSettings()
	: IndexRenderStyle(EDPIndexRenderStyle::Prefix)
	, ConditionFormatStyle(EDPConditionFormatStyle::IndentOnly)
	, bNotAsIndependentLine(false)
{
}

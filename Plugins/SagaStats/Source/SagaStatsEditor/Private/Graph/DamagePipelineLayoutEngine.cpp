// DamagePipelineLayoutEngine.cpp
#include "DamagePipelineLayoutEngine.h"

FDamagePipelineLayoutOutput FDamagePipelineLayoutEngine::Compute(const FDamagePipelineLayoutInput& Input)
{
	using namespace DamagePipelineLayoutConstants;

	FDamagePipelineLayoutOutput Output;
	const int32 RuleCount = Input.TotalPinCounts.Num();
	if (RuleCount <= 0) return Output;

	Output.RuleNodePositions.SetNum(RuleCount);

	double CurX = 0.0;
	double CurY = 0.0;

	for (int32 i = 0; i < RuleCount; ++i)
	{
		Output.RuleNodePositions[i] = FVector2D(CurX, CurY);

		// Y 累积：阶梯
		const int32 TotalPinCount = FMath::Max(1, Input.TotalPinCounts[i]);
		const double NodeHeight = (double)HeaderPadding
			+ (double)TotalPinCount * (double)PinRowHeight
			+ (double)BottomPadding;
		CurY += NodeHeight + (double)GapBetweenNodesY;

		// X 累积：下一个节点的输入通道数决定 gap
		if (i + 1 < RuleCount)
		{
			const int32 NextInputCount = FMath::Max(0, Input.InputPinCounts.IsValidIndex(i + 1) ? Input.InputPinCounts[i + 1] : 0);
			CurX += (double)NodeWidth + (double)BaseGap + (double)NextInputCount * (double)ChannelWidth;
		}
	}

	return Output;
}

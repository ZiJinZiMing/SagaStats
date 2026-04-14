// DamagePipelineLayoutEngine.h — 阶梯布局 + 输入通道布局引擎
#pragma once

#include "CoreMinimal.h"

/**
 * 阶梯 + 通道布局：
 *
 * - Y 方向（阶梯）：每个节点的 Y 原点 = 之前所有节点的 Pin 行数累积。
 *   每个 Pin 在全图有唯一 Y 行，水平段按构造不重合。
 *
 * - X 方向（通道）：每个节点前的 gap = BaseGap + InputCount × ChannelWidth。
 *   每个输入 Pin 独占一条竖直通道 X = Node.Left - (InputIdx+1) × ChannelWidth，
 *   连线的垂直段按构造不重合，也不跨节点身体。
 *
 * 双重唯一性：水平段 Y 唯一 + 垂直段 X 唯一 → 所有边按构造不可能叠加。
 */

namespace DamagePipelineLayoutConstants
{
	constexpr float ChannelWidth    = 20.f; // 每条输入通道的 X 宽度
	constexpr float NodeWidth       = 320.f; // 节点宽度估计值（用于 X 累积）
	constexpr float BaseGap         = 50.f;  // 每节点之间的基础 X gap（不含通道）
	constexpr float PinRowHeight    = 40.f;  // Pin 行高度
	constexpr float HeaderPadding   = 60.f;  // 节点顶部留白
	constexpr float BottomPadding   = 30.f;  // 节点底部留白
	constexpr float GapBetweenNodesY = 10.f; // 节点 Y 方向间距
}

struct FDamagePipelineLayoutInput
{
	/** 每个节点的总 Pin 数（输入 + 输出），用于 Y 阶梯累积 */
	TArray<int32> TotalPinCounts;

	/** 每个节点的输入 Pin 数，用于 X 通道宽度 */
	TArray<int32> InputPinCounts;
};

struct FDamagePipelineLayoutOutput
{
	/** 每个节点的位置（图世界坐标，node top-left） */
	TArray<FVector2D> RuleNodePositions;
};

class FDamagePipelineLayoutEngine
{
public:
	/**
	 * 执行布局。
	 * X 方向：累积 = NodeWidth + BaseGap + NextInputCount × ChannelWidth
	 * Y 方向：累积 = HeaderPadding + TotalPinCount × PinRowHeight + BottomPadding + GapBetweenNodesY
	 */
	static FDamagePipelineLayoutOutput Compute(const FDamagePipelineLayoutInput& Input);
};

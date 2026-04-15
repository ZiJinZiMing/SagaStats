// DamagePipelineLayoutConstants.h — DamagePipeline Graph 布局的调优常量
#pragma once

#include "CoreMinimal.h"

/**
 * DamagePipeline Graph 布局常量
 *
 * 真实布局（Slate 真实尺寸驱动）在 FDamagePipelineAssetEditor::ApplyRealSizeLayoutCorrection 里
 * 做一次遍历即完成。这里只放设计决策性常量：
 *
 * - X 方向：CurX += RealNodeWidth + BaseGap + NextInputCount × ChannelWidth
 *     BaseGap       节点之间的纯间距（与通道无关）
 *     ChannelWidth  每个输入 Pin 独占的垂直走线通道宽度（DrawingPolicy 也按这个算 DropX）
 *
 * - Y 方向：CurY += RealNodeHeight + GapBetweenNodesY
 *     GapBetweenNodesY  节点底边与下一个节点顶边的垂直间距
 */
namespace DamagePipelineLayoutConstants
{
	constexpr float BaseGap          = 20.f; // 节点间基础 X 间距（纯 padding）
	constexpr float ChannelWidth     = 12.f; // 每条输入通道的 X 宽度（DrawingPolicy 用）
	constexpr float GapBetweenNodesY = 10.f; // 节点 Y 方向间距
}

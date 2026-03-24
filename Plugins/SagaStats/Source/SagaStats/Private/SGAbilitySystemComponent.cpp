/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/


#include "SGAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "DisplayDebugHelpers.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "Meter/MeterBase.h"
#include "Meter/DecreaseMeter.h"
#include "Meter/IncreaseMeter.h"


void USGAbilitySystemComponent::RemoveAttributeSet(UAttributeSet* AttributeSet)
{
	RemoveSpawnedAttribute(AttributeSet);
}

void USGAbilitySystemComponent::RemoveAttributeSetByClass(TSubclassOf<UAttributeSet> AttributeSetClass)
{
	if (UAttributeSet* Set = const_cast<UAttributeSet*>(GetAttributeSet(AttributeSetClass)))
	{
		RemoveAttributeSet(Set);
	}
}

FOnAttributeSetAddOrRemoveEvent& USGAbilitySystemComponent::GetAttributeSetAddOrRemoveDelegate(TSubclassOf<UAttributeSet> SetClass)
{
	return AttributeSetAddOrRemoveDelegates.FindOrAdd(SetClass);
}

FOnMeterEmptiedEvent& USGAbilitySystemComponent::GetMeterEmptiedDelegate(TSubclassOf<UMeterBase> MeterClass)
{
	return MeterEmptiedDelegates.FindOrAdd(MeterClass);
}

FOnMeterFilledEvent& USGAbilitySystemComponent::GetMeterFilledDelegate(TSubclassOf<UMeterBase> MeterClass)
{
	return MeterFilledDelegates.FindOrAdd(MeterClass);
}

FOnMeterStateChangeEvent& USGAbilitySystemComponent::GetMeterStateChangeDelegate(
	TSubclassOf<UDecreaseMeter> MeterClass)
{
	return MeterStateChangeDelegates.FindOrAdd(MeterClass);
}

void USGAbilitySystemComponent::OnShowMeterDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	if (DisplayInfo.IsDisplayOn(TEXT("Meter")))
	{
		UWorld* World = HUD->GetWorld();

		AActor* TargetActor = HUD->GetCurrentDebugTargetActor();
		USGAbilitySystemComponent* SagaASC = Cast<USGAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor));

		if (SagaASC)
		{
			// 绘制标题 / Draw title
			Canvas->SetDrawColor(FColor::Yellow);
			Canvas->DrawText(GEngine->GetMediumFont(), TEXT("=== Saga Meters ==="), 10.0f, YPos, 1.2f, 1.2f);
			YPos += 25.0f;

			// 绘制所有Meter的进度条 / Draw progress bars for all meters
			SagaASC->DrawMeterProgressBars(Canvas, YPos);
		}
	}
}

void USGAbilitySystemComponent::AddSpawnedAttribute(UAttributeSet* AttributeSet)
{
	if (!IsValid(AttributeSet))
	{
		return;
	}

	if (GetSpawnedAttributes().Find(AttributeSet) == INDEX_NONE)
	{
		GetAttributeSetAddOrRemoveDelegate(AttributeSet->GetClass()).Broadcast(AttributeSet, true);
	}

	Super::AddSpawnedAttribute(AttributeSet);

	UpdateShouldTick();
}

void USGAbilitySystemComponent::RemoveSpawnedAttribute(UAttributeSet* AttributeSet)
{
	if(GetSpawnedAttributes().Contains(AttributeSet))
	{
		GetAttributeSetAddOrRemoveDelegate(AttributeSet->GetClass()).Broadcast(AttributeSet, false);
	}
	Super::RemoveSpawnedAttribute(AttributeSet);
	
	UpdateShouldTick();
}

void USGAbilitySystemComponent::RemoveAllSpawnedAttributes()
{
	for (UAttributeSet* AttributeSet : GetSpawnedAttributes())
	{
		GetAttributeSetAddOrRemoveDelegate(AttributeSet->GetClass()).Broadcast(AttributeSet, false);

	}
	Super::RemoveAllSpawnedAttributes();

	UpdateShouldTick();
}

void USGAbilitySystemComponent::OnRep_SpawnedAttributes(const TArray<UAttributeSet*>& PreviousSpawnedAttributes)
{
	if (IsUsingRegisteredSubObjectList())
	{
		// Find the attributes that got removed
		for (UAttributeSet* PreviousAttributeSet : PreviousSpawnedAttributes)
		{
			if (PreviousAttributeSet)
			{
				const bool bWasRemoved = GetSpawnedAttributes().Find(PreviousAttributeSet) == INDEX_NONE;
				if (bWasRemoved)
				{
					GetAttributeSetAddOrRemoveDelegate(PreviousAttributeSet->GetClass()).Broadcast(PreviousAttributeSet, false);
					UpdateShouldTick();
				}
			}
		}

		// Find the attributes that got added
		for (UAttributeSet* NewAttributeSet : GetSpawnedAttributes())
		{
			if (IsValid(NewAttributeSet))
			{
				const bool bIsAdded = PreviousSpawnedAttributes.Find(NewAttributeSet) == INDEX_NONE;
				if (bIsAdded)
				{
					GetAttributeSetAddOrRemoveDelegate(NewAttributeSet->GetClass()).Broadcast(NewAttributeSet, true);
					UpdateShouldTick();
				}
			}
		}
	}
	
	Super::OnRep_SpawnedAttributes(PreviousSpawnedAttributes);

}

void USGAbilitySystemComponent::DrawMeterProgressBars(UCanvas* Canvas, float& YPos)
{
	/**
	 * ===================================================================================
	 * SagaStats Meter Debug Display System / SagaStats计量条调试显示系统
	 * ===================================================================================
	 *
	 * 【系统概述 / System Overview】
	 * 为SagaStats插件提供实时的Meter状态可视化调试工具，支持UDecreaseMeter和UIncreaseMeter。
	 * Real-time Meter status visualization debug tool for SagaStats plugin, supporting UDecreaseMeter and UIncreaseMeter.
	 *
	 * 【整体布局设计 / Overall Layout Design】
	 * ┌──────────────────────────────────────────────────────────────────────────┐
	 * │ Health   ████████████████████░░ 100/150 (+5.0,CD:2.3/3.0)  (70px)       │
	 * │ State: [Normal] [Lock 1.2/2.0] [Reset]                                  │
	 * │    [青色]Regen: 5.0, RegenCD: 3.0, [青色]LockDur: 2.0,                 │
	 * │    ResetRate: 20.0, [青色]ImmuneThresh: 10.0                           │
	 * │                                                                          │
	 * │ Mana     ████████████████████████████ 150/150 (-1.0,CD:4.2/5.0) (45px)  │
	 * │    Degen: 1.0, [青色]DegenCD: 5.0                                      │
	 * │                                                                          │
	 * │ Shield   ██░░░░░░░░░░░░░░░░░░░  25/100 (Reset 10.0/s)      (70px)      │
	 * │ State: [Normal] [Lock] [Reset 10.0/s]                                   │
	 * │    Regen: 10.0, RegenCD: 1.0, LockDur: 0.0,                           │
	 * │    [青色]ResetRate: 10.0, ImmuneThresh: 0.0                            │
	 * └──────────────────────────────────────────────────────────────────────────┘
	 *    ↑        ↑                          ↑
	 *  Meter名称  进度条(10px高)          数值+状态信息
	 *  (简洁)    (200px宽,紧凑布局)       (新格式显示)
	 *    ↑
	 *  状态行(仅DecreaseMeter显示，State:后接三个状态标签，当前状态特殊着色)
	 *    ↑
	 *  属性信息行(多彩文字+黑色阴影，缩进30px，智能换行，高亮关键字段)
	 *
	 * 【自适应布局 / Adaptive Layout】
	 * - DecreaseMeter: 70px行间距(包含状态行) / 70px row spacing (with state indicator)
	 * - IncreaseMeter: 45px行间距(无状态行，更紧凑) / 45px row spacing (no state indicator, more compact) 
	 *
	 * 【核心功能 / Core Features】
	 * ✅ 智能Meter名称格式化 (移除前后缀)
	 * ✅ 独立状态指示器行 (仅DecreaseMeter，清晰展示Normal/Lock/Reset三个状态)
	 * ✅ 状态驱动的颜色编码 (状态标签根据是否激活变色)
	 * ✅ 实时CD计时器显示 (精确到0.1秒，显示剩余时间/总时间)
	 * ✅ 关键属性智能高亮 (根据状态动态高亮相关字段)
	 * ✅ 高对比度文字渲染 (黑色阴影+彩色文字)
	 * ✅ 免疫状态可视化 (灰色进度条)
	 * ✅ 自适应布局设计 (根据Meter类型动态调整间距)
	 * ✅ MeterState概念可视化 (三个状态按钮式展示，当前状态高亮)
	 * ✅ 类型优化显示 (IncreaseMeter无状态行，节省垂直空间)
	 *
	 * 【Meter名称处理 / Meter Name Processing】
	 * ├─ 自动格式化: 移除"Meter_"前缀和"_C"后缀
	 * ├─ 简洁显示: "Health"、"Mana"、"Shield"
	 * ├─ 示例转换: "SagaMeter_Health_C" → "Health"
	 * └─ 阴影效果: 黑色阴影增强文字对比度
	 *
	 * 【状态指示器系统 / State Indicator System】
	 * ├─ 适用范围: 仅UDecreaseMeter显示(IncreaseMeter无MeterState概念)
	 * ├─ 布局: "State: [状态1] [状态2] [状态3]"
	 * ├─ 三个状态标签: Normal、Lock、Reset
	 * ├─ 颜色编码:
	 * │  ├─ 当前状态: Normal=绿色, Lock=红色, Reset=橙色
	 * │  └─ 非当前状态: 灰色
	 * ├─ 详细信息显示:
	 * │  ├─ Normal: "[Normal]" (简洁显示)
	 * │  ├─ Lock: "[Lock 1.2/2.0]" (显示剩余/总时间，如有计时器)
	 * │  └─ Reset: "[Reset 10.0/s]" (显示重置速率)
	 * └─ 位置: Meter名称下方15px，属性信息上方12px
	 *
	 * 【进度条系统 / Progress Bar System】
	 * ├─ 尺寸规格: 200px × 10px (优化后的紧凑设计)
	 * ├─ 边框样式: 0.5px白色边框
	 * ├─ 背景颜色: 深灰色(40,40,40,200)
	 * ├─ 填充逻辑: 当前值/最大值百分比
	 * └─ 免疫显示: Reset状态+免疫阈值时显示灰色
	 *
	 * 【数值显示系统 / Value Display System】
	 * ├─ 位置布局: 进度条右侧10px偏移
	 * ├─ 字体设置: SmallFont 1.0x缩放
	 * ├─ 颜色方案: 黑色阴影+白色文字
	 * ├─ 新格式规则: "当前值/最大值 (速率信息,CD:剩余/总时间)"
	 * └─ CD信息: 根据Meter类型和状态动态生成
	 *
	 * 【CD信息生成规则 / CD Info Generation Rules】
	 * UDecreaseMeter (新格式):
	 *   ├─ 冷却中: "(+5.0,CD:2.3/3.0)" - 显示恢复速率和冷却进度
	 *   ├─ 锁定中: "Lock 1.2/2.0" - 显示锁定剩余/总时间
	 *   ├─ Normal状态: "+X.X/s" (恢复速率，当regen>0时)
	 *   ├─ Reset状态: "Reset X.X/s" (重置速率)
	 *   └─ Lock状态: "Locked" (无计时器时)
	 *
	 * UIncreaseMeter (新格式):
	 *   ├─ 冷却中: "(-1.0,CD:4.2/5.0)" - 显示衰减速率和冷却进度
	 *   └─ 正常衰减: "-X.X/s" (衰减速率)
	 *
	 * 【属性信息系统 / Attribute Info System】
	 * ├─ 智能布局: 每个Meter下方，缩进30px
	 * ├─ 多彩渲染: 彩色文字+黑色阴影(1px偏移)
	 * ├─ 智能高亮: 根据当前状态动态高亮相关字段(青色)
	 * │  ├─ Normal状态: 高亮Regen和RegenCD(如果活跃)
	 * │  ├─ Lock状态: 高亮LockDur字段
	 * │  ├─ Reset状态: 高亮ResetRate和ImmuneThresh(如果免疫)
	 * │  └─ IncreaseMeter: 高亮Degen和DegenCD(如果活跃)
	 * ├─ 多行换行: 自动计算宽度，智能换行
	 * ├─ 行高设置: 12px行间距
	 * ├─ 属性精简: 移除Maximum和bClear字段显示
	 * └─ 格式示例: "[青色]Regen: 5.0, [青色]RegenCD: 3.0, LockDur: 2.0"
	 *
	 * 【布局参数配置 / Layout Parameters】
	 * ├─ 自适应间距 / Adaptive spacing:
	 * │  ├─ DecreaseMeter: 70px行间距(容纳状态行+多行属性)
	 * │  └─ IncreaseMeter: 45px行间距(无状态行，更紧凑)
	 * ├─ 边距设置: 20px左边距
	 * ├─ 名称区域: 80px宽度(更紧凑)
	 * ├─ 进度条区域: 200px宽度(优化尺寸)
	 * ├─ 数值区域: 10px间距偏移
	 * ├─ 状态指示器(仅DecreaseMeter): Meter名称下方15px，缩进20px
	 * ├─ 属性区域:
	 * │  ├─ DecreaseMeter: 状态行下方12px(27.0f总偏移)，缩进30px
	 * │  └─ IncreaseMeter: 进度条下方15px(15.0f总偏移)，缩进30px
	 * ├─ 属性显示宽度: 420px最大宽度
	 * └─ 字符计算: 6px/字符用于换行计算
	 *
	 * 【技术实现要点 / Technical Implementation】
	 * ├─ friend类访问: 允许访问protected计时器成员
	 * ├─ 实时计时器: GetTimerManager().GetTimerRemaining()
	 * ├─ 类型检测: Cast<UDecreaseMeter/UIncreaseMeter>()动态判断类型
	 * ├─ 自适应布局: 根据Meter类型动态计算行间距和属性位置
	 * ├─ 累积偏移量: 使用AccumulatedYOffset支持混合类型的Meter列表
	 * ├─ 渲染优化: 先绘制阴影后绘制文字
	 * ├─ 多色文本: DrawMultiColorLine函数支持单行多色渲染
	 * ├─ 动态高亮: 基于状态和条件的智能颜色选择
	 * ├─ 内存安全: 所有指针都进行nullptr检查
	 * ├─ 早期退出: DrawMeterStateIndicator对非DecreaseMeter提前返回
	 * └─ 精度控制: 浮点数格式化保留1位小数
	 *
	 * 【使用方法 / Usage Instructions】
	 * 游戏内命令: showdebug Meter
	 * 注册方式: USGAbilitySystemComponent::OnShowMeterDebugInfo
	 * 依赖条件: 需要有效的USagaAbilitySystemComponent和Meter实例
	 * ===================================================================================
	 */

	/* ✅ 所有功能需求已完成实现 / All Feature Requirements Completed
	 * ✅ regen/CD功能表现已更新为`(+/-xxx,CD:剩余时间/总时间)`格式
	 * ✅ Lock状态显示已更新为`[Lock 剩余时间/总时间]`格式
	 * ✅ Reset状态显示已更新为`[Reset ResetRate/s]`格式
	 * ✅ 关键字段`LockDur/ResetRate/ImmuneThreshold`已实现智能高亮显示
	 * ✅ 进度条尺寸已调整为宽度200px，高度10px
	 * ✅ Maximum和bClear字段显示已移除
	 * ✅ MeterName已添加阴影效果增强可读性
	 */
	
	
	if (!Canvas)
	{
		return;
	}

	// 获取所有Meter实例 / Get all meter instances
	TArray<UMeterBase*> Meters = GetAllMeters();

	if (Meters.Num() == 0)
	{
		// 显示无Meter信息 / Show no meters message
		Canvas->SetDrawColor(FColor::White);
		Canvas->DrawText(GEngine->GetSmallFont(), TEXT("No meters found"), 20.0f, YPos, 1.0f, 1.0f);
		YPos += 20.0f;
		return;
	}

	const float ProgressBarWidth = 200.0f;
	const float ProgressBarHeight = 10.0f; // bar宽度200，高度10 / Bar width 200, height 10
	const float DecreaseRowSpacing = 70.0f; // DecreaseMeter行间距(容纳状态行+多行属性) / DecreaseMeter row spacing (with state indicator + multi-line attributes)
	const float IncreaseRowSpacing = 45.0f; // IncreaseMeter行间距(更紧凑，无状态行) / IncreaseMeter row spacing (more compact, no state indicator)
	const float LeftMargin = 20.0f;
	const float AttributeInfoIndent = 30.0f; // 属性信息缩进 / Attribute info indent

	float AccumulatedYOffset = 0.0f; // 累积的Y偏移量 / Accumulated Y offset

	for (int32 i = 0; i < Meters.Num(); ++i)
	{
		UMeterBase* Meter = Meters[i];
		if (!Meter)
		{
			continue;
		}

		// 判断Meter类型并决定布局 / Determine meter type and decide layout
		const bool bIsDecreaseMeter = Cast<UDecreaseMeter>(Meter) != nullptr;
		const float CurrentRowSpacing = bIsDecreaseMeter ? DecreaseRowSpacing : IncreaseRowSpacing;

		float CurrentYPos = YPos + AccumulatedYOffset;

		// 绘制Meter名称（不再包含状态标记） / Draw meter name (no longer includes state marker)
		FString MeterName = GetFormattedMeterName(Meter);

		// 先绘制黑色阴影增强对比度 / Draw black shadow first for better contrast
		Canvas->SetDrawColor(FColor::Black);
		Canvas->DrawText(GEngine->GetSmallFont(), MeterName, LeftMargin + 1.0f, CurrentYPos + 1.0f, 1.0f, 1.0f);

		// 再绘制白色名称文字 / Then draw white name text
		Canvas->SetDrawColor(FColor::White);
		Canvas->DrawText(GEngine->GetSmallFont(), MeterName, LeftMargin, CurrentYPos, 1.0f, 1.0f);

		// 绘制进度条 / Draw progress bar
		FVector2D ProgressPosition(LeftMargin + 80.0f, CurrentYPos);
		FVector2D ProgressSize(ProgressBarWidth, ProgressBarHeight);
		DrawSingleMeterProgress(Canvas, Meter, ProgressPosition, ProgressSize);

		// 根据Meter类型决定属性位置 / Determine attribute position based on meter type
		float AttributeYOffset;

		if (bIsDecreaseMeter)
		{
			// DecreaseMeter: 绘制状态指示器，然后绘制属性 / DecreaseMeter: draw state indicator, then attributes
			FVector2D StatePosition(LeftMargin, CurrentYPos + 15.0f);
			DrawMeterStateIndicator(Canvas, Meter, StatePosition);
			AttributeYOffset = 27.0f; // 状态行下方12px / 12px below state line
		}
		else
		{
			// IncreaseMeter: 直接绘制属性，更紧凑 / IncreaseMeter: draw attributes directly, more compact
			AttributeYOffset = 15.0f; // 进度条下方15px / 15px below progress bar
		}

		// 绘制属性信息 / Draw attribute info
		FVector2D AttributePosition(LeftMargin + AttributeInfoIndent, CurrentYPos + AttributeYOffset);
		float MaxAttributeWidth = ProgressBarWidth + 220.0f; // 更宽的显示区域 / Wider display area
		DrawMeterAttributeInfo(Canvas, Meter, AttributePosition, MaxAttributeWidth);

		// 累积Y偏移 / Accumulate Y offset
		AccumulatedYOffset += CurrentRowSpacing;
	}

	// 更新YPos用于后续绘制 / Update YPos for subsequent drawing
	YPos += AccumulatedYOffset + 10.0f;
}

void USGAbilitySystemComponent::DrawSingleMeterProgress(UCanvas* Canvas, UMeterBase* Meter, FVector2D Position, FVector2D Size)
{
	if (!Canvas || !Meter)
	{
		return;
	}

	// 获取Meter的当前值和最大值 / Get current and max values
	float CurrentValue = Meter->GetCurrent();
	float MaxValue = Meter->GetMaximum();
	float Progress = MaxValue > 0 ? FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f) : 0.0f;

	// 绘制背景（深灰色）/ Draw background (dark gray)
	Canvas->SetDrawColor(FColor(40, 40, 40, 200));
	Canvas->DrawTile(Canvas->DefaultTexture, Position.X, Position.Y, Size.X, Size.Y, 0, 0, 1, 1);

	// 根据Immune状态和Meter类型获取进度条颜色 / Get progress bar color based on immune status and meter type
	FColor ProgressColor = GetMeterProgressBarColor(Meter);
	Canvas->SetDrawColor(ProgressColor);
	float ProgressWidth = Size.X * Progress;
	Canvas->DrawTile(Canvas->DefaultTexture, Position.X, Position.Y, ProgressWidth, Size.Y, 0, 0, 1, 1);

	// 绘制边框 / Draw border
	DrawBorder(Canvas, Position, Size, 0.5f); // 使用更细的边框适应较小的进度条 / Use thinner border for smaller progress bar

	// 获取CD信息用于显示在括号内 / Get CD info for display in brackets
	FString CDInfo = GetMeterCDInfo(Meter);

	// 绘制数值文本 / Draw value text
	FString ValueText;
	if (!CDInfo.IsEmpty())
	{
		ValueText = FString::Printf(TEXT("%.1f/%.1f (%s)"), CurrentValue, MaxValue, *CDInfo);
	}
	else
	{
		ValueText = FString::Printf(TEXT("%.1f/%.1f"), CurrentValue, MaxValue);
	}

	// 将文字放在进度条右边 / Place text to the right of progress bar
	float TextX = Position.X + Size.X + 10.0f;
	float TextY = Position.Y + (Size.Y * 0.5f) - 6.0f; // 垂直居中对齐 / Vertically center align

	// 先绘制黑色阴影增强对比度 / Draw black shadow first for better contrast
	Canvas->SetDrawColor(FColor::Black);
	Canvas->DrawText(GEngine->GetSmallFont(), ValueText, TextX + 1.0f, TextY + 1.0f, 1.0f, 1.0f);

	// 再绘制白色文字 / Then draw white text
	Canvas->SetDrawColor(FColor::White);
	Canvas->DrawText(GEngine->GetSmallFont(), ValueText, TextX, TextY, 1.0f, 1.0f);
}


void USGAbilitySystemComponent::DrawBorder(UCanvas* Canvas, FVector2D Position, FVector2D Size, float BorderWidth)
{
	if (!Canvas)
	{
		return;
	}

	Canvas->SetDrawColor(FColor::White);

	// 上边框 / Top border
	Canvas->DrawTile(Canvas->DefaultTexture, Position.X, Position.Y, Size.X, BorderWidth, 0, 0, 1, 1);
	// 下边框 / Bottom border
	Canvas->DrawTile(Canvas->DefaultTexture, Position.X, Position.Y + Size.Y - BorderWidth, Size.X, BorderWidth, 0, 0, 1, 1);
	// 左边框 / Left border
	Canvas->DrawTile(Canvas->DefaultTexture, Position.X, Position.Y, BorderWidth, Size.Y, 0, 0, 1, 1);
	// 右边框 / Right border
	Canvas->DrawTile(Canvas->DefaultTexture, Position.X + Size.X - BorderWidth, Position.Y, BorderWidth, Size.Y, 0, 0, 1, 1);
}

TArray<UMeterBase*> USGAbilitySystemComponent::GetAllMeters() const
{
	TArray<UMeterBase*> Meters;

	// 遍历所有AttributeSet，查找Meter类型 / Iterate through all AttributeSets to find meter types
	for (const UAttributeSet* AttributeSet : GetSpawnedAttributes())
	{
		if (const UMeterBase* Meter = Cast<UMeterBase>(AttributeSet))
		{
			Meters.Add(const_cast<UMeterBase*>(Meter));
		}
	}

	// 按类名排序以保持一致的显示顺序 / Sort by class name for consistent display order
	Meters.Sort([](const UMeterBase& A, const UMeterBase& B)
	{
		return A.GetClass()->GetName() < B.GetClass()->GetName();
	});

	return Meters;
}

FColor USGAbilitySystemComponent::GetMeterNameColor(UMeterBase* Meter)
{
	if (!Meter)
	{
		return FColor::White;
	}

	// 尝试转换为UDecreaseMeter以获取状态 / Try to cast to UDecreaseMeter to get state
	if (const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter))
	{
		switch (DecreaseMeter->MeterState)
		{
		case EMeterState::Normal:
			return FColor::Green;   // 绿色 / Green
		case EMeterState::Lock:
			return FColor::Red;     // 红色 / Red
		case EMeterState::Reset:
			return FColor::Orange;  // 橙色 / Orange
		default:
			return FColor::Green;
		}
	}

	// 对于其他类型的Meter，使用默认颜色 / For other meter types, use default color
	return FColor::Green;
}

FColor USGAbilitySystemComponent::GetMeterProgressBarColor(UMeterBase* Meter)
{
	if (!Meter)
	{
		return FColor::Green;
	}

	// 检查是否为UDecreaseMeter并处于免疫状态 / Check if it's UDecreaseMeter and in immune state
	if (const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter))
	{
		// 检查是否处于Reset状态且在免疫阈值内 / Check if in Reset state and within immune threshold
		if (DecreaseMeter->MeterState == EMeterState::Reset)
		{
			float ImmuneThreshold = DecreaseMeter->GetImmuneThreshold();
			float CurrentValue = DecreaseMeter->GetCurrent();

			// 如果免疫阈值<0，整个Reset状态免疫 / If immune threshold < 0, immune throughout Reset state
			if (ImmuneThreshold < 0.0f || CurrentValue < ImmuneThreshold)
			{
				return FColor(128, 128, 128); // 灰色 / Gray
			}
		}
	}

	// 非免疫状态下的颜色 / Color when not immune
	return FColor::Green; // 绿色 / Green
}

void USGAbilitySystemComponent::DrawMeterRecoveryInfo(UCanvas* Canvas, UMeterBase* Meter, FVector2D Position)
{
	if (!Canvas || !Meter)
	{
		return;
	}

	FString RecoveryText;
	bool bHasRecoveryInfo = false;

	// 处理UDecreaseMeter的恢复信息 / Handle UDecreaseMeter recovery info
	if (const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter))
	{
		// 优先检查计时器状态 / Priority check timer states
		if (DecreaseMeter->RegenerationCooldownTimer.IsValid())
		{
			float RemainingTime = DecreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(DecreaseMeter->RegenerationCooldownTimer);
			if (RemainingTime > 0.0f)
			{
				RecoveryText = FString::Printf(TEXT("CD: %.1fs"), RemainingTime);
				bHasRecoveryInfo = true;
			}
		}
		else if (DecreaseMeter->LockStateTimer.IsValid())
		{
			float RemainingTime = DecreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(DecreaseMeter->LockStateTimer);
			if (RemainingTime > 0.0f)
			{
				RecoveryText = FString::Printf(TEXT("Lock: %.1fs"), RemainingTime);
				bHasRecoveryInfo = true;
			}
		}
		// 如果没有活跃计时器，根据状态显示相应信息 / If no active timers, show info based on state
		else
		{
			switch (DecreaseMeter->MeterState)
			{
			case EMeterState::Normal:
				if (DecreaseMeter->GetRegeneration() > 0.0f)
				{
					RecoveryText = FString::Printf(TEXT("+%.1f/s"), DecreaseMeter->GetRegeneration());
					bHasRecoveryInfo = true;
				}
				break;
			case EMeterState::Lock:
				RecoveryText = TEXT("Locked");
				bHasRecoveryInfo = true;
				break;
			case EMeterState::Reset:
				if (DecreaseMeter->GetResetRate() > 0.0f)
				{
					RecoveryText = FString::Printf(TEXT("Reset: +%.1f/s"), DecreaseMeter->GetResetRate());
					bHasRecoveryInfo = true;
				}
				break;
			}
		}
	}
	// 处理UIncreaseMeter的衰减信息 / Handle UIncreaseMeter degeneration info
	else if (const UIncreaseMeter* IncreaseMeter = Cast<UIncreaseMeter>(Meter))
	{
		// 优先检查衰减冷却计时器 / Priority check degeneration cooldown timer
		if (IncreaseMeter->DegenerationCooldownTimer.IsValid())
		{
			float RemainingTime = IncreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(IncreaseMeter->DegenerationCooldownTimer);
			if (RemainingTime > 0.0f)
			{
				RecoveryText = FString::Printf(TEXT("CD: %.1fs"), RemainingTime);
				bHasRecoveryInfo = true;
			}
		}
		// 如果没有活跃计时器，检查是否可以衰减 / If no active timer, check if can degenerate
		else if (IncreaseMeter->GetDegeneration() > 0.0f)
		{
			RecoveryText = FString::Printf(TEXT("-%.1f/s"), IncreaseMeter->GetDegeneration());
			bHasRecoveryInfo = true;
		}
	}

	// 绘制恢复信息文本 / Draw recovery info text
	if (bHasRecoveryInfo)
	{
		// 先绘制黑色阴影 / Draw black shadow first
		Canvas->SetDrawColor(FColor::Black);
		Canvas->DrawText(GEngine->GetSmallFont(), RecoveryText, Position.X + 1.0f, Position.Y + 3.0f, 1.0f, 1.0f);

		// 再绘制青色文字 / Then draw cyan text
		Canvas->SetDrawColor(FColor::Cyan);
		Canvas->DrawText(GEngine->GetSmallFont(), RecoveryText, Position.X, Position.Y + 2.0f, 1.0f, 1.0f);
	}
}

FString USGAbilitySystemComponent::GetMeterCDInfo(UMeterBase* Meter)
{
	if (!Meter)
	{
		return FString();
	}

	// 处理UDecreaseMeter的CD信息 / Handle UDecreaseMeter CD info
	if (const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter))
	{
		// 优先检查计时器状态 / Priority check timer states
		if (DecreaseMeter->RegenerationCooldownTimer.IsValid())
		{
			float RemainingTime = DecreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(DecreaseMeter->RegenerationCooldownTimer);
			if (RemainingTime > 0.0f)
			{
				float TotalTime = DecreaseMeter->GetRegenerationCooldown();
				FString RegenInfo;
				if (DecreaseMeter->GetRegeneration() > 0.0f)
				{
					RegenInfo = FString::Printf(TEXT("+%.1f,"), DecreaseMeter->GetRegeneration());
				}
				return FString::Printf(TEXT("%sCD:%.1f/%.1f"), *RegenInfo, RemainingTime, TotalTime);
			}
		}
		else if (DecreaseMeter->LockStateTimer.IsValid())
		{
			float RemainingTime = DecreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(DecreaseMeter->LockStateTimer);
			if (RemainingTime > 0.0f)
			{
				float TotalTime = DecreaseMeter->GetLockDuration();
				return FString::Printf(TEXT("Lock %.1f/%.1f"), RemainingTime, TotalTime);
			}
		}
		// 如果没有活跃计时器，根据状态显示相应信息 / If no active timers, show info based on state
		else
		{
			switch (DecreaseMeter->MeterState)
			{
			case EMeterState::Normal:
				if (DecreaseMeter->GetRegeneration() > 0.0f)
				{
					return FString::Printf(TEXT("+%.1f/s"), DecreaseMeter->GetRegeneration());
				}
				break;
			case EMeterState::Lock:
				// Lock状态但没有计时器，表示永久锁定 / Lock state but no timer means permanent lock
				return TEXT("Locked");
			case EMeterState::Reset:
				if (DecreaseMeter->GetResetRate() > 0.0f)
				{
					return FString::Printf(TEXT("Reset %.1f/s"), DecreaseMeter->GetResetRate());
				}
				break;
			}
		}
	}
	// 处理UIncreaseMeter的CD信息 / Handle UIncreaseMeter CD info
	else if (const UIncreaseMeter* IncreaseMeter = Cast<UIncreaseMeter>(Meter))
	{
		// 优先检查衰减冷却计时器 / Priority check degeneration cooldown timer
		if (IncreaseMeter->DegenerationCooldownTimer.IsValid())
		{
			float RemainingTime = IncreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(IncreaseMeter->DegenerationCooldownTimer);
			if (RemainingTime > 0.0f)
			{
				float TotalTime = IncreaseMeter->GetDegenerationCooldown();
				FString DegenInfo;
				if (IncreaseMeter->GetDegeneration() > 0.0f)
				{
					DegenInfo = FString::Printf(TEXT("-%.1f,"), IncreaseMeter->GetDegeneration());
				}
				return FString::Printf(TEXT("%sCD:%.1f/%.1f"), *DegenInfo, RemainingTime, TotalTime);
			}
		}
		// 如果没有活跃计时器，检查是否可以衰减 / If no active timer, check if can degenerate
		else if (IncreaseMeter->GetDegeneration() > 0.0f)
		{
			return FString::Printf(TEXT("-%.1f/s"), IncreaseMeter->GetDegeneration());
		}
	}

	return FString(); // 返回空字符串表示没有CD信息 / Return empty string if no CD info
}

FString USGAbilitySystemComponent::GetFormattedMeterName(UMeterBase* Meter)
{
	if (!Meter)
	{
		return FString();
	}

	// 获取原始类名 / Get original class name
	FString MeterName = Meter->GetClass()->GetName();

	// 去掉 "Meter_" 前缀 / Remove "Meter_" prefix
	if (MeterName.StartsWith(TEXT("Meter_")))
	{
		MeterName = MeterName.RightChop(6); // Remove "Meter_"
	}

	// 去掉 "_C" 后缀 / Remove "_C" suffix
	if (MeterName.EndsWith(TEXT("_C")))
	{
		MeterName = MeterName.LeftChop(2); // Remove "_C"
	}

	// 不再添加状态指示器，保持名称简洁 / No longer add state indicators, keep name concise
	return MeterName;
}

FString USGAbilitySystemComponent::GetMeterAttributeInfo(UMeterBase* Meter)
{
	if (!Meter)
	{
		return FString();
	}

	TArray<FString> AttributeStrings;

	// 删除Maximum和bClear字段显示 / Remove Maximum and bClear field display

	// UDecreaseMeter特有属性 / UDecreaseMeter specific attributes
	if (const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter))
	{
		AttributeStrings.Add(FString::Printf(TEXT("Regen: %.1f"), DecreaseMeter->GetRegeneration()));
		AttributeStrings.Add(FString::Printf(TEXT("RegenCD: %.1f"), DecreaseMeter->GetRegenerationCooldown()));
		AttributeStrings.Add(FString::Printf(TEXT("LockDur: %.1f"), DecreaseMeter->GetLockDuration()));
		AttributeStrings.Add(FString::Printf(TEXT("ResetRate: %.1f"), DecreaseMeter->GetResetRate()));
		AttributeStrings.Add(FString::Printf(TEXT("ImmuneThresh: %.1f"), DecreaseMeter->GetImmuneThreshold()));
	}
	// UIncreaseMeter特有属性 / UIncreaseMeter specific attributes
	else if (const UIncreaseMeter* IncreaseMeter = Cast<UIncreaseMeter>(Meter))
	{
		AttributeStrings.Add(FString::Printf(TEXT("Degen: %.1f"), IncreaseMeter->GetDegeneration()));
		AttributeStrings.Add(FString::Printf(TEXT("DegenCD: %.1f"), IncreaseMeter->GetDegenerationCooldown()));
	}

	// 用逗号连接所有属性 / Join all attributes with commas
	return FString::Join(AttributeStrings, TEXT(", "));
}

void USGAbilitySystemComponent::DrawMeterAttributeInfo(UCanvas* Canvas, UMeterBase* Meter, FVector2D Position, float MaxWidth)
{
	if (!Canvas || !Meter)
	{
		return;
	}

	// 获取属性信息数组，包含颜色信息 / Get attribute info array with color information
	TArray<TPair<FString, FColor>> AttributeStrings;

	//获取选中状态颜色
	auto GetAttributeDrawColor = [](bool Condition){return Condition ? FColor::Cyan: FColor::White;};
	
	// UDecreaseMeter特有属性 / UDecreaseMeter specific attributes
	if (const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter))
	{
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("Regen: %.1f"), DecreaseMeter->GetRegeneration()), GetAttributeDrawColor(DecreaseMeter->MeterState == EMeterState::Normal && DecreaseMeter->CanRegeneration())));
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("RegenCD: %.1f"), DecreaseMeter->GetRegenerationCooldown()), GetAttributeDrawColor(DecreaseMeter->MeterState == EMeterState::Normal && DecreaseMeter->RegenerationCooldownTimer.IsValid())));

		// 高亮LockDur字段 / Highlight LockDur field
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("LockDur: %.1f"), DecreaseMeter->GetLockDuration()), GetAttributeDrawColor(DecreaseMeter->MeterState == EMeterState::Lock) ));

		// 高亮ResetRate字段 / Highlight ResetRate field
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("ResetRate: %.1f"), DecreaseMeter->GetResetRate()), GetAttributeDrawColor(DecreaseMeter->MeterState == EMeterState::Reset) ));

		// 高亮ImmuneThreshold字段 / Highlight ImmuneThreshold field
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("ImmuneThresh: %.1f"), DecreaseMeter->GetImmuneThreshold()), GetAttributeDrawColor(DecreaseMeter->IsInResetImmune())));
	}
	// UIncreaseMeter特有属性 / UIncreaseMeter specific attributes
	else if (const UIncreaseMeter* IncreaseMeter = Cast<UIncreaseMeter>(Meter))
	{
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("Degen: %.1f"), IncreaseMeter->GetDegeneration()), GetAttributeDrawColor(IncreaseMeter->CanDegeneration())));
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("DegenCD: %.1f"), IncreaseMeter->GetDegenerationCooldown()), GetAttributeDrawColor(IncreaseMeter->DegenerationCooldownTimer.IsValid())));
	}

	// 设置字体 / Set font
	UFont* SmallFont = GEngine->GetSmallFont();

	// 计算每行能容纳的字符数 / Calculate characters per line
	float CharWidth = 6.0f; // SmallFont大约字符宽度 / Approximate SmallFont character width
	int32 MaxCharsPerLine = FMath::Max(1, (int32)(MaxWidth / CharWidth));

	// 构建多行显示 / Build multi-line display
	TArray<TPair<FString, FColor>> CurrentLine;
	float CurrentYPos = Position.Y;
	const float LineHeight = 12.0f; // 行高 / Line height

	for (int32 i = 0; i < AttributeStrings.Num(); ++i)
	{
		TPair<FString, FColor> AttributePair = AttributeStrings[i];

		// 计算当前行的长度 / Calculate current line length
		int32 CurrentLineLength = 0;
		for (const auto& Pair : CurrentLine)
		{
			CurrentLineLength += Pair.Key.Len() + 2; // +2 for ", "
		}
		CurrentLineLength += AttributePair.Key.Len();

		// 检查是否需要换行 / Check if line break is needed
		if (CurrentLineLength > MaxCharsPerLine && CurrentLine.Num() > 0)
		{
			// 绘制当前行 / Draw current line
			DrawMultiColorLine(Canvas, Position.X, CurrentYPos, CurrentLine, SmallFont);
			CurrentYPos += LineHeight;
			CurrentLine.Empty(); // 清空当前行开始新行 / Clear current line to start new line
		}

		CurrentLine.Add(AttributePair);
	}

	// 绘制最后一行 / Draw the last line
	if (CurrentLine.Num() > 0)
	{
		DrawMultiColorLine(Canvas, Position.X, CurrentYPos, CurrentLine, SmallFont);
	}
}

void USGAbilitySystemComponent::DrawMultiColorLine(UCanvas* Canvas, float StartX, float YPos, const TArray<TPair<FString, FColor>>& ColoredTexts, UFont* Font)
{
	if (!Canvas || !Font || ColoredTexts.Num() == 0)
	{
		return;
	}

	float CurrentX = StartX;

	for (int32 i = 0; i < ColoredTexts.Num(); ++i)
	{
		const TPair<FString, FColor>& ColoredText = ColoredTexts[i];
		FString DisplayText = ColoredText.Key;

		// 添加分隔符（除了第一个元素） / Add separator (except for first element)
		if (i > 0)
		{
			// 绘制分隔符阴影 / Draw separator shadow
			Canvas->SetDrawColor(FColor::Black);
			Canvas->DrawText(Font, TEXT(", "), CurrentX + 1.0f, YPos + 1.0f, 1.0f, 1.0f);
			// 绘制分隔符 / Draw separator
			Canvas->SetDrawColor(FColor::White);
			Canvas->DrawText(Font, TEXT(", "), CurrentX, YPos, 1.0f, 1.0f);
			CurrentX += 12.0f; // ", " 的大约宽度 / Approximate width of ", "
		}

		// 绘制文本阴影 / Draw text shadow
		Canvas->SetDrawColor(FColor::Black);
		Canvas->DrawText(Font, DisplayText, CurrentX + 1.0f, YPos + 1.0f, 1.0f, 1.0f);

		// 绘制彩色文本 / Draw colored text
		Canvas->SetDrawColor(ColoredText.Value);
		Canvas->DrawText(Font, DisplayText, CurrentX, YPos, 1.0f, 1.0f);

		// 更新X位置 / Update X position
		CurrentX += DisplayText.Len() * 6.0f; // 估算字符宽度 / Estimated character width
	}
}

void USGAbilitySystemComponent::DrawMeterStateIndicator(UCanvas* Canvas, UMeterBase* Meter, FVector2D Position)
{
	/**
	 * ===================================================================================
	 * 【状态指示器系统 / State Indicator System】
	 * ===================================================================================
	 *
	 * 功能：绘制独立的状态指示器行，清晰展示DecreaseMeter的三个可能状态
	 * Function: Draw independent state indicator line, clearly showing three possible states of DecreaseMeter
	 *
	 * 重要：仅对UDecreaseMeter有效，IncreaseMeter将提前返回
	 * Important: Only works for UDecreaseMeter, IncreaseMeter will return early
	 *
	 * 布局格式 / Layout Format:
	 * "State: [Normal] [Lock 1.2/2.0] [Reset 10.0/s]"
	 *
	 * 颜色规则 / Color Rules:
	 * - 当前状态 / Current state:
	 *   - Normal: 绿色 / Green (0,255,0)
	 *   - Lock: 红色 / Red (255,0,0)
	 *   - Reset: 橙色 / Orange (255,165,0)
	 * - 非当前状态 / Inactive states: 灰色 / Gray (128,128,128)
	 *
	 * 详细信息 / Detailed Info:
	 * - Normal: 简洁显示 / Simple display "[Normal]"
	 * - Lock: 显示计时器 / Show timer "[Lock 1.2/2.0]" (剩余/总时间 / remaining/total)
	 * - Reset: 显示速率 / Show rate "[Reset 10.0/s]"
	 *
	 * ===================================================================================
	 */

	if (!Canvas || !Meter)
	{
		return;
	}

	// 只为DecreaseMeter绘制状态指示器 / Only draw state indicator for DecreaseMeter
	const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter);
	if (!DecreaseMeter)
	{
		return; // IncreaseMeter没有MeterState概念，直接返回 / IncreaseMeter has no MeterState concept, return early
	}

	UFont* SmallFont = GEngine->GetSmallFont();
	float CurrentX = Position.X;
	float CurrentY = Position.Y;

	// 绘制 "State:" 标签 / Draw "State:" label
	FString StateLabel = TEXT("State: ");

	// 阴影 / Shadow
	Canvas->SetDrawColor(FColor::Black);
	Canvas->DrawText(SmallFont, StateLabel, CurrentX + 1.0f, CurrentY + 1.0f, 1.0f, 1.0f);
	// 文字 / Text
	Canvas->SetDrawColor(FColor::White);
	Canvas->DrawText(SmallFont, StateLabel, CurrentX, CurrentY, 1.0f, 1.0f);
	CurrentX += StateLabel.Len() * 6.0f;

	// 获取当前状态 / Get current state
	EMeterState CurrentState = DecreaseMeter->MeterState;

	// 定义三个状态及其显示信息 / Define three states and their display info
	struct FStateInfo
	{
		EMeterState State;
		FString Text;
		/*FColor ActiveColor;
		FColor InactiveColor;*/
	};

	TArray<FStateInfo> States;
	
	/*
	, FColor::Cyan, FColor(128, 128, 128)
	*/
	
	// Normal 状态 / Normal state
	States.Add({EMeterState::Normal, TEXT("[Normal]")});

	// Lock 状态 / Lock state
	FString LockText = TEXT("[Lock]");
	if (DecreaseMeter && DecreaseMeter->LockStateTimer.IsValid())
	{
		float RemainingTime = DecreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(DecreaseMeter->LockStateTimer);
		float TotalTime = DecreaseMeter->GetLockDuration();
		if (RemainingTime > 0.0f)
		{
			LockText = FString::Printf(TEXT("[Lock %.1f/%.1f]"), RemainingTime, TotalTime);
		}
	}
	States.Add({EMeterState::Lock, LockText});

	// Reset 状态 / Reset state
	FString ResetText = TEXT("[Reset, ");
	
	if (DecreaseMeter)
	{
		float ResetRate = DecreaseMeter->GetResetRate();
		if (ResetRate > 0.0f)
		{
			ResetText = FString::Printf(TEXT("[Reset %.1f/s "), ResetRate);
		}
	}
	
	States.Add({EMeterState::Reset, ResetText});

	// 绘制三个状态标签 / Draw three state labels
	for (int32 i = 0; i < States.Num(); ++i)
	{
		const FStateInfo& StateInfo = States[i];

		// 添加状态间的空格 / Add spacing between states
		if (i > 0)
		{
			CurrentX += 6.0f; // 状态之间的间距 / Spacing between states
		}
		const FColor& InteractionColor = FColor(128, 128, 128);

		// 确定颜色（当前状态使用激活颜色，其他使用灰色） / Determine color (active color for current state, gray for others)
		FColor StateColor = (StateInfo.State == CurrentState) ? FColor::Cyan :  InteractionColor;

		// 绘制阴影 / Draw shadow
		Canvas->SetDrawColor(FColor::Black);
		Canvas->DrawText(SmallFont, StateInfo.Text, CurrentX + 1.0f, CurrentY + 1.0f, 1.0f, 1.0f);

		// 绘制状态文字 / Draw state text
		Canvas->SetDrawColor(StateColor);
		Canvas->DrawText(SmallFont, StateInfo.Text, CurrentX, CurrentY, 1.0f, 1.0f);

		// 更新X位置 / Update X position
		CurrentX += StateInfo.Text.Len() * 6.0f;

		// Reset状态特殊处理：添加Immune指示器 / Special handling for Reset state: add Immune indicator
		if (StateInfo.State == EMeterState::Reset && DecreaseMeter)
		{
			// 检查是否处于免疫状态 / Check if in immune state
			bool bIsImmune = DecreaseMeter->IsInResetImmune();

			// Immune文本（不包含括号） / Immune text (without bracket)
			FString ImmuneText = TEXT("(Immune)");

			// 确定Immune文本颜色（独立的颜色逻辑） / Determine Immune text color (independent color logic)
			FColor ImmuneColor;
			if (bIsImmune)
			{
				// 处于免疫状态时，使用高亮颜色 / When immune, use highlight color (yellow)
				ImmuneColor = FColor::Red;
			}
			else
			{
				// 未免疫时使用灰色 / When not immune, use gray
				ImmuneColor = InteractionColor;
			}

			// 绘制Immune阴影 / Draw Immune shadow
			Canvas->SetDrawColor(FColor::Black);
			Canvas->DrawText(SmallFont, ImmuneText, CurrentX + 1.0f, CurrentY + 1.0f, 1.0f, 1.0f);

			// 绘制Immune文字 / Draw Immune text
			Canvas->SetDrawColor(ImmuneColor);
			Canvas->DrawText(SmallFont, ImmuneText, CurrentX, CurrentY, 1.0f, 1.0f);

			// 更新X位置 / Update X position
			CurrentX += ImmuneText.Len() * 6.0f + 12;

			// 绘制闭合括号"]"，使用与Reset状态相同的颜色 / Draw closing bracket "]" with same color as Reset state
			FString ClosingBracket = TEXT("]");

			// 绘制括号阴影 / Draw bracket shadow
			Canvas->SetDrawColor(FColor::Black);
			Canvas->DrawText(SmallFont, ClosingBracket, CurrentX + 1.0f, CurrentY + 1.0f, 1.0f, 1.0f);

			// 绘制括号，使用StateColor（与Reset状态标签颜色一致） / Draw bracket with StateColor (same as Reset state label)
			Canvas->SetDrawColor(StateColor);
			Canvas->DrawText(SmallFont, ClosingBracket, CurrentX, CurrentY, 1.0f, 1.0f);

			// 更新X位置 / Update X position
			CurrentX += ClosingBracket.Len() * 6.0f;
		}
	}
}

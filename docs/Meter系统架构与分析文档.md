# SagaStats Meter系统架构与分析文档

## 项目信息
- **项目名称**: SagaStats
- **引擎版本**: Unreal Engine 5.5.4
- **文档版本**: v1.0
- **创建日期**: 2025-07-16
- **最后更新**: 2025-07-16

---

## 目录
1. [系统概述](#系统概述)
2. [核心架构](#核心架构)
3. [技术实现](#技术实现)
4. [配置系统](#配置系统)
5. [游戏机制](#游戏机制)
6. [性能优化](#性能优化)
7. [使用示例](#使用示例)
8. [最佳实践](#最佳实践)
9. [问题与限制](#问题与限制)
10. [版本历史](#版本历史)

---

## 系统概述

### 项目简介
SagaStats是一个基于虚幻引擎5.5构建的**高级状态管理系统**，通过扩展Gameplay Ability System(GAS)实现了功能强大的Meter计量器系统。该系统结合了现代游戏设计理念和虚幻引擎的原生架构，为RPG、动作冒险等需要复杂属性管理的游戏提供了完整的解决方案。

### 核心特性
- ✅ **模块化架构**: 清晰的继承层次和接口设计
- ✅ **数据驱动配置**: 通过CSV数据表配置计量器参数
- ✅ **智能状态机制**: 支持复杂的状态转换和恢复逻辑
- ✅ **守护系统**: 多层保护机制和伤害分配
- ✅ **网络同步**: 完整的多人游戏支持
- ✅ **蓝图友好**: 完全支持蓝图开发流程
- ✅ **异步事件**: 丰富的状态监听和回调机制

---

## 核心架构

### 类层次结构
```
USagaAttributeSet (基础属性集)
└── USagaMeterBase (抽象计量器基类)
    ├── USagaIncreaseMeter (递增型计量器)
    │   ├── 魔法值系统
    │   ├── 耐力系统
    │   └── 经验值系统
    └── USagaDecreaseMeter (递减型计量器)
        ├── 生命值系统
        ├── 护甲系统
        └── 护盾系统
```

### 核心组件

#### 1. USagaMeterBase (抽象基类)
**核心属性**:
```cpp
// 当前值（支持夹紧限制）
FSagaClampedGameplayAttributeData Current;

// 最大值
FGameplayAttributeData Maximum;

// 累积值和减少值（用于间接操作）
FGameplayAttributeData Accumulate;
FGameplayAttributeData Reduce;

// 实际影响的数值（用于记录）
FGameplayAttributeData ImpactedAccumulate;
FGameplayAttributeData ImpactedReduce;
```

**核心功能**:
- 提供所有计量器的通用接口
- 实现基础的累积和减少逻辑
- 支持填满和清空状态检测
- 提供网络同步和事件回调

#### 2. USagaIncreaseMeter (递增型计量器)
**特殊属性**:
```cpp
FGameplayAttributeData Degeneration;          // 衰减速率（每秒）
FGameplayAttributeData DegenerationCooldown;  // 衰减冷却时间
```

**应用场景**:
- 魔法值：使用后自然恢复，长时间不用会衰减
- 耐力：奔跑消耗，停止后恢复
- 经验值：只增不减的积累型属性

**核心机制**:
```cpp
// 自动衰减逻辑
void Tick(float DeltaTime) override
{
    if (CanDegeneration() && !IsEmptied())
    {
        float NewValue = GetCurrent() - GetDegeneration() * DeltaTime;
        SetAttributeValue(GetCurrentAttribute(), NewValue);
    }
}
```

#### 3. USagaDecreaseMeter (递减型计量器)
**特殊属性**:
```cpp
FGameplayAttributeData Regeneration;         // 再生速率（每秒）
FGameplayAttributeData RegenerationCooldown; // 再生冷却时间
FGameplayAttributeData LockDuration;         // 锁定持续时间
FGameplayAttributeData ResetRate;            // 快速恢复速率
FGameplayAttributeData ImmuneThreshold;      // 免疫阈值
```

**状态机制**:
```cpp
enum class EMeterState : uint8
{
    Normal,  // 正常状态：可以再生和受到伤害
    Lock,    // 锁定状态：不能再生，通常在清空后进入
    Reset,   // 重置状态：快速恢复到最大值
};
```

**应用场景**:
- 生命值：受伤后延迟恢复，支持守护机制
- 护盾：临时保护，破碎后长时间锁定
- 护甲：物理防护，可修复恢复

---

## 技术实现

### 间接属性修改机制
系统采用间接修改方式，避免直接操作Current属性：
```cpp
GameplayEffect → 修改Accumulate/Reduce → 触发OnAccumulate/OnReduce → 实际修改Current
```

**优势**:
- 🎯 **可控性**: 所有变化都经过逻辑处理
- 🎯 **可追踪**: 记录每次变化的实际影响值
- 🎯 **可扩展**: 易于添加复杂的计算逻辑

### 事件系统架构
```cpp
// 核心事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMeterEmptiedEvent, 
    TSubclassOf<USagaMeterBase>, MeterClass, USagaMeterBase*, MeterInstance);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMeterFilledEvent,
    TSubclassOf<USagaMeterBase>, MeterClass, USagaMeterBase*, MeterInstance);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMeterStateChangeEvent,
    TSubclassOf<USagaDecreaseMeter>, MeterClass, USagaDecreaseMeter*, MeterInstance, 
    EMeterState, NewState);
```

### 异步监听系统
```cpp
// 等待计量器清空
UAbilityAsync_WaitMeterEmptied* WaitMeterEmptied(AActor* TargetActor, 
    TSubclassOf<USagaMeterBase> MeterClass);

// 等待计量器填满
UAbilityAsync_WaitMeterFilled* WaitMeterFilled(AActor* TargetActor,
    TSubclassOf<USagaMeterBase> MeterClass);

// 等待状态变化
UAbilityAsync_WaitMeterStateChange* WaitMeterStateChange(AActor* TargetActor,
    TSubclassOf<USagaDecreaseMeter> MeterClass);
```

### 守护机制实现
```cpp
// 守护计算逻辑
UFUNCTION(BlueprintNativeEvent)
void CalcGuardReduce(USagaDecreaseMeter* ProtectedMeter, float InReduce, 
                     float& OutGuardReduce, float& OutRemainReduce) const;
```

**伤害分配流程**:
1. 伤害首先作用于守护计量器（如护盾）
2. 守护计量器按算法承受部分伤害
3. 剩余伤害传递给被保护的计量器（如生命值）
4. 记录各层的实际承受伤害

---

## 配置系统

### 数据表配置结构
所有计量器参数通过CSV数据表配置，支持热重载和版本控制。

#### 生命值配置示例 (DT_Health.csv)
```csv
,BaseValue
Meter_Health.Maximum,100
Meter_Health.Regeneration,10
Meter_Health.RegenerationCooldown,2
Meter_Health.LockDuration,2
Meter_Health.ResetRate,100
Meter_Health.ImmuneThreshold,100
```

#### 护盾配置示例 (DT_Shield.csv)
```csv
,BaseValue
Meter_Shield.Maximum,50
Meter_Shield.Regeneration,0
Meter_Shield.RegenerationCooldown,0
Meter_Shield.LockDuration,10
Meter_Shield.ResetRate,0
Meter_Shield.ImmuneThreshold,0
```

#### 魔法值配置示例 (DT_MyIncrease.csv)
```csv
,BaseValue
Meter_MyIncrease.Maximum,100
Meter_MyIncrease.Degeneration,10
Meter_MyIncrease.DegenerationCooldown,2
```

### GameplayEffect集成
```cpp
// 增长效果配置
GE_MyIncrease:
- 持续时间: Infinite
- 修改器: Additive to SagaMeterBase.Accumulate
- 数值: ScalableFloat(1.0)

// 减少效果配置  
GE_MyDecrease:
- 持续时间: Infinite
- 修改器: Additive to SagaMeterBase.Reduce
- 数值: ScalableFloat(1.0)

// 复杂伤害效果
GE_ReduceHealth:
- 执行计算: EC_HealthMeter
- 计算标识: "SagaMeter.Calc.Reduce"
- 支持守护机制和伤害分配
```

---

## 游戏机制

### 生命值系统机制
1. **正常状态**: 每秒恢复10点生命值
2. **受伤打断**: 受到伤害后停止恢复2秒
3. **守护保护**: 护盾优先承受伤害
4. **快速恢复**: 特殊情况下以100/秒的速度恢复
5. **伤害免疫**: 在重置状态下提供临时免疫

### 护盾系统机制
1. **临时保护**: 最大值50点，无自动恢复
2. **破碎锁定**: 破碎后锁定10秒无法恢复
3. **优先承伤**: 作为生命值的守护层
4. **动态分配**: 支持运行时动态设置护盾值

### 魔法值系统机制
1. **使用消耗**: 施法时消耗魔法值
2. **自然恢复**: 停止使用后自动恢复
3. **长期衰减**: 长时间不使用会缓慢衰减
4. **冷却机制**: 衰减前有2秒保护期

---

## 性能优化

### Tick优化策略
```cpp
// 智能Tick控制
bool USagaMeterBase::ShouldTick() const
{
    // 只有在需要更新时才Tick
    return HasAuthority() && (NeedRegeneration() || NeedDegeneration());
}
```

### 网络同步优化
- **选择性复制**: 只复制必要的属性变化
- **RepNotify优化**: 避免不必要的客户端更新
- **批量同步**: 将多个属性变化合并同步

### 内存管理
- **对象池**: 重用临时计量器对象
- **智能指针**: 正确的生命周期管理
- **延迟清理**: 避免频繁的内存分配

---

## 使用示例

### 创建自定义计量器
```cpp
// 1. 创建蓝图类，继承USagaDecreaseMeter
// 2. 配置数据表
// 3. 设置GameplayEffect
// 4. 集成到AbilitySystemComponent
```

### 蓝图使用示例
```blueprint
// 获取计量器实例
MeterInstance = GetAbilitySystemComponent()->GetAttributeSet(MeterClass)

// 监听状态变化
AsyncTask = WaitMeterEmptied(TargetActor, HealthMeterClass)
AsyncTask.OnEmptied.AddDynamic(this, OnHealthEmptied)

// 应用伤害效果
ApplyGameplayEffectToSelf(GE_ReduceHealth, 1.0)
```

### C++使用示例
```cpp
// 获取计量器组件
if (USagaAbilitySystemComponent* ASC = GetAbilitySystemComponent())
{
    // 监听计量器事件
    ASC->GetMeterEmptiedDelegate(HealthMeterClass).AddDynamic(
        this, &AMyCharacter::OnHealthEmptied);
    
    // 应用效果
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
        GE_ReduceHealth, 1.0f, ASC->MakeEffectContext());
    ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
```

---

## 最佳实践

### 设计原则
1. **组合优于继承**: 通过配置而非代码变更定制行为
2. **数据驱动**: 使用数据表配置参数，便于平衡调整
3. **事件驱动**: 使用异步监听器响应状态变化
4. **网络友好**: 设计时考虑多人游戏的同步需求

### 命名约定
- 属性集类：`SAS_` 前缀 (Saga Attribute Set)
- 计量器类：`Meter_` 前缀
- 游戏效果：`GE_` 前缀 (Gameplay Effect)
- 数据表：`DT_` 前缀 (Data Table)
- 执行计算：`EC_` 前缀 (Execution Calculation)

### 文件组织
```
Plugins/SagaStats/
├── Source/SagaStats/          # 核心运行时代码
├── Source/SagaStatsEditor/    # 编辑器扩展
└── Content/                   # 蓝图和资源

Content/ExampleContent/
├── 01_SagaStats_Welcome/      # 系统介绍
├── 02_Builtin_Clamping/       # 夹紧功能
├── 03_MMC/                    # 修改量计算
├── 04_EC/                     # 执行计算
└── 05_SagaMeter/              # 完整示例
    ├── Meter/IncreaseMeter/   # 递增型示例
    ├── Meter/DecreaseMeter/   # 递减型示例
    └── Meter/GuardMeter/      # 守护型示例
```

### 调试建议
```cpp
// 启用调试输出
UE_LOG(LogSagaStats, Log, TEXT("Meter %s: Current=%.1f, Max=%.1f"), 
       *GetClass()->GetName(), GetCurrent(), GetMaximum());

// 使用控制台命令
AbilitySystem.Effect.ListActive
AbilitySystem.Ability.Grant Meter_Health
```

---

## 问题与限制

### 已知问题
1. **精度问题**: 浮点数计算可能存在精度误差
2. **性能考虑**: 大量计量器同时Tick可能影响性能
3. **复杂度**: 状态机逻辑较复杂，调试困难

### 当前限制
1. **UE版本依赖**: 仅适用于UE 5.5+版本
2. **GAS依赖**: 必须正确配置AbilitySystemComponent
3. **编辑器限制**: 某些功能仅在编辑器环境可用

### 解决方案
1. 使用`GreaterOrNearlyEqual`等函数处理精度问题
2. 实现智能Tick机制，避免不必要的更新
3. 提供丰富的调试信息和可视化工具

---

## 版本历史

### v1.0 (2025-07-16)
- ✅ 完成核心Meter系统架构
- ✅ 实现递增和递减计量器
- ✅ 添加守护机制支持
- ✅ 完善异步事件系统
- ✅ 集成UE5.5.4 GAS
- ✅ 提供完整示例和文档

### 未来规划
- 🔄 性能优化和内存管理改进
- 🔄 更多自定义计量器类型
- 🔄 可视化调试工具
- 🔄 移动端优化
- 🔄 更丰富的AI集成接口

---

## 联系信息

- **项目位置**: `D:\UnrealEngine\UnrealEngine\Projects\SagaStats`
- **文档维护**: Claude Code
- **最后更新**: 2025-07-16

---

*该文档将随着系统的迭代更新而持续维护和完善。*
# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

SagaStats是一个基于Unreal Engine 5.5的状态系统，扩展了游戏能力系统（Gameplay Ability System，GAS），支持完全可蓝图化的属性定义和数值计算。

## 核心架构

### 主要模块结构

1. **SagaStats核心插件** (`Plugins/SagaStats/`)
   - 运行时模块：核心属性系统实现
   - 编辑器模块：蓝图编辑器集成和自定义UI

2. **GameplayAbilities插件** (`Plugins/GameplayAbilities/`)
   - Epic官方GAS插件的本地副本
   - 包含GAS的所有核心功能

3. **主项目** (`Source/SagaStats_544/`)
   - 基础项目设置
   - 依赖：Core, CoreUObject, Engine, InputCore, EnhancedInput

### 关键类层次结构

```
USagaAttributeSet (继承自UAttributeSet)
├── USagaMeterBase (抽象基类)
    ├── USagaIncreaseMeter (增加型仪表)
    └── USagaDecreaseMeter (减少型仪表)

USagaAbilitySystemComponent (继承自UAbilitySystemComponent)
├── 扩展的属性集管理
├── 仪表状态事件系统
└── 自定义委托系统
```

## 开发工作流程

### 编译项目
- 使用 Visual Studio 2022 或 Rider
- 目标平台：Windows x64
- 引擎版本：UE 5.5.4

### 运行项目
- 通过 UnrealEditor 启动
- 或使用 `SagaStats_544.uproject` 直接打开

### 调试
- 使用 Visual Studio 调试器
- 在编辑器中使用 `AbilitySystem` 控制台命令调试GAS功能：
  - `AbilitySystem.Ability.Grant <ClassName>`
  - `AbilitySystem.Ability.Activate <TagName>`
  - `AbilitySystem.Effect.Apply <ClassName>`
  - `AbilitySystem.Effect.ListActive`

## 核心功能特性

### 1. 属性系统 (Attribute System)
- **FSagaClampedGameplayAttributeData**: 支持最小值/最大值夹紧的属性数据
- **SAGA_ATTRIBUTE_ACCESSORS**: 自动生成属性访问器的宏
- **蓝图可视化属性创建**: 通过编辑器UI创建新属性

### 2. 仪表系统 (Meter System)
- **USagaMeterBase**: 仪表基类，支持Current/Maximum/Accumulate属性
- **USagaIncreaseMeter**: 增加型仪表（如经验条）
- **USagaDecreaseMeter**: 减少型仪表（如生命值），支持状态系统
- **数据表集成**: 通过CSV文件配置仪表数据

### 3. 异步能力任务 (Async Ability Tasks)
- **AbilityAsync_WaitAttributeSetAddOrRemove**: 等待属性集添加/移除
- **AbilityAsync_WaitMeterEmptied**: 等待仪表耗尽
- **AbilityAsync_WaitMeterFilled**: 等待仪表填满
- **AbilityAsync_WaitMeterStateChange**: 等待仪表状态变化

### 4. 编辑器扩展
- **SagaK2Node_SwitchGameplayAttribute**: 蓝图节点，用于基于属性的分支
- **自定义属性集编辑器**: 可视化属性创建和编辑
- **详细面板定制**: 增强属性和仪表的编辑体验

## 示例内容结构

项目包含5个示例关卡展示不同功能：

1. **01_SagaStats_Welcome**: 插件介绍和基础设置
2. **02_Builtin_Clamping**: 内置夹紧功能演示
3. **03_MMC**: 修改量计算类(MMC)使用示例
4. **04_EC**: 执行计算类(EC)使用示例
5. **05_SagaMeter**: 仪表系统完整演示

## 常用开发任务

### 创建新的属性集
1. 在内容浏览器中右键 → 蓝图类
2. 选择 "SagaAttributeSet" 作为父类
3. 使用编辑器UI添加属性变量
4. 编译保存

### 创建新的仪表
1. 继承 `USagaMeterBase` 或其子类
2. 实现必要的仪表逻辑
3. 创建对应的数据表(DataTable)
4. 配置游戏效果(GameplayEffect)

### 集成到现有项目
1. 将SagaStats插件复制到目标项目的Plugins文件夹
2. 在.uproject文件中添加插件引用
3. 在Build.cs中添加依赖："SagaStats", "GameplayAbilities"
4. 重新生成项目文件

## 重要约定

### 命名约定
- 属性集类：`SAS_` 前缀 (Saga Attribute Set)
- 仪表类：`Meter_` 前缀
- 游戏效果：`GE_` 前缀 (Gameplay Effect)
- 数据表：`DT_` 前缀 (Data Table)

### 文件组织
- 核心系统代码位于 `Plugins/SagaStats/Source/SagaStats/`
- 编辑器扩展位于 `Plugins/SagaStats/Source/SagaStatsEditor/`
- 示例内容位于 `Content/ExampleContent/`

### 网络复制
- 所有仪表属性默认支持网络复制
- 使用 `SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY` 宏处理复制通知
- 委托系统支持多播事件

## 关键文件位置

### 核心系统文件
- `SagaAttributeSet.h/.cpp`: 属性集基类
- `SagaMeterBase.h/.cpp`: 仪表基类
- `SagaAbilitySystemComponent.h/.cpp`: 扩展的能力系统组件
- `SagaStatsType.h`: 核心数据类型定义

### 编辑器文件
- `SagaAttributeSetDetails.h/.cpp`: 属性集详细面板
- `SagaK2Node_SwitchGameplayAttribute.h/.cpp`: 自定义蓝图节点
- `SagaAttributeSetBlueprintEditor.h/.cpp`: 属性集蓝图编辑器

### 实用工具
- `SagaBlueprintLibrary.h/.cpp`: 蓝图函数库
- `SSUtils.h/.cpp`: 静态工具函数
- `SSExecutionCalculationBlueprintLibrary.h/.cpp`: 执行计算蓝图库

## 注意事项

- 本项目使用UE 5.5.4版本，某些功能可能与其他版本不兼容
- 仪表系统基于GAS，需要正确配置AbilitySystemComponent
- 编辑器扩展功能仅在编辑器环境中可用
- 所有异步任务都需要正确的生命周期管理

这个系统为RPG、动作冒险游戏等需要复杂属性和状态管理的游戏提供了强大的基础框架。
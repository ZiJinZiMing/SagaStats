# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

MySagaStats 是一个 Unreal Engine 5.7 多变体游戏机制框架项目，用于验证"声明式机制编排框架"（DPU + DC + Condition）。包含三个游戏变体（Combat、SideScrolling、Platforming），共享基础角色/输入/游戏模式架构。

## 构建命令

```bash
# 使用 UnrealBuildTool 构建（需要 UE 5.7 安装）
# Game 目标
UnrealBuildTool MySagaStats Win64 Development

# Editor 目标
UnrealBuildTool MySagaStatsEditor Win64 Development

# 也可通过 Visual Studio 打开 .sln 文件构建
```

引擎关联 ID：`{3F3CF689-4E12-7DE7-05B3-F185724CAC93}`

## 架构

### 模块结构

单一 C++ 模块 `MySagaStats`，包含基础层 + 三个变体子目录：

- `Source/MySagaStats/` — 基础类（AMySagaStatsCharacter, GameMode, PlayerController）
- `Source/MySagaStats/Variant_Combat/` — 战斗系统（近战combo、蓄力攻击、AI、生命条）
- `Source/MySagaStats/Variant_SideScrolling/` — 横版动作（墙跳、软平台、交互、捡拾）
- `Source/MySagaStats/Variant_Platforming/` — 平台跳跃（二段跳、墙跳、冲刺）

### 插件

- `Plugins/GameplayAbilities/` — 自定义 Gameplay Ability System 插件（Runtime + Editor 两个子模块）
- 引擎插件：StateTree（AI状态树）、EnhancedInput

### 关键设计模式

- **分层继承**：基础类 → 变体特化类 → 蓝图实现（Content/ 下的 BP_ 资产）
- **接口驱动**：Combat 变体通过 `ICombatAttacker`、`ICombatDamageable`、`ICombatActivatable` 接口解耦
- **AnimNotify 驱动战斗**：`AnimNotify_DoAttackTrace`、`AnimNotify_CheckCombo`、`AnimNotify_CheckChargedAttack` 在动画中触发战斗逻辑
- **StateTree AI**：CombatAIController 使用 StateTreeAIComponent 而非行为树
- **Enhanced Input**：所有输入通过 InputAction 资产 + InputMappingContext 管理

### Build.cs 依赖

主模块依赖：Core, CoreUObject, Engine, InputCore, EnhancedInput, AIModule, StateTreeModule, GameplayStateTreeModule, UMG, Slate

GameplayAbilities 插件额外依赖：GameplayTags, GameplayTasks, MovieScene, PhysicsCore, DataRegistry

### 内容/关卡

四个可玩地图：
- `/Game/ThirdPerson/Lvl_ThirdPerson` — 基础演示
- `/Game/Variant_Combat/Lvl_Combat` — 战斗演示
- `/Game/Variant_SideScrolling/Lvl_SideScrolling` — 横版演示
- `/Game/Variant_Platforming/Lvl_Platforming` — 平台演示

### 历史对话记录

项目包含从 Claude Web 导出的对话数据，位于 `chats/` 目录。当需要延续之前的设计讨论时，应从已导出的 md 文件中读取上下文。

**已导出对话（优先从这些文件读取）**：

- `chats/L3_conversation_V2.md` — "L3动作游戏模块化流程设计V2"（405条消息，2026-03-11 ~ 03-24），核心讨论声明式机制编排框架（DPU + DC + Condition）的迭代设计，包含 MVP DAG 拓扑、防御判定、只狼/鬼泣5管线验证等

**原始数据（仅在需要查找其他对话时使用）**：

- `chats/conversations.json` — 全部 59 个对话的单行 JSON（~40MB），用 Python `json` 模块解析，结构：`data[i]['name']` 对话标题，`data[i]['chat_messages'][n]['text'/'sender']` 消息内容

**注意**：导出数据中 Assistant 的工具调用显示为 "This block is not supported"，但文本内容完整保留；Human 附件仅有元数据无内容。

### 设计理论

项目基于声明式机制编排框架（文档位于 `Doc/`）：
- **DPU**（Discrete Processing Unit）— 声明式机制处理体
- **DC**（Dynamic Context）— 共享上下文
- **Condition** — 执行条件门控
- DPU 根据 produces 和 Condition 引用的 DC 字段自动拓扑排序执行

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

MySagaStats 是一个 Unreal Engine 5.7 项目，核心是 **SagaStats 插件**中的 **DPU Framework**——用于动作游戏模块化伤害管线的声明式编排框架。项目同时包含三个游戏变体（Combat、SideScrolling、Platforming）用于验证框架。

## 构建命令

```bash
# Editor 目标（开发用）
UnrealBuildTool MySagaStatsEditor Win64 Development
# 也可通过 Visual Studio 打开 .sln 文件构建
```

引擎路径：`D:\UnrealEngine\UnrealEngine`
引擎关联 ID：`{3F3CF689-4E12-7DE7-05B3-F185724CAC93}`

## 核心架构：DPU Framework（v4.5）

代码位于 `Plugins/SagaStats/Source/SagaStats/` 的 `Public/DPUFramework/` 和 `Private/DPUFramework/`。

### 设计文档体系

当前版本的设计文档（优先从这些文件读取）：
- `全量资料/模块化伤害管线_模型规范文档_v4_5.md` — 模型层规范（WHY+WHAT+HOW）
- `全量资料/模块化伤害管线_Fact模型设计.md` — Fact 模型设计 v2（DPU产出结构化，去掉独立 Fact Key）
- `全量资料/模块化伤害管线_实现层架构文档.md` — 实现层架构文档 v4.5
- `全量资料/模块化伤害管线_本轮讨论总结.md` — 本轮四个核心决策的完整记录
- `Doc/damagePipeline/待讨论问题集.md` — 当前所有待讨论问题和实现进度

### 四条设计思想（内容域 WHY）

1. **机制优先** — 体验由机制驱动，不是由数值驱动
2. **数值兜底** — 数值不驱动体验，但保证数学基础正确
3. **反馈清晰可感** — 每个机制结果必须被玩家感知到
4. **涌现自治** — 多机制组合效果由 DPU+Condition 在 DC 上自然产生

### 三个核心概念 + Fact 模型

| 概念 | 实现类 | 职责 |
|------|--------|------|
| **DPU**（机制处理体） | `UDPUDefinition` + `UDPULogicBase` | 自洽的积木块。Execute 是纯函数：读 DC（只读）→ 返回 Fact（FInstancedStruct）。**DPU 不写 DC**——PipelineAsset 拿到返回值后以 DPU 身份标识写入 DC |
| **Fact**（结构化产出） | `FInstancedStruct` + `UFactMethodRegistry` | DPU 产出的结构体，自带领域查询方法。以 DPU 身份标识为 DC 索引（v4.5 去掉了独立 Fact Key——经三步检验确认为偶然复杂度） |
| **DC**（共享上下文） | `UDamageContext` | `TMap<FName, FInstancedStruct>`，DPU 名为索引。Fact 缺失视为 false（R3） |
| **Condition**（条件门控） | `UConditionNode` 树（And/Or/FactQuery/Compare） | 属性面板内联编辑。FactQuery/Compare 直接引用 `UDPUDefinition*`（DataAsset 引用，类型安全）+ 领域方法。bReverse 取反。递归 GetConsumedDPUs() 自动收集依赖 |
| **PipelineAsset**（管线资产） | `UPipelineAsset` | 自洽的管线容器：Build() 烘焙拓扑排序 → Execute(DC) 按序执行 DPU 并将返回的 Fact 写入 DC。**不存在独立的 PipelineManager** |

### 关键数据流

```
Attack命中 → 创建 DC → 填入事件上下文
→ PipelineAsset::Execute(DC)
  → 按烘焙的拓扑顺序逐个 DPU：
    → EvaluateCondition(DC)（条件树求值 + bReverse）
    → 通过 → FInstancedStruct Fact = DPULogic::Execute(DC)（纯函数，DC 只读）
             → PipelineAsset 以 DPU->DPUName 将 Fact 写入 DC
    → 不通过 → 跳过
→ DC 中所有 Fact 填满 → 表现选取阶段（Phase 1.5 待实现）
```

### 文件清单

```
Plugins/SagaStats/Source/SagaStats/
├── Public/DPUFramework/
│   ├── DamageContext.h        — DC，TMap<FName, FInstancedStruct> + 标量兼容包装器
│   ├── ConditionNode.h        — 条件树（And/Or/FactQuery/Compare + bReverse）
│   │                            FactQuery/Compare 引用 UDPUDefinition*（DataAsset）
│   │                            MethodName/FieldName 通过 GetOptions 联动下拉
│   ├── DPUDefinition.h        — DPU 定义 DataAsset（DPUName, ProducesFactType:UScriptStruct*, Condition, LogicClass, BasePriority）
│   ├── DPULogicBase.h         — DPU 逻辑基类（BlueprintNativeEvent Execute 返回 FInstancedStruct，DC 参数 const）
│   ├── PipelineAsset.h        — Pipeline（Build/Execute/StableTopologicalSort/ExportMermaidDAG + bIsBaked 缓存）
│   ├── FactMethodRegistry.h   — Fact 领域方法注册表（UEngineSubsystem，C++ Register<T> + 蓝图 RegisterBP）
│   ├── SekiroFacts.h          — 只狼 MVP Fact 结构体定义（7 复杂 + 5 信号）
│   ├── DPUTestLogics.h        — 测试用 DPU Logic 子类（13个，全部返回 FInstancedStruct）
│   └── DPUPipelineTestActor.h — 运行时测试 Actor
└── Private/DPUFramework/
    ├── 对应 .cpp 实现
    └── SekiroFacts.cpp         — Fact 领域方法注册（FactMethodRegistry::Initialize 调用）
```

### 编辑器扩展

位于 `Plugins/SagaStats/Source/SagaStatsEditor/`：
- `SGPipelineAssetDetails` — PipelineAsset 的 Build 按钮 + 烘焙状态显示
- ConditionNode 的 MethodName/FieldName 下拉通过 `UPROPERTY(meta=(GetOptions="..."))` 实现
- ConditionNode 的 SourceDPU 是 `TObjectPtr<UDPUDefinition>`，编辑器自动提供 DataAsset 选择器

### Build.cs 依赖

SagaStats Runtime：Core, CoreUObject, Engine, Projects, NetCore, StructUtils, GameplayTags, GameplayTasks, GameplayAbilities

SagaStatsEditor：Core, AssetTools, SagaStats, StructUtils + 标准编辑器模块

### Mermaid DAG 导出

PipelineAsset::Execute() 后自动生成 `.mmd` 文件到 `Saved/Graphs/`。包含：
- DPU 节点（颜色区分执行/跳过、Condition 文本、Produces Fact 类型名 + 色块）
- 依赖连线（每个 DPU 间依赖独立着色 + linkStyle）
- DC Initial / DC Final 节点
- 隐藏链控制水平布局（graph LR + ~~~）
- Mermaid 实体语法：`#9632;`（■），`#amp;`（&），不带前导 `&`

## 游戏变体（验证用）

| 变体 | 目录 | 关卡 |
|------|------|------|
| Combat | `Source/MySagaStats/Variant_Combat/` | `/Game/Variant_Combat/Lvl_Combat` |
| SideScrolling | `Source/MySagaStats/Variant_SideScrolling/` | `/Game/Variant_SideScrolling/Lvl_SideScrolling` |
| Platforming | `Source/MySagaStats/Variant_Platforming/` | `/Game/Variant_Platforming/Lvl_Platforming` |

## 历史对话记录

项目包含从 Claude Web 导出的对话数据，位于 `chats/` 目录。当需要延续之前的设计讨论时，应从已导出的 md 文件中读取上下文。

**已导出对话（优先从这些文件读取）**：

- `chats/L3_conversation_V2.md` — "L3动作游戏模块化流程设计V2"（405条消息，2026-03-11 ~ 03-24），核心讨论声明式机制编排框架（DPU + DC + Condition）的迭代设计，包含 MVP DAG 拓扑、防御判定、只狼/鬼泣5管线验证等

**原始数据（仅在需要查找其他对话时使用）**：

- `chats/conversations.json` — 全部 59 个对话的单行 JSON（~40MB），用 Python `json` 模块解析，结构：`data[i]['name']` 对话标题，`data[i]['chat_messages'][n]['text'/'sender']` 消息内容

**注意**：导出数据中 Assistant 的工具调用显示为 "This block is not supported"，但文本内容完整保留；Human 附件仅有元数据无内容。

## 关键约束（R1-R6）

- **R1**：DC 是唯一通信介质，DPU 之间不直接调用
- **R2**：DPU 自洽——不知道自己的身份标识，不知道 Condition，不知道其他 DPU。Execute 是纯函数（读 DC → 返回 Fact）
- **R3**：Condition 可读 DC 全部内容，Fact 缺失视为 false
- **R4**：Condition 与 DPU 是装配关系（可替换不影响 DPU 内部）
- **R5**：执行顺序由依赖关系自动决定（GetConsumedDPUs → 拓扑排序）
- **R6**：禁止循环依赖

## 设计方法论

- **Essential/Accidental Complexity** — 模型层只含本质复杂度，偶然复杂度属于实现层
- **三步检验** — 引入新概念时：①删除测试 ②来源追溯 ③换表达验证
- **内容域 vs 工具层** — 文档标题用内容域（模块化伤害管线），不用工具层（声明式编排框架）

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

MySagaStats 是一个 Unreal Engine 5.7 项目，核心是 **SagaStats 插件**中的 **DamagePipeline**（原 DPU Framework / DamageFramework）——用于动作游戏模块化伤害管线的声明式编排框架。代码当前版本为 **v4.6**（实现层），模型规范文档版本为 **v4.7**。

项目同时包含三个游戏变体（Combat、SideScrolling、Platforming）用于验证框架。

## 构建命令

```bash
# Editor 目标（开发用）
"D:/UnrealEngine-JYDev/Engine/Build/BatchFiles/Build.bat" MySagaStatsEditor Win64 Development -Project="D:/MyWorkSpace/MySagaStats/MySagaStats.uproject" -WaitMutex
```

引擎路径：`D:\UnrealEngine-JYDev`
引擎关联 ID：`{3F3CF689-4E12-7DE7-05B3-F185724CAC93}`

## 核心架构：DamagePipeline（v4.6 代码 / v4.7 模型）

代码位于 `Plugins/SagaStats/Source/SagaStats/` 的 `Public/DamagePipeline/` 和 `Private/DamagePipeline/`。

### 设计文档体系（优先读取）

- `全量资料/模块化伤害管线_模型规范文档_v4_7.md` — 模型层规范（WHY+WHAT+HOW）
- `全量资料/模块化伤害管线_实现层架构文档.md` — 实现层架构文档 v4.7
- `全量资料/模块化伤害管线_DamageEffect模型设计.md` — DamageEffect 模型设计 v3
- `全量资料/模块化伤害管线_实现层变更记录_v4.6.md` — **v4.6 实现层相对 v4.7 模型文档的架构差异**（7条）
- `Doc/damagePipeline/待讨论问题集.md` — 当前所有待讨论问题和实现进度

### 四条设计思想（内容域 WHY）

1. **机制优先** — 体验由机制驱动，不是由数值驱动
2. **数值兜底** — 数值不驱动体验，但保证数学基础正确
3. **反馈清晰可感** — 每个机制结果必须被玩家感知到
4. **涌现自治** — 多机制组合效果由 DamageRule+Condition 在 DC 上自然产生

### 术语对照（v4.5 → v4.6）

| 旧术语 | 新术语 | 类 |
|---|---|---|
| DPU | DamageRule | `UDamageRule` (原 `UDPUDefinition`/`UDamageRuleDefinition`) |
| Fact | DamageEffect | USTRUCT 如 `FMixupEffect`（原 `FMixupResult`） |
| Logic | Operation | `UDamageOperationBase` (原 `UDPULogicBase`) |
| PipelineAsset | DamagePipeline | `UDamagePipeline` (原 `UPipelineAsset`) |
| ConditionNode | Condition/Predicate（双层） | `UDamageCondition` + `UDamagePredicate` |

### 核心类

| 概念 | 实现类 | 职责 |
|---|---|---|
| **DamageRule** | `UDamageRule` (DataAsset) | 规则定义：Description + Condition 树 + OperationClass。`GetProducesEffectType()` 从 OperationClass CDO 反查 |
| **DamageOperation** | `UDamageOperationBase` | 逻辑基类。`Execute(Context, OutEffect&)` —— 框架传入预创建的 OutEffect，Operation 只填字段 |
| **DamageEffect** | USTRUCT（如 `FGuardEffect`） | 规则产出的结构化数据。DC 以 `UScriptStruct*` 为键存储 |
| **DC** | `UDamageContext` | 共享上下文。`TMap<UScriptStruct*, FInstancedStruct> DamageEffects` —— 类型即 key。攻击上下文和规则产出共用同一存储 |
| **Condition 树** | `UDamagePredicate`(容器) + `UDamageCondition`(原子) | 双层结构：Predicate 负责 AND/OR/Single 组合 + bReverse；Condition 是原子检查，子类即域方法 |
| **DamagePipeline** | `UDamagePipeline` (DataAsset) | 持有 DamageRules 列表 + `Build()`（拓扑排序）+ `Execute(Context)`（按序执行） |

### 关键数据流

```
Attack命中 → 创建 Context（UDamageContext）
→ Context->SetEffectByType(FSekiroAttackContext)  // 攻击上下文以 Effect 类型存入
→ DamagePipeline::Execute(Context)
  → 按烘焙的拓扑顺序逐个 DamageRule：
    → Predicate->EvaluatePredicate(Context)（递归求值 + bReverse）
    → 通过 → FInstancedStruct OutEffect(ProducesEffectType)  // 框架预创建
             → Operation->Execute(Context, OutEffect)  // Operation 填字段
             → Context->SetEffectByType(OutEffect)  // 框架写入 DC
    → 不通过 → 跳过
```

### v4.6 实现层相对 v4.7 模型文档的 7 条架构差异

这些差异是实现层的决策，与模型文档描述不同，详见 `实现层变更记录_v4.6.md`：

1. **双层条件树** — DamagePredicate(容器) + DamageCondition(原子)，非模型文档的单层
2. **子类即域方法** — 无 MethodName，每个域方法一个 DamageCondition 子类
3. **UScriptStruct\* 作 DC 键** — 非模型文档的 FName(RuleName)
4. **OutEffect 直传** — Execute(DC, OutEffect&)，非模型文档的"返回 Effect → Pipeline 写 DC"
5. **bReverse 仅在 Predicate 上** — 非模型文档的"所有节点都有"
6. **EffectType HideInDetailPanel** — 子类的类级属性，实例不可修改
7. **Build 时 EffectType 校验** — DamagePipeline::Build() 前置校验

### 文件清单

```
Plugins/SagaStats/Source/SagaStats/
├── Public/DamagePipeline/
│   ├── DamageContext.h         — DC，TMap<UScriptStruct*, FInstancedStruct> 统一存储
│   ├── DamageCondition.h       — 条件原子基类（Evaluate 接收按 EffectType 预取的 Effect）
│   ├── DamagePredicate.h       — 谓词容器（Single/And/Or + bReverse）
│   ├── DamageOperationBase.h   — Operation 基类（Execute(Context, OutEffect&)）
│   ├── DamageRule.h            — DamageRule DataAsset（Description + Condition + OperationClass）
│   ├── DamagePipeline.h        — Pipeline（Build/Execute/Mermaid 导出）
│   ├── DamageContextLibrary.h  — 蓝图 CustomThunk 接口（SetEffect/GetEffect 通配结构体）
│   ├── DamagePipelineTestActor.h — 运行时测试 Actor
│   └── Sekiro/                 — 只狼验证用的具体 DamageRule（Effect+Condition+Operation 三位一体）
│       ├── DR_AttackContext.h   — FSekiroAttackContext + IsLightning/IsInAir Condition
│       ├── DR_Mixup.h           — FMixupEffect + IsGuard/IsJustGuard
│       ├── DR_Guard.h           — FGuardEffect + GuardSuccess/GuardIsJustGuard
│       ├── DR_Death.h / DR_Hurt.h / DR_Collapse.h / ...
└── Private/DamagePipeline/      — 对应 .cpp 实现
```

### 蓝图 CustomThunk API（UDamageContextLibrary）

**重要**：蓝图中操作 DamageContext 必须通过 `UDamageContextLibrary` 的 static 函数，**不能**直接在 UDamageContext 上定义 CustomThunk 成员函数（会导致 Stack 解析错乱崩溃——因为没有配套的 K2Node）。

```cpp
// Public/DamagePipeline/DamageContextLibrary.h
UCLASS()
class UDamageContextLibrary : public UBlueprintFunctionLibrary {
    // 蓝图：直接传结构体，类型自动推断
    UFUNCTION(BlueprintCallable, CustomThunk, meta=(CustomStructureParam="Value"))
    static void SetEffect(UDamageContext* Context, const int32& Value);

    // 蓝图：Valid/NotValid 执行引脚 + 通配结构体输出
    UFUNCTION(BlueprintCallable, CustomThunk,
        meta=(CustomStructureParam="OutValue", ExpandEnumAsExecs="ExecResult"))
    static void GetEffect(UDamageContext* Context, EStructUtilsResult& ExecResult, int32& OutValue);
};
```

C++ 侧模板 API：`Context->SetEffect<T>(Val)` / `Context->GetEffect<T>()` / `Context->HasEffect<T>()`。
C++ 内部 API（运行时 UScriptStruct\*）：`SetEffectByType(FInstancedStruct)` / `GetEffectByType(UScriptStruct*)`。

## 编辑器扩展：DamagePipeline Graph Editor

位于 `Plugins/SagaStats/Source/SagaStatsEditor/`。

**自定义资产编辑器**：双击 DamagePipeline 资产打开 `FDamagePipelineAssetEditor`（左 Details + 右 Graph），Toolbar 的 Build 按钮触发拓扑排序并刷新 Graph 视图。

```
SagaStatsEditor/
├── Public/
│   ├── Graph/
│   │   ├── DamagePipelineGraph.h           — UEdGraph 子类，RebuildGraph 创建节点+连接
│   │   ├── DamagePipelineGraphNode.h       — UEdGraphNode，持有 UDamageRule 引用
│   │   └── DamagePipelineGraphSchema.h     — 只读 Schema + GetColorForEffectType 调色板
│   └── DamagePipelineCommands.h            — TCommands（Build 命令 + F7 快捷键）
└── Private/
    ├── Graph/
    │   ├── SDamagePipelineGraphNode.h/cpp   — Slate 节点外观（ASCII 树 Condition + 色块 + Description）+ NodeFactory
    │   ├── DamagePipelineConnectionDrawingPolicy.h/cpp — 电路板 Manhattan 折线 + 通道化 DropX
    │   ├── DamagePipelineLayoutConstants.h  — 布局常量（BaseGap / ChannelWidth / GapBetweenNodesY）
    │   └── DamagePipelineGraphSchema.cpp
    ├── AssetTypes/
    │   ├── DamagePipelineFactory.h/cpp      — 右键创建 DamagePipeline 资产
    │   ├── DamageRuleFactory.h/cpp          — 右键创建 DamageRule 资产
    │   ├── DamageConditionBlueprintFactory.h/cpp  — 右键创建 DamageCondition 蓝图类
    │   ├── DamageOperationBlueprintFactory.h/cpp  — 右键创建 DamageOperation 蓝图类
    │   └── DamageEffectStructFactory.h/cpp  — 右键创建 Effect 蓝图结构体（UDS）
    ├── DamagePipelineAssetEditor.h/cpp      — FAssetEditorToolkit（双面板 + Build + 真实尺寸布局）
    ├── DamagePipelineAssetTypeActions.h/cpp — 双击资产打开自定义编辑器
    └── DamagePipelineCommands.cpp
```

**阶梯式布局**：使用 Slate 真实尺寸驱动（`ApplyRealSizeLayoutCorrection`），每个节点 X/Y 单调递增，配合通道化 DropX 连线。RebuildGraph 只创建数据层（位置留 0,0），真实布局由 FTSTicker 在 Slate widget 就绪后一帧完成。

### Mermaid DAG 导出

`DamagePipeline::Execute()` 后自动生成 `.mmd` 文件到 `Saved/Graphs/`。节点显示 RuleName、Condition 文本、Produces Effect 类型。DC Init 节点显示非 DamageRule 产出的 Effect（攻击上下文），DC Final 显示 RuleName→TypeName 映射。

## CoreRedirects（重要）

任何 UClass/UStruct/UPROPERTY 的重命名操作**必须**在 `Plugins/SagaStats/Config/DefaultSagaStats.ini` 的 `[CoreRedirects]` 节中添加对应条目，否则蓝图资产引用会丢失。当前已包含 v4.5→v4.6 的完整重命名映射（DPU→DamageRule、Fact→Effect、Logic→Operation 等约60条）。

## Build.cs 依赖

**SagaStats Runtime**：Core, CoreUObject, Engine, Projects, NetCore, StructUtils, GameplayTags, GameplayTasks, GameplayAbilities

**SagaStatsEditor**：Core, AssetTools, SagaStats, StructUtils, BlueprintGraph, GraphEditor, UnrealEd, Slate, SlateCore, EditorStyle, EditorFramework, PropertyEditor, ToolWidgets 等

## 游戏变体（验证用）

| 变体 | 目录 | 关卡 |
|------|------|------|
| Combat | `Source/MySagaStats/Variant_Combat/` | `/Game/Variant_Combat/Lvl_Combat` |
| SideScrolling | `Source/MySagaStats/Variant_SideScrolling/` | `/Game/Variant_SideScrolling/Lvl_SideScrolling` |
| Platforming | `Source/MySagaStats/Variant_Platforming/` | `/Game/Variant_Platforming/Lvl_Platforming` |

测试 Actor：`ADamagePipelineTestActor`（按 1-5 键触发 5 个只狼场景）。

## 历史对话记录

项目包含从 Claude Web 导出的对话数据，位于 `chats/` 目录：

- `chats/L3_conversation_V2.md` — "L3动作游戏模块化流程设计V2"（405条消息），核心讨论声明式机制编排框架的迭代设计
- `chats/conversations.json` — 全部 59 个对话的原始 JSON（~40MB）

## 关键约束（R1-R6）

- **R1**：DC 是唯一通信介质，DamageRule 之间不直接调用
- **R2**：Operation 自洽——Execute 是纯函数（读 Context → 填 OutEffect），不知道自己的身份标识，不知道其他 Rule
- **R3**：Condition 可读 DC 全部内容，Effect 缺失视为 false（天然容错）
- **R4**：Condition 与 DamageRule 是装配关系（可替换不影响 Operation 内部）
- **R5**：执行顺序由 EffectType 产销关系自动决定（ProducesEffectType ↔ ConsumedEffectType 拓扑排序）
- **R6**：禁止循环依赖

## 设计方法论

- **Essential/Accidental Complexity** — 模型层只含本质复杂度，偶然复杂度属于实现层
- **三步检验** — 引入新概念时：①删除测试 ②来源追溯 ③换表达验证
- **内容域 vs 工具层** — 文档标题用内容域（模块化伤害管线），不用工具层（声明式编排框架）
- **双端协作** — 模型文档由网页 Claude 维护（v4.7），实现层由本地 Claude 维护（v4.6）。实现层架构变更需要输出 `实现层变更记录_vX.md` 同步给网页 Claude

## 工作流约定

1. **重命名 = 必须写 CoreRedirects**（蓝图资产兼容性）
2. **大型计划执行完毕后，启动独立 Agent 做审计**（检查命名一致性、Redirects 覆盖、删除完整性、编译警告）
3. **sed 批量替换要小心 for 循环变量**——替换类型名时不要改到循环变量名，否则会产生声明/引用不匹配的悬空引用

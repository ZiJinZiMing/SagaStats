# DamagePipeline Editor — L4 工具层文档

> **本目录范围**：DamagePipeline 的编辑器可视化实现（L4 工具层）。
> **不包含**：领域模型、设计哲学、DSL 语义——这些属于 L3 DSL 本体，见 `../damagePipeline/`。

---

## 为什么单独一个目录

DamagePipeline 的**核心本体**是一种领域 DSL（L3），由 `DamageRule + DamageEffect + DamageCondition + DamageContext` 构成，这些是**领域模型**。

DamagePipeline 的**编辑器**（Graph 视图、节点 Slate 外观、电路板连线、阶梯布局）是 L3 之上的**工具层**（L4），它让 L3 DSL 可视化和可编辑，但**它本身不是 DSL 的一部分**：

- 删掉编辑器，DamagePipeline 运行时行为不变
- 删掉 DamageRule，编辑器无内容可呈现

混在同一目录容易让新读者误把编辑器的 UX 决策当成 DSL 定义的一部分，违反层级分离原则。

---

## 文档清单

| 文件 | 内容 | 读者 |
|---|---|---|
| [GraphEditor实现.md](./GraphEditor实现.md) | 阶梯通道布局 / Manhattan 电路板走线 / EffectType 配色 / 真实尺寸驱动 | 编辑器程序 |

---

## 与 `doc/damagePipeline/` 的关系

| 关注点 | 去 `damagePipeline/` | 去 `damagePipelineEditor/`（本目录） |
|---|---|---|
| 什么是 DamageRule / DamageEffect / DamageCondition | ✅ | ❌ |
| 四条设计思想 / R1-R6 约束 | ✅ | ❌ |
| Build 拓扑排序算法原理 | ✅（执行引擎） | ❌ |
| Graph 节点在屏幕上的布局算法 | ❌ | ✅ |
| 连线走线如何避免重叠 | ❌ | ✅ |
| Condition 树的 ASCII 可视化 | ❌ | ✅ |

---

## 相关代码

- `Plugins/SagaStats/Source/SagaStatsEditor/` —— 整个编辑器模块
- `Plugins/SagaStats/Source/SagaStatsEditor/Private/Graph/` —— Graph 视图核心
- `Plugins/SagaStats/Source/SagaStatsEditor/Private/AssetTypes/` —— 资产创建工厂

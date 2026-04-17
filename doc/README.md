# 项目文档中心

> 完整技术文档

---

## 📚 文档目录

### 核心模块文档

#### 🗡️ DamagePipeline（模块化伤害管线）
**位置**: [damagePipeline/](./damagePipeline/)

动作游戏模块化伤害管线的 **L3 领域 DSL**（领域特定语言）实现。基于 DamageRule + DamageContext + DamageCondition 三大原语 + 自动拓扑排序，解决"玩法耦合 vs 代码解耦"的核心矛盾。当前版本：**v4.7**。

**快速链接**:
- [📖 模块概览（含 L3=DSL 客观定义论证）](./damagePipeline/README.md)
- [🚀 快速入门（含蓝图实战与 FAQ）](./damagePipeline/快速入门.md)
- [📐 架构设计（DSL 七要素纵向深挖）](./damagePipeline/架构设计.md)
- [🔍 术语速查表（按 DSL 七要素分类）](./damagePipeline/术语速查表.md)

**关键特性**:
- 四条设计思想（机制优先、数值兜底、反馈清晰可感、涌现自治）
- DamageRule + DC + Condition 三大原语
- 产销声明自动拓扑排序 + Kahn BFS 稳定版本
- DC 存储 `TMap<UScriptStruct*, FInstancedStruct>`——类型即 key
- 缺失容错（R3）支持动态管线

---

#### 🎨 DamagePipeline Editor（L4 工具层）
**位置**: [damagePipelineEditor/](./damagePipelineEditor/)

DamagePipeline 的 Graph 可视化编辑器——阶梯 Y + 通道 X 双重唯一性布局 + 3 段 Manhattan 走线 + EffectType 配色。**这是 L4 工具层，不是 DSL 本体**，L3 DSL 核心文档在 [damagePipeline/](./damagePipeline/)。

**快速链接**:
- [📖 模块概览（L4 层级定位）](./damagePipelineEditor/README.md)
- [🔧 GraphEditor 实现原理与代码清单](./damagePipelineEditor/GraphEditor实现.md)

---

#### 🔌 UltraWire（Graph Editor 连线插件调研）
**位置**: [ultraWire/](./ultraWire/)

Fab 商城插件 UltraWire 的纯学术设计哲学调研。重点是"代码架构如何承载设计哲学"——把 PCB 电路板的可读性美学植入 UE Graph Editor 的工程实践。

**快速链接**:
- [📖 模块概览（WHY/HOW/WHAT 总览）](./ultraWire/README.md)
- [📐 架构设计：HOW → WHAT 纵向深挖](./ultraWire/架构设计.md)
- [🔍 术语速查表](./ultraWire/术语速查表.md)

**关键洞察**:
- 四条设计命题（可读性第一、走线可插拔、效果分层、零侵入）
- 策略模式下的 4 种几何走线 + A\* 智能路由
- 5 个独立静态效果服务类（Glow / Bubble / Label / Crossing / Line）
- Slate 样式 snapshot-then-replace 的可逆主题引擎

---

## 🗂️ 文档组织

```
doc/
├── README.md                  # 本文件 - 文档中心索引
├── damagePipeline/            # L3 DSL 核心（v4.7）
│   ├── README.md              # 全员入口 + L3=DSL 论证 + 四设计思想
│   ├── 快速入门.md            # 蓝图实战与 FAQ
│   ├── 架构设计.md            # DSL 七要素纵向深挖
│   ├── 术语速查表.md          # 按 DSL 七要素分类的术语表
│   ├── 待讨论问题集.md        # 活跃讨论 Part A/B/C
│   └── _archive/              # 历史决策归档
│       ├── README.md
│       └── 已解决决策_v4.3-v4.7.md
├── damagePipelineEditor/      # L4 编辑器工具层（与 L3 DSL 核心分离）
│   ├── README.md              # L4 定位说明
│   └── GraphEditor实现.md     # 布局 / 走线 / 配色
└── ultraWire/                 # UltraWire 插件调研
```



## 📖 文档类型说明

### 📘 README.md
- 模块概览和导航
- 快速链接和目录
- 基本信息汇总

### 🚀 快速入门.md
- 5-10 分钟快速了解模块
- 核心概念速览
- 简单示例和常见模式
- 开发工作流

### 📐 架构设计.md
- 完整的技术架构文档
- 详细的类图和流程图
- 深入的设计原理说明
- 包含所有 mermaid 架构图

### 🔍 速查表.md
- 快速查询参考
- 按功能分类
- API 和接口列表
- 使用示例

---

## 🛠️ 文档工具

### Mermaid 图表
所有架构文档都使用 Mermaid 图表，支持：
- 流程图（flowchart）
- 类图（classDiagram）
- 序列图（sequenceDiagram）
- 思维导图（mindmap）
- 关系图（graph）

**查看方式**：
- GitHub 原生支持
- VS Code + Mermaid 插件
- 在线编辑器：https://mermaid.live/

---

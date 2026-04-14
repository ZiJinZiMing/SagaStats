# 项目文档中心

> 完整技术文档

---

## 📚 文档目录

### 核心模块文档

#### 🗡️ DamagePipeline（模块化伤害管线）
**位置**: [damagePipeline/](./damagePipeline/)

动作游戏模块化伤害管线的设计哲学深度调研。基于声明式机制编排框架（DPU + DC + Condition），解决"玩法耦合 vs 代码解耦"的核心矛盾。

**快速链接**:
- [📖 模块概览](./damagePipeline/README.md)
- [🚀 快速入门](./damagePipeline/快速入门.md)
- [📐 架构设计：代码如何承载设计哲学](./damagePipeline/架构设计.md)
- [🔍 术语速查表](./damagePipeline/术语速查表.md)

**关键特性**:
- 四条设计思想（机制优先、数值兜底、反馈清晰可感、涌现自治）
- DPU + DC + Condition 三概念模型
- 产销声明自动拓扑排序
- 固定管线 / 动态管线双模式

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
├── README.md              # 本文件 - 文档中心索引
├── damagePipeline/       # 模块化伤害管线（设计哲学调研）
│   ├── README.md         # 总览：设计哲学的核心命题
│   ├── 快速入门.md       # 5分钟理解核心概念
│   ├── 架构设计.md       # 重点：代码架构如何承载设计哲学
│   └── 术语速查表.md     # 术语、原则、案例快速参考
├── gameWorkspots/        # GameWorkspots 交互动画系统
│   ├── README.md         # 模块概览
│   ├── 架构设计.md       # 完整架构文档
│   ├── WorkspotTree详解.md # 动画树结构详解
│   └── 节点类型速查表.md # 所有节点类型查询
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

# 模块化伤害管线：DamageEffect 模型设计 v4.7

> **文档性质**：模型层补充文档。定义DamageEffect的概念、结构、与DamageRule/DamageCondition/DC的关系。基于模型规范v4.7。
> **解决的问题**：DamageRule产出从扁平键值对升级为有类型定义的结构体（DamageEffect），自带领域查询方法，可被DamageCondition结构化查询。
> **v3变更**：①术语体系更新（DPU→DamageRule，Fact→DamageEffect，Condition→DamageCondition，DPULogic→DamageOperation）。②DamageConditionNode按DamageRule分子类，直接承载领域方法（UFUNCTION反射自动发现）。③去掉FactMethodRegistry、FactQuery、Compare——领域方法和数值比较统一由DamageConditionNode子类承载。

---

## 一、核心概念

### 1.1 DamageEffect是什么

DamageEffect是DamageRule的**结构化产出**。每个DamageRule执行后产出一个DamageEffect，以DamageRule自身的身份标识存入DC。

当前阶段DamageEffect承载的是**条件数据**（供后续DamageRule的DamageCondition判断）。未来扩展将包含**表现信息**（Channel/Priority/资源引用，供表现选取阶段使用）。

| 概念 | 是什么 | 存在时机 | 示例 |
|------|--------|---------|------|
| **DamageEffect类型** | UStruct结构体定义——纯数据字段 | 编译时 | `FGuardEffect`、`FMixupEffect` |
| **DamageEffect实例** | DamageRule执行后创建的结构体实例 | 运行时 | `FGuardEffect{ Success=true, ... }` |

### 1.2 DamageEffect以DamageRule身份标识

DamageEffect在DC中的索引键就是产出它的**DamageRule的身份标识**（FName）。不存在独立的Key概念。

```
Rule_Mixup 执行后 → DC中存入 { "Mixup": FMixupEffect实例 }
Rule_Guard 执行后 → DC中存入 { "Guard": FGuardEffect实例 }
Rule_LightningInAir 执行后 → DC中存入 { "LightningInAir": FLightningInAirEffect实例 }
```

使用者的思考路径：**"我依赖Rule_Guard的结果"** → DamageCondition中引用"Guard"。

### 1.3 为什么不需要独立Key

v1版本中Key作为独立概念存在——允许多个互斥DamageRule共享同一个Key。

**三步检验**：

| 步骤 | 结果 |
|------|------|
| **删除测试** | 删掉独立Key → 消费者用OR组合多个DamageRule源 → 表达能力不丢失 |
| **来源追溯** | 从v1.7借来的概念，不是从当前使用者需求长出来的 |
| **换表达验证** | 换成GAS → GameplayAbility直接依赖其他Ability的产出，不存在中间Key层 |

**结论：独立Key是偶然复杂度。**

### 1.4 多个互斥DamageRule产出同类概念时的处理

```
场景：崩躯干有三种触发方式

Rule_Collapse         → 产出自己的DamageEffect（非格挡崩躯干）
Rule_CollapseGuard    → 产出自己的DamageEffect（格挡崩躯干）
Rule_CollapseJustGuard→ 产出自己的DamageEffect（弹刀崩躯干）

后续Rule_LightningCollapse依赖"崩躯干发生了"：
  DamageCondition:
    OR
    ├─ ConditionNode_Collapse.IsCollapsed
    ├─ ConditionNode_CollapseGuard.IsCollapsed
    └─ ConditionNode_CollapseJustGuard.IsCollapsed
```

规模膨胀（10+个互斥DamageRule）时，引入**汇总DamageRule**统一产出。

---

## 二、DamageEffect 定义

### 2.1 DamageEffect是纯数据结构

DamageEffect只包含**数据字段**，不包含领域方法。领域方法定义在对应的DamageConditionNode子类上（见第五章）。

```
FMixupEffect {
  ResultType: EMixupType        // Guard, JustGuard, Hurt, Mikiri, Jump, Lightning
  DamageElement: EDamageElement  // None, Lightning, Poison, Fire
  DamageLevel: int
  AttackPower: float
}
```

### 2.2 两种DamageEffect形态

| 形态 | 结构 | 示例 | DamageCondition中的引用 |
|------|------|------|----------------------|
| **复杂Effect** | 多字段结构体 | FMixupEffect, FGuardEffect | `ConditionNode_Mixup.IsGuard` |
| **信号Effect** | 纯存在性标记，无内部字段 | FLightningInAirEffect | `ConditionNode_LightningInAir`（存在即true） |

信号Effect对应的DamageConditionNode子类不需要领域方法——Evaluate直接返回DamageEffect是否存在。

---

## 三、DamageRule 的 Produces 声明

### 3.1 每个DamageRule产出一个DamageEffect

每个DamageRule声明自己产出的**DamageEffect类型**。DamageEffect在DC中以DamageRule的身份标识（FName）为索引存储。

```
Rule_Guard {
  produces: FGuardEffect
  DamageCondition: ConditionNode_Mixup.IsGuard
  DamageOperation: UOperation_Guard
}

Rule_LightningInAir {
  produces: FLightningInAirEffect
  DamageCondition: ConditionNode_Mixup.IsLightning && IsInAir
  DamageOperation: UOperation_LightningInAir
}
```

### 3.2 独占性规则

每个DamageRule产出自己的DamageEffect，以DamageRule身份标识为索引写入DC。不存在多个DamageRule写入同一个索引的情况。

---

## 四、DC 的 DamageEffect 存储

### 4.1 存储结构

DC以`TMap<FName, FInstancedStruct>`存储DamageEffect。Key是DamageRule的身份标识（FName），Value是该DamageRule产出的DamageEffect实例（FInstancedStruct）。

FInstancedStruct同时支持C++定义的USTRUCT和蓝图定义的UUserDefinedStruct。

### 4.2 读写操作

| 操作 | 谁做 | 说明 |
|------|------|------|
| **写入** | DamagePipeline执行DamageRule后 | DamageOperation返回DamageEffect → DamagePipeline以DamageRule身份标识写入DC |
| **读取** | 后续DamageRule的DamageOperation | 读前序DamageRule的DamageEffect作为输入 |
| **条件查询** | DamageConditionNode子类的Evaluate | 读取绑定的DamageEffect，调用领域方法 |

### 4.3 缺失处理

**DamageEffect缺失视为false**。DamageRule未加入管线或DamageCondition为false未执行时，该DamageRule的DamageEffect不存在于DC中。消费者引用时视为false——天然容错。

---

## 五、DamageCondition 条件树

### 5.1 条件树结构

DamageCondition以条件树形式配置，节点分为两类：

| 类别 | 节点 | 逻辑 |
|------|------|------|
| **复合节点** | AND / OR | 组合子节点的布尔逻辑 |
| **叶子节点** | 按DamageRule分的DamageConditionNode子类 | 读取对应DamageRule的DamageEffect，调用领域方法 |

所有节点都有**bReverse取反开关**。

### 5.2 DamageConditionNode按DamageRule分子类

**每个DamageRule对应一个DamageConditionNode子类。** 领域方法直接定义在子类上（UFUNCTION），方法列表通过UE5反射自动发现。

```
DamageConditionNode_Guard（绑定FGuardEffect）：
  领域方法（UFUNCTION）：
    IsParry()       → 读FGuardEffect，判断 Success && bIsJustGuard
    IsGuardBreak()  → 读FGuardEffect，判断 PostureChange <= 0

DamageConditionNode_Mixup（绑定FMixupEffect）：
  领域方法（UFUNCTION）：
    IsGuard()       → ResultType == Guard || JustGuard
    IsLightning()   → DamageElement == Lightning
    IsHeavyAttack() → DamageLevel > 2    ← 数值比较也包装为领域方法
```

**三者通过DamageEffect类型建立联系**：DamageRule produces FGuardEffect，DamageConditionNode_Guard绑定FGuardEffect。

### 5.3 领域方法的约束

| 约束 | 说明 |
|------|------|
| **纯函数** | 不修改DamageEffect数据，不修改DC |
| **返回Bool** | 与DamageCondition的布尔逻辑对齐 |
| **命名有领域含义** | IsParry而非ResultType==JustGuard |
| **数值比较也包装为领域方法** | IsHeavyAttack()替代DamageLevel>2——阈值修改集中 |

### 5.4 属性面板中的呈现

```
Rule_Death 的 DamageCondition：

  ▼ [AND]                                                           bReverse: □
    ├─ [ConditionNode_Mixup ▾] Method: [IsGuardSuccess ▾]              bReverse: ✓
    └─ [ConditionNode_LightningInAir ▾] Method: (无)                   bReverse: ✓

策划读法："博弈结果不是格挡成功，且不是空中接雷"
```

### 5.5 新建一套DamageRule的工作流

```
1. 新建DamageEffect结构体（数据字段）
   → FGuardEffect { Success, bIsJustGuard, PostureChange, ... }

2. 新建DamageConditionNode子类（绑定DamageEffect类型，定义领域方法）
   → UDamageConditionNode_Guard
     绑定：FGuardEffect
     UFUNCTION: IsParry(), IsGuardBreak()

3. 新建DamageRule定义（Produces=DamageEffect类型，DamageOperation，DamageCondition）

三者通过DamageEffect类型建立联系。不需要全局注册表。
蓝图扩展：蓝图继承UDamageConditionNode，蓝图函数天然是UFunction，反射路径和C++一致。
```

### 5.6 产销关系自动收集

每个DamageConditionNode子类通过绑定的DamageEffect类型反查哪个DamageRule produces它：

```
DamageConditionNode_Guard.GetConsumedRules()
  → 绑定FGuardEffect → 反查Rule_Guard → return ["Guard"]

AND/OR.GetConsumedRules()
  → 递归所有Children → 合并去重
```

---

## 六、与 AIToken Condition 系统的复用关系

| 复用部分 | AIToken | 伤害管线 |
|---------|---------|---------|
| 条件树骨架 | Predicate（Single/And/Or） | DamageConditionNode（AND/OR） |
| UClass标记 | DefaultToInstanced, EditInlineNew | 相同 |
| bReverse取反 | 基类统一处理 | 相同 |

| 扩展部分 | 差异 |
|---------|------|
| 上下文 | AIToken：固定结构。伤害管线：动态DamageEffect集合（DC） |
| 条件原子 | AIToken：每种判断一个子类。伤害管线：按DamageRule分子类，UFUNCTION反射发现领域方法 |
| 蓝图支持 | AIToken：C++固定结构。伤害管线：FInstancedStruct支持蓝图UUserDefinedStruct |

---

## 七、完整生命周期

```
开发阶段（DamageRule开发者）：
  1. 定义DamageEffect结构体（UStruct）—— 数据字段
  2. 创建DamageConditionNode子类 —— 绑定DamageEffect类型，定义领域方法（UFUNCTION）
  3. 创建DamageRule定义 —— Produces=DamageEffect类型，DamageOperation逻辑类
  4. 实现DamageOperation —— 读DC、执行逻辑、返回DamageEffect实例

配置阶段（管线使用者）：
  5. 属性面板中配置DamageCondition条件树
     → 选DamageConditionNode子类（类名=DamageRule名）→ 选领域方法（UFUNCTION下拉）
  6. 系统从条件树递归提取依赖的DamageRule列表
     → 自动建立DamageRule间依赖 → 自动拓扑排序

运行阶段：
  7. DamageOperation执行 → 返回DamageEffect实例
     → DamagePipeline以DamageRule身份标识写入DC
  8. 后续DamageRule的DamageCondition评估
     → DamageConditionNode子类读取绑定的DamageEffect → 调用领域方法
     → DamageEffect在DC中不存在 → 视为false（天然容错）
```

---

## 八、三步检验

| 概念 | 删除测试 | 来源 | 结论 |
|------|---------|------|------|
| DamageEffect类型（UStruct） | 删掉→产出只能是散装键值→丧失组织性 | 从Rule_Guard的复杂产出中长出来 | 本质 |
| DamageEffect以DamageRule身份标识 | 删掉→需要中间Key层→丧失心智模型对齐 | 从"使用者想说我依赖Rule_Guard"中长出来 | 本质 |
| ~~独立Key~~ | 删掉→OR组合表达→不丢失 | 从v1.7借来 | **偶然（已移除）** |
| DamageConditionNode按DamageRule分子类 | 删掉→回到通用节点+全局注册表→丧失直观性 | 从AIToken的Predicate子类模式长出来 | 本质 |
| 领域方法（UFUNCTION） | 删掉→用户拼字段比较→丧失易用性 | 从使用者用领域语言配置中长出来 | 本质 |
| ~~FactMethodRegistry~~ | 删掉→领域方法直接在ConditionNode上 | 从GAS借来 | **偶然（已移除）** |
| ~~FactQuery/Compare~~ | 删掉→被具体ConditionNode子类替代 | 被更直观的子类替代 | **偶然（已移除）** |
| OR组合多DamageRule源 | 删掉→多互斥Rule时无法表达 | 从崩躯干需求中长出来 | 本质 |
| 汇总DamageRule | 删掉→10+互斥Rule时OR膨胀 | 设计选择 | 偶然（规模优化） |

---

## 九、术语表

| 术语 | 定义 |
|------|------|
| **DamageEffect类型** | UStruct结构体定义。纯数据字段。由DamageRule开发者定义。 |
| **DamageEffect实例** | DamageRule执行后创建的结构体实例。以DamageRule身份标识写入DC。 |
| **DamageRule身份标识** | DamageRule的FName。DC中DamageEffect的索引键。 |
| **DamageConditionNode子类** | 按DamageRule分的条件树叶子节点。绑定DamageEffect类型，承载领域方法（UFUNCTION）。 |
| **领域方法** | DamageConditionNode子类上的UFUNCTION。返回Bool，名字有业务含义。数值比较也包装为领域方法。 |
| **信号DamageEffect** | 无内部字段的DamageEffect。存在即true。对应的ConditionNode子类不需要领域方法。 |
| **汇总DamageRule** | 多个互斥DamageRule产出同类概念时，可引入的统一结算节点。 |

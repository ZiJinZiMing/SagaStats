# 模块化伤害管线：Fact 模型设计 v2

> **文档性质**：模型层补充文档。定义Fact的概念、结构、与DPU/Condition/DC的关系。基于模型规范v4.5。
> **解决的问题**：DPU产出从扁平键值对升级为有类型定义的结构体（Fact），自带领域查询方法，可被Condition结构化查询。
> **v2变更**：去掉独立Fact Key概念。每个DPU产出一个以DPU自身为标识的Fact——使用者的思考路径从"我依赖哪个Key"变为"我依赖哪个DPU的结果"。多个互斥DPU产出同类概念时，消费者用OR组合表达。独立Fact Key经三步检验确认为偶然复杂度。

---

## 一、核心概念

### 1.1 Fact是什么

Fact是DPU的**结构化产出**。每个DPU执行后产出一个Fact，以DPU自身的身份标识存入DC。

| 概念 | 是什么 | 存在时机 | 示例 |
|------|--------|---------|------|
| **Fact类型** | UStruct结构体定义——数据字段+领域方法 | 编译时 | `FGuardResult`、`FMixupResult` |
| **Fact实例** | DPU执行后创建的结构体实例 | 运行时 | `FGuardResult{ Success=true, ... }` |

### 1.2 Fact以DPU身份标识

Fact在DC中的索引键就是产出它的**DPU的身份标识**（FName）。不存在独立的"Fact Key"概念。

```
DPU_Mixup 执行后 → DC中存入 { "Mixup": FMixupResult实例 }
DPU_Guard 执行后 → DC中存入 { "Guard": FGuardResult实例 }
DPU_LightningInAir 执行后 → DC中存入 { "LightningInAir": FLightningInAirResult实例 }
```

使用者的思考路径：**"我依赖DPU_Guard的结果"** → Condition中引用"Guard"。

### 1.3 为什么不需要独立Fact Key

v1版本中Fact Key作为独立概念存在——允许多个互斥DPU produces同一个Key（如DPU_Collapse和DPU_CollapseGuard共享"CollapseResult"）。

**三步检验**：

| 步骤 | 结果 |
|------|------|
| **删除测试** | 删掉独立Key → 消费者用OR组合多个DPU源（Collapse \|\| CollapseGuard）→ 表达能力不丢失 |
| **来源追溯** | 从v1.7 Fact Key概念"借来"的，不是从当前使用者需求"长出来"的 |
| **换表达验证** | 换成GAS → GameplayAbility直接依赖其他Ability的产出，不存在中间Key层 → 不再出现 |

**结论：独立Fact Key是偶然复杂度。** 它引入了使用者不需要的间接层——使用者要管理Key的命名、Key和DPU的绑定、Key和Fact类型的映射。实际上使用者只想说"我依赖DPU_Guard"。

### 1.4 多个互斥DPU产出同类概念时的处理

```
场景：崩躯干有三种触发方式

DPU_Collapse         → 产出自己的Fact（非格挡崩躯干）
DPU_CollapseGuard    → 产出自己的Fact（格挡崩躯干）
DPU_CollapseJustGuard→ 产出自己的Fact（弹刀崩躯干）

后续DPU_LightningCollapse依赖"崩躯干发生了"：
  Condition:
    OR
    ├─ FactQuery: Collapse.IsCollapsed
    ├─ FactQuery: CollapseGuard.IsCollapsed
    └─ FactQuery: CollapseJustGuard.IsCollapsed

三个互斥DPU运行时只有一个执行 → 其余两个的Fact不存在 → 缺失视为false → OR天然处理
```

**新增同类DPU时**：消费者的Condition需要加一个OR子项。这和v4.3中"新机制加入时调整已有DPU的Condition"是同一个操作模式。

**如果同类DPU数量膨胀（10+个）**：应引入一个"汇总DPU"统一产出结果（如DPU_PostureSettlement），消费者只依赖汇总节点。汇总节点本身是一个有明确职责的DPU，不是纯粹为了聚合而存在的空壳。

---

## 二、Fact 定义

### 2.1 Fact的两个组成部分

| 组成 | 是什么 | 谁定义 | 谁使用 |
|------|--------|--------|--------|
| **数据字段** | 机制处理的结果数据（Bool/Int/Float/Enum） | DPU开发者 | DPU内部逻辑、后续DPU |
| **领域方法** | 对数据字段的有业务含义的查询函数，返回Bool | DPU开发者 | Condition条件树 |

```
FMixupResult {
  // 数据字段
  ResultType: EMixupType        // Guard, JustGuard, Hurt, Mikiri, Jump, Lightning
  DamageElement: EDamageElement  // None, Lightning, Poison, Fire
  DamageLevel: int
  AttackPower: float
  
  // 领域方法（通过FactMethodRegistry注册）
  IsGuard()       → ResultType == Guard || ResultType == JustGuard
  IsJustGuard()   → ResultType == JustGuard
  IsLightning()   → DamageElement == Lightning
  IsHeavyAttack() → DamageLevel > 2
}
```

**领域方法的核心价值**：方法名就是条件的业务含义。使用者在Condition中看到的是`Guard.IsParry`——"DPU_Guard的结果是弹刀吗？"，不是`GetEnum("ResultType") == JustGuard`。

### 2.2 两种Fact形态

| 形态 | 结构 | 示例 | Condition中的引用 |
|------|------|------|------------------|
| **复杂Fact** | 多字段结构体+领域方法 | FMixupResult, FGuardResult | `Mixup.IsGuard` `Guard.IsParry` |
| **信号Fact** | 纯存在性标记，无内部字段 | FLightningInAirResult | `LightningInAir`（存在即true） |

信号Fact不需要领域方法——它的存在本身就是全部信息。

---

## 三、DPU 的 Produces 声明

### 3.1 每个DPU产出一个Fact

每个DPU声明自己产出的**Fact类型**。Fact在DC中以DPU的身份标识（FName）为索引存储。

```
DPU_Guard {
  produces: FGuardResult         ← Fact类型
  Condition: Mixup.IsGuard       ← 引用DPU_Mixup的产出
  逻辑：格挡判定
}

DPU_LightningInAir {
  produces: FLightningInAirResult
  Condition: Mixup.IsLightning && IsInAir
  逻辑：空中蓄电
}
```

Produces声明不需要单独的Fact Key——DPU名就是Fact在DC中的索引。

### 3.2 DPU身份标识→Fact类型的映射

系统需要知道每个DPU产出的Fact类型（UScriptStruct*），用途：

| 用途 | 为什么需要类型信息 |
|------|-----------------|
| 属性面板联动下拉 | 选了DPU名后，要查该Fact类型注册了哪些领域方法 |
| 运行时类型安全检查 | 检查DPU实际写入的类型是否与声明一致 |
| Compare节点反射访问 | 需要知道类型才能按字段名反射取值 |

这个映射自然存在于DPU的produces声明中——DPU声明了自己产出什么类型的Fact，系统从DPU池中收集即可。

### 3.3 独占性规则

> 每个DPU产出自己的Fact，以DPU身份标识为索引写入DC。同一个DPU身份在DC中只会有一个Fact。

不存在多个DPU写入同一个索引的情况——因为索引就是DPU自身。

---

## 四、DC 的 Fact 存储

### 4.1 存储结构与技术选型

DC以`TMap<FName, FInstancedStruct>`存储Fact。Key是DPU的身份标识（FName），Value是DPU产出的Fact实例（FInstancedStruct）。

**FInstancedStruct的选型理由**：UE5的FInstancedStruct允许同一个TMap中存储不同类型的结构体实例。它同时支持C++定义的USTRUCT和蓝图编辑器中创建的UUserDefinedStruct——DPU开发者可以在蓝图层定义新的Fact类型。

### 4.2 读写操作

| 操作 | 谁做 | C++路径 | 蓝图路径 |
|------|------|---------|---------|
| **写入Fact** | DPU执行后 | `DC->SetFact<T>(DPUName, Value)` 模板 | `DC->SetFact(DPUName, FInstancedStruct)` 通用 |
| **读取Fact数据** | 后续DPU内部逻辑 | `DC->GetFact<T>(DPUName)` 模板（类型安全） | `DC->GetFact(DPUName)` 返回FInstancedStruct（反射访问） |
| **查询领域方法** | Condition评估 | `DC->EvaluateFactMethod(DPUName, MethodName)` | 同左（统一入口） |
| **判断Fact存在** | Condition评估 | `DC->HasFact(DPUName)` | 同左 |

### 4.3 缺失Fact的处理

**Fact缺失视为false**。当DPU未加入当前管线（动态管线模式）或Condition为false未执行时，该DPU的Fact不存在于DC中。消费者引用时视为false——天然容错。

```
Condition引用"Guard.IsParry"：
  → DC中找"Guard" → 不存在（DPU_Guard未执行）→ 整个表达式视为false

Condition引用"!LightningInAir"：
  → DC中找"LightningInAir" → 不存在 → 视为false → !false = true
```

---

## 五、Condition 条件树

### 5.1 条件树结构

Condition在属性面板中以**条件树**形式配置（复用AIToken Condition系统的Predicate模式）：

```
节点类型：

  AND         — 所有子节点为true才true
  OR          — 任一子节点为true即true
  FactQuery   — 引用DPU名 + 领域方法名
  Compare     — 引用DPU名 + 字段名 + 运算符 + 值
  
  所有节点都有bReverse取反开关
```

### 5.2 属性面板中的呈现

```
DPU_Death 的 Condition：

  ▼ [AND]                                                 bReverse: □
    ├─ [FactQuery] DPU: [Mixup ▾] Method: [IsGuardSuccess ▾]  bReverse: ✓
    └─ [FactQuery] DPU: [LightningInAir ▾] Method: (无)       bReverse: ✓

等价于：!Mixup.IsGuardSuccess && !LightningInAir
策划读法："博弈结果不是格挡成功，且不是空中接雷"
```

```
DPU_LightningCollapse 的 Condition：

  ▼ [AND]
    ├─ [FactQuery] DPU: [Mixup ▾] Method: [IsLightning ▾]     bReverse: □
    └─ [OR]
       ├─ [FactQuery] DPU: [Collapse ▾] Method: [IsCollapsed ▾]
       ├─ [FactQuery] DPU: [CollapseGuard ▾] Method: [IsCollapsed ▾]
       └─ [FactQuery] DPU: [CollapseJustGuard ▾] Method: [IsCollapsed ▾]

等价于：Mixup.IsLightning && (Collapse.IsCollapsed || CollapseGuard.IsCollapsed || CollapseJustGuard.IsCollapsed)
策划读法："是雷电攻击，且发生了崩躯干（任意一种）"
```

### 5.3 FactQuery节点的联动下拉

```
用户选 DPU = "Guard"
→ 从DPU池中找到DPU_Guard → 其produces类型为FGuardResult
→ 从FactMethodRegistry中查到 FGuardResult 有 ["IsParry", "IsGuardBreak"]
→ Method下拉显示这两个选项
```

| 数据来源 | 提供什么 |
|---------|---------|
| **DPU池中的produces声明** | DPU名 → Fact类型 |
| **FactMethodRegistry** | Fact类型 → 方法名列表 |
| **Condition属性面板** | 串联两者，实现联动下拉 |

### 5.4 产销关系从条件树中递归收集

每个条件树节点实现GetConsumedDPUs()方法：

```
AND.GetConsumedDPUs()       → 递归所有Children → 合并去重
OR.GetConsumedDPUs()        → 同上
FactQuery.GetConsumedDPUs() → return { DPUName }
Compare.GetConsumedDPUs()   → return { DPUName }
```

**完全自动，无需手动声明依赖。**

### 5.5 产销关系匹配

```
DPU_Guard        身份标识: "Guard"
DPU_CollapseJG   Condition树中GetConsumedDPUs(): ["Guard"]
                 → "Guard"匹配DPU_Guard的身份标识
                 → 建立依赖边：DPU_Guard → DPU_CollapseJG
                 → 拓扑排序：DPU_Guard在DPU_CollapseJG之前执行
```

产销关系匹配直接用**DPU身份标识**——不需要中间的Key层。消费者依赖的就是DPU本身。

---

## 六、Fact 领域方法注册

### 6.1 FactMethodRegistry

全局方法注册表，映射(UScriptStruct*, FName MethodName) → 查询函数。

#### 两条注册路径

| 路径 | 适用场景 | 注册方式 | 类型安全 |
|------|---------|---------|---------|
| **C++路径** | C++定义的USTRUCT | `Register<T>(MethodName, lambda)` 模板 | 编译时 |
| **蓝图路径** | 蓝图定义的UUserDefinedStruct | `RegisterBP(UScriptStruct*, MethodName, UFunction*)` | 运行时 |

两条路径注册的方法在Registry中统一存储。CallMethod对调用者完全透明。

#### 调用链路

```
Condition评估：DC->EvaluateFactMethod("Guard", "IsParry")
  → DC从Facts中取出"Guard"的FInstancedStruct
  → Registry.CallMethod(FInstancedStruct, "IsParry")
    → 查注册表：(FGuardResult::StaticStruct(), "IsParry") → 查询函数
    → return bool
  → Fact不存在 → return false
```

### 6.2 InstancedStruct 兼容性

所有环节对C++和蓝图结构体都兼容：

| 环节 | C++结构体 | 蓝图结构体 |
|------|----------|-----------|
| DC存储 | ✅ | ✅ |
| DPU写入/读取 | ✅ 模板 | ✅ 反射 |
| FactQuery节点 | ✅ | ✅ |
| Compare节点 | ✅ | ✅ 统一走属性反射 |
| 领域方法注册 | ✅ 模板+lambda | ✅ RegisterBP+UFunction |
| Method下拉 | ✅ | ✅ |
| 产销关系收集 | ✅ | ✅ |

### 6.3 领域方法的约束

| 约束 | 说明 |
|------|------|
| **纯函数** | 不修改Fact数据，不修改DC，不修改任何外部状态 |
| **返回Bool** | 统一返回类型，与Condition的布尔逻辑对齐 |
| **命名有领域含义** | 方法名就是业务问题的自然语言表达 |
| **C++/蓝图均可定义** | C++用lambda，蓝图用BlueprintFunctionLibrary |

### 6.4 领域方法 vs Compare节点

| 方式 | 适用场景 | 示例 |
|------|---------|------|
| **领域方法** | 有明确业务含义的判断，可能涉及多个字段 | `IsParry()` = Success && IsJustGuard |
| **Compare节点** | 简单的单字段数值比较 | `DamageLevel > 2` |

---

## 七、完整生命周期

```
开发阶段（DPU开发者）：
  1. 定义Fact类型（UStruct）—— 数据字段
  2. 注册领域方法（FactMethodRegistry）—— 有业务含义的查询函数
  3. DPU声明produces的Fact类型
  4. DPU逻辑代码中构造Fact实例，以DPU身份标识写入DC

配置阶段（管线使用者）：
  5. 属性面板中配置Condition条件树
     → 选FactQuery → 选DPU名（下拉）→ 选领域方法（联动下拉）
  6. 系统从条件树递归提取依赖的DPU列表（GetConsumedDPUs）
     → 自动建立DPU间依赖 → 自动拓扑排序

运行阶段：
  7. DPU执行 → 构造Fact实例 → 以DPU名写入DC（FInstancedStruct）
  8. 后续DPU的Condition评估
     → 以DPU名从DC取Fact → 通过FactMethodRegistry调用领域方法
     → DPU名在DC中不存在 → 视为false（天然容错）
```

---

## 八、与 AIToken Condition 系统的复用关系

| 复用部分（骨架直接搬） | AIToken | 伤害管线 |
|----------------------|---------|---------|
| 条件树骨架 | Predicate（Single/And/Or） | ConditionNode（AND/OR） |
| UClass标记 | DefaultToInstanced, EditInlineNew | 相同 |
| bReverse取反 | Condition和Predicate两层 | ConditionNode基类 |
| 属性面板嵌套编辑 | 已验证可用 | 直接复用 |

| 扩展部分（新增） | 差异 |
|----------------|------|
| 上下文 | AIToken：固定结构。伤害管线：动态Fact集合（DC），FInstancedStruct存储 |
| 条件原子 | AIToken：每种判断一个子类。伤害管线：通用FactQuery节点+DPU名+MethodName |
| 方法分发 | AIToken：硬编码在子类中。伤害管线：FactMethodRegistry动态分发（C++/蓝图双路径） |
| 蓝图结构体支持 | AIToken：Context是C++固定结构。伤害管线：FInstancedStruct支持蓝图UUserDefinedStruct |

---

## 九、三步检验

| 概念 | 删除测试 | 来源 | 结论 |
|------|---------|------|------|
| Fact类型（UStruct） | 删掉→产出只能是散装键值→丧失组织性 | 从DPU_Guard的复杂产出中长出来 | 本质 |
| Fact以DPU身份标识 | 删掉→产销关系需要中间Key层→丧失使用者心智模型对齐 | 从"使用者想说我依赖DPU_Guard"中长出来 | 本质 |
| ~~独立Fact Key~~ | 删掉→消费者用OR组合多DPU源→表达能力不丢失 | 从v1.7借来，不是当前需求长出来的 | **偶然（v2已移除）** |
| 领域方法 | 删掉→用户拼GetBool/GetInt→丧失易用性 | 从"使用者用领域语言配置"中长出来 | 本质（易用性） |
| FactMethodRegistry | 删掉→领域方法无法动态分发→丧失扩展性 | 从"Fact类型是开放的"中长出来 | 本质（扩展性） |
| 条件树（AND/OR/FactQuery/Compare） | 删掉→回到字符串手写→丧失易用性 | 从AIToken已验证的Predicate模式搬来 | 偶然但高价值 |
| GetConsumedDPUs递归收集 | 删掉→手动声明依赖→丧失自动化 | 从R5（依赖关系自动排序）长出来 | 本质 |
| bReverse | 删掉→需要单独NOT节点→功能不丢 | 从AIToken搬来 | 偶然（语法糖） |
| OR组合多DPU源 | 删掉→多互斥DPU产出同类概念时无法表达→丧失表达力 | 从崩躯干三种来源的需求中长出来 | 本质 |
| 汇总DPU | 删掉→10+互斥DPU时OR膨胀→可用但不优雅 | 设计选择，非本质 | 偶然（规模大时的优化） |

---

## 十、术语表

| 术语 | 定义 |
|------|------|
| **Fact类型** | UStruct结构体定义。包含数据字段和领域方法。由DPU开发者定义。 |
| **Fact实例** | DPU执行后创建的结构体实例。以DPU身份标识写入DC。 |
| **DPU身份标识** | DPU的FName。同时用于：DC中Fact的索引键、Condition中的引用、产销关系匹配。 |
| **领域方法** | Fact上的查询函数。返回Bool，名字有业务含义。通过FactMethodRegistry注册。 |
| **信号Fact** | 无内部字段的Fact。存在即true，缺失即false。不需要领域方法。 |
| **FactMethodRegistry** | 全局方法注册表。映射(UScriptStruct*, MethodName)→查询函数。支持C++/蓝图双注册路径。 |
| **FactQuery** | 条件树原子节点。持有DPU名+MethodName，通过Registry调用领域方法。 |
| **Compare** | 条件树原子节点。持有DPU名+FieldName+比较运算符+比较值。 |
| **条件树** | Condition的配置结构。AND/OR/FactQuery/Compare嵌套。递归GetConsumedDPUs()自动收集依赖。 |
| **汇总DPU** | 多个互斥DPU产出同类概念时，可引入的统一结算节点。设计选择，非必须。 |

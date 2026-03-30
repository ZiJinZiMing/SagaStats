# 模块化伤害管线：Fact 模型设计 v1

> **文档性质**：模型层补充文档。定义Fact的概念、结构、与DPU/Condition/DC的关系。基于模型规范v4.4。
> **解决的问题**：DPU产出从扁平键值对升级为有类型定义的结构体（Fact），自带领域查询方法，可被Condition结构化查询。
> **v1变更**：①澄清Fact Key/Fact类型/Fact实例三个概念的精确定义和关系。②确认Key与类型一一对应，讨论"用类型自动派生Key"的简化方案（A/B），当前选择方案A。③FInstancedStruct兼容性检查——DC存储、FactMethodRegistry注册、Compare节点反射对C++和蓝图结构体均兼容。④FactMethodRegistry双注册路径：C++模板+lambda路径（编译时类型安全）和蓝图RegisterBP+UFunction路径（运行时反射）。

---

## 一、核心概念澄清

### 1.1 四个相关概念

| 概念 | 是什么 | 存在时机 | 示例 |
|------|--------|---------|------|
| **Fact类型** | UStruct结构体定义——有哪些字段、什么数据类型 | 编译时 | `FGuardResult`、`FMixupResult` |
| **Fact Key** | FName标识符——DC中的索引键，也是产销关系匹配的唯一标识 | 编译时 | `"GuardResult"`、`"MixupResult"` |
| **Fact实例** | DPU执行后创建的结构体实例——具体的数据 | 运行时 | `FGuardResult{ Success=true, IsJustGuard=true, ... }` |
| **Fact**（口语） | 泛指以上三者，根据上下文判断 | — | — |

### 1.2 Fact Key和Fact类型是一一对应的

一个Fact Key永远对应一个Fact类型，一个Fact类型也永远对应一个Fact Key。

```
"GuardResult"    ↔ FGuardResult     （永远）
"MixupResult"    ↔ FMixupResult     （永远）
"CollapseResult" ↔ FCollapseResult  （永远）
```

不存在"同一个Key有时候是FCollapseResult有时候是FDeathResult"的情况。

### 1.3 多个DPU可以produces同一个Fact Key

不同的DPU可以声明produces同一个Fact Key+类型——只要它们的Condition互斥，保证运行时只有一个实际写入。

```
DPU_Collapse      produces: { "CollapseResult": FCollapseResult }
DPU_CollapseGuard produces: { "CollapseResult": FCollapseResult }

Condition互斥：
  DPU_Collapse      → !(Guard && GuardSuccess)
  DPU_CollapseGuard → GuardSuccess && !IsJustGuard

后续消费者只引用"CollapseResult"——不关心是哪个DPU写入的。
```

---

## 二、Fact 定义

### 2.1 Fact的两个组成部分

每个Fact类型包含**数据字段**和**领域方法**：

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

**领域方法的核心价值**：方法名就是条件的业务含义。使用者在Condition中看到的是`IsGuard`、`IsLightning`，不是`GetEnum("ResultType") == Guard`。方法内部可能访问一个字段也可能访问多个——使用者不需要关心。

### 2.2 两种Fact形态

| 形态 | 结构 | 示例 | Condition中的引用 |
|------|------|------|------------------|
| **复杂Fact** | 多字段结构体+领域方法 | FMixupResult, FGuardResult | `MixupResult.IsGuard` |
| **信号Fact** | 纯存在性标记，无内部字段 | LightningInAirResult | `LightningInAirResult`（存在即true） |

信号Fact不需要领域方法——它的存在本身就是全部信息。

---

## 三、DPU 的 Produces 声明与 Fact Key-Type 映射

### 3.1 Produces声明

DPU的produces声明自己写入DC的**Fact Key列表**：

```
DPU_Guard {
  produces: ["GuardResult"]
  Condition: MixupResult.IsGuard
  逻辑：格挡判定
}

DPU_LightningInAir {
  produces: ["LightningInAirResult"]
  Condition: MixupResult.IsLightning && IsInAir
  逻辑：空中蓄电
}
```

Produces声明用于产销关系匹配——系统通过Fact Key将生产者DPU和消费者DPU的Condition关联起来。

### 3.2 Fact Key → Fact类型的映射

系统需要在某个地方知道Fact Key对应的Fact类型（UScriptStruct*），用途：

| 用途 | 为什么需要类型信息 |
|------|-----------------|
| 属性面板联动下拉 | 选了Fact Key后，要查该类型注册了哪些领域方法 |
| 运行时类型安全检查 | 检查DPU实际写入的类型是否与声明一致 |
| Compare节点反射访问 | 需要知道类型才能按字段名反射取值 |

**这个映射在哪声明是实现选择。** 可能的路径包括：

| 路径 | 做法 | 特点 |
|------|------|------|
| DPU produces中声明 | `produces: { "GuardResult": FGuardResult }` Key-Value形式 | 声明和使用在同一处，但DPU承担了额外职责 |
| FactMethodRegistry注册时顺带建立 | 注册方法时自动记录Fact Key→类型映射 | 零冗余，但要求方法注册先于Condition配置 |
| 独立的全局Fact注册表 | 集中管理所有Fact Key→类型映射 | 最清晰，但多一个维护点 |

模型层不指定具体路径——只要求系统在Condition配置时和运行时能查到此映射。

### 3.3 独占性规则

> 同一个Fact Key在同一次DC实例中只会被写入一次。

多个DPU可以声明produces同一个Fact Key（如DPU_Collapse和DPU_CollapseGuard），但它们的Condition必须互斥。系统可以在Debug模式下检测违规（同一个Fact Key被写入两次）。

---

## 四、DC 的 Fact 存储

### 4.1 存储结构与技术选型

DC以`TMap<FName, FInstancedStruct>`存储Fact。

**FInstancedStruct的选型理由**：UE5的FInstancedStruct允许同一个TMap中存储不同类型的结构体实例。它同时支持C++定义的USTRUCT和蓝图编辑器中创建的UUserDefinedStruct——DPU开发者可以在蓝图层定义新的Fact类型，不需要写C++。

### 4.2 读写操作

| 操作 | 谁做 | C++路径 | 蓝图路径 |
|------|------|---------|---------|
| **写入Fact** | DPU执行后 | `DC->SetFact<T>(Key, Value)` 模板 | `DC->SetFact(Key, FInstancedStruct)` 通用 |
| **读取Fact数据** | 后续DPU内部逻辑 | `DC->GetFact<T>(Key)` 模板（类型安全） | `DC->GetFact(Key)` 返回FInstancedStruct（反射访问） |
| **查询领域方法** | Condition评估 | `DC->EvaluateFactMethod(Key, MethodName)` | 同左（统一入口） |
| **判断Fact存在** | Condition评估 | `DC->HasFact(Key)` | 同左 |

C++路径通过模板提供编译时类型安全。蓝图路径通过FInstancedStruct直接操作，内部用UE5属性反射访问字段。两条路径对Condition评估层完全透明——EvaluateFactMethod不区分Fact是C++还是蓝图定义的。

### 4.3 缺失Fact的处理

**Fact缺失视为false**——与v4.4一致。

```
Condition引用"GuardResult.IsParry"：
  → DC中找"GuardResult" → 不存在 → 整个表达式视为false

Condition引用"!LightningInAirResult"：
  → DC中找"LightningInAirResult" → 不存在 → 视为false → !false = true
```

缺失判断在**Fact Key级别**。Fact存在但某字段不存在→DPU实现bug→应报错。

---

## 五、Condition 条件树

### 5.1 条件树结构

Condition在属性面板中以**条件树**形式配置（复用AIToken Condition系统的Predicate模式）：

```
节点类型：

  AND         — 所有子节点为true才true
  OR          — 任一子节点为true即true
  FactQuery   — 引用Fact的领域方法（FactName + MethodName）
  Compare     — 引用Fact的数据字段做比较运算（FactName + FieldName + 运算符 + 值）
  
  所有节点都有bReverse取反开关
```

### 5.2 属性面板中的呈现

```
DPU_Death 的 Condition：

  ▼ [AND]                                                    bReverse: □
    ├─ [FactQuery] Fact: [MixupResult ▾] Method: [IsGuardSuccess ▾]  bReverse: ✓
    └─ [FactQuery] Fact: [LightningInAirResult ▾] Method: (无)       bReverse: ✓

等价于：!MixupResult.IsGuardSuccess && !LightningInAirResult
```

### 5.3 FactQuery节点的联动下拉实现链路

```
用户选 FactName = "GuardResult"
→ 从Fact Key→类型映射中查到 "GuardResult" → FGuardResult（UScriptStruct*）
→ 从FactMethodRegistry中查到 FGuardResult 有 ["IsParry", "IsGuardBreak"]
→ MethodName下拉显示这两个选项
```

三个数据来源各司其职：

| 数据来源 | 提供什么 |
|---------|---------|
| **Fact Key→类型映射**（来源见3.2节） | Fact Key → Fact类型 |
| **FactMethodRegistry** | Fact类型 → 方法名列表 |
| **Condition属性面板** | 串联两者，实现联动下拉 |

### 5.4 产销关系从条件树中递归收集

每个条件树节点实现GetConsumedFacts()方法：

```
AND.GetConsumedFacts()       → 递归所有Children → 合并去重
OR.GetConsumedFacts()        → 同上
FactQuery.GetConsumedFacts() → return { FactName }
Compare.GetConsumedFacts()   → return { FactName }
```

**完全自动，无需手动声明consumes。**

### 5.5 产销关系匹配

```
DPU_Guard        produces中的Key: ["GuardResult"]
DPU_CollapseJG   Condition树中GetConsumedFacts(): ["GuardResult"]
                 → "GuardResult"匹配DPU_Guard的produces
                 → 建立依赖边：DPU_Guard → DPU_CollapseJG
                 → 拓扑排序：DPU_Guard在DPU_CollapseJG之前执行
```

匹配只看**Fact Key（FName）**——不看Fact类型，不看领域方法名。因为DPU要么整体执行要么整体不执行，Fact要么整体写入要么整体不存在。

---

## 六、Fact 领域方法注册

### 6.1 FactMethodRegistry

全局方法注册表，映射(UScriptStruct*, FName MethodName) → 查询函数。

#### 两条注册路径

因为Fact类型可能是C++定义的USTRUCT（编译时已知），也可能是蓝图编辑器中创建的UUserDefinedStruct（编译时不存在），Registry需要支持两条注册路径：

| 路径 | 适用场景 | 注册方式 | 类型安全 |
|------|---------|---------|---------|
| **C++路径** | C++定义的USTRUCT | `Register<T>(MethodName, lambda)` 模板 | 编译时 |
| **蓝图路径** | 蓝图定义的UUserDefinedStruct | `RegisterBP(UScriptStruct*, MethodName, UFunction*)` | 运行时 |

两条路径注册的方法在Registry中统一存储。CallMethod对调用者（Condition评估）完全透明——不区分方法是C++注册还是蓝图注册的。

```
C++ DPU开发者：
  Registry.Register<FGuardResult>("IsParry",
      [](const FGuardResult& R) { return R.Success && R.bIsJustGuard; });

蓝图 DPU开发者：
  1. 蓝图编辑器中定义UUserDefinedStruct
  2. BlueprintFunctionLibrary中写方法（输入FInstancedStruct，输出bool）
  3. Registry.RegisterBP(StructType, "IsParry", UFunction*)
```

#### 调用链路

```
Condition评估：DC->EvaluateFactMethod("GuardResult", "IsParry")
  → DC从Facts中取出"GuardResult"的FInstancedStruct
  → Registry.CallMethod(FInstancedStruct, "IsParry")
    → 查注册表：(UScriptStruct*, "IsParry") → 查询函数
    → 调用查询函数（不区分C++注册还是蓝图注册）
    → return bool
  → Fact不存在 → return false
```

#### 属性面板联动

```
查询可用方法（属性面板下拉）：
  Registry.GetMethodsForStruct(FGuardResult::StaticStruct())
  → ["IsParry", "IsGuardBreak"]
  → C++注册和蓝图注册的方法都会出现在列表中
```

### 6.2 InstancedStruct 兼容性

以下是Fact模型各环节对C++结构体和蓝图结构体的兼容性：

| 环节 | C++结构体 | 蓝图结构体 | 说明 |
|------|----------|-----------|------|
| DC存储（FInstancedStruct） | ✅ | ✅ | FInstancedStruct支持任意UStruct |
| DPU写入Fact | ✅ 模板 | ✅ 通用接口 | 两条路径 |
| DPU读取Fact | ✅ 模板（类型安全） | ✅ 反射 | C++用`GetFact<T>`，蓝图用`GetFact`+Break |
| Produces声明（UScriptStruct*） | ✅ | ✅ | UUserDefinedStruct继承自UScriptStruct |
| 条件树骨架（AND/OR） | ✅ | ✅ | 与Fact类型无关 |
| FactQuery节点 | ✅ | ✅ | 通过Registry统一调用 |
| Compare节点 | ✅ | ✅ | 统一走UE5属性反射（FProperty*） |
| 领域方法注册 | ✅ 模板+lambda | ✅ RegisterBP+UFunction | 两条注册路径 |
| MethodName下拉 | ✅ | ✅ | 两种注册方式都进入同一个Registry |
| 产销关系收集 | ✅ | ✅ | 只看Fact Key（FName），不看类型 |

**所有环节对C++和蓝图结构体都兼容。** 核心保障是：Registry的双注册路径，DC的双读写路径，Compare节点统一走UE5属性反射。

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
| **Compare节点** | 简单的单字段数值比较（走UE5属性反射，C++/蓝图结构体通用） | `DamageLevel > 2` |

---

## 七、完整生命周期

```
开发阶段（DPU开发者）：
  1. 定义Fact类型（UStruct）—— 数据字段
  2. 注册领域方法（FactMethodRegistry）—— 有业务含义的查询函数
  3. 建立Fact Key → Fact类型的映射（具体方式见3.2节）
  4. DPU声明produces的Fact Key列表
  5. DPU逻辑代码中构造Fact实例并通过Fact Key写入DC

配置阶段（管线使用者）：
  6. 属性面板中配置Condition条件树
     → 选FactQuery → 选Fact Key（下拉）→ 选领域方法（联动下拉）
  7. 系统从条件树递归提取consumes（Fact Key列表）
     → 与DPU池的produces匹配 → 自动建立依赖 → 自动拓扑排序

运行阶段：
  8. DPU执行 → 构造Fact实例 → 通过Fact Key写入DC（FInstancedStruct）
  9. 后续DPU的Condition评估
     → 通过Fact Key从DC取Fact → 通过FactMethodRegistry调用领域方法
     → Fact Key在DC中不存在 → 视为false（天然容错）
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
| 上下文 | AIToken：固定结构（Source+Holder）。伤害管线：动态Fact集合（DC），FInstancedStruct存储 |
| 条件原子 | AIToken：每种判断一个子类。伤害管线：通用FactQuery节点+Fact Key+MethodName配置 |
| 方法分发 | AIToken：硬编码在子类中。伤害管线：FactMethodRegistry动态分发（C++模板路径+蓝图反射路径） |
| Key→类型映射 | AIToken：不需要（Context固定）。伤害管线：从DPU produces声明中收集 |
| 蓝图结构体支持 | AIToken：Context是C++固定结构。伤害管线：FInstancedStruct支持蓝图定义的UUserDefinedStruct |

---

## 九、三步检验

| 概念 | 删除测试 | 来源 | 结论 |
|------|---------|------|------|
| Fact类型（UStruct） | 删掉→产出只能是散装键值→丧失组织性 | 从DPU_Guard的复杂产出中长出来 | 本质 |
| Fact Key（FName） | 删掉→产销关系无法匹配→拓扑排序断了 | 从v1.7 Fact Key概念成熟回归 | 本质 |
| Key与类型一一对应 | 违反→同一Key可能有不同类型→类型安全崩溃 | 从"崩躯干不管怎么触发都是CollapseResult"长出来 | 本质 |
| 领域方法 | 删掉→用户拼GetBool/GetInt→丧失易用性 | 从"使用者用领域语言配置"中长出来 | 本质（易用性） |
| FactMethodRegistry | 删掉→领域方法无法动态分发→丧失扩展性 | 从"Fact类型是开放的"中长出来 | 本质（扩展性） |
| 条件树（AND/OR/FactQuery/Compare） | 删掉→回到字符串手写→丧失易用性 | 从AIToken已验证的Predicate模式搬来 | 偶然但高价值 |
| GetConsumedFacts递归收集 | 删掉→手动声明consumes→丧失自动化 | 从R5（依赖关系自动排序）长出来 | 本质 |
| bReverse | 删掉→需要单独NOT节点→功能不丢 | 从AIToken搬来 | 偶然（语法糖） |

---

## 十、术语表

| 术语 | 定义 |
|------|------|
| **Fact类型** | UStruct结构体定义。包含数据字段。由DPU开发者定义。与Fact Key一一对应。 |
| **Fact Key** | FName标识符。DC中的索引键、produces声明的内容、Condition引用的标识、产销关系匹配的唯一依据。与Fact类型一一对应。 |
| **Fact实例** | DPU执行后创建的结构体实例。通过Fact Key写入DC。运行时存在。 |
| **领域方法** | Fact上的查询函数。返回Bool，名字有业务含义。通过FactMethodRegistry注册和调用。 |
| **信号Fact** | 无内部字段的Fact。存在即true，缺失即false。不需要领域方法。 |
| **FactMethodRegistry** | 全局方法注册表。映射(UScriptStruct*, MethodName)→查询函数。提供属性面板联动下拉的数据来源。 |
| **FactQuery** | 条件树的原子节点。持有Fact Key和MethodName，通过Registry调用领域方法。 |
| **Compare** | 条件树的原子节点。持有Fact Key、FieldName、比较运算符、比较值。 |
| **条件树** | Condition的配置结构。AND/OR/FactQuery/Compare节点嵌套。属性面板内联编辑。递归GetConsumedFacts()自动收集产销关系。 |

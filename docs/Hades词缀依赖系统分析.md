# Hades 词缀依赖系统分析文档

## 文档信息
- **文档标题**: 《Hades》词缀依赖关系系统深度分析
- **创建日期**: 2025-07-21
- **版本**: v1.0
- **用途**: 为SagaStats插件词缀依赖系统设计提供参考实现

## 概述

基于对《Hades》游戏源码文件 `UpgradeChoice.lua` 的深入分析，本文档详细记录了Hades中Boon（祝福/词缀）系统的依赖关系实现机制，为我们的词缀系统设计提供实战参考。

## 核心依赖系统架构

### 1. LinkedUpgrades 核心机制

Hades使用 `LinkedUpgrades` 表来定义复杂的词缀依赖关系：

```lua
-- 位置：UpgradeChoice.lua 第723-814行
function GetPriorityDependentTraits(lootData)
    if lootData.LinkedUpgrades == nil then
        return nil
    end
    
    local linkedTraits = {}
    for i, kvp in ipairs(orderedLinkedUpgrades) do
        local traitName = kvp.Key
        local dependencyTable = kvp.Value
        local valid = false
        
        -- OneOf 依赖检查
        if dependencyTable.OneOf ~= nil then
            for j, dependentTraitName in ipairs(dependencyTable.OneOf) do
                if HeroHasTrait(dependentTraitName) then
                    valid = true
                end
            end
        end
        
        -- OneFromEachSet 复合依赖检查
        if dependencyTable.OneFromEachSet ~= nil then
            valid = true
            for i, traitSet in ipairs(dependencyTable.OneFromEachSet) do
                local foundInSet = false
                for j, dependentTraitName in ipairs(dependencyTable.OneFromEachSet[i]) do
                    if HeroHasTrait(dependentTraitName) then
                        foundInSet = true
                    end
                end
                if not foundInSet then
                    valid = false
                    break
                end
            end
        end
        
        -- 优先级概率处理
        if valid and dependencyTable.PriorityChance ~= nil then
            table.insert(linkedTraits, { 
                TraitName = traitName, 
                PriorityChance = dependencyTable.PriorityChance 
            })
        end
    end
    
    return linkedTraits
end
```

### 2. 依赖关系的数据结构

Hades中的依赖表结构：

```lua
LinkedUpgrades = {
    ["DuoBoonName"] = {
        -- 基础依赖：需要拥有其中任意一个
        OneOf = { "ZeusTrait", "PoseidonTrait" },
        
        -- 复合依赖：需要从每个集合中至少拥有一个
        OneFromEachSet = {
            { "ZeusAttack", "ZeusSpecial", "ZeusCast" },    -- 宙斯技能组
            { "PoseidonDash", "PoseidonCall" }              -- 波塞冬技能组
        },
        
        -- 满足依赖后的出现概率
        PriorityChance = 0.8
    }
}
```

### 3. 品质升级系统

```lua
-- 位置：UpgradeChoice.lua 第610-623行
function GetUpgradedRarity(baseRarity)
    local rarityTable = {
        Common = "Rare",
        Rare = "Epic", 
        Epic = "Heroic"
    }
    
    -- 支持运行时替换品质升级表
    if HasHeroTraitValue("ReplaceUpgradedRarityTable") then
        rarityTable = GetHeroTraitValues("ReplaceUpgradedRarityTable")[1]
    end
    
    return rarityTable[baseRarity]
end
```

### 4. 槽位互斥机制

```lua
-- 位置：UpgradeChoice.lua 第636-684行
function GetReplacementTraits(traitNames, onlyFromLootName)
    local occupiedSlots = {}
    
    -- 扫描已占用的槽位
    for i, traitData in pairs(CurrentRun.Hero.Traits) do
        if traitData.Slot then
            if not occupiedSlots[traitData.Slot] then
                occupiedSlots[traitData.Slot] = { 
                    TraitName = traitData.Name, 
                    Rarity = "Common" 
                }
            end
            
            -- 记录该槽位的最高品质
            if traitData.Rarity ~= nil and 
               GetRarityValue(traitData.Rarity) > GetRarityValue(occupiedSlots[traitData.Slot].Rarity) then
                occupiedSlots[traitData.Slot].Rarity = traitData.Rarity
            end
        end
    end
    
    -- 生成替换选项
    for index, traitName in ipairs(traitNames) do
        local slot = TraitData[traitName].Slot
        if occupiedSlots[slot] and GetUpgradedRarity(occupiedSlots[slot].Rarity) ~= nil then
            local data = { 
                ItemName = traitName, 
                Type = "Trait", 
                TraitToReplace = occupiedSlots[slot].TraitName, 
                OldRarity = occupiedSlots[slot].Rarity, 
                Rarity = GetUpgradedRarity(occupiedSlots[slot].Rarity) 
            }
            table.insert(priorityOptions, data)
        end
    end
    
    return { GetRandomValue(priorityOptions) }
end
```

## 关键设计模式分析

### 1. 依赖验证的层次结构

**第一层：基础依赖 (OneOf)**
- 用途：双神Boon需要两个神祇中任意一个的基础Boon
- 逻辑：满足列表中任意一个条件即可
- 实现：简单的OR逻辑

**第二层：复合依赖 (OneFromEachSet)**
- 用途：传奇Boon需要满足多个不同类型的前置条件
- 逻辑：每个集合都必须至少满足一个条件
- 实现：嵌套的AND/OR逻辑

**第三层：品质依赖**
- 用途：高级Boon要求前置Boon达到特定品质
- 逻辑：检查现有Boon的稀有度是否满足要求
- 实现：品质等级比较

### 2. 优先级和概率控制

```lua
-- 满足依赖条件的词缀获得出现优先权
if valid and dependencyTable.PriorityChance ~= nil then
    table.insert(linkedTraits, { 
        TraitName = traitName, 
        PriorityChance = dependencyTable.PriorityChance 
    })
end
```

**设计意图**：
- 确保玩家在满足依赖后能合理概率地获得高级词缀
- 避免依赖词缀过于稀有导致构建路径断裂
- 提供可调节的平衡参数

### 3. 词缀替换机制

Hades实现了同槽位词缀的升级替换：

```lua
-- 检查是否为替换升级
if upgradeData.TraitToReplace then
    local numOldTrait = GetTraitNameCount(CurrentRun.Hero, upgradeData.TraitToReplace)
    RemoveWeaponTrait(upgradeData.TraitToReplace)
    AddTraitToHero({ TraitData = upgradeData })
    
    -- 保持原有的叠加层数
    for i = 1, numOldTrait - 1 do
        AddTraitToHero({ TraitName = upgradeData.Name })
    end
end
```

## 对SagaStats系统的设计启发

### 1. 数据结构设计

```cpp
USTRUCT(BlueprintType)
struct FSagaAffixDependency 
{
    // OneOf模式：需要其中任意一个
    UPROPERTY(EditAnywhere, Category = "Basic Dependencies")
    TArray<FSagaAffixReference> OneOfRequired;
    
    // OneFromEachSet模式：需要每个集合中至少一个
    UPROPERTY(EditAnywhere, Category = "Complex Dependencies")
    TArray<FSagaAffixSet> OneFromEachSetRequired;
    
    // 品质要求
    UPROPERTY(EditAnywhere, Category = "Quality Dependencies")
    TMap<FSagaAffixReference, ESagaAffixRarity> QualityRequirements;
    
    // 优先级概率
    UPROPERTY(EditAnywhere, Category = "Priority", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PriorityChance = 1.0f;
};

USTRUCT(BlueprintType)
struct FSagaAffixSet
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSagaAffixReference> AffixesInSet;
};
```

### 2. 依赖验证算法

```cpp
class SAGASTATS_API USagaAffixDependencyValidator
{
public:
    // 基于Hades逻辑的依赖验证
    UFUNCTION(BlueprintCallable, Category = "Saga Affix")
    static bool ValidateAffixDependencies(
        const FSagaAffixDependency& Dependency,
        const USagaAbilitySystemComponent* ASC)
    {
        // OneOf 验证
        if (!Dependency.OneOfRequired.IsEmpty())
        {
            bool bHasOneOf = false;
            for (const FSagaAffixReference& RequiredAffix : Dependency.OneOfRequired)
            {
                if (ASC->HasAffix(RequiredAffix))
                {
                    bHasOneOf = true;
                    break;
                }
            }
            if (!bHasOneOf) return false;
        }
        
        // OneFromEachSet 验证
        for (const FSagaAffixSet& AffixSet : Dependency.OneFromEachSetRequired)
        {
            bool bHasFromSet = false;
            for (const FSagaAffixReference& AffixInSet : AffixSet.AffixesInSet)
            {
                if (ASC->HasAffix(AffixInSet))
                {
                    bHasFromSet = true;
                    break;
                }
            }
            if (!bHasFromSet) return false;
        }
        
        // 品质要求验证
        for (const auto& QualityPair : Dependency.QualityRequirements)
        {
            const USagaAffixBase* ExistingAffix = ASC->GetAffix(QualityPair.Key);
            if (!ExistingAffix || ExistingAffix->GetRarity() < QualityPair.Value)
            {
                return false;
            }
        }
        
        return true;
    }
};
```

### 3. 双神词缀实现

```cpp
UCLASS(Blueprintable)
class SAGASTATS_API USagaDuoAffix : public USagaAffixBase
{
    GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duo Affix")
    FSagaAffixReference PrimaryGodAffix;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duo Affix")  
    FSagaAffixReference SecondaryGodAffix;
    
public:
    virtual bool CanBeOffered_Implementation(const USagaAbilitySystemComponent* ASC) const override
    {
        // 基于Hades逻辑：检查两个神祇的词缀是否都存在
        return ASC->HasAnyAffixFromCategory(PrimaryGodCategory) && 
               ASC->HasAnyAffixFromCategory(SecondaryGodCategory);
    }
};
```

## 实现建议

### 1. 渐进式实现策略

**第一阶段：基础依赖**
- 实现OneOf依赖模式
- 建立基础的依赖验证框架
- 支持简单的前置条件检查

**第二阶段：复合依赖**
- 实现OneFromEachSet模式
- 添加品质依赖检查
- 支持复杂的依赖组合

**第三阶段：高级功能**
- 实现优先级概率系统
- 添加词缀替换机制
- 完善双神词缀逻辑

### 2. 数据配置方案

```json
{
    "LegendaryAffix_Example": {
        "Dependencies": {
            "OneOf": ["FireGod_BasicAffix", "WaterGod_BasicAffix"],
            "OneFromEachSet": [
                ["FireGod_Attack", "FireGod_Special"],
                ["WaterGod_Defense", "WaterGod_Utility"]
            ],
            "QualityRequirements": {
                "FireGod_BasicAffix": "Rare"
            },
            "PriorityChance": 0.8
        }
    }
}
```

### 3. 性能优化考虑

- **缓存依赖检查结果**：避免重复计算
- **增量更新**：只在词缀变化时重新验证
- **预计算可用词缀**：提前筛选满足条件的词缀池

## 总结

Hades的词缀依赖系统展现了以下核心设计原则：

1. **简单而强大的模型**：OneOf和OneFromEachSet两种模式足以表达复杂依赖
2. **数据驱动设计**：所有依赖关系通过配置定义，便于调整和扩展
3. **合理的概率控制**：通过PriorityChance保证游戏体验的流畅性
4. **优雅的替换机制**：支持同槽位词缀的品质升级
5. **层次化验证逻辑**：从简单到复杂的依赖检查顺序

这套系统为我们的SagaStats词缀依赖设计提供了经过实战验证的参考方案，既保证了系统的灵活性，又确保了实现的可行性。

---

## 参考文件
- **源文件**: `D:\SteamLibrary\steamapps\common\Hades\Content\Scripts\UpgradeChoice.lua`
- **分析日期**: 2025-07-21
- **相关文档**: `游戏词缀系统设计文档.md`
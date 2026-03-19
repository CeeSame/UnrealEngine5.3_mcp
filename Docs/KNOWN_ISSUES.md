# Known Issues & TODO

> Last updated: 2026-03-19

## Critical Issues

### 1. Widget Blueprint 操作导致引擎崩溃

**问题描述**:
- 使用 MCP 命令操作 Widget Blueprint 时会导致 UE 引擎崩溃
- 影响命令: `find_blueprint_nodes`, `add_blueprint_event_node`, `add_blueprint_function_node` 等

**根本原因**:
1. `FindOrCreateEventGraph()` 函数假设蓝图有 `EventGraph`，但 Widget Blueprint 使用 `WidgetGraph`
2. `FindBlueprintByName()` 硬编码了 `/Game/Blueprints/` 路径前缀

**相关文件**:
- `UnrealMCP/Source/UnrealMCP/Private/Commands/UnrealMCPCommonUtils.cpp:160-180` - `FindOrCreateEventGraph()`
- `UnrealMCP/Source/UnrealMCP/Private/Commands/UnrealMCPCommonUtils.cpp:154-158` - `FindBlueprintByName()`
- `UnrealMCP/Source/UnrealMCP/Private/Commands/UnrealMCPBlueprintNodeCommands.cpp` - 所有节点操作命令

**修复方案**:
```cpp
// UnrealMCPCommonUtils.cpp - FindOrCreateEventGraph()
UEdGraph* FUnrealMCPCommonUtils::FindOrCreateEventGraph(UBlueprint* Blueprint)
{
    if (!Blueprint) return nullptr;

    // 检查是否是 Widget Blueprint
    if (Blueprint->IsA<UWidgetBlueprint>())
    {
        // Widget Blueprint 使用 WidgetGraph
        UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(Blueprint);
        if (WidgetBP->WidgetGraph)
        {
            return WidgetBP->WidgetGraph;
        }
        // 如果没有，创建一个
        // ...
    }

    // 普通 Blueprint 使用 UbergraphPages
    for (UEdGraph* Graph : Blueprint->UbergraphPages)
    {
        if (Graph->GetName().Contains(TEXT("EventGraph")))
        {
            return Graph;
        }
    }
    // ...
}

// UnrealMCPCommonUtils.cpp - FindBlueprintByName()
UBlueprint* FUnrealMCPCommonUtils::FindBlueprintByName(const FString& BlueprintName)
{
    // 如果已经是完整路径，直接使用
    if (BlueprintName.StartsWith(TEXT("/Game/")) ||
        BlueprintName.StartsWith(TEXT("/Engine/")))
    {
        return LoadObject<UBlueprint>(nullptr, *BlueprintName);
    }

    // 否则使用默认路径
    FString AssetPath = TEXT("/Game/Blueprints/") + BlueprintName;
    return LoadObject<UBlueprint>(nullptr, *AssetPath);
}
```

**优先级**: High

---

### 2. 蓝图路径查找问题

**问题描述**:
- `FindBlueprintByName()` 只支持 `/Game/Blueprints/` 前缀
- 传入完整路径 `/Game/BP_getTrackersPos_hy/MovieRenderQuene_BurnIn` 时会查找失败

**相关文件**:
- `UnrealMCP/Source/UnrealMCP/Private/Commands/UnrealMCPCommonUtils.cpp:154-158`

**修复方案**: 见上方代码

**优先级**: Medium

---

## Enhancement TODO

### 1. 添加 Widget Blueprint 专用支持

- [ ] 添加 `UWidgetBlueprint` 类型检测
- [ ] 实现 `FindOrCreateWidgetGraph()` 函数
- [ ] 支持 Widget 特有事件（`OnConstruct`, `OnPaint` 等）

### 2. 增强路径处理

- [ ] 支持完整资产路径
- [ ] 支持相对路径
- [ ] 添加路径验证和错误提示

### 3. 添加更多 MRQ 功能

- [ ] `SetMRQSettings` - 设置 MRQ 渲染配置
- [ ] `StartMRQRender` - 启动 MRQ 渲染
- [ ] `GetMRQJobStatus` - 获取渲染任务状态

---

## Workarounds

### Widget Blueprint 修改

由于 Widget Blueprint 操作会导致崩溃，当前建议：

1. **手动修改**: 在 UE 编辑器中手动打开蓝图进行修改
2. **使用 C++**: 创建 C++ 类来实现功能，然后在蓝图中继承
3. **创建新蓝图**: 使用 MCP 创建新的普通 Blueprint，然后在其中调用 Widget 相关功能

### 蓝图路径问题

如果蓝图不在 `/Game/Blueprints/` 路径下：

1. 将蓝图移动到 `/Game/Blueprints/` 路径
2. 或使用 `list_assets` 命令确认资产存在后，手动修改 C++ 代码中的路径

---

## Testing Checklist

修复后需要测试的场景：

- [ ] 在 `/Game/Blueprints/` 路径下创建/修改普通 Blueprint
- [ ] 在非标准路径下创建/修改普通 Blueprint
- [ ] 创建/修改 Widget Blueprint
- [ ] 添加节点到 Widget Blueprint 的 Event Construct
- [ ] 调用自定义 Blueprint Function Library 函数
- [ ] 连接节点并编译 Widget Blueprint

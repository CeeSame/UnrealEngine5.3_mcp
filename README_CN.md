# UnrealMCP - UE5.3 移植版

> 基于 [chongdashu/unreal-mcp](https://github.com/chongdashu/unreal-mcp) 移植，原版本为 UE5.5

## 项目概述

UnrealMCP 是一个为 Unreal Engine 提供的 **Model Context Protocol (MCP)** 实现插件，允许 AI 助手（如 Claude Code、Cursor、Windsurf）通过 MCP 协议控制 Unreal Engine 编辑器。

### 主要功能

- **TCP 服务器**: 默认端口 `55557`，监听 AI 助手连接
- **编辑器面板**: 可视化显示连接状态、服务器地址和端口
- **Actor 操作**: 创建、删除、修改 Actor 及其属性，支持直接设置 StaticMesh
- **Blueprint 操作**: 创建蓝图、添加组件、设置属性、编译蓝图
- **Blueprint 节点**: 添加事件节点、函数调用、变量、连接节点
- **UMG 界面**: 创建 Widget、添加控件、绑定事件
- **输入映射**: 创建输入动作映射
- **MRQ 工具**: 获取 Movie Render Queue 设置、时间膨胀信息、关卡序列信息（独立于 MCP，支持离线使用）

## 快速开始

### 前置要求

- **Unreal Engine 5.3**
- **Visual Studio 2022** + C++ 工具链 (MSVC v143)
- **Python 3.10+** (用于 MCP 客户端)

### 安装步骤

1. **清理编译缓存** (如果之前用其他 UE 版本打开过):
   ```bash
   rm -rf MCPGameProject/.vs
   rm -rf MCPGameProject/Binaries
   rm -rf MCPGameProject/Intermediate
   rm -rf MCPGameProject/Saved
   rm -rf MCPGameProject/DerivedDataCache
   rm -rf MCPGameProject/Plugins/UnrealMCP/Binaries
   rm -rf MCPGameProject/Plugins/UnrealMCP/Intermediate
   rm -f MCPGameProject/*.sln
   ```

2. **打开项目**:
   - 右键 [MCPGameProject.uproject](MCPGameProject/MCPGameProject.uproject) → **Generate Visual Studio project files**
   - 打开 `MCPGameProject.sln`，编译 (Ctrl+Shift+B)
   - 双击 `MCPGameProject.uproject` 启动编辑器

3. **验证插件加载**:
   - 打开 **Window → Output Log**
   - 查找: `UnrealMCPBridge: Server started on 127.0.0.1:55557`

### 配置 MCP 客户端

**安装 Python 依赖**:
```bash
py -m pip install fastmcp
```

**添加 MCP 服务器到 Claude Code**:
```bash
claude mcp add unreal -- py E:/VibeCoding/unreal-mcp/Python/unreal_mcp_server.py
```

**验证连接**:
```bash
claude mcp list
# 应显示: unreal: ... ✓ Connected
```

## 编辑器面板

插件提供了一个可视化控制面板：

**打开方式**: 菜单 **Window → UnrealMCP**

**面板功能**:
- 显示连接状态 (Connected/Disconnected)
- 显示服务器地址和端口
- 启动/停止服务器按钮
- 使用说明

```
┌─────────────────────────────────────┐
│       UnrealMCP Control Panel       │
├─────────────────────────────────────┤
│  Status:    Connected (绿色)         │
│  Address:   127.0.0.1               │
│  Port:      55557                   │
├─────────────────────────────────────┤
│  [Start Server]  [Stop Server]      │
└─────────────────────────────────────┘
```

## 项目结构

```
unreal-mcp/
├── MCPGameProject/                 # UE 项目
│   ├── Plugins/
│   │   └── UnrealMCP/              # MCP 插件
│   │       ├── Source/UnrealMCP/
│   │       │   ├── Private/
│   │       │   │   ├── Commands/           # 命令处理器
│   │       │   │   │   ├── UnrealMCPEditorCommands.cpp
│   │       │   │   │   ├── UnrealMCPBlueprintCommands.cpp
│   │       │   │   │   └── UnrealMCPMRQLibrary.cpp  # MRQ 工具库
│   │       │   │   ├── UnrealMCPBridge.cpp    # 主桥接类
│   │       │   │   ├── MCPServerRunnable.cpp  # TCP 服务器线程
│   │       │   │   ├── UnrealMCPModule.cpp    # 模块入口 + Tab 注册
│   │       │   │   └── UnrealMCPPanel.cpp     # Slate UI 面板
│   │       │   └── Public/
│   │       │       ├── UnrealMCPBridge.h
│   │       │       ├── UnrealMCPModule.h
│   │       │       ├── UnrealMCPPanel.h
│   │       │       └── Commands/
│   │       │           └── UnrealMCPMRQLibrary.h  # MRQ 工具库头文件
│   │       └── UnrealMCP.uplugin
│   └── MCPGameProject.uproject
├── Python/                         # Python MCP 客户端
│   ├── unreal_mcp_server.py        # FastMCP 服务器
│   └── tools/                      # MCP 工具模块
└── README_CN.md                    # 本文件
```

## 可用命令

### Editor 命令
| 命令 | 说明 |
|------|------|
| `get_actors_in_level` | 获取关卡中所有 Actor |
| `find_actors_by_name` | 按名称查找 Actor |
| `spawn_actor` | 生成 Actor (支持 asset_path 参数) |
| `delete_actor` | 删除 Actor |
| `set_actor_transform` | 设置 Actor 变换 |
| `get_actor_properties` | 获取 Actor 属性 |
| `set_actor_property` | 设置 Actor 属性 |
| `focus_viewport` | 聚焦视口 |
| `take_screenshot` | 截图 |
| `get_project_info` | 获取项目信息（名称、路径、引擎版本、当前关卡） |
| `list_assets` | 列出项目资产（支持 path、filter、recursive 参数） |
| `get_asset_details` | 获取资产详情（支持蓝图信息） |

### spawn_actor 示例

创建带 Cube mesh 的 StaticMeshActor:
```json
{
  "type": "spawn_actor",
  "params": {
    "type": "StaticMeshActor",
    "name": "MyCube",
    "location": [0, 0, 100],
    "asset_path": "/Engine/BasicShapes/Cube.Cube"
  }
}
```

### Blueprint 命令
| 命令 | 说明 |
|------|------|
| `create_blueprint` | 创建蓝图 |
| `add_component_to_blueprint` | 添加组件到蓝图 |
| `set_component_property` | 设置组件属性 |
| `set_physics_properties` | 设置物理属性 |
| `compile_blueprint` | 编译蓝图 |
| `set_blueprint_property` | 设置蓝图属性 |
| `set_static_mesh_properties` | 设置静态网格属性 |
| `set_pawn_properties` | 设置 Pawn 属性 |

### Blueprint Node 命令
| 命令 | 说明 |
|------|------|
| `connect_blueprint_nodes` | 连接蓝图节点 |
| `add_blueprint_event_node` | 添加事件节点 |
| `add_blueprint_function_node` | 添加函数调用节点 |
| `add_blueprint_variable` | 添加变量 |
| `add_blueprint_input_action_node` | 添加输入动作节点 |
| `add_blueprint_self_reference` | 添加 Self 引用 |
| `find_blueprint_nodes` | 查找蓝图节点 |

### UMG 命令
| 命令 | 说明 |
|------|------|
| `create_umg_widget_blueprint` | 创建 UMG Widget |
| `add_text_block_to_widget` | 添加文本块 |
| `add_button_to_widget` | 添加按钮 |
| `bind_widget_event` | 绑定控件事件 |

### MRQ Library (Blueprint Function Library)

**独立于 MCP，支持离线使用**。可在任意蓝图中直接调用，无需 MCP 连接。

| 函数 | 类别 | 输出结构体 | 说明 |
|------|------|-----------|------|
| `GetMRQSettings` | UnrealMCP\|MRQ | FMRQSettingsData | 获取 MRQ 渲染设置（分辨率、帧率、抗锯齿） |
| `GetTimeDilationInfo` | UnrealMCP\|MRQ | FTimeDilationData | 获取时间膨胀信息（播放速率） |
| `GetLevelSequenceInfo` | UnrealMCP\|MRQ | FLevelSequenceData | 获取关卡序列详细信息 |
| `GenerateMRQExportJson` | UnrealMCP\|MRQ | String | 生成完整 MRQ 导出 JSON |
| `IsMRQRenderingActive` | UnrealMCP\|MRQ | Boolean | 检查是否正在 MRQ 渲染 |
| `GenerateTrackerExportJson` | UnrealMCP\|MRQ | String | 生成追踪点 + MRQ 设置的完整 JSON |

#### 结构体定义

**FMRQSettingsData**:
- `ResolutionWidth` (Int32) - 渲染分辨率宽度
- `ResolutionHeight` (Int32) - 渲染分辨率高度
- `FrameRate` (Float) - 输出帧率
- `AntiAliasingMethod` (String) - 抗锯齿方法 (None, FXAA, TAA, MSAA, TSR)
- `SpatialSamples` (Int32) - 空间采样数
- `TemporalSamples` (Int32) - 时间采样数

**FTimeDilationData**:
- `bEnabled` (Boolean) - 是否启用时间膨胀
- `Value` (Float) - 时间膨胀值 (1.0 = 正常, 0.5 = 半速, 2.0 = 双速)

**FLevelSequenceData**:
- `Name` (String) - 序列名称
- `Path` (String) - 序列路径
- `PlaybackStart` (Int32) - 播放起始帧
- `PlaybackEnd` (Int32) - 播放结束帧
- `DisplayRate` (Float) - 显示帧率
- `bTimeDilationEnabled` (Boolean) - 是否启用时间膨胀
- `TimeDilationValue` (Float) - 时间膨胀值

#### 蓝图使用示例

在 Burn-in Widget 或其他蓝图中：

1. **获取 MRQ 设置**:
   - 节点类别: `UnrealMCP | MRQ`
   - 调用 `GetMRQSettings`，传入 `WorldContextObject` (Self)
   - 返回值检查是否成功，输出结构体包含设置数据

2. **生成完整导出 JSON**:
   - 调用 `GenerateTrackerExportJson`
   - 传入追踪点名称数组、位置数组、总帧数
   - 直接获得完整 JSON 字符串用于文件写入

#### 导出 JSON 格式

```json
{
  "Anim": [{"trackerpoint": {"x": 317.0, "y": -648.1}}],
  "NullLayers": ["trackerpoint"],
  "MRQSettings": {
    "resolution": {"width": 1920, "height": 1080},
    "frame_rate": 30.0,
    "anti_aliasing": {"method": "MSAA", "spatial_samples": 4, "temporal_samples": 1}
  },
  "TimeDilation": {"enabled": true, "value": 0.5},
  "LevelSequence": {
    "name": "LS_Seq_CutTest",
    "path": "/Game/BP_getTrackersPos_hy/LS_Seq_CutTest",
    "playback_start": 0,
    "playback_end": 100,
    "display_rate": 30.0,
    "time_dilation_enabled": true,
    "time_dilation_value": 0.5
  }
}
```

## API 兼容性说明

| API | UE5.3 状态 | 说明 |
|-----|-----------|------|
| Core APIs | ✅ 兼容 | Actor, Component, UObject |
| Socket/Networking | ✅ 兼容 | ISocketSubsystem, FSocket |
| JSON | ✅ 兼容 | FJsonObject, FJsonSerializer |
| EditorSubsystem | ✅ 兼容 | UEditorSubsystem |
| Blueprint Nodes | ✅ 兼容 | UK2Node_* 系列 |
| Property System | ✅ 兼容 | FProperty, FBoolProperty 等 |
| Slate UI | ✅ 兼容 | SCompoundWidget, Nomad Tab |

## 故障排除

### 问题: "modules are missing or built with a different engine version"
**解决**: 清理编译缓存后重新打开项目
```bash
rm -rf MCPGameProject/Intermediate MCPGameProject/Binaries MCPGameProject/Saved
```

### 问题: "Unable to build while Live Coding is active"
**解决**: 关闭 UE 编辑器后再编译，或按 Ctrl+Alt+F11 禁用 Live Coding

### 问题: Marketplace 插件冲突
**解决**: 清理 UE 缓存
```powershell
rm -r "$env:LOCALAPPDATA\UnrealEngine\Intermediate\Build\BuildRules"
```

### 问题: MCP 连接失败
**解决**:
1. 确认 UE 编辑器正在运行
2. 检查 Output Log 中是否有 `Server started on 127.0.0.1:55557`
3. 确认 Python 已安装 fastmcp: `py -m pip install fastmcp`

## 相关链接

- [原仓库](https://github.com/chongdashu/unreal-mcp)
- [Model Context Protocol](https://modelcontextprotocol.io/)
- [UE5.3 Release Notes](https://docs.unrealengine.com/5.3/en-US/unreal-engine-5-3-release-notes/)

## 已知限制

### 多 UE 编辑器实例支持 (待定)

**当前状态**: 仅支持单个 UE 编辑器实例

**问题描述**:
- 插件使用固定端口 `55557`
- 如果同时打开多个 UE 编辑器，只有第一个启动的能绑定端口
- 后续实例会因端口被占用而无法启动 MCP 服务器
- MCP 客户端无法选择连接哪个 UE 实例

**待实现方案**:
1. **动态端口分配** - 每个 UE 实例自动使用不同端口
2. **可配置端口** - 通过配置文件或命令行参数指定端口
3. **项目名称识别** - MCP 客户端通过项目名称匹配连接目标
4. **多连接管理** - 支持同时管理多个 UE 连接

---

## 更新日志

- **2026-03-19**: 添加 MRQ Library 功能
  - 新增 [UnrealMCPMRQLibrary](MCPGameProject/Plugins/UnrealMCP/Source/UnrealMCP/Private/Commands/UnrealMCPMRQLibrary.cpp) Blueprint Function Library
  - 支持 MRQ 渲染时获取设置（分辨率、帧率、抗锯齿）
  - 支持获取时间膨胀信息和关卡序列信息
  - 独立于 MCP，支持离线使用
  - 纯蓝图项目可直接调用
  - 更新 [UnrealMCP.Build.cs](MCPGameProject/Plugins/UnrealMCP/Source/UnrealMCP/UnrealMCP.Build.cs) 添加 MovieRenderPipelineCore, LevelSequence 模块依赖
- **2026-03-18**: 添加项目结构读取功能
  - 新增 `get_project_info` 命令：获取项目名称、路径、引擎版本、当前关卡
  - 新增 `list_assets` 命令：列出项目资产，支持路径过滤和递归搜索
  - 新增 `get_asset_details` 命令：获取资产详情，包括蓝图父类和变量数量
  - 更新 [UnrealMCPEditorCommands](MCPGameProject/Plugins/UnrealMCP/Source/UnrealMCP/Private/Commands/UnrealMCPEditorCommands.cpp)
  - 更新 [UnrealMCPBridge](MCPGameProject/Plugins/UnrealMCP/Source/UnrealMCP/Private/UnrealMCPBridge.cpp) 添加命令路由
- **2026-03-18**: 从 UE5.5 移植到 UE5.3
  - 修改 [UnrealMCP.Build.cs](MCPGameProject/Plugins/UnrealMCP/Source/UnrealMCP/UnrealMCP.Build.cs): `IWYUSupport` → `bEnforceIWYU`
  - 添加 `EngineVersion: 5.3.0` 到 [UnrealMCP.uplugin](MCPGameProject/Plugins/UnrealMCP/UnrealMCP.uplugin)
  - 修复 [Target.cs](MCPGameProject/Source/MCPGameProject.Target.cs) 版本 API 兼容性
  - 添加 `spawn_actor` 的 `asset_path` 参数支持
  - 添加编辑器控制面板 [UnrealMCPPanel](MCPGameProject/Plugins/UnrealMCP/Source/UnrealMCP/Private/UnrealMCPPanel.cpp)
  - 修复 Python MCP 脚本 FastMCP API 兼容性

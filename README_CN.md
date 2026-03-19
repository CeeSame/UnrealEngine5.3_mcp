# UnrealMCP - UE5.3 移植版

Unreal Engine 5.3 MCP 插件，允许 AI 助手（Claude Code、Cursor、Windsurf）通过 Model Context Protocol 控制 UE 编辑器。

## 功能特性

| 类别 | 功能 |
|------|------|
| **Actor 管理** | 创建/删除 Actor、设置变换、查询属性 |
| **蓝图开发** | 创建蓝图、添加组件、设置属性、编译 |
| **蓝图节点** | 添加事件/函数节点、变量、连接节点 |
| **UMG 界面** | 创建 Widget、添加控件、绑定事件 |
| **MRQ 工具** | 获取渲染设置、时间膨胀、序列信息（离线可用） |
| **编辑器面板** | 可视化连接状态、服务器控制 |

## 目录结构

```
UnrealEngine5.3_mcp/
├── UnrealMCP/          # UE 插件
│   ├── UnrealMCP.uplugin
│   └── Source/UnrealMCP/
├── Python/             # MCP 客户端
│   ├── unreal_mcp_server.py
│   ├── tools/
│   └── scripts/
└── Docs/               # 文档
```

## 快速开始

### 前置要求
- Unreal Engine 5.3
- Visual Studio 2022 + MSVC v143
- Python 3.10+

### 安装插件

1. 将 `UnrealMCP/` 复制到项目的 `Plugins/` 目录
2. 右键 `.uproject` → Generate Visual Studio project files
3. 编译项目，启动编辑器
4. 验证：Output Log 显示 `Server started on 127.0.0.1:55557`

### 配置 MCP 客户端

```bash
pip install fastmcp
claude mcp add unreal -- python <path>/Python/unreal_mcp_server.py
```

### MCP 配置示例

```json
{
  "mcpServers": {
    "unreal": {
      "command": "python",
      "args": ["<path>/Python/unreal_mcp_server.py"]
    }
  }
}
```

## 编辑器面板

**打开方式**: 菜单 **Window → UnrealMCP**

## MRQ 蓝图函数库

独立于 MCP，可在蓝图中直接调用（节点类别：`UnrealMCP | MRQ`）：

| 函数 | 说明 |
|------|------|
| `GetMRQSettings` | 获取 MRQ 渲染设置（分辨率、帧率、抗锯齿） |
| `GetTimeDilationInfo` | 获取时间膨胀信息 |
| `GetLevelSequenceInfo` | 获取关卡序列信息 |
| `GenerateTrackerExportJson` | 生成追踪点 + MRQ 设置的完整 JSON |

## 可用命令

<details>
<summary>Editor 命令</summary>

| 命令 | 说明 |
|------|------|
| `get_actors_in_level` | 获取关卡所有 Actor |
| `find_actors_by_name` | 按名称查找 Actor |
| `spawn_actor` | 生成 Actor |
| `delete_actor` | 删除 Actor |
| `set_actor_transform` | 设置变换 |
| `get/set_actor_property` | 获取/设置属性 |
| `get_project_info` | 获取项目信息 |
| `list_assets` | 列出资产 |

</details>

<details>
<summary>Blueprint 命令</summary>

| 命令 | 说明 |
|------|------|
| `create_blueprint` | 创建蓝图 |
| `add_component_to_blueprint` | 添加组件 |
| `set_component_property` | 设置组件属性 |
| `compile_blueprint` | 编译蓝图 |

</details>

<details>
<summary>Blueprint Node 命令</summary>

| 命令 | 说明 |
|------|------|
| `connect_blueprint_nodes` | 连接节点 |
| `add_blueprint_event_node` | 添加事件节点 |
| `add_blueprint_function_node` | 添加函数节点 |
| `add_blueprint_variable` | 添加变量 |

</details>

<details>
<summary>UMG 命令</summary>

| 命令 | 说明 |
|------|------|
| `create_umg_widget_blueprint` | 创建 Widget |
| `add_text_block_to_widget` | 添加文本块 |
| `add_button_to_widget` | 添加按钮 |
| `bind_widget_event` | 绑定事件 |

</details>

## 故障排除

| 问题 | 解决方案 |
|------|----------|
| 模块版本不匹配 | 清理 `Intermediate/`、`Binaries/` 后重新编译 |
| MCP 连接失败 | 确认 UE 运行且 Output Log 显示服务器已启动 |
| Live Coding 冲突 | 关闭编辑器后编译，或禁用 Live Coding |

## License

MIT

---

## 致谢

基于 [chongdashu/unreal-mcp](https://github.com/chongdashu/unreal-mcp) 移植（原版 UE5.5 → UE5.3）

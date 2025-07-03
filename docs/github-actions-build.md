# GitHub Actions 自动编译指南

本文档介绍如何使用GitHub Actions在云端自动编译小智ESP32固件，无需本地安装ESP-IDF开发环境。

## 🚀 功能特性

### 自动编译系统
- **多平台支持**: ESP32、ESP32-S3、ESP32-C3
- **多开发板支持**: 70+种开发板配置
- **多AI模型支持**: Xiaozhi、OpenAI、Google、Anthropic
- **自动发布**: 编译完成后自动打包发布

### 工作流类型
1. **构建测试** (`build.yml`) - 代码推送时自动编译
2. **发布构建** (`release.yml`) - 创建正式发布版本
3. **AI模型测试** (`test-ai-models.yml`) - 测试AI模型配置

## 📋 支持的编译配置

### 开发板类型
- `bread-compact-wifi` - 面包板WiFi版
- `esp-box-3` - 乐鑫ESP-BOX-3
- `m5stack-core-s3` - M5Stack CoreS3
- `lichuang-dev` - 立创开发板
- `atoms3r-echo-base` - AtomS3R Echo Base
- `xmini-c3` - 虾哥Mini C3
- `bread-compact-esp32` - ESP32面包板

### AI模型类型
- `xiaozhi` - 小智官方服务器
- `openai` - OpenAI ChatGPT
- `google` - Google Gemini
- `anthropic` - Anthropic Claude
- `custom` - 自定义服务器

## 🔧 使用方法

### 方法1: 自动触发编译

当您推送代码到main分支或创建Pull Request时，GitHub Actions会自动开始编译：

```bash
git push origin main
```

编译完成后，可以在Actions页面下载编译好的固件。

### 方法2: 手动触发编译

1. 进入GitHub仓库页面
2. 点击 `Actions` 标签
3. 选择 `Build and Test` 工作流
4. 点击 `Run workflow` 按钮
5. 选择编译参数：
   - **Board Type**: 选择开发板类型
   - **AI Model**: 选择AI模型提供商
6. 点击 `Run workflow` 开始编译

### 方法3: 创建发布版本

创建新的Git标签来触发发布编译：

```bash
# 创建标签
git tag v1.8.0
git push origin v1.8.0
```

或者手动触发发布工作流：

1. 进入 `Actions` → `Create Release`
2. 点击 `Run workflow`
3. 输入版本号 (如 v1.8.0)
4. 选择是否为预发布版本
5. 开始编译和发布

## 📁 编译产物

### 固件文件
编译完成后会生成以下文件：
- `xiaozhi-{version}-{chip}-{board}-{ai_model}.bin` - 主固件文件
- `xiaozhi-{version}-{chip}-{board}-{ai_model}.elf` - 调试文件

### 下载方式
1. **Actions页面下载**:
   - 进入Actions页面
   - 点击对应的编译任务
   - 在Artifacts部分下载固件

2. **Release页面下载**:
   - 进入Releases页面
   - 下载最新发布的固件包

## 🔍 编译状态查看

### 实时查看编译过程
1. 进入Actions页面
2. 点击正在运行的工作流
3. 点击具体的编译任务
4. 查看实时日志输出

### 编译结果
- ✅ **成功**: 绿色勾号，可下载固件
- ❌ **失败**: 红色叉号，查看错误日志
- 🟡 **进行中**: 黄色圆点，正在编译

## ⚙️ 自定义编译配置

### 修改编译矩阵
编辑 `.github/workflows/build.yml` 文件中的matrix配置：

```yaml
strategy:
  matrix:
    include:
      - target: esp32s3
        board: your-custom-board
        ai_model: openai
        description: "自定义配置"
```

### 添加新的开发板
1. 在 `main/boards/` 目录下添加开发板配置
2. 更新 `main/Kconfig.projbuild` 添加配置选项
3. 在GitHub Actions工作流中添加编译配置

## 🧪 测试功能

### AI模型连接测试
手动触发AI模型测试工作流：

1. 进入 `Actions` → `Test AI Models`
2. 点击 `Run workflow`
3. 选择要测试的AI模型
4. 输入设备IP地址（如果有实际设备）
5. 开始测试

### 配置验证
测试工作流会自动验证：
- 配置文件完整性
- Python测试脚本语法
- 文档文件存在性
- 编译配置正确性

## 📊 编译统计

### 构建矩阵
每次编译会测试多种配置组合：
- **8种主要配置** (不同开发板+AI模型组合)
- **3种芯片平台** (ESP32, ESP32-S3, ESP32-C3)
- **自动化测试** (代码质量检查)

### 编译时间
- 单个配置编译时间: ~5-10分钟
- 完整矩阵编译时间: ~30-60分钟
- 并行编译: 最多同时运行20个任务

## 🔧 故障排除

### 常见编译错误

1. **配置错误**
   ```
   Error: Unknown board type
   ```
   解决：检查开发板名称是否正确

2. **依赖缺失**
   ```
   Error: Component not found
   ```
   解决：检查submodules是否正确初始化

3. **内存不足**
   ```
   Error: Not enough memory
   ```
   解决：优化配置或使用更小的组件

### 调试方法
1. 查看详细的编译日志
2. 检查配置文件内容
3. 对比成功的编译配置
4. 在本地复现编译环境

## 💡 最佳实践

### 开发流程
1. **本地开发**: 在feature分支开发新功能
2. **推送测试**: 推送到GitHub触发自动编译
3. **创建PR**: 创建Pull Request进行代码审查
4. **合并发布**: 合并到main分支并创建release

### 版本管理
- 使用语义化版本号 (如 v1.8.0)
- 在CHANGELOG中记录重要变更
- 为重大更新创建预发布版本

### 资源优化
- 避免频繁推送触发不必要的编译
- 使用draft PR进行开发中的代码
- 定期清理旧的编译产物

## 🔗 相关链接

- [GitHub Actions文档](https://docs.github.com/en/actions)
- [ESP-IDF CI Action](https://github.com/espressif/esp-idf-ci-action)
- [项目Actions页面](https://github.com/DuanxianshengGO/xiaozhi-esp32/actions)

通过GitHub Actions，您可以轻松地在云端编译固件，无需本地安装复杂的开发环境，大大简化了开发和部署流程！

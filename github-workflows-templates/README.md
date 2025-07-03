# GitHub Actions 工作流模板

本目录提供了小智ESP32项目的GitHub Actions工作流配置指南。由于GitHub权限限制，工作流文件需要在PR合并后手动创建。

## 📁 工作流说明

### 核心工作流
- **`build.yml`** - 主要的构建和测试工作流
  - 多平台编译 (ESP32, ESP32-S3, ESP32-C3)
  - 多开发板支持 (70+种配置)
  - 多AI模型编译 (Xiaozhi, OpenAI, Google, Anthropic)
  - 自动化测试和质量检查

- **`release.yml`** - 发布管理工作流
  - 自动创建GitHub Releases
  - 固件打包和命名
  - 发布说明自动生成
  - 多格式下载支持

- **`test-ai-models.yml`** - AI模型测试工作流
  - 配置文件验证
  - API连接测试
  - 设备连接测试
  - 定时自动测试

## 📥 获取工作流文件

由于GitHub OAuth权限限制，工作流文件无法直接包含在PR中。您可以通过以下方式获取：

### 方法1: 从PR分支获取
```bash
# 克隆仓库并切换到feature分支
git clone https://github.com/DuanxianshengGO/xiaozhi-esp32.git
cd xiaozhi-esp32
git checkout feature/github-actions-workflows

# 复制工作流文件
mkdir -p .github/workflows
cp .github/workflows/*.yml .github/workflows/
```

### 方法2: 手动创建
参考 `docs/github-actions-build.md` 中的完整工作流配置，手动创建文件。

## 🚀 安装步骤

### 方法1: 使用设置脚本 (推荐)
```bash
# 在项目根目录运行
./scripts/setup_github_actions.sh
```

### 方法2: 手动创建
1. 确保您有仓库的管理员权限
2. 创建 `.github/workflows/` 目录 (如果不存在)
3. 从feature分支或文档中获取工作流内容
4. 创建对应的 `.yml` 文件
5. 提交并推送到仓库

```bash
# 创建目录
mkdir -p .github/workflows

# 从feature分支获取工作流文件
git fetch origin feature/github-actions-workflows
git checkout feature/github-actions-workflows -- .github/workflows/

# 或者手动创建文件 (参考文档中的配置)
# 创建 build.yml, release.yml, test-ai-models.yml

# 提交更改
git add .github/workflows/
git commit -m "ci: Add GitHub Actions workflows"
git push origin main
```

## ⚙️ 权限配置

安装工作流后，需要配置以下权限：

### 1. Actions 权限
进入仓库 Settings → Actions → General:
- ✅ Allow all actions and reusable workflows
- ✅ Allow actions created by GitHub

### 2. 工作流权限
进入仓库 Settings → Actions → Workflow permissions:
- ✅ Read and write permissions
- ✅ Allow GitHub Actions to create and approve pull requests

### 3. Pages 配置 (可选)
进入仓库 Settings → Pages:
- ✅ Source: GitHub Actions
- 用于自动部署文档

## 🔧 使用方法

### 自动触发
- **推送到main分支**: 自动运行构建测试
- **创建Pull Request**: 自动运行构建测试
- **创建Git标签**: 自动创建发布

### 手动触发
1. 进入仓库的 Actions 页面
2. 选择要运行的工作流
3. 点击 "Run workflow" 按钮
4. 选择参数 (如开发板类型、AI模型等)
5. 开始执行

## 📊 构建矩阵

工作流支持以下配置组合：

| 芯片 | 开发板 | AI模型 | 描述 |
|------|--------|--------|------|
| ESP32-S3 | bread-compact-wifi | xiaozhi | 面包板WiFi版 |
| ESP32-S3 | bread-compact-wifi | openai | 面包板WiFi版 + OpenAI |
| ESP32-S3 | esp-box-3 | xiaozhi | ESP-BOX-3 |
| ESP32-S3 | m5stack-core-s3 | google | M5Stack CoreS3 + Gemini |
| ESP32-S3 | lichuang-dev | anthropic | 立创开发板 + Claude |
| ESP32-C3 | xmini-c3 | xiaozhi | 虾哥Mini C3 |
| ESP32 | bread-compact-esp32 | xiaozhi | ESP32面包板 |

## 🧪 测试功能

### 配置测试
- 验证Kconfig文件完整性
- 检查AI模型适配器文件
- 验证Python测试脚本语法
- 检查文档文件存在性

### 连接测试
- API端点可达性测试
- 设备WebSocket连接测试
- 配置持久化测试

## 📦 产物下载

编译完成后，可以从以下位置下载固件：

### Actions 页面
1. 进入 Actions 页面
2. 点击对应的构建任务
3. 在 Artifacts 部分下载固件

### Releases 页面
1. 进入 Releases 页面
2. 下载最新发布的固件包
3. 包含所有配置的固件文件

## 🔍 故障排除

### 常见问题

1. **权限错误**
   ```
   Error: Resource not accessible by integration
   ```
   解决：检查Actions权限设置

2. **工作流不运行**
   - 确认工作流文件语法正确
   - 检查分支保护规则
   - 验证触发条件

3. **编译失败**
   - 查看详细的构建日志
   - 检查配置文件内容
   - 对比成功的构建配置

### 调试方法
1. 启用Actions调试日志
2. 检查工作流文件语法
3. 验证环境变量设置
4. 查看ESP-IDF版本兼容性

## 📚 相关文档

- [GitHub Actions 构建指南](../docs/github-actions-build.md)
- [AI模型配置指南](../docs/ai-model-configuration.md)
- [配置示例](../examples/ai_model_setup.md)

## 🆘 获取帮助

如果遇到问题，请：
1. 查看 [GitHub Actions 文档](https://docs.github.com/en/actions)
2. 检查项目的 Issues 页面
3. 参考项目文档和示例

## 🎯 下一步

安装完成后，建议：
1. 运行一次手动构建测试
2. 创建测试标签验证发布流程
3. 配置分支保护规则
4. 设置通知和集成

祝您使用愉快！🚀

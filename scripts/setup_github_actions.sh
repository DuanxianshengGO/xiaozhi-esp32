#!/bin/bash

# GitHub Actions 设置脚本
# 用于在合并PR后设置GitHub Actions工作流

set -e

echo "🚀 GitHub Actions 设置脚本"
echo "================================"

# 检查是否在git仓库中
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo "❌ 错误: 请在git仓库根目录中运行此脚本"
    exit 1
fi

# 检查是否有.github目录
if [ ! -d ".github" ]; then
    echo "📁 创建 .github 目录..."
    mkdir -p .github/workflows
fi

# 检查工作流文件是否存在
WORKFLOWS_DIR=".github/workflows"
REQUIRED_WORKFLOWS=("build.yml" "release.yml" "test-ai-models.yml")

echo "🔍 检查工作流文件..."

for workflow in "${REQUIRED_WORKFLOWS[@]}"; do
    if [ -f "$WORKFLOWS_DIR/$workflow" ]; then
        echo "✅ $workflow 已存在"
    else
        echo "❌ $workflow 不存在"
        echo "   请从PR中复制工作流文件到 $WORKFLOWS_DIR/ 目录"
    fi
done

# 检查权限设置
echo ""
echo "⚙️ GitHub Actions 权限设置检查"
echo "================================"

echo "请确保以下权限已正确设置:"
echo ""
echo "1. 仓库设置 → Actions → General:"
echo "   ✅ Allow all actions and reusable workflows"
echo "   ✅ Allow actions created by GitHub"
echo ""
echo "2. 仓库设置 → Actions → Workflow permissions:"
echo "   ✅ Read and write permissions"
echo "   ✅ Allow GitHub Actions to create and approve pull requests"
echo ""
echo "3. 仓库设置 → Pages (可选):"
echo "   ✅ Source: GitHub Actions"
echo "   ✅ 用于自动部署文档"

# 检查分支保护规则
echo ""
echo "🛡️ 分支保护建议"
echo "==============="
echo "建议为main分支设置以下保护规则:"
echo "- ✅ Require status checks to pass before merging"
echo "- ✅ Require branches to be up to date before merging"
echo "- ✅ Require review from code owners"

# 创建示例配置文件
echo ""
echo "📝 创建示例配置文件..."

# 创建.github/dependabot.yml
cat > .github/dependabot.yml << 'EOF'
version: 2
updates:
  - package-ecosystem: "github-actions"
    directory: "/"
    schedule:
      interval: "weekly"
    commit-message:
      prefix: "ci"
      include: "scope"
EOF

echo "✅ 创建了 .github/dependabot.yml (自动更新依赖)"

# 创建.github/ISSUE_TEMPLATE
mkdir -p .github/ISSUE_TEMPLATE

cat > .github/ISSUE_TEMPLATE/bug_report.md << 'EOF'
---
name: Bug Report
about: 报告一个问题
title: '[BUG] '
labels: bug
assignees: ''
---

## 问题描述
简要描述遇到的问题

## 复现步骤
1. 
2. 
3. 

## 预期行为
描述您期望发生的情况

## 实际行为
描述实际发生的情况

## 环境信息
- 开发板: 
- AI模型: 
- 固件版本: 
- 编译方式: (本地/GitHub Actions)

## 日志信息
```
粘贴相关日志
```

## 截图
如果适用，请添加截图来帮助解释问题
EOF

cat > .github/ISSUE_TEMPLATE/feature_request.md << 'EOF'
---
name: Feature Request
about: 建议新功能
title: '[FEATURE] '
labels: enhancement
assignees: ''
---

## 功能描述
简要描述您希望添加的功能

## 使用场景
描述这个功能的使用场景和价值

## 详细设计
如果有具体的实现想法，请详细描述

## 替代方案
是否考虑过其他实现方式

## 附加信息
其他相关信息或参考资料
EOF

echo "✅ 创建了 GitHub Issue 模板"

# 创建PR模板
cat > .github/pull_request_template.md << 'EOF'
## 变更说明
简要描述此PR的变更内容

## 变更类型
- [ ] 新功能 (feature)
- [ ] 问题修复 (bugfix)
- [ ] 文档更新 (docs)
- [ ] 代码重构 (refactor)
- [ ] 性能优化 (perf)
- [ ] 测试相关 (test)
- [ ] 构建相关 (build)
- [ ] CI/CD相关 (ci)

## 测试
- [ ] 本地编译测试通过
- [ ] 功能测试通过
- [ ] 回归测试通过
- [ ] 文档已更新

## 检查清单
- [ ] 代码遵循项目规范
- [ ] 已添加必要的测试
- [ ] 已更新相关文档
- [ ] 变更向后兼容
- [ ] 已测试多种配置

## 相关Issue
关联的Issue编号: #

## 截图/日志
如果适用，请添加截图或日志
EOF

echo "✅ 创建了 Pull Request 模板"

# 显示下一步操作
echo ""
echo "🎯 下一步操作"
echo "============="
echo ""
echo "1. 提交新创建的文件:"
echo "   git add .github/"
echo "   git commit -m 'ci: Add GitHub Actions configuration and templates'"
echo "   git push origin main"
echo ""
echo "2. 检查GitHub仓库设置:"
echo "   - 进入仓库 Settings → Actions"
echo "   - 确认权限设置正确"
echo "   - 启用GitHub Pages (可选)"
echo ""
echo "3. 测试工作流:"
echo "   - 进入 Actions 标签页"
echo "   - 手动触发 'Build and Test' 工作流"
echo "   - 检查编译结果"
echo ""
echo "4. 创建第一个发布:"
echo "   - git tag v1.8.0"
echo "   - git push origin v1.8.0"
echo "   - 检查自动创建的Release"

# 检查当前分支
CURRENT_BRANCH=$(git branch --show-current)
echo ""
echo "📋 当前状态"
echo "==========="
echo "当前分支: $CURRENT_BRANCH"
echo "工作目录: $(pwd)"

if [ "$CURRENT_BRANCH" != "main" ]; then
    echo ""
    echo "⚠️  注意: 您当前不在main分支"
    echo "   建议切换到main分支后再运行此脚本"
    echo "   git checkout main"
fi

echo ""
echo "✅ GitHub Actions 设置脚本执行完成!"
echo ""
echo "📚 更多信息请参考:"
echo "   - docs/github-actions-build.md"
echo "   - https://docs.github.com/en/actions"
echo ""
echo "🎉 祝您使用愉快!"

# AI模型配置指南

本文档介绍如何配置小智AI聊天机器人以使用不同的大语言模型，包括ChatGPT、Gemini、Claude等。

## 支持的AI模型

### 1. Xiaozhi官方服务器（默认）
- **提供商**: xiaozhi
- **模型**: Qwen系列
- **类型**: 实时语音交互
- **配置**: 无需额外配置，使用官方服务器

### 2. OpenAI ChatGPT
- **提供商**: openai
- **支持模型**: 
  - `gpt-4o-realtime-preview` (实时语音)
  - `gpt-4o-mini` (文本对话)
- **类型**: 实时语音交互
- **API文档**: https://platform.openai.com/docs/api-reference/realtime

### 3. Google Gemini
- **提供商**: google
- **支持模型**:
  - `gemini-2.0-flash-exp`
  - `gemini-1.5-pro`
- **类型**: 文本对话
- **API文档**: https://ai.google.dev/docs

### 4. Anthropic Claude
- **提供商**: anthropic
- **支持模型**:
  - `claude-3-5-sonnet-20241022`
  - `claude-3-haiku-20240307`
- **类型**: 文本对话
- **API文档**: https://docs.anthropic.com/claude/reference

### 5. 自定义服务器
- **提供商**: custom
- **类型**: 可配置
- **说明**: 支持自定义实现的服务器

## 编译时配置

在编译固件时，可以通过menuconfig配置默认的AI模型：

```bash
idf.py menuconfig
```

在菜单中选择 `Xiaozhi Assistant` -> `AI Model Provider`，然后选择要使用的提供商。

### OpenAI配置
```
AI Model Provider: OpenAI (ChatGPT)
OpenAI API Key: your-api-key-here
OpenAI Model: gpt-4o-realtime-preview
```

### Google配置
```
AI Model Provider: Google (Gemini)
Google API Key: your-api-key-here
Google Model: gemini-2.0-flash-exp
```

### Anthropic配置
```
AI Model Provider: Anthropic (Claude)
Anthropic API Key: your-api-key-here
Anthropic Model: claude-3-5-sonnet-20241022
```

## 运行时配置

设备启动后，可以通过MCP工具动态配置AI模型：

### 1. 查看当前配置
```json
{
  "method": "tools/call",
  "params": {
    "name": "get_ai_model_config"
  }
}
```

### 2. 设置AI模型提供商
```json
{
  "method": "tools/call",
  "params": {
    "name": "set_ai_model_provider",
    "arguments": {
      "provider": "openai"
    }
  }
}
```

### 3. 设置API密钥
```json
{
  "method": "tools/call",
  "params": {
    "name": "set_api_key",
    "arguments": {
      "api_key": "your-api-key-here"
    }
  }
}
```

### 4. 设置模型名称
```json
{
  "method": "tools/call",
  "params": {
    "name": "set_model_name",
    "arguments": {
      "model_name": "gpt-4o-realtime-preview"
    }
  }
}
```

### 5. 设置系统提示词
```json
{
  "method": "tools/call",
  "params": {
    "name": "set_system_prompt",
    "arguments": {
      "system_prompt": "你是一个有用的AI助手，请用中文回答问题。"
    }
  }
}
```

### 6. 测试连接
```json
{
  "method": "tools/call",
  "params": {
    "name": "test_ai_model_connection"
  }
}
```

## API密钥获取

### OpenAI API密钥
1. 访问 https://platform.openai.com/
2. 注册账号并登录
3. 进入 API Keys 页面
4. 创建新的API密钥
5. 复制密钥并保存

### Google API密钥
1. 访问 https://makersuite.google.com/app/apikey
2. 登录Google账号
3. 创建新的API密钥
4. 复制密钥并保存

### Anthropic API密钥
1. 访问 https://console.anthropic.com/
2. 注册账号并登录
3. 进入 API Keys 页面
4. 创建新的API密钥
5. 复制密钥并保存

## 注意事项

1. **API费用**: 使用第三方AI模型需要支付相应的API费用
2. **网络要求**: 确保设备能够访问相应的API服务器
3. **延迟**: 第三方API可能比官方服务器有更高的延迟
4. **功能差异**: 不同模型支持的功能可能有所不同
5. **安全性**: 请妥善保管API密钥，避免泄露

## 故障排除

### 连接失败
- 检查网络连接
- 验证API密钥是否正确
- 确认API服务器地址是否正确

### 音频问题
- OpenAI Realtime API支持实时语音
- Google和Anthropic目前只支持文本对话
- 确保音频格式配置正确

### 配置重置
如果配置出现问题，可以使用重置工具：
```json
{
  "method": "tools/call",
  "params": {
    "name": "reset_ai_model_config"
  }
}
```

## 开发者信息

如需添加新的AI模型支持，请参考 `main/ai_model_adapter.h` 和 `main/ai_model_adapter.cc` 文件中的实现。

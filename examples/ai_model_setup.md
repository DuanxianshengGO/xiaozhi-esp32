# AI模型配置示例

本文档提供了配置不同AI模型的具体示例。

## 快速开始

### 1. 编译时配置OpenAI

```bash
# 配置项目
idf.py menuconfig

# 在菜单中选择:
# Xiaozhi Assistant -> AI Model Provider -> OpenAI (ChatGPT)
# 然后设置:
# OpenAI API Key: sk-your-api-key-here
# OpenAI Model: gpt-4o-realtime-preview

# 编译和烧录
idf.py build flash monitor
```

### 2. 运行时配置Google Gemini

设备启动后，通过MCP工具配置：

```json
// 1. 设置提供商为Google
{
  "type": "mcp",
  "payload": {
    "jsonrpc": "2.0",
    "method": "tools/call",
    "params": {
      "name": "set_ai_model_provider",
      "arguments": {
        "provider": "google"
      }
    },
    "id": 1
  }
}

// 2. 设置API密钥
{
  "type": "mcp",
  "payload": {
    "jsonrpc": "2.0",
    "method": "tools/call",
    "params": {
      "name": "set_api_key",
      "arguments": {
        "api_key": "your-google-api-key"
      }
    },
    "id": 2
  }
}

// 3. 设置模型名称
{
  "type": "mcp",
  "payload": {
    "jsonrpc": "2.0",
    "method": "tools/call",
    "params": {
      "name": "set_model_name",
      "arguments": {
        "model_name": "gemini-2.0-flash-exp"
      }
    },
    "id": 3
  }
}

// 4. 测试连接
{
  "type": "mcp",
  "payload": {
    "jsonrpc": "2.0",
    "method": "tools/call",
    "params": {
      "name": "test_ai_model_connection"
    },
    "id": 4
  }
}
```

## 详细配置示例

### OpenAI ChatGPT配置

```json
// 查看当前配置
{
  "method": "tools/call",
  "params": {
    "name": "get_ai_model_config"
  }
}

// 配置OpenAI
{
  "method": "tools/call",
  "params": {
    "name": "set_ai_model_provider",
    "arguments": {
      "provider": "openai"
    }
  }
}

{
  "method": "tools/call",
  "params": {
    "name": "set_api_key",
    "arguments": {
      "api_key": "sk-proj-your-openai-api-key-here"
    }
  }
}

{
  "method": "tools/call",
  "params": {
    "name": "set_model_name",
    "arguments": {
      "model_name": "gpt-4o-realtime-preview"
    }
  }
}

{
  "method": "tools/call",
  "params": {
    "name": "set_voice_name",
    "arguments": {
      "voice_name": "alloy"
    }
  }
}

{
  "method": "tools/call",
  "params": {
    "name": "set_system_prompt",
    "arguments": {
      "system_prompt": "You are a helpful AI assistant. Please respond in Chinese."
    }
  }
}
```

### Google Gemini配置

```json
{
  "method": "tools/call",
  "params": {
    "name": "set_ai_model_provider",
    "arguments": {
      "provider": "google"
    }
  }
}

{
  "method": "tools/call",
  "params": {
    "name": "set_api_key",
    "arguments": {
      "api_key": "your-google-api-key"
    }
  }
}

{
  "method": "tools/call",
  "params": {
    "name": "set_model_name",
    "arguments": {
      "model_name": "gemini-2.0-flash-exp"
    }
  }
}

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

### Anthropic Claude配置

```json
{
  "method": "tools/call",
  "params": {
    "name": "set_ai_model_provider",
    "arguments": {
      "provider": "anthropic"
    }
  }
}

{
  "method": "tools/call",
  "params": {
    "name": "set_api_key",
    "arguments": {
      "api_key": "sk-ant-your-anthropic-api-key"
    }
  }
}

{
  "method": "tools/call",
  "params": {
    "name": "set_model_name",
    "arguments": {
      "model_name": "claude-3-5-sonnet-20241022"
    }
  }
}
```

## 使用测试脚本

项目提供了Python测试脚本来简化配置过程：

```bash
# 安装依赖
pip install websocket-client

# 基础测试
python scripts/test_ai_models.py --device ws://192.168.1.100:8080

# 配置OpenAI
python scripts/test_ai_models.py \
  --device ws://192.168.1.100:8080 \
  --provider openai \
  --api-key sk-your-api-key \
  --model gpt-4o-realtime-preview

# 配置Google Gemini
python scripts/test_ai_models.py \
  --device ws://192.168.1.100:8080 \
  --provider google \
  --api-key your-google-api-key \
  --model gemini-2.0-flash-exp

# 运行完整测试
python scripts/test_ai_models.py \
  --device ws://192.168.1.100:8080 \
  --test-all
```

## 常见问题

### Q: 如何获取API密钥？
A: 请参考 [AI模型配置文档](../docs/ai-model-configuration.md) 中的API密钥获取部分。

### Q: 为什么连接测试失败？
A: 请检查：
1. 网络连接是否正常
2. API密钥是否正确
3. 模型名称是否支持
4. 是否有足够的API配额

### Q: 如何切换回官方服务器？
A: 使用以下命令：
```json
{
  "method": "tools/call",
  "params": {
    "name": "set_ai_model_provider",
    "arguments": {
      "provider": "xiaozhi"
    }
  }
}
```

### Q: 配置保存在哪里？
A: 配置保存在设备的NVS（非易失性存储）中，重启后仍然有效。

### Q: 如何重置所有配置？
A: 使用重置命令：
```json
{
  "method": "tools/call",
  "params": {
    "name": "reset_ai_model_config"
  }
}
```

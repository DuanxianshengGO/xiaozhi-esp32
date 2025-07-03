#ifndef AI_MODEL_TOOLS_H
#define AI_MODEL_TOOLS_H

#include "mcp_server.h"
#include "ai_model_adapter.h"

class AIModelTools {
public:
    static void RegisterTools();

private:
    // 获取当前AI模型配置
    static ReturnValue GetAIModelConfig(const PropertyList& properties);
    
    // 设置AI模型提供商
    static ReturnValue SetAIModelProvider(const PropertyList& properties);
    
    // 设置API密钥
    static ReturnValue SetAPIKey(const PropertyList& properties);
    
    // 设置模型名称
    static ReturnValue SetModelName(const PropertyList& properties);
    
    // 设置基础URL
    static ReturnValue SetBaseURL(const PropertyList& properties);
    
    // 设置语音名称
    static ReturnValue SetVoiceName(const PropertyList& properties);
    
    // 设置系统提示词
    static ReturnValue SetSystemPrompt(const PropertyList& properties);
    
    // 测试AI模型连接
    static ReturnValue TestAIModelConnection(const PropertyList& properties);
    
    // 重置AI模型配置
    static ReturnValue ResetAIModelConfig(const PropertyList& properties);
    
    // 获取支持的AI模型列表
    static ReturnValue GetSupportedModels(const PropertyList& properties);
    
    // 辅助方法
    static std::string ProviderToString(AIModelProvider provider);
    static AIModelProvider StringToProvider(const std::string& provider_str);
    static std::string ModelTypeToString(AIModelType type);
};

#endif // AI_MODEL_TOOLS_H

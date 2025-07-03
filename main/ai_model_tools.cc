#include "ai_model_tools.h"
#include "application.h"
#include "board.h"

#include <esp_log.h>
#include <cJSON.h>

#define TAG "AIModelTools"

void AIModelTools::RegisterTools() {
    auto& mcp_server = McpServer::GetInstance();
    
    // 获取AI模型配置
    mcp_server.AddTool(
        "get_ai_model_config",
        "Get current AI model configuration",
        PropertyList(),
        GetAIModelConfig
    );
    
    // 设置AI模型提供商
    PropertyList provider_props;
    provider_props.AddProperty(Property("provider", kPropertyTypeString));
    mcp_server.AddTool(
        "set_ai_model_provider",
        "Set AI model provider (xiaozhi, openai, google, anthropic, custom)",
        provider_props,
        SetAIModelProvider
    );
    
    // 设置API密钥
    PropertyList api_key_props;
    api_key_props.AddProperty(Property("api_key", kPropertyTypeString));
    mcp_server.AddTool(
        "set_api_key",
        "Set API key for the AI model provider",
        api_key_props,
        SetAPIKey
    );
    
    // 设置模型名称
    PropertyList model_props;
    model_props.AddProperty(Property("model_name", kPropertyTypeString));
    mcp_server.AddTool(
        "set_model_name",
        "Set the AI model name to use",
        model_props,
        SetModelName
    );
    
    // 设置基础URL
    PropertyList url_props;
    url_props.AddProperty(Property("base_url", kPropertyTypeString));
    mcp_server.AddTool(
        "set_base_url",
        "Set the base URL for custom AI model provider",
        url_props,
        SetBaseURL
    );
    
    // 设置语音名称
    PropertyList voice_props;
    voice_props.AddProperty(Property("voice_name", kPropertyTypeString));
    mcp_server.AddTool(
        "set_voice_name",
        "Set the voice name for speech synthesis",
        voice_props,
        SetVoiceName
    );
    
    // 设置系统提示词
    PropertyList prompt_props;
    prompt_props.AddProperty(Property("system_prompt", kPropertyTypeString));
    mcp_server.AddTool(
        "set_system_prompt",
        "Set the system prompt for the AI model",
        prompt_props,
        SetSystemPrompt
    );
    
    // 测试AI模型连接
    mcp_server.AddTool(
        "test_ai_model_connection",
        "Test connection to the configured AI model",
        PropertyList(),
        TestAIModelConnection
    );
    
    // 重置AI模型配置
    mcp_server.AddTool(
        "reset_ai_model_config",
        "Reset AI model configuration to defaults",
        PropertyList(),
        ResetAIModelConfig
    );
    
    // 获取支持的AI模型列表
    mcp_server.AddTool(
        "get_supported_models",
        "Get list of supported AI model providers",
        PropertyList(),
        GetSupportedModels
    );
}

ReturnValue AIModelTools::GetAIModelConfig(const PropertyList& properties) {
    AIModelConfig config = AIModelAdapter::LoadConfigFromNVS();
    
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "provider", ProviderToString(config.provider).c_str());
    cJSON_AddStringToObject(root, "model_type", ModelTypeToString(config.type).c_str());
    cJSON_AddStringToObject(root, "model_name", config.model_name.c_str());
    cJSON_AddStringToObject(root, "base_url", config.base_url.c_str());
    cJSON_AddStringToObject(root, "voice_name", config.voice_name.c_str());
    cJSON_AddStringToObject(root, "system_prompt", config.system_prompt.c_str());
    cJSON_AddNumberToObject(root, "sample_rate", config.sample_rate);
    cJSON_AddStringToObject(root, "audio_format", config.audio_format.c_str());
    
    // 不显示API密钥，只显示是否已设置
    cJSON_AddBoolToObject(root, "api_key_set", !config.api_key.empty());
    
    char* json_str = cJSON_PrintUnformatted(root);
    std::string result(json_str);
    cJSON_free(json_str);
    cJSON_Delete(root);
    
    return result;
}

ReturnValue AIModelTools::SetAIModelProvider(const PropertyList& properties) {
    try {
        std::string provider_str = properties["provider"].value<std::string>();
        AIModelProvider provider = StringToProvider(provider_str);
        
        AIModelConfig config = AIModelAdapter::LoadConfigFromNVS();
        config.provider = provider;
        
        // 根据提供商设置默认值
        switch (provider) {
            case AIModelProvider::kOpenAI:
                if (config.model_name.empty()) config.model_name = "gpt-4o-realtime-preview";
                if (config.base_url.empty()) config.base_url = "wss://api.openai.com/v1/realtime";
                config.type = AIModelType::kRealtime;
                break;
            case AIModelProvider::kGoogle:
                if (config.model_name.empty()) config.model_name = "gemini-2.0-flash-exp";
                if (config.base_url.empty()) config.base_url = "https://generativelanguage.googleapis.com/v1beta/models";
                config.type = AIModelType::kChatCompletion;
                break;
            case AIModelProvider::kAnthropic:
                if (config.model_name.empty()) config.model_name = "claude-3-5-sonnet-20241022";
                if (config.base_url.empty()) config.base_url = "https://api.anthropic.com/v1/messages";
                config.type = AIModelType::kChatCompletion;
                break;
            default:
                break;
        }
        
        AIModelAdapter::SaveConfigToNVS(config);
        
        return std::string("AI model provider set to: ") + provider_str;
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

ReturnValue AIModelTools::SetAPIKey(const PropertyList& properties) {
    try {
        std::string api_key = properties["api_key"].value<std::string>();
        
        AIModelConfig config = AIModelAdapter::LoadConfigFromNVS();
        config.api_key = api_key;
        AIModelAdapter::SaveConfigToNVS(config);
        
        return std::string("API key updated successfully");
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

ReturnValue AIModelTools::SetModelName(const PropertyList& properties) {
    try {
        std::string model_name = properties["model_name"].value<std::string>();
        
        AIModelConfig config = AIModelAdapter::LoadConfigFromNVS();
        config.model_name = model_name;
        AIModelAdapter::SaveConfigToNVS(config);
        
        return std::string("Model name set to: ") + model_name;
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

ReturnValue AIModelTools::SetBaseURL(const PropertyList& properties) {
    try {
        std::string base_url = properties["base_url"].value<std::string>();
        
        AIModelConfig config = AIModelAdapter::LoadConfigFromNVS();
        config.base_url = base_url;
        AIModelAdapter::SaveConfigToNVS(config);
        
        return std::string("Base URL set to: ") + base_url;
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

ReturnValue AIModelTools::SetVoiceName(const PropertyList& properties) {
    try {
        std::string voice_name = properties["voice_name"].value<std::string>();
        
        AIModelConfig config = AIModelAdapter::LoadConfigFromNVS();
        config.voice_name = voice_name;
        AIModelAdapter::SaveConfigToNVS(config);
        
        return std::string("Voice name set to: ") + voice_name;
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

ReturnValue AIModelTools::SetSystemPrompt(const PropertyList& properties) {
    try {
        std::string system_prompt = properties["system_prompt"].value<std::string>();
        
        AIModelConfig config = AIModelAdapter::LoadConfigFromNVS();
        config.system_prompt = system_prompt;
        AIModelAdapter::SaveConfigToNVS(config);
        
        return std::string("System prompt updated successfully");
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

ReturnValue AIModelTools::TestAIModelConnection(const PropertyList& properties) {
    try {
        AIModelConfig config = AIModelAdapter::LoadConfigFromNVS();
        AIModelProvider provider = AIModelAdapter::GetProviderFromConfig();
        
        if (provider == AIModelProvider::kXiaozhi) {
            return std::string("Using Xiaozhi official server, no test needed");
        }
        
        auto adapter = AIModelAdapter::CreateAdapter(provider);
        if (!adapter) {
            return std::string("Failed to create adapter for provider");
        }
        
        if (!adapter->Initialize(config)) {
            return std::string("Failed to initialize adapter");
        }
        
        if (!adapter->Connect()) {
            return std::string("Failed to connect to AI model");
        }
        
        adapter->Disconnect();
        return std::string("Connection test successful");
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

ReturnValue AIModelTools::ResetAIModelConfig(const PropertyList& properties) {
    try {
        // 清除NVS中的配置
        Settings settings("ai_model", true);
        settings.EraseKey("api_key");
        settings.EraseKey("model_name");
        settings.EraseKey("base_url");
        settings.EraseKey("voice_name");
        settings.EraseKey("system_prompt");
        
        return std::string("AI model configuration reset to defaults");
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

ReturnValue AIModelTools::GetSupportedModels(const PropertyList& properties) {
    cJSON* root = cJSON_CreateArray();
    
    cJSON* xiaozhi = cJSON_CreateObject();
    cJSON_AddStringToObject(xiaozhi, "provider", "xiaozhi");
    cJSON_AddStringToObject(xiaozhi, "description", "Xiaozhi official server with Qwen models");
    cJSON_AddStringToObject(xiaozhi, "type", "realtime");
    cJSON_AddItemToArray(root, xiaozhi);
    
    cJSON* openai = cJSON_CreateObject();
    cJSON_AddStringToObject(openai, "provider", "openai");
    cJSON_AddStringToObject(openai, "description", "OpenAI ChatGPT models");
    cJSON_AddStringToObject(openai, "type", "realtime");
    cJSON_AddStringToObject(openai, "models", "gpt-4o-realtime-preview, gpt-4o-mini");
    cJSON_AddItemToArray(root, openai);
    
    cJSON* google = cJSON_CreateObject();
    cJSON_AddStringToObject(google, "provider", "google");
    cJSON_AddStringToObject(google, "description", "Google Gemini models");
    cJSON_AddStringToObject(google, "type", "chat_completion");
    cJSON_AddStringToObject(google, "models", "gemini-2.0-flash-exp, gemini-1.5-pro");
    cJSON_AddItemToArray(root, google);
    
    cJSON* anthropic = cJSON_CreateObject();
    cJSON_AddStringToObject(anthropic, "provider", "anthropic");
    cJSON_AddStringToObject(anthropic, "description", "Anthropic Claude models");
    cJSON_AddStringToObject(anthropic, "type", "chat_completion");
    cJSON_AddStringToObject(anthropic, "models", "claude-3-5-sonnet-20241022, claude-3-haiku-20240307");
    cJSON_AddItemToArray(root, anthropic);
    
    cJSON* custom = cJSON_CreateObject();
    cJSON_AddStringToObject(custom, "provider", "custom");
    cJSON_AddStringToObject(custom, "description", "Custom server implementation");
    cJSON_AddStringToObject(custom, "type", "configurable");
    cJSON_AddItemToArray(root, custom);
    
    char* json_str = cJSON_PrintUnformatted(root);
    std::string result(json_str);
    cJSON_free(json_str);
    cJSON_Delete(root);
    
    return result;
}

std::string AIModelTools::ProviderToString(AIModelProvider provider) {
    switch (provider) {
        case AIModelProvider::kXiaozhi: return "xiaozhi";
        case AIModelProvider::kOpenAI: return "openai";
        case AIModelProvider::kGoogle: return "google";
        case AIModelProvider::kAnthropic: return "anthropic";
        case AIModelProvider::kCustom: return "custom";
        default: return "unknown";
    }
}

AIModelProvider AIModelTools::StringToProvider(const std::string& provider_str) {
    if (provider_str == "xiaozhi") return AIModelProvider::kXiaozhi;
    if (provider_str == "openai") return AIModelProvider::kOpenAI;
    if (provider_str == "google") return AIModelProvider::kGoogle;
    if (provider_str == "anthropic") return AIModelProvider::kAnthropic;
    if (provider_str == "custom") return AIModelProvider::kCustom;
    throw std::invalid_argument("Unknown provider: " + provider_str);
}

std::string AIModelTools::ModelTypeToString(AIModelType type) {
    switch (type) {
        case AIModelType::kChatCompletion: return "chat_completion";
        case AIModelType::kRealtime: return "realtime";
        case AIModelType::kMultimodal: return "multimodal";
        default: return "unknown";
    }
}

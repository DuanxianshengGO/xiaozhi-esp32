#ifndef AI_MODEL_ADAPTER_H
#define AI_MODEL_ADAPTER_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>

#include <cJSON.h>
#include "protocol.h"

enum class AIModelProvider {
    kXiaozhi,
    kOpenAI,
    kGoogle,
    kAnthropic,
    kCustom
};

enum class AIModelType {
    kChatCompletion,    // 传统的文本对话模型
    kRealtime,          // 实时语音模型 (如 OpenAI Realtime API)
    kMultimodal         // 多模态模型
};

struct AIModelConfig {
    AIModelProvider provider;
    AIModelType type;
    std::string api_key;
    std::string model_name;
    std::string base_url;
    std::string endpoint;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> parameters;
    
    // 语音相关配置
    std::string voice_name;
    int sample_rate = 16000;
    std::string audio_format = "opus";
    
    // 系统提示词
    std::string system_prompt;
};

class AIModelAdapter {
public:
    virtual ~AIModelAdapter() = default;
    
    // 初始化适配器
    virtual bool Initialize(const AIModelConfig& config) = 0;
    
    // 获取适配器类型
    virtual AIModelProvider GetProvider() const = 0;
    virtual AIModelType GetModelType() const = 0;
    
    // 连接和断开
    virtual bool Connect() = 0;
    virtual void Disconnect() = 0;
    virtual bool IsConnected() const = 0;
    
    // 发送文本消息
    virtual bool SendTextMessage(const std::string& message) = 0;
    
    // 发送音频数据
    virtual bool SendAudioData(const std::vector<uint8_t>& audio_data) = 0;
    
    // 开始/停止语音会话
    virtual bool StartVoiceSession() = 0;
    virtual bool StopVoiceSession() = 0;
    
    // 设置回调函数
    virtual void SetTextResponseCallback(std::function<void(const std::string&)> callback) = 0;
    virtual void SetAudioResponseCallback(std::function<void(const std::vector<uint8_t>&)> callback) = 0;
    virtual void SetErrorCallback(std::function<void(const std::string&)> callback) = 0;
    virtual void SetStatusCallback(std::function<void(const std::string&)> callback) = 0;
    
    // 获取配置信息
    virtual const AIModelConfig& GetConfig() const = 0;
    
    // 工具函数
    static std::unique_ptr<AIModelAdapter> CreateAdapter(AIModelProvider provider);
    static AIModelProvider GetProviderFromConfig();
    static AIModelConfig LoadConfigFromNVS();
    static void SaveConfigToNVS(const AIModelConfig& config);
};

// OpenAI 适配器
class OpenAIAdapter : public AIModelAdapter {
public:
    OpenAIAdapter();
    ~OpenAIAdapter() override;
    
    bool Initialize(const AIModelConfig& config) override;
    AIModelProvider GetProvider() const override { return AIModelProvider::kOpenAI; }
    AIModelType GetModelType() const override;
    
    bool Connect() override;
    void Disconnect() override;
    bool IsConnected() const override;
    
    bool SendTextMessage(const std::string& message) override;
    bool SendAudioData(const std::vector<uint8_t>& audio_data) override;
    
    bool StartVoiceSession() override;
    bool StopVoiceSession() override;
    
    void SetTextResponseCallback(std::function<void(const std::string&)> callback) override;
    void SetAudioResponseCallback(std::function<void(const std::vector<uint8_t>&)> callback) override;
    void SetErrorCallback(std::function<void(const std::string&)> callback) override;
    void SetStatusCallback(std::function<void(const std::string&)> callback) override;
    
    const AIModelConfig& GetConfig() const override { return config_; }

private:
    AIModelConfig config_;
    std::unique_ptr<WebSocket> websocket_;
    bool connected_ = false;
    std::string session_id_;
    
    std::function<void(const std::string&)> text_callback_;
    std::function<void(const std::vector<uint8_t>&)> audio_callback_;
    std::function<void(const std::string&)> error_callback_;
    std::function<void(const std::string&)> status_callback_;
    
    void HandleWebSocketMessage(const std::string& message);
    std::string CreateSessionConfig();
    std::string CreateTextMessage(const std::string& text);
    std::string CreateAudioMessage(const std::vector<uint8_t>& audio_data);
};

// Google Gemini 适配器
class GoogleAdapter : public AIModelAdapter {
public:
    GoogleAdapter();
    ~GoogleAdapter() override;
    
    bool Initialize(const AIModelConfig& config) override;
    AIModelProvider GetProvider() const override { return AIModelProvider::kGoogle; }
    AIModelType GetModelType() const override { return AIModelType::kChatCompletion; }
    
    bool Connect() override;
    void Disconnect() override;
    bool IsConnected() const override;
    
    bool SendTextMessage(const std::string& message) override;
    bool SendAudioData(const std::vector<uint8_t>& audio_data) override;
    
    bool StartVoiceSession() override;
    bool StopVoiceSession() override;
    
    void SetTextResponseCallback(std::function<void(const std::string&)> callback) override;
    void SetAudioResponseCallback(std::function<void(const std::vector<uint8_t>&)> callback) override;
    void SetErrorCallback(std::function<void(const std::string&)> callback) override;
    void SetStatusCallback(std::function<void(const std::string&)> callback) override;
    
    const AIModelConfig& GetConfig() const override { return config_; }

private:
    AIModelConfig config_;
    std::unique_ptr<Http> http_;
    bool connected_ = false;
    
    std::function<void(const std::string&)> text_callback_;
    std::function<void(const std::vector<uint8_t>&)> audio_callback_;
    std::function<void(const std::string&)> error_callback_;
    std::function<void(const std::string&)> status_callback_;
    
    std::string CreateChatRequest(const std::string& message);
    void ProcessChatResponse(const std::string& response);
};

// Anthropic Claude 适配器
class AnthropicAdapter : public AIModelAdapter {
public:
    AnthropicAdapter();
    ~AnthropicAdapter() override;
    
    bool Initialize(const AIModelConfig& config) override;
    AIModelProvider GetProvider() const override { return AIModelProvider::kAnthropic; }
    AIModelType GetModelType() const override { return AIModelType::kChatCompletion; }
    
    bool Connect() override;
    void Disconnect() override;
    bool IsConnected() const override;
    
    bool SendTextMessage(const std::string& message) override;
    bool SendAudioData(const std::vector<uint8_t>& audio_data) override;
    
    bool StartVoiceSession() override;
    bool StopVoiceSession() override;
    
    void SetTextResponseCallback(std::function<void(const std::string&)> callback) override;
    void SetAudioResponseCallback(std::function<void(const std::vector<uint8_t>&)> callback) override;
    void SetErrorCallback(std::function<void(const std::string&)> callback) override;
    void SetStatusCallback(std::function<void(const std::string&)> callback) override;
    
    const AIModelConfig& GetConfig() const override { return config_; }

private:
    AIModelConfig config_;
    std::unique_ptr<Http> http_;
    bool connected_ = false;
    
    std::function<void(const std::string&)> text_callback_;
    std::function<void(const std::vector<uint8_t>&)> audio_callback_;
    std::function<void(const std::string&)> error_callback_;
    std::function<void(const std::string&)> status_callback_;
    
    std::string CreateMessageRequest(const std::string& message);
    void ProcessMessageResponse(const std::string& response);
};

// 自定义服务器适配器
class CustomAdapter : public AIModelAdapter {
public:
    CustomAdapter();
    ~CustomAdapter() override;
    
    bool Initialize(const AIModelConfig& config) override;
    AIModelProvider GetProvider() const override { return AIModelProvider::kCustom; }
    AIModelType GetModelType() const override;
    
    bool Connect() override;
    void Disconnect() override;
    bool IsConnected() const override;
    
    bool SendTextMessage(const std::string& message) override;
    bool SendAudioData(const std::vector<uint8_t>& audio_data) override;
    
    bool StartVoiceSession() override;
    bool StopVoiceSession() override;
    
    void SetTextResponseCallback(std::function<void(const std::string&)> callback) override;
    void SetAudioResponseCallback(std::function<void(const std::vector<uint8_t>&)> callback) override;
    void SetErrorCallback(std::function<void(const std::string&)> callback) override;
    void SetStatusCallback(std::function<void(const std::string&)> callback) override;
    
    const AIModelConfig& GetConfig() const override { return config_; }

private:
    AIModelConfig config_;
    std::unique_ptr<Protocol> protocol_;
    bool connected_ = false;
    
    std::function<void(const std::string&)> text_callback_;
    std::function<void(const std::vector<uint8_t>&)> audio_callback_;
    std::function<void(const std::string&)> error_callback_;
    std::function<void(const std::string&)> status_callback_;
};

#endif // AI_MODEL_ADAPTER_H

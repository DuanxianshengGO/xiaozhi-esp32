#include "ai_model_adapter.h"
#include "board.h"
#include "settings.h"
#include "assets/lang_config.h"
#include "base64_utils.h"

#include <esp_log.h>
#include <cJSON.h>
#include <cstring>
#include <algorithm>

#define TAG "AIModelAdapter"

// 静态工厂方法
std::unique_ptr<AIModelAdapter> AIModelAdapter::CreateAdapter(AIModelProvider provider) {
    switch (provider) {
        case AIModelProvider::kOpenAI:
            return std::make_unique<OpenAIAdapter>();
        case AIModelProvider::kGoogle:
            return std::make_unique<GoogleAdapter>();
        case AIModelProvider::kAnthropic:
            return std::make_unique<AnthropicAdapter>();
        case AIModelProvider::kCustom:
            return std::make_unique<CustomAdapter>();
        case AIModelProvider::kXiaozhi:
        default:
            return nullptr; // 使用原有的协议
    }
}

AIModelProvider AIModelAdapter::GetProviderFromConfig() {
#if CONFIG_AI_MODEL_PROVIDER_OPENAI
    return AIModelProvider::kOpenAI;
#elif CONFIG_AI_MODEL_PROVIDER_GOOGLE
    return AIModelProvider::kGoogle;
#elif CONFIG_AI_MODEL_PROVIDER_ANTHROPIC
    return AIModelProvider::kAnthropic;
#elif CONFIG_AI_MODEL_PROVIDER_CUSTOM
    return AIModelProvider::kCustom;
#else
    return AIModelProvider::kXiaozhi;
#endif
}

AIModelConfig AIModelAdapter::LoadConfigFromNVS() {
    Settings settings("ai_model", false);
    AIModelConfig config;
    
    // 从编译时配置加载默认值
    config.provider = GetProviderFromConfig();
    
    switch (config.provider) {
        case AIModelProvider::kOpenAI:
            config.api_key = CONFIG_OPENAI_API_KEY;
            config.model_name = CONFIG_OPENAI_MODEL;
            config.base_url = "wss://api.openai.com/v1/realtime";
            config.type = AIModelType::kRealtime;
            config.voice_name = "alloy";
            break;
            
        case AIModelProvider::kGoogle:
            config.api_key = CONFIG_GOOGLE_API_KEY;
            config.model_name = CONFIG_GOOGLE_MODEL;
            config.base_url = "https://generativelanguage.googleapis.com/v1beta/models";
            config.type = AIModelType::kChatCompletion;
            break;
            
        case AIModelProvider::kAnthropic:
            config.api_key = CONFIG_ANTHROPIC_API_KEY;
            config.model_name = CONFIG_ANTHROPIC_MODEL;
            config.base_url = "https://api.anthropic.com/v1/messages";
            config.type = AIModelType::kChatCompletion;
            break;
            
        case AIModelProvider::kCustom:
            config.base_url = CONFIG_CUSTOM_SERVER_URL;
            config.type = AIModelType::kChatCompletion;
            break;
            
        default:
            break;
    }
    
    // 从NVS覆盖配置
    std::string nvs_api_key = settings.GetString("api_key");
    if (!nvs_api_key.empty()) {
        config.api_key = nvs_api_key;
    }
    
    std::string nvs_model = settings.GetString("model_name");
    if (!nvs_model.empty()) {
        config.model_name = nvs_model;
    }
    
    std::string nvs_base_url = settings.GetString("base_url");
    if (!nvs_base_url.empty()) {
        config.base_url = nvs_base_url;
    }
    
    std::string nvs_voice = settings.GetString("voice_name");
    if (!nvs_voice.empty()) {
        config.voice_name = nvs_voice;
    }
    
    // 设置默认系统提示词
    config.system_prompt = "You are a helpful AI assistant. Please respond in " + std::string(Lang::NAME) + ".";
    std::string nvs_prompt = settings.GetString("system_prompt");
    if (!nvs_prompt.empty()) {
        config.system_prompt = nvs_prompt;
    }
    
    return config;
}

void AIModelAdapter::SaveConfigToNVS(const AIModelConfig& config) {
    Settings settings("ai_model", true);
    
    settings.SetString("api_key", config.api_key);
    settings.SetString("model_name", config.model_name);
    settings.SetString("base_url", config.base_url);
    settings.SetString("voice_name", config.voice_name);
    settings.SetString("system_prompt", config.system_prompt);
}

// OpenAI 适配器实现
OpenAIAdapter::OpenAIAdapter() {
}

OpenAIAdapter::~OpenAIAdapter() {
    Disconnect();
}

bool OpenAIAdapter::Initialize(const AIModelConfig& config) {
    config_ = config;
    
    if (config_.api_key.empty()) {
        ESP_LOGE(TAG, "OpenAI API key is required");
        return false;
    }
    
    if (config_.model_name.empty()) {
        config_.model_name = "gpt-4o-realtime-preview";
    }
    
    return true;
}

AIModelType OpenAIAdapter::GetModelType() const {
    if (config_.model_name.find("realtime") != std::string::npos) {
        return AIModelType::kRealtime;
    }
    return AIModelType::kChatCompletion;
}

bool OpenAIAdapter::Connect() {
    if (connected_) {
        return true;
    }
    
    websocket_ = std::unique_ptr<WebSocket>(Board::GetInstance().CreateWebSocket());
    if (!websocket_) {
        ESP_LOGE(TAG, "Failed to create WebSocket");
        return false;
    }
    
    // 设置认证头
    std::string auth_header = "Bearer " + config_.api_key;
    websocket_->SetHeader("Authorization", auth_header.c_str());
    websocket_->SetHeader("OpenAI-Beta", "realtime=v1");
    
    // 构建WebSocket URL
    std::string url = config_.base_url + "?model=" + config_.model_name;
    
    ESP_LOGI(TAG, "Connecting to OpenAI Realtime API: %s", url.c_str());
    
    if (!websocket_->Connect(url.c_str())) {
        ESP_LOGE(TAG, "Failed to connect to OpenAI");
        return false;
    }
    
    // 设置消息处理回调
    websocket_->OnMessage([this](const std::string& message) {
        HandleWebSocketMessage(message);
    });
    
    websocket_->OnError([this](const std::string& error) {
        ESP_LOGE(TAG, "WebSocket error: %s", error.c_str());
        if (error_callback_) {
            error_callback_(error);
        }
        connected_ = false;
    });
    
    connected_ = true;
    
    // 发送会话配置
    std::string session_config = CreateSessionConfig();
    websocket_->Send(session_config);
    
    if (status_callback_) {
        status_callback_("Connected to OpenAI");
    }
    
    return true;
}

void OpenAIAdapter::Disconnect() {
    if (websocket_) {
        websocket_->Close();
        websocket_.reset();
    }
    connected_ = false;
    session_id_.clear();
}

bool OpenAIAdapter::IsConnected() const {
    return connected_ && websocket_ && websocket_->IsConnected();
}

bool OpenAIAdapter::SendTextMessage(const std::string& message) {
    if (!IsConnected()) {
        ESP_LOGE(TAG, "Not connected to OpenAI");
        return false;
    }
    
    std::string json_message = CreateTextMessage(message);
    return websocket_->Send(json_message);
}

bool OpenAIAdapter::SendAudioData(const std::vector<uint8_t>& audio_data) {
    if (!IsConnected()) {
        ESP_LOGE(TAG, "Not connected to OpenAI");
        return false;
    }
    
    std::string json_message = CreateAudioMessage(audio_data);
    return websocket_->Send(json_message);
}

bool OpenAIAdapter::StartVoiceSession() {
    if (!IsConnected()) {
        return false;
    }
    
    // OpenAI Realtime API 会话已经在连接时开始
    return true;
}

bool OpenAIAdapter::StopVoiceSession() {
    // 可以发送停止消息或者直接断开连接
    return true;
}

void OpenAIAdapter::SetTextResponseCallback(std::function<void(const std::string&)> callback) {
    text_callback_ = callback;
}

void OpenAIAdapter::SetAudioResponseCallback(std::function<void(const std::vector<uint8_t>&)> callback) {
    audio_callback_ = callback;
}

void OpenAIAdapter::SetErrorCallback(std::function<void(const std::string&)> callback) {
    error_callback_ = callback;
}

void OpenAIAdapter::SetStatusCallback(std::function<void(const std::string&)> callback) {
    status_callback_ = callback;
}

void OpenAIAdapter::HandleWebSocketMessage(const std::string& message) {
    cJSON* json = cJSON_Parse(message.c_str());
    if (!json) {
        ESP_LOGE(TAG, "Failed to parse JSON message");
        return;
    }
    
    cJSON* type = cJSON_GetObjectItem(json, "type");
    if (!cJSON_IsString(type)) {
        cJSON_Delete(json);
        return;
    }
    
    std::string type_str = type->valuestring;
    
    if (type_str == "session.created") {
        cJSON* session = cJSON_GetObjectItem(json, "session");
        if (session) {
            cJSON* id = cJSON_GetObjectItem(session, "id");
            if (cJSON_IsString(id)) {
                session_id_ = id->valuestring;
                ESP_LOGI(TAG, "Session created: %s", session_id_.c_str());
            }
        }
    } else if (type_str == "response.audio.delta") {
        cJSON* delta = cJSON_GetObjectItem(json, "delta");
        if (cJSON_IsString(delta) && audio_callback_) {
            // Base64 解码音频数据
            std::string base64_audio = delta->valuestring;
            std::vector<uint8_t> audio_data = Base64Utils::Decode(base64_audio);
            audio_callback_(audio_data);
        }
    } else if (type_str == "response.text.delta") {
        cJSON* delta = cJSON_GetObjectItem(json, "delta");
        if (cJSON_IsString(delta) && text_callback_) {
            text_callback_(delta->valuestring);
        }
    } else if (type_str == "error") {
        cJSON* error = cJSON_GetObjectItem(json, "error");
        if (error) {
            cJSON* message_obj = cJSON_GetObjectItem(error, "message");
            if (cJSON_IsString(message_obj) && error_callback_) {
                error_callback_(message_obj->valuestring);
            }
        }
    }
    
    cJSON_Delete(json);
}

std::string OpenAIAdapter::CreateSessionConfig() {
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "session.update");

    cJSON* session = cJSON_CreateObject();
    cJSON_AddStringToObject(session, "modalities", "text,audio");
    cJSON_AddStringToObject(session, "instructions", config_.system_prompt.c_str());

    cJSON* voice = cJSON_CreateObject();
    cJSON_AddStringToObject(voice, "voice", config_.voice_name.c_str());
    cJSON_AddItemToObject(session, "voice", voice);

    cJSON* input_audio_format = cJSON_CreateObject();
    cJSON_AddStringToObject(input_audio_format, "type", "g711_ulaw");
    cJSON_AddNumberToObject(input_audio_format, "sample_rate", 8000);
    cJSON_AddItemToObject(session, "input_audio_format", input_audio_format);

    cJSON* output_audio_format = cJSON_CreateObject();
    cJSON_AddStringToObject(output_audio_format, "type", "g711_ulaw");
    cJSON_AddNumberToObject(output_audio_format, "sample_rate", 8000);
    cJSON_AddItemToObject(session, "output_audio_format", output_audio_format);

    cJSON_AddItemToObject(root, "session", session);

    char* json_string = cJSON_PrintUnformatted(root);
    std::string result(json_string);
    cJSON_free(json_string);
    cJSON_Delete(root);

    return result;
}

std::string OpenAIAdapter::CreateTextMessage(const std::string& text) {
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "conversation.item.create");

    cJSON* item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "type", "message");
    cJSON_AddStringToObject(item, "role", "user");

    cJSON* content = cJSON_CreateArray();
    cJSON* content_item = cJSON_CreateObject();
    cJSON_AddStringToObject(content_item, "type", "input_text");
    cJSON_AddStringToObject(content_item, "text", text.c_str());
    cJSON_AddItemToArray(content, content_item);
    cJSON_AddItemToObject(item, "content", content);

    cJSON_AddItemToObject(root, "item", item);

    char* json_string = cJSON_PrintUnformatted(root);
    std::string result(json_string);
    cJSON_free(json_string);
    cJSON_Delete(root);

    return result;
}

std::string OpenAIAdapter::CreateAudioMessage(const std::vector<uint8_t>& audio_data) {
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "input_audio_buffer.append");

    // Base64编码音频数据
    std::string base64_audio = Base64Utils::Encode(audio_data);
    cJSON_AddStringToObject(root, "audio", base64_audio.c_str());

    char* json_string = cJSON_PrintUnformatted(root);
    std::string result(json_string);
    cJSON_free(json_string);
    cJSON_Delete(root);

    return result;
}

// Google 适配器实现
GoogleAdapter::GoogleAdapter() {
}

GoogleAdapter::~GoogleAdapter() {
    Disconnect();
}

bool GoogleAdapter::Initialize(const AIModelConfig& config) {
    config_ = config;

    if (config_.api_key.empty()) {
        ESP_LOGE(TAG, "Google API key is required");
        return false;
    }

    if (config_.model_name.empty()) {
        config_.model_name = "gemini-2.0-flash-exp";
    }

    return true;
}

bool GoogleAdapter::Connect() {
    if (connected_) {
        return true;
    }

    http_ = std::unique_ptr<Http>(Board::GetInstance().CreateHttp());
    if (!http_) {
        ESP_LOGE(TAG, "Failed to create HTTP client");
        return false;
    }

    // 设置通用头部
    http_->SetHeader("Content-Type", "application/json");

    connected_ = true;

    if (status_callback_) {
        status_callback_("Connected to Google Gemini");
    }

    return true;
}

void GoogleAdapter::Disconnect() {
    if (http_) {
        http_.reset();
    }
    connected_ = false;
}

bool GoogleAdapter::IsConnected() const {
    return connected_;
}

bool GoogleAdapter::SendTextMessage(const std::string& message) {
    if (!IsConnected()) {
        ESP_LOGE(TAG, "Not connected to Google");
        return false;
    }

    std::string request_body = CreateChatRequest(message);
    std::string url = config_.base_url + "/" + config_.model_name + ":generateContent?key=" + config_.api_key;

    http_->SetContent(request_body);

    if (!http_->Open("POST", url)) {
        ESP_LOGE(TAG, "Failed to open HTTP connection");
        return false;
    }

    if (http_->GetStatusCode() != 200) {
        ESP_LOGE(TAG, "HTTP request failed with status: %d", http_->GetStatusCode());
        return false;
    }

    std::string response = http_->ReadAll();
    http_->Close();

    ProcessChatResponse(response);
    return true;
}

bool GoogleAdapter::SendAudioData(const std::vector<uint8_t>& audio_data) {
    // Google Gemini 目前不支持实时音频，需要先转换为文本
    ESP_LOGW(TAG, "Google Gemini does not support real-time audio");
    return false;
}

bool GoogleAdapter::StartVoiceSession() {
    // Google Gemini 不支持实时语音会话
    return false;
}

bool GoogleAdapter::StopVoiceSession() {
    return true;
}

void GoogleAdapter::SetTextResponseCallback(std::function<void(const std::string&)> callback) {
    text_callback_ = callback;
}

void GoogleAdapter::SetAudioResponseCallback(std::function<void(const std::vector<uint8_t>&)> callback) {
    audio_callback_ = callback;
}

void GoogleAdapter::SetErrorCallback(std::function<void(const std::string&)> callback) {
    error_callback_ = callback;
}

void GoogleAdapter::SetStatusCallback(std::function<void(const std::string&)> callback) {
    status_callback_ = callback;
}

std::string GoogleAdapter::CreateChatRequest(const std::string& message) {
    cJSON* root = cJSON_CreateObject();

    cJSON* contents = cJSON_CreateArray();
    cJSON* content = cJSON_CreateObject();

    cJSON* parts = cJSON_CreateArray();
    cJSON* part = cJSON_CreateObject();
    cJSON_AddStringToObject(part, "text", message.c_str());
    cJSON_AddItemToArray(parts, part);

    cJSON_AddItemToObject(content, "parts", parts);
    cJSON_AddItemToArray(contents, content);
    cJSON_AddItemToObject(root, "contents", contents);

    // 添加系统指令
    if (!config_.system_prompt.empty()) {
        cJSON* system_instruction = cJSON_CreateObject();
        cJSON* system_parts = cJSON_CreateArray();
        cJSON* system_part = cJSON_CreateObject();
        cJSON_AddStringToObject(system_part, "text", config_.system_prompt.c_str());
        cJSON_AddItemToArray(system_parts, system_part);
        cJSON_AddItemToObject(system_instruction, "parts", system_parts);
        cJSON_AddItemToObject(root, "systemInstruction", system_instruction);
    }

    char* json_string = cJSON_PrintUnformatted(root);
    std::string result(json_string);
    cJSON_free(json_string);
    cJSON_Delete(root);

    return result;
}

void GoogleAdapter::ProcessChatResponse(const std::string& response) {
    cJSON* root = cJSON_Parse(response.c_str());
    if (!root) {
        ESP_LOGE(TAG, "Failed to parse Google response");
        if (error_callback_) {
            error_callback_("Failed to parse response");
        }
        return;
    }

    cJSON* candidates = cJSON_GetObjectItem(root, "candidates");
    if (cJSON_IsArray(candidates) && cJSON_GetArraySize(candidates) > 0) {
        cJSON* candidate = cJSON_GetArrayItem(candidates, 0);
        cJSON* content = cJSON_GetObjectItem(candidate, "content");
        if (content) {
            cJSON* parts = cJSON_GetObjectItem(content, "parts");
            if (cJSON_IsArray(parts) && cJSON_GetArraySize(parts) > 0) {
                cJSON* part = cJSON_GetArrayItem(parts, 0);
                cJSON* text = cJSON_GetObjectItem(part, "text");
                if (cJSON_IsString(text) && text_callback_) {
                    text_callback_(text->valuestring);
                }
            }
        }
    } else {
        cJSON* error = cJSON_GetObjectItem(root, "error");
        if (error) {
            cJSON* message = cJSON_GetObjectItem(error, "message");
            if (cJSON_IsString(message) && error_callback_) {
                error_callback_(message->valuestring);
            }
        }
    }

    cJSON_Delete(root);
}

// Anthropic 适配器实现
AnthropicAdapter::AnthropicAdapter() {
}

AnthropicAdapter::~AnthropicAdapter() {
    Disconnect();
}

bool AnthropicAdapter::Initialize(const AIModelConfig& config) {
    config_ = config;

    if (config_.api_key.empty()) {
        ESP_LOGE(TAG, "Anthropic API key is required");
        return false;
    }

    if (config_.model_name.empty()) {
        config_.model_name = "claude-3-5-sonnet-20241022";
    }

    return true;
}

bool AnthropicAdapter::Connect() {
    if (connected_) {
        return true;
    }

    http_ = std::unique_ptr<Http>(Board::GetInstance().CreateHttp());
    if (!http_) {
        ESP_LOGE(TAG, "Failed to create HTTP client");
        return false;
    }

    // 设置Anthropic特定的头部
    http_->SetHeader("Content-Type", "application/json");
    http_->SetHeader("x-api-key", config_.api_key.c_str());
    http_->SetHeader("anthropic-version", "2023-06-01");

    connected_ = true;

    if (status_callback_) {
        status_callback_("Connected to Anthropic Claude");
    }

    return true;
}

void AnthropicAdapter::Disconnect() {
    if (http_) {
        http_.reset();
    }
    connected_ = false;
}

bool AnthropicAdapter::IsConnected() const {
    return connected_;
}

bool AnthropicAdapter::SendTextMessage(const std::string& message) {
    if (!IsConnected()) {
        ESP_LOGE(TAG, "Not connected to Anthropic");
        return false;
    }

    std::string request_body = CreateMessageRequest(message);

    http_->SetContent(request_body);

    if (!http_->Open("POST", config_.base_url)) {
        ESP_LOGE(TAG, "Failed to open HTTP connection");
        return false;
    }

    if (http_->GetStatusCode() != 200) {
        ESP_LOGE(TAG, "HTTP request failed with status: %d", http_->GetStatusCode());
        return false;
    }

    std::string response = http_->ReadAll();
    http_->Close();

    ProcessMessageResponse(response);
    return true;
}

bool AnthropicAdapter::SendAudioData(const std::vector<uint8_t>& audio_data) {
    // Anthropic Claude 目前不支持实时音频
    ESP_LOGW(TAG, "Anthropic Claude does not support real-time audio");
    return false;
}

bool AnthropicAdapter::StartVoiceSession() {
    return false;
}

bool AnthropicAdapter::StopVoiceSession() {
    return true;
}

void AnthropicAdapter::SetTextResponseCallback(std::function<void(const std::string&)> callback) {
    text_callback_ = callback;
}

void AnthropicAdapter::SetAudioResponseCallback(std::function<void(const std::vector<uint8_t>&)> callback) {
    audio_callback_ = callback;
}

void AnthropicAdapter::SetErrorCallback(std::function<void(const std::string&)> callback) {
    error_callback_ = callback;
}

void AnthropicAdapter::SetStatusCallback(std::function<void(const std::string&)> callback) {
    status_callback_ = callback;
}

std::string AnthropicAdapter::CreateMessageRequest(const std::string& message) {
    cJSON* root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "model", config_.model_name.c_str());
    cJSON_AddNumberToObject(root, "max_tokens", 1024);

    if (!config_.system_prompt.empty()) {
        cJSON_AddStringToObject(root, "system", config_.system_prompt.c_str());
    }

    cJSON* messages = cJSON_CreateArray();
    cJSON* msg = cJSON_CreateObject();
    cJSON_AddStringToObject(msg, "role", "user");
    cJSON_AddStringToObject(msg, "content", message.c_str());
    cJSON_AddItemToArray(messages, msg);
    cJSON_AddItemToObject(root, "messages", messages);

    char* json_string = cJSON_PrintUnformatted(root);
    std::string result(json_string);
    cJSON_free(json_string);
    cJSON_Delete(root);

    return result;
}

void AnthropicAdapter::ProcessMessageResponse(const std::string& response) {
    cJSON* root = cJSON_Parse(response.c_str());
    if (!root) {
        ESP_LOGE(TAG, "Failed to parse Anthropic response");
        if (error_callback_) {
            error_callback_("Failed to parse response");
        }
        return;
    }

    cJSON* content = cJSON_GetObjectItem(root, "content");
    if (cJSON_IsArray(content) && cJSON_GetArraySize(content) > 0) {
        cJSON* content_item = cJSON_GetArrayItem(content, 0);
        cJSON* text = cJSON_GetObjectItem(content_item, "text");
        if (cJSON_IsString(text) && text_callback_) {
            text_callback_(text->valuestring);
        }
    } else {
        cJSON* error = cJSON_GetObjectItem(root, "error");
        if (error) {
            cJSON* message = cJSON_GetObjectItem(error, "message");
            if (cJSON_IsString(message) && error_callback_) {
                error_callback_(message->valuestring);
            }
        }
    }

    cJSON_Delete(root);
}

// 自定义适配器实现
CustomAdapter::CustomAdapter() {
}

CustomAdapter::~CustomAdapter() {
    Disconnect();
}

bool CustomAdapter::Initialize(const AIModelConfig& config) {
    config_ = config;

    if (config_.base_url.empty()) {
        ESP_LOGE(TAG, "Custom server URL is required");
        return false;
    }

    return true;
}

AIModelType CustomAdapter::GetModelType() const {
    // 根据URL判断类型，默认为聊天完成
    if (config_.base_url.find("websocket") != std::string::npos ||
        config_.base_url.find("ws://") == 0 ||
        config_.base_url.find("wss://") == 0) {
        return AIModelType::kRealtime;
    }
    return AIModelType::kChatCompletion;
}

bool CustomAdapter::Connect() {
    if (connected_) {
        return true;
    }

    // 这里可以根据配置创建适当的协议实例
    // 为了简化，我们假设使用原有的协议系统
    connected_ = true;

    if (status_callback_) {
        status_callback_("Connected to custom server");
    }

    return true;
}

void CustomAdapter::Disconnect() {
    if (protocol_) {
        protocol_.reset();
    }
    connected_ = false;
}

bool CustomAdapter::IsConnected() const {
    return connected_;
}

bool CustomAdapter::SendTextMessage(const std::string& message) {
    if (!IsConnected()) {
        ESP_LOGE(TAG, "Not connected to custom server");
        return false;
    }

    // 实现自定义服务器的文本消息发送
    // 这里需要根据具体的自定义协议实现
    ESP_LOGW(TAG, "Custom text message sending not implemented");
    return false;
}

bool CustomAdapter::SendAudioData(const std::vector<uint8_t>& audio_data) {
    if (!IsConnected()) {
        ESP_LOGE(TAG, "Not connected to custom server");
        return false;
    }

    // 实现自定义服务器的音频数据发送
    ESP_LOGW(TAG, "Custom audio data sending not implemented");
    return false;
}

bool CustomAdapter::StartVoiceSession() {
    return IsConnected();
}

bool CustomAdapter::StopVoiceSession() {
    return true;
}

void CustomAdapter::SetTextResponseCallback(std::function<void(const std::string&)> callback) {
    text_callback_ = callback;
}

void CustomAdapter::SetAudioResponseCallback(std::function<void(const std::vector<uint8_t>&)> callback) {
    audio_callback_ = callback;
}

void CustomAdapter::SetErrorCallback(std::function<void(const std::string&)> callback) {
    error_callback_ = callback;
}

void CustomAdapter::SetStatusCallback(std::function<void(const std::string&)> callback) {
    status_callback_ = callback;
}

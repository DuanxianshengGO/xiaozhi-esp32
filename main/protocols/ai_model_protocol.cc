#include "ai_model_protocol.h"
#include "../board.h"
#include "../assets/lang_config.h"

#include <esp_log.h>
#include <cJSON.h>

#define TAG "AIModelProtocol"

AIModelProtocol::AIModelProtocol() {
    // 加载配置
    config_ = AIModelAdapter::LoadConfigFromNVS();
    
    // 创建适配器
    InitializeAdapter();
}

AIModelProtocol::~AIModelProtocol() {
    CloseAudioChannel();
}

bool AIModelProtocol::InitializeAdapter() {
    AIModelProvider provider = AIModelAdapter::GetProviderFromConfig();
    
    if (provider == AIModelProvider::kXiaozhi) {
        // 使用原有的协议，不需要适配器
        return false;
    }
    
    adapter_ = AIModelAdapter::CreateAdapter(provider);
    if (!adapter_) {
        ESP_LOGE(TAG, "Failed to create AI model adapter");
        return false;
    }
    
    if (!adapter_->Initialize(config_)) {
        ESP_LOGE(TAG, "Failed to initialize AI model adapter");
        adapter_.reset();
        return false;
    }
    
    // 设置回调函数
    adapter_->SetAudioResponseCallback([this](const std::vector<uint8_t>& audio_data) {
        OnAdapterAudioResponse(audio_data);
    });
    
    adapter_->SetTextResponseCallback([this](const std::string& text) {
        OnAdapterTextResponse(text);
    });
    
    adapter_->SetErrorCallback([this](const std::string& error) {
        OnAdapterError(error);
    });
    
    adapter_->SetStatusCallback([this](const std::string& status) {
        OnAdapterStatus(status);
    });
    
    return true;
}

bool AIModelProtocol::OpenAudioChannel() {
    if (!adapter_) {
        ESP_LOGE(TAG, "AI model adapter not initialized");
        return false;
    }
    
    if (audio_channel_opened_) {
        return true;
    }
    
    if (!adapter_->Connect()) {
        ESP_LOGE(TAG, "Failed to connect to AI model");
        return false;
    }
    
    // 对于支持实时语音的模型，启动语音会话
    if (adapter_->GetModelType() == AIModelType::kRealtime) {
        if (!adapter_->StartVoiceSession()) {
            ESP_LOGE(TAG, "Failed to start voice session");
            return false;
        }
        voice_session_active_ = true;
    }
    
    audio_channel_opened_ = true;
    ESP_LOGI(TAG, "Audio channel opened successfully");
    
    return true;
}

void AIModelProtocol::CloseAudioChannel() {
    if (!audio_channel_opened_) {
        return;
    }
    
    if (adapter_) {
        if (voice_session_active_) {
            adapter_->StopVoiceSession();
            voice_session_active_ = false;
        }
        adapter_->Disconnect();
    }
    
    audio_channel_opened_ = false;
    ESP_LOGI(TAG, "Audio channel closed");
}

bool AIModelProtocol::IsAudioChannelOpened() {
    return audio_channel_opened_ && adapter_ && adapter_->IsConnected();
}

bool AIModelProtocol::SendAudio(AudioStreamPacket&& packet) {
    if (!IsAudioChannelOpened()) {
        ESP_LOGE(TAG, "Audio channel not opened");
        return false;
    }
    
    if (adapter_->GetModelType() == AIModelType::kRealtime) {
        // 对于实时语音模型，直接发送音频数据
        std::vector<uint8_t> audio_data = ConvertAudioPacket(packet);
        return adapter_->SendAudioData(audio_data);
    } else {
        // 对于文本模型，需要先将音频转换为文本
        // 这里需要集成语音识别服务
        ESP_LOGW(TAG, "Audio to text conversion not implemented for this model type");
        return false;
    }
}

bool AIModelProtocol::SendText(const std::string& text) {
    if (!adapter_) {
        ESP_LOGE(TAG, "AI model adapter not initialized");
        return false;
    }
    
    if (!adapter_->IsConnected()) {
        ESP_LOGE(TAG, "Not connected to AI model");
        return false;
    }
    
    return adapter_->SendTextMessage(text);
}

bool AIModelProtocol::SendMcpMessage(const std::string& payload) {
    // AI模型协议暂不支持MCP消息
    // 可以考虑将MCP消息转换为文本消息发送
    ESP_LOGW(TAG, "MCP message sending not supported in AI model protocol");
    return false;
}

void AIModelProtocol::OnIncomingAudio(std::function<void(AudioStreamPacket&&)> callback) {
    audio_callback_ = callback;
}

void AIModelProtocol::OnIncomingText(std::function<void(const std::string&)> callback) {
    text_callback_ = callback;
}

void AIModelProtocol::OnMcpMessage(std::function<void(const std::string&)> callback) {
    mcp_callback_ = callback;
}

void AIModelProtocol::OnNetworkError(std::function<void(const std::string&)> callback) {
    error_callback_ = callback;
}

void AIModelProtocol::OnAdapterAudioResponse(const std::vector<uint8_t>& audio_data) {
    if (audio_callback_) {
        AudioStreamPacket packet = ConvertAudioData(audio_data);
        audio_callback_(std::move(packet));
    }
}

void AIModelProtocol::OnAdapterTextResponse(const std::string& text) {
    if (text_callback_) {
        text_callback_(text);
    }
}

void AIModelProtocol::OnAdapterError(const std::string& error) {
    ESP_LOGE(TAG, "AI model adapter error: %s", error.c_str());
    if (error_callback_) {
        error_callback_(error);
    }
}

void AIModelProtocol::OnAdapterStatus(const std::string& status) {
    ESP_LOGI(TAG, "AI model adapter status: %s", status.c_str());
}

AudioStreamPacket AIModelProtocol::ConvertAudioData(const std::vector<uint8_t>& audio_data) {
    AudioStreamPacket packet;
    packet.payload = audio_data;
    packet.sample_rate = config_.sample_rate;
    packet.frame_duration = 60; // 默认60ms帧
    return packet;
}

std::vector<uint8_t> AIModelProtocol::ConvertAudioPacket(const AudioStreamPacket& packet) {
    return packet.payload;
}

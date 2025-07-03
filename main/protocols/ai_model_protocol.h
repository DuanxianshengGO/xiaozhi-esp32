#ifndef AI_MODEL_PROTOCOL_H
#define AI_MODEL_PROTOCOL_H

#include "protocol.h"
#include "../ai_model_adapter.h"

#include <memory>
#include <string>
#include <vector>
#include <functional>

class AIModelProtocol : public Protocol {
public:
    AIModelProtocol();
    ~AIModelProtocol() override;

    // Protocol interface implementation
    bool OpenAudioChannel() override;
    void CloseAudioChannel() override;
    bool IsAudioChannelOpened() override;
    
    bool SendAudio(AudioStreamPacket&& packet) override;
    bool SendText(const std::string& text) override;
    bool SendMcpMessage(const std::string& payload) override;
    
    void OnIncomingAudio(std::function<void(AudioStreamPacket&&)> callback) override;
    void OnIncomingText(std::function<void(const std::string&)> callback) override;
    void OnMcpMessage(std::function<void(const std::string&)> callback) override;
    void OnNetworkError(std::function<void(const std::string&)> callback) override;

private:
    std::unique_ptr<AIModelAdapter> adapter_;
    AIModelConfig config_;
    
    // 回调函数
    std::function<void(AudioStreamPacket&&)> audio_callback_;
    std::function<void(const std::string&)> text_callback_;
    std::function<void(const std::string&)> mcp_callback_;
    std::function<void(const std::string&)> error_callback_;
    
    // 状态管理
    bool audio_channel_opened_ = false;
    bool voice_session_active_ = false;
    
    // 音频处理
    void OnAdapterAudioResponse(const std::vector<uint8_t>& audio_data);
    void OnAdapterTextResponse(const std::string& text);
    void OnAdapterError(const std::string& error);
    void OnAdapterStatus(const std::string& status);
    
    // 辅助方法
    bool InitializeAdapter();
    AudioStreamPacket ConvertAudioData(const std::vector<uint8_t>& audio_data);
    std::vector<uint8_t> ConvertAudioPacket(const AudioStreamPacket& packet);
};

#endif // AI_MODEL_PROTOCOL_H

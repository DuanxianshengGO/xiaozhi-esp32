#!/usr/bin/env python3
"""
AI模型适配器测试脚本

用于测试不同AI模型的连接和基本功能
"""

import json
import websocket
import threading
import time
import sys
import argparse

class AIModelTester:
    def __init__(self, device_url):
        self.device_url = device_url
        self.ws = None
        self.connected = False
        
    def connect(self):
        """连接到设备"""
        try:
            self.ws = websocket.WebSocketApp(
                self.device_url,
                on_open=self.on_open,
                on_message=self.on_message,
                on_error=self.on_error,
                on_close=self.on_close
            )
            
            # 在新线程中运行WebSocket
            wst = threading.Thread(target=self.ws.run_forever)
            wst.daemon = True
            wst.start()
            
            # 等待连接
            timeout = 10
            while not self.connected and timeout > 0:
                time.sleep(0.1)
                timeout -= 0.1
                
            return self.connected
        except Exception as e:
            print(f"连接失败: {e}")
            return False
    
    def on_open(self, ws):
        print("✅ 已连接到设备")
        self.connected = True
    
    def on_message(self, ws, message):
        try:
            data = json.loads(message)
            if data.get("type") == "mcp":
                self.handle_mcp_response(data.get("payload", {}))
            else:
                print(f"📨 收到消息: {message}")
        except json.JSONDecodeError:
            print(f"📨 收到非JSON消息: {message}")
    
    def on_error(self, ws, error):
        print(f"❌ WebSocket错误: {error}")
    
    def on_close(self, ws, close_status_code, close_msg):
        print("🔌 连接已关闭")
        self.connected = False
    
    def handle_mcp_response(self, payload):
        """处理MCP响应"""
        if "result" in payload:
            print(f"✅ MCP结果: {json.dumps(payload['result'], indent=2, ensure_ascii=False)}")
        elif "error" in payload:
            print(f"❌ MCP错误: {payload['error']}")
    
    def send_mcp_command(self, method, arguments=None):
        """发送MCP命令"""
        if not self.connected:
            print("❌ 设备未连接")
            return False
        
        message = {
            "type": "mcp",
            "payload": {
                "jsonrpc": "2.0",
                "method": "tools/call",
                "params": {
                    "name": method,
                    "arguments": arguments or {}
                },
                "id": int(time.time())
            }
        }
        
        try:
            self.ws.send(json.dumps(message))
            print(f"📤 发送命令: {method}")
            return True
        except Exception as e:
            print(f"❌ 发送失败: {e}")
            return False
    
    def test_get_supported_models(self):
        """测试获取支持的模型列表"""
        print("\n🔍 测试: 获取支持的AI模型")
        return self.send_mcp_command("get_supported_models")
    
    def test_get_current_config(self):
        """测试获取当前配置"""
        print("\n🔍 测试: 获取当前AI模型配置")
        return self.send_mcp_command("get_ai_model_config")
    
    def test_set_provider(self, provider):
        """测试设置AI模型提供商"""
        print(f"\n🔍 测试: 设置AI模型提供商为 {provider}")
        return self.send_mcp_command("set_ai_model_provider", {"provider": provider})
    
    def test_set_api_key(self, api_key):
        """测试设置API密钥"""
        print("\n🔍 测试: 设置API密钥")
        # 隐藏API密钥的显示
        masked_key = api_key[:8] + "..." + api_key[-4:] if len(api_key) > 12 else "***"
        print(f"API密钥: {masked_key}")
        return self.send_mcp_command("set_api_key", {"api_key": api_key})
    
    def test_set_model_name(self, model_name):
        """测试设置模型名称"""
        print(f"\n🔍 测试: 设置模型名称为 {model_name}")
        return self.send_mcp_command("set_model_name", {"model_name": model_name})
    
    def test_connection(self):
        """测试AI模型连接"""
        print("\n🔍 测试: 测试AI模型连接")
        return self.send_mcp_command("test_ai_model_connection")
    
    def test_reset_config(self):
        """测试重置配置"""
        print("\n🔍 测试: 重置AI模型配置")
        return self.send_mcp_command("reset_ai_model_config")

def main():
    parser = argparse.ArgumentParser(description="AI模型适配器测试工具")
    parser.add_argument("--device", default="ws://192.168.1.100:8080", 
                       help="设备WebSocket地址 (默认: ws://192.168.1.100:8080)")
    parser.add_argument("--provider", choices=["xiaozhi", "openai", "google", "anthropic", "custom"],
                       help="要测试的AI模型提供商")
    parser.add_argument("--api-key", help="API密钥")
    parser.add_argument("--model", help="模型名称")
    parser.add_argument("--test-all", action="store_true", help="运行所有测试")
    
    args = parser.parse_args()
    
    print("🚀 AI模型适配器测试工具")
    print(f"📡 连接设备: {args.device}")
    
    tester = AIModelTester(args.device)
    
    if not tester.connect():
        print("❌ 无法连接到设备")
        sys.exit(1)
    
    try:
        # 基础测试
        time.sleep(1)
        tester.test_get_supported_models()
        
        time.sleep(1)
        tester.test_get_current_config()
        
        # 如果指定了提供商，进行相关测试
        if args.provider:
            time.sleep(1)
            tester.test_set_provider(args.provider)
            
            if args.api_key:
                time.sleep(1)
                tester.test_set_api_key(args.api_key)
            
            if args.model:
                time.sleep(1)
                tester.test_set_model_name(args.model)
            
            time.sleep(1)
            tester.test_connection()
        
        if args.test_all:
            print("\n🧪 运行完整测试套件...")
            
            # 测试OpenAI
            time.sleep(1)
            tester.test_set_provider("openai")
            time.sleep(1)
            tester.test_set_model_name("gpt-4o-realtime-preview")
            
            # 测试Google
            time.sleep(1)
            tester.test_set_provider("google")
            time.sleep(1)
            tester.test_set_model_name("gemini-2.0-flash-exp")
            
            # 测试Anthropic
            time.sleep(1)
            tester.test_set_provider("anthropic")
            time.sleep(1)
            tester.test_set_model_name("claude-3-5-sonnet-20241022")
            
            # 重置配置
            time.sleep(1)
            tester.test_reset_config()
        
        print("\n✅ 测试完成")
        
    except KeyboardInterrupt:
        print("\n⏹️ 测试被用户中断")
    except Exception as e:
        print(f"\n❌ 测试过程中出现错误: {e}")
    
    # 保持连接一段时间以查看响应
    time.sleep(3)

if __name__ == "__main__":
    main()

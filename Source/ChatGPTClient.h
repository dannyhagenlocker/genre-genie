/*
  ==============================================================================

    ChatGPTClient.h
    Created: 21 May 2025 9:07:08pm
    Author:  Danny Hagenlocker

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <JuceHeader.h>


class ChatGPTClient : public juce::Thread
{
public:
    ChatGPTClient();
    ~ChatGPTClient() override;

    void sendMessageAsync(const juce::String& userMessage);
    void clearHistory();
    void resetConversation(const juce::String& systemPrompt = {});
    
    std::function<void(const juce::String& response)> onResponse;

private:
    void run() override;

    juce::String apiKey;
    juce::String apiURL;
    juce::String latestUserMessage;
    juce::Array<juce::var> messageHistory;

    juce::CriticalSection lock;
    bool shouldSend = false;

    juce::String loadApiKeyFromConfig();
    juce::String createRequestBody();
    
};

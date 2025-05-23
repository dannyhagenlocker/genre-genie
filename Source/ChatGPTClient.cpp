/*
  ==============================================================================

    ChatGPTClient.cpp
    Created: 21 May 2025 9:07:14pm
    Author:  Danny Hagenlocker

  ==============================================================================
*/

#include "ChatGPTClient.h"
#include <fstream>

ChatGPTClient::ChatGPTClient()
    : Thread("ChatGPTClientThread")
{
    // Optional system message to establish context
    const juce::String systemPrompt = "You are an expert audio engineer assistant. Your job is to listen to a user's prompt and output the appropriate EQ settings that would match the described genre, artist, or style. You will also be provided with the current state of the EQ settings in JSON format. You will return a JSON object that specifies the frequency, gain (in dB), and Q factor for each EQ band matching the format given in each prompt. You should focus on matching the tonal character and mix aesthetic described by the user. Use musical intuition and common mixing practices when making choices.";

    resetConversation(systemPrompt);  // <-- Reset chat on startup with system prompt
    startThread();
    
    
    apiKey = loadApiKeyFromConfig();
    apiURL = "https://api.openai.com/v1/chat/completions";
}

ChatGPTClient::~ChatGPTClient()
{
    stopThread(500);
}

// Utility function to load API key from embedded config.json
juce::String ChatGPTClient::loadApiKeyFromConfig()
{
    // Load the JSON text from binary resource
    juce::String jsonText = juce::String::fromUTF8(BinaryData::config_json, BinaryData::config_jsonSize);

    // Attempt to parse the JSON
    juce::var parsedJson = juce::JSON::parse(jsonText);
    if (parsedJson.isObject())
    {
        if (auto* obj = parsedJson.getDynamicObject())
        {
            if (obj->hasProperty("openai_api_key"))
            {
                auto apiKey = obj->getProperty("openai_api_key").toString();
                DBG("Loaded API key from BinaryData: " + apiKey);
                return apiKey;
            }
        }
    }

    DBG("API key not found or failed to parse embedded config.json.");
    return {};
}

void ChatGPTClient::sendMessageAsync(const juce::String& userMessage)
{
    {
        const juce::ScopedLock sl(lock);
        
        // Add user message to history
        juce::DynamicObject::Ptr userMsg = new juce::DynamicObject();
        userMsg->setProperty("role", "user");
        userMsg->setProperty("content", userMessage);
        messageHistory.add(juce::var(userMsg.get()));
        
        latestUserMessage = userMessage;
        shouldSend = true;
    }

    notify();
}

void ChatGPTClient::clearHistory()
{
    const juce::ScopedLock sl(lock);
    messageHistory.clear();
}

void ChatGPTClient::resetConversation(const juce::String& systemPrompt)
{
    const juce::ScopedLock sl(lock);
    messageHistory.clear();

    if (systemPrompt.isNotEmpty())
    {
        juce::DynamicObject::Ptr sysMsg = new juce::DynamicObject();
        sysMsg->setProperty("role", "system");
        sysMsg->setProperty("content", systemPrompt);
        messageHistory.add(juce::var(sysMsg.get()));
    }
}

void ChatGPTClient::run()
{
    while (!threadShouldExit())
    {
        wait(-1); // Wait for notify()

        juce::String body;
        {
            const juce::ScopedLock sl(lock);
            if (!shouldSend) continue;
            shouldSend = false;

            // Create request body from full history
            body = createRequestBody();
        }

        juce::URL url(apiURL);
        juce::URL urlWithPostData = url.withPOSTData(body);

        juce::StringPairArray headers;
        headers.set("Content-Type", "application/json");
        headers.set("Authorization", "Bearer " + apiKey);

        juce::StringArray headerLines;
        for (int i = 0; i < headers.size(); ++i)
            headerLines.add(headers.getAllKeys()[i] + ": " + headers.getAllValues()[i]);
        juce::String headerString = headerLines.joinIntoString("\r\n");

        juce::URL::InputStreamOptions options =
            juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
                .withHttpRequestCmd("POST")
                .withExtraHeaders(headerString)
                .withConnectionTimeoutMs(10000);

        std::unique_ptr<juce::InputStream> stream = urlWithPostData.createInputStream(options);

        if (stream)
        {
            juce::String responseString = stream->readEntireStreamAsString();

            juce::var json = juce::JSON::parse(responseString);
            if (json.isObject())
            {
                juce::var choices = json["choices"];
                if (choices.isArray() && choices[0]["message"]["content"].isString())
                {
                    auto reply = choices[0]["message"]["content"].toString().trim();

                    {
                        const juce::ScopedLock sl(lock);
                        juce::DynamicObject::Ptr assistantMsg = new juce::DynamicObject();
                        assistantMsg->setProperty("role", "assistant");
                        assistantMsg->setProperty("content", reply);
                        messageHistory.add(juce::var(assistantMsg.get()));
                    }

                    if (onResponse)
                        juce::MessageManager::callAsync([this, reply]() { onResponse(reply); });
                }
            }
        }
    }
}

juce::String ChatGPTClient::createRequestBody()
{
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty("model", "gpt-3.5-turbo");
    root->setProperty("messages", juce::var(messageHistory));
    return juce::JSON::toString(juce::var(root.get()));
}

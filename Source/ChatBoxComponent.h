#pragma once

#include <JuceHeader.h>


class ChatBoxComponent : public juce::Component, public juce::Button::Listener, public juce::TextEditor::Listener
{
public:
    ChatBoxComponent();
    ~ChatBoxComponent() override = default;

    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

    std::function<void(const juce::String&)> onUserMessage;

    void appendMessage(const juce::String& speaker, const juce::String& message);

private:
    juce::TextEditor chatDisplay;
    juce::TextEditor inputBox;
    juce::TextButton sendButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChatBoxComponent)
};

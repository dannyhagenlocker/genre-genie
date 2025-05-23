#include "ChatBoxComponent.h"

ChatBoxComponent::ChatBoxComponent()
{
    // Chat display setup
    chatDisplay.setMultiLine(true);
    chatDisplay.setReadOnly(true);
    chatDisplay.setScrollbarsShown(true);
    chatDisplay.setColour(juce::TextEditor::backgroundColourId, juce::Colour(23,23,23));
    chatDisplay.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    chatDisplay.setFont(juce::Font(20.0f));
    addAndMakeVisible(chatDisplay);

    // Input box
    inputBox.setMultiLine(false);
    inputBox.setReturnKeyStartsNewLine(false);
    inputBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(48,48,48));
    inputBox.setFont(juce::Font(20.0f));
    inputBox.addListener(this);
    addAndMakeVisible(inputBox);

    // Send button
    sendButton.setButtonText("Send");
    sendButton.addListener(this);
    addAndMakeVisible(sendButton);
}

void ChatBoxComponent::resized()
{
    auto bounds = getLocalBounds().reduced(12);

    auto inputHeight = 30;
    auto buttonWidth = 60;

    chatDisplay.setBounds(bounds.removeFromTop(bounds.getHeight() - inputHeight - 10));
    inputBox.setBounds(bounds.removeFromLeft(bounds.getWidth() - buttonWidth - 10));
    sendButton.setBounds(bounds);
}

void ChatBoxComponent::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    if (&editor == &inputBox)
        buttonClicked(&sendButton); // Reuse the send logic
}

void ChatBoxComponent::buttonClicked(juce::Button* button)
{
    if (button == &sendButton)
    {
        auto message = inputBox.getText().trim();

        if (message.isNotEmpty())
        {
            inputBox.clear();

            if (onUserMessage)
            {
                // Ensure the callback runs on a non-GUI thread if it needs to be
                std::function<void()> safeCallback = [cb = onUserMessage, message]() {
                    cb(message);
                };

                std::thread(std::move(safeCallback)).detach(); // run in background if needed
            }
        }
    }
}

void ChatBoxComponent::appendMessage(const juce::String& speaker, const juce::String& message)
{
    // Ensure GUI update happens on the message thread
    juce::MessageManager::callAsync([this, speaker, message]() {
        chatDisplay.moveCaretToEnd();
        chatDisplay.insertTextAtCaret(speaker + ": " + message + "\n");
    });
}

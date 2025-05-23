//
//  PluginColorConstants.h
//  GenreGenie
//
//  Created by Danny Hagenlocker on 5/21/25.
//

#pragma once

#include <juce_graphics/juce_graphics.h>

namespace PluginColors
{
    // Primary
    const juce::Colour accent          = juce::Colour(16u, 163u, 127u);
    const juce::Colour accentDark     = juce::Colour(0u, 172u, 1u);
    const juce::Colour textMain       = juce::Colour(245u, 247u, 250u);
    const juce::Colour background     = juce::Colour(30, 30, 46);
    const juce::Colour backgroundDark = juce::Colour(45u, 47u, 58u);
    const juce::Colour gridLine       = juce::Colours::dimgrey;
    const juce::Colour borderLine     = juce::Colours::darkgrey;

    // FFT
    const juce::Colour fftLeftChannel  = juce::Colour(97u, 18u, 167u); // purple
    const juce::Colour fftRightChannel = juce::Colour(215u, 201u, 134u);

    // Labels and text
    const juce::Colour labelMain       = juce::Colours::white;
    const juce::Colour labelAlt        = juce::Colours::lightgrey;
    const juce::Colour labelDisabledBg = juce::Colours::darkgrey;
    const juce::Colour labelDisabledFg = juce::Colours::grey;

    // Bypass/Power
    const juce::Colour bypassOn        = juce::Colours::dimgrey;
    const juce::Colour bypassOff       = accent;
} // namespace PluginColors

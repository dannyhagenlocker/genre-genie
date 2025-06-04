/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginColorConstants.h"

void LookAndFeel::drawRotarySlider(juce::Graphics & g,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider & slider)
{
    using namespace juce;
    
    auto bounds = Rectangle<float>(x, y, width, height);
    
    auto enabled = slider.isEnabled();
    
    g.setColour(enabled ? Colour(16u, 163u, 127u) : Colours::darkgrey );
    g.fillEllipse(bounds);
    
    g.setColour(enabled ? Colour(245u, 247u, 250u) : Colours::grey);
    g.drawEllipse(bounds, 1.f);
    
    if( auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();
        Path p;
        
        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);
        
        p.addRoundedRectangle(r, 2.f);
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
        
        g.fillPath(p);
        
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());
        
        g.setColour(enabled ? Colours::black : Colours::darkgrey);
        g.fillRect(r);
        
        g.setColour(enabled ? Colours::white : Colours::lightgrey);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

void LookAndFeel::drawToggleButton(juce::Graphics &g,
                                   juce::ToggleButton &toggleButton,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    using namespace juce;
    
    if( auto* pb = dynamic_cast<PowerButton*>(&toggleButton) )
    {
        Path powerButton;
        
        auto bounds = toggleButton.getLocalBounds();
        
        auto size = jmin(bounds.getWidth(), bounds.getHeight()) - 6;
        auto r = bounds.withSizeKeepingCentre(size, size).toFloat();
        
        float ang = 30.f; //30.f;
        
        size -= 6;
        
        powerButton.addCentredArc(r.getCentreX(),
                                  r.getCentreY(),
                                  size * 0.5,
                                  size * 0.5,
                                  0.f,
                                  degreesToRadians(ang),
                                  degreesToRadians(360.f - ang),
                                  true);
        
        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());
        
        PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);
        
        auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colour(16u, 163u, 127u);
        
        g.setColour(color);
        g.strokePath(powerButton, pst);
        g.drawEllipse(r, 2);
    }
    else if( auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton) )
    {
        auto color = ! toggleButton.getToggleState() ? Colours::dimgrey : Colour(16u, 163u, 127u);
        
        g.setColour(color);
        
        auto bounds = toggleButton.getLocalBounds();
        g.drawRect(bounds);
        
        g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
    }
}
//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;
    
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    
    auto range = getRange();
    
    auto sliderBounds = getSliderBounds();
    
//    g.setColour(Colours::red);
//    g.drawRect(getLocalBounds());
//    g.setColour(Colours::yellow);
//    g.drawRect(sliderBounds);
    
    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                      startAng,
                                      endAng,
                                      *this);
    
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;
    
    g.setColour(Colour(16u, 163u, 127u));
    g.setFont(getTextHeight());
    
    auto numChoices = labels.size();
    for( int i = 0; i < numChoices; ++i )
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);
        
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);
        
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
    
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    
    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    
    return r;
    
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if( auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param) )
        return choiceParam->getCurrentChoiceName();
    
    juce::String str;
    bool addK = false;
    
    if( auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param) )
    {
        float val = getValue();
        
        if( val > 999.f )
        {
            val /= 1000.f; //1001 / 1000 = 1.001
            addK = true;
        }
        
        str = juce::String(val, (addK ? 2 : 0));
    }
    else
    {
        jassertfalse; //this shouldn't happen!
    }
    
    if( suffix.isNotEmpty() )
    {
        str << " ";
        if( addK )
            str << "k";
        
        str << suffix;
    }
    
    return str;
}
//==============================================================================
ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor& p) :
audioProcessor(p),
leftPathProducer(audioProcessor.leftChannelFifo),
rightPathProducer(audioProcessor.rightChannelFifo)
{
    const auto& params = audioProcessor.getParameters();
    for( auto param : params )
    {
        param->addListener(this);
    }

    updateChain();
    
    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for( auto param : params )
    {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::updateResponseCurve()
{
    using namespace juce;
    auto responseArea = getAnalysisArea();
    
    auto w = responseArea.getWidth();
    
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    std::vector<double> mags;
    
    mags.resize(w);
    
    for( int i = 0; i < w; ++i )
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);
        
        if(! monoChain.isBypassed<ChainPositions::Peak>() )
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if( !monoChain.isBypassed<ChainPositions::LowCut>() )
        {
            if( !lowcut.isBypassed<0>() )
                mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if( !lowcut.isBypassed<1>() )
                mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if( !lowcut.isBypassed<2>() )
                mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if( !lowcut.isBypassed<3>() )
                mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        
        if( !monoChain.isBypassed<ChainPositions::HighCut>() )
        {
            if( !highcut.isBypassed<0>() )
                mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if( !highcut.isBypassed<1>() )
                mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if( !highcut.isBypassed<2>() )
                mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if( !highcut.isBypassed<3>() )
                mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
            
        mags[i] = Decibels::gainToDecibels(mag);
    }
    
    responseCurve.clear();
    
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
    
    for( size_t i = 1; i < mags.size(); ++i )
    {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }
}

void ResponseCurveComponent::paint (juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colour(23, 23, 23));

    drawBackgroundGrid(g);
    
    auto responseArea = getAnalysisArea();
    
    if( shouldShowFFTAnalysis )
    {
        auto leftChannelFFTPath = leftPathProducer.getPath();
        leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
        
        g.setColour(Colour(97u, 18u, 167u)); //purple-
        g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));
        
        auto rightChannelFFTPath = rightPathProducer.getPath();
        rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
        
        g.setColour(Colour(215u, 201u, 134u));
        g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
    }
    
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
    
    Path border;
    
    border.setUsingNonZeroWinding(false);
    
    border.addRoundedRectangle(getRenderArea(), 4);
    border.addRectangle(getLocalBounds());
    
    g.setColour(Colour(33, 33, 33));
    
    g.fillPath(border);
    
    drawTextLabels(g);
    
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);
}

std::vector<float> ResponseCurveComponent::getFrequencies()
{
    return std::vector<float>
    {
        20, /*30, 40,*/ 50, 100,
        200, /*300, 400,*/ 500, 1000,
        2000, /*3000, 4000,*/ 5000, 10000,
        20000
    };
}

std::vector<float> ResponseCurveComponent::getGains()
{
    return std::vector<float>
    {
        -24, -12, 0, 12, 24
    };
}

std::vector<float> ResponseCurveComponent::getXs(const std::vector<float> &freqs, float left, float width)
{
    std::vector<float> xs;
    for( auto f : freqs )
    {
        auto normX = juce::mapFromLog10(f, 20.f, 20000.f);
        xs.push_back( left + width * normX );
    }
    
    return xs;
}

void ResponseCurveComponent::drawBackgroundGrid(juce::Graphics &g)
{
    using namespace juce;
    auto freqs = getFrequencies();
    
    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();
    
    auto xs = getXs(freqs, left, width);
    
    g.setColour(Colours::dimgrey);
    for( auto x : xs )
    {
        g.drawVerticalLine(x, top, bottom);
    }
    
    auto gain = getGains();
    
    for( auto gDb : gain )
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        
        g.setColour(gDb == 0.f ? Colour(16u, 163u, 127u) : Colours::darkgrey );
        g.drawHorizontalLine(y, left, right);
    }
}

void ResponseCurveComponent::drawTextLabels(juce::Graphics &g)
{
    using namespace juce;
    g.setColour(Colours::lightgrey);
    const int fontHeight = 10;
    g.setFont(fontHeight);
    
    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();
    
    auto freqs = getFrequencies();
    auto xs = getXs(freqs, left, width);
    
    for( int i = 0; i < freqs.size(); ++i )
    {
        auto f = freqs[i];
        auto x = xs[i];

        bool addK = false;
        String str;
        if( f > 999.f )
        {
            addK = true;
            f /= 1000.f;
        }

        str << f;
        if( addK )
            str << "k";
        str << "Hz";
        
        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;

        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(1);
        
        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }
    
    auto gain = getGains();

    for( auto gDb : gain )
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        
        String str;
        if( gDb > 0 )
            str << "+";
        str << gDb;
        
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(getWidth() - textWidth);
        r.setCentre(r.getCentreX(), y);
        
        g.setColour(gDb == 0.f ? Colour(16u, 163u, 127u) : Colours::lightgrey );
        
        g.drawFittedText(str, r, juce::Justification::centredLeft, 1);
        
        str.clear();
        str << (gDb - 24.f);

        r.setX(1);
        textWidth = g.getCurrentFont().getStringWidth(str);
        r.setSize(textWidth, fontHeight);
        g.setColour(Colours::lightgrey);
        g.drawFittedText(str, r, juce::Justification::centredLeft, 1);
    }
}

void ResponseCurveComponent::resized()
{
    using namespace juce;
    
    responseCurve.preallocateSpace(getWidth() * 3);
    updateResponseCurve();
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;
    while( leftChannelFifo->getNumCompleteBuffersAvailable() > 0 )
    {
        if( leftChannelFifo->getAudioBuffer(tempIncomingBuffer) )
        {
            auto size = tempIncomingBuffer.getNumSamples();

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
                                              monoBuffer.getReadPointer(0, size),
                                              monoBuffer.getNumSamples() - size);

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                                              tempIncomingBuffer.getReadPointer(0, 0),
                                              size);
            
            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
        }
    }
    
    const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();
    const auto binWidth = sampleRate / double(fftSize);

    while( leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0 )
    {
        std::vector<float> fftData;
        if( leftChannelFFTDataGenerator.getFFTData( fftData) )
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
        }
    }
    
    while( pathProducer.getNumPathsAvailable() > 0 )
    {
        pathProducer.getPath( leftChannelFFTPath );
    }
}

void ResponseCurveComponent::timerCallback()
{
    if( shouldShowFFTAnalysis )
    {
        auto fftBounds = getAnalysisArea().toFloat();
        auto sampleRate = audioProcessor.getSampleRate();
        
        leftPathProducer.process(fftBounds, sampleRate);
        rightPathProducer.process(fftBounds, sampleRate);
    }

    if( parametersChanged.compareAndSetBool(false, true) )
    {
        updateChain();
        updateResponseCurve();
    }
    
    repaint();
}

void ResponseCurveComponent::updateChain()
{
    auto chainSettings = getChainSettings(audioProcessor.apvts);
    
    monoChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    monoChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
    monoChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    
    auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
    updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    
    auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
    
    updateCutFilter(monoChain.get<ChainPositions::LowCut>(),
                    lowCutCoefficients,
                    chainSettings.lowCutSlope);
    
    updateCutFilter(monoChain.get<ChainPositions::HighCut>(),
                    highCutCoefficients,
                    chainSettings.highCutSlope);
}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();
    
    bounds.removeFromTop(12);
    bounds.removeFromBottom(2);
    bounds.removeFromLeft(20);
    bounds.removeFromRight(20);
    
    return bounds;
}


juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
    auto bounds = getRenderArea();
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    return bounds;
}
//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "db/Oct"),

compThresholdSlider(*audioProcessor.apvts.getParameter("Comp Threshold"), "dB"),
compRatioSlider(*audioProcessor.apvts.getParameter("Comp Ratio"), ":1"),
compAttackSlider(*audioProcessor.apvts.getParameter("Comp Attack"), "ms"),
compReleaseSlider(*audioProcessor.apvts.getParameter("Comp Release"), "ms"),

distortionAmountSlider(*audioProcessor.apvts.getParameter("Distortion Amount"), ""),
delayTimeSlider(*audioProcessor.apvts.getParameter("Delay Time"), "ms"),
delayFeedbackSlider(*audioProcessor.apvts.getParameter("Delay Feedback"), ""),
delayMixSlider(*audioProcessor.apvts.getParameter("Delay Mix"), "%"),
reverbSizeSlider(*audioProcessor.apvts.getParameter("Reverb Size"), ""),
reverbDecaySlider(*audioProcessor.apvts.getParameter("Reverb Decay"), "s"),
reverbMixSlider(*audioProcessor.apvts.getParameter("Reverb Mix"), "%"),


responseCurveComponent(audioProcessor),

peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),

compThresholdSliderAttachment(audioProcessor.apvts, "Comp Threshold", compThresholdSlider),
compRatioSliderAttachment(audioProcessor.apvts, "Comp Ratio", compRatioSlider),
compAttackSliderAttachment(audioProcessor.apvts, "Comp Attack", compAttackSlider),
compReleaseSliderAttachment(audioProcessor.apvts, "Comp Release", compReleaseSlider),

distortionAmountAttachment(audioProcessor.apvts, "Distortion Amount", distortionAmountSlider),
delayTimeAttachment(audioProcessor.apvts, "Delay Time", delayTimeSlider),
delayFeedbackAttachment(audioProcessor.apvts, "Delay Feedback", delayFeedbackSlider),
delayMixAttachment(audioProcessor.apvts, "Delay Mix", delayMixSlider),
reverbSizeAttachment(audioProcessor.apvts, "Reverb Size", reverbSizeSlider),
reverbDecayAttachment(audioProcessor.apvts, "Reverb Decay", reverbDecaySlider),
reverbMixAttachment(audioProcessor.apvts, "Reverb Mix", reverbMixSlider),

lowcutBypassButtonAttachment(audioProcessor.apvts, "LowCut Bypassed", lowcutBypassButton),
peakBypassButtonAttachment(audioProcessor.apvts, "Peak Bypassed", peakBypassButton),
highcutBypassButtonAttachment(audioProcessor.apvts, "HighCut Bypassed", highcutBypassButton),
compBypassButtonAttachment(audioProcessor.apvts, "Comp Bypassed", compBypassButton),
distortionBypassAttachment(audioProcessor.apvts, "Distortion Bypassed", distortionBypassButton),
delayBypassAttachment(audioProcessor.apvts, "Delay Bypassed", delayBypassButton),
reverbBypassAttachment(audioProcessor.apvts, "Reverb Bypassed", reverbBypassButton),
analyzerEnabledButtonAttachment(audioProcessor.apvts, "Analyzer Enabled", analyzerEnabledButton)
{
    peakFreqSlider.labels.add({0.f, "20Hz"});
    peakFreqSlider.labels.add({1.f, "20kHz"});
    
    peakGainSlider.labels.add({0.f, "-24dB"});
    peakGainSlider.labels.add({1.f, "+24dB"});
    
    peakQualitySlider.labels.add({0.f, "0.1"});
    peakQualitySlider.labels.add({1.f, "10.0"});
    
    lowCutFreqSlider.labels.add({0.f, "20Hz"});
    lowCutFreqSlider.labels.add({1.f, "20kHz"});
    
    highCutFreqSlider.labels.add({0.f, "20Hz"});
    highCutFreqSlider.labels.add({1.f, "20kHz"});
    
    lowCutSlopeSlider.labels.add({0.0f, "12"});
    lowCutSlopeSlider.labels.add({1.f, "48"});
    
    highCutSlopeSlider.labels.add({0.0f, "12"});
    highCutSlopeSlider.labels.add({1.f, "48"});
    
    compThresholdSlider.labels.add({0.f, "-60dB"});
    compThresholdSlider.labels.add({1.f, "0dB"});
    compRatioSlider.labels.add({0.f, "1:1"});
    compRatioSlider.labels.add({1.f, "20:1"});
    compAttackSlider.labels.add({0.f, "1ms"});
    compAttackSlider.labels.add({1.f, "100ms"});
    compReleaseSlider.labels.add({0.f, "10ms"});
    compReleaseSlider.labels.add({1.f, "500ms"});
    
    // Distortion
    distortionAmountSlider.labels.add({ 0.f, "Soft" });
    distortionAmountSlider.labels.add({ 1.f, "Hard" });

    // Delay
    delayTimeSlider.labels.add({ 0.f, "1ms" });
    delayTimeSlider.labels.add({ 1.f, "2000ms" });

    delayFeedbackSlider.labels.add({ 0.f, "0%" });
    delayFeedbackSlider.labels.add({ 1.f, "95%" });

    delayMixSlider.labels.add({ 0.f, "Dry" });
    delayMixSlider.labels.add({ 1.f, "Wet" });

    // Reverb
    reverbSizeSlider.labels.add({ 0.f, "Small" });
    reverbSizeSlider.labels.add({ 1.f, "Large" });

    reverbDecaySlider.labels.add({ 0.f, "Short" });
    reverbDecaySlider.labels.add({ 1.f, "Long" });

    reverbMixSlider.labels.add({ 0.f, "Dry" });
    reverbMixSlider.labels.add({ 1.f, "Wet" });


    
    for( auto* comp : getComps() )
    {
        addAndMakeVisible(comp);
    }
    
    peakBypassButton.setLookAndFeel(&lnf);
    highcutBypassButton.setLookAndFeel(&lnf);
    lowcutBypassButton.setLookAndFeel(&lnf);
    compBypassButton.setLookAndFeel(&lnf);
    distortionBypassButton.setLookAndFeel(&lnf);
    delayBypassButton.setLookAndFeel(&lnf);
    reverbBypassButton.setLookAndFeel(&lnf);

    analyzerEnabledButton.setLookAndFeel(&lnf);
    
    auto safePtr = juce::Component::SafePointer<SimpleEQAudioProcessorEditor>(this);
    peakBypassButton.onClick = [safePtr]()
    {
        if( auto* comp = safePtr.getComponent() )
        {
            auto bypassed = comp->peakBypassButton.getToggleState();
            
            comp->peakFreqSlider.setEnabled( !bypassed );
            comp->peakGainSlider.setEnabled( !bypassed );
            comp->peakQualitySlider.setEnabled( !bypassed );
        }
    };
    

    lowcutBypassButton.onClick = [safePtr]()
    {
        if( auto* comp = safePtr.getComponent() )
        {
            auto bypassed = comp->lowcutBypassButton.getToggleState();
            
            comp->lowCutFreqSlider.setEnabled( !bypassed );
            comp->lowCutSlopeSlider.setEnabled( !bypassed );
        }
    };
    
    highcutBypassButton.onClick = [safePtr]()
    {
        if( auto* comp = safePtr.getComponent() )
        {
            auto bypassed = comp->highcutBypassButton.getToggleState();
            
            comp->highCutFreqSlider.setEnabled( !bypassed );
            comp->highCutSlopeSlider.setEnabled( !bypassed );
        }
    };
    
    compBypassButton.onClick = [safePtr]()
    {
        if( auto* comp = safePtr.getComponent() )
        {
            auto bypassed = comp->compBypassButton.getToggleState();
            comp->compThresholdSlider.setEnabled(!bypassed);
            comp->compRatioSlider.setEnabled(!bypassed);
            comp->compAttackSlider.setEnabled(!bypassed);
            comp->compReleaseSlider.setEnabled(!bypassed);
        }
    };
    
    distortionBypassButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            bool bypassed = comp->distortionBypassButton.getToggleState();
            comp->distortionAmountSlider.setEnabled(!bypassed);
        }
    };

    delayBypassButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            bool bypassed = comp->delayBypassButton.getToggleState();
            comp->delayTimeSlider.setEnabled(!bypassed);
            comp->delayFeedbackSlider.setEnabled(!bypassed);
            comp->delayMixSlider.setEnabled(!bypassed);
        }
    };

    reverbBypassButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            bool bypassed = comp->reverbBypassButton.getToggleState();
            comp->reverbSizeSlider.setEnabled(!bypassed);
            comp->reverbDecaySlider.setEnabled(!bypassed);
            comp->reverbMixSlider.setEnabled(!bypassed);
        }
    };

    analyzerEnabledButton.onClick = [safePtr]()
    {
        if( auto* comp = safePtr.getComponent() )
        {
            auto enabled = comp->analyzerEnabledButton.getToggleState();
            comp->responseCurveComponent.toggleAnalysisEnablement(enabled);
        }
    };
    
    setSize (1000, 500);
    
    addAndMakeVisible(chatBox);
    
    chatBox.onUserMessage = [this](const juce::String& userInput)
    {
        chatBox.appendMessage("You", userInput);

        // Inject JSON state
        juce::String jsonState = getJSONFromParameters();
        juce::String modifiedInput = userInput + R"(

        You are an expert audio engineer assistant. Your job is to listen to a user's prompt and output the appropriate EQ, compressor, distortion, delay, and reverb settings that would match the described genre, artist, or style. You will also be provided with the current state of the settings in JSON format.

        Please return:
        1. A new JSON object (wrapped in triple backticks and marked as ```json ```) that contains the updated settings in the exact same format as they were provided.
        2. Then, after closing the code block, write a plain text explanation (no markdown, no formatting) on a new line. Do not include any backticks or formatting after the JSON block. Do not reopen or continue the code block after the explanation.

        Here is the current plugin state in JSON format, follow its structure exactly:
        )" + jsonState + R"(

        Now, respond with the updated parameters and explanation.
        )";

        chatClient.sendMessageAsync(modifiedInput);
    };


    chatClient.onResponse = [this](const juce::String& reply)
    {
        std::cout << reply;
        juce::MessageManager::callAsync([this, reply]()
        {
            // Utility to extract JSON block from triple-backtick markdown
            auto extractJSONFromResponse = [](const juce::String& response) -> juce::String
            {
                int start = response.indexOf(juce::String("```json"));
                if (start == -1)
                    return {};

                start += juce::String("```json").length();
                juce::String remaining = response.substring(start);
                int relativeEnd = remaining.indexOf(juce::String("```"));
                if (relativeEnd == -1)
                    return {};

                int end = start + relativeEnd;
                return response.substring(start, end).trim();
            };

            auto extractExplanation = [](const juce::String& response) -> juce::String
            {
                int codeStart = response.indexOf(juce::String("```json"));
                int codeEnd = -1;
                if (codeStart != -1)
                {
                    juce::String remaining = response.substring(codeStart + juce::String("```json").length());
                    int relativeEnd = remaining.indexOf(juce::String("```"));
                    if (relativeEnd != -1)
                    {
                        codeEnd = codeStart + juce::String("```json").length() + relativeEnd + juce::String("```").length();
                    }
                }

                // Return everything except the JSON block
                if (codeStart != -1 && codeEnd != -1)
                    return (response.substring(0, codeStart) + "\n" + response.substring(codeEnd)).trim();

                return response.trim(); // fallback: return whole thing
            };

            const juce::String jsonBlock = extractJSONFromResponse(reply);
            const juce::String explanation = extractExplanation(reply);

            if (!explanation.isEmpty())
                chatBox.appendMessage("Genie", explanation);

            if (!jsonBlock.isEmpty()) {
                //chatBox.appendMessage("Bot (JSON)", jsonBlock);
                applyParametersFromJSON(jsonBlock);  // <- Apply to plugin
            }
        });
    };

}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
    peakBypassButton.setLookAndFeel(nullptr);
    highcutBypassButton.setLookAndFeel(nullptr);
    lowcutBypassButton.setLookAndFeel(nullptr);

    analyzerEnabledButton.setLookAndFeel(nullptr);
}

juce::String SimpleEQAudioProcessorEditor::getJSONFromParameters() const
{
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    juce::Array<juce::var> eqParams;

    auto addParam = [&](const juce::String& id, const juce::String& type, const juce::String& unit,
                        std::optional<std::pair<float, float>> range,
                        std::optional<std::vector<int>> choices,
                        float currentVal)
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("id", id);
        obj->setProperty("type", type);
        obj->setProperty("unit", unit);
        obj->setProperty("current", currentVal);

        if (range.has_value())
        {
            juce::Array<juce::var> rangeArray;
            rangeArray.add(range->first);
            rangeArray.add(range->second);
            obj->setProperty("range", juce::var(rangeArray));
        }

        if (choices.has_value())
        {
            juce::Array<juce::var> choiceArray;
            for (auto c : *choices)
                choiceArray.add(c);
            obj->setProperty("choices", juce::var(choiceArray));
        }

        eqParams.add(juce::var(obj));
    };

    // Populate with your parameters
    addParam("LowCut Freq", "Low Cut", "Hz", {{20, 20000}}, std::nullopt, lowCutFreqSlider.getValue());
    addParam("LowCut Slope", "Low Cut Slope", "dB/Oct", std::nullopt, {{12, 24, 36, 48}}, lowCutSlopeSlider.getValue());
    addParam("Peak Freq", "Peak Band", "Hz", {{20, 20000}}, std::nullopt, peakFreqSlider.getValue());
    addParam("Peak Gain", "Peak Gain", "dB", {{-24, 24}}, std::nullopt, peakGainSlider.getValue());
    addParam("Peak Quality", "Q (Bandwidth)", "Q", {{0.1f, 10.f}}, std::nullopt, peakQualitySlider.getValue());
    addParam("HighCut Freq", "High Cut", "Hz", {{20, 20000}}, std::nullopt, highCutFreqSlider.getValue());
    addParam("HighCut Slope", "High Cut Slope", "dB/Oct", std::nullopt, {{12, 24, 36, 48}}, highCutSlopeSlider.getValue());
    
    addParam("Comp Threshold", "Compressor", "dB", {{-60.f, 0.f}}, std::nullopt, compThresholdSlider.getValue());
    addParam("Comp Ratio", "Compressor", ":1", {{1.f, 20.f}}, std::nullopt, compRatioSlider.getValue());
    addParam("Comp Attack", "Compressor", "ms", {{1.f, 100.f}}, std::nullopt, compAttackSlider.getValue());
    addParam("Comp Release", "Compressor", "ms", {{10.f, 500.f}}, std::nullopt, compReleaseSlider.getValue());
    
    // Distortion
    addParam("Distortion Amount", "Distortion", "", {{0.f, 1.f}}, std::nullopt, distortionAmountSlider.getValue());

    // Delay
    addParam("Delay Time", "Delay", "ms", {{1.f, 2000.f}}, std::nullopt, delayTimeSlider.getValue());
    addParam("Delay Feedback", "Delay", "", {{0.f, 0.95f}}, std::nullopt, delayFeedbackSlider.getValue());
    addParam("Delay Mix", "Delay", "%", {{0.f, 100.f}}, std::nullopt, delayMixSlider.getValue());

    // Reverb
    addParam("Reverb Size", "Reverb", "", {{0.f, 1.f}}, std::nullopt, reverbSizeSlider.getValue());
    addParam("Reverb Decay", "Reverb", "s", {{0.f, 10.f}}, std::nullopt, reverbDecaySlider.getValue());
    addParam("Reverb Mix", "Reverb", "%", {{0.f, 100.f}}, std::nullopt, reverbMixSlider.getValue());

    root->setProperty("eq_parameters", juce::var(eqParams));
    
    DBG(juce::JSON::toString(juce::var(root.get()), true));
    return juce::JSON::toString(juce::var(root.get()), true);
}

void SimpleEQAudioProcessorEditor::applyParametersFromJSON(const juce::String& jsonString)
{
    juce::var parsed = juce::JSON::parse(jsonString);
    if (!parsed.isObject())
        return;

    auto* obj = parsed.getDynamicObject();
    if (!obj)
        return;

    juce::var eqParams = obj->getProperty("eq_parameters");
    if (!eqParams.isArray())
        return;

    for (const juce::var& paramVar : *eqParams.getArray())
    {
        auto* paramObj = paramVar.getDynamicObject();
        if (!paramObj)
            continue;

        juce::String id    = paramObj->getProperty("id").toString();
        juce::String type  = paramObj->getProperty("type").toString();  // optional
        float currentValue = static_cast<float>(paramObj->getProperty("current"));

        if (auto* p = audioProcessor.apvts.getParameter(id))
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost(p->convertTo0to1(currentValue));
            p->endChangeGesture();
        }
    }
}


//==============================================================================
void SimpleEQAudioProcessorEditor::paint(juce::Graphics &g)
{
    using namespace juce;
    
    g.fillAll (Colour(33u, 33u, 33u));
    
    Path curve;
    
    auto bounds = getLocalBounds();
    auto center = bounds.getCentre();
    
    g.setFont(Font("Iosevka Term Slab", 30, 0)); //https://github.com/be5invis/Iosevka
    
    String title { "GenreGenie" };
    g.setFont(30);
    auto titleWidth = g.getCurrentFont().getStringWidth(title) + 25;
    
    curve.startNewSubPath(center.x, 32);
    curve.lineTo(center.x - titleWidth * 0.45f, 32);
    
    auto cornerSize = 20;
    auto curvePos = curve.getCurrentPosition();
    curve.quadraticTo(curvePos.getX() - cornerSize, curvePos.getY(),
                      curvePos.getX() - cornerSize, curvePos.getY() - 16);
    curvePos = curve.getCurrentPosition();
    curve.quadraticTo(curvePos.getX(), 2,
                      curvePos.getX() - cornerSize, 2);
    
    curve.lineTo({0.f, 2.f});
    curve.lineTo(0.f, 0.f);
    curve.lineTo(center.x, 0.f);
    curve.closeSubPath();
    
    g.setColour(Colour(23, 23, 23));
    g.fillPath(curve);
    
    curve.applyTransform(AffineTransform().scaled(-1, 1));
    curve.applyTransform(AffineTransform().translated(getWidth(), 0));
    g.fillPath(curve);
    
    
    g.setColour(Colour(245u, 247u, 250u));
    g.drawFittedText(title, bounds, juce::Justification::centredTop, 1);
    g.setFont(14);
    
    // How far below the slider to draw the label (adjust as needed)
    constexpr int labelOffset = 6;
    constexpr int labelHeight = 16;
    auto drawSliderLabelBelow = [&](juce::Graphics& g, const juce::Component& comp, const juce::String& text)
    {
        auto bounds = comp.getBounds();
        juce::Rectangle<int> labelArea(bounds.getX(), bounds.getBottom() + labelOffset, bounds.getWidth(), labelHeight);
        g.drawFittedText(text, labelArea, juce::Justification::centredTop, 1);
    };
    drawSliderLabelBelow(g, lowCutSlopeSlider, "LowCut");
    drawSliderLabelBelow(g, peakQualitySlider, "Peak");
    drawSliderLabelBelow(g, highCutSlopeSlider, "HighCut");
    
    drawSliderLabelBelow(g, compThresholdSlider, "Threshold");
    drawSliderLabelBelow(g, compRatioSlider, "Ratio");
    drawSliderLabelBelow(g, compAttackSlider, "Attack");
    drawSliderLabelBelow(g, compReleaseSlider, "Release");
    
    drawSliderLabelBelow(g, distortionAmountSlider, "Distortion");
    drawSliderLabelBelow(g, delayTimeSlider, "Time");
    drawSliderLabelBelow(g, delayFeedbackSlider, "Feedback");
    drawSliderLabelBelow(g, delayMixSlider, "Mix");
    drawSliderLabelBelow(g, reverbSizeSlider, "Size");
    drawSliderLabelBelow(g, reverbDecaySlider, "Decay");
    drawSliderLabelBelow(g, reverbMixSlider, "Mix");

    
    auto buildDate = Time::getCompilationDate().toString(true, false);
    auto buildTime = Time::getCompilationDate().toString(false, true);
    g.setFont(12);
    auto topRightArea = getLocalBounds().removeFromTop(30).removeFromRight(150);
    g.drawFittedText("Build: " + buildDate + "\n" + buildTime, topRightArea, Justification::topRight, 2);
}












void SimpleEQAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(10);        // Top margin
    bounds.removeFromLeft(10);       // Left padding
    bounds.removeFromRight(10);      // Right padding

    // Divide into thirds
    auto thirdWidth = bounds.getWidth() / 3;
    auto leftColumn = bounds.removeFromLeft(thirdWidth);
    auto middleColumn = bounds.removeFromLeft(thirdWidth);
    auto rightColumn = bounds;

    // === LEFT COLUMN ===

//    auto analyzerEnabledArea = leftColumn.removeFromTop(25);
//    analyzerEnabledArea.setWidth(50);
//    analyzerEnabledArea.setX(5);
//    analyzerEnabledButton.setBounds(analyzerEnabledArea);
//    leftColumn.removeFromTop(15);

    auto responseHeight = leftColumn.getHeight() * 0.5f;
    auto responseArea = leftColumn.removeFromTop(responseHeight);
    responseCurveComponent.setBounds(responseArea);

    chatBox.setBounds(leftColumn);

    // === MIDDLE COLUMN ===
    middleColumn.removeFromTop(40);
    auto eqArea = middleColumn.removeFromTop(middleColumn.getHeight() * 0.6f);
    auto eqColumnWidth = eqArea.getWidth() / 3;

    const int largeKnobHeight = 100;
    const int smallKnobHeight = 70;
    const int knobPadding = 10;

    auto lowCutArea = eqArea.removeFromLeft(eqColumnWidth);
    auto peakArea = eqArea.removeFromLeft(eqColumnWidth);
    auto highCutArea = eqArea;

    lowcutBypassButton.setBounds(lowCutArea.removeFromTop(25));
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(largeKnobHeight));
    lowCutArea.removeFromTop(knobPadding);
    lowCutSlopeSlider.setBounds(lowCutArea.removeFromTop(largeKnobHeight));

    peakBypassButton.setBounds(peakArea.removeFromTop(25));
    peakFreqSlider.setBounds(peakArea.removeFromTop(smallKnobHeight));
    peakArea.removeFromTop(knobPadding);
    peakGainSlider.setBounds(peakArea.removeFromTop(smallKnobHeight));
    peakArea.removeFromTop(knobPadding);
    peakQualitySlider.setBounds(peakArea.removeFromTop(smallKnobHeight));

    highcutBypassButton.setBounds(highCutArea.removeFromTop(25));
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(largeKnobHeight));
    highCutArea.removeFromTop(knobPadding);
    highCutSlopeSlider.setBounds(highCutArea.removeFromTop(largeKnobHeight));

    // Compressor Section
    middleColumn.removeFromTop(25);
    const int compBypassHeight = 25;
    compBypassButton.setBounds(middleColumn.removeFromTop(compBypassHeight));
    middleColumn.removeFromTop(5); // spacing

    auto compKnobWidth = middleColumn.getWidth() / 4;
    compThresholdSlider.setBounds(middleColumn.removeFromLeft(compKnobWidth));
    compRatioSlider.setBounds(middleColumn.removeFromLeft(compKnobWidth));
    compAttackSlider.setBounds(middleColumn.removeFromLeft(compKnobWidth));
    compReleaseSlider.setBounds(middleColumn);

    // === RIGHT COLUMN: Distortion, Delay (row), Reverb (row) ===

    const int fxKnobSize = 90;
    const int fxKnobSpacing = 10;
    const int distortionKnobSize = 120;
    const int fxSectionSpacing = 30; // more spacing between sections

    // --- Distortion (solo knob) ---
    auto distortionTop = rightColumn.removeFromTop(distortionKnobSize + 25 + fxKnobSpacing);
    distortionBypassButton.setBounds(distortionTop.removeFromTop(25));
    distortionAmountSlider.setBounds(distortionTop.withSizeKeepingCentre(distortionKnobSize, distortionKnobSize));

    rightColumn.removeFromTop(fxSectionSpacing); // more separation from next section

    // --- Delay (row of 3 knobs) ---
    auto delayTop = rightColumn.removeFromTop(fxKnobSize + 25);
    delayBypassButton.setBounds(delayTop.removeFromTop(25));

    auto delayRow = delayTop;
    auto delayKnobWidth = delayRow.getWidth() / 3;
    delayTimeSlider.setBounds(delayRow.removeFromLeft(delayKnobWidth));
    delayFeedbackSlider.setBounds(delayRow.removeFromLeft(delayKnobWidth));
    delayMixSlider.setBounds(delayRow);

    rightColumn.removeFromTop(fxSectionSpacing); // more separation from next section

    // --- Reverb (row of 3 knobs) ---
    auto reverbTop = rightColumn.removeFromTop(fxKnobSize + 25);
    reverbBypassButton.setBounds(reverbTop.removeFromTop(25));

    auto reverbRow = reverbTop;
    auto reverbKnobWidth = reverbRow.getWidth() / 3;
    reverbSizeSlider.setBounds(reverbRow.removeFromLeft(reverbKnobWidth));
    reverbDecaySlider.setBounds(reverbRow.removeFromLeft(reverbKnobWidth));
    reverbMixSlider.setBounds(reverbRow);
}













std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider,
        &responseCurveComponent,
        
        &lowcutBypassButton,
        &peakBypassButton,
        &highcutBypassButton,
        &analyzerEnabledButton,
        
        &compThresholdSlider,
        &compRatioSlider,
        &compAttackSlider,
        &compReleaseSlider,
        &compBypassButton,
        
        &distortionAmountSlider,
        &distortionBypassButton,
        
        &delayTimeSlider,
        &delayFeedbackSlider,
        &delayMixSlider,
        &delayBypassButton,
        
        &reverbSizeSlider,
        &reverbDecaySlider,
        &reverbMixSlider,
        &reverbBypassButton,

    };
}

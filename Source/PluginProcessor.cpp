/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessor::SimpleEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SimpleEQAudioProcessor::~SimpleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    // Compressor is stereo
    spec.numChannels = getTotalNumOutputChannels();
    compressor.prepare(spec);
    
    // Reverb is stereo
    reverb.prepare(spec);

    // Optional: set initial delay size
    delayLine.setDelay(apvts.getRawParameterValue("Delay Time")->load());
    delayLine.prepare(spec);
    
    updateSettings();
    updateDelaySettings();
    
    leftChannelFifo.prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);
}

void SimpleEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (//layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    updateSettings();

    juce::dsp::AudioBlock<float> block(buffer);

    /**========================
     *   1. Process EQ
     *=========================*/
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);

    /**========================
     *   2. Compressor
     *=========================*/
    if (!apvts.getRawParameterValue("Comp Bypassed")->load())
    {
        juce::dsp::ProcessContextReplacing<float> context(block);
        compressor.process(context);
    }

    /**========================
     *   3. Distortion
     *=========================*/
    if (!apvts.getRawParameterValue("Distortion Bypassed")->load())
    {
        float amount = apvts.getRawParameterValue("Distortion Amount")->load();
        distortion.functionToUse = [amount](float x)
        {
            return std::tanh(amount * x);
        };

        auto numChannels = block.getNumChannels();
        auto numSamples = block.getNumSamples();

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            for (size_t i = 0; i < numSamples; ++i)
            {
                auto sample = block.getSample(ch, i);
                block.setSample(ch, i, distortion.functionToUse(sample));
            }
        }
    }



    /**========================
     *   4. Delay
     *=========================*/
    if (!apvts.getRawParameterValue("Delay Bypassed")->load() && false)
    {
        auto delayTimeMs = apvts.getRawParameterValue("Delay Time")->load();
        auto delayTimeSamples = static_cast<int>(getSampleRate() * delayTimeMs / 1000.0f);
        delayLine.setDelay(delayTimeSamples);

        auto feedback = apvts.getRawParameterValue("Delay Feedback")->load();
        auto mix = apvts.getRawParameterValue("Delay Mix")->load();

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                auto delayed = delayLine.popSample(0);
                auto input = channelData[i];
                delayLine.pushSample(0, input + delayed * feedback);
                channelData[i] = input * (1.0f - mix) + delayed * mix;
            }
        }
    }

    /**========================
     *   5. Reverb
     *=========================*/
    if (!apvts.getRawParameterValue("Reverb Bypassed")->load())
    {
        juce::dsp::Reverb::Parameters params;
        params.roomSize = apvts.getRawParameterValue("Reverb Size")->load();
        params.wetLevel = apvts.getRawParameterValue("Reverb Mix")->load();
        params.dryLevel = 1.0f - params.wetLevel;
        params.damping  = juce::jlimit(0.0f, 1.0f, apvts.getRawParameterValue("Reverb Decay")->load() / 10.0f);
        reverb.setParameters(params);

        juce::dsp::ProcessContextReplacing<float> context(block);
        reverb.process(context);
    }

    /**========================
     *   Final: FFT Visualization
     *=========================*/
    leftChannelFifo.update(buffer);
    rightChannelFifo.update(buffer);
}


//==============================================================================
bool SimpleEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEQAudioProcessor::createEditor()
{
    return new SimpleEQAudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void SimpleEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if( tree.isValid() )
    {
        apvts.replaceState(tree);
        updateSettings();
    }
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());
    
    settings.lowCutBypassed = apvts.getRawParameterValue("LowCut Bypassed")->load() > 0.5f;
    settings.peakBypassed = apvts.getRawParameterValue("Peak Bypassed")->load() > 0.5f;
    settings.highCutBypassed = apvts.getRawParameterValue("HighCut Bypassed")->load() > 0.5f;
    
    return settings;
}

Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                               chainSettings.peakFreq,
                                                               chainSettings.peakQuality,
                                                               juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
}

void SimpleEQAudioProcessor::updatePeakFilter(const ChainSettings &chainSettings)
{
    auto peakCoefficients = makePeakFilter(chainSettings, getSampleRate());
    
    leftChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
    rightChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
    
    updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
}

void updateCoefficients(Coefficients &old, const Coefficients &replacements)
{
    *old = *replacements;
}

void SimpleEQAudioProcessor::updateLowCutFilters(const ChainSettings &chainSettings)
{
    auto cutCoefficients = makeLowCutFilter(chainSettings, getSampleRate());
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
    
    leftChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    rightChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    
    updateCutFilter(rightLowCut, cutCoefficients, chainSettings.lowCutSlope);
    updateCutFilter(leftLowCut, cutCoefficients, chainSettings.lowCutSlope);
}

void SimpleEQAudioProcessor::updateHighCutFilters(const ChainSettings &chainSettings)
{
    auto highCutCoefficients = makeHighCutFilter(chainSettings, getSampleRate());
    
    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();
    
    leftChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    rightChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    
    updateCutFilter(leftHighCut, highCutCoefficients, chainSettings.highCutSlope);
    updateCutFilter(rightHighCut, highCutCoefficients, chainSettings.highCutSlope);
}

void SimpleEQAudioProcessor::updateSettings()
{
    auto chainSettings = getChainSettings(apvts);
    
    updateLowCutFilters(chainSettings);
    updatePeakFilter(chainSettings);
    updateHighCutFilters(chainSettings);
    updateCompressorSettings();
    updateDistortionSettings();
    updateDelaySettings();
    updateReverbSettings();
}

void SimpleEQAudioProcessor::updateCompressorSettings() {
    compressor.setThreshold(apvts.getRawParameterValue("Comp Threshold")->load());
    compressor.setRatio(apvts.getRawParameterValue("Comp Ratio")->load());
    compressor.setAttack(apvts.getRawParameterValue("Comp Attack")->load());
    compressor.setRelease(apvts.getRawParameterValue("Comp Release")->load());
}

void SimpleEQAudioProcessor::updateDistortionSettings() {
}

void SimpleEQAudioProcessor::updateDelaySettings() {
}

void SimpleEQAudioProcessor::updateReverbSettings() {
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    /* EQUALIZER */
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "LowCut Freq", 1 },
                                                           "LowCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           20.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "HighCut Freq", 1 },
                                                           "HighCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           20000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Peak Freq", 1 },
                                                           "Peak Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           750.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Peak Gain", 1 },
                                                           "Peak Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Peak Quality", 1 },
                                                           "Peak Quality",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
                                                           1.f));
    
    juce::StringArray stringArray;
    for( int i = 0; i < 4; ++i )
    {
        juce::String str;
        str << (12 + i*12);
        str << " db/Oct";
        stringArray.add(str);
    }
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "LowCut Slope", 1 }, "LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "HighCut Slope", 1 }, "HighCut Slope", stringArray, 0));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "LowCut Bypassed", 1 }, "LowCut Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "Peak Bypassed", 1 }, "Peak Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "HighCut Bypassed", 1 }, "HighCut Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "Analyzer Enabled", 1 }, "Analyzer Enabled", true));
    
    /* COMPRESSOR */
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Comp Threshold", 1 }, "Comp Threshold",
        juce::NormalisableRange<float>(-60.f, 0.f, 1.f), -24.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Comp Ratio", 1 }, "Comp Ratio",
        juce::NormalisableRange<float>(1.f, 20.f, 0.1f), 4.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Comp Attack", 1 }, "Comp Attack",
        juce::NormalisableRange<float>(1.f, 100.f, 0.1f), 20.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Comp Release", 1 }, "Comp Release",
        juce::NormalisableRange<float>(10.f, 500.f, 0.1f), 250.f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "Comp Bypassed", 1 }, "Comp Bypassed", false));
    
    /* DISTORTION */
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Distortion Amount", 1 }, "Distortion Amount",
        juce::NormalisableRange<float>(1.f, 10.f, 0.1f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "Distortion Bypassed", 1}, "Distortion Bypassed", false));

    /* DELAY */
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Delay Time", 1 }, "Delay Time",
        juce::NormalisableRange<float>(1.f, 750.f, 1.f), 500.f)); // in ms
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Delay Feedback", 1 }, "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Delay Mix", 1 }, "Delay Mix",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.3f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "Delay Bypassed", 1 }, "Delay Bypassed", false));
    
    /* REVERB */
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Reverb Size", 1 }, "Reverb Size",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Reverb Decay", 1 }, "Reverb Decay",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "Reverb Mix", 1 }, "Reverb Mix",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.3f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "Reverb Bypassed", 1 }, "Reverb Bypassed", false));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
}

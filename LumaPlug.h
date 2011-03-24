/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef LUMAPLUG_H
#define LUMAPLUG_H

#include "Luma.h"


//==============================================================================
/**
    A simple plugin filter that just applies a gain change to the audio
    passing through it.

*/
class LumaPlug  :	public AudioProcessor,
					public ChangeBroadcaster
{
public:
    //==============================================================================
    LumaPlug();
    ~LumaPlug();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

	void processBlock (AudioSampleBuffer& buffer,
                       MidiBuffer& midiMessages);

    //==============================================================================
    AudioProcessorEditor* createEditor();

    //==============================================================================
    const String getName() const;

    int getNumParameters();

    float getParameter (int index);
    void setParameter (int index, float newValue);

    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (const int channelIndex) const;
    const String getOutputChannelName (const int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;

    //==============================================================================
    int getNumPrograms()                                        { return 0; }
    int getCurrentProgram()                                     { return 0; }
    void setCurrentProgram (int index)                          { }
    const String getProgramName (int index)                     { return String::empty; }
    void changeProgramName (int index, const String& newName)   { }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);
	
	void setScriptText(const juce::String text) { scriptText_ = text; }
	const juce::String getScriptText() { return scriptText_; }
	
	void newTextLoadedFromFile(String scriptText)
	{
		scriptText_ = scriptText;
		sendChangeMessage(this);
	}
	
private:
	class LumaDocument : public FileBasedDocument
	{
	public:
		LumaDocument(LumaPlug* aPlug);
	protected:
		const String getDocumentTitle();
		const String loadDocument (const File& file);
		const String saveDocument (const File& file);
		const File getLastDocumentOpened();
		void setLastDocumentOpened (const File& file);
	private:
		LumaPlug* plug;
	};

public:
	LumaDocument* getDocument() { return lumaDoc_; }
	
	juce_UseDebuggingNewOperator

private:	
	Luma* luma_;
	LumaDocument* lumaDoc_;
	juce::String scriptText_;
};


#endif

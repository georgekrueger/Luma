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

#include "juce_amalgamated.h"
#include "LumaPlug.h"
#include "LumaPlugEditor.h"
#include <math.h>

using std::vector;
using boost::shared_ptr;

//==============================================================================
/**
    This function must be implemented to create a new instance of your
    plugin object.
*/
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LumaPlug();
}

//==============================================================================
LumaPlug::LumaPlug()
{

	luma_ = new Luma;
	lumaDoc_ = new LumaDocument(this);
}

LumaPlug::~LumaPlug()
{
	deleteAndZero(luma_);
	deleteAndZero(lumaDoc_);
}

//==============================================================================
const String LumaPlug::getName() const
{
    return "Luma Plugin";
}

int LumaPlug::getNumParameters()
{
    return 0;
}

float LumaPlug::getParameter (int index)
{
	return 0.0;
}

void LumaPlug::setParameter (int index, float newValue)
{
    /*if (index == 0)
    {
        if (gain != newValue)
        {
            gain = newValue;

            // if this is changing the gain, broadcast a change message which
            // our editor will pick up.
            sendChangeMessage (this);
        }
    }*/
}

const String LumaPlug::getParameterName (int index)
{
    //if (index == 0)
    //    return T("gain");

    return String::empty;
}

const String LumaPlug::getParameterText (int index)
{
    //if (index == 0)
    //    return String (gain, 2);

    return String::empty;
}

const String LumaPlug::getInputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

const String LumaPlug::getOutputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

bool LumaPlug::isInputChannelStereoPair (int index) const
{
    return false;
}

bool LumaPlug::isOutputChannelStereoPair (int index) const
{
    return false;
}

bool LumaPlug::acceptsMidi() const
{
    return false;
}

bool LumaPlug::producesMidi() const
{
    return true;
}

//==============================================================================
void LumaPlug::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void LumaPlug::releaseResources()
{
}

void LumaPlug::processBlock (AudioSampleBuffer& buffer,
                                   MidiBuffer& midiMessages)
{
	// we don't want any midi input events
	midiMessages.clear();
	
	bool isRunning = false;
	AudioPlayHead* playHead = getPlayHead();
	if (playHead)
	{
		//printf("playhead: 0x%x\n", playHead);
		AudioPlayHead::CurrentPositionInfo posInfo;
		playHead->getCurrentPosition(posInfo);
		isRunning = posInfo.isPlaying || posInfo.isRecording;
		luma_->SetBPM(posInfo.bpm);
	}
	
	if (isRunning && !luma_->IsRunning())
	{
		std::string error;
		bool setScriptOK = luma_->SetScript(scriptText_.toUTF8(), error);
		if (!setScriptOK && getActiveEditor())
		{
			((LumaPlugEditor*)getActiveEditor())->Log(error.c_str());
		}
//		else if (getActiveEditor())
//		{
//			((LumaPlugEditor*)getActiveEditor())->Log("Play");
//		}
		luma_->Start();
	}
	else if (!isRunning && luma_->IsRunning())
	{
		luma_->Stop();
//		if (getActiveEditor())
//		{
//			((LumaPlugEditor*)getActiveEditor())->Log("Stop");
//		}
	}

    if (luma_->IsRunning())
	{
		double sampleRate = getSampleRate();
		int numSamples = buffer.getNumSamples();
		float elapsed = (float(numSamples) / float(sampleRate)) * 1000.0;
		//printf("Elapsed: %f\n", elapsed);
		vector<shared_ptr<LumaEvent> > events;
		vector<float> offsets;
		luma_->Update(elapsed, events, offsets);
		
		if (events.size() > 0)
		{
			for (unsigned int i = 0; i < events.size(); i++)
			{
				//printf("New Event.  Offset = %d, OffsetSamples = %d, Type = %d\n\n", 
				//	offsets[i], midiEvent->deltaFrames, events[i]->GetType());
				
				int eventOffset = lroundf( ( float(offsets[i]) / 1000.0 ) * sampleRate );
				
				if (events[i]->GetType() == kLumaEvent_NoteOn)
				{
					NoteOnEvent* noteOn = (NoteOnEvent*)events[i].get();
					MidiMessage msg = MidiMessage::noteOn(1, noteOn->GetPitch(), (juce::uint8)noteOn->GetVelocity());
					midiMessages.addEvent(msg, eventOffset);
				}
				else if (events[i]->GetType() == kLumaEvent_NoteOff)
				{
					NoteOffEvent* noteOff = (NoteOffEvent*)events[i].get();
					MidiMessage msg = MidiMessage::noteOff(1, noteOff->GetPitch());
					midiMessages.addEvent(msg, eventOffset);
				}
				else
				{
					fprintf(stderr, "LumaVST: Unknown event type: %d\n", events[i]->GetType());
				}
			}
			
			// clear the used luma events
			events.clear();
		}
	}
	
	/*
	Simple test of sending midi from the plugin
	
	static int count = 0;
	count += buffer.getNumSamples();
	if (count >= 20000)
	{
		//MidiMessage msg = MidiMessage::noteOff(0, 60);
		//midiMessages.addEvent(msg, 0);
	
		MidiMessage msg = MidiMessage::noteOn(1, 60, (juce::uint8)100);
		midiMessages.addEvent(msg, 0);
		
		count = 0;
	}
	*/
}

//==============================================================================
AudioProcessorEditor* LumaPlug::createEditor()
{
    return new LumaPlugEditor (this);
}

//==============================================================================
void LumaPlug::getStateInformation (MemoryBlock& destData)
{
	/*
    // you can store your parameters as binary data if you want to or if you've got
    // a load of binary to put in there, but if you're not doing anything too heavy,
    // XML is a much cleaner way of doing it - here's an example of how to store your
    // params as XML..

    // create an outer XML element..
    XmlElement xmlState (T("MYPLUGINSETTINGS"));

    // add some attributes to it..
    xmlState.setAttribute (T("pluginVersion"), 1);
    //xmlState.setAttribute (T("gainLevel"), gain);
    xmlState.setAttribute (T("uiWidth"), lastUIWidth);
    xmlState.setAttribute (T("uiHeight"), lastUIHeight);

    // you could also add as many child elements as you need to here..


    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xmlState, destData);
	*/
	
	const char* data = scriptText_.toUTF8();
	if (strlen(data) > 0)
	{
		destData.append(data, strlen(data));
	}
}

void LumaPlug::setStateInformation (const void* data, int sizeInBytes)
{
	/*
    // use this helper function to get the XML from this binary blob..
    XmlElement* const xmlState = getXmlFromBinary (data, sizeInBytes);

    if (xmlState != 0)
    {
        // check that it's the right type of xml..
        if (xmlState->hasTagName (T("MYPLUGINSETTINGS")))
        {
            // ok, now pull out our parameters..
            //gain = (float) xmlState->getDoubleAttribute (T("gainLevel"), gain);

            lastUIWidth = xmlState->getIntAttribute (T("uiWidth"), lastUIWidth);
            lastUIHeight = xmlState->getIntAttribute (T("uiHeight"), lastUIHeight);

            sendChangeMessage (this);
        }

        delete xmlState;
    }
	*/
	
	if (sizeInBytes > 0)
	{
		scriptText_ = juce::String::fromUTF8((const juce::uint8*)data, sizeInBytes);
	}
}

//==============================================================================

LumaPlug::LumaDocument::LumaDocument(LumaPlug* aPlug)
	: FileBasedDocument (".lua",
                         "*.lua",
                         "Load a lua file",
                         "Save a lua file")
{
	plug = aPlug;
}

const String LumaPlug::LumaDocument::getDocumentTitle()
{
    if (! getFile().exists())
        return "Unnamed";

    return getFile().getFileNameWithoutExtension();
}

const String LumaPlug::LumaDocument::loadDocument (const File& file)
{
    FileInputStream* stream = file.createInputStream();
	juce::int64 size = stream->getTotalLength();
	if (size <= 0)
	{
		return String("Input file size is zero!");
	}
	MemoryBlock* buffer = new MemoryBlock(size+1, true);
	int bytesRead = stream->read(buffer->getData(), size);
	if (bytesRead != size)
	{
		return String("Bytes read from file differs from file size!");
	}
	
	String text = String(static_cast<char*>(buffer->getData()));
	plug->newTextLoadedFromFile(text);
	
	delete buffer;

    return String::empty;
}

const String LumaPlug::LumaDocument::saveDocument (const File& file)
{
	if (file.exists())
	{
		bool deleteSuccess = file.deleteFile();
		if (!deleteSuccess)
		{
			return String("Delete of existing file failed!");
		}
	}

	FileOutputStream* stream = file.createOutputStream();
	if (!stream)
	{
		return String("Failed to obtain output stream to save file!");
	}
	String scriptText = plug->getScriptText();
	if (scriptText.length() > 0)
	{
		bool writeSuccess = stream->write(scriptText.toUTF8(), scriptText.length());
		if (!writeSuccess)
		{
			return String("Write to output stream failed!");
		}
		stream->flush();
	}

    return String::empty;
}

const File LumaPlug::LumaDocument::getLastDocumentOpened()
{
    return File::nonexistent;
}

void LumaPlug::LumaDocument::setLastDocumentOpened (const File& file)
{
}

//==============================================================================



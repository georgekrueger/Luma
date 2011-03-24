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
#include "LumaPlugEditor.h"

//==============================================================================
LumaPlugEditor::LumaPlugEditor (LumaPlug* const ownerFilter)
    : AudioProcessorEditor (ownerFilter)
{
	addAndMakeVisible( editor_ = new TextEditor );
	editor_->setMultiLine(true, true);
	editor_->setReturnKeyStartsNewLine(true);
	editor_->setTabKeyUsedAsCharacter(true);
	editor_->setScrollbarsShown(true);
	editor_->addListener(this);
	editor_->setText(ownerFilter->getScriptText());
	
	addAndMakeVisible( log_ = new TextEditor );
	log_->setMultiLine(true, true);
	log_->setReturnKeyStartsNewLine(true);
	log_->setTabKeyUsedAsCharacter(true);
	log_->setScrollbarsShown(true);

    // add the triangular resizer component for the bottom-right of the UI
    addAndMakeVisible (resizer = new ResizableCornerComponent (this, &resizeLimits));
    resizeLimits.setSizeLimits (150, 150, 800, 800);

    // set our component's initial size to be the last one that was stored in the filter's settings
    setSize (600, 500);

    // register ourselves with the filter - it will use its ChangeBroadcaster base
    // class to tell us when something has changed, and this will call our changeListenerCallback()
    // method.
    ownerFilter->addChangeListener (this);
}

LumaPlugEditor::~LumaPlugEditor()
{
    //getFilter()->removeChangeListener (this);

    deleteAllChildren();
}

void LumaPlugEditor::changeListenerCallback (void *objectThatHasChanged)
{
	/*LumaPlug* plug = dynamic_cast<LumaPlug*>(objectThatHasChanged);
	if (plug)
	{
		editor_->setText(plug->getScriptText());
	}*/
	editor_->setText(getFilter()->getScriptText());
}

void LumaPlugEditor::textEditorTextChanged(TextEditor& editor)
{
	getFilter()->setScriptText(editor.getText());
}

//==============================================================================
void LumaPlugEditor::paint (Graphics& g)
{
    // just clear the window
    g.fillAll (Colour::greyLevel (0.9f));
}

void LumaPlugEditor::resized()
{
	editor_->setBounds(0, 0, getWidth(), getHeight() - 100);
	
	log_->setBounds(0, getHeight() - 100, getWidth(), 100);

    resizer->setBounds (getWidth() - 16, getHeight() - 16, 16, 16);

    // if we've been resized, tell the filter so that it can store the new size
    // in its settings
//    getFilter()->lastUIWidth = getWidth();
//    getFilter()->lastUIHeight = getHeight();
}

//==============================================================================
/*
void LumaPlugEditor::changeListenerCallback (void* source)
{
    // this is the filter telling us that it's changed, so we'll update our
    // display of the time, midi message, etc.
    updateParametersFromFilter();
}
*/

/*
void LumaPlugEditor::sliderValueChanged (Slider*)
{
    getFilter()->setParameterNotifyingHost (0, (float) gainSlider->getValue());
}
*/

//==============================================================================
/*
void LumaPlugEditor::updateParametersFromFilter()
{
    LumaPlug* const filter = getFilter();

    // we use this lock to make sure the processBlock() method isn't writing to the
    // lastMidiMessage variable while we're trying to read it, but be extra-careful to
    // only hold the lock for a minimum amount of time..
    filter->getCallbackLock().enter();

    // take a local copy of the info we need while we've got the lock..
    const AudioPlayHead::CurrentPositionInfo positionInfo (filter->lastPosInfo);
    const float newGain = filter->getParameter (0);

    // ..release the lock ASAP
    filter->getCallbackLock().exit();


    // ..and after releasing the lock, we're free to do the time-consuming UI stuff..
    String infoText;
    infoText << String (positionInfo.bpm, 2) << T(" bpm, ")
             << positionInfo.timeSigNumerator << T("/") << positionInfo.timeSigDenominator
             << T("  -  ") << timeToTimecodeString (positionInfo.timeInSeconds)
             << T("  -  ") << ppqToBarsBeatsString (positionInfo.ppqPosition,
                                                    positionInfo.ppqPositionOfLastBarStart,
                                                    positionInfo.timeSigNumerator,
                                                    positionInfo.timeSigDenominator);

    if (positionInfo.isPlaying)
        infoText << T("  (playing)");

    infoLabel->setText (infoText, false);

    // Update our slider.

    //   (note that it's important here to tell the slider not to send a change
    //   message, because that would cause it to call the filter with a parameter
    //   change message again, and the values would drift out.
	
    gainSlider->setValue (newGain, false);

    setSize (filter->lastUIWidth,
             filter->lastUIHeight);
}
*/

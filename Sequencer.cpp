
#include "Sequencer.h"
#include <stdio.h>

using std::vector;
using std::map;
using std::string;
using std::map;
using boost::shared_ptr;


Sequencer::Sequencer() : running_(false)
{
}

Sequencer::~Sequencer()
{
}

void Sequencer::GetActiveNotes(vector<NoteOnEvent>& notes)
{
	// stop all active notes
	map<short, NoteOnEvent>::iterator i;
	for (i = activeNotes_.begin(); i != activeNotes_.end(); i++)
	{
		notes.push_back(i->second);
	}
}

void Sequencer::AddPhrase(const Phrase& phrase)
{
	sequences_.push_back(phrase);
}

void Sequencer::Update(float elapsedTime, vector<shared_ptr<LumaEvent> >& events, vector<float>& offsets)
{
	// check active notes
	map<short, NoteOnEvent>::iterator i;
	for (i = activeNotes_.begin(); i != activeNotes_.end(); )
	{
		NoteOnEvent* e = &i->second;
		map<short, NoteOnEvent>::iterator next = i;
		next++;
		if (e->GetLength() - elapsedTime < 0)
		{
			// signal note off event
			shared_ptr<NoteOffEvent> noteOff = shared_ptr<NoteOffEvent>(new NoteOffEvent(e->GetPitch(), e->GetChannel()));
			events.push_back(noteOff);
			offsets.push_back(e->GetLength());
			// remove from active notes
			activeNotes_.erase(i);
		}
		else
		{
			e->SetLength(e->GetLength() - elapsedTime);
		}
		i = next;
	}	
	
	// update event sequences
	vector<Phrase>::iterator seq;
	for (seq = sequences_.begin(); seq != sequences_.end(); )
	{
		seq->Update(elapsedTime, events, offsets);
		
		if (seq->IsDone()) {
			seq = sequences_.erase(seq);
			printf("Sequence ended\n");
		}
		else {
			seq++;
		}
	}

	// remember any note on events, so we can send a note off
	vector<shared_ptr<LumaEvent> >::iterator j;
	for (j = events.begin(); j != events.end(); j++)
	{
		if ((*j)->GetType() == kLumaEvent_NoteOn) {
			NoteOnEvent* noteOn = (NoteOnEvent*) j->get();
			activeNotes_[noteOn->GetPitch()] = *noteOn;
		}
	}
}

void Sequencer::Start()
{
	running_ = true;
}

void Sequencer::Stop()
{
	running_ = false;
	sequences_.clear();
	activeNotes_.clear();
}

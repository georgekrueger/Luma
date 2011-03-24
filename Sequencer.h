#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "LumaEvent.h"
#include "Phrase.h"
#include <vector>
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>


class Sequencer
{
public:
	Sequencer();
	~Sequencer();
	
	void AddPhrase(const Phrase& phrase);

	void GetActiveNotes(std::vector<NoteOnEvent>& notes);
	void ClearActiveNotes() { activeNotes_.clear(); }

	void Update(float elapsedTime, std::vector<boost::shared_ptr<LumaEvent> >& events, std::vector<float>& offsets);
	
	void Start();
	void Stop();
	
	bool IsRunning() { return running_; }

private:
	// notes that are on
	std::map<short, NoteOnEvent> activeNotes_;
	// active note sequences
	std::vector<Phrase> sequences_;
	// is sequencer running
	bool running_;
};

#endif
/*
 *  Luma.h
 *  vst 2.4 examples
 *
 *  Created by George Krueger on 7/29/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
 
 #ifndef LUMA_H
 #define LUMA_H

#include "LumaEvent.h"
#include "Sequencer.h"
#include "luainc.h"
#include <vector>
#include <boost/shared_ptr.hpp>
#include <string.h>

class Luma
{
public:
	Luma();
	~Luma();
	
	bool SetScript(const char* text, std::string& error);
	
	void SetBPM(float bpm);
	
	void Start();
	void Stop();
	bool IsRunning() { return sequencer_->IsRunning(); }
	
	void Update(float elapsed, std::vector<boost::shared_ptr<LumaEvent> >& events, std::vector<float>& offsets);
	
	void GetActiveNotes(std::vector<NoteOnEvent>& notes) { sequencer_->GetActiveNotes(notes); }
	void ClearActiveNotes() { sequencer_->ClearActiveNotes(); }
	
	Sequencer* GetSequencer() { return sequencer_; }

	float GetTimerOffset() { return timerOffset_; }

public:
	void AddListener(const char* name, float time, unsigned long reps);
	void RemoveListener(const char* name);

private:
	typedef struct Listener
	{
		Listener() { time = 0; elapsed = 0; reps = 0; done = false; }
		Listener(float aTime, float aElapsed, unsigned long aReps) : time(aTime), elapsed(aElapsed), reps(aReps), done(false) {}

		float			time;
		float			elapsed;
		unsigned long	reps;
		bool			done;
	};

private:
	// sequencer
	Sequencer* sequencer_;
	// active listeners
	std::map<const char*, Listener> listeners_;
	// keep track of lua state for now
	lua_State* luaState_;
	
	float updateTime_;
	float timerOffset_;
	
	float bpm_;
};

#endif

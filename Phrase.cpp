
#include "Phrase.h"
#include <algorithm>
#include <map>

using std::vector;
using std::map;
using boost::shared_ptr;

Phrase::Phrase(float length, int loopCount)
	:	length_(length), 
		currTime_(0), currIter_(1), maxIter_(loopCount),
		currEvent_(0)
{
}

Phrase::~Phrase()
{
}

void Phrase::AddEvent(float position, shared_ptr<LumaEvent> event)
{
	//printf("Add NoteOn Event, position = %d, pitch = %d, velocity = %d, length = %d, channel = %d\n", 
	//	position, pitch, velocity, length, channel);

	// add event
	events_.push_back(PhraseEvent(position, event));

	// sort events
	sort(events_.begin(), events_.end());

	// recalibrate current position
	currEvent_ = 0;
	while (currEvent_ < events_.size() && events_[currEvent_].position_ < currTime_) 
		currEvent_++;
}

void Phrase::Update(float elapsed, vector<shared_ptr<LumaEvent> >& events, vector<float>& offsets)
{
	if (events_.size() == 0) return;
	if (IsDone()) return;

	currTime_ += elapsed;

	// check for events that are due
	while (currEvent_ < events_.size() && events_[currEvent_].position_ <= currTime_ && currIter_ <= maxIter_) {
		shared_ptr<LumaEvent> e = events_[currEvent_].event_;
		float offset = elapsed - (currTime_ - events_[currEvent_].position_);
		offsets.push_back(offset);
		events.push_back(e);
		
		currEvent_++;
		
		// if update time is past the end of phrase and all events for the current cycle have 
		// been handled then wrap around to the beginning of the phrase and check for any events 
		// that should happen in remaining time
		if (currEvent_ >= events_.size() && currTime_ >= length_)
		{
			currIter_++;
			currEvent_ = 0;
			currTime_ = currTime_ - length_;
		}
	}
	
	if (currEvent_ >= events.size() && currTime_ >= length_)
	{
		currIter_++;
		currEvent_ = 0;
		currTime_ = currTime_ - length_;
	}
	
}

bool Phrase::IsDone() 
{ 
	return (currIter_ > maxIter_);
}

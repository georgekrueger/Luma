#ifndef PHRASE_H
#define PHRASE_H

#include <vector>
#include "LumaEvent.h"
#include <boost/shared_ptr.hpp>

class Phrase
{
public:
	Phrase(float length, int loopCount);
	~Phrase();

	void AddEvent(float position, boost::shared_ptr<LumaEvent> event);

	// Update the phrase with the given millisecond value.
	// Any due events are put into the notes vector
	void Update(float elapsed, std::vector<boost::shared_ptr<LumaEvent> >& events, std::vector<float>& offsets);

	bool IsDone();

private:
	struct PhraseEvent
	{
		PhraseEvent(float position, boost::shared_ptr<LumaEvent> event) : position_(position), event_(event) {}
		
		float							position_;
		boost::shared_ptr<LumaEvent>	event_;
		
		friend bool operator < (const PhraseEvent& left, const PhraseEvent& right)
		{
			return (left.position_ < right.position_);
		}
	};


	std::vector<PhraseEvent>			events_;
	int									currEvent_;
	float								length_;	// length in ms
	float								currTime_;	// current location in ms
	int									currIter_;	// current iteration
	int									maxIter_;	// max iterations to do
};

#endif
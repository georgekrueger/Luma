/*
 *  Event.h
 *
 *  Created by George Krueger on 7/26/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LUMA_EVENT_H
#define LUMA_EVENT_H

enum LumaEventType
{
	kLumaEvent_None = 0,
	kLumaEvent_NoteOn,
	kLumaEvent_NoteOff
};

class LumaEvent
{
public:
	LumaEvent() : type_(kLumaEvent_None) {}
	LumaEvent(LumaEventType type)
		: type_(type) {}
	~LumaEvent() {}
	
	LumaEventType GetType() const { return type_; }
	
private:
	LumaEventType type_;
};

class NoteOnEvent : public LumaEvent
{
public:
	NoteOnEvent() {}
	NoteOnEvent(short pitch, short velocity, float length, short channel)
		: LumaEvent(kLumaEvent_NoteOn), pitch_(pitch), velocity_(velocity), length_(length), channel_(channel) {}
	~NoteOnEvent() {};
	
	short			GetPitch()		{ return pitch_; }
	short			GetVelocity()	{ return velocity_; }
	float			GetLength()		{ return length_; }
	short			GetChannel()	{ return channel_; }
	
	void			SetPitch(short val)				{ pitch_ = val; }
	void			SetVelocity(short val)			{ velocity_ = val; }
	void			SetLength(float val)			{ length_ = val; }
	void			SetChannel(short val)			{ channel_ = val; }
	
private:
	short pitch_;
	short velocity_;
	float length_;
	short channel_;
};

class NoteOffEvent : public LumaEvent
{
public:
	NoteOffEvent() {}
	NoteOffEvent(short pitch, short channel)
		: LumaEvent(kLumaEvent_NoteOff), pitch_(pitch), channel_(channel) {}
	~NoteOffEvent() {};
	
	short			GetPitch()		{ return pitch_; }
	short			GetChannel()	{ return channel_; }
	
private:
	short pitch_;
	short channel_;
};

#endif


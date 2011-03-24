/*
 *  LumaFuncs.h
 *  vst 2.4 examples
 *
 *  Created by George Krueger on 7/29/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
 
#ifndef LUMA_FUNCS_H
#define LUMA_FUNCS_H
 
#include "Luma.h"
#include "MidiPitchConsts.h"
#include "Scale.h"
#include <boost/shared_ptr.hpp>

using boost::shared_ptr;

void luma_error(lua_State* luaState, char* errorMsg)
{
	lua_pushstring(luaState, errorMsg);
	lua_error(luaState);
}

int luma_note (lua_State *luaState)
{
	if (!lua_isnumber(luaState, 1) || !lua_isnumber(luaState, 2) || !lua_isnumber(luaState, 3)) {
		luma_error(luaState, "incorrect arguments to 'luma_note'");
	}

	int pitch = lua_tonumber(luaState, 1);
	//printf("pitch %d\n", pitch);

	int velocity = lua_tonumber(luaState, 2);
	//printf("velocity %d\n", velocity);

	int length = lua_tonumber(luaState, 3);
	//printf("length %d\n", length);
	
	// optional argument is offset.  Defaults to zero.
	int offset = 0;
	if (lua_isnumber(luaState, 4)) {
		offset = lua_tonumber(luaState, 4);
	}
	
	/*
	// optional argument is channel.  Defaults to first channel.
	int channel = 0;
	if (lua_isnumber(luaState, 4)) {
		int tmp = lua_tonumber(luaState, 4);
		if (tmp >= 1 && tmp <= 16) {
			channel = tmp - 1;
		}
	}
	*/
	
	
	// get sequencer
	lua_pushstring(luaState, "LUMA_PTR");
	lua_gettable(luaState, LUA_REGISTRYINDEX);
	int lumaAddr = lua_tonumber(luaState, -1);
	Luma* luma = (Luma*)lumaAddr;
	Sequencer* sequencer = luma->GetSequencer();
	lua_pop(luaState, 1);
	
	//printf("Note ON event, pitch %d vel %d len %d chan %d\n", pitch, velocity, length, channel);
	shared_ptr<LumaEvent> note = shared_ptr<LumaEvent>(new NoteOnEvent(pitch, velocity, length, 0));
	Phrase phrase(0, 1);
	phrase.AddEvent(luma->GetTimerOffset() + offset, note);
	sequencer->AddPhrase(phrase);

	return 0;
}

int luma_phrase (lua_State *luaState)
{
	if (!lua_istable(luaState, 1) || !lua_istable(luaState, 2) || !lua_istable(luaState, 3) 
		|| !lua_istable(luaState, 4) || !lua_isnumber(luaState, 5) || !lua_isnumber(luaState, 6)) 
	{
		luma_error(luaState, "incorrect arguments to 'luma_phrase'");
		return 0;
	}

	int length = lua_tonumber(luaState, 5);
	int loopCount = lua_tonumber(luaState, 6);
	
	// optional argument is channel.  Defaults to first channel.
	int channel = 0;
	if (lua_isnumber(luaState, 7)) {
		int tmp = lua_tonumber(luaState, 7);
		if (tmp >= 1 && tmp <= 16) {
			channel = tmp - 1;
		}
	}
	
	// get luma
	lua_pushstring(luaState, "LUMA_PTR");
	lua_gettable(luaState, LUA_REGISTRYINDEX);
	int lumaAddr = (int)lua_tonumber(luaState, -1);
	Luma* luma = (Luma*)lumaAddr;
	Sequencer* sequencer = luma->GetSequencer();
	lua_pop(luaState, 1);

	Phrase phrase(length, loopCount);

	static int maxEvents = 500;

	// check sizes of tables
	int numPositions = luaL_getn(luaState, 1);  // get size of table
	int numPitches = luaL_getn(luaState, 2);
	int numVelocities = luaL_getn(luaState, 3);
	int numLengths = luaL_getn(luaState, 4);
	if (numPositions == 0 || numPitches == 0 || numVelocities == 0 || numLengths == 0 ||
		numPositions > maxEvents || numPitches > maxEvents || numVelocities > maxEvents || numLengths > maxEvents)
	{
		luma_error(luaState, "each of the tables (position, pitch, velocity, length) must have atleast one item");
		return 0;
	}
	
	// allocate tables
	float positions[numPositions];
	int pitches[numPitches];
	int velocities[numVelocities];
	float lengths[numLengths];

	// fill positions table
	for (int i=1; i<=numPositions; i++)
	{
		lua_rawgeti(luaState, 1, i);
		if (!lua_isnumber(luaState, -1))
		{
			luma_error(luaState, "position is not a number");
			return 0;
		}
		positions[i-1] = lua_tonumber(luaState, -1);
		lua_pop(luaState, 1);
	}
	
	// fill pitches table
	for (int i=1; i<=numPitches; i++)
	{
		lua_rawgeti(luaState, 2, i);
		if (!lua_isnumber(luaState, -1))
		{
			luma_error(luaState, "pitch is not a number");
			return 0;
		}
		pitches[i-1] = lua_tonumber(luaState, -1);
		lua_pop(luaState, 1);
	}
	
	// fill velocities table
	for (int i=1; i<=numVelocities; i++)
	{
		lua_rawgeti(luaState, 3, i);
		if (!lua_isnumber(luaState, -1))
		{
			luma_error(luaState, "velocity is not a number");
			return 0;
		}
		velocities[i-1] = lua_tonumber(luaState, -1);
		lua_pop(luaState, 1);
	}
	
	// fill lengths table
	for (int i=1; i<=numLengths; i++)
	{
		lua_rawgeti(luaState, 4, i);
		if (!lua_isnumber(luaState, -1))
		{
			luma_error(luaState, "length is not a number");
			return 0;
		}
		lengths[i-1] = lua_tonumber(luaState, -1);
		lua_pop(luaState, 1);
	}

	// now we add the events to the phrase
	int currPos = 0;
	int currPitch = 0;
	int currVel = 0;
	int currLen = 0;
	for (int i=0; i<numPositions; i++)
	{
		shared_ptr<LumaEvent> note = shared_ptr<LumaEvent>(
			new NoteOnEvent(pitches[currPitch], velocities[currVel], lengths[currLen], channel));
		phrase.AddEvent(positions[currPos] + luma->GetTimerOffset(), note);
		
		++currPos;
		if (++currPitch >= numPitches) currPitch = 0;
		if (++currVel >= numVelocities) currVel = 0;
		if (++currLen >= numLengths) currLen = 0;
	}
	
	sequencer->AddPhrase(phrase);
	
	return 0;

	/*
	// This is the old code for parsing nested event structure.  Keeping it around for a while just in case.
	
	//printf("Parsing phrase: length = %d, loopCount = %d, # events = %d\n", length, loopCount, n);
    
	for (int i=1; i<=n; i++) {
		// put t[i] on top of stack (each t[i] is a table with a time offset and note object)
		lua_rawgeti(luaState, 1, i);
		if (!lua_istable(luaState, -1)) {
			luma_error(luaState, "invalid note object in sequence");
		}
		
		// Get event time
		// push key to top of stack
		lua_pushinteger(luaState, 1);
		// replaces key with table value on top of stack
		lua_gettable(luaState, -2);
		// make sure pitch is a number
		if (!lua_isnumber(luaState, -1)) {
			luma_error(luaState, "invalid time value");
		}
		int time = (int)lua_tonumber(luaState, -1);
		// done with value, pop it off the stack
		lua_pop(luaState, 1);

		// push note table onto stack
		// push key to top of stack
		lua_pushinteger(luaState, 2);
		// replaces key with table value on top of stack
		lua_gettable(luaState, -2);
		// make sure pitch is a number
		if (!lua_istable(luaState, -1)) {
			luma_error(luaState, "invalid note object");
		}

		// get note pitch
		// push key to top of stack
		//lua_pushstring(luaState, "pitch");
		lua_pushnumber(luaState, 1);
		// replaces key with table value on top of stack
		lua_gettable(luaState, -2);
		// make sure pitch is a number
		if (!lua_isnumber(luaState, -1)) {
			luma_error(luaState, "invalid value for pitch");
		}
		int pitch = (int)lua_tonumber(luaState, -1);
		// done with value, pop it off the stack
		lua_pop(luaState, 1);

		// get note velocity
		// push key to top of stack
		//lua_pushstring(luaState, "velocity");
		lua_pushnumber(luaState, 2);
		// replaces key with table value on top of stack
		lua_gettable(luaState, -2);
		// make sure pitch is a number
		if (!lua_isnumber(luaState, -1)) {
			luma_error(luaState, "invalid value for velocity");
		}
		int velocity = (int)lua_tonumber(luaState, -1);
		// done with value, pop it off the stack
		lua_pop(luaState, 1);

		// get note length
		// push key to top of stack
		//lua_pushstring(luaState, "length");
		lua_pushnumber(luaState, 3);
		// replaces key with table value on top of stack
		lua_gettable(luaState, -2);
		// make sure pitch is a number
		if (!lua_isnumber(luaState, -1)) {
			luma_error(luaState, "invalid value for length");
		}
		int length = (int)lua_tonumber(luaState, -1);
		// done with value, pop it off the stack
		lua_pop(luaState, 1);

		// pop note object from top of stack
		lua_pop(luaState, 1);

		// pop event object from top of stack
		lua_pop(luaState, 1);

		shared_ptr<LumaEvent> note = shared_ptr<LumaEvent>(new NoteOnEvent(pitch, velocity, length, channel));
		phrase.AddEvent(time + luma->GetTimerOffset(), note);

		//printf("Phrase item (%d ms): { pitch = %d, velocity = %d, length = %d }\n", time, pitch, velocity, length);
	}
	*/
}

int luma_starttimer (lua_State *luaState)
{
	// args:
	//	callback function name
	//	interval
	if (!lua_isstring(luaState, 1) || !lua_isnumber(luaState, 2)) {
		luma_error(luaState, "incorrect arguments to function 'luma_starttimer'");
	}
	
	const char* func = lua_tostring(luaState, 1);
	double time = lua_tonumber(luaState, 2);
	int reps = 0;
	if (lua_isnumber(luaState, 3)) {
		reps = lua_tonumber(luaState, 3);
	}
	
	// get sequencer
	lua_pushstring(luaState, "LUMA_PTR");
	lua_gettable(luaState, LUA_REGISTRYINDEX);
	int lumaAddr = (int)lua_tonumber(luaState, -1);
	Luma* luma = (Luma*)lumaAddr;
	lua_pop(luaState, 1);
	
	luma->AddListener(func, time, reps);
	
	//printf("luma_starttimer func = '%s', time = '%d', reps = '%d'\n", func, time, reps);
	
	return 0;
}

int luma_stoptimer (lua_State *luaState)
{
	// args:
	//	callback function name
	if (!lua_isstring(luaState, 1)) {
		luma_error(luaState, "incorrect arguments to function 'luma_stoptimer'");
	}
	
	const char* func = lua_tostring(luaState, 1);
	
	// get sequencer
	lua_pushstring(luaState, "LUMA_PTR");
	lua_gettable(luaState, LUA_REGISTRYINDEX);
	int lumaAddr = (int)lua_tonumber(luaState, -1);
	Luma* luma = (Luma*)lumaAddr;
	lua_pop(luaState, 1);
	
	luma->RemoveListener(func);
	
	//printf("luma_starttimer func = '%s'\n", func);
	
	return 0;
	
}

int luma_scale (lua_State *luaState)
{
	int n = lua_gettop(luaState);
	if (n < 3 || n > 4 || !lua_isstring(luaState, 1) || !lua_isstring(luaState, 2) || !lua_isstring(luaState, 3)) {
		luma_error(luaState, "incorrect arguments to function 'luma_scale'");
	}
	
	const char* scaleName = lua_tostring(luaState, 1);
	const char* rootPitchName = lua_tostring(luaState, 2);
	const char* endPitchName = lua_tostring(luaState, 3);
	int validDegrees[25];
	int numValidDegrees = 0;
	if (n == 4)
	{
		if (lua_istable(luaState, 4))
		{
			int tableSize = luaL_getn(luaState, 4);
			for (int i=1; i<=tableSize; i++)
			{
				lua_pushnumber(luaState, i);
				lua_gettable(luaState, 4);
				if (lua_isnumber(luaState, -1))
				{
					validDegrees[numValidDegrees++] = lua_tonumber(luaState, -1);
					lua_pop(luaState, 1);
				}
			}
		}
	}
	
	lua_pop(luaState, n);
	
	unsigned char rootPitch = GetMidiPitch(rootPitchName);
	unsigned char endPitch = GetMidiPitch(endPitchName);
	Scale scale;
	if (!GetScale(scaleName, scale)) {
		printf("invalid scale '%s'\n", scaleName);
		luma_error(luaState, "invalid scale");
	}
	
	ScalePitchSeq pitchSeq;
	GetScalePitchSequence(rootPitch, endPitch, scale.type, validDegrees, numValidDegrees, pitchSeq);
	
	printf("Scale: %s, starting at %s, ending at %s, has %d pitches\n", scaleName, rootPitchName, endPitchName, pitchSeq.count);
	
	lua_newtable(luaState);
	for (int i=0; i<pitchSeq.count; i++)
	{
		lua_pushnumber(luaState, i+1);
		unsigned char pitch = pitchSeq.pitches[i];
		lua_pushnumber(luaState, pitch);
		lua_settable(luaState, 1);
	}
	
	return 1;
}



int luma_seq_gen(lua_State *luaState)
{
	int index = lua_tonumber(luaState, lua_upvalueindex(3));
	int step = lua_tonumber(luaState, lua_upvalueindex(2));
	int tableSize = luaL_getn(luaState, lua_upvalueindex(1));
	
	// push key to top of stack
	lua_pushnumber(luaState, index);
	// replace key with value at top of stack
	lua_gettable(luaState, lua_upvalueindex(1));
	
	index += step;
	if (index > tableSize)
		index = 1;
	else if (index < 1)
		index = tableSize;
	
	// update index
	lua_pushnumber(luaState, index);
	lua_replace(luaState, lua_upvalueindex(3));

	return 1;
}

int luma_seq (lua_State *luaState) 
{
	// args:
	//	table
	//	step size
	//  direction: "up" or "down"
	int n = lua_gettop(luaState);
	if (n != 2 || !lua_istable(luaState, 1) || !lua_isnumber(luaState, 2)) {
		luma_error(luaState, "incorrect arguments to function 'luma_seq'");
	}
	
	int step = lua_tonumber(luaState, 2);
	
	// push index
	if (step > 0)
		lua_pushnumber(luaState, 1);
	else if (step < 0)
		lua_pushnumber(luaState, luaL_getn(luaState, 1));
	else
		luma_error(luaState, "invalid direction argument to 'luma_seq'");
	
	// push closure
	lua_pushcclosure(luaState, &luma_seq_gen, 3);
	
	return 1;
}

int luma_rand_gen(lua_State *luaState)
{
	int tableSize = luaL_getn(luaState, lua_upvalueindex(1));
	int index = (rand() % tableSize) + 1;
	// push key to top of stack
	lua_pushnumber(luaState, index);
	// replace key with value at top of stack
	lua_gettable(luaState, lua_upvalueindex(1));

	return 1;
}

int luma_rand (lua_State *luaState) 
{
	int n = lua_gettop(luaState);
	if (n != 1 || !lua_istable(luaState, 1)) {
		luma_error(luaState, "incorrect arguments to function 'luma_seq'");
	}
	
	lua_pushcclosure(luaState, &luma_rand_gen, 1);
	
	return 1;
}

int luma_rand_choose(lua_State *luaState)
{
	int n = lua_gettop(luaState);
	if (n != 1 || !lua_istable(luaState, 1)) {
		luma_error(luaState, "incorrect arguments to function 'luma_rand_choose'");
	}

	int tableSize = luaL_getn(luaState, 1);
	int index = (rand() % tableSize) + 1;
	// push key to top of stack
	lua_pushnumber(luaState, index);
	// replace key with value at top of stack
	lua_gettable(luaState, 1);
	
	lua_remove(luaState, 1);

	return 1;
}

#endif


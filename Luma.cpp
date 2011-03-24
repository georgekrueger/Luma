/*
 *  Luma.cpp
 *  vst 2.4 examples
 *
 *  Created by George Krueger on 7/29/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "Luma.h"
#include "LumaFuncs.h"
#include <map>
#include <vector>

using std::map;
using std::vector;
using boost::shared_ptr;

Luma::Luma()
	: updateTime_(0), timerOffset_(0), bpm_(0)
{
	srand( time(NULL) );

	// open a lua vm
	luaState_ = lua_open();
	// open std lua libs
	luaL_openlibs(luaState_);

	lua_register(luaState_, "luma_note", luma_note);
	lua_register(luaState_, "luma_phrase", luma_phrase);
	lua_register(luaState_, "luma_starttimer", luma_starttimer);
	lua_register(luaState_, "luma_stoptimer", luma_stoptimer);
	lua_register(luaState_, "luma_seq_gen", luma_seq);
	//lua_register(luaState, "luma_randex_gen", luma_randex);
	lua_register(luaState_, "luma_rand_gen", luma_rand);
	lua_register(luaState_, "luma_rand_choose", luma_rand_choose);
	lua_register(luaState_, "luma_scale", luma_scale);

	sequencer_ = new Sequencer;
	
	// save reference to luma object in registry so functions can access
	// it through the lua state oject
	lua_pushstring (luaState_, "LUMA_PTR");
	lua_pushinteger(luaState_, (int)this);
	lua_settable (luaState_, LUA_REGISTRYINDEX);
	
	//char* scriptName = "/Users/george/test.lua";
	//if (luaL_dofile(luaState_, scriptName) != 0) {
	//	fprintf(stderr, "%s\n", lua_tostring(luaState_, -1));
	//}
}

Luma::~Luma()
{
	lua_close(luaState_);

	delete sequencer_;
}

bool Luma::SetScript(const char* text, std::string& error)
{
	if (luaL_dostring(luaState_, text) != 0)
	{
		// script failed
		error.append(lua_tostring(luaState_, -1));
		lua_pop(luaState_, 1);
		return false;
	}
	return true;
}

void Luma::SetBPM(float bpm)
{
	if (bpm != bpm_)
	{
		bpm_ = bpm;
		float beat = 60000.0 / bpm_;
		lua_pushnumber(luaState_, bpm_);
		lua_setglobal(luaState_, "bpm");
		lua_pushnumber(luaState_, beat);
		lua_setglobal(luaState_, "beat");
	}
}

void Luma::Start()
{
	timerOffset_ = 0;
	updateTime_ = 0;
	
	sequencer_->Start();

	// lua callback function
	//printf("Luma::Start(), luaState = 0x%x\n", luaState_);
	lua_getfield(luaState_, LUA_GLOBALSINDEX, "start"); /* function to be called */	
	if (!lua_isnil(luaState_, -1)) {
		lua_call(luaState_, 0, 0);
	}
	else {
		lua_pop(luaState_, 1);
	}
}

void Luma::Stop()
{
	listeners_.clear();
	sequencer_->Stop();
}

void Luma::Update(float elapsed, std::vector<shared_ptr<LumaEvent> >& events, std::vector<float>& offsets)
{
	updateTime_ = elapsed;

	// update listeners
	map<const char*, Listener>::iterator l;
	for (l = listeners_.begin(); l != listeners_.end(); l++)
	{
		const char* name = l->first;
		Listener& listener = l->second;
		listener.elapsed += elapsed;
		if (listener.elapsed > listener.time) 
		{
			timerOffset_ = listener.time - (listener.elapsed - elapsed);
			
			//printf("Timer Fired.  UpdateElapsed = %f, ListenerElapsed = %f, ListenerTime = %d, Offset = %f\n",
			//	elapsed, listener.elapsed, listener.time, timerOffset_);
			
			listener.elapsed = listener.elapsed - listener.time;
			
			// repeat count of zero is infinite, otherwise
			// decrement count and check for last repitition
			if (listener.reps != 0) {
				listener.reps -= 1;
				if (listener.reps == 0) 
				{
					listener.done = true;
				}
			}
			
			// lua callback function
			lua_getfield(luaState_, LUA_GLOBALSINDEX, name); /* function to be called */
			lua_pushinteger(luaState_, listener.time);
			lua_call(luaState_, 1, 0);
		}
	}
	
	timerOffset_ = 0;
	
	// remove listeners
	for (l = listeners_.begin(); l != listeners_.end(); l++)
	{
		Listener& listener = l->second;
		if (listener.done) {
			listeners_.erase(l);
		}
	}
	
	// update sequencer
	sequencer_->Update(elapsed, events, offsets);

}

void Luma::AddListener(const char* name, float time, unsigned long reps)
{
	float leftoverTime = updateTime_ - timerOffset_;
	listeners_[name] = Listener(time, leftoverTime, reps);
}

void Luma::RemoveListener(const char* name)
{
	map<const char*, Listener>::iterator i = listeners_.find(name);
	
	if (i != listeners_.end())
	{
		i->second.done = true;
	}
}



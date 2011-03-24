/*
 *  Scale.h
 *  Luma
 *
 *  Created by George Krueger on 6/17/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef SCALE_H
#define SCALE_H

typedef enum ScaleType
{
	kScaleChromatic = 0,
	kScaleIonian,
	kScaleDorian,
	kScalePhrygian,
	kScaleLydian,
	kScaleMixolydian,
	kScaleAeolian,
	kScaleLocrian,
	kScaleNone
};
static const int numScaleTypes = kScaleNone;

typedef struct Scale
{
	ScaleType type;
	char name[40];
	unsigned char intervals[12];
	unsigned char degrees;
};

static const Scale scales[numScaleTypes] = 
{
	{ kScaleChromatic, "Chromatic", { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, 12 },
	{ kScaleIonian, "Ionian", { 2, 2, 1, 2, 2, 2, 1 }, 7 },
	{ kScaleDorian, "Dorian", { 2, 1, 2, 2, 2, 1, 2 }, 7 },
	{ kScalePhrygian, "Phrygian", { 1, 2, 2, 2, 1, 2, 2 }, 7 },
	{ kScaleLydian, "Lydian", { 2, 2, 2, 1, 2, 2, 1}, 7 },
	{ kScaleMixolydian, "Mixolydian", { 2, 2, 1, 2, 2, 1, 2 }, 7 },
	{ kScaleAeolian, "Aeolian", { 2, 1, 2, 2, 1, 2, 2 }, 7 },
	{ kScaleLocrian, "Locrian", { 1, 2, 2, 1, 2, 2, 2 }, 7 }
};

typedef struct ScalePitchSeq
{
	unsigned char pitches[128];
	unsigned char count;
};


bool GetScale(const char* scaleName, Scale& scale)
{
	for (int i=0; i<numScaleTypes; i++)
	{
		if (strcmp(scales[i].name, scaleName) == 0) { 
			scale = scales[i];
			return true;
		}
	}
	return false;
}

template <class T>
bool InArray(T item, T* array, unsigned long arraySize)
{
	for (unsigned long i=0; i<arraySize; i++)
	{
		if (array[i] == item)
			return true;
	}
	return false;
}

/**
* Get the pitches of scale starting on a given root note
*/
void GetScalePitchSequence(unsigned char rootPitch, unsigned char endPitch, ScaleType type, 
		int* validDegrees, int numValidDegrees, ScalePitchSeq& seq)
{
	unsigned char pitch = rootPitch;
	seq.count = 0;
	const Scale& scale = scales[type];
	int i = 0;
	while (pitch <= endPitch) {
		if (numValidDegrees == 0 || InArray(i+1, validDegrees, numValidDegrees))
		{
			seq.pitches[seq.count++] = pitch;
		}
		pitch += scale.intervals[i];
		if (++i >= scale.degrees)
			i = 0;
	}
}

#endif
/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 * Extension by Julian Villegas, University of Aizu, 2014.
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef analyze_beatEstimation_h_
#define analyze_beatEstimation_h_

#include "core_pins.h"
#include "AudioStream.h"

class AudioAnalyzeBeatEstimation : public AudioStream{
public:
	AudioAnalyzeBeatEstimation(void): AudioStream(1, inputQueueArray) { init();}
	bool available() {
		if (outputflag == true) {
			outputflag = false;
			return true;
		}
		return false;
	}
	virtual void update(void);
	uint32_t ac(void); //autocorrelation of ODFs

private:
	void init(void);
    static const int AC_LENGTH = 256;
	audio_block_t *blocklist[16];
	int16_t buffer[2048] __attribute__ ((aligned));
    uint32_t ODFbuffer[AC_LENGTH+1] __attribute__ ((aligned)); // Save previous 6s of ODF
 	uint8_t state;
    uint16_t index;
	uint32_t energy;
    uint32_t silence;
	uint32_t prevEnergy;
	uint32_t currentODF;
    uint32_t AC_m1; //previous AC(0)
	uint32_t AC_m2; //2 previous AC(0)
    int16_t minT; // min outputs before a new beat (for a max of 240 bpm)
    int16_t T;
    
    float thrshld; // as defined in eq. 1, p. 3 Glover et al.
	//float lambda;
	float alpha;
	//float omega;
    float mean;
    uint32_t past[15]; //m = 7
	
    bool outputflag;
	audio_block_t *inputQueueArray[1];
};

#endif

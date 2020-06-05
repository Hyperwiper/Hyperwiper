/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
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
// 
#ifndef peaks_h_
#define peaks_h_

#include "core_pins.h"
#include "AudioStream.h"

class AudioAnalysisPeaks : public AudioStream{
public:
	AudioAnalysisPeaks(void): AudioStream(1, inputQueueArray), 
									outputflag(false) { init();}
	bool available() {
		if (outputflag == true) {
			outputflag = false;
			return true;
		}
		return false;
	}
	virtual void update(void);
	uint32_t isPeak(void);
	void updatePast(uint32_t newValue);
private:
	void init(void);
	audio_block_t *blocklist[20];
	int16_t buffer[2048] __attribute__ ((aligned (4)));
 	uint8_t state;
	uint32_t energy;
	uint32_t prevEnergy;
	uint32_t currentODF;
	uint32_t prevODF;
	uint32_t twoBackODF;
	uint32_t v; // largest peak detected
	uint32_t past[7]; //m = 7
	uint32_t peak;
	// as defined in eq. 1, p. 13 Glover et al. 
	//Real-time detection of musical onsets with linear prediction and sinusoidal modeling
	float thrshld; 
	float lambda;
	float alpha;
	float omega;
	
	float mean;
	float median;
	
	bool outputflag;
	audio_block_t *inputQueueArray[1];
};

#endif

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
// 
#ifndef analyze_peaks_h_
#define analyze_peaks_h_

#include "core_pins.h"
#include "AudioStream.h"

class AudioAnalyzePeaks : public AudioStream{
public:
	AudioAnalyzePeaks(void): AudioStream(1, inputQueueArray) { init();}
	bool available() {
		if (outputflag == true) {
			outputflag = false;
			return true;
		}
		return false;
	}
	virtual void update(void);
	uint32_t getPeak(void);
	uint32_t ODF(void);

private:
	void init(void);
	audio_block_t *blocklist[16];
	int16_t buffer[2048] __attribute__ ((aligned));
 	uint8_t state;
	uint32_t energy;
    uint32_t silence;
	uint32_t prevEnergy;
	uint32_t currentODF;
	uint32_t prevODF;
	uint32_t twoBackODF;
	uint32_t v; // largest peak detected
	uint32_t past[7]; //m = 7
	uint32_t forMedian[7];
	float peak;

	float thrshld; // as defined in eq. 1, p. 3 Glover et al.
	float lambda;
	float alpha;
	float omega;
	
	float mean;
	float median;
	
    bool outputflag;
	audio_block_t *inputQueueArray[1];
};

#endif

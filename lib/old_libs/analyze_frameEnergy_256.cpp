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

#include "analyze_frameEnergy_256.h"

// 140312 - PAH - slightly faster copy
static void copy_to_buffer(void *destination, const void *source){
	const uint16_t *src = (const uint16_t *)source;
	uint16_t *dst = (uint16_t *)destination;
	for (int i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
		*dst++ = *src++;
	}
}

void AudioAnalyzeFrameEnergy256::update(void){
	audio_block_t *block;
    const int16_t *p, *end;
	block = receiveReadOnly();
	if (!block){ 
		return;
	}
    if (!prevblock) {
		prevblock = block;
		return;
	}
    copy_to_buffer(buffer, prevblock->data);
	copy_to_buffer(buffer+128, block->data);
	if (!m_enabled) {
		release(block);
		return;
	}
	p = buffer;
	end = p + 2*AUDIO_BLOCK_SAMPLES;
	sum = 0;
	do {
		int16_t d=*p++;
		sum += (uint32_t)d*(uint32_t)d;
	} while (p < end);
    release(prevblock);
	prevblock = block;
}

void AudioAnalyzeFrameEnergy256::begin(bool noReset){
	m_enabled=true;
}

uint32_t AudioAnalyzeFrameEnergy256::Sum(void){
	return sum;
}
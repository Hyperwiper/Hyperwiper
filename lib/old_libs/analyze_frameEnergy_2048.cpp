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

#include "analyze_frameEnergy_2048.h"

// 140312 - PAH - slightly faster copy
static void copy_to_buffer(void *destination, const void *source){
	const uint16_t *src = (const uint16_t *)source;
	uint16_t *dst = (uint16_t *)destination;
	for (int i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
		*dst++ = *src++;
	}
}

void AudioAnalyzeFrameEnergy2048::update(void){
	audio_block_t *block;
    const int16_t *p, *end;
	block = receiveReadOnly();
	if (!block){ 
		return;
	}
    switch (state) {
        case 0:
            blocklist[0] = block;
            state = 1;
            break;
        case 1:
            blocklist[1] = block;
            state = 2;
            break;
        case 2:
            blocklist[2] = block;
            state = 3;
            break;
        case 3:
            blocklist[3] = block;
            state = 4;
            break;
        case 4:
            blocklist[4] = block;
            state = 5;
            break;
        case 5:
            blocklist[5] = block;
            state = 6;
            break;
        case 6:
            blocklist[6] = block;
            state = 7;
            break;
        case 7:
            blocklist[7] = block;
            state = 8;
            break;
        case 8:
            blocklist[8] = block;
            state = 9;
            break;
        case 9:
            blocklist[9] = block;
            state = 10;
            break;
        case 10:
            blocklist[10] = block;
            state = 11;
            break;
        case 11:
            blocklist[11] = block;
            state = 12;
            break;
        case 12:
            blocklist[12] = block;
            state = 13;
            break;
        case 13:
            blocklist[13] = block;
            state = 14;
            break;
        case 14:
            blocklist[14] = block;
            state = 15;
            break;
        case 15:
            blocklist[15] = block;
            copy_to_buffer(buffer+0x000, blocklist[0]->data);
            copy_to_buffer(buffer+0x080, blocklist[1]->data);
            copy_to_buffer(buffer+0x100, blocklist[2]->data);
            copy_to_buffer(buffer+0x180, blocklist[3]->data);
            copy_to_buffer(buffer+0x200, blocklist[4]->data);
            copy_to_buffer(buffer+0x280, blocklist[5]->data);
            copy_to_buffer(buffer+0x300, blocklist[6]->data);
            copy_to_buffer(buffer+0x380, blocklist[7]->data);
            copy_to_buffer(buffer+0x400, blocklist[8]->data);
            copy_to_buffer(buffer+0x480, blocklist[9]->data);
            copy_to_buffer(buffer+0x500, blocklist[10]->data);
            copy_to_buffer(buffer+0x580, blocklist[11]->data);
            copy_to_buffer(buffer+0x600, blocklist[12]->data);
            copy_to_buffer(buffer+0x680, blocklist[13]->data);
            copy_to_buffer(buffer+0x700, blocklist[14]->data);
            copy_to_buffer(buffer+0x780, blocklist[15]->data);

            p = buffer;
            end = p + 16*AUDIO_BLOCK_SAMPLES;
            sum = 0;
            do {
                int16_t d=*p++;
                sum += (uint32_t)d*(uint32_t)d;
            } while (p < end);
            release(blocklist[0]);
            release(blocklist[1]);
            release(blocklist[2]);
            release(blocklist[3]); // 75% overlap
            
            blocklist[0] = blocklist[4];
            blocklist[1] = blocklist[5];
            blocklist[2] = blocklist[6];
            blocklist[3] = blocklist[7];
            
            blocklist[4] = blocklist[8];
            blocklist[5] = blocklist[9];
            blocklist[6] = blocklist[10];
            blocklist[7] = blocklist[11];
            
            blocklist[8] = blocklist[12];
            blocklist[9] = blocklist[13];
            blocklist[10] = blocklist[14];
            blocklist[11] = blocklist[15];
            
            state = 12;
    }
}

void AudioAnalyzeFrameEnergy2048::begin(bool noReset){
	m_enabled=true;
}

uint32_t AudioAnalyzeFrameEnergy2048::Sum(void){
	return sum;
}
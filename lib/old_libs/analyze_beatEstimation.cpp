
/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 * Extension by Julian Villegas, University of Aizu, 2014.
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.	 Please support PJRC's efforts to develop
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

/* This program computes an Onset Detection Function (ODF)
 * Based on "Real-time detection of musical onsets with linear prediction and
 *  sinusoidal modeling" by ￼￼John Glover, Victor Lazzarini and Joseph Timoney
 * (2011)
 */

#include "analyze_beatEstimation.h"

void AudioAnalyzeBeatEstimation::init(void){
    silence = 19000; // Empirically estimated
    state = 0;
	energy = 0;
	prevEnergy = 0;
	currentODF = 0;
    index = 0;
    alpha = 1;
    minT = 22;
    T = 0;
    for (int i = 0; i < AC_LENGTH+1; i++){
		ODFbuffer[i] = 0;
        if (i<15) {
            past[i] = 0;
        }
	}
}

// 140312 - PAH - slightly faster copy
static void copy_to_buffer(void *destination, const void *source){
	const uint16_t *src = (const uint16_t *)source;
	uint16_t *dst = (uint16_t *)destination;
	for (int i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
		*dst++ = *src++;
	}
}

void AudioAnalyzeBeatEstimation::update(void){
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
            prevEnergy = energy;
            energy = 0;
            do {
                int16_t d=*p++;
                energy += (uint32_t)d*(uint32_t)d;
            } while (p < end);
            
//            if (energy <= silence) {
//                energy = 0; //detecting silence
//                //v = 0;
//            }
            
            //alas! abs() doesn't work!!
            if (energy >= prevEnergy) {
                currentODF = energy - prevEnergy;
            } else {
                currentODF = prevEnergy - energy;
            }
            
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
            
            // update T
            T+=1;
            
            outputflag = true;
            state = 12;
            break;
    }
}

uint32_t AudioAnalyzeBeatEstimation::ac(void){
    // returns the current term of the autocorrelation function
    uint32_t ac[AC_LENGTH+1] __attribute__ ((aligned));
    uint32_t sum;
    uint32_t AC_0;
    uint32_t out;
    
    // update the odf buffer
    if (index < AC_LENGTH) {
        ODFbuffer[index++] = currentODF;
    }else{
        for(int i=0; i<AC_LENGTH; i++) { // Change later for a circular buffer
            ODFbuffer[i] = ODFbuffer[i+1];
        }
        ODFbuffer[AC_LENGTH] = currentODF;
    }

    // update previous AC
    AC_m2 = AC_m1;
    AC_m1 = ac[AC_LENGTH/2];
    for (int i=0; i<AC_LENGTH; i++) {
        sum=0;
        for (int j=1; j <= AC_LENGTH-i; j++) {
            sum += ODFbuffer[j] * ODFbuffer[j+i]/1000000;
        }
        ac[i]=sum;
    }
    AC_0 = ac[AC_LENGTH/2];
//    
//    // compute mean
    mean = 0.0;
    for (int i = 1; i < 15; i++){
        past[i-1] = past[i];
        mean += past[i];
    }
    past[14] = AC_0;
    mean /= 15.0;
    
    // is the input not mute?
    if(energy){
        // is it local maximum?
        if ((AC_m1 > AC_0)	&& (AC_m1 > AC_m2)){
            // update max peak
            //        if(prevODF > v){
            //            v = prevODF;
            //        }
            mean =
            //compute threshold
            // lambda * median +
            thrshld = alpha * mean;// + omega * v; //omega = 0
            
            if (AC_m1 > thrshld){
                if (T>=minT) {
                    out =  AC_m1;
                    T = 0;
                } else{
                    out = 0;
                }
            } else {
                out =  0;
            }
        }else{
            out =  0;
        }
    } else {
        out =  0;
    }
    return out;
}



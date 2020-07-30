#use python V3
#librosa and numpy needed, to install it use: pip3 install librosa numpy
import librosa
import numpy as np
#time of the ticks
#3/4,4/4,4/4
rythms=[[0,0.33,0.33],[0,0.5],[0,0.25,0.25,0.25]] #onset
measures=[4,4,4,4] #4/4
names=["ternary","binary","quaternary"]
bpm=[60,120,240]
sr=44100
i=0
for ticks in rythms:
	for j in bpm:						
		beat_times=[]
		c=0
		measure=measures[i]*(60/j)
		for k in ticks:
			c+=(measure*k)
			beat_times.append(c)		
		click = librosa.clicks(times=beat_times[0],click_freq=1000, sr=sr,length=round(sr*measure))
		clicks = librosa.clicks(times=beat_times[1:len(beat_times)],click_freq=500, sr=sr,length=round(sr*measure))
		s=click+clicks
		signal=[]
		for l in range(0,100): #number of repetitions				
			signal.append(s)		
		librosa.output.write_wav("["+str(j)+"bpm]"+names[i]+'_clicks.wav',np.array(signal).flatten(),sr)
	i+=1
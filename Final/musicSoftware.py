import pygame, sys
import statistics
import serial
from distutils.util import strtobool

# import wave
COM_PORT = "COM9"
BAUD_RATES = 115200
ser1 = serial.Serial(COM_PORT,BAUD_RATES) # ser1 is for strength and toundboard 
COM_PORT = "COM10"
ser2 = serial.Serial(COM_PORT,BAUD_RATES) #ser2 is for emg and keyboard
COM_PORT = "COM11"
ser3 = serial.Serial(COM_PORT,BAUD_RATES) #ser2 is for keyboard
BPM = 76
beat = 4
tempo = 1000 * 60 / BPM

#key state for touch board
touchPrePress = [False,False,False,False]
touchNowPress = [False,False,False,False]
#Strength for key press
strength = [0,0,0,0,0]
#emg  shirt period data
thresholdMax = 20
thresholdMin = 2
period = 5
rleg_emg = list()
rvar = 0
lleg_emg = list()
lvar = 0
rleg_hold = False
lleg_hold = False
# keyboard hold
keyboardHold= [False] * 21


drumPath = ["hdr.wav", "mdr.wav", "ldr.wav","bell.wav","hoho.wav"]
drumTempoList = [[[], [], [0,  1, 2],[],[]],
                 [[0], [0,1,2], [0,1,2],[0, 0.5, 1, 1.5, 2,2.5],[]]]
drumSoundList = list()

soundPath = "sound\\"
pianoPath = ["C.wav", "D.wav", "E.wav", "FU.wav", "G.wav", "A.wav", "B.wav"]
pianoSoundList = list()

keySoundList = list()

chordSoundList = list()
chordList = [[1,[4,6],1,[4,6]], [2,[4,6],2,[4,6] ], [2,[4,7],2,[4,7]], [1,[3,5],1,[3,5]]]
playList = list()
playListIndex = 0

pygame.mixer.pre_init(44100, -16, 2, 512)
pygame.mixer.init()
pygame.init()
pygame.mixer.set_num_channels(30)
screen = pygame.display.set_mode([640, 480])
pygame.time.delay(1000)

for i in drumPath:
    drumSoundList.append(pygame.mixer.Sound(i))
    drumSoundList[-1].set_volume(0.5)
for i in pianoPath:
    pianoSoundList.append(pygame.mixer.Sound(soundPath+"L" + i))
    chordSoundList.append(pygame.mixer.Sound(soundPath+"L" + i))
for i in pianoPath:
    pianoSoundList.append(pygame.mixer.Sound(soundPath+i))
    chordSoundList.append(pygame.mixer.Sound(soundPath+i))
for i in pianoPath:
    pianoSoundList.append( pygame.mixer.Sound(soundPath+"H" +i))
    chordSoundList.append(pygame.mixer.Sound(soundPath+"H" +i))
for i in range(0, len(chordSoundList)):
    chordSoundList[i].set_volume(0.45)
for i in range(len(pianoSoundList)):
    pianoSoundList[i].set_volume(0.6)


def insertPlayList(playSound):
    i = playListIndex
    while i < len(playList) and playList[i][1] < playSound[1]:
        i += 1
    playList.insert(i, playSound)


class drumGroup:
    def __init__(self):
        self.startTime = 0
        self.counter = 0

    def setTimer(self):
        self.startTime = pygame.time.get_ticks()
        self.counter = 0

    def autoStamp(self,drumTempoIndex):
        if drumTempoIndex==0:
            return
        if pygame.time.get_ticks()>self.startTime + self.counter*tempo*beat:
            self.counter +=1
            self.stamp(drumTempoIndex-1)

    def stampTempo(self):
        current_time = pygame.time.get_ticks()
        for i in range(4):
            insertPlayList([3, current_time + tempo * i, 0])

    def stamp(self, drumTempoIndex):
        current_time = pygame.time.get_ticks()
        for i in range(len(drumSoundList)):
            for t in drumTempoList[drumTempoIndex][i]:
                insertPlayList([i, current_time + tempo * t, 0])


class chordGroup:
    def __init__(self):
        pass

    def play(self, chordIndex):
        for i in chordList[chordIndex]:
            chordSoundList[i].play()

    def playSplit(self, chordIndex):
        current_time = pygame.time.get_ticks()
        delta_time = 0
        for i in chordList[chordIndex]:
            if isinstance(i,list):
                for j in i:
                    insertPlayList([j, current_time + delta_time , 1])
            else:
                insertPlayList([i, current_time + delta_time , 1])
            delta_time += tempo*0.5 

class keyGroup:
    def __init__(self):
        pass

    def play(self, chordIndex):
        for i in chordList[chordIndex]:
            chordSoundList[i].play()



drum = drumGroup()
stampCount = 0
chord = chordGroup()

clock = pygame.time.Clock()
while ser1.in_waiting:
    dump = ser1.readline()
while ser2.in_waiting:
    dump = ser2.readline()
while ser3.in_waiting:
    dump = ser3.readline()
while 1:
    drum.autoStamp(stampCount)
    while ser1.in_waiting:
        data_raw = ser1.readline()
        data = data_raw.decode()
        #print(data[0:len(data)-2])
        if data[0]=='d':
            for i in range(4):
                touchNowPress[i] = strtobool(data[i+1])
        elif data[0]=='s':
            for i in range(5):
                strength[i] = float(data[i*4+1:i*4+5])
        
    while ser2.in_waiting:
        data_raw = ser2.readline()
        data = data_raw.decode()
        #print(data[0:len(data)-2])
        if data[0]=='l':
            if len(rleg_emg)>=period:                                                                                                                                                                                                             
                rvar = statistics.pstdev(rleg_emg)
                lvar = statistics.pstdev(lleg_emg)
                if rleg_hold ==False and rvar>thresholdMax: #press
                    rleg_hold = True
                    print("RP")
                    for i in range(len(pianoSoundList)):
                        if keyboardHold[i]!=True:
                            pianoSoundList[i].stop()
                elif rleg_hold==True and rvar<thresholdMin: # release
                    print("RR")
                    rleg_hold=False
                if lleg_hold ==False and lvar>thresholdMax: #press
                    print("LP")
                    lleg_hold = True
                elif lleg_hold==True and lvar<thresholdMin: # release
                    lleg_hold=False
                    print("LR")
                    drum.setTimer()
                    stampCount+=1
                    if stampCount > len(drumTempoList):
                        stampCount = 0
                    
                lleg_emg = list()
                rleg_emg =list()
            lleg_emg.append(int(data.split()[0][1:]))
            rleg_emg.append(int(data.split()[1]))

        if data[0]=='p':
            keyboardHold[int(data[1:])] = True
            pianoSoundList[12-int(data[1:])].play()
            pianoSoundList[12-int(data[1:])].set_volume(0.5+(max(strength)-0.5)*0.5)
        elif data[0] == 'r':
            keyboardHold[int(data[1:])]= False
            
            
    while ser3.in_waiting:
        data_raw = ser3.readline()
        data = data_raw.decode()
        #print(data[0:len(data)-2])
        if data[0]=='p':
            keyboardHold[int(data[1:])] = True
            pianoSoundList[21-int(data[1:])].play()
            pianoSoundList[21-int(data[1:])].set_volume(0.5+(max(strength)-0.5)*0.5)

        elif data[0] == 'r':
            keyboardHold[int(data[1:])]= False


           

    if playListIndex < len(playList):
        while playList[playListIndex][1] < pygame.time.get_ticks():
            if playList[playListIndex][2] == 0:
                drumSoundList[playList[playListIndex][0]].play()
            elif playList[playListIndex][2] == 1:
                chordSoundList[playList[playListIndex][0]].play()
            playListIndex += 1
            if playListIndex == len(playList):
                break

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            sys.exit()
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_a:
                drum.stampTempo()
            if event.key == pygame.K_s:
                drumSoundList[4].play()

            if event.key == pygame.K_KP1:
                pianoSoundList[7].play()
            if event.key == pygame.K_KP2:
                pianoSoundList[8].play()
            if event.key == pygame.K_KP3:
                pianoSoundList[9].play()
            if event.key == pygame.K_KP4:
                pianoSoundList[10].play()
            if event.key == pygame.K_KP5:
                pianoSoundList[11].play()
            if event.key == pygame.K_KP6:
                pianoSoundList[12].play()
            if event.key == pygame.K_KP7:
                pianoSoundList[13].play()
            if event.key == pygame.K_w:
                chord.playSplit(0)
            if event.key == pygame.K_e:
                chord.playSplit(1)
            if event.key == pygame.K_r:
                chord.playSplit(2)
            if event.key == pygame.K_t:
                chord.playSplit(3)

    for i in range(4):
        if touchPrePress[i] == False and touchNowPress[i] ==True: #Button press
            chord.playSplit(i)
    for i in range(4):
        touchPrePress[i] = touchNowPress[i]
            

    clock.tick(40)
# ser.close()

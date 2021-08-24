import threading, Queue, sys
from time import sleep
import urllib, json, sys
from struct import pack

import serial


serialPort = serial.Serial("COM7", baudrate=115200) # Bluetooth

# READ DATA FROM dump1090 running in localhost
url1 = "http://localhost/data/aircraft.json"

urlList = [url1]
#urlList = [url1, url2, url2]


# MSG TYPES ----------------------------------------------------------
LOCAL_POSITION = 0xAA
LOCAL_VECTOR = 0xAB

ADSB_LOCATION = 0xBA
MLAT_LOCATION = 0xBB
TISB_LOCATION = 0xBC
AIRPORT_LOCATION = 0xBD

ADSB_VECTOR = 0xCA
MLAT_VECTOR = 0xCB
TISB_VECTOR = 0xCC

frames = 0

# LOCATION MSG FIELDS ------------------------------------------------
FORMAT1 = '<BBxxIiff'
#< little endian    
#B uint8_t  (1 byte)  >>  MSGTYPE
#B uint8_t  (1 byte)  >>  SEQUENCE
#x pad      (1 byte)  >>  PAD BYTE
#x pad      (1 byte)  >>  PAD BYTE
#I uint_32t (4 bytes) >>  ICAO
#i int_32t  (4 bytes) >>  ALTITUDE
#f float    (4 bytes) >>  LAT
#f float    (4 bytes) >>  LON
#x not type (3 bytes) >>  3 PAD BYTES

#TOTAL = 17 bytes + 3 pad bytes = 20 bytes

# VECTOR MSG FIELDS ------------------------------------------------
#FORMAT2 = 'BBIHHhxxxxxx'
FORMAT2 = '<BBxxIHHhxxxxxx'

#< little endian    
#B uint8_t    (1 byte)  >>  MSGTYPE
#B uint8_t    (1 byte)  >>  SEQUENCE
#I uint_32t   (4 bytes) >>  ICAO
#H uint_16_t  (2 bytes) >>  GROUNDSPEED
#H uint_16_t  (2 bytes) >>  TRACK
#h int_16_t   (2 bytes) >>  BARO_RATE
#x no type    (8 bytes  >>  8 PAD BYTES

#TOTAL = 12 bytes + 8 pad bytes = 20 bytes


# ENCODE Fx --------------------------------------------------------
def encodeLocation(ICAO_ADD, ALT, LAT, LON):  
  return pack(FORMAT1, ADSB_LOCATION, 25, ICAO_ADD, ALT, LAT, LON)

def encodeVector(ICAO_ADD, GSPEED, TRACK, VSPEED):
  return pack(FORMAT2, ADSB_VECTOR, 25, ICAO_ADD, GSPEED, TRACK, VSPEED)

def encodeMyLocation(ICAO_ADD, ALT, LAT, LON):  
  return pack(FORMAT1, LOCAL_POSITION, 25, ICAO_ADD, ALT, LAT, LON)

def encodeMyVector(ICAO_ADD, GSPEED, TRACK, VSPEED):
  return pack(FORMAT2, LOCAL_VECTOR, 25, ICAO_ADD, GSPEED, TRACK, VSPEED)


def decodeLocation(MSG):
  return unpack(FORMAT1, MSG)

def decodeVector(MSG):
  return unpack(FORMAT2, MSG)


########################################################################################
# THREAD DISPATCHER
########################################################################################
class threadDispatcher:

    threads = []
    run = True

    def __init__(self, threadFunction, param1=None, param2=None):
        
        self.thread = threading.Thread(target=threadFunction, args=(param1, param2))        
        self.thread.setDaemon(True)
        self.threads.append(self.thread)        
        self.thread.start()        

    @staticmethod
    def joinAll():
        threadDispatcher.run = False
        for thread in threadDispatcher.threads:            
            thread.join()

########################################################################################
# WORKER FX
########################################################################################
def worker(outQ, url):    
    response = urllib.urlopen(url)
    jsonData = json.loads( response.read() )    

    for aircraft in jsonData['aircraft']:

        # Get Aircraft ICAO Hex Address
        try: icao_hex = int(aircraft['hex'],16)
        except ValueError:
            try: icao_hex = int(aircraft['hex'].strip('~'),16)
            except: pass

        # If location data exists extract it
        if ('seen_pos' in aircraft) and (int(aircraft['seen_pos']) < 10):
            
            # Get Altitude, 0 if aircraft in ground
            try: aircraft['alt'] = int(aircraft['alt_baro'])
            except KeyError: 
                try: aircraft['alt'] = int(aircraft['alt_geom'])
                except: pass
            except ValueError:
                aircraft['alt'] = 0

            # Put location data into queue
            outQ.put( ['L',
                       icao_hex,
                       aircraft['alt'],
                       aircraft['lat'],
                       aircraft['lon'],
                       aircraft['seen_pos']
                      ]                      
                     )

        # If vector data available extract it
        if ('gs' in aircraft) and ('track' in aircraft) and ('baro_rate' in aircraft):
            outQ.put( ['V',
                       icao_hex,
                       aircraft['gs'],
                       aircraft['track'],
                       aircraft['baro_rate'],
                       aircraft['seen']
                      ]                      
                     )

        elif 'gs' and 'track' and 'geom_rate' in aircraft:
            outQ.put( ['V',
                       icao_hex,
                       aircraft['gs'],
                       aircraft['track'],
                       aircraft['geom_rate'],
                       aircraft['seen']
                      ]                      
                     )

        elif 'gs' and 'track' in aircraft:
            outQ.put( ['V',
                       icao_hex,
                       aircraft['gs'],
                       aircraft['track'],
                       0,
                       aircraft['seen']
                      ]                      
                     )


            
        else: pass



########################################################################################
# SERIAL OUTPUT FX
########################################################################################
def SerialOutThread(serialOutQ, serialPort):

    global frames

    print(">> SERIAL OUTPUT THREAD STARTED")

    while threadDispatcher.run:

        if not serialOutQ.empty():
            msg = serialOutQ.get()
            frames+=1
        
            if(serialPort != None):
              #print msg
              serialPort.write( str(msg).rstrip() ) 
              sleep(0.005)

            else: pass
  
    
    if(serialPort != None):        
        print(">> CLOSSING SERIAL PORT..."),
        serialPort.close()
        print("DONE")

    print(">> SERIAL THREAD TERMINATED")


########################################################################################
# MESSAGE ENCODING FX
########################################################################################
def EncodeMsg(data, outputQ):

    if(data[0] == 'L'):
        msg = encodeLocation(data[1], data[2], data[3], data[4])
        outputQ.put(msg)
        
    elif(data[0] == 'V'):
        msg = encodeVector(data[1], data[2], data[3], data[4])
        outputQ.put(msg)
        
    elif(data[0] == 'ML'):
        msg = encodeMyLocation(data[1], data[2], data[3], data[4])
        outputQ.put(msg)

    elif(data[0] == 'MV'):
        msg = encodeMyVector(data[1], data[2], data[3], data[4])
        outputQ.put(msg)

    else: pass

                
########################################################################################
# MAIN
########################################################################################

localCache = dict()
dataQ = Queue.Queue()
serialOutQ = Queue.Queue()

threadDispatcher(SerialOutThread, serialOutQ, serialPort)

# Encode ownship data (vector/location) and send
myvector = ['MV' , 0, 0, 45, 0]
mylocation = ['ML', 0, 0, 29.56, -95.08] # HOU
EncodeMsg(mylocation, serialOutQ)

try:
    while True:
        if not dataQ.empty():
            msg = dataQ.get()
            #print msg            
            EncodeMsg(msg, serialOutQ)            
            
        else:            
            for url in urlList:
                threadDispatcher(worker, dataQ, url)

            print(">> SENDING: %d msgs/sec  \r" % (frames) )
            #sys.stdout.write(">> SENDING: %d msgs/sec  \r" % (frames) )
            #sys.stdout.flush()
            
            frames = 0
            sleep(1)
            

except KeyboardInterrupt:    
    threadDispatcher.joinAll()
    print ">> ADESP->EXIT"


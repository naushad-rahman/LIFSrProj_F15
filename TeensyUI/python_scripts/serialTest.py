import serial
import threading                    #For multithreading
import Queue

timesToRun = 500000
firstRun = True
startBtnClicked = 0
teensySerialData = serial.Serial("COM4", 115200)
i = 0
#f = open('serialTest.txt', 'w')
prevTimeElapsed = 0
end = False
convdone = False

recieved_data = Queue.Queue()
time_data = Queue.Queue()
pmt_data = Queue.Queue()

class serialReadThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        ## Set global precedence to previously defined values
        global firstRun
        global startBtnClicked
        while (startBtnClicked < timesToRun):
            inputBytes = teensySerialData.read(size = 6)
            if (firstRun == True):
                ## Only run once to ensure buffer is completely flushed
                firstRun = False
                teensySerialData.flushInput()
                continue
            #Bytes read in and stored in a char array of size six
            recieved_data.put(inputBytes)
            startBtnClicked += 1

#this is the weakest link and needs to be sped up.
class dataConverterThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        global startBtnClicked
        global convdone

        while (not recieved_data.empty() or startBtnClicked < timesToRun):
            #Get data to work with from queue
            working_data = recieved_data.get()

            #The ord function converts a char to its corresponding ASCII integer, which we can then convert to a float
            timeByte3 = float(ord(working_data[0]))
            timeByte2 = float(ord(working_data[1]))
            timeByte1 = float(ord(working_data[2]))
            timeByte0 = float(ord(working_data[3]))
            pmtByte1 = float(ord(working_data[4]))
            pmtByte0 = float(ord(working_data[5]))

            recieved_data.task_done()
            time_data.put([timeByte0, timeByte1, timeByte2, timeByte3])
            pmt_data.put([pmtByte0, pmtByte1])
        convdone = True

class dataReadThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        global startBtnClicked
        global end
        global prevTimeElapsed
        global convdone
        i = 0
        missed = 0
        f2 = open('serialTest2.txt', 'w')
        while (not time_data.empty() or not convdone):
            i += 1
            data = time_data.get()
            timeElapsed = float(data[3])*256*256*256 + float(data[2])*256*256 + float(data[1])*256 + float(data[0])
            dif = timeElapsed - prevTimeElapsed
            if ((dif > 150) and i != 1): # or True):
                f2.write(str(i) + "   " + str(timeElapsed) + "   " + str(dif) + '\n')
                missed += (dif/100) - 1
            prevTimeElapsed = timeElapsed
        f2.write("missed time: " + str(missed) + "   (" + str((missed/timesToRun)*100) + "%)")
        f2.close
        end = True

serial_read_thread = serialReadThread()
serial_read_thread.daemon = True
serial_read_thread.start()
data_converter_thread = dataConverterThread()
data_converter_thread.daemon = True
data_converter_thread.start()
data_read_thread = dataReadThread()
data_read_thread.daemon = True
data_read_thread.start()
while (not end):
    pass

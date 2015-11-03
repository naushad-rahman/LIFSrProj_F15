# DSP Functions for uFluidics Senior Project
# Developed by Luke Newmeyer
# October 29, 2015

import numpy as np #Types and statistics functions
from scipy import signal #Includes convolve functions and others
#from __future__ import division #Essential for processing functions
#from __future__ import print_function #Debug purposes

CODE_VERSION = "udsp Version 1.0"
CODE_DATE = "November 3, 2015"

# Constants for Filtereing and Thresholding
SAMPLE_RATE = 9091 #samples/second
FLUORESCE_PERIOD_MIN = 0.05 #seconds
THRESHOLD_WIDTH = FLUORESCE_PERIOD_MIN * SAMPLE_RATE #samples
THRESHOLD_SD = 2 #standard deviations (from mean to set height threshold)
CALIBRATION_PERIOD = 1 #seconds
CALIBRATION_WIDTH = CALIBRATION_PERIOD * SAMPLE_RATE #samples
FILTBLK_SIZE = 200 #samples

# Filter Definition
LP_FILTER_TYPE = "Spring Summer Low Pass Filter (spsu_lp_filter_norm)"
LP_FILTER = [0.000326, 0.000692, 0.001207, 0.001891, 0.002756, 0.003806, 
	0.005036, 0.006430, 0.007961, 0.009591, 0.011275, 0.012958, 0.014582, 
	0.016086, 0.017413, 0.018507, 0.019325, 0.019829, 0.020000, 0.019829, 
	0.019325, 0.018507, 0.017413, 0.016086, 0.014582, 0.012958, 0.011275, 
	0.009591, 0.007961, 0.006430, 0.005036, 0.003806, 0.002756, 0.001891, 
	0.001207, 0.000692, 0.000326]

# Threshold Filter Definition
THRESHOLD_FILTER = np.ones(THRESHOLD_WIDTH) / THRESHOLD_WIDTH

# Internal udsp Data Structures
signal_ol = np.zeros(len(LP_FILTER)-1)
threshold_ol = np.zeros(len(THRESHOLD_FILTER)-1)
signal_ary = []#np.zeros(FILTBLK_SIZE) #TODO: remove this when teensyData.py is updated to teensyData2.py

# Continous Filtering Using Overlap-add Method
# s1: array of data to be filtered
# s2: array of filter (s2 < s1)
# ol: overlap (returned from previous filter operation)
# returned s3: array of filtered data
# returned ol: returned overlap
def continous_filter(s1, s2, ol):
	s3 = signal.convolve(s1, s2)
	s3[0:len(s2)-1] = s3[0:len(s2)-1] + ol
	ol = s3[len(s3)-len(s2)+1:]
	return s3[0:len(s1)], ol

# Calibration of Thresholding
# s1: array of samples as calibration sample
# sd: number of standard deviations out to set threshold
# returned bl: baseline
# returned th: threshold
def calibrate(s1):
	global THRESHOLD_SD
	th = THRESHOLD_SD * np.std(s1)
	bl = np.mean(s1)
	return bl, th

# Filter Signal Continously by LP_FILTER
# signal: array of samples to be filtered
# returned signal: filtered signal by LP_FILTER
def filter_signal(signal):
	global LP_FILTER
	global signal_ol
	signal, signal_ol = continous_filter(signal, LP_FILTER, signal_ol)
	return signal

def filter_threshold(signal):
	global THRESHOLD_FILTER
	global threshold_ol
	signal, threshold_ol = continous_filter(signal, THRESHOLD_FILTER, threshold_ol)
	return signal

def calibrate(signal):
	global THRESHOLD_SD
	baseline = np.mean(signal)
	threshold = np.std(signal) * THRESHOLD_SD
	return baseline, threshold

# Threshold Siganl Section
#def threshold(s1, th):

'''
class dataFilterThread (threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)
	def run(self):
		global udsp.lp_filter

class dataThresholdThread (threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)
	def run(self):
		while (startBtnClicked):
		
		
def calibrateButtonClicked():
	global calibrateBtnClicked
	global calibrateBtn

'''

from Tkinter import *               #For window
import tkMessageBox                 #For error message box
import json                         #For tags

## Create window
chooseTag = Tk()
instr = Label(chooseTag, text = "Which tag(s) to filter for?")
instr.pack(padx = 10, pady = 10, anchor = W)
inputBoxFrame = Frame(chooseTag)
inputBoxFrame.pack(padx = 10, anchor = W)
inputBoxLeftFrame = Frame(chooseTag)
inputBoxLeftFrame.pack(in_ = inputBoxFrame, padx = 10, side = LEFT, anchor = W)
inputBoxRightFrame = Frame(chooseTag)
inputBoxRightFrame.pack(in_ = inputBoxFrame, padx = 10, side = LEFT, anchor = W)
inputBoxFrameL0 = Frame(chooseTag, pady = 2)
inputBoxFrameL0.pack(in_ = inputBoxLeftFrame, padx = 10, anchor = W)
inputBoxFrameR0 = Frame(chooseTag, pady = 2)
inputBoxFrameR0.pack(in_ = inputBoxRightFrame, padx = 10, anchor = W)
inputBoxFrameL1 = Frame(chooseTag, pady = 2)
inputBoxFrameL1.pack(in_ = inputBoxLeftFrame, padx = 10, anchor = W)
inputBoxFrameR1 = Frame(chooseTag, pady = 2)
inputBoxFrameR1.pack(in_ = inputBoxRightFrame, padx = 10, anchor = W)
inputBoxFrameL2 = Frame(chooseTag, pady = 2)
inputBoxFrameL2.pack(in_ = inputBoxLeftFrame, padx = 10, anchor = W)
inputBoxFrameR2 = Frame(chooseTag, pady = 2)
inputBoxFrameR2.pack(in_ = inputBoxRightFrame, padx = 10, anchor = W)
inputBoxFrameL3 = Frame(chooseTag, pady = 2)
inputBoxFrameL3.pack(in_ = inputBoxLeftFrame, padx = 10, anchor = W)
inputBoxFrameR3 = Frame(chooseTag, pady = 2)
inputBoxFrameR3.pack(in_ = inputBoxRightFrame, padx = 10, anchor = W)
inputBoxFrameL4 = Frame(chooseTag, pady = 2)
inputBoxFrameL4.pack(in_ = inputBoxLeftFrame, padx = 10, anchor = W)
inputBoxFrameR4 = Frame(chooseTag, pady = 2)
inputBoxFrameR4.pack(in_ = inputBoxRightFrame, padx = 10, anchor = W)
l = Label(chooseTag, text = "exclude   include")
l.pack(padx = 10, pady = 10, anchor = W)
checks = Frame(chooseTag)
checks.pack(anchor = W)
brokenChecks = Frame(checks)
brokenChecks.pack(anchor = W)
successChecks = Frame(checks)
successChecks.pack(anchor = W)
wrongChecks = Frame(checks)
wrongChecks.pack(anchor = W)

deviceNumLabel = Label(chooseTag, text = "Microfluidic device number: ")
deviceNumEntry = Entry(chooseTag)
deviceNumUsedLabel = Label(chooseTag, text = "Times device has been used: ")
deviceNumUsedEntry = Entry(chooseTag)
laserPosLabel = Label(chooseTag, text = "Laser position: ")
laserPosEntry = Entry(chooseTag)
analogGainLabel = Label(chooseTag, text = "Analog gain: ")
analogGainEntry = Entry(chooseTag)
laserVoltLabel = Label(chooseTag, text = "Laser voltage: ")
laserVoltEntry = Entry(chooseTag)
pmtVoltLabel = Label(chooseTag, text = "PMT control voltage: ")
pmtVoltEntry = Entry(chooseTag)
hvSettingsLabel = Label(chooseTag, text = "HV settings: ")
hvSettingsEntry = Entry(chooseTag)
buffSolLabel = Label(chooseTag, text = "Buffer solution used: ")
buffSolEntry = Entry(chooseTag)
fluorophoreLabel = Label(chooseTag, text = "Fluorophore used: ")
fluorophoreEntry = Entry(chooseTag)
fluorophoreConLabel = Label(chooseTag, text = "Fluorophore concentration: ")
fluorophoreConEntry = Entry(chooseTag)

deviceNumLabel.pack(in_ = inputBoxFrameL0, side = LEFT)
deviceNumEntry.pack(in_ = inputBoxFrameL0, side = LEFT)
deviceNumUsedLabel.pack(in_ = inputBoxFrameR0, side = LEFT)
deviceNumUsedEntry.pack(in_ = inputBoxFrameR0, side = LEFT)
laserPosLabel.pack(in_ = inputBoxFrameL1, side = LEFT)
laserPosEntry.pack(in_ = inputBoxFrameL1, side = LEFT)
analogGainLabel.pack(in_ = inputBoxFrameR1, side = LEFT)
analogGainEntry.pack(in_ = inputBoxFrameR1, side = LEFT)
laserVoltLabel.pack(in_ = inputBoxFrameL2, side = LEFT)
laserVoltEntry.pack(in_ = inputBoxFrameL2, side = LEFT)
pmtVoltLabel.pack(in_ = inputBoxFrameR2, side = LEFT)
pmtVoltEntry.pack(in_ = inputBoxFrameR2, side = LEFT)
hvSettingsLabel.pack(in_ = inputBoxFrameL3, side = LEFT)
hvSettingsEntry.pack(in_ = inputBoxFrameL3, side = LEFT)
buffSolLabel.pack(in_ = inputBoxFrameR3, side = LEFT)
buffSolEntry.pack(in_ = inputBoxFrameR3, side = LEFT)
fluorophoreLabel.pack(in_ = inputBoxFrameL4, side = LEFT)
fluorophoreEntry.pack(in_ = inputBoxFrameL4, side = LEFT)
fluorophoreConLabel.pack(in_ = inputBoxFrameR4, side = LEFT)
fluorophoreConEntry.pack(in_ = inputBoxFrameR4, side = LEFT)

## Variables showing if the boxes are checked (1) or not (0)
includeBroken = IntVar()
includeSuccess = IntVar()
includeWrong = IntVar()
excludeBroken = IntVar()
excludeSuccess = IntVar()
excludeWrong = IntVar()

## Create the checkboxes
excludeBrokenCheck = Checkbutton(chooseTag, variable=excludeBroken, onvalue = 1, offvalue = 0)
includeBrokenCheck = Checkbutton(chooseTag, text="Equipment failure", variable=includeBroken, onvalue = 1, offvalue = 0)
excludeSuccessCheck = Checkbutton(chooseTag, variable=excludeSuccess, onvalue = 1, offvalue = 0)
includeSuccessCheck = Checkbutton(chooseTag, text="Successful experiment", variable=includeSuccess, onvalue = 1, offvalue = 0)
excludeWrongCheck = Checkbutton(chooseTag, variable=excludeWrong, onvalue = 1, offvalue = 0)
includeWrongCheck = Checkbutton(chooseTag, text="I just don't know what went wrong", variable=includeWrong, onvalue = 1, offvalue = 0)

excludeBrokenCheck.pack(in_ = brokenChecks, side = LEFT, padx = 10)
includeBrokenCheck.pack(in_ = brokenChecks, side = LEFT, padx = 10)
excludeSuccessCheck.pack(in_ = successChecks, side = LEFT, padx = 10)
includeSuccessCheck.pack(in_ = successChecks, side = LEFT, padx = 10)
excludeWrongCheck.pack(in_ = wrongChecks, side = LEFT, padx = 10)
includeWrongCheck.pack(in_ = wrongChecks, side = LEFT, padx = 10)

verbose = IntVar()
verboseCheck = Checkbutton(chooseTag, text="Show full text", variable=verbose, onvalue = 1, offvalue = 0)
verboseCheck.pack(pady = 10)

## code for submit button
def submit():
    ## Create an error message box if the user tries to include and exclude the same tag.
    if ((excludeBroken.get() == 1 and includeBroken.get() == 1) or (excludeSuccess.get() == 1 and includeSuccess.get() == 1) or (excludeWrong.get() == 1 and includeWrong.get() == 1)):
        tkMessageBox.showerror("ERROR", "Cannot include and exclude a tag.")
    else:
	    ## Load the tags into a dictionary
        with open('RecordedData\\tags.json', 'r') as fp:
            checkDict = json.load(fp)
        outputList = []
		## Iterate through the tags and save any that match the check boxes and device number
        for key in checkDict:
            if (deviceNumEntry.get() != "" and (checkDict[key]["deviceNum"] != deviceNumEntry.get())):
                continue
            elif (deviceNumUsedEntry.get() != "" and (checkDict[key]["deviceUsed"] != deviceNumUsedEntry.get())):
                continue
            elif (laserPosEntry.get() != "" and (checkDict[key]["laserPos"] != laserPosEntry.get())):
                continue
            elif (analogGainEntry.get() != "" and (checkDict[key]["analogGain"] != analogGainEntry.get())):
                continue
            elif (laserVoltEntry.get() != "" and (checkDict[key]["laserVolt"] != laserVoltEntry.get())):
                continue
            elif (pmtVoltEntry.get() != "" and (checkDict[key]["pmtVolt"] != pmtVoltEntry.get())):
                continue
            elif (hvSettingsEntry.get() != "" and (checkDict[key]["hvSettings"] != hvSettingsEntry.get())):
                continue
            elif (buffSolEntry.get() != "" and (checkDict[key]["buffSol"] != buffSolEntry.get())):
                continue
            elif (fluorophoreEntry.get() != "" and (checkDict[key]["fluorophore"] != fluorophoreEntry.get())):
                continue
            elif (fluorophoreConEntry.get() != "" and (checkDict[key]["fluorophoreCon"] != fluorophoreConEntry.get())):
                continue
            elif ((includeBroken.get() == 1 and checkDict[key]["broken"] == 0) or (excludeBroken.get() == 1 and checkDict[key]["broken"] == 1)):
                continue
            elif ((includeSuccess.get() == 1 and checkDict[key]["success"] == 0) or (excludeSuccess.get() == 1 and checkDict[key]["success"] == 1)):
                continue
            elif ((includeWrong.get() == 1 and checkDict[key]["wrong"] == 0) or (excludeWrong.get() == 1 and checkDict[key]["wrong"] == 1)):
                continue
            else:
                outputList.append(key)
		## Create the output
        output = ""
        for outkey in sorted(outputList):
            output += "***" + outkey + "***"
            if verbose.get() == 1:
                if checkDict[key]["deviceNum"] != "":
                    output += "\n[Microfluidic device #" + checkDict[key]["deviceNum"] + "]"
                if checkDict[outkey]["success"] == 1:
                    output += "\n[Successful experiment]"
                if checkDict[outkey]["broken"] == 1:
                    output += "\n[Equipment failure]"
                if checkDict[outkey]["wrong"] == 1:
                    output += "\n[I just don't know what went wrong]"
                output += "\n" + checkDict[outkey]["text"]
            output += "\n\n"
        if output == "":
            output = "No results."
        ## close the prompt window
        chooseTag.destroy()
		## Create a new window with a textbox showing the results
        outputWindow = Tk()
        textBox = Text(outputWindow)
        textBox.pack(padx = 10, pady = 10)
        textBox.focus_set()
        textBox.insert(END, output)

submitButton = Button(chooseTag, text = "Submit", width = 10, command = submit)
submitButton.pack(pady = 10)

chooseTag.mainloop()

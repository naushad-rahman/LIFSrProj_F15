from Tkinter import *               #For prompt
import json                         #For tags

chooseTag = Tk()
instr = Label(chooseTag, text = "Which tag(s) to filter for?")
instr.pack(padx = 10, pady = 10, anchor = W)
l = Label(chooseTag, text = "exclude   include")
l.pack(padx = 10, pady = 10, anchor = W)
checks = Frame(chooseTag)
checks.pack()
brokenChecks = Frame(checks)
brokenChecks.pack(anchor = W)
successChecks = Frame(checks)
successChecks.pack(anchor = W)
wrongChecks = Frame(checks)
wrongChecks.pack(anchor = W)

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

def submit():
    if ((excludeBroken.get() == 1 and includeBroken.get() == 1) or (excludeSuccess.get() == 1 and includeSuccess.get() == 1) or (excludeWrong.get() == 1 and includeWrong.get() == 1)):
        tkMessageBox.showerror("ERROR", "Cannot include and exclude a tag.")
    else:
        with open('RecordedData\\tags.json', 'r') as fp:
            checkDict = json.load(fp)
        outputList = []
        for key in checkDict:
            if ((includeBroken.get() == 1 and checkDict[key]["broken"] == False) or (excludeBroken.get() == 1 and checkDict[key]["broken"] == True)):
                continue
            elif ((includeSuccess.get() == 1 and checkDict[key]["success"] == False) or (excludeSuccess.get() == 1 and checkDict[key]["success"] == True)):
                continue
            elif ((includeWrong.get() == 1 and checkDict[key]["wrong"] == False) or (excludeWrong.get() == 1 and checkDict[key]["wrong"] == True)):
                continue
            else:
                outputList.append(key)
        output = ""
        for outkey in sorted(outputList):
            output += "***" + outkey + "***"
            if verbose.get() == 1:
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
        chooseTag.destroy()
        outputWindow = Tk()
        textBox = Text(outputWindow)
        textBox.pack(padx = 10, pady = 10)
        textBox.focus_set()
        textBox.insert(END, output)


submitButton = Button(chooseTag, text = "Submit", width = 10, command = submit)
submitButton.pack(pady = 10)

chooseTag.mainloop()

import serial
import time
import MySQLdb

# DB Data Indeces
CAGE = 0
PROGRAM = 1
MOUSE = 2
TRIAL = 3
TRIAL_START = 4
TRIAL_END = 5
DURATION = 6
RESULT = 7
WAIT_FOR_AP = 8
WAITED = 9
ITERATION = 10

def parseInput(data):
    parsedData = []

    for text in data:
        parsedData.append(int(text))
    
    return parsedData

def saveToDatabase(data):
    print("Parsing data...")
    data = parseInput(data)
    
    print("Create a connection to cage database")
    db = MySQLdb.connect(
        host="localhost",
        user="root",
        passwd="root",
        db="cage")

    print("Create cursor to execute queries")
    cur = db.cursor()

    print("Store information in database")
    try:
        cur.execute("""
            INSERT INTO trial_data
            (`cage`, `program`, `mouse`, `trial`, `trialStart`, `trialEnd`, `duration`, `result`, `wait4AP`, `waited`, `iteration`)
            VALUES(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)""", (
            data[CAGE], data[PROGRAM], data[MOUSE], data[TRIAL], data[TRIAL_START], data[TRIAL_END],
            data[DURATION], data[RESULT], data[WAIT_FOR_AP], data[WAITED], data[ITERATION]
        ))
        db.commit()
        print("Commit")
    except MySQLdb.Error, e:
        try:
            print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            return None
        except IndexError:
            print "MySQL Error: %s" % str(e)
            return None
    except TypeError, e:
        print("MySQL Error: TypeError: %s" % str(e))
        return None
    except ValueError, e:
        print("MySQL Error: ValueError: %s" % str(e))
        return None  

    print("Close database connection")
    db.close() 

# Establish connection to Arduino
ser = serial.Serial('/dev/ttyACM1',9600)
ser.open()
time.sleep(5)

while True:
    data = ser.readline().split()
    print("INPUT: " + str(len(data)))
    if len(data) == 11:
        print("Saving data...")
        saveToDatabase(data)

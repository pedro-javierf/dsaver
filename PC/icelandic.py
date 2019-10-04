#!/usr/bin/python3
import sys
import time
import serial

ver      = "1.1"
port     = ""
filename = ""
baudrate = 115200 #default value
counter  = 0
mem_page = b''
device_detected = False
done     = False

def main():
    print("dsaver " + ver)
    print("(Íslensk þýðing eftir: Elías Andri)")
    print("Inngangs raðgátt: ", end= '')
    port = input()

    print("[>] Bíð eftir dsaver tæki",end='',flush=True)
    while(not(device_detected)):
        try:
            ser = serial.Serial(port, baudrate, timeout=5) #5 sec timeout
            device_detected = True
            print("",flush=True)
        except:
            print(".",end='',flush=True)
            time.sleep(1)

    tmp = ser.read(8)
    if(tmp==b'dsaver\r\n'):
        print("[>] dsaver tæki fundið. ")
    else:
        print("[!] Óþekkt tæki fundið. Hættir")
        sys.exit(1)

    #selection menu DUMP / INJECT
    opt = -1
    while(opt<0 or opt>2):
        print()
        print("Veldu valkost:")
        print("1. Henda Vistuðum Leik ")
        print("2. Láttu inn Vistaðan Leik ")
        print("0. Hætta")
        print("> ",end='')
        try:
            opt = int(input())
        except:
            time.sleep(0.1)

    if(opt==1):
        print("Inntak úttak skráarnafn: ", end='')
        filename=input()
        with open(str(filename), "wb") as savegame:
            time.sleep(0.5)
            ser.write(b'\x45')

            while(not(done)):

                try:
                    #read one byte
                    x = ser.read()
                    counter+=1

                    # Write byte to the file
                    savegame.write(x)
                except:
                    print("[!] Tækið hefur aftengts")
                    print("counter: "+str(counter))
                    done = True

                #debug
                #print(x)
                if(counter%100==0):
                    print('.',end='',flush=True)
                if(counter==65536):
                    done = True
                    print("\n[>] Hent var kóða á réttan hátt",flush=True)


    elif(opt==2):
        print("Sláðu inn skráarnafn: ", end='')
        filename=input()
        with open(str(filename), "rb") as savegame:
            ser.write(b'\x49') #inject command

            #send frames of 32 bytes each

            while(not(done)):
                mem_page = savegame.read(32) #read 32 bytes from file
                #print(mem_page)
                #print()
                ser.write(mem_page) #send 32 bytes to arduino
                counter += 1
                if(counter==2048):
                    done = True
                time.sleep(0.1)
                print(counter)

        print("[>] Búið")
        #file is closed on with-open block exit

    
if __name__ == '__main__':
    main()
#include "VineAlert.h"
#include "JC_Button.h" // https://github.com/JChristensen/JC_Button


using namespace RetailAlert;

SYSTEM_THREAD(ENABLED);

// PUB-SUB constants
constexpr char * DEVICE_NAME = "PEX_ALERT";
constexpr char * WINDOWSHUT_MSSG = "Office window shut.";
constexpr char * WINDOWOPEN_MSSG = "Office window open.";
constexpr char * BWINDOWSHUT_MSSG = "Bathroom window shut.";
constexpr char * BWINDOWOPEN_MSSG = "Bathroom window open.";
constexpr char * BDSHUT_MSSG = "Bathroom door shut.";
constexpr char * BDOPEN_MSSG = "Bathroom door open.";
constexpr char * SDCLOSED_MSSG = "Sliding door closed.";
constexpr char * SDOPEN_MSSG = "Sliding door open.";
constexpr char * KEEPBOLT_MSSG = "Cavern door locked.";
constexpr char * KEEPOPEN_MSSG = "Cavern door unlocked.";
constexpr char * MDKEEPBOLT_MSSG = "Main door locked.";
constexpr char * MDKEEPOPEN_MSSG = "Main door unlocked.";
constexpr char * RSATUS_MSSG = "Requesting Status";
constexpr char * SHOPL_MSSG = "SHOPLIFTER";
constexpr char * RESCUE_MSSG = "RESCUE";
constexpr char * RAIN_MSSG = "RAIN";
constexpr char * CAV_FAIL_MSSG = "COMMS FAIL FROM CAVERN";
constexpr char * CAV_FAIL2_MSSG = "2nd COMMS FAIL FROM CAVERN";
constexpr char * OFF_FAIL_MSSG = "COMMS FAIL FROM OFFICE";
constexpr char * OFF_FAIL2_MSSG = "2nd COMMS FAIL FROM OFFICE";


int slidingstate = 3;
int cavernkeepstate = 3;
int officewindowlocked = 3;
int bathroomwindowlocked = 3;
int maindoorlocked = 3;
boolean CavernFail = false;
boolean OfficeFail = false;


IndicatorLED BathroomLED(D3);	//should be D3
IndicatorLED OfficeLED(D4);	//should be D4
IndicatorLED SlidingLED(D0);	//Should be D0
IndicatorLED CavernLED(D1);	//should be D1
IndicatorLED MainLED(D2);	//should be D2


MillisTimer HeartbeatCavern(960000, HBCavCallback, false);
MillisTimer ResetHBCavern(3600000, ResetCavCallback, false);
MillisTimer HeartbeatOffice(960000, HBOfficeCallback, false);
MillisTimer ResetHBOffice(3600000, ResetOfficeCallback, false);



// function declarations
void eventHandler(const char * event,
        const char * data);

#include "DFRobotDFPlayerMini.h"
#include "TimeAlarms/TimeAlarms.h"
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

boolean alreadyRun = false;

void setup() {
        Particle.subscribe(DEVICE_NAME, eventHandler, MY_DEVICES);
        setZone();
        Serial.begin(9600);
		HeartbeatCavern.begin();
		ResetHBCavern.begin();
		HeartbeatCavern.start();
		HeartbeatOffice.begin();
		ResetHBOffice.begin();
		HeartbeatOffice.start();
        CavernLED.begin();
		MainLED.begin();	
		SlidingLED.begin();	
		OfficeLED.begin();	
		BathroomLED.begin();	
        Particle.function("slstate", setSlidingState);
        Particle.function("cvstate", setCavernState);
        Particle.function("mnstate", setMainState);
        Particle.function("ofstate", setOfficeState);
        Particle.function("btstate", setBathState);
        Alarm.alarmRepeat(8, 1, 00, setZone);
		Alarm.alarmRepeat(10, 13, 00, checkDoors);
		Alarm.alarmRepeat(10, 13, 30, checkDoors);

        Serial1.begin(9600);

        if (!myDFPlayer.begin(Serial1)) { // use Serial1 to communicate with mp3.
                Serial.println(F("Unable to begin:"));
                Serial.println(F("1.Please recheck the connection!"));
                Serial.println(F("2.Please insert the SD card!"));
                while (true);
        }
        Serial.println(F("DFPlayer Mini online."));

        myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms

        //----Set volume----
        myDFPlayer.volume(30); //Set volume value (0~30).
        myDFPlayer.volumeUp(); //Volume Up
        myDFPlayer.volumeDown(); //Volume Down

        //----Set different EQ----
        myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);

        //----Set device we use SD as default----
        //  myDFPlayer.outputDevice(DFPLAYER_DEVICE_U_DISK);
        myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

        //----Read imformation----
        Serial.println(myDFPlayer.readState()); //read mp3 state
        Serial.println(myDFPlayer.readVolume()); //read current volume
        Serial.println(myDFPlayer.readEQ()); //read EQ setting
        Serial.println(myDFPlayer.readFileCounts()); //read all file counts in SD card
        Serial.println(myDFPlayer.readCurrentFileNumber()); //read current play file number
        Serial.println(myDFPlayer.readFileCountsInFolder(3)); //read fill counts in folder SD:/03

}

void loop() {
        if (alreadyRun == false) {
                InitialPub();
        }

        ToggleSwitchAction::update();
        //digitalClockDisplay();
        MillisTimer::processTimers();

        Alarm.delay(0); // wait one second between clock display

        static unsigned long timer = millis();

        if (millis() - timer > 3000) {
                timer = millis();
        }

        if (myDFPlayer.available()) {
                printDetail(myDFPlayer.readType(), myDFPlayer.read());
        }

        if (cavernkeepstate == 0) {
                CavernLED.off();
        } else {
                CavernLED.on();
        }
        if (slidingstate == 0) {
                SlidingLED.off();
        } else {
                SlidingLED.on();
        }
        if (bathroomwindowlocked == 0) {
                BathroomLED.off();
        } else {
                BathroomLED.on();
        }
        if (maindoorlocked == 0) {
                MainLED.off();
        } else {
                MainLED.on();
        }
        if (officewindowlocked == 0) {
                OfficeLED.off();
        } else {
                OfficeLED.on();
        }

}

// functions to be called when an alarm triggers:

void Repeats() {
        Serial.println("15 second timer");
}

void printDetail(uint8_t type, int value) {
        switch (type) {
        case TimeOut:
                Serial.println(F("Time Out!"));
                break;
        case WrongStack:
                Serial.println(F("Stack Wrong!"));
                break;
        case DFPlayerCardInserted:
                Serial.println(F("Card Inserted!"));
                break;
        case DFPlayerCardRemoved:
                Serial.println(F("Card Removed!"));
                break;
        case DFPlayerCardOnline:
                Serial.println(F("Card Online!"));
                break;
        case DFPlayerPlayFinished:
                Serial.print(F("Number:"));
                Serial.print(value);
                Serial.println(F(" Play Finished!"));
                break;
        case DFPlayerError:
                Serial.print(F("DFPlayerError:"));
                switch (value) {
                case Busy:
                        Serial.println(F("Card not found"));
                        break;
                case Sleeping:
                        Serial.println(F("Sleeping"));
                        break;
                case SerialWrongStack:
                        Serial.println(F("Get Wrong Stack"));
                        break;
                case CheckSumNotMatch:
                        Serial.println(F("Check Sum Not Match"));
                        break;
                case FileIndexOut:
                        Serial.println(F("File Index Out of Bound"));
                        break;
                case FileMismatch:
                        Serial.println(F("Cannot Find File"));
                        break;
                case Advertise:
                        Serial.println(F("In Advertise"));
                        break;
                default:
                        break;
                }
                break;
        default:
                break;
        }
}

void digitalClockDisplay() {
        // digital clock display of the time
        Serial.print(Time.hour());
        printDigits(Time.minute());
        printDigits(Time.second());
        Serial.println();
}

void printDigits(int digits) {
        Serial.print(":");
        if (digits < 10)
                Serial.print('0');
        Serial.print(digits);
}

void HBCavCallback(void) {
        
        if (CavernFail == false) {
                HeartbeatCavern.stop();
				CavernFail = true;
                Particle.publish(DEVICE_NAME, CAV_FAIL_MSSG, 60, PRIVATE);
				HeartbeatCavern.start();
				ResetHBCavern.start();
        }
        else if (CavernFail == true) {
                HeartbeatCavern.stop();
                Particle.publish(DEVICE_NAME, CAV_FAIL2_MSSG, 60, PRIVATE);
				CavernFail = false;
        }
        
}

void ResetCavCallback(void) {
CavernFail = false;
        
}

void HBOfficeCallback(void) {
        
        if (OfficeFail == false) {
                HeartbeatOffice.stop();
				OfficeFail = true;
                Particle.publish(DEVICE_NAME, OFF_FAIL_MSSG, 60, PRIVATE);
				HeartbeatOffice.start();
				ResetHBOffice.start();
        }
        else if (OfficeFail == true) {
                HeartbeatOffice.stop();
                Particle.publish(DEVICE_NAME, OFF_FAIL2_MSSG, 60, PRIVATE);
				OfficeFail = false;
        }
        
}

void ResetOfficeCallback(void) {
OfficeFail = false;
        
}


void eventHandler(const char * event,
        const char * data) {
        if (strcmp(data, SHOPL_MSSG) == 0) {
                //myDFPlayer.playMp3Folder(1);
        } else if (strcmp(data, RESCUE_MSSG) == 0) {
                //myDFPlayer.playMp3Folder(4);
        } else if (strcmp(data, RAIN_MSSG) == 0) {
                myDFPlayer.playMp3Folder(20);
        } else if (strcmp(data, MDKEEPOPEN_MSSG) == 0) {
                maindoorlocked = 1;
        } else if (strcmp(data, MDKEEPBOLT_MSSG) == 0) {
                maindoorlocked = 0;
        } else if (strcmp(data, SDOPEN_MSSG) == 0) {
                slidingstate = 1;
        } else if (strcmp(data, SDCLOSED_MSSG) == 0) {
                slidingstate = 0;
        } else if (strcmp(data, KEEPOPEN_MSSG) == 0) {
                cavernkeepstate = 1;
        } else if (strcmp(data, KEEPBOLT_MSSG) == 0) {
                cavernkeepstate = 0;
        } else if (strcmp(data, BWINDOWOPEN_MSSG) == 0) {
                bathroomwindowlocked = 1;
        } else if (strcmp(data, BWINDOWSHUT_MSSG) == 0) {
                bathroomwindowlocked = 0;
        } else if (strcmp(data, WINDOWOPEN_MSSG) == 0) {
                officewindowlocked = 1;
        } else if (strcmp(data, WINDOWSHUT_MSSG) == 0) {
                officewindowlocked = 0;}
     	if (sscanf(data, "K%d, S%d", & cavernkeepstate, & slidingstate)) {
			     HeartbeatCavern.stop();
				 HeartbeatCavern.start();

		}else if (sscanf(data, "O%d, B%d", & officewindowlocked, & bathroomwindowlocked)) {
			     HeartbeatOffice.stop();
				 HeartbeatOffice.start();
		}
}

void setZone() {
        Particle.syncTime();
        int month = Time.month();
        int day = Time.day();
        int weekday = Time.weekday();
        int previousSunday = day - weekday + 1;

        if (month < 3 || month > 11) {
                Time.zone(0);
        } else if (month > 3 && month < 11) {
                Time.zone(1);
        } else if (month == 3) {
                int offset = (previousSunday >= 8) ? 1 : 0;
                Time.zone(offset);
        } else {
                int offset = (previousSunday <= 0) ? 1 : 0;
                Time.zone(offset);
        }
}

void checkDoors() {
        if (cavernkeepstate == 1) {
        myDFPlayer.playMp3Folder(28);
        } else {
        myDFPlayer.playMp3Folder(27);
        }
}

void InitialPub() {
        Particle.publish(DEVICE_NAME, RSATUS_MSSG, 60, PRIVATE);
        alreadyRun = true;
}

int setSlidingState(String s) {
        slidingstate = atoi(s);
        return slidingstate;
}
int setBathState(String s) {
        bathroomwindowlocked = atoi(s);
        return bathroomwindowlocked;
}
int setOfficeState(String s) {
        officewindowlocked = atoi(s);
        return officewindowlocked;
}
int setMainState(String s) {
        maindoorlocked = atoi(s);
        return maindoorlocked;
}
int setCavernState(String s) {
        cavernkeepstate = atoi(s);
        return cavernkeepstate;
}
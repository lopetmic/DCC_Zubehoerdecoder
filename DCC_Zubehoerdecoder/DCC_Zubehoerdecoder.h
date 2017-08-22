/* Universeller DCC - Zubehördecoder
 * 2 Ausgänge / Zubehöradresse
 * Einstellbare Funktionalität:
 *  - Servo mit Umschaltrelais zur Weichenpolarisierung
 *  - Doppelspulenantriebe
 *  - statische/blinkende Ausgänge  
 *  - Einstellen der Servoendlagen per Drehencoder. 
 *    Der Drehencoder bezieht sich immer auf die zuletzt gestellte Weichenposition .
 *  - Die Betriebsmodi und Startverhalten wird über die Analogeingänge A4/A5 (parametrierbar) eingestellt. Dazu 
 *    müssen dort Pullups eingebaut werden. Jenachdem wieweit die Spannung  heruntergezogen wird werden
 *    die Modi eingestellt:
 *     A7:   5V (offen) normaler Betriebsmodus, kein PoM
 *           3,3V (Spannungsteiler 1:2) PoM immer aktiv, Adresse immer aus defaults
 *           1,6V (Spannungsteiler 2:1) 
 *           0V Programmiermodus / PoM ( 1. Empfamgenes Telegramm bestimmt Adresse )
 *     A6:   wird A4 auf 0 gezogen , wird der aktuell vom Drehencoder beeinflusste Servo in die  
 *           Mittellage gebracht. Sobald der Encoder wieder bewegt wird, bewegt sich das Servo wieder
 *           zur vorhergehenden Position.
 *           Ist A4 beim Programmstart auf 0, werden alle CV's auf die Defaults zurückgesetzt
 *                  
 * Eigenschaften:
 * Bis zu 8 (aufeinanderfolgende) Zubehöradressen ansteuerbar. Je nach verfügbaren Digitalausgängen 
 * sind ggfs auch mehr möglich.
 * 1. Adresse per Programmierung einstellbar
 * 
 *  Das Verhalten der konfigurierten Funktionen wird über CV-Programmierung festgelegt: 
 *  Bei Servoausgängen die Endlagen und die Geschwindigkeit
 *  bei Doppelspulenantrieben die Einschaltzeit der Spulen.
 *  bei blinkenden Ausgängen das Blinkverhalten ( in V3.0 noch nicht realisiert )
 *  
 *  Aufteilung der CV's:
 *  CV      Bedeutung    
 *  47      Kennung für Erstinitiierung, allgemeine Optionen die für den gesamten Decoder gelten
 *  48/49   Pom-Adresse
 *  50-54   Parameter für 1. Weichenadresse
 *  55-59   Parameter für 2. Weichenadresse
 *  ...
 *  Bedeutung der CV's bei den verschiedenen Funktione (CV-Nummern für 1. Weichenadresse)
 *  FSERVO Servo:
 *  CV50    Bit0 = 1: AutoOff der Servoimpulse bei Stillstand des Servo
 *  CV51    Position des Servo für Weichenstellung '0' ( in Grad, 0...180 )
 *  CV52    Position des Servo für Weichenstellung '1' ( in Grad, 0...180 )
 *  CV53    Geschwindigkeit des Servo
 *  CV54    aktuelle Weichenstellung ( nicht manuell verändern! )
 *  
 *  FCOIL Doppelspulenantrieb: ( derzeit nur mit automatischer Abschaltung )
 *  CV50    Bit0 = 1: Spulenausgang automatisch abschalten
 *               = 0: Spulenausgang über DCC-Befehl abschalten
 *  CV51    Einschaltdauer der Spule 1 ( in 10ms Einheiten )
 *  CV52    minimale Ausschaltdauer der Spule ( in 10ms Einheiten )
 *  CV53    -
 *  CV54    aktuelle Weichenstellung ( nicht manuell verändern! )
 *  
 *  FSTATIC statischer/Blinkender Ausgang 
 *  CV50    Bit0 = 1: Blinken,  0: statisch
 *          Bit1 = 1: Beim Blinken starten erst beide Leds dann Wechselblinken
 *          Bit2: mit weichem Auf/Abblenden 
 *  CV51    Einschaltzeit des Blinkens ( 10ms Einheiten )
 *  CV52    Ausschaltzeit des Blinkens ( 10ms Einheiten )
 *  CV53    1. Einschaltzeit beim Start des Blinkens
 *  CV54    aktueller Zusatnd ( nicht manuell verändern! )
 *  
 *  FSIGNAL2 / FSIGNAL3  Signaldecoder mit 2/3 Weichenadressen 
 *          bei den Folgeadressen ist als Typ FSIGNAL0 einzutragen
 *  CV50    Signalmodus / reserviert
 *  CV51    Bitmuster der Ausgänge für Zustand 000
 *  CV52    Bitmuster der Ausgänge für Zustand 001
 *  CV53    Überblendzeit in 10ms-Schritten
 *  CV55    Bitmuster hard/soft gibt an, welche Ausgänge 'hart' umschalten (Bit=1)
 *          und Welche Ausgänge weich überblenden (Bit=0)
 *  CV56    Bitmuster der Ausgänge für Zustand 010
 *  CV57    Bitmuster der Ausgänge für Zustand 011
 *  CV58    reserved
 *  die folgenden CV's sind nur relevant bei FSIGNAL3 (3 Adressen, 8 Zustände 6 Ausgänge)
 *  CV60     Bitmuster der Ausgänge für Zustand 100
 *  CV61     Bitmuster der Ausgänge für Zustand 101
 *  CV62     Bitmuster der Ausgänge für Zustand 110
 *  CV63     Bitmuster der Ausgänge für Zustand 111
*/
#define ENCODER_DOUBLE  // Eigenschaften des Drehencoders (Impulse per Raststellung)


// vom Anwender änderbare Parameter um den Zubehördecoder an die verwendete HW anzupassen

// Beispiel für Variante mit 4 Servos + 3 statischen Ausgängen, mit Betriebsmode Led an Pin 13 (interne Led)

//----------------------------------------------------------------
// Hardwareabhängige Konstante ( nicht per CV änderbar)
//----------------------------------------------------------------
// Eingänge analog: ( Bei Nano und Mini - Versionen kann hier auch A7 und A6 verwendet werden, um mehr
//                    digital nutzbare Ports freizubekommen.
//                    beim UNO sind A7+A6 nicht vorhanden! )
const byte betrModeP    =   A7;     // Analogeingang zur Bestimmung des Betriebsmodus. Wird nur beim
                                    // Programmstart eingelesen!
const byte resModeP     =   A6;     // Rücksetzen CV-Werte + Mittelstellung Servos

// Eingänge digital (die Ports A0-A5 lassen sich auch digital verwenden): ---------

// Drehencoder zur Servojustierung ...........
#define ENCODER_AKTIV       // Wird diese Zeile auskommentiert, wird der Encoder nicht verwendet. 
                            // Die Encoder-Ports werden dann ignoriert, und können anderweitig 
                            // verwendet werden.
const byte encode1P     =   A5;     // Eingang Drehencoder zur Justierung.
const byte encode2P     =   A4;
// ..................................................
// defines für die Konfigurationen der Decoder Module
#define DECMODULE_1
//#define DECMODULE_2
//#define DECMODULE_3
//#define DECMODULE_4
//#define DECEXAMPLE
// .................................................
//-------------------------------------------------------------------------------------------------------
// Betriebswerte ( per CV änderbar ) Diese Daten werden nur im Initiierungsmodus in die CV's geschrieben.
// Der Initiierungsmodus lässt sich per Mode-Eingang aktivieren oder er ist automatisch aktiv, wenn keine
// sinnvollen Werte im CV47 stehen.
//-------------------------------------------------------------------------------------------------------
// Konfiguration für Decoder des Moduls 1
#ifdef DECMODULE_1
const byte DccAddr          = 1;    // DCC-Decoderadresse
const byte iniMode          = 0x50 | AUTOADDR /*| ROCOADDR*/;  // default-Betriebsmodus ( CV47 )
const int  PomAddr          = 1;    // Adresse für die Pom-Programmierung ( CV48/49 )

// Ausgänge:  mit NC gekennzeichnete Ausgänge werden keinem Port zugeordnet. Damit können Ports gespart werden,
//            z.B. wenn bei einem Servo kein Polarisierungsrelais benötigt wird
const byte modePin      =   13;     // Anzeige Betriebszustand (Normal/Programmierung) (Led)
//                                2l/3L,       L1,       W1,    Gl.Sp,   Fb Wst, Fb hintn,       W2,  Fb vorn
const byte iniTyp[]     =   {   FSTATIC,  FSTATIC,   FSERVO,   FSERVO,    FCOIL,    FCOIL,   FSERVO,    FCOIL };
const byte out1Pins[]   =   {         3,        6,       A0,       A1,        7,       10,       A2,       12 };
const byte out2Pins[]   =   {        NC,       NC,        5,       NC,        8,        9,       A3,       11 };
const byte out3Pins[]   =   {        NC,       NC,       NC,       NC,       NC,       NC,       NC,       NC };

// Funktionsspezifische Parameter. Diese Parameter beginnen bei CV 50 und pro Funktionsausgang gibt es
// 5 CV-Werte. Die ersten 4 Werte steuern das Verhalten und in der folgenden Tabelle sind Erstinitiierungswerte
// für diese CV's enthalten. Der 5. Wert dient internen Zwecken und wird hier nicht initiiert
// In der Betriebsart 'INIMode' werden Mode und Parx Werte bei jedem Start aus der folgenden Tabelle übernommen
// Die Tabellenwerte müssen an die Typaufteilung ( iniTyp, s.o.) angepasst werden.
const byte iniFmode[]     = {         0,        0, SAUTOOFF, SAUTOOFF,  CDCCOFF,  CDCCOFF, SAUTOOFF,  CDCCOFF };
const byte iniPar1[]      = {         0,        0,       78,       45,        0,        0,      125,        0 };
const byte iniPar2[]      = {         0,        0,      155,      145,        0,        0,       65,        0 };
const byte iniPar3[]      = {         0,        0,        8,        8,        0,        0,        8,        0 };

#endif

//------------------------------------------------------------------------------------
// Konfiguration für Decoder des Moduls 2
#ifdef DECMODULE_2
const byte DccAddr          = 9;    // DCC-Decoderadresse
const byte iniMode          = 0x50 | AUTOADDR /*| ROCOADDR*/;  // default-Betriebsmodus ( CV47 )
const int  PomAddr          = 9;    // Adresse für die Pom-Programmierung ( CV48/49 )

// Ausgänge:  mit NC gekennzeichnete Ausgänge werden keinem Port zugeordnet. Damit können Ports gespart werden,
//            z.B. wenn bei einem Servo kein Polarisierungsrelais benötigt wird
const byte modePin      =   13;     // Anzeige Betriebszustand (Normal/Programmierung) (Led)
//                                2l/3L,       W3, Fb hintn, Fb mitte,  Fb vorn,       L2,  Srv Res,   Fb Res
const byte iniTyp[]     =   {   FSTATIC,   FSERVO,    FCOIL,    FCOIL,    FCOIL,  FSTATIC,   FSERVO,    FCOIL };
const byte out1Pins[]   =   {         3,       A0,       A3,       10,        9,       A2,       A1,        5 };
const byte out2Pins[]   =   {        NC,        7,       12,       11,        8,       NC,       NC,        6 };
const byte out3Pins[]   =   {        NC,       NC,       NC,       NC,       NC,       NC,       NC,       NC };

// Funktionsspezifische Parameter. Diese Parameter beginnen bei CV 50 und pro Funktionsausgang gibt es
// 5 CV-Werte. Die ersten 4 Werte steuern das Verhalten und in der folgenden Tabelle sind Erstinitiierungswerte
// für diese CV's enthalten. Der 5. Wert dient internen Zwecken und wird hier nicht initiiert
// In der Betriebsart 'INIMode' werden Mode und Parx Werte bei jedem Start aus der folgenden Tabelle übernommen
// Die Tabellenwerte müssen an die Typaufteilung ( iniTyp, s.o.) angepasst werden.
const byte iniFmode[]     = {         0, SAUTOOFF,  CDCCOFF,  CDCCOFF,  CDCCOFF,        0, SAUTOOFF,  CDCCOFF };
const byte iniPar1[]      = {         0,       76,        0,        0,        0,        0,       60,        0 };
const byte iniPar2[]      = {         0,       98,        0,        0,        0,        0,      120,        0 };
const byte iniPar3[]      = {         0,        8,        8,        8,        0,        0,        8,        0 };

#endif

//------------------------------------------------------------------------------------
// Konfiguration für Decoder des Moduls 3
#ifdef DECMODULE_3
const byte DccAddr          = 17;    // DCC-Decoderadresse
const byte iniMode          = 0x50 | AUTOADDR /*| ROCOADDR*/;  // default-Betriebsmodus ( CV47 )
const int  PomAddr          = 17;    // Adresse für die Pom-Programmierung ( CV48/49 )

// Ausgänge:  mit NC gekennzeichnete Ausgänge werden keinem Port zugeordnet. Damit können Ports gespart werden,
//            z.B. wenn bei einem Servo kein Polarisierungsrelais benötigt wird
const byte modePin      =   13;     // Anzeige Betriebszustand (Normal/Programmierung) (Led)
//                                2l/3L,       W4, Fb hintn, Fb mitte,  Fb vorn,       L3,  Srv Res,    L Res
const byte iniTyp[]     =   {   FSTATIC,   FSERVO,    FCOIL,    FCOIL,    FCOIL,  FSTATIC,   FSERVO,  FSTATIC };
const byte out1Pins[]   =   {         3,       A0,        7,       10,       11,       A2,       A1,        5 };
const byte out2Pins[]   =   {        NC,       A3,        8,        9,       12,       NC,       NC,       NC };
const byte out3Pins[]   =   {        NC,       NC,       NC,       NC,       NC,       NC,       NC,       NC };

// Funktionsspezifische Parameter. Diese Parameter beginnen bei CV 50 und pro Funktionsausgang gibt es
// 5 CV-Werte. Die ersten 4 Werte steuern das Verhalten und in der folgenden Tabelle sind Erstinitiierungswerte
// für diese CV's enthalten. Der 5. Wert dient internen Zwecken und wird hier nicht initiiert
// In der Betriebsart 'INIMode' werden Mode und Parx Werte bei jedem Start aus der folgenden Tabelle übernommen
// Die Tabellenwerte müssen an die Typaufteilung ( iniTyp, s.o.) angepasst werden.
const byte iniFmode[]     = {         0, SAUTOOFF,  CDCCOFF,  CDCCOFF,  CDCCOFF,        0, SAUTOOFF,        0 };
const byte iniPar1[]      = {         0,       70,        0,        0,        0,        0,       60,        0 };
const byte iniPar2[]      = {         0,      110,        0,        0,        0,        0,      120,        0 };
const byte iniPar3[]      = {         0,        8,        8,        8,        0,        0,        8,        0 };

#endif

//------------------------------------------------------------------------------------
// Konfiguration für Decoder des Moduls 4
#ifdef DECMODULE_4
const byte DccAddr          = 25;    // DCC-Decoderadresse
const byte iniMode          = 0x50 | AUTOADDR /*| ROCOADDR*/;  // default-Betriebsmodus ( CV47 )
const int  PomAddr          = 25;    // Adresse für die Pom-Programmierung ( CV48/49 )

// Ausgänge:  mit NC gekennzeichnete Ausgänge werden keinem Port zugeordnet. Damit können Ports gespart werden,
//            z.B. wenn bei einem Servo kein Polarisierungsrelais benötigt wird
const byte modePin      =   13;     // Anzeige Betriebszustand (Normal/Programmierung) (Led)
//                                2l/3L,       L1,       W1,    Gl.Sp,   Fb Wst, Fb hintn,       W2,  Fb vorn
const byte iniTyp[]     =   {   FSTATIC,  FSTATIC,   FSERVO,   FSERVO,    FCOIL,    FCOIL,   FSERVO,    FCOIL };
const byte out1Pins[]   =   {         3,        6,       A0,       A1,        7,        9,       A2,       11 };
const byte out2Pins[]   =   {        NC,       NC,        5,       NC,        8,       10,       A3,       12 };
const byte out3Pins[]   =   {        NC,       NC,       NC,       NC,       NC,       NC,       NC,       NC };

// Funktionsspezifische Parameter. Diese Parameter beginnen bei CV 50 und pro Funktionsausgang gibt es
// 5 CV-Werte. Die ersten 4 Werte steuern das Verhalten und in der folgenden Tabelle sind Erstinitiierungswerte
// für diese CV's enthalten. Der 5. Wert dient internen Zwecken und wird hier nicht initiiert
// In der Betriebsart 'INIMode' werden Mode und Parx Werte bei jedem Start aus der folgenden Tabelle übernommen
// Die Tabellenwerte müssen an die Typaufteilung ( iniTyp, s.o.) angepasst werden.
const byte iniFmode[]     = {         0,        0, SAUTOOFF, SAUTOOFF,  CDCCOFF,  CDCCOFF, SAUTOOFF,  CDCCOFF };
const byte iniPar1[]      = {         0,        0,       60,       60,        0,        0,       60,        0 };
const byte iniPar2[]      = {         0,        0,      120,      120,        0,        0,      120,        0 };
const byte iniPar3[]      = {         0,        0,        8,       8 ,        0,        0,        8,        0 };

#endif

//------------------------------------------------------------------------------------
// Konfiguration für Beispieldecoder
#ifdef DECEXAMPLE
const byte DccAddr          = 1;    // DCC-Decoderadresse
const byte iniMode          = 0x50 | AUTOADDR /*| ROCOADDR*/;  // default-Betriebsmodus ( CV47 )
const int  PomAddr          = 50;    // Adresse für die Pom-Programmierung ( CV48/49 )

// Ausgänge:  mit NC gekennzeichnete Ausgänge werden keinem Port zugeordnet. Damit können Ports gespart werden,
//            z.B. wenn bei einem Servo kein Polarisierungsrelais benötigt wird
const byte modePin      =   13;     // Anzeige Betriebszustand (Normal/Programmierung) (Led)
const byte iniTyp[]     =   {   FCOIL,   FSIGNAL2, FSIGNAL0,   FSERVO,   FSERVO,    FSTATIC };
const byte out1Pins[]   =   {      A2,          9,       12,       A0,       A1,          5 };  // output-pins der Funktionen
const byte out2Pins[]   =   {      A3,         10,       NC,        7,        3,          6 };
const byte out3Pins[]   =   {      NC,         11,       NC,        8,       NC,         NC };

// Funktionsspezifische Parameter. Diese Parameter beginnen bei CV 50 und pro Funktionsausgang gibt es
// 5 CV-Werte. Die ersten 4 Werte steuern das Verhalten und in der folgenden Tabelle sind Erstinitiierungswerte
// für diese CV's enthalten. Der 5. Wert dient internen Zwecken und wird hier nicht initiiert
// In der Betriebsart 'INIMode' werden Mode und Parx Werte bei jedem Start aus der folgenden Tabelle übernommen
// Die Tabellenwerte müssen an die Typaufteilung ( iniTyp, s.o.) angepasst werden.
const byte iniFmode[]     = {        0,         0,0b11110000,        0,        0,  BLKMODE|BLKSOFT };
const byte iniPar1[]      = {        0, 0b0000010,0b00000100,        0,        0,               50 };
const byte iniPar2[]      = {        0, 0b0000001,0b00001001,      180,      180,               50 };
const byte iniPar3[]      = {        0,        50,         8,        8,        8,              100 };

#endif
//------------------------------------------------------------------------------------

/* die folgenden Werte dienen als Beispiele für sinnvolle Einträge in der obigen Paramtertabelle. 
// Sie werden im Programm nicht direkt verwendet!
// Standardwerte für Servoausgang 
const byte iniServoMode     = SAUTOOFF;     // = (Mode) automatische Pulsabschaltung
const byte iniServoGerade   = 0;     // = Par1;
const byte iniServoAbzw     = 180;   // = Par2;
const byte inispeed = 8;             // = Par3;

// Standardwerte für Puls-Ausgang (Doppelspule)
const byte iniCoilMode     = CAUTOOFF;     // = (Mode) automatische Pulsbegrenzung eingeschaltet (0=AUS)
const byte iniCoilOn       = 50;    // = (Par1) 500ms Impuls
const byte iniCoilOff      = 20;    // = (Par2) mindestens 2Sec Pause zwischen 2 Pulsen

// Standardwerte für statisch/Blinkende Ausgänge
// sind 2 Ausgänge definiert (out1Pins, out2Pins), arbeiten sie im Gegentakt.
const byte iniStaticMode    = BLINKMODE; // Bit0: Blinken/Statisch
                                         // Bit1: Beim Blinken starten erst beide Leds dann Wechselblinken
                                         // Bit2: mit weichem Auf/Abblenden (Pins müssen PWM-fähig sein)
const byte iniBlinkOn       = 100;   // = (Par1) 1sec ein
const byte iniBlinkOff      = 50 ;   // = (Par2) 0,5sec aus
*/

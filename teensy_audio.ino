#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>
#include <LiquidCrystal.h>

#define CONTROL_DELAY 10

#define EFFECT1_BUTTON 31 // activate switch for synth
#define EFFECT2_BUTTON 32 // activate switch for effect2
#define CONTROL1_A A14 // pin 33
#define CONTROL1_B A15 // pin 34
#define CONTROL2_A A17 // pin 36
#define CONTROL2_B A18 // pin 37

// cycle through options for effects
#define CYCLE_BUTTON1_F 27 
#define CYCLE_BUTTON1_B 28
#define CYCLE_BUTTON2_F 29
#define CYCLE_BUTTON2_B 30

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=84,194
AudioSynthWaveform       waveform1;      //xy=86,252
AudioMixer4              mixer1;         //xy=246,236
AudioAnalyzePeak         peak1;          //xy=306,40
AudioAnalyzeNoteFrequency notefreq1;      //xy=308,80
AudioEffectBitcrusher    bitcrusher1;    //xy=412,210
AudioEffectWaveshaper    waveshape1;     //xy=414,264
AudioMixer4              mixer2;         //xy=576,236
AudioOutputI2S           i2s2;           //xy=738,242
AudioConnection          patchCord1(i2s1, 1, peak1, 0);
AudioConnection          patchCord2(i2s1, 1, notefreq1, 0);
AudioConnection          patchCord3(i2s1, 1, mixer1, 0);
AudioConnection          patchCord4(waveform1, 0, mixer1, 1);
AudioConnection          patchCord5(mixer1, bitcrusher1);
AudioConnection          patchCord6(mixer1, waveshape1);
AudioConnection          patchCord7(bitcrusher1, 0, mixer2, 0);
AudioConnection          patchCord8(waveshape1, 0, mixer2, 1);
AudioConnection          patchCord9(mixer2, 0, i2s2, 0);
AudioConnection          patchCord11(mixer2, 0, i2s2, 1);
AudioConnection          patchCord10(mixer1, 0, mixer2, 2);
AudioControlSGTL5000     sgtl5000_1;     //xy=494,52
// GUItool: end automatically generated code

//Change this to reflect whatever pins are actually going to be used
LiquidCrystal lcd(0, 1, 2, 3, 4, 5);


bool e1active = false; // the arduino IDE makes this look like an L, but this is actually e-one-active

// WAVESHAPE_REF is used to restore WAVESHAPE when turning distortion off or turning it down
const float WAVESHAPE_REF[17] = {  -1.0,  -0.875,  -0.75,  -0.625,  -0.5, 
                                    -0.375, -0.25,  -0.125,  0,  0.125,  0.25,
                                    0.375,  0.5, 0.625,  0.75,  0.875,  1.0};

float WAVESHAPE[17] = {  -1.0,  -0.875,  -0.75,  -0.625,  -0.5, 
                        -0.375, -0.25,  -0.125,  0,  0.125,  0.25,
                        0.375,  0.5, 0.625,  0.75,  0.875,  1.0};                       


// synth variables                        
float amp = 0.5;
float freq = 41.0; // default to E1 (open E string on a bass)
int currentWaveform = WAVEFORM_SQUARE;


// variables for controlling effect #2
// 0 = off
// 1 = overdrive
// 2 = bitcrusher
int effect2 = 0;
int prevEffect2 = 1;
int controlTimer = 0; // used to delay how often controls are checked

// variable used for bitcrusher
float bitsCrushed;
float bitsSampleRate;

Bounce cycle1F = Bounce(CYCLE_BUTTON1_F, 5);
Bounce cycle1B = Bounce(CYCLE_BUTTON1_B, 5);
Bounce cycle2F = Bounce(CYCLE_BUTTON2_F, 5);
Bounce cycle2B = Bounce(CYCLE_BUTTON2_B, 5);

Bounce e1Switch = Bounce(EFFECT1_BUTTON, 5);
Bounce e2Switch = Bounce(EFFECT2_BUTTON, 5);

void bitcrusher(int val1, int val2);

void setup() {

    lcd.begin(16, 2);
    lcd.print("Synth:          ");
    lcd.setCursor(0,1);
    lcd.print("E2:             ");

    // audio connections require memory
    AudioMemory(31);

    // setup Audio Adapter
    sgtl5000_1.enable();
    sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
    sgtl5000_1.lineInLevel(13);
    sgtl5000_1.volume(0.5); // 0.5 is plenty loud for headphones, may need to adjust for amp

    // setup inputs
    pinMode(CYCLE_BUTTON1_F, INPUT_PULLUP);
    pinMode(CYCLE_BUTTON2_F, INPUT_PULLUP);
    pinMode(CYCLE_BUTTON1_B, INPUT_PULLUP);
    pinMode(CYCLE_BUTTON2_B, INPUT_PULLUP);
    pinMode(EFFECT1_BUTTON, INPUT);
    pinMode(EFFECT2_BUTTON, INPUT);

    // setup synthesizer
    waveform1.amplitude(0.5);
    currentWaveform = WAVEFORM_SQUARE;

    // setup frequency detector
    notefreq1.begin(0.15);

    Serial.begin(9600);

}

void loop() {

    // check memory usage
    //Serial.print(AudioMemoryUsageMax());
    //Serial.print(" || ");
    //Serial.println(AudioMemoryUsage());


    // update cycle buttons and corresponding variables
    cycle1F.update();
    cycle1B.update();
    cycle2F.update();
    cycle2B.update();
    e1Switch.update();
    e2Switch.update();


// CYCLE OPTIONS
// This code handles cycle button presses to change parameters for blocks.
// Currently, this includes the waveform type for the synth and the type off
// effect for effect2.
//-----------------------------------------------------------------------------

    if (cycle1F.fallingEdge() && e1active) {
        switch (currentWaveform) {
            case WAVEFORM_SQUARE:
                currentWaveform = WAVEFORM_SAWTOOTH;
                lcd.setCursor(0,0);
                lcd.print("Synth: SAW      ");
                break;
            case WAVEFORM_SAWTOOTH:
                currentWaveform = WAVEFORM_TRIANGLE;
                lcd.setCursor(0,0);
                lcd.print("Synth: TRI      ");
                break;
            case WAVEFORM_TRIANGLE:
                currentWaveform = WAVEFORM_SINE;
                lcd.setCursor(0,0);
                lcd.print("Synth: SIN      ");
                break;
            case WAVEFORM_SINE:
                currentWaveform = WAVEFORM_SQUARE;
                lcd.setCursor(0,0);
                lcd.print("Synth: SQR      ");
                break;
            default:
                currentWaveform = WAVEFORM_SAWTOOTH;
        }
        //AudioNoInterrupts();
        waveform1.begin(currentWaveform);
        //AudioInterrupts();
    }
    else if (cycle1B.fallingEdge() && e1active) {
        switch (currentWaveform) {
            case WAVEFORM_SQUARE:
                currentWaveform = WAVEFORM_SINE;
                lcd.setCursor(0,0);
                lcd.print("Synth: SIN      ");
                break;
            case WAVEFORM_SAWTOOTH:
                currentWaveform = WAVEFORM_SQUARE;
                lcd.setCursor(0,0);
                lcd.print("Synth: SQR      ");
                break;
            case WAVEFORM_TRIANGLE:
                currentWaveform = WAVEFORM_SAWTOOTH;
                lcd.setCursor(0,0);
                lcd.print("Synth: SAW      ");
                break;
            case WAVEFORM_SINE:
                currentWaveform = WAVEFORM_TRIANGLE;
                lcd.setCursor(0,0);
                lcd.print("Synth: TRI      ");
                break;
            default:
                currentWaveform = WAVEFORM_SAWTOOTH;
                lcd.setCursor(0,0);
                lcd.print("Synth: SAW      ");
        }
        //AudioNoInterrupts();
        waveform1.begin(currentWaveform);
        //AudioInterrupts();
    }
    if (cycle2F.fallingEdge()) {
        //Serial.println("cycle2F pressed");
        switch (effect2) {
            case 1: // bitcrusher
                effect2 = 2;
                mixer2.gain(0, 1.0);
                mixer2.gain(1, 0);
                mixer2.gain(2, 0);
                lcd.setCursor(0,1);
                lcd.print("E2: Bitcrusher  ");
                break;
            case 2:
                //Serial.println("2F pressed - overdrive");
                effect2 = 1;
                mixer2.gain(0, 0);
                mixer2.gain(1, 1.0);
                mixer2.gain(2, 0);
                lcd.setCursor(0,1);
                lcd.print("E2: Overdrive   ");
                break;
            default:
                effect2 = 1;
        }
    }
    else if (cycle2B.fallingEdge()) {
        switch (effect2) {
            case 1:
                effect2 = 2;
                mixer2.gain(0, 1.0);
                mixer2.gain(1, 0);
                mixer2.gain(2, 0);
                lcd.setCursor(0,1);
                lcd.print("E2: Bitcrusher  ");
                break;
            case 2:
                effect2 = 1;
                mixer2.gain(0, 0);
                mixer2.gain(1, 1.0);
                mixer2.gain(2, 0);
                lcd.setCursor(0,1);
                lcd.print("E2: Overdrive   ");
                break;
            default:
                effect2 = 1;                
        }
    }

// END CYCLE OPTIONS
//-----------------------------------------------------------------------------


// STOMP SWITCHES
// This section handles the stomp switches, which activate/deactivate the 2
// stages of the pedal.
//-----------------------------------------------------------------------------

    if (e1Switch.fallingEdge()) {
        e1active = false;
        mixer1.gain(0, 1.0);
        mixer1.gain(1, 0.0);
        lcd.setCursor(0,0);
        lcd.print("Synth: Off      ");
    }    
    else if (e1Switch.risingEdge()){
        e1active = true;
        // pass synth signal on, block original signal
        waveform1.amplitude(0);
        mixer1.gain(0, 0);
        mixer1.gain(1, 1.0);
        waveform1.begin(currentWaveform);
        lcd.setCursor(0,0);
        lcd.print("Synth:          ");
    }

    if (e2Switch.fallingEdge()) {
        //Serial.println("effect 2: off");
        prevEffect2 = effect2;
        effect2 = 0;
        mixer2.gain(0, 0);
        mixer2.gain(1, 0);
        mixer2.gain(2, 1.0);
        lcd.setCursor(0, 1);
        lcd.print("E2: Off         ");
    }
    else if (e2Switch.risingEdge()){
        effect2 = prevEffect2;
        switch (effect2) {
            case 1:
                // send overdrive signal to final mixer
                //Serial.println("Passing overdrive");
                effect2 = 1;
                mixer2.gain(0, 0);
                mixer2.gain(1, 1.0);
                mixer2.gain(2, 0);
        
                lcd.setCursor(0,1);
                lcd.print("E2: Overdrive   ");
                break;
            case 2:
                // send bitcrusher signal to final mixer
                //Serial.println("Passing bitcrusher");
                mixer2.gain(0, 1.0);
                mixer2.gain(1, 0);
                mixer2.gain(2, 0);
        
                lcd.setCursor(0,1);
                lcd.print("E2: Bitcrusher  ");
                break;
            default:
                // send overdrive signal to final mixer
                //Serial.print("switch-case defaulted, passing overdrive");
                effect2 = 1;
                mixer2.gain(0, 0);
                mixer2.gain(1, 1.0);
                mixer2.gain(2, 0);
        
                lcd.setCursor(0,1);
                lcd.print("E2: Overdrive   ");
        }
    }

// END STOMP SWITCHES
//-----------------------------------------------------------------------------


// SYNTHESIZER CODE
// This code is to update the synthesizer's frequency and amplitude
//-----------------------------------------------------------------------------
    
    // if synth is active, synthesize a signal and pass it on
    if (e1active) {
        //Serial.print("entered synth loop || ");
        // frequency detection
        //if (notefreq1.available()) {
            freq = notefreq1.read();
            //waveform1.frequency(freq);
        //}

        //amplitude detection
        if (peak1.available()) {
            amp = peak1.read();
            //waveform1.amplitude(amp);
        }

        // adjust waveform to match input signal
        //AudioNoInterrupts();
        waveform1.frequency(freq);
        waveform1.amplitude(amp);
        //AudioInterrupts();
        
        
    }

// END SYNTHSIZER CODE
// ----------------------------------------------------------------------------

// EFFECT 2 CODE
// This code read the value of effect2, and applies the appropriate affect
//-----------------------------------------------------------------------------

    //if bitcrusher 
    if (effect2 == 2) {
        
        if ( controlTimer < CONTROL_DELAY) {
            controlTimer++;
            delay(10);
        }
        else {
            controlTimer = 0;
            bitsCrushed = (float)analogRead(CONTROL2_A);              //getting the bit that will be changed 
            bitsSampleRate = (float)analogRead (CONTROL2_B);          //getting the sample that will be changed        
            bitcrusher(bitsCrushed, bitsSampleRate);
        }
        
    }
    
    // else if overdrive
    else if (effect2 == 1) {
           
        if (controlTimer < CONTROL_DELAY) {
            controlTimer++;
            delay(10);
        }
        else {
            controlTimer = 0;
            //Serial.println("overdrive loop entered");
            float thresh = map(analogRead(CONTROL2_A), 0, 1023, 1023, 0);
            thresh = thresh / 1023;
            //Serial.print(thresh);
            //Serial.print(" || ");
            for (int i = 0; i < 8; i++) {
                //Serial.print(abs(WAVESHAPE[i]));
                if (abs(WAVESHAPE[i]) > thresh) {
                    WAVESHAPE[i] = -thresh;
                }
                else {
                    WAVESHAPE[i] = WAVESHAPE_REF[i];
                }
            }
            for (int i = 9; i < 17; i++) {
                if (WAVESHAPE[i] > thresh) {
                    WAVESHAPE[i] = thresh;
                }
                else {
                    WAVESHAPE[i] = WAVESHAPE_REF[i];
                }
            }

            AudioNoInterrupts();
            waveshape1.shape(WAVESHAPE, 17);
            AudioInterrupts();
        }
    }



// END EFFECT 2 CODE
//-----------------------------------------------------------------------------

    //Serial.print(e1active);
    //Serial.print(" || ");
    //Serial.print(currentWaveform);
    //Serial.print(" || ");
    //Serial.print(effect2);
    //Serial.print(" || ");
    //Serial.print(freq);
    //Serial.print(" || ");
    //Serial.print(amp);
    //Serial.print(" || ");
    //Serial.print(AudioMemoryUsageMax());
    //Serial.println();
}



void bitcrusher(int val1, int val2) {
    // call this function after the following code has been run
        /*
        bitsCrushed = (float)analogRead(CONTROL2_A);       //getting the bit that will be changed 
        bitsSampleRate = (float)analogRead (CONTROL2_B);   //getting the sample that will be changed        
        distortion(bitsCrushed, bitsSampleRate);
        */
    
    val1 = map(val1, 0, 1023, 16, 4);
    //float expx = 0.125* pow(val1,2);
    //expx = constrain(expx, 1, 16);
    //bitsCrushed = expx;
    //bitsCrushed = val1;

    val2 = map(val2, 0, 1023, 44100, 1000);

    //serial monitor for troubleshooting
    
    bitcrusher1.bits(val1);
    bitcrusher1.sampleRate(val2);

}

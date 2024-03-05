//#############################################################################
//                                                                            #
//   FILE:   potentiometer_control_pwm_duty.c                                 #
//                                                                            #
//   CODES OF THE STUDY                                                       #
//                                                                            #
//   STUDY NAME : PWM duty cycle configuration using potentiometer            #
//   MAIN GOAL  : ADC control & PWM generation and adjustments                #
//   CONTACT    : tturac.turk@gazi.edu.tr  &  linkedin.com/in/talhaturacturk  #
//                                                                            #
//   This work is an empty project setup for Driverlib development.           #
//                                                                            #
//   Watch Expression                                                         #
//   adcAResult0 = Digital display of the voltage at pin A0                   #
//                                                                            #
//   Configure desired EPWM frequency & duty                                  #
//   ePWM2A is on GPIO2                                                       #
//   ePWM2B is on GPIO3                                                       #
//                                                                            #
//#############################################################################

#include "driverlib.h"
#include "device.h"
#include "f28002x_device.h"
#include "f28002x_sysctrl.h"
#include "f28002x_epwm_defines.h"

uint32_t  freq     = 10000.0;    // Desired PWM Frequency
uint16_t  per1     = 5000;       // CMP Period Val.
uint16_t  deadband = 50.0;       // deadband Val.
uint16_t  per2;
uint16_t  adcAResult0;
float32_t dutyRatio;



void    PWM_Setup();
void    GPIO_Setup();
void    ADC_init();
void    ASYSCTL_init();
void    INTERRUPT_init();
void    PinMux_init();
void    TriggerEPWM_init();
void    change_duty_ratio();
extern __interrupt void adcA1ISR(void);

// Main
void main(void)
{

#ifdef _FLASH
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
#endif

        Device_init();                  // Initialize device clock and peripherals
        Device_initGPIO();              // Disable pin locks and enable internal pull-ups.
        Interrupt_initModule();         // Initialize PIE and clear PIE registers. Disables CPU interrupts.
                                        // Initialize the PIE vector table with pointers to the shell Interrupt
        Interrupt_initVectorTable();    // Service Routines (ISR).



        DINT;                           //  -> Disable all interrupt
        IER = 0x0000;                   //  -> Interrupt Enable Register  -> contains enable bits for all the maskable CPU interrupt levels
        IFR = 0x0000;                   //  -> Interrupt Flag Register    -> used for identify and clear pending interrupt

        InitSysCtrl();
        Device_init();
        InitGpio();                     //  -> Disable pin locks and enable internal pullups.
        InitPieCtrl();                  //  -> Basic setup of PIE table
        InitPieVectTable();             //  -> default ISR's int PIE
        InitCpuTimers();

                                        // Enable global Interrupts and higher priority real-time debug events:
        EINT;                           // Enable Global interrupt INTM
        ERTM;                           // Enable Global realtime interrupt DBGM

        EALLOW;
        PinMux_init();
        GPIO_Setup();
        ASYSCTL_init();
        ADC_init();
        INTERRUPT_init();
        PWM_Setup();
        TriggerEPWM_init();
        EDIS;

        // IDLE loop. Just sit and loop forever (optional):
        while(1)
        {
            change_duty_ratio();
        }
}


void change_duty_ratio(void)
{
    per2 = (dutyRatio * per1);
    EPwm2Regs.CMPA.bit.CMPA = per2;     // Set duty as dutyRatio
    EPwm2Regs.CMPB.bit.CMPB = per2;     // Set duty as dutyRatio
}


// Function to configure ePWM1 to generate the SOC. - Configure EPWM1 ADC SOCA trigger
void TriggerEPWM_init(void)
{
    EPWM_disableADCTrigger(EPWM1_BASE, EPWM_SOC_A);                            // Disable SOCA

    EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPA);   // Configure the SOC to occur on the first up-count event
    EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 1);

    // Set the compare A value to 1000 and the period to 1999
    // Assuming ePWM clock is 100MHz, this would give 50kHz sampling
    // 50MHz ePWM clock would give 25kHz sampling, etc.
    // The sample rate can also be modulated by changing the ePWM period
    // directly (ensure that the compare A value is less than the period).

    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, 1000);
    EPWM_setTimeBasePeriod(EPWM1_BASE, 1999);

    EPWM_setClockPrescaler(EPWM1_BASE,                                         // Set the local ePWM module clock divider to /1
                           EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_1);

    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_STOP_FREEZE);    // Freeze the counter

    // Start ePWM1, enabling SOCA and putting the counter in up-count mode
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP);
}


// ADC A Interrupt 1 ISR
__interrupt void adcA1ISR(void)
{
    // Store results
      adcAResult0 = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
    //adcAResult1 = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1);   CAN BE USED IF MORE INPUT IS REQUIRED
    //adcAResult2 = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);   CAN BE USED IF MORE INPUT IS REQUIRED

    dutyRatio = adcAResult0 / 4096.0;                                         // Duty Cycle Rate

    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);                     // Clear the interrupt flag

    if(true == ADC_getInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1))    // Check if overflow has occurred
    {
        ADC_clearInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    }

    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);     // Acknowledge the interrupt
}


// ADCA initialization
void ADC_init()
{                                                                           // ADC Initialization: Write ADC configurations and power up the ADC
    ADC_setOffsetTrimAll(ADC_REFERENCE_INTERNAL,ADC_REFERENCE_3_3V);        // Configures the ADC module's offset trim
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_2_0);                           // Configures the analog-to-digital converter module prescaler.
    ADC_setInterruptPulseMode(ADCA_BASE, ADC_PULSE_END_OF_CONV);            // Sets the timing of the end-of-conversion pulse
    ADC_enableConverter(ADCA_BASE);                                         // Powers up the analog-to-digital converter core.
    DEVICE_DELAY_US(5000);                                                  // Delay for 1ms to allow ADC time to power up

                                                                            // SOC Configuration: Setup ADC EPWM channel and trigger settings
    ADC_disableBurstMode(ADCA_BASE);                                        // Disables SOC burst mode.
    ADC_setSOCPriority(ADCA_BASE, ADC_PRI_ALL_ROUND_ROBIN);                 // Sets the priority mode of the SOCs.


    // Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.     // Start of Conversion 0 Configuration
                                                                                            // SOC number      : 0
                                                                                            // Trigger         : ADC_TRIGGER_EPWM1_SOCA
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN0, 8U);    // Channel         : ADC_CH_ADCIN0
    ADC_setInterruptSOCTrigger(ADCA_BASE, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE);       // Sample Window   : 8 SYSCLK cycles
                                                                                            //  Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE

    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);                    //  ADC Interrupt 1 Configuration
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);                                        //  SOC/EOC number  : 0
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);                                   //  Interrupt Source: enabled
    ADC_disableContinuousMode(ADCA_BASE, ADC_INT_NUMBER1);                                  //  Continuous Mode : disabled
}


// asysctl initialization
void ASYSCTL_init()
{
    ASysCtl_disableTemperatureSensor();                                         // Disables the temperature sensor output to the ADC.
    ASysCtl_setAnalogReferenceInternal( ASYSCTL_VREFHIA | ASYSCTL_VREFHIC );    // Set the analog voltage reference selection to internal.
    ASysCtl_setAnalogReference1P65( ASYSCTL_VREFHIA | ASYSCTL_VREFHIC );        // Set the internal analog voltage reference selection to 1.65V.
}


// Interrupt Settings for INT_ADCA1
void INTERRUPT_init()
{
    Interrupt_register(INT_ADCA1, &adcA1ISR);
    Interrupt_enable(INT_ADCA1);
}


//myEPWM1 initialization
void PWM_Setup(){

     EALLOW;
     EPwm2Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;                  // Set immediate load
     EPwm2Regs.TBPRD = per1;                                    // PWM frequency = 1 / period      1/(TBPRD+1)*TBCLK)

     EPwm2Regs.TBPHS.bit.TBPHS = 0;
     EPwm2Regs.TBCTR = 0;                                       // Time base counter =0
     EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;             // Count-up mode: used for asymmetric PWM
     //EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;                   // Disable phase loading
     EPwm2Regs.TBCTL.bit.SWFSYNC = 1;                           // sync at "0"
     EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;                   // Set the clock rate
     EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;                      // Set the clock rate
     EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_PRD;               // Load on CTR=PRD
     EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_PRD;               // Load on CTR=PRD
     EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;                // Shadow mode. Operates as a double buffer.
     EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;                // Shadow mode. Operates as a double buffer.

     EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;                         // Clear PWM1A on event A, up count
     EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR;                       // Set PWM1A on event A, down count
     EPwm2Regs.AQCTLA.bit.CBU = AQ_NO_ACTION;
     EPwm2Regs.AQCTLA.bit.CBD = AQ_NO_ACTION;
     EPwm2Regs.AQCTLA.bit.ZRO = AQ_NO_ACTION;
     EPwm2Regs.AQCTLA.bit.PRD = AQ_NO_ACTION;

     EPwm2Regs.AQCTLB.bit.CBU = AQ_CLEAR;                       // Set PWM1B on event B, up count
     EPwm2Regs.AQCTLB.bit.CBD = AQ_SET;                         // Clear PWM1B on event B, down count
     EPwm2Regs.AQCTLB.bit.CAU = AQ_NO_ACTION;
     EPwm2Regs.AQCTLB.bit.CAD = AQ_NO_ACTION;
     EPwm2Regs.AQCTLB.bit.PRD = AQ_NO_ACTION;
     EPwm2Regs.AQCTLB.bit.ZRO = AQ_NO_ACTION;

     EPwm2Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;                  //ET_CTR_ZERO;

     EPwm2Regs.DBCTL.bit.IN_MODE = DBA_ALL;                     // EPWMxA is the source for both delays
     EPwm2Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;             // Enable Dead-band module
     EPwm2Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;                  // Active High Complementary (AHC)
     EPwm2Regs.DBRED.bit.DBRED = deadband;                      // RED = 30 TBCLKs initially eskiden 30 du
     EPwm2Regs.DBFED.bit.DBFED = deadband;                      // FED = 30 TBCLKs initially

     EDIS;
}

void GPIO_Setup(void)
{
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1 ; //ePWM2A
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1 ; //ePWM2B

    //GPIO_setPinConfig(GPIO_2_EPWM2_A);
    //GPIO_setPinConfig(GPIO_3_EPWM2_B);
    EDIS;
}


// ANALOG -> Pinmux
void PinMux_init()
{
    // Analog PinMux for A0/C15
    GPIO_setPinConfig(GPIO_231_GPIO231);
    // AIO -> Analog mode selected
    GPIO_setAnalogMode(231, GPIO_ANALOG_ENABLED);
    // Analog PinMux for A1
    GPIO_setPinConfig(GPIO_232_GPIO232);
    // AIO -> Analog mode selected
    GPIO_setAnalogMode(232, GPIO_ANALOG_ENABLED);
}

// End of file

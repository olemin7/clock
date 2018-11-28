#include "IRremoteESP8266_nec.h"
#include "IRremoteInt.h"

int MATCH(int measured, int desired) {return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);}
int MATCH_MARK(int measured_ticks, int desired_us) {return MATCH(measured_ticks, (desired_us + MARK_EXCESS));}
int MATCH_SPACE(int measured_ticks, int desired_us) {return MATCH(measured_ticks, (desired_us - MARK_EXCESS));}

//IRRecv------------------------------------------------------

extern "C" {
	#include "user_interface.h"
	#include "gpio.h"
}

static ETSTimer timer;
volatile irparams_t irparams;

static void ICACHE_FLASH_ATTR read_timeout(void *arg) {
    os_intr_lock();
	irparams.rcvstate = STATE_IDLE;
	os_intr_unlock();
}

static void ICACHE_FLASH_ATTR gpio_intr(void *arg) {
    uint32_t gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

    static uint32_t start = 0;
    uint32_t now = system_get_time();
    uint32_t delta=(now - start) / USECPERTICK + 1;
    switch(irparams.rcvstate){
    case STATE_IDLE: //check if val =1
    	if(0==(gpio_status&(1<<irparams.recvpin)))goto l_Error;
    	irparams.rcvstate = STATE_MARK;
    	break;
    case STATE_MARK:
//l_Error://check if new cmd come
       	if (MATCH_MARK(delta, NEC_HDR_MARK)){ // Initial mark
   	   	irparams.rcvstate = STATE_SPACE;
   	    break;
    	}
    l_Error:
		irparams.rcvstate = STATE_IDLE;
		os_timer_disarm(&timer);
		return;
    case STATE_SPACE:
    	 // Initial space
    	  if (MATCH_SPACE(delta, NEC_HDR_SPACE)) {
    		  irparams.rcvstate = STATE_DATA_MARK;
    		  irparams.BitExpected=NEC_BITS;
    		  irparams.isRepeat=false;
    		  break;
    	  }
    	  if (MATCH_SPACE(delta, NEC_RPT_SPACE)) {
    		  irparams.rcvstate = STATE_REPEAT;
    		  break;
    	  }
    	  goto l_Error;
    case STATE_REPEAT:
  	  if (MATCH_SPACE(delta, NEC_BIT_MARK)) {
  		  //repeat previous command need check time out
  		  if((irparams.Rpt_TimeOut-now)>((NEC_RPT_TIMEOUT-1)*USECPERTICK))return;
  		  irparams.isRepeat=true;
  		  irparams.buffer=irparams.PrevValue;
  		  goto l_CMD_RETURN;
  	  }
  	  goto l_Error;
    case STATE_DATA_MARK:
    	if (MATCH_SPACE(delta, NEC_BIT_MARK)) {
    		irparams.rcvstate = STATE_DATA;
    		break;
    	}
    	goto l_Error;
    case STATE_DATA:
    	irparams.rcvstate = STATE_DATA_MARK;
    	irparams.buffer<<=1;
    	irparams.BitExpected--;
    	if(0==irparams.BitExpected)irparams.rcvstate = STATE_END_OF_MSG;
    	if (MATCH_SPACE(delta, NEC_ONE_SPACE)) {
    		irparams.buffer|=1;
    	    break;
    	   }
    	if (MATCH_SPACE(delta, NEC_ZERO_SPACE)) {
    	    break;
    	   }
    	goto l_Error;
    case STATE_END_OF_MSG:
     	if (MATCH_SPACE(delta, NEC_BIT_MARK)) {
    l_CMD_RETURN:
     			if(false==irparams.isValueReady)// skip if previous was not readet
     				irparams.Value=irparams.buffer;
        		irparams.rcvstate = STATE_IDLE;
        		irparams.isValueReady=true;
        		irparams.Rpt_TimeOut=now;
        		irparams.PrevValue=irparams.buffer;
        		return;
     	}
     	goto l_Error;
    }

    start = now;

    os_timer_disarm(&timer);
    os_timer_arm(&timer, 15, 0);
}

IRrecv::IRrecv(int recvpin) {
  irparams.recvpin = recvpin;
}

// initialization
void IRrecv::enableIRIn() {
	
  // initialize state machine variables
  irparams.rcvstate = STATE_IDLE;

  // set pin modes  
  //PIN_FUNC_SELECT(IR_IN_MUX, IR_IN_FUNC);
  GPIO_DIS_OUTPUT(irparams.recvpin);
  
  // Initialize timer
  os_timer_disarm(&timer);
  os_timer_setfn(&timer, (os_timer_func_t *)read_timeout, &timer);

  // ESP Attach Interrupt
  ETS_GPIO_INTR_DISABLE();
  ETS_GPIO_INTR_ATTACH(gpio_intr, NULL);
  gpio_pin_intr_state_set(GPIO_ID_PIN(irparams.recvpin), GPIO_PIN_INTR_ANYEDGE);

  //gpio_pin_intr_state_set(GPIO_ID_PIN(irparams.recvpin), GPIO_PIN_INTR_POSEDGE);
  ETS_GPIO_INTR_ENABLE();
  //ETS_INTR_UNLOCK();  
  
  //attachInterrupt(irparams.recvpin, readIR, CHANGE);  
  //irReadTimer.initializeUs(USECPERTICK, readIR).start();
  //os_timer_arm_us(&irReadTimer, USECPERTICK, 1);
  //ets_timer_arm_new(&irReadTimer, USECPERTICK, 1, 0);
}

void IRrecv::disableIRIn() {
  //irReadTimer.stop();
  //os_timer_disarm(&irReadTimer);   
   ETS_INTR_LOCK();
   ETS_GPIO_INTR_DISABLE();
}

void IRrecv::resume() {
  irparams.rcvstate = STATE_IDLE;
}

// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
bool IRrecv::decode(decode_results *results) {
  if(false==irparams.isValueReady)return false;
  ETS_GPIO_INTR_DISABLE();
  irparams.isValueReady=false;
  results->bits = NEC_BITS;
  results->value = irparams.Value;
  results->decode_type = NEC;

  ETS_GPIO_INTR_ENABLE();

  return true;
}
//-------------------------------------------
char IRdecode_NEC2char(unsigned long value){
	switch(value){
		case IR_NEC_0 :return '0';
		case IR_NEC_1 :return '1';
		case IR_NEC_2 :return '2';
		case IR_NEC_3 :return '3';
		case IR_NEC_4 :return '4';
		case IR_NEC_5 :return '5';
		case IR_NEC_6 :return '6';
		case IR_NEC_7 :return '7';
		case IR_NEC_8 :return '8';
		case IR_NEC_9 :return '9';
		case IR_NEC_STAR   :return '*';
		case IR_NEC_DRASH  :return '#';
		default: return 0;
	}
}
//eof

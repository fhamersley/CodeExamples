; Assembly code for the COMP2121 major project for the ATmega2560 microcontroller
; The project involved processing several inputs from a keypad, buttons and potentiometer
; and outputing to an LCD screen, LED array, motor and speaker.
; The task was to develop software for a 'microwave' to run on the given board in a group of two.

; Author: Fraser Hamersley & Alex Davidson for COMP2121

.include "m2560def.inc"

;;;;DEFINITIONS;;;;
.def temp = r16
.def temp2 = r17
.def temp3 = r18
.def timesec = r20
.def timemin = r21
.def turntable_timer = r22
.def magnetron_timer = r23

.equ LCD_RS = 7
.equ LCD_E = 6
.equ LCD_RW = 5
.equ LCD_BE = 4

;;;;CLEAR DATA MEMEORY MACRO;;;;

.macro clear ;clears 2 bytes of data memory
    ldi ZL, low(@0)
    ldi ZH, high(@0)
    clr r19
    st Z+, r19
    st Z, r19
.endmacro

;;;;LCD MACROS;;;;

.macro lcd_set
	sbi PORTA, @0
.endmacro
.macro lcd_clr
	cbi PORTA, @0
.endmacro
.macro do_lcd_command
	push temp
	ldi r16, @0
	rcall lcd_command
	rcall lcd_wait
	pop temp
.endmacro
.macro do_lcd_data
	push temp
	ldi r16, @0
	rcall lcd_data
	rcall lcd_wait
	pop temp
.endmacro
.macro do_lcd_data2
	push temp
	mov r16, @0
	rcall lcd_data
	rcall lcd_wait
	pop temp
.endmacro


;;;;DSEG;;;;
.dseg
count: .byte 1					;used for counting to 2 half seconds
debounce_counter: .byte 1 		;used for counting loops in debouncing algorithm
prev_keypress: .byte 1			;not debounced keypress is stored here
lcd_buffer: .byte 1				;used for lcd
timercount: .byte 2				;used for storing timer value
magnetron_timercount: .byte 2	;used for storing manetron timer value
key_input: .byte 1				;debounced key press is stored here
num_input: .byte 1				;decoded key_input is stored here
time_digits: .byte 4			;time_digits+4 is msb of minutes
power_level: .byte 1			;magnetron power level
mreg: .byte 1					;[turn|table][magne|tron][door][direction][mo|de]
ereg: .byte 1					;[minutes*10][minutes][seconds*10][seconds][X][X][X][power_menu]
								;definitions of each provided in design manual

;;;;CSEG;;;;
.cseg
.org 0x0000
jmp RESET
.org INT0addr
jmp PBA
.org INT1addr
jmp PBB
.org OVF0addr
jmp TIMEROVF

					;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~;
					; ____  ___   __   ___  _____   ;
        			; |   ||     |__  |       |     ;
					; | _/ |--      | |--     |     ; 
RESET:				; |  \ |___   __| |___    |     ;
;;;;STACK UP;;;;	;<><><><><><><><><><><><><><><><;       
ldi temp, low(RAMEND)
out SPL, temp
ldi temp, high(RAMEND)
out SPH, temp

;;;;LED SETUP(port C & port B);;;;
ser temp
out DDRC, temp
out DDRB, temp

;;;;LCD SETUP(port A & F);;;;
ser temp
out DDRF, temp
out DDRA, temp
clr temp
out PORTF, temp
out PORTA, temp

;;;;KEYPAD SETUP(port L);;;;
ldi temp, 0x0F
sts DDRL, temp

;;;;MAGNETRON SETUP(port K);;;;
ser temp
sts DDRK, temp
clr temp
sts PORTK, temp

;;;;MREG SETUP;;;;
clear mreg

;;;;EREG SETUP;;;;
ldi temp, 0b00010000  ;first digit is placed in LSB of seconds
sts ereg, temp

;;;;NUM_INPUT SETUP;;;;
clr temp
sts num_input, temp

;;;;KEY_INPUT SETUP;;;;
ldi temp, 0b11110000	;default key_input is no input
sts key_input, temp

;;;;LCD SETUP;;;;;;;;;;;;;;;;;;;;;;
do_lcd_command 0b00111000 ; 2x5x7
rcall sleep_5ms
do_lcd_command 0b00111000 ; 2x5x7
rcall sleep_1ms
do_lcd_command 0b00111000 ; 2x5x7
do_lcd_command 0b00111000 ; 2x5x7
do_lcd_command 0b00001000 ; display off?
do_lcd_command 0b00000001 ; clear display
do_lcd_command 0b00000110 ; increment, no display shift
do_lcd_command 0b00001100 ; Cursor on, bar, blink
do_lcd_command 0b00000010; GO HOME

;;;;TIMER SETUP;;;;
clr temp
out TCCR0A, temp
ldi temp, 2
out TCCR0B, temp
ldi temp, (1 << TOIE0)
sts TIMSK0, temp

;;;;PIN INTERRUPT SETUP;;;;
ldi temp, (2 << ISC00) ;falling edge interrupt set for interrupt pin 0
sts EICRA, temp
in temp, EIMSK
ori temp, 0b00000011    ;enable interrupt on pins 0 and 1 (interrupt mask register)
out EIMSK, temp

;INTIALISE ALL OTHER REGISTERS
ldi magnetron_timer, 0b00000011
clr turntable_timer
clr timemin
clr timesec

;;;;CLEAR DATA SEGMENTS;;;;
clear count
clear timercount
clear time_digits
clear time_digits + 2
clear debounce_counter

sei
jmp main
				; 	END RESET ---------------------------------

;==================================================
;			*MAIN*			  					//
main:						 					//
						  						//
end:						  					//
						  						//
rcall keypad	;keypad is an infinite loop     //
						  						//
;	    					  					//
;=================================================

;;;;int0;;;;
PBA: ;;;;CLOSE DOOR;;;;
	push temp
	in temp, SREG
	push temp
	clr temp
	out PORTB, temp ;turn topmost LED off
	lds temp, mreg ;check what mode and that door is open and then do correct change for each mode
	andi temp, 0b00001011
	cpi temp, 0b00001000
	breq cd_entry
	cpi temp, 0b00001001
	breq cd_pause
	rjmp end_PBA

	cd_entry:
		lds temp, mreg	
		andi temp, 0b11110111
		ori temp, 0b00000000
		sts mreg, temp
		rjmp end_PBA

	cd_pause:
		lds temp, mreg	
		andi temp, 0b11110111
		ori temp, 0b00000001
		sts mreg, temp
		rjmp end_PBA
	
	end_PBA:
		pop temp
		out SREG, temp
		pop temp
		reti
;;;;int1;;;;
PBB: ;;;;OPEN DOOR;;;;
	push temp
	in temp, SREG
	push temp	
	lds temp, mreg
	ser temp ;turn topmost LED on
	out PORTB, temp
	lds temp, mreg ;check what mode and that door is open and then do correct change for each mode
	andi temp, 0b00001011
	cpi temp, 0b00000000
	breq od_entry
	cpi temp, 0b00000001
	breq od_pause
	cpi temp, 0b00000010
	breq od_running
	cpi temp, 0b00000011
	breq od_finished
	rjmp end_PBB

	od_entry:
		lds temp, mreg	
		andi temp, 0b11110111
		ori temp, 0b00001000
		sts mreg, temp
		rjmp end_PBB

	od_pause:
		lds temp, mreg	
		andi temp, 0b11110111
		ori temp, 0b00001000
		sts mreg, temp
		rjmp end_PBB

	od_running:
		lds temp, mreg	
		andi temp, 0b11110100
		ori temp, 0b00001001
		sts mreg, temp
		rjmp end_PBB

	od_finished:
		lds temp, mreg	
		andi temp, 0b11110100
		ori temp, 0b00001000
		sts mreg, temp
		ldi temp, 0b00010000
		sts ereg, temp
		clear time_digits
		clear time_digits + 2
		do_lcd_command 0b00000001 ; clear display
		rjmp end_PBB
	
	end_PBB:
		pop temp
		out SREG, temp
		pop temp
		reti

MAGNETRON_TIMER_OVERFLOW:
push temp
push temp2

lds temp, mreg
mov temp2, temp
andi temp2, 0b00000011
cpi temp2, 2
brne magnetron_off

inc magnetron_timer
andi magnetron_timer, 0b00000011
breq magnetron_on	;every full second, magnetron is on for all non zero settings

sbrs temp, 5
rjmp magnetron_low;LOW SETTING - turns off if one quarter second has passed (since XXXXXX00)
sbrs temp, 4
rjmp magnetron_med;MEDIUM SETTING - turns off if two quarter seconds have passed (")

magnetron_on:
;turn on motor
	ser temp2
	sts PORTK, temp2
	rjmp end_magnetron_overflow

magnetron_low:
	sbrc magnetron_timer, 0
	rjmp magnetron_off
	rjmp end_magnetron_overflow

magnetron_med:
	sbrs magnetron_timer, 0
	rjmp magnetron_off
	rjmp end_magnetron_overflow

magnetron_off:
	clr temp2
	sts PORTK, temp2

end_magnetron_overflow:
	pop temp2
	pop temp
	ret

///////////////////////////////////// _____		 ___	       ___
;				TIMER_OVERFLOW//////	|		|   |   \  /  |_
TIMEROVF:					///////		|		|___|    \/   |
	push temp
	in temp, SREG
	push temp

	microwave_update:
		rcall update_microwave			
		rcall update_magnetron
	
	pop temp
	out SREG, temp
	pop temp
	reti

;;;;UPDATE MAGNETRON;;;;
;Checks if 1/4 second.
;If so, go to magnetron_timer_overflow
update_magnetron:
	push temp
	push temp2

	lds temp, mreg
	andi temp, 0b00110000
	cpi temp, 0b00000000
	breq end_mag_update	;magnetron is off, skip to end
	;Check if one quarter of a second
	;if so, clear 2byte magnetron_timercount,
	;increment magnetron_timer
	;examine last two bits of magnetron_timer vs mreg to turn motor on/off
	lds YL, magnetron_timercount
	lds YH, magnetron_timercount + 1
	adiw Y, 1
	ldi temp, high(1953)
	cpi YL, low(1953)
	cpc YH, temp
	brne not_quarter_second

	clear magnetron_timercount
	rcall MAGNETRON_TIMER_OVERFLOW
	rjmp end_mag_update

	not_quarter_second:
		sts magnetron_timercount, YL
		sts magnetron_timercount + 1, YH

end_mag_update:
	pop temp2
	pop temp
	ret


//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\
;;;;;UPDATE MICROWAVE;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Makes state updates every 128 microseconds
;Branches to correct mode to update state
;Calls synchronous updates every 1/2 second
update_microwave:
	push temp
	push temp2
	
	lds temp, mreg
	andi temp, 0b00000011	;check each mode and jump to function caller
	breq entry
	cpi temp, 0b00000001
	breq pause
	cpi temp, 0b00000010
	breq running
	cpi temp, 0b00000011
	breq finish
	rjmp other_asynchronous_updates   ;just in case
	
entry:
	rcall entry_mode
	rjmp other_asynchronous_updates
pause:
	rcall pause_mode
	rjmp other_asynchronous_updates
running:
	rcall running_mode_asynchronous
	rjmp other_asynchronous_updates
finish:
	rcall finish_mode
	rjmp other_asynchronous_updates

other_asynchronous_updates:
	lds temp, key_input
	andi temp, 0b11111110 ;clearing LSB of key_input making it no longer valid (all key checks look for this LSB to be set)
	sts key_input, temp
	lds YL, timercount
	lds YH, timercount + 1
	adiw Y, 1
	ldi temp, high(3906)
	cpi YL, low(3906)
	cpc YH, temp
	brne not_second

synchronous_updates:
	clear timercount
	lds temp, mreg
	andi temp, 0b00000011 ;Check if in running mode to call running modes synchronous updates
	cpi temp, 0b00000010
	brne display_now
	rcall running_mode_sync ;Running mode is only called ever 1/2 sec to account for timedown call
display_now:
	rcall lcd_display
	pop temp2
	pop temp
	ret
	
not_second:
	sts timercount, YL
	sts timercount + 1, YH
	pop temp2
	pop temp
	ret
;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\----------------------------------------
;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^END UPDATE MICROWAVE^


;					ENTRY_MODE
entry_mode:
	push temp
	push temp2
	lds temp, mreg
	andi temp, 0b00001000
	cpi temp, 0b00001000
	breq entry_mode_end	;door is open - we can't do anything; bail.
	lds temp, ereg
	andi temp, 0b00000001
	breq enter_time

	rcall power_menu
	rjmp entry_mode_end

	enter_time:
		lds temp, key_input
		cpi temp, 0b11100001	;star
		breq entry_to_running
		cpi temp, 0b01111111	;'A'
		breq to_power_menu
		cpi temp, 0b10110001    ;'#'
		breq clear_time
		rcall decode_numerical_input
		rcall set_timer
		rjmp entry_mode_end
	entry_to_running:
		rcall turntable_reverse
		lds temp, mreg
		andi temp, 0b11111100	;clear mode bits
		ori temp, 0b00000010	;set to running mode
		sts mreg, temp
		rcall add_minute_if_zero
		;do_lcd_command 0b00000001	;clear screen
		rjmp entry_mode_end
	to_power_menu:
		ldi temp, 0b00010001
		sts ereg, temp
		rjmp entry_mode_end
	clear_time:
		clear time_digits
		clear time_digits + 2
		clr timesec
		clr timemin
		ldi temp, 0b00010000
		sts ereg, temp
	entry_mode_end:
		pop temp2
		pop temp
		ret
;Associated entry mode functions below
power_menu:	
	push temp
	lds temp, key_input  ;look for a valid input and then set the corresponding power level in the mreg
	cpi temp, 0b11101111	;1
	breq l_percent
	cpi temp, 0b11011111	;2
	breq m_percent
	cpi temp, 0b10111111	;3
	breq h_percent
	cpi temp, 0b10110001	;#
	breq power_to_time
	rjmp power_mode_end
	l_percent:
		lds temp, mreg
		andi temp, 0b11001111
		ori temp, 0b00010000
		sts mreg, temp
		ldi temp, 3
		out PORTC, temp ;set the LED bar to show power level
		ldi temp, 0b00010000;
		sts ereg, temp
		do_lcd_command 0b00000001
		rjmp power_mode_end
	m_percent:
		lds temp, mreg
		andi temp, 0b11001111
		ori temp, 0b00100000
		sts mreg, temp
		ldi temp, 15
		out PORTC, temp ;set the LED bar to show power level
		ldi temp, 0b00010000;
		sts ereg, temp
		do_lcd_command 0b00000001
		rjmp power_mode_end
	h_percent:
		lds temp, mreg
		andi temp, 0b11001111
		ori temp, 0b00110000
		sts mreg, temp
		ldi temp, 0xFF
		out PORTC, temp ;set the LED bar to show power level
		ldi temp, 0b00010000;
		sts ereg, temp
		do_lcd_command 0b00000001
		rjmp power_mode_end
	power_to_time:
		ldi temp, 0b00010000;
		sts ereg, temp
		do_lcd_command 0b00000001
		rjmp power_mode_end
	power_mode_end:
		pop temp
		ret

decode_numerical_input:	;converts keypad input into a binary number, if at all
	push temp			;uses simple switch case type logic
	push temp2
	clr temp2
	ldi temp2, 9
	lds temp, key_input
	cpi temp, 0b10111001	;'9'
	breq numerical_input_end
	dec temp2
	cpi temp, 0b11011001	;'8'
	breq numerical_input_end
	dec temp2
	cpi temp, 0b11101001	;'7'
	breq numerical_input_end
	dec temp2
	cpi temp, 0b10111101	;'6'
	breq numerical_input_end
	dec temp2
	cpi temp, 0b11011101	;'5'
	breq numerical_input_end
	dec temp2
	cpi temp, 0b11101101	;'4'
	breq numerical_input_end
	dec temp2
	cpi temp, 0b10111111	;'3'
	breq numerical_input_end
	dec temp2
	cpi temp, 0b11011111	;'2'
	breq numerical_input_end
	dec temp2
	cpi temp, 0b11101111	;'1'
	breq numerical_input_end
	dec temp2
	cpi temp, 0b11010001	;'0'
	breq numerical_input_end
	dec temp2	;temp2 = 0xFF => no valid numerical input
	numerical_input_end:
		sts num_input, temp2  ;places the decoded key into num_input
		pop temp2
		pop temp
		ret
set_timer:
	push temp
	push temp2
	push temp3
	;check if there is numerical input to be read. If not, bail immediately
	lds temp, num_input
	cpi temp, 0xFF
	breq end_set_timer

	lds temp2, ereg
	cpi temp2, 0		;check if another digit can be accepted (one of the upper nibble bits in ereg set)
	breq end_set_timer
	rcall shift_time_digits
	rcall update_time
	lsl temp2
	sts ereg, temp2

	end_set_timer:
		pop temp3
		pop temp2
		pop temp
		ret

add_minute_if_zero:
	push temp
	clr temp
	cpi timemin, 0
	cpc	timesec, temp
	brne add_minute_if_zero_end
	inc timemin

add_minute_if_zero_end:
	pop temp
	ret

shift_time_digits:	;shifts each digit to be 1 more significant place
					;key_input shifts into least digit
	push temp
	push temp2
	lds temp, time_digits
	lds temp2, time_digits + 1
	sts time_digits + 1, temp
	lds temp, time_digits + 2
	sts time_digits + 2, temp2
	sts time_digits + 3, temp
	lds temp, num_input
	sts time_digits, temp
	pop temp2
	pop temp
	ret

update_time:
	push temp
	push temp2
	clr timemin
	clr timesec
	ldi temp2, 10
	;seconds
	lds temp, time_digits
	add timesec, temp
	;seconds*10
	lds temp, time_digits + 1
	mul temp, temp2
	add timesec, r0
	;minutes
	lds temp, time_digits + 2
	add timemin, temp
	;minutes*10
	lds temp, time_digits + 3
	mul temp, temp2
	add timemin, r0

	pop temp2
	pop temp
	ret
;----------------------------------------	end associated entry mode functions


;	PAUSE MODE
;examine keypad and make mode transitions in mreg if needed
pause_mode:
	push temp
	lds temp, key_input
	cpi temp, 0b11100001 ;if star is pushed jump to function to change mode to running
	breq pause_to_running
	cpi temp, 0b10110001 ;if hash is pushed jump to function to change mode to entry
	breq pause_to_entry
	rjmp pause_end ;otherwise do nothing

pause_to_running:
	lds temp, mreg
	andi temp, 0b11111100  ;clear mode bits
	sbrc temp, 3			; don't run if door is open
	rjmp pause_end
	ori temp, 0b00000010  ;set new mode bits
	sts mreg, temp
	rcall turntable_reverse
	rjmp pause_end

pause_to_entry:
	lds temp, mreg
	andi temp, 0b11111100  ;clear mode bits
	sbrc temp, 3		; don't run if door is open
	rjmp pause_end
	sts mreg, temp
	ldi temp, 0b00010000
	sts ereg, temp
	clear time_digits
	clear time_digits + 2
	clr timemin
	clr timesec

pause_end:
	pop temp
	ret
;----------------------Pause mode end---------------------------

;Asynchronous 			***RUNNING MODE*** 			update function
;Looks for keypad presses, to add/subtract time
;or to transition to pause mode
running_mode_asynchronous:
	push temp
	push temp2
	lds temp, key_input
	cpi temp, 0b11100001 ;if star is pushed, add one hot minute
	breq running_add_min
	cpi temp, 0b10110001 ;if hash is pushed, pause
	breq running_to_pause
	cpi temp, 0b01111001 ;if C is pushed add 30secs
	breq add_thirty
	cpi temp, 0b01110001 ;if D is pushed minus 30sec
	breq minus_thirty
	rjmp running_async_end

running_add_min:
	cpi timemin, 99 ;if time is already max, do nothing
	breq running_async_end
	inc timemin
	rjmp running_async_end

running_to_pause:
	lds temp, mreg
	andi temp, 0b11111100
	inc temp
	sts mreg, temp
	rjmp running_async_end

add_thirty:
	cpi timemin, 99 ;if time min is 99 then will check time sec
	breq check_sec
	cpi timesec, 30
	brlo add_thirty_simply
	add_thirty_complex: ;timesec is more than 30 so a min to timemin is added and 30 seconds taken from timesec
		subi timesec, 30
		inc timemin
		rjmp running_async_end
	add_thirty_simply: ;timesec is less than 30 so 30secs can be added to timesec
		ldi temp2, 30
		add timesec, temp2
		rjmp running_async_end
	check_sec:		
		cpi timesec, 70		;if timesec is lower than 70 then add 30
		brlo add_thirty_simply
		rjmp running_async_end ;else do nothing

minus_thirty:
	cpi timesec, 30 ;first check timesec for greater than 30
	brsh minus_thirty_simply ;if so then deduct 30 from timesec and end
	cpi timemin, 0 ;then check if timemin is 0, if so make timesec 0 and end
	breq minus_thirty_zero
	dec timemin  ;otherwise take a min off timemin and add 30secs to timesec then end
	ldi temp2, 30
	add timesec, temp2
	rjmp running_async_end
	minus_thirty_zero:
		clr timesec ;
		rjmp running_async_end
	minus_thirty_simply:
		subi timesec, 30
		rjmp running_async_end

running_async_end:
	pop temp2
	pop temp
	ret
	;------------------Async running mode end---------------------------

;	FINISHED MODE
;simply look for # in keypress to update to entry mdoe

finish_mode:
	push temp
	lds temp, mreg ;first check mreg if door has been opened
	andi temp, 0b00001000
	cpi temp, 0b00001000
	breq finish_to_entry
	lds temp, key_input ;then check key to see if stop (#) has been pushed
	cpi temp, 0b10110001
	breq finish_to_entry
	rjmp finish_mode_end ;if nothing, do nothing
	
	finish_to_entry:	
		lds temp, mreg
		andi temp, 0b11111100
		ori temp, 0b00000000
		sts mreg, temp
		ldi temp, 0b00010000
		sts ereg, temp
		clear time_digits
		clear time_digits + 2
		do_lcd_command 0b00000001 ;clear display


	finish_mode_end:
		pop temp
		ret
;--------------------------finish mode end----------------------------

running_mode_sync:
	push temp
	lds temp, count
	inc temp ;Increase the half-second counter by one.
	sts count, temp
	sbrc temp, 0
	rcall timedown	
	rcall turntable_update

running_mode_end:
	pop temp
	ret

turntable_reverse:
	push temp
	push temp2
	ser temp2
	lds temp, mreg
	ori temp, 0b11111011
	eor temp, temp2	;toggles all bits in temp
	;temp is now 00000X00, X is toggled direction bit
	lds temp2, mreg
	andi temp2, 0b11111011	;clear direction bit
	add temp2, temp			;input new direction bit
	sts mreg, temp2
	pop temp2
	pop temp
	ret

/////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
;KEYPAD: Stores encoded input into key_input (data memory)/////////
keypad:
	;pull up columns, ground all rows, read for any zero columns
	;algorithm described in design manual	

	ldi temp2, 0xFF
check_row:
	lsl temp2
	sts PORTL, temp2
	lds temp, PINL
	mov temp3, temp
	andi temp, 0xF0
	cpi temp, 0xF0	;checking columns only	
	breq check_row_2	;nothing is pressed
	rjmp keypad_epilogue
check_row_2:
	cpi temp2, 0b11110000
	breq keypad_epilogue
	rjmp check_row

keypad_epilogue:
	lds temp, debounce_counter
	cpi temp, 8  ;8ms debounce
	breq store_and_loop
	lds temp2, prev_keypress
	cp temp3, temp2
	breq sleepytime
	clr temp2					;clear debounce counter
	sts debounce_counter, temp2	;]
	sleepytime:
		rcall sleep_1ms
		lds temp2, debounce_counter	;|
		inc temp2					; }increment debounce_counter
		sts debounce_counter, temp2	;|
		sts prev_keypress, temp3
		rjmp keypad

store_and_loop:
	lds temp2, key_input
	cp temp2, temp3
	breq clear_debounce
	lds temp2, prev_keypress
	cp temp3, temp2
	brne clear_debounce
	inc temp3
	sts key_input, temp3
clear_debounce:
	clr temp2
	sts debounce_counter, temp2
	rjmp keypad

;-------------------	end keypad (it will loop constantly)	--------------------------
;----------------------------------------------------------------------------------------/


;	***TURNTABLE UPDATE***
;does not display, only updates mreg
;if 2.5s has passed in running mode

turntable_update:	
	push temp
	push temp2
	inc turntable_timer
	cpi turntable_timer, 5
	brne end_turntable_update
	;shift mreg right 6 times (temp)
	;increment or decrement depending on direction 
	;replace ms2bits in mreg (temp2) with ls2bits in temp (temp)
	clr turntable_timer
	lds temp, mreg
	clr temp2
	p_loop:				;lsr through mreg so turntable bits
		inc temp2		;are in least significant spot,
		lsr temp		;all other bits zero
		cpi temp2, 6
		brne p_loop
	lds temp2, mreg
	sbrc temp2, 2
	rjmp turn_anticlockwise

	turn_clockwise:			;TURN_CLOCKWISE
		inc temp
		rjmp replace_turntable
	turn_anticlockwise:		;TURN_ANTICLOCKWISE
		dec temp

	replace_turntable:
		andi temp2, 0b00111111 	;clear turntable position in mreg(temp2)
		sbrc temp, 0
		ori temp2, 0b01000000	;|
		sbrc temp, 1			;|-Bitwise setting correct position in mreg(temp2)
		ori temp2, 0b10000000	;|

	sts mreg, temp2

	end_turntable_update:
		pop temp2
		pop temp
		ret
;---------------------------------End turntable update	-----------------------


;;;;TIMER FUNCTION;;;;
;
timedown:
	push temp
	push temp2
	cpi timesec, 0
	breq mindown
	dec timesec
	pop temp2
	pop temp
	ret

mindown:
	cpi timemin, 0
	breq reached_zero
	dec timemin
	ldi timesec, 59
	pop temp2
	pop temp
	ret

reached_zero:
	;modify mreg
	lds temp, mreg
	ori temp, 0b00000011
	sts mreg, temp
	;immediately call display_finish
	do_lcd_command 0b00000001 ; clear display
	rcall display_finish
	pop temp2
	pop temp
	ret
	

///////////////////////////////////////////////////////////
;;;;DISPLAY FUNCTIONS;;;;----------------------------------
;			        			---------------------------

lcd_display:
	push temp
	push temp2
	push temp3

	do_lcd_command 0b00000011 ; go home
	clr temp
	lds temp, mreg
	andi temp, 0b00000011
	breq displayentry
	cpi temp, 0b00000001
	breq displaypause
	cpi temp, 0b00000010
	breq displayrunning
	cpi temp, 0b00000011
	breq displayfinish
	jmp display_epilogue ;just incase :S

displayentry:	
	lds temp, ereg
	sbrs temp, 0
	rcall display_time ;does not display time if in power mode
	sbrc temp, 0
	rcall display_power ;does display power if in power mode
	rcall display_platter
	rcall display_door
	rjmp display_epilogue

displaypause:
	rcall display_time
	rcall display_platter
	rcall display_door
	rjmp display_epilogue

displayrunning:
	rcall display_time
	rcall display_platter
	rcall display_door
	rjmp display_epilogue

displayfinish:
	rcall display_finish
	rcall display_platter
	rcall display_door
	rjmp display_epilogue
	
display_epilogue:
	pop temp3
	pop temp2
	pop temp
	ret
;--------------------- end LCD display ----------------
	

;				***DISPLAY TIME***
display_time:
	push temp
	push temp2
	push temp3
	;CONVERTING TIMESEC TO 2 DIGITS
	mov temp, timesec
	clr temp2
	ldi temp3, 10
	loopa1:
		cpi temp, 10
		brlo push1
		sub temp, temp3
		inc temp2
		rjmp loopa1
	push1:
		push temp        ;store the values in the stack
		push temp2

	;CONVERTING TIMEMIN TO 2 DIGITS
	mov temp, timemin
	clr temp2
	loopa2:
		cpi temp, 10
		brlo push2
		sub temp, temp3
		inc temp2
		rjmp loopa2
	push2:
		push temp		;store the values in the stack
		push temp2

	;;;;CONVERT TO ASCII, OUTPUT ONTO SCREEN;;;
	display:
		pop temp			;pull the data from the stack
		ldi temp3, 48		;add 48 to each digitfor ascii value
		add temp, temp3
		do_lcd_data2 temp	;display digit and repeat
		pop temp
		add temp, temp3
		do_lcd_data2 temp
		do_lcd_data ':'		;display :
		pop temp
		add temp, temp3
		do_lcd_data2 temp
		pop temp
		add temp, temp3
		do_lcd_data2 temp

		pop temp3
		pop temp2
		pop temp
		ret
;;;;;;;;;;;;;;;;;;;;------- end display time ----------------


;;;;DISPLAY POWER;;;;
display_power:
	push temp
	do_lcd_data 'S'
	do_lcd_data 'e'
	do_lcd_data 't'
	do_lcd_data ' '
	do_lcd_data 'P'
	do_lcd_data 'o'
	do_lcd_data 'w'
	do_lcd_data 'e'
	do_lcd_data 'r'
	do_lcd_data ' '
	do_lcd_data '1'
	do_lcd_data '/'
	do_lcd_data '2'
	do_lcd_data '/'
	do_lcd_data '3'
	pop temp
	ret

;;;;DISPLAYING PLATTER ROTATION;;;;

display_platter:
	push temp
	push temp2
	do_lcd_command 0b00000011 ; go home
	clr temp

;Shift to right side, top line of LCD
platter_loop:	
	inc temp
	do_lcd_command 0b00010111
	cpi temp, 15
	brne platter_loop	

	clr temp
	lds temp2, mreg
platter_loop2:		;lsr through mreg so turntable bits
	inc temp		;are in least significant spot,
	lsr temp2		;all other bits zero
	cpi temp, 6
	brne platter_loop2

;Check where the turntable is, display appropriately
	cpi temp2, 2
	breq dash
	cpi temp2, 1
	breq fslash
	cpi temp2, 3
	breq bslash
	do_lcd_data '|'
display_platter_epilogue:
	pop temp2
	pop temp
	ret
dash:	do_lcd_data '-'
		rjmp display_platter_epilogue
fslash:	do_lcd_data '/'
		rjmp display_platter_epilogue
bslash:	do_lcd_data 205
		rjmp display_platter_epilogue

display_door:
	push temp
	do_lcd_command 0b11000000	;Second line
	clr temp
	door_loop:		;Shift to right side, bottom line of LCD
		inc temp
		do_lcd_command 0b00010111
		cpi temp, 15
		brlo door_loop
	lds temp, mreg	;If door flag is set output 'O' on LCD, if the bit is clear diplay a 'C' on LCD
	andi temp, 0b00001000;
	cpi temp, 0b00000000;
	breq closed
	cpi temp, 0b00001000
	breq open
	rjmp display_end ;just in case
	closed:
		do_lcd_data 'C'
		rjmp display_end
	open:
		do_lcd_data 'O'
		rjmp display_end

	display_end:
		pop temp
		ret

display_finish:
	push temp
	do_lcd_data 'D'
	do_lcd_data 'O'
	do_lcd_data 'N'
	do_lcd_data 'E'
	do_lcd_command 0b11000000 ;second line
	do_lcd_data 'R'
	do_lcd_data 'E'
	do_lcd_data 'M'
	do_lcd_data 'O'
	do_lcd_data 'V'
	do_lcd_data 'E'
	do_lcd_data ' '
	do_lcd_data 'F'
	do_lcd_data 'O'
	do_lcd_data 'O'
	do_lcd_data 'D'
	pop temp
ret

;;;;;;;;;;;;;;;;;;;;;
;;;;LCD FUNCTIONS;;;;	credit for the below goes to D. Murphy, UNSW CSE
do_lcd_data3:
	push temp
	lds temp, lcd_buffer
	rcall lcd_data
	rcall lcd_wait
	pop temp
	ret

lcd_command:
	push temp
	out PORTF, r16
	rcall sleep_1ms
	lcd_set LCD_E
	rcall sleep_1ms
	lcd_clr LCD_E
	rcall sleep_1ms
	pop temp
	ret

lcd_data:
	push temp
	out PORTF, r16
	lcd_set LCD_RS
	rcall sleep_1ms
	lcd_set LCD_E
	rcall sleep_1ms
	lcd_clr LCD_E
	rcall sleep_1ms
	lcd_clr LCD_RS
	pop temp
	ret

lcd_wait:
	push temp
	clr r16
	out DDRF, r16
	out PORTF, r16
	lcd_set LCD_RW
lcd_wait_loop:
	rcall sleep_1ms
	lcd_set LCD_E
	rcall sleep_1ms
	in r16, PINF
	lcd_clr LCD_E
	sbrc r16, 7
	rjmp lcd_wait_loop
	lcd_clr LCD_RW
	ser r16
	out DDRF, r16
	pop temp
	ret

.equ F_CPU = 16000000
.equ DELAY_1MS = F_CPU / 4 / 1000 - 4
; 4 cycles per iteration - setup/call-return overhead

sleep_1ms:
	push r24
	push r25
	ldi r25, high(DELAY_1MS)
	ldi r24, low(DELAY_1MS)
delayloop_1ms:
	sbiw r25:r24, 1
	brne delayloop_1ms
	pop r25
	pop r24
	ret

sleep_5ms:
	push temp
	push temp2
	rcall sleep_1ms
	rcall sleep_1ms
	rcall sleep_1ms
	rcall sleep_1ms
	rcall sleep_1ms
	pop temp2
	pop temp
	ret


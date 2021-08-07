/*
 * 
 * File:   main.c
 * Author: Barak Daniel
 * Subject: This file is the main file of the 
 * clock project for the Embedded course on
 * Shenkar College.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "System/system.h"
#include "System/delay.h"
#include "oledDriver/oledC.h"
#include "oledDriver/oledC_colors.h"
#include "oledDriver/oledC_shapes.h"

/*
 * Defines
 */

#define MAX_WIDTH 95;
#define MAX_HEIGHT 95;

void conevrtTwoDigToString(int value, char* buffer) 
{
    buffer[0] = (value/10)+48;
    buffer[1] = (value%10)+48;
    buffer[2] = '\0';
}

/*
 *  Data Types 
 */
enum mode {analog=0, digital=1, menu=2};
enum ampm { on=0, off=1 };

typedef struct app_state {
    enum mode mode;
    enum mode lastClockType;
    enum ampm ampm_mode;
    bool ampm_flag;
    bool date_flag;
    bool alarm;
    unsigned char s1_cnt;
    unsigned char s2_cnt;
    unsigned char menuOption;
    unsigned char prevMenuOption;
    unsigned char numOfMenuOptions;
    unsigned char currMenu;
    bool selectPressed;
} app_state;

typedef struct date_time {
    unsigned char month, day;
    unsigned char hours, mintues, seconds;
} date_time;

/*
 * Globals
 */
date_time currTime;
date_time prevTime;
date_time setClock;
unsigned char setClockPtr;
app_state myAppState;
unsigned days_per_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
unsigned char middlePoint[2] = {47,47};
unsigned char clockPoints[60][2] = {{47,7},{51,7},{55,8},{59,9},{63,10},{67,12},{71,15},{74,17},{77,20},{79,23},{82,27},{84,31},{85,35},{86,39},{87,43},
                                    {87,47},{87,51},{86,55},{85,59},{84,63},{82,67},{79,71},{77,74},{74,77},{71,79},{67,82},{63,84},{59,85},{55,86},{51,87},
                                    {47,87},{43,87},{39,86},{35,85},{31,84},{27,82},{23,79},{20,77},{17,74},{15,71},{12,67},{10,63},{9,59},{8,55},{7,51},
                                    {7,47},{7,43},{8,39},{9,35},{10,31},{12,27},{15,23},{17,20},{20,17},{23,15}, {27,12},{31,10},{35,9},{39,8},{43,7} };


void initAppState(app_state* state) {
    state->mode = analog;
    state->lastClockType = analog;
    state->ampm_mode = on;
    state->alarm = false;
    state->ampm_flag = true;
    state->date_flag = true;
    state->menuOption = 0;
    state->prevMenuOption = 0;
    state->numOfMenuOptions = 5;
    state->selectPressed = false;
    state->currMenu = 0;
}

void initTime(date_time* time) {
    time->month = 12;
    time->day = 31;
    time->hours = 19;
    time->mintues = 9;
    time->seconds = 30;
}

void copyTime(date_time* to, date_time* from) {
    to->month = from->month;
    to->day = from->day;
    to->hours = from->hours;
    to->mintues = from->mintues;
    to->seconds = from->seconds;
}

void incTime(date_time* time) {
    time->seconds += 1;
    
    if(time->seconds == 60) {
        time->seconds = 0;
        time->mintues += 1;
            if(time->mintues == 60) {
                time->mintues = 0;
                time->hours += 1;
                    if(time->hours == 24) {
                        time->hours = 0;
                        time->day += 1;
                        myAppState.ampm_flag = true;
                        myAppState.date_flag = true;
                        if(time->day > days_per_month[time->month - 1]) {
                            time->day = 1;
                            time->month += 1;
                            if(time->month > 12) 
                                time->month = 1;
                        }                        
                    } else if(time->hours == 12) {
                        myAppState.ampm_flag = true;
                    }
            }
    }
}

void  getDateString(date_time* time, char* ret_buffer) {
    char date_string[6];
    char buff[3];
    conevrtTwoDigToString(time->day, buff);
    strcpy(date_string, buff);
    strcat(date_string, "/");
    conevrtTwoDigToString(time->month, buff);
    strcat(date_string, buff);
    
    strcpy(ret_buffer, date_string);
}

void  getTimeString(date_time* time, char* ret_buffer) {
    char time_string[9];
    char buff[3];
    int temp_hrs = time->hours;
    if(myAppState.ampm_mode==0 && temp_hrs >= 12) temp_hrs -= 12; 
    conevrtTwoDigToString(temp_hrs, buff);
    strcpy(time_string, buff);
    strcat(time_string, ":");
    conevrtTwoDigToString(time->mintues, buff);
    strcat(time_string, buff);
    strcat(time_string, ":");
    conevrtTwoDigToString(time->seconds, buff);
    strcat(time_string, buff);
    
    strcpy(ret_buffer, time_string);
}

/*
 * Graphics
 */

    /* 
     * ALARM
     */
void drawAlarm() {
    oledC_DrawRing(6,6,4,2, OLEDC_COLOR_RED);
}

void removeAlarm() {
    oledC_DrawRing(6,6,4,2, OLEDC_COLOR_BLACK);
}

    /* 
     * AMPM
     */
void clearAMPM() {
    oledC_DrawRectangle(1, 88, 15, 95, OLEDC_COLOR_BLACK);
}

void drawAMPM() {   
    clearAMPM();
    
    if(currTime.hours < 12) {
        char ampm_str[3] = "AM";
        oledC_DrawString(1, 88, 1, 1, ampm_str, OLEDC_COLOR_WHITE);
    }
    else {
        char ampm_str[3] = "PM";
        oledC_DrawString(1, 88, 1, 1, ampm_str, OLEDC_COLOR_WHITE);
    }
}

    /* 
     * Date
     */
void removeDate() {
    oledC_DrawRectangle(68, 88, 95, 95, OLEDC_COLOR_BLACK);
}

void drawDate() {
    removeDate();
    char date[6];
    getDateString(&currTime, date);
    oledC_DrawString(65, 88, 1, 1, date, OLEDC_COLOR_WHITE);    
}

    /* 
     * ANALOG
     */

void drawAnalogStatus() {
    oledC_DrawCircle(middlePoint[0], middlePoint[1], 3, OLEDC_COLOR_WHITE);
    
    unsigned char point[2];    
    point[0] = clockPoints[currTime.seconds][0]-(clockPoints[currTime.seconds][0]-middlePoint[0])/5;
    point[1] = clockPoints[currTime.seconds][1]-(clockPoints[currTime.seconds][1]-middlePoint[1])/5;
    oledC_DrawLine(middlePoint[0], middlePoint[1], point[0], point[1], 1, OLEDC_COLOR_WHITE);
    point[0] = clockPoints[currTime.mintues][0]-(clockPoints[currTime.mintues][0]-middlePoint[0])/5;
    point[1] = clockPoints[currTime.mintues][1]-(clockPoints[currTime.mintues][1]-middlePoint[1])/5;
    oledC_DrawLine(middlePoint[0], middlePoint[1], point[0], point[1], 2, OLEDC_COLOR_WHITE);
    point[0] = middlePoint[0] + (clockPoints[currTime.hours*5%60][0]-middlePoint[0])/2;
    point[1] = middlePoint[1] + (clockPoints[currTime.hours*5%60][1]-middlePoint[1])/2;
    oledC_DrawLine(middlePoint[0], middlePoint[1], point[0], point[1], 4, OLEDC_COLOR_WHITE); 
    if(myAppState.ampm_flag) {
        drawAMPM();
        myAppState.ampm_flag = false;
    }
    if(myAppState.date_flag) {
        drawDate();
        myAppState.date_flag = false;
    }
}

void clearAnalogStatus() {
    unsigned char point[2];
    point[0] = clockPoints[prevTime.seconds][0]-(clockPoints[prevTime.seconds][0]-middlePoint[0])/5;
    point[1] = clockPoints[prevTime.seconds][1]-(clockPoints[prevTime.seconds][1]-middlePoint[1])/5;
    oledC_DrawLine(middlePoint[0], middlePoint[1], point[0], point[1], 1, OLEDC_COLOR_BLACK);
    if(prevTime.mintues != currTime.mintues) {
        point[0] = clockPoints[prevTime.mintues][0]-(clockPoints[prevTime.mintues][0]-middlePoint[0])/5;
        point[1] = clockPoints[prevTime.mintues][1]-(clockPoints[prevTime.mintues][1]-middlePoint[1])/5;
        oledC_DrawLine(middlePoint[0], middlePoint[1], point[0], point[1], 2, OLEDC_COLOR_BLACK);
    }   
    if(prevTime.hours != currTime.hours) {
        point[0] = middlePoint[0] + (clockPoints[prevTime.hours*5%60][0]-middlePoint[0])/2;
        point[1] = middlePoint[1] + (clockPoints[prevTime.hours*5%60][1]-middlePoint[1])/2;
        oledC_DrawLine(middlePoint[0], middlePoint[1], point[0], point[1], 4, OLEDC_COLOR_BLACK);  
    }
    copyTime(&prevTime, &currTime);
}

void drawAnalogBase() {
    oledC_clearScreen();
    uint8_t i;
    
    unsigned char newPoint[2];
    unsigned char width = 0;
    for(i=0; i<60; i++) {
        if(i%5==0) {
            if(i%15 == 0) 
                width = 2; 
            else 
                width = 1;
            newPoint[0] = middlePoint[0] + 9*(clockPoints[i][0] - middlePoint[0])/10;
            newPoint[1] = middlePoint[1] + 9*(clockPoints[i][1] - middlePoint[1])/10;
            oledC_DrawLine(newPoint[0], newPoint[1], clockPoints[i][0], clockPoints[i][1], width, OLEDC_COLOR_BLUE);
        }
    }
    
    myAppState.ampm_flag = true;
    myAppState.date_flag = true;
    drawAnalogStatus();
    if(myAppState.alarm == true) drawAlarm();
    else removeAlarm();
    
    copyTime(&prevTime, &currTime);
}

    /*
     * DIGITAL
     */
void drawDigitalStatus() {
    char time[9];
    getTimeString(&currTime, time);
    
    oledC_DrawString(1, 37, 2, 2, time, OLEDC_COLOR_WHITE);    
    
    if(myAppState.ampm_mode == 0 && myAppState.ampm_flag) {
        drawAMPM();
        myAppState.ampm_flag = false;
    }
    if(myAppState.date_flag) {
        drawDate();
        myAppState.date_flag = false;
    }
}

void clearDigitalStatus() {
    int x1, y1, x2, y2;
    y1 = 37;
    y2 = 53;
    
    if(currTime.seconds%10 == 0) {
        x1 = 67;
        x2 = 95;
        oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
    }
    else {
        x1 = 78;
        x2 = 95;
        oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
    }
    if(prevTime.mintues != currTime.mintues) {
        if(currTime.mintues%10 == 0) {
            x1 = 34;
            x2 = 56;
            oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
        }
        else {
            x1 = 45;
            x2 = 56;
            oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
        }
    }   
    if(prevTime.hours != currTime.hours) {
        if(currTime.hours%10 == 0) {
            x1 = 1;
            x2 = 23;
            oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
        }
        else {
            x1 = 12;
            x2 = 23;
            oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
        } 
    }
    
    copyTime(&prevTime, &currTime);
}

void drawDigitalBase() {
    oledC_clearScreen();
    drawDigitalStatus();
    
    if(myAppState.ampm_mode == 0) myAppState.ampm_flag = true;
    myAppState.date_flag = true;
    if(myAppState.alarm == true) drawAlarm();
    else removeAlarm();    
    
    copyTime(&prevTime, &currTime);
}

    /*
     * MENU
     */
void clearMenuStatus() {
    int x1, y1, x2, y2;
    y1 = 88;
    y2 = 95;
    
    if(currTime.seconds%10 == 0) {
        x1 = 84;
        x2 = 95;
        oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
    }
    else {
        x1 = 90;
        x2 = 95;
        oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
    }
    if(prevTime.mintues != currTime.mintues) {
        if(currTime.mintues%10 == 0) {
            x1 = 66;
            x2 = 78;
            oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
        }
        else {
            x1 = 72;
            x2 = 78;
            oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
        }
    }   
    if(prevTime.hours != currTime.hours) {
        if(currTime.hours%10 == 0) {
            x1 = 48;
            x2 = 60;
            oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
        }
        else {
            x1 = 54;
            x2 = 60;
            oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
        } 
    }
    copyTime(&prevTime, &currTime);
    
    if(myAppState.prevMenuOption != myAppState.menuOption) {
        oledC_DrawRectangle(3, 1+(myAppState.prevMenuOption * 11), 9, 9+(myAppState.prevMenuOption * 11), OLEDC_COLOR_BLACK);
        myAppState.prevMenuOption = myAppState.menuOption;
    }
}

void drawMenuStatus() {
    char time[9];
    getTimeString(&currTime, time);
    
    oledC_DrawString(48, 88, 1, 1, time, OLEDC_COLOR_WHITE); 
    oledC_DrawString(3, 1+(myAppState.menuOption * 11), 1, 1, ">", OLEDC_COLOR_WHITE); 
    
    if(myAppState.ampm_mode == 0 && myAppState.ampm_flag) {
        drawAMPM();
        myAppState.ampm_flag = false;
    }
}

void drawMenuBase() {
    oledC_clearScreen();
 
    myAppState.menuOption = 0;
    myAppState.numOfMenuOptions = 5;
    
    oledC_DrawString(12, 1, 1, 1, "Display Mode", OLEDC_COLOR_WHITE);    
    oledC_DrawString(12, 12, 1, 1, "Set Time", OLEDC_COLOR_WHITE);    
    oledC_DrawString(12, 23, 1, 1, "Set Date", OLEDC_COLOR_WHITE);    
    oledC_DrawString(12, 34, 1, 1, "12H/24H", OLEDC_COLOR_WHITE);    
    oledC_DrawString(12, 44, 1, 1, "Alarm", OLEDC_COLOR_WHITE);    
    drawMenuStatus();    
    
    if(myAppState.ampm_mode == 0) myAppState.ampm_flag = true;
    if(myAppState.alarm == true) drawAlarm();
    else removeAlarm();  
}

    /*
     * DISPLAY MENU
     */
void drawDisplayMenuBase() {
    oledC_clearScreen();
    
    myAppState.menuOption = 0;
    myAppState.numOfMenuOptions = 2;
    
    oledC_DrawString(12, 1, 1, 1, "Analog", OLEDC_COLOR_WHITE);    
    oledC_DrawString(12, 12, 1, 1, "Digital", OLEDC_COLOR_WHITE);  
    drawMenuStatus();    
    
    if(myAppState.ampm_mode == 0) myAppState.ampm_flag = true;
    if(myAppState.alarm == true) drawAlarm();
    else removeAlarm();  
}

    /*
     * SET TIME MENU
     */
void drawSetTimeMenuBase() {
    oledC_clearScreen();
    
    myAppState.menuOption = 0;
    myAppState.numOfMenuOptions = 5;
    
    initTime(&setClock);
    copyTime(&setClock, &currTime);
    char time[3];
    char option[16];
    
    conevrtTwoDigToString(setClock.hours, time);
    strcpy(option, "Hours: ");
    option[7] = time[0];
    option[8] = time[1];
    option[9] = '\0';
    oledC_DrawString(12, 1, 1, 1, option, OLEDC_COLOR_WHITE); 
    
    conevrtTwoDigToString(setClock.mintues, time);
    strcpy(option, "Minutes: ");
    option[9] = time[0];
    option[10] = time[1];
    option[11] = '\0';
    oledC_DrawString(12, 12, 1, 1, option, OLEDC_COLOR_WHITE);
    
    conevrtTwoDigToString(setClock.seconds, time);
    strcpy(option, "Seconds: ");
    option[9] = time[0];
    option[10] = time[1];
    option[11] = '\0';
    oledC_DrawString(12, 23, 1, 1, option, OLEDC_COLOR_WHITE);    

    
    oledC_DrawString(12, 34, 1, 1, "Confirm", OLEDC_COLOR_WHITE);
    oledC_DrawString(12, 45, 1, 1, "Back", OLEDC_COLOR_WHITE);
    
    drawMenuStatus();  
    
    if(myAppState.ampm_mode == 0) myAppState.ampm_flag = true;
    if(myAppState.alarm == true) drawAlarm();
    else removeAlarm();  
}

void drawSetTimeStatus() {
    char time[3];
    
    switch(myAppState.menuOption) {
        case 0:
            setClock.hours = (setClock.hours + 1) % 24;
            oledC_DrawRectangle(54, 1, 64, 9, OLEDC_COLOR_BLACK);
            conevrtTwoDigToString(setClock.hours, time);
            oledC_DrawString(54, 1, 1, 1, time, OLEDC_COLOR_WHITE);
            break;
        case 1:
            setClock.mintues = (setClock.mintues + 1) % 60;
            oledC_DrawRectangle(66, 12, 78, 20, OLEDC_COLOR_BLACK);
            conevrtTwoDigToString(setClock.mintues, time);
            oledC_DrawString(66, 12, 1, 1, time, OLEDC_COLOR_WHITE);
            break;
        case 2:
            setClock.seconds = 0;
            oledC_DrawRectangle(66, 23, 78, 9, OLEDC_COLOR_BLACK);
            conevrtTwoDigToString(setClock.seconds, time);
            oledC_DrawString(66, 23, 1, 1, time, OLEDC_COLOR_WHITE);
            break;
        case 3:
            copyTime(&currTime, &setClock);
            myAppState.currMenu = 0;
            drawMenuBase();
            break;
        case 4:
            myAppState.currMenu = 0;
            drawMenuBase();
            break;
        default:
            break;
    }
}

    /*
     * 12H/24H Interval MENU
     */
void drawHourMenuBase() {
    oledC_clearScreen();
    
    myAppState.menuOption = 0;
    myAppState.numOfMenuOptions = 2;
    
    oledC_DrawString(12, 1, 1, 1, "12H Base", OLEDC_COLOR_WHITE);    
    oledC_DrawString(12, 12, 1, 1, "24H Base", OLEDC_COLOR_WHITE);  
    drawMenuStatus();    
    
    if(myAppState.ampm_mode == 0) myAppState.ampm_flag = true;
    if(myAppState.alarm == true) drawAlarm();
    else removeAlarm();
}

/*
 * System Initialization
 */

void menuSelect() {
    /*
     * main menu = 0
     * display menu = 1
     * set time menu = 2
     * hour base menu = 4
     */
    if(myAppState.currMenu == 0) {          // MAIN MENU
        switch(myAppState.menuOption) {
            case 0:
                drawDisplayMenuBase();
                myAppState.currMenu = 1;
                break;
            case 1:
                drawSetTimeMenuBase();
                myAppState.currMenu = 2;
                break;
            case 3:
                drawHourMenuBase();
                myAppState.currMenu = 4;
                break;
        }
    }
    else if(myAppState.currMenu == 1) {     // DISPLAY MENU
        switch(myAppState.menuOption) {
            case 0:
                drawAnalogBase();
                myAppState.mode = analog;
                myAppState.lastClockType = analog;
                drawAnalogBase();
                break;
            case 1:
                myAppState.mode = digital;
                myAppState.lastClockType = digital;
                drawDigitalBase();
                break;
        }  
        myAppState.currMenu = 0;
    }
    else if(myAppState.currMenu == 2) {     // SET TIME MENU
        drawSetTimeStatus();
    }
    else if(myAppState.currMenu == 4) {     // HOUR BASE MENU
        switch(myAppState.menuOption) {
            case 0:
                myAppState.ampm_mode = 0;
                myAppState.ampm_flag = true;
                break;
            case 1:
                myAppState.ampm_mode = 1;
                myAppState.ampm_flag = false;
                break;
        }
        myAppState.currMenu = 0;
        myAppState.mode = myAppState.lastClockType;
        if(myAppState.lastClockType == analog) drawAnalogBase();
        else drawDigitalBase();
    }
}

void __attribute__((__interrupt__)) _T1Interrupt(void) {
    if(PORTAbits.RA11 == 0) {
        myAppState.s1_cnt += 1;
    } else myAppState.s1_cnt = 0;
    
    if(myAppState.s1_cnt >= 2) {
        myAppState.s1_cnt = 0;
        
        if(myAppState.mode != menu) {
            myAppState.mode = menu;
            drawMenuBase();       
        } else {
            myAppState.mode = myAppState.lastClockType;
            myAppState.currMenu = 0;
            myAppState.menuOption = 0;
            myAppState.numOfMenuOptions = 5;
            if(myAppState.lastClockType == analog) drawAnalogBase();
            else drawDigitalBase();
        }
}
    
    switch(myAppState.mode) {
        case menu:
            if(myAppState.selectPressed == true) {
                menuSelect();
                myAppState.selectPressed = false;
            }
            if(myAppState.mode == menu) {
                clearMenuStatus();
                drawMenuStatus();
            }
            break;
        case analog:
            clearAnalogStatus();
            drawAnalogStatus();
            break;
        case digital:
            clearDigitalStatus();
            drawDigitalStatus();
            break;

    }

    incTime(&currTime);
    IFS0bits.T1IF = 0; //flag interrupt 
}

void Timer_Initialize(void) {
    // Timer initialize
    T1CONbits.TON = 1;      /* Start Timer */
    T1CONbits.TCKPS = 0b11; /* Select 1:256 Prescaler */
    PR1 = 16595;            /* Count limit */
    IFS0bits.T1IF = 0;      /* Flag interrupt */
    IEC0bits.T1IE = 1;      /* Enable interrupt */
}

void oled_init() {
    oledC_setBackground(OLEDC_COLOR_BLACK); 
    oledC_clearScreen();
}

void btn_init() {
    //LEDS
    TRISAbits.TRISA9 = 0;
    TRISAbits.TRISA8 = 0;
}

/*
                         Main application
 */
int main(void)
{
    // initialize the system
    SYSTEM_Initialize();
    btn_init();
    initAppState(&myAppState);
    initTime(&currTime);
    oled_init();
    drawAnalogBase();
    copyTime(&prevTime, &currTime);    
    Timer_Initialize();
    
    
    while(1){
        int i;
        
        if(PORTAbits.RA11 == 0) {
            LATAbits.LATA8 = 1;
                
            if(myAppState.mode == menu && myAppState.prevMenuOption == myAppState.menuOption) {
                myAppState.menuOption = (myAppState.menuOption + 1)%myAppState.numOfMenuOptions;
            }
            for(i=0; i<100; i++);
            LATAbits.LATA8 = 0;
        }
            
        if(PORTAbits.RA12 == 0) {
            LATAbits.LATA9 = 1; 
            
            myAppState.selectPressed = true;
            for(i=0; i<100; i++);
            LATAbits.LATA9 = 0;
        }
    }
  
  return 1;
}
/**
 End of File
*/
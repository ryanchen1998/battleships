#include "system.h"
#include "pacer.h"
#include "tinygl.h"
#include "task.h"
#include <stdio.h>
#include "timer.h"
// #include "../fonts/font5x7_1.h"
#include "../fonts/font3x5_1.h"
#include <stdbool.h>
#include "navswitch.h"
#include "lights.h"
#include "setupGame.h"
#include "shoot.h"

#define DISPLAY_TASK_RATE 250
#define SOLID_LED_RATE 250
#define FLASHING_LED_RATE 250
#define WAIT_TIME 250
#define PACER_RATE 500

int checkWin(void)
{
    return (numSolidAtk >= TOTALSHIPSIZE);
}

void displayWin(void)
{
    // tinygl_text_speed_set (10);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_font_set (&font3x5_1);
    tinygl_clear();
    tinygl_text("  YOU WIN");
    // tinygl_text("!H!");
    while(1) {
        pacer_wait();
        tinygl_update();
    }
}

int checkLoss(void)
{
    return (numFlashingDef >= TOTALSHIPSIZE);
}

void displayLoss(void)
{
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_font_set (&font3x5_1);
    tinygl_clear();
    tinygl_text("  YOU LOSE");
    // tinygl_text("!H!");
    while(1) {
        pacer_wait();
        tinygl_update();
    }
}

int main (void)
{
    //solidPointsAtk[0] = tinygl_point(4,6); solidPointsAtk[1] = tinygl_point(3,6);
    // solidPointsAtk[2] = tinygl_point(2,4); solidPointsAtk[3] = tinygl_point(3,3);
    //numSolidAtk += 2; //numSolidAtk += 2;
    //
    // flashingPointsAtk[0] = tinygl_point(1,5); flashingPointsAtk[1] = tinygl_point(3,0);
    // flashingPointsAtk[2] = tinygl_point(0,3); flashingPointsAtk[3] = tinygl_point(4,3);
    // numFlashingAtk += 4;

    //Initialisation
    system_init ();
    pacer_init (PACER_RATE);
    tinygl_init (DISPLAY_TASK_RATE);
    timer_init();
    navswitch_init();
    button_init();
    ir_uart_init();

    int newShotFlashes = 0;
    tinygl_point_t newShot = tinygl_point(0, 0);
    int solidUpdated = 1;
    int currentScreen;

    // int slave = 1;


    placeShips();
    masterSlave();
    if (slave) {
        currentScreen = DEF;
    } else {
        currentScreen = ATK;
    }

    // tinygl_font_set (&font5x7_1);
    // if (slave) {
    //     tinygl_text("S");
    // } else {
    //     tinygl_text("M");
    // }
    // tinygl_update();

    Shot shot = createShot(2, 3);

    int buttonCounter = 0;
    int waitingOnReply = 0;

    // displayWin();

    while (1)
        {
            pacer_wait ();
            navswitch_update ();
            button_update();

            //TODO: Make all these (solid + flashing) update at 250hz
            tinygl_update ();
            if (solidUpdated) {
                if (currentScreen == ATK) {
                    tinygl_clear();
                    // hidePoints(solidPointsDef, numSolidDef);
                    displayPoints(solidPointsAtk, numSolidAtk);
                } else if (currentScreen == DEF) {
                    tinygl_clear();
                    // hidePoints(solidPointsAtk, numSolidAtk);
                    displayPoints(solidPointsDef, numSolidDef);
                }

                solidUpdated = 0;
                // solidUpdated = 1;
            }

            //Flashes the flashing lights
            flashLights(currentScreen);
            // flashLights(DEF);

            //Quick flashes the new light/shot
            if (newShotFlashes > 0) {
                if (fastFlash(newShot)) {
                    newShotFlashes--;
                }
                if (newShotFlashes == 0) {
                    addPoint(newShot, FLASHING, DEF); //DEF hardcoded rn
                }
            }

            // if (buttonCounter == 2) {
                if (currentScreen == ATK) {

                    drawShot(shot, 1);

                    if (navswitch_push_event_p (NAVSWITCH_NORTH)) {
                        moveShotNorth(&shot);
                    }
                    if (navswitch_push_event_p (NAVSWITCH_SOUTH)) {
                        moveShotSouth(&shot);
                    }
                    if (navswitch_push_event_p (NAVSWITCH_EAST)) {
                        moveShotEast(&shot);
                    }
                    if (navswitch_push_event_p (NAVSWITCH_WEST)) {
                        moveShotWest(&shot);
                    }
                    displayPoints(solidPointsAtk, numSolidAtk);

                    if (button_push_event_p (0)) {
                        // tinygl_text("B");
                        if (!waitingOnReply) {
                            ir_uart_putc(sendShot(&shot));
                            drawShot(shot, 0);
                            waitingOnReply = 1;
                        }

                        // while(1) {
                            // flashLights(currentScreen);
                            // if (ir_uart_read_ready_p()) {
                            //     tinygl_point_t tinyglShot = tinygl_point(shot.col, shot.row);
                            //     char hit_miss = ir_uart_getc();
                            //     if (hit_miss == 'H') {
                            //         //tinygl_text("H");
                            //         addPoint(tinyglShot, SOLID, ATK);
                            //         // displayPoints(solidPointsDef, numSolidDef);
                            //         currentScreen = DEF;
                            //     } else if (hit_miss == 'M') {
                            //         //tinygl_text("M");
                            //         addPoint(tinyglShot, FLASHING, ATK);
                            //         // displayPoints(flashingPointsDef, numFlashingDef);
                            //         currentScreen = DEF;
                            //     }
                            //     // switchScreen(DEF);
                            //
                            //     break;
                            // }
                        // }

                    }
                    if (waitingOnReply) {
                        if (ir_uart_read_ready_p()) {
                            tinygl_point_t tinyglShot = tinygl_point(shot.col, shot.row);
                            char hit_miss = ir_uart_getc();
                            if (hit_miss == 'H') {
                                //tinygl_text("H");
                                addPoint(tinyglShot, SOLID, ATK);
                                // displayPoints(solidPointsDef, numSolidDef);
                                solidUpdated = 1;
                                currentScreen = DEF;
                            } else if (hit_miss == 'M') {
                                //tinygl_text("M");
                                addPoint(tinyglShot, FLASHING, ATK);
                                // displayPoints(flashingPointsDef, numFlashingDef);
                                solidUpdated = 1;
                                currentScreen = DEF;
                            }

                            if (checkWin()) {
                                displayWin();
                                break;
                            }
                            // switchScreen(DEF);
                            waitingOnReply = 0;
                            // break;
                        }
                    }
                }
                tinygl_update ();

                if (currentScreen == DEF) {
                   // addPoint(tinygl_point(3, 3) , SOLID, DEF);
                   //displayPoints(solidPointsDef, numSolidDef);
                   if (ir_uart_read_ready_p()) {
                       char coord = ir_uart_getc();
                       shot.col = coord & 0x0F;
                       shot.row = (coord & 0xF0) >> 4;

                       //drawShot(shot, 1);

                       tinygl_point_t tinyglShot = tinygl_point(shot.col, shot.row);
                       if (in(tinyglShot, SOLID, DEF)) {
                           ir_uart_putc('H');
                           currentScreen = ATK;
                           solidUpdated = 1;
                           addPoint(tinyglShot, FLASHING, DEF);
                           shot.col = 2; shot.row = 3;

                           if (checkLoss()) {
                               displayLoss();
                               break;
                           }
                       } else {
                           ir_uart_putc('M');
                           currentScreen = ATK;
                           solidUpdated = 1;
                           shot.col = 2; shot.row = 3;
                           //Flash shot, don't add
                           // addPoint(tinyglShot, FLASHING, DEF);
                       }

                       // shot.col = 2; shot.row = 3;
                   }
               }
               tinygl_update ();

            //     buttonCounter = 2;
            // } else {buttonCounter++;}








            // if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
            //     placeShips();
            //
            //     // displayPoints(solidPointsAtk, numSolidAtk);
            //     if (in(tinygl_point(0, 0), SOLID, DEF)) {
            //         addPoint(tinygl_point(4,6), SOLID, DEF);
            //     }
            //     //displayPoints(solidPointsDef, numSolidDef);
            //     solidUpdated = 1;
            // }
       }
}

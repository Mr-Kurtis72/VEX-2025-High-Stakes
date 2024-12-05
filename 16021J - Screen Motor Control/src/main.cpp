#include "main.h"
#include "liblvgl/llemu.hpp"
#include "pros/adi.h"
#include "pros/adi.hpp"
#include "pros/motors.hpp"


pros::Motor testmotor(1);
pros::adi::DigitalOut pneumatics('A');

/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */



float motorvel = 127 * 1;



std::string leftButtonText = "Left button was pressed!";
std::string centerButtonText = "Center button was pressed!";
std::string rightButtonText = "Right button was pressed!";

void on_left_button() { 
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
        testmotor.move(-motorvel);
	} else testmotor.brake();
}
void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
        pneumatics.set_value(HIGH);
        
	} else pneumatics.set_value(LOW);

    //testmotor.move(motorvel);
}
void on_right_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(4, rightButtonText);
	} else {
		pros::lcd::clear_line(4);
	}

    //testmotor.set_brake_mode()
}


// initialize function. Runs on program startup
void initialize() {
    pros::lcd::initialize(); // initialize brain screen

    pros::lcd::register_btn0_cb(on_left_button); //left button
    pros::lcd::register_btn1_cb(on_center_button); //center button
    pros::lcd::register_btn2_cb(on_right_button); //right button

    // print position to brain screen
    pros::Task screen_task([&]() {
        while (true) {
            // print robot location to the brain screen
            pros::lcd::print(0, "Buttons Bitmap: %d\n", pros::lcd::read_buttons());

            pros::delay(20);
        }
    });
}
void disabled() {}


void competition_initialize() {}


void autonomous() {}


pros::Controller controller(pros::E_CONTROLLER_MASTER);

void opcontrol() {
    // loop forever
    while (true) {
        // get left y and right x positions
        int leftY = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        int rightX = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);


        // delay to save resources
        pros::delay(25);
    }
}
#include "main.h"
#include "lemlib/api.hpp"
#include "lemlib/chassis/trackingWheel.hpp"
#include "pros/abstract_motor.hpp"
#include "pros/misc.h"
#include <cstdio>
#include <unordered_map>


pros::Controller controller(pros::E_CONTROLLER_MASTER);

static std::unordered_map<std::string, pros::controller_digital_e_t> buttonMap = {
    {"Stake Lock", pros::E_CONTROLLER_DIGITAL_L1},
    {"Conveyer Spin", pros::E_CONTROLLER_DIGITAL_R1},
    {"Reverse Conveyer Spin", pros::E_CONTROLLER_DIGITAL_R2}
};

int intake_velocity = 127;


//pneumatic control
pros::adi::DigitalOut stake_lock('E');

//intake motor
pros::Motor intake_motor1 (1);
pros::Motor intake_motor2 (5);


// left motor group
pros::MotorGroup left_motor_group({-7, -6}, pros::MotorGears::blue);
// right motor group
pros::MotorGroup right_motor_group({18, 19}, pros::MotorGears::blue);

// drivetrain settings
lemlib::Drivetrain drivetrain(&left_motor_group, // left motor group
                              &right_motor_group, // right motor group
                              12.75, // 10 inch track width
                              lemlib::Omniwheel::NEW_325, // using new 4" omnis
                              360, // drivetrain rpm is 360
                              2 // horizontal drift is 2 (for now)
);

// imu
pros::Imu imu(11);
// horizontal tracking wheel encoder
pros::adi::Encoder horizontal_encoder('A', 'B', false);
// vertical tracking wheel encoder
pros::adi::Encoder vertical_encoder('G', 'H', false);
// horizontal tracking wheel
lemlib::TrackingWheel horizontal_tracking_wheel(&horizontal_encoder, lemlib::Omniwheel::NEW_275, 0);
// vertical tracking wheel
lemlib::TrackingWheel vertical_tracking_wheel(&vertical_encoder, lemlib::Omniwheel::NEW_275, 0);

// odometry settings
lemlib::OdomSensors sensors(&vertical_tracking_wheel, // vertical tracking wheel 1, set to null
                            nullptr, // vertical tracking wheel 2, set to nullptr as we are using IMEs
                            &horizontal_tracking_wheel, // horizontal tracking wheel 1
                            nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
                            &imu // inertial sensor
);

// lateral PID controller
lemlib::ControllerSettings lateral_controller(10, // proportional gain (kP)
                                              0, // integral gain (kI)
                                              3, // derivative gain (kD)
                                              3, // anti windup
                                              1, // small error range, in inches
                                              100, // small error range timeout, in milliseconds
                                              3, // large error range, in inches
                                              500, // large error range timeout, in milliseconds
                                              20 // maximum acceleration (slew)
);

// angular PID controller
lemlib::ControllerSettings angular_controller(2, // proportional gain (kP)
                                              0, // integral gain (kI)
                                              10, // derivative gain (kD)
                                              3, // anti windup
                                              1, // small error range, in degrees
                                              100, // small error range timeout, in milliseconds
                                              3, // large error range, in degrees
                                              500, // large error range timeout, in milliseconds
                                              0 // maximum acceleration (slew)
);

// create the chassis
lemlib::Chassis chassis(drivetrain, // drivetrain settings
                        lateral_controller, // lateral PID settings
                        angular_controller, // angular PID settings
                        sensors // odometry sensors
);


//ran as a task; enables or disables the stake lock upon button press
void toggle_stake_lock(){
    static bool pressed = false;
    while (true){
        if(controller.get_digital(buttonMap["Stake Lock"])){
            pressed = !pressed;
            stake_lock.set_value(pressed);

            pros::c::delay(250);
        }
        pros::c::delay(25);   
    }
}

void conveyer_spin(){
    //static bool pressed = false;
    while(true){
        if(controller.get_digital(buttonMap["Conveyer Spin"])){
            intake_motor1.move(intake_velocity);
            intake_motor2.move(intake_velocity);
        } else if(controller.get_digital(buttonMap["Reverse Conveyer Spin"])){
            intake_motor1.move(-intake_velocity);
            intake_motor2.move(-intake_velocity);
        } else{
            intake_motor1.brake();
            intake_motor2.brake();
        }
        pros::c::delay(25);
    }
}

// initialize function. Runs on program startup
void initialize() {
    pros::lcd::initialize(); // initialize brain screen
    chassis.calibrate(); // calibrate sensors
    chassis.setPose(-60, 0, 90);// set chassis pose

    // print position to brain screen
    pros::Task screen_task([&]() {
        while (true) {
            // print robot location to the brain screen
            pros::lcd::print(0, "X: %f", chassis.getPose().x); // x
            pros::lcd::print(1, "Y: %f", chassis.getPose().y); // y
            pros::lcd::print(2, "Theta: %f", chassis.getPose().theta); // heading
            // delay to save resources
            pros::delay(25);
        }
    });
}



/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */
void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(3, "I was pressed!");
	} else {
		pros::lcd::clear_line(3);
	}
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */

// path file name is "example.txt".
// "." is replaced with "_" to overcome c++ limitations
ASSET(path_txt);

void autonomous() {
    
    // lookahead distance: 15 inches
    // timeout: 2000 ms
    chassis.follow(path_txt, 15, 10000);
    // follow the next path, but with the robot going backwards
}
/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */

void opcontrol() {
    pros::Task stake_lock_task(toggle_stake_lock);
    pros::Task conveyer_spin_task(conveyer_spin);

    // loop forever
    while (true) {
        // get left y and right x positions
        int leftY = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        int rightX = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);

        // move the robot
        chassis.arcade(leftY, rightX);


        //stake_lock.set_value(controller.get_digital(buttonMap["Stake Lock"]));

        // delay to save resources
        pros::delay(25);
    }
}
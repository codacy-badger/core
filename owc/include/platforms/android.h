#pragma once

#include <android_native_app_glue.h>
#include "template/enum.h"

namespace oi {

	namespace wc {

		class Window;

		struct WindowExt {
			
			struct android_app *app;

			static Window *getWindow(struct android_app *app);
			static void handleCmd(struct android_app *app, int32_t cmd);
			static int32_t handleInput(struct android_app *app, AInputEvent *event);
			static void initDisplay(Window *w);
			static void terminate(Window *w);
		};

		DEnum(KeyExt, int32_t, Undefined = AKEYCODE_UNKNOWN, Zero = AKEYCODE_0, One = AKEYCODE_1, Two = AKEYCODE_2, Three = AKEYCODE_3, Four = AKEYCODE_4, Five = AKEYCODE_5, Six = AKEYCODE_6, Seven = AKEYCODE_7, Eight = AKEYCODE_8, Nine = AKEYCODE_9,
			A = AKEYCODE_A, B = AKEYCODE_B, C = AKEYCODE_C, D = AKEYCODE_D, E = AKEYCODE_E, F = AKEYCODE_F, G = AKEYCODE_G, H = AKEYCODE_H, I = AKEYCODE_I, J = AKEYCODE_J, K = AKEYCODE_K, L = AKEYCODE_L, M = AKEYCODE_M, N = AKEYCODE_N, O = AKEYCODE_O,
			P = AKEYCODE_P, Q = AKEYCODE_Q, R = AKEYCODE_R, S = AKEYCODE_S, T = AKEYCODE_T, U = AKEYCODE_U, V = AKEYCODE_V, W = AKEYCODE_W, X = AKEYCODE_X, Y = AKEYCODE_Y, Z = AKEYCODE_Z, Minus = AKEYCODE_MINUS, Equals = AKEYCODE_EQUALS, Left_brace = AKEYCODE_LEFT_BRACKET,
			Right_brace = AKEYCODE_RIGHT_BRACKET, Semicolon = AKEYCODE_SEMICOLON, Apostrophe = AKEYCODE_APOSTROPHE, Tilde = AKEYCODE_GRAVE, Backslash = AKEYCODE_BACKSLASH, Comma = AKEYCODE_COMMA, Period = AKEYCODE_PERIOD, Slash = AKEYCODE_SLASH, Space = AKEYCODE_SPACE,
			n0 = AKEYCODE_NUMPAD_0, n1 = AKEYCODE_NUMPAD_1, n2 = AKEYCODE_NUMPAD_2, n3 = AKEYCODE_NUMPAD_3, n4 = AKEYCODE_NUMPAD_4, n5 = AKEYCODE_NUMPAD_5, n6 = AKEYCODE_NUMPAD_6, n7 = AKEYCODE_NUMPAD_7, n8 = AKEYCODE_NUMPAD_8, n9 = AKEYCODE_NUMPAD_9,
			Subtract = AKEYCODE_NUMPAD_SUBTRACT, Add = AKEYCODE_NUMPAD_ADD, Decimal = AKEYCODE_NUMPAD_DOT, Multiply = AKEYCODE_NUMPAD_MULTIPLY, Divide = AKEYCODE_NUMPAD_DIVIDE, F1 = AKEYCODE_F1, F2 = AKEYCODE_F2, F3 = AKEYCODE_F3, F4 = AKEYCODE_F4,
			F5 = AKEYCODE_F5, F6 = AKEYCODE_F6, F7 = AKEYCODE_F7, F8 = AKEYCODE_F8, F9 = AKEYCODE_F9, F10 = AKEYCODE_F10, F11 = AKEYCODE_F11, F12 = AKEYCODE_F12, Page_up = AKEYCODE_PAGE_UP, Page_down = AKEYCODE_PAGE_DOWN,
			Home = AKEYCODE_HOME, Insert = AKEYCODE_INSERT, Scroll_lock = AKEYCODE_SCROLL_LOCK, Num_lock = AKEYCODE_NUM_LOCK, Caps_lock = AKEYCODE_CAPS_LOCK, Tab = AKEYCODE_TAB, Enter = AKEYCODE_ENTER, Backspace = AKEYCODE_DEL, Esc = AKEYCODE_ESCAPE,
			Plus = AKEYCODE_PLUS, Left_shift = AKEYCODE_SHIFT_LEFT, Right_shift = AKEYCODE_SHIFT_RIGHT, Left_alt = AKEYCODE_ALT_LEFT, Right_alt = AKEYCODE_ALT_RIGHT,
			Volume_down = AKEYCODE_VOLUME_DOWN, Volume_up = AKEYCODE_VOLUME_UP, Power = AKEYCODE_POWER, Back = AKEYCODE_BACK, Delete = AKEYCODE_FORWARD_DEL,
			Up = AKEYCODE_DPAD_UP, Down = AKEYCODE_DPAD_DOWN, Left = AKEYCODE_DPAD_LEFT, Right = AKEYCODE_DPAD_RIGHT
		);

		//PS4/DS4 implementation
		DEnum(ControllerButtonExt, int32_t, Undefined = AKEYCODE_UNKNOWN, Cross = AKEYCODE_BUTTON_B, Square = AKEYCODE_BUTTON_A, Triangle = AKEYCODE_BUTTON_X, Circle = AKEYCODE_BUTTON_C,
			L1 = AKEYCODE_BUTTON_Y, R1 = AKEYCODE_BUTTON_Z, L3 = AKEYCODE_BUTTON_SELECT, R3 = AKEYCODE_BUTTON_START, Share = AKEYCODE_BUTTON_L2, Options = AKEYCODE_BUTTON_R2
		);

	}

}
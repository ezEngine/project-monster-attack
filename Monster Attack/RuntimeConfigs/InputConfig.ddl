InputAction
{
	string %Set{"Default"}
	string %Action{"MoveForwards"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"keyboard_w"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"MoveBackwards"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"keyboard_s"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"StrafeLeft"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"keyboard_a"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"StrafeRight"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"keyboard_d"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"RotateLeft"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"keyboard_left"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"mouse_move_negx"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"RotateRight"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"keyboard_right"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"mouse_move_posx"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"Run"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"keyboard_left_shift"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"Jump"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"keyboard_space"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"LookUp"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"keyboard_up"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"mouse_move_negy"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"LookDown"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"keyboard_down"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"mouse_move_posy"}
		float %Scale{1}
	}
}

InputAction
{
	string %Set{"Default"}
	string %Action{"MoveForwards"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_w"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_leftstick_posy"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"MoveBackwards"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_s"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_leftstick_negy"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"StrafeLeft"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_a"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_leftstick_negx"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"StrafeRight"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_d"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_leftstick_posx"}
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
	Slot
	{
		string %Key{"controller0_rightstick_negx"}
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
	Slot
	{
		string %Key{"controller0_rightstick_posx"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"Run"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_left_shift"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_button_x"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"Jump"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_space"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_button_a"}
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
	Slot
	{
		string %Key{"controller0_rightstick_posy"}
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
	Slot
	{
		string %Key{"controller0_rightstick_negy"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"Shoot"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_right_ctrl"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"mouse_button_0"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_right_shoulder"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"Select_MagicBullet"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_1"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_pad_up"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"Select_SpikeTrap"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_2"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_pad_right"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"Select_ArrowTrap"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_3"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_pad_left"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"Select_TarTrap"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_4"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_pad_down"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"StartRound"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_g"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_button_start"}
		float %Scale{1}
	}
}

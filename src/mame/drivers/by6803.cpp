// license:BSD-3-Clause
// copyright-holders:Robbbert, Quench
/********************************************************************************************

    PINBALL
    Bally MPU A084-91786-AH06 (6803)

There are no dipswitches; everything is done with a numeric keypad located just inside the
door. The system responds with messages on the display.

ToDo:
- Everything
- Fails PIA test
- Artwork
- Operator keypad
- Various sound boards
- Inputs, Solenoids vary per game
- Mechanical

*********************************************************************************************/


#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6800/m6801.h"
//#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
//#include "audio/midway.h"

#define VERBOSE 1
#include "logmacro.h"

#include "by6803.lh"


class by6803_state : public genpin_class
{
public:
	by6803_state(machine_config const &mconfig, device_type type, char const *tag)
		: by6803_state(mconfig, type, tag, s_solenoid_features_default)
	{ }

	void init_by6803()   { m_outhole[0] = 0x0080; }
	void init_eballchp() { m_outhole[0] = 0x0080; }


	DECLARE_INPUT_CHANGED_MEMBER(activity_button);
	DECLARE_INPUT_CHANGED_MEMBER(self_test);
	DECLARE_CUSTOM_INPUT_MEMBER(outhole);
	DECLARE_CUSTOM_INPUT_MEMBER(spinner);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x0);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x1);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x2);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x3);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x4);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x5);

	void by6803(machine_config &config);

protected:
	typedef uint8_t solenoid_feature_data[20][8];

	by6803_state(machine_config const &mconfig, device_type type, char const *tag, solenoid_feature_data const &solenoid_features)
		: genpin_class(mconfig, type, tag)
		, m_solenoid_features(solenoid_features)
		, m_maincpu(*this, "maincpu")
		, m_pia0(*this, "pia0")
		, m_pia1(*this, "pia1")
		, m_zero_crossing_freq_timer(*this, "timer_z_freq")
		, m_zero_crossing_active_timer(*this, "timer_z_pulse")
		, m_io_test(*this, "TEST")
		, m_io_x0(*this, "X0")
		, m_io_x1(*this, "X1")
		, m_io_x2(*this, "X2")
		, m_io_x3(*this, "X3")
		, m_io_x4(*this, "X4")
		, m_io_x5(*this, "X5")
		, m_io_zx(*this, "ZX")
		, m_io_outhole(*this, "OUTHOLE")
		, m_io_spinner(*this, "SPINNER")
		, m_lamps_a(*this, "lampA%u", 0U)
		, m_lamps_b(*this, "lampB%u", 0U)
		, m_display(*this, "digit%u%u", 0U, 0U)
		, m_solenoids(*this, "solenoid%u", 0U)
		, m_spinners(*this, "spinner%u", 0U)
		, m_scrn_switches(*this, "switch%u", 0U)
	{ }


	DECLARE_READ8_MEMBER(port1_r);
	DECLARE_WRITE8_MEMBER(port1_w);
	DECLARE_READ8_MEMBER(port2_r);
	DECLARE_WRITE8_MEMBER(port2_w);
	DECLARE_READ8_MEMBER(pia0_a_r);
	DECLARE_WRITE8_MEMBER(pia0_a_w);
	DECLARE_READ8_MEMBER(pia0_b_r);
	DECLARE_WRITE8_MEMBER(pia0_b_w);
	DECLARE_READ8_MEMBER(pia1_a_r);
	DECLARE_WRITE8_MEMBER(pia1_a_w);
	DECLARE_WRITE8_MEMBER(pia1_b_w);
	DECLARE_READ_LINE_MEMBER(pia0_ca1_r);
	DECLARE_READ_LINE_MEMBER(pia0_cb1_r);
	DECLARE_WRITE_LINE_MEMBER(pia0_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia0_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(pia1_cb2_w);

	virtual void machine_reset() override;
	virtual void machine_start() override;

	//TIMER_DEVICE_CALLBACK_MEMBER(pia0_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_z_freq);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_z_pulse);

	void by6803_io(address_map &map);
	void by6803_map(address_map &map);

	uint8_t m_pia0_a;
	uint8_t m_pia0_b;
	uint8_t m_pia1_a;
	uint8_t m_pia1_b;

	static solenoid_feature_data const s_solenoid_features_default;


private:
	bool m_pia0_ca2;
	bool m_pia0_cb1;
	bool m_pia0_cb2;
	bool m_pia1_ca1;
	bool m_pia1_ca2;
	bool m_pia1_cb2;
	bool m_zerocross_phase;

	uint8_t m_lamp_decode;
	solenoid_feature_data const &m_solenoid_features;
	uint8_t m_digit;
	uint8_t m_segment[7];

	uint8_t m_port1, m_port2;
	uint8_t m_io_hold_x[6];       // Holds switches closed (drop targets, balls in outholes/saucers/kickbacks etc). Solenoid activity releases the switch.
	uint16_t m_outhole[2];        // Optionally put a ball in the outhole(s) at power on. Trough switches can be is a separate part of the switch matrix.
	uint16_t m_spinner[4];        // Helps animate a simulated Spinner on the playfield layout
	uint8_t m_disp_key[49];       // Helps re-assign keys on the display layouts switch matrix. Use ASCII values
	//uint8_t m_digit;
	//uint8_t m_segment;
	required_device<m6803_cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_device<timer_device> m_zero_crossing_freq_timer;
	required_device<timer_device> m_zero_crossing_active_timer;
	required_ioport m_io_test;
	required_ioport m_io_x0;
	required_ioport m_io_x1;
	required_ioport m_io_x2;
	required_ioport m_io_x3;
	required_ioport m_io_x4;
	required_ioport m_io_x5;
	required_ioport m_io_zx;
	required_ioport m_io_outhole;
	required_ioport m_io_spinner;
	output_finder<15 * 3> m_lamps_a;
	output_finder<15 * 3> m_lamps_b;
	output_finder<7, 8> m_display;
	output_finder<20> m_solenoids;
	output_finder<3> m_spinners;
	output_finder<(6 * 8) + 1> m_scrn_switches;
	void update_lamps(u8 lamp_decoder_addr, u8 lamp_decoder_data);
};

class eballchp_state : public by6803_state
{
public:
	eballchp_state(machine_config const &mconfig, device_type type, char const *tag)
		: by6803_state(mconfig, type, tag, s_solenoid_features_eballchp)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_eballchp;
};



void by6803_state::by6803_map(address_map &map)
{
	map(0x0020, 0x0023).rw(m_pia0, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0040, 0x0043).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x17ff).ram().share("nvram"); // 6116 ram
	map(0x8000, 0xffff).rom();
}

void by6803_state::by6803_io(address_map &map)
{
	map(M6801_PORT1, M6801_PORT1).rw(FUNC(by6803_state::port1_r), FUNC(by6803_state::port1_w)); // P10-P17
	map(M6801_PORT2, M6801_PORT2).rw(FUNC(by6803_state::port2_r), FUNC(by6803_state::port2_w)); // P20-P24
}

static INPUT_PORTS_START( by6803 )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Self Test") PORT_CHANGED_MEMBER(DEVICE_SELF, by6803_state, self_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Activity")  PORT_CHANGED_MEMBER(DEVICE_SELF, by6803_state, activity_button, 0)

	PORT_START("ZX")                                // The Zero Crossing timer frequency on the MPU board is derived from mains power frequency * 2
	PORT_CONFNAME( 0x01, 0x01, "Mains AC Power")    // It affects audio playback rate and other game timers, so allow users to configure it
	PORT_CONFSETTING(    0x00, "50Hz Frequency")
	PORT_CONFSETTING(    0x01, "60Hz Frequency")

	PORT_START("SPINNER")                           // Allow user to select the simulated rotation speed of Spinners
	PORT_CONFNAME( 0x07, 0x02, "Spinner Simulation")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ))
	PORT_CONFSETTING(    0x01, "Slow Speed")
	PORT_CONFSETTING(    0x02, "Medium Speed")
	PORT_CONFSETTING(    0x03, "High Speed")
	PORT_CONFSETTING(    0x07, "Turbo Speed")

	PORT_START("OUTHOLE")                           // Start machine with or without a ball in the Outhole
	PORT_CONFNAME( 0x01, 0x01, "Outhole State on Power-Up")
	PORT_CONFSETTING(    0x00, "Ball Not in Outhole")
	PORT_CONFSETTING(    0x01, "Ball in Outhole")

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)   PORT_CODE(KEYCODE_ENTER_PAD) // PAD ENTER
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CODE(KEYCODE_0_PAD)     // PAD 0
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)   PORT_CODE(KEYCODE_DEL_PAD)   // PAD KBD/CLR
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)       PORT_CODE(KEYCODE_PLUS_PAD)  // PAD GAME
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by6803_state, outhole, (void *)0x0080)  // PORT_CODE(KEYCODE_BACKSPACE)
//	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_CODE(KEYCODE_6)           PORT_CODE(KEYCODE_3_PAD)     // PAD 3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5)           PORT_CODE(KEYCODE_2_PAD)     // PAD 2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CODE(KEYCODE_7)           PORT_CODE(KEYCODE_1_PAD)     // PAD 1
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)       PORT_CODE(KEYCODE_SLASH_PAD) // PAD A
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_TILT2 ) PORT_NAME("Slam Tilt")         PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)

	// from here, vary per game
	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)       PORT_CODE(KEYCODE_6_PAD)     // PAD 6
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)           PORT_CODE(KEYCODE_5_PAD)     // PAD 5
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)           PORT_CODE(KEYCODE_4_PAD)     // PAD 4
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)           PORT_CODE(KEYCODE_ASTERISK)  // PAD B
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)           PORT_CODE(KEYCODE_9_PAD)     // PAD 9
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)           PORT_CODE(KEYCODE_8_PAD)     // PAD 8
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)           PORT_CODE(KEYCODE_7_PAD)     // PAD 7
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)           PORT_CODE(KEYCODE_MINUS_PAD) // PAD C
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD)
INPUT_PORTS_END

static INPUT_PORTS_START( eballchp )
	PORT_INCLUDE( by6803 )

	PORT_MODIFY("X0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by6803_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_X)

	PORT_MODIFY("X3")   /* Drop Targets and Spinner */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by6803_state, switch_hold_x3, (void *)0x0301)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by6803_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by6803_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by6803_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by6803_state, switch_hold_x3, (void *)0x0340)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by6803_state, spinner,        (void *)0x1380)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END



INPUT_CHANGED_MEMBER( by6803_state::activity_button )
{
	if(newval)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( by6803_state::self_test )
{
///	LOG("Selftest Write=%02x\n", newval);

	m_pia0->ca1_w(newval);
}

READ_LINE_MEMBER( by6803_state::pia0_ca1_r )
{
///	LOG("Selftest Read=%02x\n", m_io_test->read() & 0x01);
	return (m_io_test->read() & 0x01);
}
READ_LINE_MEMBER( by6803_state::pia0_cb1_r )
{
	return m_pia0_cb1;
}

CUSTOM_INPUT_MEMBER( by6803_state::spinner)
{
	uint16_t data = (uintptr_t)param & 0xffff;
	uint8_t bit_mask = data & 0xff;
	uint8_t port = (data >> 8) & 0x07;
	uint8_t spinner_num = ((data >> 12) - 1) & 0x03;
	bool state = false;


	switch (data & 0x7ff)
	{
		case 0x008:  if (machine().input().code_pressed(KEYCODE_SLASH)) state = true; break;  // X0:I3
		case 0x010:  if (machine().input().code_pressed(KEYCODE_STOP))  state = true; break;  // X0:I4
		case 0x040:  if (machine().input().code_pressed(KEYCODE_L))     state = true; break;  // X0:I6
		case 0x120:  if (machine().input().code_pressed(KEYCODE_COLON)) state = true; break;  // X1:I5
		case 0x140:  if (machine().input().code_pressed(KEYCODE_L))     state = true; break;  // X1:I6
		case 0x201:  if (machine().input().code_pressed(KEYCODE_COMMA)) state = true; break;  // X2:I0
		case 0x202:  if (machine().input().code_pressed(KEYCODE_M))     state = true; break;  // X2:I1
		case 0x208:  if (machine().input().code_pressed(KEYCODE_B))     state = true; break;  // X2:I3
		case 0x210:  if (machine().input().code_pressed(KEYCODE_V))     state = true; break;  // X2:I4
		case 0x301:  if (machine().input().code_pressed(KEYCODE_K))     state = true; break;  // X3:I0
		case 0x304:  if (machine().input().code_pressed(KEYCODE_H))     state = true; break;  // X3:I0
		case 0x320:  if (machine().input().code_pressed(KEYCODE_D))     state = true; break;  // X3:I5
		case 0x340:  if (machine().input().code_pressed(KEYCODE_S))     state = true; break;  // X3:I6
		case 0x380:  if (machine().input().code_pressed(KEYCODE_A))     state = true; break;  // X3:I7
		case 0x401:  if (machine().input().code_pressed(KEYCODE_O))     state = true; break;  // X4:I0
		case 0x402:  if (machine().input().code_pressed(KEYCODE_I))     state = true; break;  // X4:I1
		case 0x480:  if (machine().input().code_pressed(KEYCODE_Q))     state = true; break;  // X4:I7
		default:     break;
	}

	/* Here we simulate the Spinner */
	if (state)  // Rotate the animated Spinner in the Layout File
	{
		uint8_t spin_speed = m_io_spinner->read();

		if (spin_speed)
			m_spinner[spinner_num] = (m_spinner[spinner_num] + spin_speed) & 0x0fff;    // Animate rotating spinner
		else if ((m_io_hold_x[port] & bit_mask) == 0x00)
			m_spinner[spinner_num] = 0x1100;                                            // Flip the spinner once on the closure of its switch

		m_io_hold_x[port] |= bit_mask;
		m_spinners[spinner_num] = (m_spinner[spinner_num] >> 5) & 0x0f;
	}
	else
	{
		m_io_hold_x[port] &= ~(bit_mask);
		if (m_spinner[spinner_num])
			m_spinners[spinner_num] = 0;
		m_spinner[spinner_num] = 0x0000;
	}

	return (((m_io_hold_x[port] & bit_mask) && (m_spinner[spinner_num] & 0x1040)) ? true : false);
}

CUSTOM_INPUT_MEMBER( by6803_state::outhole )
{
	uint8_t bit_mask = ((uintptr_t)param & 0xff);
	uint8_t port = (((uintptr_t)param >> 8) & 0x07);

	/* Here we simulate the ball sitting in the Outhole so the Outhole Solenoid can release it */

	if (machine().input().code_pressed_once(KEYCODE_BACKSPACE))  m_io_hold_x[port] |= bit_mask;

	return ((m_io_hold_x[port] & bit_mask) ? true : false);
}


CUSTOM_INPUT_MEMBER( by6803_state::switch_hold_x0 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x01) && machine().input().code_pressed_once(KEYCODE_BACKSLASH))   m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x01) && machine().input().code_pressed_once(KEYCODE_ENTER_PAD))   m_io_hold_x[0] |= bit_mask;  // PAD ENTER
	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_CLOSEBRACE))  m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_0_PAD))       m_io_hold_x[0] |= bit_mask;  // PAD 0
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_OPENBRACE))   m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_DEL_PAD))     m_io_hold_x[0] |= bit_mask;  // PAD KBD/CLR
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_SLASH))       m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_PLUS_PAD))    m_io_hold_x[0] |= bit_mask;  // PAD GAME
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_STOP))        m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x40) && machine().input().code_pressed_once(KEYCODE_L))           m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x80) && machine().input().code_pressed_once(KEYCODE_BACKSPACE))   m_io_hold_x[0] |= bit_mask;

	return ((m_io_hold_x[0] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( by6803_state::switch_hold_x1 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_ENTER))  m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_QUOTE))  m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x80) && machine().input().code_pressed_once(KEYCODE_COLON))  m_io_hold_x[1] |= bit_mask;

	return ((m_io_hold_x[1] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( by6803_state::switch_hold_x2 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x01) && machine().input().code_pressed_once(KEYCODE_COMMA))     m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x01) && machine().input().code_pressed_once(KEYCODE_6_PAD))     m_io_hold_x[2] |= bit_mask;  // PAD 6
	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_M))         m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_5_PAD))     m_io_hold_x[2] |= bit_mask;  // PAD 5
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_N))         m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_4_PAD))     m_io_hold_x[2] |= bit_mask;  // PAD 4
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_B))         m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_ASTERISK))  m_io_hold_x[2] |= bit_mask;  // PAD B
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_V))         m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x20) && machine().input().code_pressed_once(KEYCODE_C))         m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x40) && machine().input().code_pressed_once(KEYCODE_X))         m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x80) && machine().input().code_pressed_once(KEYCODE_Z))         m_io_hold_x[2] |= bit_mask;

	return ((m_io_hold_x[2] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( by6803_state::switch_hold_x3 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x01) && machine().input().code_pressed_once(KEYCODE_K))          m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x01) && machine().input().code_pressed_once(KEYCODE_9_PAD))      m_io_hold_x[3] |= bit_mask;  // PAD 9
	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_J))          m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_8_PAD))      m_io_hold_x[3] |= bit_mask;  // PAD 8
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_H))          m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_7_PAD))      m_io_hold_x[3] |= bit_mask;  // PAD 7
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_G))          m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_MINUS_PAD))  m_io_hold_x[3] |= bit_mask;  // PAD C
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_F))          m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x20) && machine().input().code_pressed_once(KEYCODE_D))          m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x40) && machine().input().code_pressed_once(KEYCODE_S))          m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x80) && machine().input().code_pressed_once(KEYCODE_A))          m_io_hold_x[3] |= bit_mask;

	return ((m_io_hold_x[3] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( by6803_state::switch_hold_x4 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x01) && machine().input().code_pressed_once(KEYCODE_O))  m_io_hold_x[4] |= bit_mask;
	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_I))  m_io_hold_x[4] |= bit_mask;
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_U))  m_io_hold_x[4] |= bit_mask;
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_Y))  m_io_hold_x[4] |= bit_mask;
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_R))  m_io_hold_x[4] |= bit_mask;
	if ((bit_mask & 0x20) && machine().input().code_pressed_once(KEYCODE_E))  m_io_hold_x[4] |= bit_mask;
	if ((bit_mask & 0x40) && machine().input().code_pressed_once(KEYCODE_W))  m_io_hold_x[4] |= bit_mask;
	if ((bit_mask & 0x80) && machine().input().code_pressed_once(KEYCODE_Q))  m_io_hold_x[4] |= bit_mask;

	return ((m_io_hold_x[4] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( by6803_state::switch_hold_x5 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x01) && machine().input().code_pressed_once(KEYCODE_1_PAD))  m_io_hold_x[5] |= bit_mask;
	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_2_PAD))  m_io_hold_x[5] |= bit_mask;
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_3_PAD))  m_io_hold_x[5] |= bit_mask;
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_4_PAD))  m_io_hold_x[5] |= bit_mask;
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_5_PAD))  m_io_hold_x[5] |= bit_mask;
	if ((bit_mask & 0x20) && machine().input().code_pressed_once(KEYCODE_6_PAD))  m_io_hold_x[5] |= bit_mask;
	if ((bit_mask & 0x40) && machine().input().code_pressed_once(KEYCODE_7_PAD))  m_io_hold_x[5] |= bit_mask;
	if ((bit_mask & 0x80) && machine().input().code_pressed_once(KEYCODE_8_PAD))  m_io_hold_x[5] |= bit_mask;

	return ((m_io_hold_x[5] & bit_mask) ? true : false);
}



READ8_MEMBER( by6803_state::port1_r )
{
	return m_port1;
}

// P10-17 - goes to peripheral bus
WRITE8_MEMBER( by6803_state::port1_w )
{
	m_port1 = data; // sound data = P10,11,12,13,24; P14-17 unknown
}

READ8_MEMBER( by6803_state::port2_r )
{
	return m_port2;
}

// P20 - input from a phase
// P21 - output to phase circuit
// P22 - LED, connects to reset circuit, could be a watchdog
// P23 - high
// P24 - sound strobe
WRITE8_MEMBER( by6803_state::port2_w )
{
	m_port2 = data;
	output().set_value("led0", BIT(data, 2)); // P22 drives LED
}

// display latch strobes; display blanking
WRITE_LINE_MEMBER( by6803_state::pia0_ca2_w )
{
#if 0                   // Display Blanking - Out of sync with video redraw rate and causes flicker so it's disabled
	if (m_pia0_ca2 && (state == false))
	{
		for (int digit = 1; digit <= 8; digit++)
		{
			m_display[0][digit] = 0;
			m_display[1][digit] = 0;
			m_display[2][digit] = 0;
			m_display[3][digit] = 0;
			m_display[4][digit] = 0;
			m_display[5][digit] = 0;
			m_display[6][digit] = 0;
		}
	}
#endif


	if ((m_pia0_ca2 == false) && state)
	{
		static constexpr uint8_t patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 };  // MC14543 - BCD to 7 Segment Display Decoder

		m_display[0][0] = patterns[0];                   // Nuovo-Bell Games first GI lit digit
		m_display[0][m_digit] = patterns[m_segment[0]];  // Credits/Match/BallInPlay
		m_display[1][m_digit] = patterns[m_segment[1]];  // Player 1
		m_display[2][m_digit] = patterns[m_segment[2]];  // Player 2
		m_display[3][m_digit] = patterns[m_segment[3]];  // Player 3
		m_display[4][m_digit] = patterns[m_segment[4]];  // Player 4
		m_display[5][m_digit] = patterns[m_segment[5]];  // Player 5
		m_display[6][m_digit] = patterns[m_segment[6]];  // Player 6

		LOG("Display Player 0 = %02x: %02x written to digit %01x of Player 0 display. Segment is %02x\n", m_display[0][m_digit], patterns[m_segment[0]], m_digit, m_segment[0]);
	}

	m_pia0_ca2 = state;
}

// lamp strobe 1 when high
WRITE_LINE_MEMBER( by6803_state::pia0_cb2_w )
{
	LOG("New U1 CB2 state %01x, was %01x.   PIA=%02x\n", state, m_pia0_cb2, m_pia0_a);

	if ((state) || ((m_pia0_cb2==true) && (state==false)))
	{
		if (m_lamp_decode != (m_pia0_a & 0x0f))
		{
			if ((m_pia0_a & 0x0f) < 0x0f)
			{
				update_lamps((m_pia0_a & 0x0f), m_pia0_a);
				LOG("ON:  Phase=%02x, Lamp_decode now=%02x, Lamp_decode was %02x,  PIA0-U8-PortA=%02x\n", m_zerocross_phase, (m_pia0_a & 0x0f), m_lamp_decode, m_pia0_a);
			}
		}
		m_lamp_decode = (m_pia0_a & 0x0f);
	}

	m_pia0_cb2 = state;
}

// sol bank select (0 to enable sol selection)
WRITE_LINE_MEMBER( by6803_state::pia1_cb2_w )
{
}

READ8_MEMBER( by6803_state::pia0_a_r )
{
	return m_pia0_a;
}

// d0=p1,2   d1=p3,4   d2=?   d3=?  (active low, also pia0:ca2 must be low)
// d4-7 do digit select; d0-4 switch matrix
// d0-3 lamp rows & d5=0 & pia0:cb2=1 (1st lamp bank)
// d0-3 lamp rows & d6=0 & pia0:cb2=1 (2nd lamp bank)
// d0-3 lamp rows & d7=0 & pia0:cb2=1 (3rd lamp bank)
WRITE8_MEMBER( by6803_state::pia0_a_w )
{
#if 0
// This is all wrong
	switch (data)
	{
		case 0x10: // wrong
			m_digits[m_digit] = m_segment;
			break;
		case 0x1d:
			m_digits[8+m_digit] = m_segment;
			break;
		case 0x1b:
			m_digits[16+m_digit] = m_segment;
			break;
		case 0x07:
			m_digits[24+m_digit] = m_segment;
			break;
		case 0x0f:
			m_digits[32+m_digit] m_segment;
			break;
		default:
			break;
	}
#endif

	// LOG("Writing %02x to U10 PIA, CB2 state is %01x,  CA2 state is %01x, Lamp_Dec is %02x\n",data, m_pia0_cb2, m_pia0_ca2, (m_lamp_decode & 0x0f));

	if (!m_pia0_ca2)
	{
		if (BIT(m_pia1_a, 0) == 0)              // Credit/Ball in Play Display
			m_segment[0] = (data >> 4);
		if (BIT(data, 0) == 0)                  // Player Display 1
			m_segment[1] = (data >> 4);
		if (BIT(data, 1) == 0)                  // Player Display 2
			m_segment[2] = (data >> 4);
		if (BIT(data, 2) == 0)                  // Player Display 3
			m_segment[3] = (data >> 4);
		if (BIT(data, 3) == 0)                  // Player Display 4
			m_segment[4] = (data >> 4);
	}


	if (m_pia0_cb2)
		m_lamp_decode = (data & 0x0f);

	if (m_lamp_decode < 0x0f)
		update_lamps(m_lamp_decode, data);

	m_pia0_a = data;
}

// switch returns
READ8_MEMBER( by6803_state::pia0_b_r )
{
	uint8_t data = 0;

	if (BIT(m_pia0_a, 0))
		data |= m_io_x0->read();

	if (BIT(m_pia0_a, 1))
		data |= m_io_x1->read();

	if (BIT(m_pia0_a, 2))
		data |= m_io_x2->read();

	if (BIT(m_pia0_a, 3))
		data |= m_io_x3->read();

	if (BIT(m_pia0_a, 4))
		data |= m_io_x4->read();

	return data;
}

WRITE8_MEMBER( by6803_state::pia0_b_w )
{
	m_pia0_b = data;
}

READ8_MEMBER( by6803_state::pia1_a_r )
{
	return m_pia1_a;
}

// segment data; d0 & pia0:ca2 = comma; passed to digits when PA0? is high (assume they mean pia0:pa0)
WRITE8_MEMBER( by6803_state::pia1_a_w )
{
	if (!m_pia0_ca2)
	{
		if (BIT(data, 0) == 0)            // Credit/Ball in Play Display
			m_segment[0] = (m_pia0_a >> 4);
	}

	m_digit = 0;

	if (BIT(data, 7))
		m_digit = 1;
	else if (BIT(data, 6))
		m_digit = 2;
	else if (BIT(data, 5))
		m_digit = 3;
	else if (BIT(data, 4))
		m_digit = 4;
	else if (BIT(data, 3))
		m_digit = 5;
	else if (BIT(data, 2))
		m_digit = 6;
	else if (BIT(data, 1))
		m_digit = 7;

	m_pia1_a = data;
#if 0
	m_segment = data >> 1;
#endif
}


WRITE8_MEMBER( by6803_state::pia1_b_w )
{
	if (!m_pia1_cb2)
	{
		if ((data & 0x0f) < 0x0f)   // Momentary Solenoids
		{
			if (m_solenoid_features[(data & 0x0f)][0] != 0xff)     // Play solenoid audio sample
			{
				if (m_solenoids[(data & 0x0f)] == 0)
				{
					m_samples->start(m_solenoid_features[(data & 0x0f)][0], m_solenoid_features[(data & 0x0f)][1]);
				}
			}

			m_solenoids[(data & 0x0f)] = 1;

			if (m_solenoid_features[(data & 0x0f)][3])  // Release/Activate relevant switch after firing Solenoid
			{
				for (int sw_strobe_mask = 0; sw_strobe_mask <= 4; sw_strobe_mask++)
				{
					if (m_solenoid_features[(data & 0x0f)][2] & (1 << sw_strobe_mask) )
					{
						if (m_solenoid_features[(data & 0x0f)][2] & 0x80)  // Upper bit means we want to activate a switch (drop a target)
						{
							m_io_hold_x[sw_strobe_mask] |= (m_solenoid_features[(data & 0x0f)][3]);   // Activate a switch
						}
						else
						{
							m_io_hold_x[sw_strobe_mask] &= ~(m_solenoid_features[(data & 0x0f)][3]);  // Release a switch
						}
					}
				}
			}
		}
		else                        // Rest output - all momentary solenoids are off
		{
			std::fill_n(std::begin(m_solenoids) + 00, 15, false);
		}
	}


#if 0
	if ((m_pia1_b & 0x10) && ((data & 0x10) == 0))
	{
		m_solenoids[16] = 1;
		if (m_solenoid_features[16][0] != 0xff)
			m_samples->start(m_solenoid_features[16][0], m_solenoid_features[16][1]);
	}
	else if ((data & 0x10) && ((m_pia1_b & 0x10) == 0))
	{
		m_solenoids[16] = 0;
		if (m_solenoid_features[16][0] != 0xff)
			m_samples->start(m_solenoid_features[16][0], m_solenoid_features[16][2]);
	}
#endif
	if ((m_pia1_b & 0x20) && ((data & 0x20) == 0))
	{
		m_solenoids[17] = 1;                                // Coin Lockout Coil engage
		if (m_solenoid_features[17][0] != 0xff)
			m_samples->start(m_solenoid_features[17][0], m_solenoid_features[17][1]);
	}
	else if ((data & 0x20) && ((m_pia1_b & 0x20) == 0))
	{
		m_solenoids[17] = 0;                               // Coin Lockout Coil release
		if (m_solenoid_features[17][0] != 0xff)
			m_samples->start(m_solenoid_features[17][0], m_solenoid_features[17][2]);
	}
	if ((m_pia1_b & 0x40) && ((data & 0x40) == 0))
	{
		m_solenoids[18] = 1;                                // Flipper Enable Relay engage
		if (m_solenoid_features[18][0] != 0xff)
			m_samples->start(m_solenoid_features[18][0], m_solenoid_features[18][1]);
	}
	else if ((data & 0x40) && ((m_pia1_b & 0x40) == 0))
	{
		m_solenoids[18] = 0;                               // Flipper Enable Relay release
		if (m_solenoid_features[18][0] != 0xff)
			m_samples->start(m_solenoid_features[18][0], m_solenoid_features[18][2]);
	}
	if ((m_pia1_b & 0x80) && ((data & 0x80) == 0))
	{
		m_solenoids[19] = 1;
		if (m_solenoid_features[19][0] != 0xff)
			m_samples->start(m_solenoid_features[19][0], m_solenoid_features[19][1]);
	}
	else if ((data & 0x80) && ((m_pia1_b & 0x80) == 0))
	{
		m_solenoids[19] = 0;
		if (m_solenoid_features[19][0] != 0xff)
			m_samples->start(m_solenoid_features[19][0], m_solenoid_features[19][2]);
	}

	m_pia1_b = data;
}


void by6803_state::update_lamps( u8 lamp_decoder_addr, u8 lamp_decoder_data)
{
	if (!m_zerocross_phase)
	{
	// Update the Lamps on the phase A if their respective SCRs aren't already latched
		if (!m_lamps_a[(0  + lamp_decoder_addr)]) {
			 m_lamps_a[(0  + lamp_decoder_addr)] = !BIT(lamp_decoder_data, 5);
			LOG("ON:  Phase=%02x, Lamp_decode now=%02x, Lamp_decode was %02x,  PIA0-U8-PortA=%02x\n", m_zerocross_phase, (m_pia0_a & 0x0f), m_lamp_decode, m_pia0_a);
		}
		if (!m_lamps_a[(15 + lamp_decoder_addr)]) {
			 m_lamps_a[(15 + lamp_decoder_addr)] = !BIT(lamp_decoder_data, 6);
			LOG("ON:  Phase=%02x, Lamp_decode now=%02x, Lamp_decode was %02x,  PIA0-U8-PortA=%02x\n", m_zerocross_phase, (m_pia0_a & 0x0f), m_lamp_decode, m_pia0_a);
		}
		if (!m_lamps_a[(30 + lamp_decoder_addr)]) {
			 m_lamps_a[(30 + lamp_decoder_addr)] = !BIT(lamp_decoder_data, 7);
			LOG("ON:  Phase=%02x, Lamp_decode now=%02x, Lamp_decode was %02x,  PIA0-U8-PortA=%02x\n", m_zerocross_phase, (m_pia0_a & 0x0f), m_lamp_decode, m_pia0_a);
		}
	}
	else
	{
	// Update the Lamps on the phase A if their respective SCRs aren't already latched
		if (!m_lamps_b[(0  + lamp_decoder_addr)]) {
			 m_lamps_b[(0  + lamp_decoder_addr)] = !BIT(lamp_decoder_data, 5);
			LOG("ON:  Phase=%02x, Lamp_decode now=%02x, Lamp_decode was %02x,  PIA0-U8-PortA=%02x\n", m_zerocross_phase, (m_pia0_a & 0x0f), m_lamp_decode, m_pia0_a);
		}
		if (!m_lamps_b[(15 + lamp_decoder_addr)]) {
			 m_lamps_b[(15 + lamp_decoder_addr)] = !BIT(lamp_decoder_data, 6);
			LOG("ON:  Phase=%02x, Lamp_decode now=%02x, Lamp_decode was %02x,  PIA0-U8-PortA=%02x\n", m_zerocross_phase, (m_pia0_a & 0x0f), m_lamp_decode, m_pia0_a);
		}
		if (!m_lamps_b[(30 + lamp_decoder_addr)]) {
			 m_lamps_b[(30 + lamp_decoder_addr)] = !BIT(lamp_decoder_data, 7);
			LOG("ON:  Phase=%02x, Lamp_decode now=%02x, Lamp_decode was %02x,  PIA0-U8-PortA=%02x\n", m_zerocross_phase, (m_pia0_a & 0x0f), m_lamp_decode, m_pia0_a);
		}

		// Rest output - all lamps are off
	}
}



void by6803_state::machine_start()
{
	genpin_class::machine_start();

	m_lamps_a.resolve();
	m_lamps_b.resolve();
	m_display.resolve();
	m_solenoids.resolve();
	m_spinners.resolve();
	//m_relay.resolve();
	m_scrn_switches.resolve();

	save_item(NAME(m_pia0_a));
	save_item(NAME(m_pia0_b));
	save_item(NAME(m_pia1_a));
	save_item(NAME(m_pia1_b));
	save_item(NAME(m_io_hold_x[0]));
	save_item(NAME(m_io_hold_x[1]));
	save_item(NAME(m_io_hold_x[2]));
	save_item(NAME(m_io_hold_x[3]));
	save_item(NAME(m_io_hold_x[4]));
	save_item(NAME(m_io_hold_x[5]));
	save_item(NAME(m_pia0_ca2));
	save_item(NAME(m_pia0_cb1));
	save_item(NAME(m_pia0_cb2));
	save_item(NAME(m_pia1_ca1));
	save_item(NAME(m_pia1_ca2));
	save_item(NAME(m_pia1_cb2));
	save_item(NAME(m_zerocross_phase));
}

void by6803_state::machine_reset()
{
	m_pia0_a = 0;
	m_pia0_b = 0xff;
	m_pia0_cb2 = 0;
	m_pia1_a = 0;
	m_pia1_b = 0xff;
	m_port2 = M6801_MODE_2 | 0x18;
	m_zerocross_phase = 0;

	m_io_hold_x[0] = m_io_hold_x[1] = m_io_hold_x[2] = m_io_hold_x[3] = m_io_hold_x[4] = m_io_hold_x[5] = 0;   // Clear the temp switch holds

	if (m_io_outhole->read())           // Put a ball in the Outhole at power-on if selected in the machine configuration
	{
		m_io_hold_x[(m_outhole[0] >> 8) & 0x07] = m_outhole[0] & 0xff;
		if (m_outhole[1] & 0x8000)
			m_io_hold_x[(m_outhole[1] >> 8) & 0x07] = m_outhole[1] & 0xff;
	}

	if (m_io_zx->read())                // Machine configuration sets the Zero Crossing frequency based countries DC rectified mains line frequency
		m_zero_crossing_freq_timer->adjust(attotime::from_hz(120), 0, attotime::from_hz(120));  // Country whose mains line frequency is 60Hz
	else
		m_zero_crossing_freq_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));  // Country whose mains line frequency is 50Hz


	for (int i = 1; i<=40; i++)
	{
		if ((m_disp_key[i] > 0) && (m_disp_key[i] <128))	// Change the switch key characters in the default layout where games need require customisations
			m_scrn_switches[i] = m_disp_key[i];
	}

///	output().set_value("switch24", 47);
///	m_scrn_switches[13] = 65;

	std::fill(std::begin(m_lamps_a), std::end(m_lamps_a), 0);
	std::fill(std::begin(m_lamps_b), std::end(m_lamps_b), 0);
	std::fill_n(std::begin(m_solenoids) + 00, 15, false);
}


TIMER_DEVICE_CALLBACK_MEMBER( by6803_state::timer_z_freq )
{
/*  Zero Crossing Detector - this timing is based on 50Hz AC line power input converted to unregulated DC

    -+                          +---+
     |                          |   |
     |<-------- 9.30ms -------->|<->|700us
     |                          |   |
     +--------------------------+   +-----
*/

	m_zero_crossing_active_timer->adjust(attotime::from_usec(7800));	// 7800

	m_zerocross_phase ^= 1;
	LOG("Phase=%01x\n", m_zerocross_phase);


	if (m_zerocross_phase)
	{									// Phase A  Clear phase B
		std::fill(std::begin(m_lamps_b), std::end(m_lamps_b), 0);	///
		if (!BIT(m_port2, 1))
		{
			m_port2 |= 0x01;
			LOG("Rising ZeroCrossing, Phase=%01x, Asserting M6803_TIN_Pin20\n", m_zerocross_phase);
			m_maincpu->set_input_line(M6801_TIN_LINE, ASSERT_LINE);
		}
	}
	else
	{									// Phase B  Don't clear any phase
		m_pia0_cb1 = true;
		LOG("Rising ZeroCrossing, Phase=%01x, m_pia0_cb1=%01x\n", m_zerocross_phase,m_pia0_cb1);
		m_pia0->cb1_w(m_pia0_cb1);
	}

	/*** Zero Crossing - power to all Lamp SCRs is cut off and reset ***/
}
TIMER_DEVICE_CALLBACK_MEMBER( by6803_state::timer_z_pulse )
{
	/*** Line Power to DC Zero Crossing has ended ***/

	if (m_zerocross_phase)
	{									// Phase A  Don't clear any phase
		if (!BIT(m_port2, 1))
		{
			m_port2 &= 0xfe;
			LOG("Trailing ZeroCrossing, Phase=%01x, Clearing M6803_TIN_Pin20\n", m_zerocross_phase);
			m_maincpu->set_input_line(M6801_TIN_LINE, CLEAR_LINE);
		}
	//	std::fill(std::begin(m_lamps_a), std::end(m_lamps_a), 0);	// This is the real one but causes excessive lamp flicker
	///	std::fill(std::begin(m_lamps_b), std::end(m_lamps_b), 0);
	}
	else
	{									// Phase B  Clear phase A
		m_pia0_cb1 = false;
		LOG("Trailing ZeroCrossing, Phase=%01x, m_pia0_cb1=%01x\n", m_zerocross_phase,m_pia0_cb1);
		m_pia0->cb1_w(m_pia0_cb1);
	//	std::fill(std::begin(m_lamps_b), std::end(m_lamps_b), 0);	// This is the real one but causes excessive lamp flicker
		std::fill(std::begin(m_lamps_a), std::end(m_lamps_a), 0);
	}
}



by6803_state::solenoid_feature_data const by6803_state::s_solenoid_features_default =
{
	// Assign a mechanical audio sample corresponding to a solenoid function, and optionally:
	//  - release any switches being held closed eg. fallen drop targets, ball sitting in outhole/saucer/kickbacks, etc
	//  - activate any switches eg. make a drop target fall, etc
	// The following table serves these functions and is configured on a per game basis:
	// 00-15 { SC, AS, SSB, SRB } Momentary coils
	//                      SRB = Switch Return Bits used to release/assert a switch depending on game specific usage. Bit is active Hi
	//                 SSB      = Switch Strobe Bits that the Return Bits lives within the switch matrix
	//                 SSB        Bit 0x80 Hi indicates we want to activate a switch, Lo indicates we want to release a switch
	//                 SSB        Bit 0x40 Hi indicates we want to change a switch in a different part of the switch matrix - uses: m_outhole[1]
	//             AS           = Audio Sample (mechanical and/or chime) to playback
	//         SC               = MAME Sound Channel to play the audio sample on (make sure samples that might play at the same time are on different channels)
	//         SC                 Value 0xff indicates not to play any audio sample
	//
	//
	// 16-19 { SC, SE, SR, N/U } Constant Coils
	//                     N/U = Not Used
	//                 SR      = Audio Sample Release to playback
	//             SE          = Audio Sample Engage to playback
	//         SC              = MAME Sound Channel to play the audio sample on (make sure samples that might play at the same time are on different channels)
	//         SC                Value 0xff indicates not to play any audio sample
	//
	// 16-19 { SC, AS, SSB, SRB } Constant Coils with some manual Momentary Coils requiring switch manipulation
	//                      SRB = Switch Return Bits used to release/assert a switch depending on game specific usage. Bit is active Hi
	//                 SSB      = Switch Strobe Bits that the Return Bits lives within the switch matrix
	//                 SSB        Bit 0x80 Hi indicates we want to activate a switch, Lo indicates we want to release a switch
	//             AS           = Audio Sample (mechanical and/or chime) to playback
	//         SC               = MAME Sound Channel to play the audio sample on (make sure samples that might play at the same time are on different channels)
	//         SC                 Value 0xff indicates not to play any audio sample
	//
	//      {  SC,   AS,    SSB,  SRB }
	/*00*/  { 0x00, 0x00,  0x00, 0x00 },
	/*01*/  { 0x00, 0x00,  0x00, 0x00 },
	/*02*/  { 0x00, 0x00,  0x00, 0x00 },
	/*03*/  { 0x00, 0x00,  0x00, 0x00 },
	/*04*/  { 0x00, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x00, 0x0a,  0x00, 0x00 },
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },
	/*12*/  { 0x00, 0x0b,  0x00, 0x00 },
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },
	/*14*/  { 0x00, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//      {  SC,   SE,   SR,   N/U  }      // When N/U value is 0x00
	//      {  SC,   AS,    SSB,  SRB }      // When SRB value is 0x00
	/*16*/  { 0x02, 0x05,  0x10, 0x01 },     // Eg. Saucer
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }      // None
};



by6803_state::solenoid_feature_data const eballchp_state::s_solenoid_features_eballchp =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x03, 0x0b,  0x08, 0x01 },     // Drop Target #1/2 Reset Single
	/*01*/  { 0x03, 0x0b,  0x08, 0x04 },     // Drop Target #3   Reset Single
	/*02*/  { 0x03, 0x0b,  0x08, 0x08 },     // Drop Target #4/5 Reset Single
	/*03*/  { 0x03, 0x0b,  0x08, 0x20 },     // Drop Target #6   Reset Single
	/*04*/  { 0x03, 0x0b,  0x08, 0x40 },     // Drop Target #7   Reset Single
	/*05*/  { 0xff, 0x00,  0x00, 0x00 },
	/*06*/  { 0xff, 0x00,  0x00, 0x00 },
	/*07*/  { 0x02, 0x05,  0x01, 0x10 },     // Saucer Upper
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*10*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0xff, 0x00,  0x00, 0x00 },
	/*13*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*14*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};



MACHINE_CONFIG_START(by6803_state::by6803)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M6803, XTAL(3'579'545))
	MCFG_DEVICE_PROGRAM_MAP(by6803_map)
	MCFG_DEVICE_IO_MAP(by6803_io)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_by6803)

	/* Sound */
	genpin_audio(config);

	/* Devices */
	MCFG_DEVICE_ADD("pia0", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, by6803_state, pia0_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, by6803_state, pia0_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(*this, by6803_state, pia0_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by6803_state, pia0_b_w))
	MCFG_PIA_READCA1_HANDLER(READLINE(*this, by6803_state, pia0_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(*this, by6803_state, pia0_cb1_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(*this, by6803_state, pia0_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by6803_state, pia0_cb2_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("maincpu", M6803_IRQ_LINE))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("maincpu", M6803_IRQ_LINE))
	MCFG_TIMER_DRIVER_ADD("timer_z_freq", by6803_state, timer_z_freq)						// Mains Line Frequency * 2 (100Hz or 120Hz depending on country)
	MCFG_TIMER_DRIVER_ADD(m_zero_crossing_active_timer, by6803_state, timer_z_pulse)		// Active pulse length from Zero Crossing detector

	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, by6803_state, pia1_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, by6803_state, pia1_a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by6803_state, pia1_b_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by6803_state, pia1_cb2_w))

	//MCFG_SPEAKER_STANDARD_MONO("speaker")
	//MCFG_DEVICE_ADD("tcs", MIDWAY_TURBO_CHEAP_SQUEAK) // Cheap Squeak Turbo
	//MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 1.0)
MACHINE_CONFIG_END


/*-----------------------------------------------------------
/ Atlantis #2006
/-----------------------------------------------------------*/
ROM_START(atlantis)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u26_cpu.rom", 0x8000, 0x4000, CRC(b98491e1) SHA1(b867e2b24e93c4ee19169fe93c0ebfe0c1e2fc25))
	ROM_LOAD( "u27_cpu.rom", 0xc000, 0x4000, CRC(8ea2b4db) SHA1(df55a9fb70d1cabad51dc2b089af7904a823e1d8))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("u4_snd.rom", 0x00000, 0x8000, CRC(6a48b588) SHA1(c58dbfd920c279d7b9d2de8558d73c687b29ce9c))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
	ROM_LOAD("u19_snd.rom", 0x10000, 0x8000, CRC(1387467c) SHA1(8b3dd6c2fc94cfebc1879795532c651cda202846))
	ROM_RELOAD(0x10000+0x8000, 0x8000)
	ROM_LOAD("u20_snd.rom", 0x20000, 0x8000, CRC(d5a6a773) SHA1(30807e03655d2249c801007350bfb228a2e8a0a4))
	ROM_RELOAD(0x20000+0x8000, 0x8000)
ROM_END

/*------------------------------------
/ Beat the Clock #OC70
/------------------------------------*/
ROM_START(beatclck)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "btc_u3.cpu", 0xc000, 0x4000, CRC(9ba822ab) SHA1(f28d38411df3978bcaf24177fa1b47037a586cbb))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("btc_u2.snd", 0xc000, 0x1000, CRC(fd22fd2a) SHA1(efad3b94e91d07930ada5366d389f35377dfbd99))
	ROM_LOAD("btc_u3.snd", 0xd000, 0x1000, CRC(22311a4a) SHA1(2c22ba9228e44e68b9308b3bf8803edcd70fa5b9))
	ROM_LOAD("btc_u4.snd", 0xe000, 0x1000, CRC(af1cf23b) SHA1(ebfa3afafd7850dfa2664d3c640fbfa631012455))
	ROM_LOAD("btc_u5.snd", 0xf000, 0x1000, CRC(230cf329) SHA1(45b17a785b81cd5b1d7fdfb720cf1990994b52b7))
ROM_END

ROM_START(beatclck2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "btc_lights_pro_111385_c70-803-05_u3.cpu", 0xc000, 0x4000, CRC(dff5bad6) SHA1(915495d60be7ca12f00364b6e4b99c822ecfc7aa))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("btc_u2.snd", 0xc000, 0x1000, CRC(fd22fd2a) SHA1(efad3b94e91d07930ada5366d389f35377dfbd99))
	ROM_LOAD("btc_u3.snd", 0xd000, 0x1000, CRC(22311a4a) SHA1(2c22ba9228e44e68b9308b3bf8803edcd70fa5b9))
	ROM_LOAD("btc_u4.snd", 0xe000, 0x1000, CRC(af1cf23b) SHA1(ebfa3afafd7850dfa2664d3c640fbfa631012455))
	ROM_LOAD("btc_u5.snd", 0xf000, 0x1000, CRC(230cf329) SHA1(45b17a785b81cd5b1d7fdfb720cf1990994b52b7))
ROM_END

/*------------------------------------
/ Karate Fight
/------------------------------------*/

/*------------------------------------
/ Black Belt #OE52
/------------------------------------*/
ROM_START(blackblt)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.cpu", 0x8000, 0x4000, CRC(7c771910) SHA1(1df8ae478c3626a5200215bfca557ca42e064d2b))
	ROM_LOAD( "u3.cpu", 0xc000, 0x4000, CRC(bad0f4c3) SHA1(5e5240fda9f7f7f15f1953f12b132ba1c4fc886e))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("blck_u7.snd", 0x8000, 0x8000, CRC(db8bce07) SHA1(6327cfbb2761f4d190e2852f3321cdd0cc1e46a8))
ROM_END

ROM_START(blackblt2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.cpu", 0x8000, 0x4000, CRC(b86d16ec) SHA1(2e4601e725261aca67e4d706f310b14eb7578d8b))
	ROM_LOAD( "cpu_u3.cpu", 0xc000, 0x4000, CRC(c63e3e6f) SHA1(cd3f66c3796eaf64c36cabba9d74cc8c690d9d8b))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("blb_u2.snd", 0xc000, 0x1000, NO_DUMP)
	ROM_LOAD("blb_u3.snd", 0xd000, 0x1000, NO_DUMP)
	ROM_LOAD("blb_u4.snd", 0xe000, 0x1000, NO_DUMP)
	ROM_LOAD("blb_u5.snd", 0xf000, 0x1000, NO_DUMP)
ROM_END

/*------------------------------------
/ Blackwater 100 #OH07
/------------------------------------*/
ROM_START(black100)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.cpu", 0x8000, 0x4000, CRC(411fa773) SHA1(9756c7eee0f78792823a0b0379d2baac28cb03e8))
	ROM_LOAD( "u3.cpu", 0xc000, 0x4000, CRC(d6f6f890) SHA1(8fe4dae471f4c89f2fd72c6e647ead5206881c63))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.bin", 0x00001, 0x10000, CRC(a0ecb282) SHA1(4655e0b85f7e8af8dda853279696718d3adbf7e3))
	ROM_LOAD16_BYTE("u11.bin", 0x00000, 0x10000, CRC(3f117ba3) SHA1(b4cded8fdd90ca030c6ff12c817701402c94baba))
	ROM_LOAD16_BYTE("u14.bin", 0x20001, 0x10000, CRC(b45bf5c4) SHA1(396ddf346e8ebd8cb91777521d93564d029f40b1))
	ROM_LOAD16_BYTE("u13.bin", 0x20000, 0x10000, CRC(f5890443) SHA1(77cd18cf5541ae9f7e2dd1c060a9bf29b242d05d))
ROM_END

ROM_START(black100s)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "sb2.cpu", 0x8000, 0x4000, CRC(b6fdbb0f) SHA1(5b36a725db3a1e023bbb54b8f85300fe99174b6e))
	ROM_LOAD( "sb3.cpu", 0xc000, 0x4000, CRC(ae9930b8) SHA1(1b6c63ce98939ecded300639d872df62548157a4))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.bin", 0x00001, 0x10000, CRC(a0ecb282) SHA1(4655e0b85f7e8af8dda853279696718d3adbf7e3))
	ROM_LOAD16_BYTE("u11.bin", 0x00000, 0x10000, CRC(3f117ba3) SHA1(b4cded8fdd90ca030c6ff12c817701402c94baba))
	ROM_LOAD16_BYTE("u14.bin", 0x20001, 0x10000, CRC(b45bf5c4) SHA1(396ddf346e8ebd8cb91777521d93564d029f40b1))
	ROM_LOAD16_BYTE("u13.bin", 0x20000, 0x10000, CRC(f5890443) SHA1(77cd18cf5541ae9f7e2dd1c060a9bf29b242d05d))
ROM_END

/*------------------------------------
/ City Slicker #OE79
/------------------------------------*/
ROM_START(cityslck)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.128", 0x8000, 0x4000, CRC(94bcf162) SHA1(1d83592ad2441fc5e4c6fd3ab2373614dfe78b34))
	ROM_LOAD( "u3.128", 0xc000, 0x4000, CRC(97cb2bca) SHA1(0cbd49bbce2ce26c720d8a52bd4d1256f0ac61b3))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("u7_snd.512", 0x0000, 0x10000, CRC(6941d68a) SHA1(28de4327f328d16ec4cab59642c185777535efb2))
ROM_END

/*------------------------------------
/ Dungeons & Dragons #OH06
/------------------------------------*/
ROM_START(dungdrag)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.128", 0x8000, 0x4000, CRC(cefd4330) SHA1(0bffb2b73229e9908a018e06daeceb736896e5f0))
	ROM_LOAD( "cpu_u3.128", 0xc000, 0x4000, CRC(4bacc7f5) SHA1(71dd898924e0e968c4f3ba8a261e6b382d8ae0f1))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("snd_u12.512", 0x00001, 0x10000, CRC(dd95f851) SHA1(6fa46b512bced0d1862b2621e195ef0dfd24f928))
	ROM_LOAD16_BYTE("snd_u11.512", 0x00000, 0x10000, CRC(dcd461b3) SHA1(834000cfb6c6acf5c296db58971251819971f4de))
	ROM_LOAD16_BYTE("snd_u14.512", 0x20001, 0x10000, CRC(dd9e61eb) SHA1(fd1ec58f5708d5abf3d7424954ce054454514283))
	ROM_LOAD16_BYTE("snd_u13.512", 0x20000, 0x10000, CRC(1e2d9211) SHA1(f5fcf1c07f01e7f1a7abff9ac3c481b84471d3a6))
ROM_END

/*------------------------------------
/ Eight Ball Champ #OB38
/------------------------------------*/
ROM_START(eballchp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u3_cpu.128", 0xc000, 0x4000, CRC(025f3008) SHA1(25d310f169b92ce6b348330816ddc3b5710e57da))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("u3_snd.532", 0xd000, 0x1000, CRC(4836d70d) SHA1(a4acc64609d91a84ba4c8101186d07397b496600))
	ROM_LOAD("u4_snd.532", 0xe000, 0x1000, CRC(4b49d94d) SHA1(52d5f4b7604601cd86f0e80ed7c4fe09d14f5976))
	ROM_LOAD("u5_snd.532", 0xf000, 0x1000, CRC(655441df) SHA1(9da5578856ded3dcdafed67679eb4c4134dc9f81))
ROM_END

#ifdef MISSING_GAME // same as above but with CPU2 roms as NO_DUMP
ROM_START(eballch2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u3_cpu.128", 0xc000, 0x4000, CRC(025f3008) SHA1(25d310f169b92ce6b348330816ddc3b5710e57da))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("ebcu4.snd", 0x8000, 0x2000, NO_DUMP)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_LOAD("ebcu3.snd", 0xc000, 0x2000, NO_DUMP)
	ROM_RELOAD(0xe000, 0x2000)
ROM_END
#endif

/*------------------------------------------------
/ Escape from the Lost World #OH05
/-----------------------------------------------*/
ROM_START(esclwrld)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.128", 0x8000, 0x4000, CRC(b11a97ea) SHA1(29339785a67ed7dc9eb39ddc7bb7e6baaf731210))
	ROM_LOAD( "u3.128", 0xc000, 0x4000, CRC(5385a562) SHA1(a6c39532d01db556e4bdf90020a9d9905238e8ef))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.512", 0x00001, 0x10000, CRC(0c003473) SHA1(8ada2aa546a6499c5e2b5eb45a1975b8285d25f9))
	ROM_LOAD16_BYTE("u11.512", 0x00000, 0x10000, CRC(360f6658) SHA1(c0346952dcd33bbcf4c43c51cde5433a099a7a5d))
	ROM_LOAD16_BYTE("u14.512", 0x20001, 0x10000, CRC(0b92afff) SHA1(78f51989e74ced9e0a81c4e18d5abad71de01faf))
	ROM_LOAD16_BYTE("u13.512", 0x20000, 0x10000, CRC(b056842e) SHA1(7c67e5d69235a784b9c38cb31302d206278a3814))
ROM_END

ROM_START(esclwrldg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_ger.128", 0x8000, 0x4000, CRC(0a6ab137) SHA1(0627b7c67d13f305f2287f3cfa023c8dd7721250))
	ROM_LOAD( "u3_ger.128", 0xc000, 0x4000, CRC(26d8bfbb) SHA1(3b81fb0e736d14004bbbbb2edd682fdfc1b2c832))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.512", 0x00001, 0x10000, CRC(0c003473) SHA1(8ada2aa546a6499c5e2b5eb45a1975b8285d25f9))
	ROM_LOAD16_BYTE("u11.512", 0x00000, 0x10000, CRC(360f6658) SHA1(c0346952dcd33bbcf4c43c51cde5433a099a7a5d))
	ROM_LOAD16_BYTE("u14.512", 0x20001, 0x10000, CRC(0b92afff) SHA1(78f51989e74ced9e0a81c4e18d5abad71de01faf))
	ROM_LOAD16_BYTE("u13.512", 0x20000, 0x10000, CRC(b056842e) SHA1(7c67e5d69235a784b9c38cb31302d206278a3814))
ROM_END
/*------------------------------------
/ Hardbody #OE94
/------------------------------------*/
ROM_START(hardbody)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.128", 0x8000, 0x4000, CRC(c9248b47) SHA1(54239bd7d15574ebbb70ed306a804b7b32ed516a))
	ROM_LOAD( "cpu_u3.128", 0xc000, 0x4000, CRC(31c255d0) SHA1(b6ffa2616ae9a4a121585cc402080ec6f26f8472))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.512", 0x0000, 0x10000, CRC(c96f91af) SHA1(9602a8991ca0cf9a7c68710f55c245d9c675b06f))
ROM_END

ROM_START(hardbodyg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "hrdbdy-g.u2", 0x8000, 0x4000, CRC(fce357cc) SHA1(f7d13c12aabcb3c5bb5826b1911817bd359f1941))
	ROM_LOAD( "hrdbdy-g.u3", 0xc000, 0x4000, CRC(ccac74b5) SHA1(d55cfc8ee866a9af4567d56890f5a9ecb9c3c02f))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.512", 0x0000, 0x10000, CRC(c96f91af) SHA1(9602a8991ca0cf9a7c68710f55c245d9c675b06f))
ROM_END

/*-----------------------------------------
/ Heavy Metal Meltdown #OH03
/-----------------------------------------*/
ROM_START(hvymetap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.rom", 0x8000, 0x4000, CRC(53466e4e) SHA1(af6d0e15821ff707f24bb99b8d9dfb9f929906db))
	ROM_LOAD( "u3.rom", 0xc000, 0x4000, CRC(0a08ae7e) SHA1(04f295fbe3a7bd7b929556338914c0ed94a77d62))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.rom", 0x00001, 0x10000, CRC(77933258) SHA1(42a01e97440dbb7d3da92dbfbad2516f4b553a5f))
	ROM_LOAD16_BYTE("u11.rom", 0x00000, 0x10000, CRC(b7e4de7d) SHA1(bcc89e10c368cdbc5137d8f585e109c0be25522d))
ROM_END

/*------------------------------------
/ Lady Luck #OE34
/------------------------------------*/
ROM_START(ladyluck)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u3.cpu", 0xc000, 0x4000, CRC(129f41f5) SHA1(0351419814d3f4e98a4572fdec9d53e12fe6b6be))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("u4_snd.532", 0x8000, 0x2000, CRC(e9ef01e6) SHA1(79191e776b6683b259cd1a80e9fb3183268bde56))
	ROM_RELOAD(0xa000, 0x2000)
	ROM_LOAD("u3_snd.532", 0xc000, 0x2000, CRC(1bdd6e2b) SHA1(14fc25b5f8eefe8ffab062f83e06ec19403aa00a))
	ROM_RELOAD(0xe000, 0x2000)
ROM_END

/*--------------------------------
/ MotorDome #OE14
/-------------------------------*/
ROM_START(motrdome)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "modm_u2.dat", 0x8000, 0x4000, CRC(820ca073) SHA1(0b50712f7d65f629af934deccc52d588f390a05b))
	ROM_LOAD( "modm_u3.dat", 0xc000, 0x4000, CRC(aae7c418) SHA1(9d3ea83ffff0b9696f5113043475c6e9b9a464ae))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("modm_u7.snd", 0x8000, 0x8000, CRC(29ce4679) SHA1(f17998198b542dd99a34abd678db7e031bde074b))
ROM_END

ROM_START(motrdomeg) // German version claims to be game #E69
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_11_de.bin", 0x8000, 0x4000, CRC(8a4bafd3) SHA1(d764d2e38be2df27ab982cfbedddb79f89ca2359))
	ROM_LOAD( "u3_11_de.bin", 0xc000, 0x4000, CRC(9cb10037) SHA1(7847a71a0295e8de51a8f2f8d406350eca4555bf))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("modm_u7.snd", 0x8000, 0x8000, CRC(29ce4679) SHA1(f17998198b542dd99a34abd678db7e031bde074b))
ROM_END

/*--------------------------------
/ Party Animal #OH01
/-------------------------------*/
ROM_START(prtyanim)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.128", 0x8000, 0x4000, CRC(abdc0b2d) SHA1(b93c7248ea83461101383023bd4e4a50292d8570))
	ROM_LOAD( "cpu_u3.128", 0xc000, 0x4000, CRC(e48b2d63) SHA1(190fc5a805bda9617c08a29c0bde4d94a77279e9))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("snd_u12.512", 0x00001, 0x10000, CRC(265a9494) SHA1(3b631f2b1c8c685aef32fb6c5289cd792711ff7e))
	ROM_LOAD16_BYTE("snd_u11.512", 0x00000, 0x10000, CRC(20be998f) SHA1(7f98073d0f559e081b2d6dc8c1f3462e3fe9a713))
	ROM_LOAD16_BYTE("snd_u14.512", 0x20001, 0x10000, CRC(639b3db1) SHA1(e07669c3186c963f2fea29bcf5675ac86eb07c86))
	ROM_LOAD16_BYTE("snd_u13.512", 0x20000, 0x10000, CRC(b652597b) SHA1(8b4074a545d420319712a1fdd77a3bfb282ed9cd))
ROM_END

ROM_START(prtyanimg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2g.128", 0x8000, 0x4000, CRC(8abf40a2) SHA1(04ac296c99bc176faf21f1277ff59228a2031715))
	ROM_LOAD( "cpu_u3g.128", 0xc000, 0x4000, CRC(e781dd4b) SHA1(3395ddd2d774c83cac98b6d67415d3c8cd0b04fe))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("snd_u12.512", 0x00001, 0x10000, CRC(265a9494) SHA1(3b631f2b1c8c685aef32fb6c5289cd792711ff7e))
	ROM_LOAD16_BYTE("snd_u11.512", 0x00000, 0x10000, CRC(20be998f) SHA1(7f98073d0f559e081b2d6dc8c1f3462e3fe9a713))
	ROM_LOAD16_BYTE("snd_u14.512", 0x20001, 0x10000, CRC(639b3db1) SHA1(e07669c3186c963f2fea29bcf5675ac86eb07c86))
	ROM_LOAD16_BYTE("snd_u13.512", 0x20000, 0x10000, CRC(b652597b) SHA1(8b4074a545d420319712a1fdd77a3bfb282ed9cd))
ROM_END

/*------------------------------------
/ Special Force #OE47 - 1st Game to use Sounds Deluxe Sound Hardware
/------------------------------------*/
ROM_START(specforc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_revc.128", 0x8000, 0x4000, CRC(d042af04) SHA1(0a73ee6d3ce603899fd89de70f90e9efc58b8b42))
	ROM_LOAD( "u3_revc.128", 0xc000, 0x4000, CRC(d48a5eaf) SHA1(90a5d5e928abfec699bae9d0087e90316339058f))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12_snd.512", 0x00001, 0x10000, CRC(4f48a490) SHA1(6c9a594ecc68adf3b1eda315c4704e1d025a3442))
	ROM_LOAD16_BYTE("u11_snd.512", 0x00000, 0x10000, CRC(b16eb713) SHA1(461e5ed82891d17849984137536bc6d1ab2907c2))
	ROM_LOAD16_BYTE("u14_snd.512", 0x20001, 0x10000, CRC(6911fa51) SHA1(a75f75bb4493b0ea3a423bc033d49022228d79c1))
	ROM_LOAD16_BYTE("u13_snd.512", 0x20000, 0x10000, CRC(3edda92d) SHA1(dbd95bb1c534779f56cc9e30efef159feaf22712))
ROM_END

/*------------------------------------
/ Strange Science #OE35
/------------------------------------*/

ROM_START(strngsci)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "strange_scienc.u2", 0x8000, 0x4000, CRC(0a0ebf25) SHA1(6b120e5b3aa13d1650c4ee8c4c98996b13be167e)) // 12/12/86
	ROM_LOAD( "strange_scienc.u3", 0xc000, 0x4000, CRC(c5b17b07) SHA1(823eb1e2ceb33f221b69c11eb71cb53c48d0f716)) // 12/12/86
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.256", 0x8000, 0x8000, CRC(bc33901e) SHA1(5231d8f01a107742acee2d13580a461063018a11))
ROM_END

ROM_START(strngscia)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.128", 0x8000, 0x4000, CRC(2ffcf284) SHA1(27d66806708c983092bab4ed6965c2e91e69acdc))
	ROM_LOAD( "cpu_u3.128", 0xc000, 0x4000, CRC(35257931) SHA1(d3d6b84e50677a4c5f9d5c13c9522ad6d3a1358d))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.256", 0x8000, 0x8000, CRC(bc33901e) SHA1(5231d8f01a107742acee2d13580a461063018a11))
ROM_END

ROM_START(strngscig)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpub_u2.128", 0x8000, 0x4000, CRC(48ef1052) SHA1(afcb0520ab834c0d6ef4a73f615c48653ccedc24))
	ROM_LOAD( "cpub_u3.128", 0xc000, 0x4000, CRC(da5b4b3b) SHA1(ff9babf2efc6622803db9ba8712dd8b76c8412b8))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.256", 0x8000, 0x8000, CRC(bc33901e) SHA1(5231d8f01a107742acee2d13580a461063018a11))
ROM_END

/*-------------------------------------------------------------
/ Truck Stop #2001
/-------------------------------------------------------------*/
ROM_START(trucksp3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_p3.128", 0x8000, 0x4000, CRC(79b2a5b1) SHA1(d3de91bfadc9684302b2367cfcb30ed0d6faa020))
	ROM_LOAD( "u3_p3.128", 0xc000, 0x4000, CRC(2993373c) SHA1(26490f1dd8a5329a88a2ceb1e6044711a29f1445))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASE00)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("u4sndp1.256", 0x00000, 0x8000, CRC(120a386f) SHA1(51b3b45eb7ea63758b21aad404ba12a9607fec44))
	ROM_RELOAD(0x00000 +0x8000, 0x8000)
	ROM_LOAD("u19sndp1.256", 0x10000, 0x8000, CRC(5cd43dda) SHA1(23dd8a52ea1340fc239a246af0d94da905464efb))
	ROM_RELOAD(0x10000 +0x8000, 0x8000)
	ROM_LOAD("u20sndp1.256", 0x20000, 0x8000, CRC(93ac5c33) SHA1(f6dc84eca4678188a58ba3c8ef18975164dd29b0))
	ROM_RELOAD(0x20000 +0x8000, 0x8000)
ROM_END

ROM_START(trucksp2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_p2.128", 0x8000, 0x4000, CRC(3c397dec) SHA1(2fc86ad39c935ce8615eafd67e571ac94c938cd7))
	ROM_LOAD( "u3_p2.128", 0xc000, 0x4000, CRC(d7ac519a) SHA1(612bf9fee0d54e8b1215508bd6c1ea61dcb99951))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASE00)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("u4sndp1.256", 0x00000, 0x8000, CRC(120a386f) SHA1(51b3b45eb7ea63758b21aad404ba12a9607fec44))
	ROM_RELOAD(0x00000 +0x8000, 0x8000)
	ROM_LOAD("u19sndp1.256", 0x10000, 0x8000, CRC(5cd43dda) SHA1(23dd8a52ea1340fc239a246af0d94da905464efb))
	ROM_RELOAD(0x10000 +0x8000, 0x8000)
	ROM_LOAD("u20sndp1.256", 0x20000, 0x8000, CRC(93ac5c33) SHA1(f6dc84eca4678188a58ba3c8ef18975164dd29b0))
	ROM_RELOAD(0x20000 +0x8000, 0x8000)
ROM_END


//  ( YEAR  NAME       PARENT     MACHINE    INPUT      STATE            INIT           MONITOR COMPANY  FULLNAME                               FLAGS )
GAME( 1985, eballchp,  0,         by6803,    eballchp,  eballchp_state,  init_eballchp, ROT0, "Bally", "Eight Ball Champ",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1985, beatclck,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Beat the Clock",                        MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1985, beatclck2, beatclck,  by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Beat the Clock (with flasher support)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1986, motrdome,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "MotorDome",                             MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1986, motrdomeg, motrdome,  by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "MotorDome (German)",                    MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1986, ladyluck,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Lady Luck",                             MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1986, strngsci,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Strange Science (Rev C)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1986, strngscia, strngsci,  by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Strange Science (Rev A)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1986, strngscig, strngsci,  by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Strange Science (German, Rev A)",       MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1986, specforc,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Special Force",                         MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1986, blackblt,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Black Belt",                            MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1986, blackblt2, blackblt,  by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Black Belt (Squawk and Talk)",          MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, cityslck,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "City Slicker",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, hardbody,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Hardbody",                              MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, hardbodyg, hardbody,  by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Hardbody (German)",                     MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, prtyanim,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Party Animal",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, prtyanimg, prtyanim,  by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Party Animal (German)",                 MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, hvymetap,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Heavy Metal Meltdown",                  MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, esclwrld,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Escape from the Lost World",            MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, esclwrldg, esclwrld,  by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Escape from the Lost World (German)",   MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, dungdrag,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Dungeons & Dragons",                    MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, black100,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Blackwater 100",                        MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, black100s, black100,  by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Blackwater 100 (Single Ball Play)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, trucksp3,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Truck Stop (P-3)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, trucksp2,  trucksp3,  by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Truck Stop (P-2)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1989, atlantis,  0,         by6803,    by6803,    by6803_state,    init_by6803,   ROT0, "Bally", "Atlantis",                              MACHINE_IS_SKELETON_MECHANICAL)

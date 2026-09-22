// license:BSD-3-Clause
// copyright-holders:Robbbert, Quench
/********************************************************************************************

    PINBALL
    Bally MPU AS-2518-17

    These are some very early and well known Solid State machines, such as 'Eight Ball'.

    They have orange digital 6 digit displays, and a mechanical chime unit for sounds.


ToDo:
- Bow & Arrow fails the PIA test and doesn't boot
- Dips, Inputs, Solenoids vary per game
- Mechanical

Notes:
- Black Jack appears to have a software bug. If the Spinner is the first activated switch
  on a ball, it tilts the ball in play.
*********************************************************************************************/


#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "render.h"

//#define VERBOSE 1
#include "logmacro.h"

#include "by17.lh"
#include "by17_freedom.lh"
#include "by17_nightrdr.lh"
#include "by17_blackjck.lh"
#include "by17_evelknie.lh"
#include "by17_eightbll.lh"
#include "by17_matahari.lh"
#include "by17_pwerplay.lh"
#include "by17_stk_sprs.lh"


class by17_state : public genpin_class
{
public:
	by17_state(machine_config const &mconfig, device_type type, char const *tag)
		: by17_state(mconfig, type, tag, s_solenoid_features_default)
	{ }

	void init_by17()		{ m_outhole = 0x0080; }

	DECLARE_INPUT_CHANGED_MEMBER(activity_button);
	DECLARE_INPUT_CHANGED_MEMBER(self_test);
	DECLARE_CUSTOM_INPUT_MEMBER(outhole);
	DECLARE_CUSTOM_INPUT_MEMBER(spinner);
	DECLARE_CUSTOM_INPUT_MEMBER(kicker);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x0);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x1);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x2);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x3);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x4);

	void by17(machine_config &config);


protected:
	typedef uint8_t solenoid_feature_data[20][4];

	by17_state(machine_config const &mconfig, device_type type, char const *tag, solenoid_feature_data const &solenoid_features)
		: genpin_class(mconfig, type, tag)
		, m_solenoid_features(solenoid_features)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_pia_u10(*this, "pia_u10")
		, m_pia_u11(*this, "pia_u11")
		, m_zero_crossing_freq_timer(*this, "timer_z_freq")
		, m_zero_crossing_active_timer(*this, "timer_z_pulse")
		, m_display_refresh_timer(*this, "timer_d_pulse")
		, m_io_test(*this, "TEST")
		, m_io_dsw0(*this, "DSW0")
		, m_io_dsw1(*this, "DSW1")
		, m_io_dsw2(*this, "DSW2")
		, m_io_dsw3(*this, "DSW3")
		, m_io_x0(*this, "X0")
		, m_io_x1(*this, "X1")
		, m_io_x2(*this, "X2")
		, m_io_x3(*this, "X3")
		, m_io_x4(*this, "X4")
		, m_io_outhole(*this, "OUTHOLE")
		, m_io_spinner(*this, "SPINNER")
		, m_io_zx(*this, "ZX")
		, m_lamps(*this, "lamp%u", 0U)
		, m_display(*this, "digit%u%u", 0U, 0U)
		, m_solenoids(*this, "solenoid%u", 0U)
		, m_spinners(*this, "spinner%u", 0U)
	{ }


	DECLARE_READ8_MEMBER(u10_a_r);
	DECLARE_WRITE8_MEMBER(u10_a_w);
	DECLARE_READ8_MEMBER(u10_b_r);
	DECLARE_WRITE8_MEMBER(u10_b_w);
	DECLARE_READ8_MEMBER(u11_a_r);
	DECLARE_WRITE8_MEMBER(u11_a_w);
	DECLARE_WRITE8_MEMBER(u11_b_w);
	DECLARE_READ8_MEMBER(nibble_nvram_r);
	DECLARE_WRITE8_MEMBER(nibble_nvram_w);
	DECLARE_READ_LINE_MEMBER(u10_ca1_r);
	DECLARE_READ_LINE_MEMBER(u10_cb1_r);
	DECLARE_WRITE_LINE_MEMBER(u10_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u10_cb2_w);
	DECLARE_READ_LINE_MEMBER(u11_ca1_r);
	DECLARE_READ_LINE_MEMBER(u11_cb1_r);
	DECLARE_WRITE_LINE_MEMBER(u11_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u11_cb2_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	TIMER_DEVICE_CALLBACK_MEMBER(timer_z_freq);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_z_pulse);
	TIMER_DEVICE_CALLBACK_MEMBER(u11_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_d_pulse);

	void by17_map(address_map &map);


	uint8_t m_u10a;
	uint8_t m_u10b;
	uint8_t m_u11a;
	uint8_t m_u11b;

	static solenoid_feature_data const s_solenoid_features_default;

private:
	bool m_u10_ca2;
	bool m_u10_cb1;
	bool m_u10_cb2;
	bool m_u11_ca1;
	bool m_u11_ca2;
	bool m_u11_cb2;
	uint8_t m_digit;
	uint8_t m_segment[5];
	uint8_t m_lamp_decode;
	solenoid_feature_data const &m_solenoid_features;
	uint8_t m_io_hold_x[5];       // Holds switches closed (drop targets, balls in outholes/saucers/kickbacks etc). Solenoid activity releases the switch.
	uint16_t m_outhole;           // Optionally put a ball in the outhole at power on
	uint16_t m_spinner[4];        // Helps animate a simulated Spinner on the playfield layout
	required_device<m6800_cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_nvram;
	required_device<pia6821_device> m_pia_u10;
	required_device<pia6821_device> m_pia_u11;
	required_device<timer_device> m_zero_crossing_freq_timer;
	required_device<timer_device> m_zero_crossing_active_timer;
	required_device<timer_device> m_display_refresh_timer;
	required_ioport m_io_test;
	required_ioport m_io_dsw0;
	required_ioport m_io_dsw1;
	required_ioport m_io_dsw2;
	required_ioport m_io_dsw3;
	required_ioport m_io_x0;
	required_ioport m_io_x1;
	required_ioport m_io_x2;
	required_ioport m_io_x3;
	required_ioport m_io_x4;
	required_ioport m_io_outhole;
	required_ioport m_io_spinner;
	required_ioport m_io_zx;
	output_finder<15 * 4> m_lamps;
	output_finder<7, 8> m_display;
	output_finder<20> m_solenoids;
	output_finder<3> m_spinners;
	void update_lamps(u8 lamp_board_base, u8 lamp_decoder_base, u8 lamp_decoder_addr, u8 lamp_decoder_data);
};


class freedom_state : public by17_state
{
public:
	freedom_state(machine_config const &mconfig, device_type type, char const *tag)
		: by17_state(mconfig, type, tag, s_solenoid_features_freedom)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_freedom;
};

class nightrdr_state : public by17_state
{
public:
	nightrdr_state(machine_config const &mconfig, device_type type, char const *tag)
		: by17_state(mconfig, type, tag, s_solenoid_features_nightrdr)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_nightrdr;
};

class evelknie_state : public by17_state
{
public:
	evelknie_state(machine_config const &mconfig, device_type type, char const *tag)
		: by17_state(mconfig, type, tag, s_solenoid_features_evelknie)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_evelknie;
};

class eightbll_state : public by17_state
{
public:
	eightbll_state(machine_config const &mconfig, device_type type, char const *tag)
		: by17_state(mconfig, type, tag, s_solenoid_features_eightbll)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_eightbll;
};

class blackjck_state : public by17_state
{
public:
	blackjck_state(machine_config const &mconfig, device_type type, char const *tag)
		: by17_state(mconfig, type, tag, s_solenoid_features_blackjck)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_blackjck;
};

class matahari_state : public by17_state
{
public:
	matahari_state(machine_config const &mconfig, device_type type, char const *tag)
		: by17_state(mconfig, type, tag, s_solenoid_features_matahari)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_matahari;
};

class pwerplay_state : public by17_state
{
public:
	pwerplay_state(machine_config const &mconfig, device_type type, char const *tag)
		: by17_state(mconfig, type, tag, s_solenoid_features_pwerplay)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_pwerplay;
};

class stk_sprs_state : public by17_state
{
public:
	stk_sprs_state(machine_config const &mconfig, device_type type, char const *tag)
		: by17_state(mconfig, type, tag, s_solenoid_features_stk_sprs)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_stk_sprs;
};



void by17_state::by17_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x1fff);     // A15, A14 and A13 are not connected
	map(0x0000, 0x007f).mirror(0x0100).ram();
	map(0x0088, 0x008b).mirror(0x0d74).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).mirror(0x0d6c).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0200, 0x02ff).mirror(0x0d00).ram().rw(FUNC(by17_state::nibble_nvram_r), FUNC(by17_state::nibble_nvram_w)).share("nvram");
	map(0x1000, 0x1fff).mirror(0x0000).rom();
}



static INPUT_PORTS_START( by17 )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Self Test") PORT_CHANGED_MEMBER(DEVICE_SELF, by17_state, self_test, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Activity")  PORT_CHANGED_MEMBER(DEVICE_SELF, by17_state, activity_button, nullptr)

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

	PORT_START("DSW0")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 1 (Coins/Credits)")    PORT_DIPLOCATION("SW0:!1,!2,!3,!4,!5")  // Same as Coin Slot 3
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x01, "2 Coins/3 Credits (C1/1C, C2/2C)")
	PORT_DIPSETTING(    0x00, "2 Coins/3 Credits (C1/1C, C2/2C)")
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x13, "2 Coins/9 Credits")
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x15, "2 Coins/10 Credits")
	PORT_DIPSETTING(    0x14, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0x17, "2 Coins/11 Credits")
	PORT_DIPSETTING(    0x16, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0x19, "2 Coins/12 Credits")
	PORT_DIPSETTING(    0x18, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x1b, "2 Coins/13 Credits")
	PORT_DIPSETTING(    0x1a, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0x1d, "2 Coins/14 Credits")
	PORT_DIPSETTING(    0x1c, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0x1f, "2 Coins/15 Credits")
	PORT_DIPSETTING(    0x1e, "1 Coin/15 Credits")
	PORT_DIPNAME( 0x20, 0x20, "Score Level Award")          PORT_DIPLOCATION("SW0:!6")
	PORT_DIPSETTING(    0x00, "Extra Ball")
	PORT_DIPSETTING(    0x20, "Replay")
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 07")                   PORT_DIPLOCATION("SW0:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, "Play Melodies")              PORT_DIPLOCATION("SW0:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 3 (Coins/Credits)")    PORT_DIPLOCATION("SW1:!1,!2,!3,!4,!5")  // Same as Coin Slot 1
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x01, "2 Coins/3 Credits (C1/1C, C2/2C)")
	PORT_DIPSETTING(    0x00, "2 Coins/3 Credits (C1/1C, C2/2C)")
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x13, "2 Coins/9 Credits")
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x15, "2 Coins/10 Credits")
	PORT_DIPSETTING(    0x14, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0x17, "2 Coins/11 Credits")
	PORT_DIPSETTING(    0x16, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0x19, "2 Coins/12 Credits")
	PORT_DIPSETTING(    0x18, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x1b, "2 Coins/13 Credits")
	PORT_DIPSETTING(    0x1a, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0x1d, "2 Coins/14 Credits")
	PORT_DIPSETTING(    0x1c, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0x1f, "2 Coins/15 Credits")
	PORT_DIPSETTING(    0x1e, "1 Coin/15 Credits")
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 14")                           PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "Beating Highest Score Awards")       PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, "Novelty")
	PORT_DIPSETTING(    0x40, "3 Credits")
	PORT_DIPNAME( 0x80, 0x80, "Balls per Game")                     PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x80, "5")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x01, "Maximum Credits")        PORT_DIPLOCATION("SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "20")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x05, "30")
	PORT_DIPSETTING(    0x06, "35")
	PORT_DIPSETTING(    0x07, "40")
	PORT_DIPNAME( 0x08, 0x08, "Credits Displayed")      PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x10, "Match Feature")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))           PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	// from here, game-specific options
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 22")               PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 23")               PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 24")               PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2")            PORT_DIPLOCATION("SW3:!1,!2,!3,!4")
	PORT_DIPSETTING(    0x00, "Same as Coin Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x0a, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0x0b, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0x0c, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x0d, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0x0e, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0x0f, "1 Coin/15 Credits")
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 29")               PORT_DIPLOCATION("SW3:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 30")               PORT_DIPLOCATION("SW3:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 31")               PORT_DIPLOCATION("SW3:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 32")               PORT_DIPLOCATION("SW3:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))


	PORT_START("X0")
	// custom
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	// standard
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, outhole, (void *)0x0080)  // PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT2 ) PORT_NAME("Slam Tilt") PORT_CODE(KEYCODE_EQUALS)

	// custom
	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
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
INPUT_PORTS_END

static INPUT_PORTS_START( freedom )
	PORT_INCLUDE( by17 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x40, 0x40, "Freedom Wheel Arrow Advance")        PORT_DIPLOCATION("SW0:!7")
	PORT_DIPSETTING(    0x00, "With Points Scoring")
	PORT_DIPSETTING(    0x40, "Continuously")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xe0, 0xc0, "Drop Targets Award")                 PORT_DIPLOCATION("SW2:!6,!7,!8")
	PORT_DIPSETTING(    0x60, "Unused Novelty 60")
	PORT_DIPSETTING(    0xa0, "Unused Novelty A0")
	PORT_DIPSETTING(    0xe0, "Unused Novelty E0")
	PORT_DIPSETTING(    0x20, "5000, Then 10000 Novelty")
//	PORT_DIPSETTING(    0x00, "5000, Then Extra Ball")		Actually same as option 0x20
	PORT_DIPSETTING(    0x80, "Extra Ball, Then 10000")
	PORT_DIPSETTING(    0x40, "5000, Then Extra Ball And 1 Credit")
	PORT_DIPSETTING(    0xc0, "Extra Ball, Then 1 Credit")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 25")               PORT_DIPLOCATION("SW3:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 26")               PORT_DIPLOCATION("SW3:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 27")               PORT_DIPLOCATION("SW3:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 28")               PORT_DIPLOCATION("SW3:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))

	PORT_MODIFY("X0")   /* Drop Target switches Left */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 )

	PORT_MODIFY("X2")   /* Spinners and Saucer switches */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, spinner, (void *)0x1240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)
INPUT_PORTS_END

static INPUT_PORTS_START( nightrdr )
	PORT_INCLUDE( by17 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x40, 0x00, "Coin Slot 2")                        PORT_DIPLOCATION("SW0:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x40, "1 Coin/12 Credits")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x80, 0x00, "Free Balls and Free Games")          PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 25")                           PORT_DIPLOCATION("SW3:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 26")                           PORT_DIPLOCATION("SW3:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 27")                           PORT_DIPLOCATION("SW3:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 28")                           PORT_DIPLOCATION("SW3:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "Drop Targets")                       PORT_DIPLOCATION("SW3:!7")
	PORT_DIPSETTING(    0x00, "Both Sides Down Lights Specials")
	PORT_DIPSETTING(    0x40, "Either Side Down Lights Specials")
	PORT_DIPNAME( 0x80, 0x00, "Extra Ball Outlane Lights")          PORT_DIPLOCATION("SW3:!8")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x80, "Both On")

	PORT_MODIFY("X0")   /* Drop Target switches Left */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 )

	PORT_MODIFY("X2")   /* Spinners and Top Saucer switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, spinner, (void *)0x1208)  // PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, spinner, (void *)0x2210)  // PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)

	PORT_MODIFY("X3")   /* Drop Target switches Right */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x3, (void *)0x0301)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x3, (void *)0x0302)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)
INPUT_PORTS_END

static INPUT_PORTS_START( blackjck )
	PORT_INCLUDE( by17 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x60, 0x60, "Beating Highest Score Awards")       PORT_DIPLOCATION("SW0:!6,!7")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x20, "1 Credit")
	PORT_DIPSETTING(    0x40, "2 Credits")
	PORT_DIPSETTING(    0x60, "3 Credits")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x60, 0x60, "Score Level Awards")                 PORT_DIPLOCATION("SW1:!6,!7")
	PORT_DIPSETTING(    0x20, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x40, "Extra Ball")
	PORT_DIPSETTING(    0x60, "Replay")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x80, 0x00, "Saucer Extra Ball And Special")      PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, "Separate")
	PORT_DIPSETTING(    0x80, "Together")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x60, 0x00, "Card Suit Lanes")                    PORT_DIPLOCATION("SW3:!6,!7")
	PORT_DIPSETTING(    0x60, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x00, "Reset Next Ball In Play")
	PORT_DIPSETTING(    0x40, "Remember Until All 4 Lanes Are Made")
	PORT_DIPSETTING(    0x20, "Remember Spinner And Outlane Lights All Game")
	PORT_DIPNAME( 0x80, 0x80, "Against The Dealer")                 PORT_DIPLOCATION("SW3:!8")
	PORT_DIPSETTING(    0x00, "Player Loses On Ties")
	PORT_DIPSETTING(    0x80, "Player Wins On Ties")

	PORT_MODIFY("X0")   /* Spinner switch */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, spinner, (void *)0x1010)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X3")   /* Rightside Saucer switch */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END


static INPUT_PORTS_START( evelknie )
	PORT_INCLUDE( by17 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x60, 0x60, "Beating Highest Score Awards")       PORT_DIPLOCATION("SW1:!6,!7")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x20, "1 Credit")
	PORT_DIPSETTING(    0x40, "2 Credits")
	PORT_DIPSETTING(    0x60, "3 Credits")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x60, 0x00, "Coin Slot 2")                        PORT_DIPLOCATION("SW2:!6,!7")
	PORT_DIPSETTING(    0x40, "Same as Coin Slot 1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_6C))
	PORT_DIPSETTING(    0x60, "1 Coin/12 Credits")
	PORT_DIPNAME( 0x80, 0x00, "Outlane Special Lights")             PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x80, "Both")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Completing S-U-P-E-R Awards")        PORT_DIPLOCATION("SW3:!1,!2")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x01, "Extra Ball")
	PORT_DIPSETTING(    0x02, "1 Credit")
	PORT_DIPSETTING(    0x03, "Extra Ball and 1 Credit")
	PORT_DIPNAME( 0x0c, 0x0c, "Drop Targets Down 2nd Time")         PORT_DIPLOCATION("SW3:!3,!4")
	PORT_DIPSETTING(    0x00, "No Award")
	PORT_DIPSETTING(    0x04, "5000 Points (Novelty)")
	PORT_DIPSETTING(    0x0c, "Extra Ball")
	PORT_DIPSETTING(    0x08, "5000 Points and Extra Ball")
	PORT_DIPNAME( 0x30, 0x30, "Drop Targets Down 3rd Time")         PORT_DIPLOCATION("SW3:!5,!6")
	PORT_DIPSETTING(    0x00, "No Award")
	PORT_DIPSETTING(    0x10, "5000 Points (Novelty)")
	PORT_DIPSETTING(    0x20, "Extra Ball")
	PORT_DIPSETTING(    0x30, "1 Credit")
	PORT_DIPNAME( 0xc0, 0x80, "Outlane Specials Awards")            PORT_DIPLOCATION("SW3:!7,!8")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x40, "Extra Ball")
	PORT_DIPSETTING(    0x80, "1 Credit")
	PORT_DIPSETTING(    0xc0, "2 Credits")

	PORT_MODIFY("X0")   /* Drop Target switches Left */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X3")   /* Spinners and Top Saucer switches */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, spinner, (void *)0x1320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, spinner, (void *)0x2340)  // PORT_CODE(KEYCODE_S)
INPUT_PORTS_END

static INPUT_PORTS_START( eightbll )
	PORT_INCLUDE( by17 )

	PORT_MODIFY("SPINNER")                           // Allow user to select the simulated rotation speed of Spinners
	PORT_CONFNAME( 0x07, 0x07, "Spinner Simulation")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ))
	PORT_CONFSETTING(    0x01, "Slow Speed")
	PORT_CONFSETTING(    0x02, "Medium Speed")
	PORT_CONFSETTING(    0x03, "High Speed")
	PORT_CONFSETTING(    0x07, "Turbo Speed")

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x60, 0x60, "Beating Highest Score Awards")       PORT_DIPLOCATION("SW0:!6,!7")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x20, "1 Credit")
	PORT_DIPSETTING(    0x40, "2 Credits")
	PORT_DIPSETTING(    0x60, "3 Credits")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 14")                           PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 15")                           PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x20, "Special Light Remains Lit For")       PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(    0x00, "Current Ball Only")
	PORT_DIPSETTING(    0x20, "Players Remaining Balls")
	PORT_DIPNAME( 0x40, 0x00, "Bank Shot")                          PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, "Starts At 300 Points")
	PORT_DIPSETTING(    0x40, "Starts At 600 Points and 2X")
	PORT_DIPNAME( 0x80, 0x00, "Top Lanes")                          PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, "Separate")
	PORT_DIPSETTING(    0x80, "1/9 With 4/12 and 2/10 With 3/11")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x20, 0x20, "Special Light Awards")               PORT_DIPLOCATION("SW3:!6")
	PORT_DIPSETTING(    0x00, "15,000 Points")
	PORT_DIPSETTING(    0x20, "1 Credit")
	PORT_DIPNAME( 0x40, 0x40, "Extra Ball Light Awards")            PORT_DIPLOCATION("SW3:!7")
	PORT_DIPSETTING(    0x00, "15,000 Points")
	PORT_DIPSETTING(    0x40, "Extra Ball")
	PORT_DIPNAME( 0x80, 0x80, "Score Level Awards")                 PORT_DIPLOCATION("SW3:!8")
	PORT_DIPSETTING(    0x00, "Extra Ball")
	PORT_DIPSETTING(    0x80, "1 Credit")

	PORT_MODIFY("X2")   /* Spinner switch */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, spinner, (void *)0x1201)  // PORT_CODE(KEYCODE_COMMA)
INPUT_PORTS_END


static INPUT_PORTS_START( matahari )
	PORT_INCLUDE( by17 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x60, 0x60, "Beating Highest Score Awards")           PORT_DIPLOCATION("SW0:!6,!7")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x20, "1 Credit")
	PORT_DIPSETTING(    0x40, "2 Credits")
	PORT_DIPSETTING(    0x60, "3 Credits")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 15")                               PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 22")                               PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "Saucer Award Feature")                   PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, "Start at 3000 Points")
	PORT_DIPSETTING(    0x40, "Start at 2X Bonus")
	PORT_DIPNAME( 0x80, 0x00, "A & B Special Award Feature Per Ball")   PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, "Award Special Once")
	PORT_DIPSETTING(    0x80, "Award Special Alternates")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 29")                               PORT_DIPLOCATION("SW3:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x60, 0x60, "Extra Ball / Specials Award Mode")       PORT_DIPLOCATION("SW3:!6,!7")
	PORT_DIPSETTING(    0x20, "Unused Novelty / 50,000")
	PORT_DIPSETTING(    0x00, "Novelty / 50,000")
	PORT_DIPSETTING(    0x40, "Extra Ball / 50,000")
	PORT_DIPSETTING(    0x60, "Extra Ball / Replay")
	PORT_DIPNAME( 0x80, 0x80, "Score Level Award")                      PORT_DIPLOCATION("SW3:!8")
	PORT_DIPSETTING(    0x00, "Extra Ball")
	PORT_DIPSETTING(    0x80, "Replay")

	PORT_MODIFY("X2")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0201)  // PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0202)  // PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0204)  // PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0208)  // PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END


static INPUT_PORTS_START( pwerplay )
	PORT_INCLUDE( by17 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x60, 0x60, "Beating Highest Score Awards")       PORT_DIPLOCATION("SW0:!6,!7")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x20, "1 Credit")
	PORT_DIPSETTING(    0x40, "2 Credits")
	PORT_DIPSETTING(    0x60, "3 Credits")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Drop Target Award Sequence")         PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, "5X then Extra Ball")
	PORT_DIPSETTING(    0x20, "5X and Extra Ball")
	PORT_DIPNAME( 0x40, 0x00, "Side Rollover Buttons Score")        PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, "100 Points")
	PORT_DIPSETTING(    0x40, "1,000 Points")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "Drop Target Bank Reset")             PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(    0x00, "Reset Both Banks")
	PORT_DIPSETTING(    0x20, "Reset Completed Bank Only")
	PORT_DIPNAME( 0x40, 0x00, "Pop Bumper Scores")                  PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, "Alternate 1,000 Points Top and Bottom")
	PORT_DIPSETTING(    0x40, "All Score 1,000 Points When Lit")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x30, 0x20, "Top Saucer Specials Feature")        PORT_DIPLOCATION("SW3:!5,!6")
	PORT_DIPSETTING(    0x10, DEF_STR( Unknown))
	PORT_DIPSETTING(    0x00, "Outlane Specials Do Not Light")
	PORT_DIPSETTING(    0x20, "Outlane Specials Alternate")
	PORT_DIPSETTING(    0x30, "Outlane Specials Both Light")
	PORT_DIPNAME( 0xc0, 0xc0, "Award Mode")                         PORT_DIPLOCATION("SW3:!7,!8")
	PORT_DIPSETTING(    0x40, "Unused Novelty / 50,000")
	PORT_DIPSETTING(    0x00, "Novelty / 50,000")
	PORT_DIPSETTING(    0x80, "Extra Ball / 50,000")
	PORT_DIPSETTING(    0xc0, "Extra Ball / Replay")

	PORT_MODIFY("X2")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0201)  // PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0202)  // PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0204)  // PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0208)  // PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END

static INPUT_PORTS_START( stk_sprs )
	PORT_INCLUDE( by17 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x60, 0x60, "Beating Highest Score Awards")       PORT_DIPLOCATION("SW0:!6,!7")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x20, "1 Credit")
	PORT_DIPSETTING(    0x40, "2 Credits")
	PORT_DIPSETTING(    0x60, "3 Credits")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x60, 0x60, "Score Level Awards")                 PORT_DIPLOCATION("SW1:!6,!7")
	PORT_DIPSETTING(    0x20, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x40, "Extra Ball")
	PORT_DIPSETTING(    0x60, "Replay")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "Bonus Multiplier")                   PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(    0x00, "Reset Next Ball In Play")
	PORT_DIPSETTING(    0x20, "Remembered Next Ball In Play")
	PORT_DIPNAME( 0x40, 0x00, "Strike Starting Value")              PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, "None")
	PORT_DIPSETTING(    0x40, "5000 Points")
	PORT_DIPNAME( 0x80, 0x00, "Extra Ball Lane Lights")             PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x80, "Both On")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x10, 0x00, "Pins 1-3, 2-5, 8-9")                 PORT_DIPLOCATION("SW3:!5")
	PORT_DIPSETTING(    0x00, "Separate")
	PORT_DIPSETTING(    0x10, "Awarded Together")
	PORT_DIPNAME( 0x20, 0x00, "A-B-C Lanes Awards Special On")      PORT_DIPLOCATION("SW3:!6")
	PORT_DIPSETTING(    0x00, "5th Completion")
	PORT_DIPSETTING(    0x20, "4th Completion")
	PORT_DIPNAME( 0x40, 0x00, "B Middle Lane Strike Light")         PORT_DIPLOCATION("SW3:!7")
	PORT_DIPSETTING(    0x00, "Alternates")
	PORT_DIPSETTING(    0x40, "Always On")
	PORT_DIPNAME( 0x80, 0x00, "Special Outlane Lights")             PORT_DIPLOCATION("SW3:!8")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x80, "Both On")

	PORT_MODIFY("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, spinner, (void *)0x1301)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END



CUSTOM_INPUT_MEMBER( by17_state::spinner)
{
	uint16_t data = (uintptr_t)param & 0xffff;
	uint8_t bit_mask = data & 0xff;
	uint8_t port = (data >> 8) & 0x07;
	uint8_t spinner_num = ((data >> 12) - 1) & 0x03;
	bool state = false;


	switch (data & 0x7ff)
	{
		case 0x010:  if (machine().input().code_pressed(KEYCODE_STOP))  state = true; break;  // X0:I4
		case 0x201:  if (machine().input().code_pressed(KEYCODE_COMMA)) state = true; break;  // X2:I0
		case 0x208:  if (machine().input().code_pressed(KEYCODE_B))     state = true; break;  // X2:I3
		case 0x210:  if (machine().input().code_pressed(KEYCODE_V))     state = true; break;  // X2:I4
		case 0x240:  if (machine().input().code_pressed(KEYCODE_X))     state = true; break;  // X2:I6
		case 0x301:  if (machine().input().code_pressed(KEYCODE_K))     state = true; break;  // X3:I0
		case 0x320:  if (machine().input().code_pressed(KEYCODE_D))     state = true; break;  // X3:I5
		case 0x340:  if (machine().input().code_pressed(KEYCODE_S))     state = true; break;  // X3:I6
		default:     break;
	}

	/* Here we simulate the Spinners */
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

CUSTOM_INPUT_MEMBER( by17_state::outhole )
{
	uint8_t bit_mask = ((uintptr_t)param & 0xff);
	uint8_t port = (((uintptr_t)param >> 8) & 0x07);

	/* Here we simulate the ball sitting in the Outhole so the Outhole Solenoid can release it */

	if (machine().input().code_pressed_once(KEYCODE_BACKSPACE))  m_io_hold_x[port] |= bit_mask;

	return ((m_io_hold_x[port] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( by17_state::switch_hold_x0 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x01) && machine().input().code_pressed_once(KEYCODE_BACKSLASH))   m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_CLOSEBRACE))  m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_OPENBRACE))   m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_SLASH))       m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_STOP))        m_io_hold_x[0] |= bit_mask;

	return ((m_io_hold_x[0] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( by17_state::switch_hold_x1 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_ENTER))  m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_QUOTE))  m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x20) && machine().input().code_pressed_once(KEYCODE_COLON))  m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x40) && machine().input().code_pressed_once(KEYCODE_L))      m_io_hold_x[1] |= bit_mask;

	return ((m_io_hold_x[1] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( by17_state::switch_hold_x2 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x01) && machine().input().code_pressed_once(KEYCODE_COMMA))  m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_M))      m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_N))      m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_B))      m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_V))      m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x20) && machine().input().code_pressed_once(KEYCODE_C))      m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x40) && machine().input().code_pressed_once(KEYCODE_X))      m_io_hold_x[2] |= bit_mask;
	if ((bit_mask & 0x80) && machine().input().code_pressed_once(KEYCODE_Z))      m_io_hold_x[2] |= bit_mask;

	return ((m_io_hold_x[2] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( by17_state::switch_hold_x3 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x01) && machine().input().code_pressed_once(KEYCODE_K))  m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_J))  m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_H))  m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_G))  m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_F))  m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x20) && machine().input().code_pressed_once(KEYCODE_D))  m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x40) && machine().input().code_pressed_once(KEYCODE_S))  m_io_hold_x[3] |= bit_mask;
	if ((bit_mask & 0x80) && machine().input().code_pressed_once(KEYCODE_A))  m_io_hold_x[3] |= bit_mask;

	return ((m_io_hold_x[3] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( by17_state::switch_hold_x4 )
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


READ8_MEMBER(by17_state::nibble_nvram_r)
{
	return (m_nvram[offset] | 0x0f);
}

WRITE8_MEMBER(by17_state::nibble_nvram_w)
{
	m_nvram[offset] = (data | 0x0f);
}

INPUT_CHANGED_MEMBER( by17_state::activity_button )
{
	if (newval != oldval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, (newval ? ASSERT_LINE : CLEAR_LINE));
}

INPUT_CHANGED_MEMBER( by17_state::self_test )
{
	m_pia_u10->ca1_w(newval);
}

READ_LINE_MEMBER( by17_state::u10_ca1_r )
{
	return (m_io_test->read() & 0x01);
}
READ_LINE_MEMBER( by17_state::u10_cb1_r )
{
	return m_u10_cb1;
}

WRITE_LINE_MEMBER( by17_state::u10_ca2_w )
{
#if 0                   // Display Blanking - Out of sync with video redraw rate and causes flicker so it's disabled
	if (m_u10_ca2 && (state == false))
	{
		for (int digit = 1; digit <= 8; digit++)
		{
			m_display[0][digit] = 0;
			m_display[1][digit] = 0;
			m_display[2][digit] = 0;
			m_display[3][digit] = 0;
			m_display[4][digit] = 0;
		}
	}
#endif

	if ((m_u10_ca2 == false) && state)
	{
		static constexpr uint8_t patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 };  // MC14543 - BCD to 7 Segment Display Decoder

		m_display[0][m_digit] = patterns[m_segment[0]];  // Credits/Match/BallInPlay
		m_display[1][m_digit] = patterns[m_segment[1]];  // Player 1
		m_display[2][m_digit] = patterns[m_segment[2]];  // Player 2
		m_display[3][m_digit] = patterns[m_segment[3]];  // Player 3
		m_display[4][m_digit] = patterns[m_segment[4]];  // Player 4

		LOG("Display Player 0 = %02x: %02x written to digit %01x of Player 0 display. Segment is %02x\n", m_display[0][m_digit], patterns[m_segment[0]], m_digit, m_segment[0]);
	}

	m_u10_ca2 = state;
}

WRITE_LINE_MEMBER( by17_state::u10_cb2_w )
{
	LOG("New U10 CB2 state %01x, was %01x.   PIA=%02x\n", state, m_u10_cb2, m_u10a);

	if ((state) || ((m_u10_cb2==true) && (state==false)))
	{
		if (m_lamp_decode != (m_u10a & 0x0f))
		{
			if ((m_u10a & 0x0f) < 0x0f)
			{
				update_lamps(0, 15, (m_u10a & 0x0f), m_u10a);
				LOG("ON:  Board 0, Lamp_decode now=%02x, Lamp_decode was %02x,  U10-PortA=%02x\n", (m_u10a & 0x0f), m_lamp_decode, m_u10a);
			}
		}
		m_lamp_decode = (m_u10a & 0x0f);
	}

	m_u10_cb2 = state;
}

WRITE_LINE_MEMBER( by17_state::u11_ca2_w )
{
	LOG("New U11 CA2 state %01x, was %01x.   U10-PIA-PortA=%02x\n", state, m_u11_ca2, m_u10a);

	output().set_value("led0", state);

	m_u11_ca2 = state;
}

READ_LINE_MEMBER( by17_state::u11_ca1_r )
{
	return m_u11_ca1;
}

READ_LINE_MEMBER( by17_state::u11_cb1_r )
{
	/* Pin 32 on MPU J5 AID connector tied low */
	return false;
}

WRITE_LINE_MEMBER( by17_state::u11_cb2_w )
{
	m_u11_cb2 = state;
}

READ8_MEMBER( by17_state::u10_a_r )
{
	return m_u10a;
}

WRITE8_MEMBER( by17_state::u10_a_w )
{
	LOG("Writing %02x to U10 PIA, CB2 state is %01x,  CA2 state is %01x, Lamp_Dec is %02x\n",data, m_u10_cb2, m_u10_ca2, (m_lamp_decode & 0x0f));

	if (!m_u10_ca2)
	{
		if (BIT(m_u11a, 0) == 0)            // Credit/Ball in Play Display
			m_segment[0] = (data >> 4);
		if (BIT(data, 0) == 0)              // Player Display 1
			m_segment[1] = (data >> 4);
		if (BIT(data, 1) == 0)              // Player Display 2
			m_segment[2] = (data >> 4);
		if (BIT(data, 2) == 0)              // Player Display 3
			m_segment[3] = (data >> 4);
		if (BIT(data, 3) == 0)              // Player Display 4
			m_segment[4] = (data >> 4);
	}


	if (m_u10_cb2)
		m_lamp_decode = (data & 0x0f);

	if (m_lamp_decode < 0x0f)
		update_lamps(0, 15, m_lamp_decode, data);


	m_u10a = data;
}

READ8_MEMBER( by17_state::u10_b_r )
{
	uint8_t data = 0;

	if (BIT(m_u10a, 0))
		data |= m_io_x0->read();

	if (BIT(m_u10a, 1))
		data |= m_io_x1->read();

	if (BIT(m_u10a, 2))
		data |= m_io_x2->read();

	if (BIT(m_u10a, 3))
		data |= m_io_x3->read();

	if (BIT(m_u10a, 4))
		data |= m_io_x4->read();

	if (BIT(m_u10a, 5))
		data |= m_io_dsw0->read();

	if (BIT(m_u10a, 6))
		data |= m_io_dsw1->read();

	if (BIT(m_u10a, 7))
		data |= m_io_dsw2->read();

	if (m_u10_cb2)
		data |= m_io_dsw3->read();

	return data;
}

WRITE8_MEMBER( by17_state::u10_b_w )
{
	m_u10b = data;
}

READ8_MEMBER( by17_state::u11_a_r )
{
	return m_u11a;
}

WRITE8_MEMBER( by17_state::u11_a_w )
{
	if (!m_u10_ca2)
	{
		if (BIT(data, 0) == 0)            // Credit/Ball in Play Display
			m_segment[0] = (m_u10a >> 4);
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
	else if ((BIT(data, 3)) && (BIT(data, 2)))   // Aftermarket 7th digit strobe for 6 digit games
		m_digit = 7;
	else if (BIT(data, 3))
		m_digit = 5;
	else if (BIT(data, 2))
		m_digit = 6;
	else if (BIT(data, 1))
		m_digit = 7;

	m_u11a = data;
}

WRITE8_MEMBER( by17_state::u11_b_w )
{
	if (!m_u11_cb2)
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


	if ((m_u11b & 0x10) && ((data & 0x10) == 0))
	{
		m_solenoids[16] = 1;
		if (m_solenoid_features[16][0] != 0xff)
			m_samples->start(m_solenoid_features[16][0], m_solenoid_features[16][1]);
	}
	else if ((data & 0x10) && ((m_u11b & 0x10) == 0))
	{
		m_solenoids[16] = 0;
		if (m_solenoid_features[16][0] != 0xff)
			m_samples->start(m_solenoid_features[16][0], m_solenoid_features[16][2]);
	}
	if ((m_u11b & 0x20) && ((data & 0x20) == 0))
	{
		m_solenoids[17] = 1;                                // Coin Lockout Coil engage
		if (m_solenoid_features[17][0] != 0xff)
			m_samples->start(m_solenoid_features[17][0], m_solenoid_features[17][1]);
	}
	else if ((data & 0x20) && ((m_u11b & 0x20) == 0))
	{
		m_solenoids[17] = 0;                               // Coin Lockout Coil release
		if (m_solenoid_features[17][0] != 0xff)
			m_samples->start(m_solenoid_features[17][0], m_solenoid_features[17][2]);
	}
	if ((m_u11b & 0x40) && ((data & 0x40) == 0))
	{
		m_solenoids[18] = 1;                                // Flipper Enable Relay engage
		if (m_solenoid_features[18][0] != 0xff)
			m_samples->start(m_solenoid_features[18][0], m_solenoid_features[18][1]);
	}
	else if ((data & 0x40) && ((m_u11b & 0x40) == 0))
	{
		m_solenoids[18] = 0;                               // Flipper Enable Relay release
		if (m_solenoid_features[18][0] != 0xff)
			m_samples->start(m_solenoid_features[18][0], m_solenoid_features[18][2]);
	}
	if ((m_u11b & 0x80) && ((data & 0x80) == 0))
	{
		m_solenoids[19] = 1;
		if (m_solenoid_features[19][0] != 0xff)
			m_samples->start(m_solenoid_features[19][0], m_solenoid_features[19][1]);
	}
	else if ((data & 0x80) && ((m_u11b & 0x80) == 0))
	{
		m_solenoids[19] = 0;
		if (m_solenoid_features[19][0] != 0xff)
			m_samples->start(m_solenoid_features[19][0], m_solenoid_features[19][2]);
	}

	m_u11b = data;
}


void by17_state::update_lamps(u8 lamp_board_base, u8 lamp_decoder_base, u8 lamp_decoder_addr, u8 lamp_decoder_data)
{
	if ((lamp_board_base + lamp_decoder_addr) != 15) 
	{
	// Update the Lamps on the Lamp Driver Board only if their respective SCRs aren't already latched
		if (!m_lamps[(lamp_board_base + (0*lamp_decoder_base) + lamp_decoder_addr)])
			 m_lamps[(lamp_board_base + (0*lamp_decoder_base) + lamp_decoder_addr)] = !BIT(lamp_decoder_data, 4);
		if (!m_lamps[(lamp_board_base + (1*lamp_decoder_base) + lamp_decoder_addr)])
			 m_lamps[(lamp_board_base + (1*lamp_decoder_base) + lamp_decoder_addr)] = !BIT(lamp_decoder_data, 5);
		if (!m_lamps[(lamp_board_base + (2*lamp_decoder_base) + lamp_decoder_addr)])
			 m_lamps[(lamp_board_base + (2*lamp_decoder_base) + lamp_decoder_addr)] = !BIT(lamp_decoder_data, 6);
		if (!m_lamps[(lamp_board_base + (3*lamp_decoder_base) + lamp_decoder_addr)])
			 m_lamps[(lamp_board_base + (3*lamp_decoder_base) + lamp_decoder_addr)] = !BIT(lamp_decoder_data, 7);
	}
	else
	{
		// Rest output - all lamps are off
	}
}


TIMER_DEVICE_CALLBACK_MEMBER( by17_state::timer_z_freq )
{
/*  Zero Crossing Detector - this timing is based on 50Hz AC line power input converted to unregulated DC

    -+                          +---+
     |                          |   |
     |<-------- 9.30ms -------->|<->|700us
     |                          |   |
     +--------------------------+   +-----
*/

	m_zero_crossing_active_timer->adjust(attotime::from_usec(700));

	m_u10_cb1 = true;
	m_pia_u10->cb1_w(m_u10_cb1);

	/*** Zero Crossing - power to all Lamp SCRs is cut off and reset ***/
	std::fill(std::begin(m_lamps), std::end(m_lamps), 0);
}
TIMER_DEVICE_CALLBACK_MEMBER( by17_state::timer_z_pulse )
{
	/*** Line Power to DC Zero Crossing has ended ***/

	m_u10_cb1 = false;
	m_pia_u10->cb1_w(m_u10_cb1);
}

TIMER_DEVICE_CALLBACK_MEMBER( by17_state::u11_timer )
{
/*   555 timer for display interrupt generator

     +--------------------------+   +-----
     |                          |   |
     |<-------- 2.85ms -------->|<->|300us
     |                          |   |
    -+                          +---+
*/

	m_display_refresh_timer->adjust(attotime::from_usec(2850));

	m_u11_ca1 = true;
	m_pia_u11->ca1_w(m_u11_ca1);
}

TIMER_DEVICE_CALLBACK_MEMBER( by17_state::timer_d_pulse )
{
	m_u11_ca1 = false;
	m_pia_u11->ca1_w(m_u11_ca1);
}


by17_state::solenoid_feature_data const by17_state::s_solenoid_features_default =
{
	// Assign a mechanical audio sample corresponding to a solenoid function, and optionally:
	//  - release any switches being held closed eg. fallen drop targets, ball sitting in outhole/saucer/kickbacks, etc
	//  - activate any switches eg. make a drop target fall, etc
	// The following table serves these functions and is configured on a per game basis:
	// 00-15 { SC, AS, SSB, SRB } Momentary coils
	//                      SRB = Switch Return Bits used to release/assert a switch depending on game specific usage. Bit is active Hi
	//                 SSB      = Switch Strobe Bits that the Return Bits lives within the switch matrix
	//                 SSB        Bit 0x80 Hi indicates we want to activate a switch, Lo indicates we want to release a switch
	//             AS           = Audio Sample (mechanical and/or chime) to playback
	//         SC               = MAME Sound Channel to play the audio sample on (make sure samples that might play at the same time are on different channels)
	//         SC                 Value 0xff indicates not to play any audio sample
	//
	//
	// 16-19 { SC, SE, SR, N/U } Continuous coils
	//                     N/U = Not Used
	//                 SR      = Audio Sample Release to playback
	//             SE          = Audio Sample Engage to playback
	//         SC              = MAME Sound Channel to play the audio sample on (make sure samples that might play at the same time are on different channels)
	//         SC                Value 0xff indicates not to play any audio sample
	//
	//      {  SC,   AS,    SSB,  SRB }
	/*00*/  { 0x00, 0x00,  0x00, 0x00 },
	/*01*/  { 0x05, 0x01,  0x00, 0x00 },     // Chime 10
	/*02*/  { 0x05, 0x02,  0x00, 0x00 },     // Chime 100
	/*03*/  { 0x05, 0x03,  0x00, 0x00 },     // Chime 1000
	/*04*/  { 0x05, 0x04,  0x00, 0x00 },     // Chime 10000
	/*05*/  { 0x04, 0x00,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x00, 0x00,  0x00, 0x00 },
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },
	/*12*/  { 0x00, 0x00,  0x00, 0x00 },
	/*13*/  { 0x02, 0x00,  0x00, 0x00 },
	/*14*/  { 0x00, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//      {  SC,   SE,   SR,    N/U }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};



by17_state::solenoid_feature_data const freedom_state::s_solenoid_features_freedom =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x05, 0x01,  0x00, 0x00 },     // Chime 10
	/*01*/  { 0x05, 0x02,  0x00, 0x00 },     // Chime 100
	/*02*/  { 0x05, 0x03,  0x00, 0x00 },     // Chime 1000
	/*03*/  { 0x05, 0x04,  0x00, 0x00 },     // Chime 10000
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x04, 0x80 },     // Saucer Top
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x03, 0x0b,  0x01, 0x1f },     // Drop Target Reset Left
	/*13*/  { 0x02, 0x05,  0x04, 0x80 },     // Saucer Rightside
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by17_state::solenoid_feature_data const nightrdr_state::s_solenoid_features_nightrdr =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x05, 0x01,  0x00, 0x00 },     // Chime 10
	/*01*/  { 0x05, 0x02,  0x00, 0x00 },     // Chime 100
	/*02*/  { 0x05, 0x03,  0x00, 0x00 },     // Chime 1000
	/*03*/  { 0x05, 0x04,  0x00, 0x00 },     // Chime 10000
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x04, 0x20 },     // Saucer
	/*08*/  { 0x02, 0x0b,  0x01, 0x1f },     // Drop Target Reset Left
	/*09*/  { 0x03, 0x0b,  0x08, 0x1f },     // Drop Target Reset Right
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by17_state::solenoid_feature_data const evelknie_state::s_solenoid_features_evelknie =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0x05, 0x01,  0x00, 0x00 },     // Chime 10
	/*02*/  { 0x05, 0x02,  0x00, 0x00 },     // Chime 100
	/*03*/  { 0x05, 0x03,  0x00, 0x00 },     // Chime 1000
	/*04*/  { 0x05, 0x04,  0x00, 0x00 },     // Chime 10000
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x08, 0x10 },     // Saucer
	/*08*/  { 0x00, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*09*/  { 0x00, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x03, 0x0b,  0x01, 0x1f },     // Drop Target Reset Left
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by17_state::solenoid_feature_data const eightbll_state::s_solenoid_features_eightbll =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0x05, 0x01,  0x00, 0x00 },     // Chime 10
	/*02*/  { 0x05, 0x02,  0x00, 0x00 },     // Chime 100
	/*03*/  { 0x05, 0x03,  0x00, 0x00 },     // Chime 1000
	/*04*/  { 0x05, 0x04,  0x00, 0x00 },     // Chime 10000
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x0a,  0x10, 0x02 },     // Kickback Left Outlane
	/*08*/  { 0x00, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*09*/  { 0x00, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0xff, 0x00,  0x00, 0x00 },
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by17_state::solenoid_feature_data const blackjck_state::s_solenoid_features_blackjck =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0x05, 0x01,  0x00, 0x00 },     // Chime 10
	/*02*/  { 0x05, 0x02,  0x00, 0x00 },     // Chime 100
	/*03*/  { 0x05, 0x03,  0x00, 0x00 },     // Chime 1000
	/*04*/  { 0x05, 0x04,  0x00, 0x00 },     // Chime 10000
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x08, 0x80 },     // Saucer
	/*08*/  { 0x00, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*09*/  { 0x00, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*13*/  { 0xff, 0x00,  0x00, 0x00 },
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};


by17_state::solenoid_feature_data const matahari_state::s_solenoid_features_matahari =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x02, 0x05,  0x08, 0x80 },     // Saucer
	/*01*/  { 0x05, 0x01,  0x00, 0x00 },     // Chime 10
	/*02*/  { 0x05, 0x02,  0x00, 0x00 },     // Chime 100
	/*03*/  { 0x05, 0x03,  0x00, 0x00 },     // Chime 1000
	/*04*/  { 0x05, 0x04,  0x00, 0x00 },     // Chime 10000
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left Bottom
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left Top
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right Top
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right Bottom
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x02, 0x0b,  0x04, 0xf0 },     // Drop Target Reset Left
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0x03, 0x0b,  0x04, 0x0f },     // Drop Target Reset Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by17_state::solenoid_feature_data const pwerplay_state::s_solenoid_features_pwerplay =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x00, 0x10,  0x00, 0x00 },     // Post Down
	/*01*/  { 0x05, 0x01,  0x00, 0x00 },     // Chime 10
	/*02*/  { 0x05, 0x02,  0x00, 0x00 },     // Chime 100
	/*03*/  { 0x05, 0x03,  0x00, 0x00 },     // Chime 1000
	/*04*/  { 0x05, 0x04,  0x00, 0x00 },     // Chime 10000
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x08, 0x80 },     // Saucer
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x02, 0x0b,  0x04, 0xf0 },     // Drop Target Reset Left
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0x03, 0x0b,  0x04, 0x0f },     // Drop Target Reset Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x11, 0x0f,  0x00 },     // Post Up
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by17_state::solenoid_feature_data const stk_sprs_state::s_solenoid_features_stk_sprs =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0x05, 0x01,  0x00, 0x00 },     // Chime 10
	/*02*/  { 0x05, 0x02,  0x00, 0x00 },     // Chime 100
	/*03*/  { 0x05, 0x03,  0x00, 0x00 },     // Chime 1000
	/*04*/  { 0x05, 0x04,  0x00, 0x00 },     // Chime 10000
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x08, 0x80 },     // Saucer
	/*08*/  { 0x00, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*09*/  { 0x00, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0xff, 0x00,  0x00, 0x00 },
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};


void by17_state::machine_start()
{
	genpin_class::machine_start();

	m_lamps.resolve();
	m_display.resolve();
	m_solenoids.resolve();
	m_spinners.resolve();
}

void by17_state::machine_reset()
{
	genpin_class::machine_reset();

	render_target *target = machine().render().first_target();

	target->set_view(0);

	m_u10a = 0xff;
	m_u10b = 0;
	m_u11a = 0;
	m_u11b = 0;
	m_lamp_decode = 0x0f;
	m_io_hold_x[0] = m_io_hold_x[1] = m_io_hold_x[2] = m_io_hold_x[3] = m_io_hold_x[4] = 0;   // Clear the temp switch holds

	if (m_io_outhole->read())           // Put a ball in the Outhole at power-on if selected in the machine configuration
		m_io_hold_x[(m_outhole >> 8) & 7] = m_outhole & 0xff;

	if (m_io_zx->read())                // Machine configuration sets the Zero Crossing frequency based countries DC rectified mains line frequency
		m_zero_crossing_freq_timer->adjust(attotime::from_hz(120), 0, attotime::from_hz(120));  // Country whose mains line frequency is 60Hz
	else
		m_zero_crossing_freq_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));  // Country whose mains line frequency is 50Hz
}



MACHINE_CONFIG_START(by17_state::by17)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M6800, 530000) // No xtal, just 2 chips forming a multivibrator oscillator around 530kHz
	MCFG_DEVICE_PROGRAM_MAP(by17_map)

	MCFG_NVRAM_ADD_0FILL("nvram")   // 'F' filled causes Credit Display to be blank on first startup

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_by17)

	/* Sound */
	genpin_audio(config);

	/* Devices */
	MCFG_DEVICE_ADD("pia_u10", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, by17_state, u10_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, by17_state, u10_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(*this, by17_state, u10_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by17_state, u10_b_w))
	MCFG_PIA_READCA1_HANDLER(READLINE(*this, by17_state, u10_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(*this, by17_state, u10_cb1_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(*this, by17_state, u10_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by17_state, u10_cb2_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_TIMER_DRIVER_ADD("timer_z_freq", by17_state, timer_z_freq)						// Mains Line Frequency * 2 (100Hz or 120Hz depending on country)
	MCFG_TIMER_DRIVER_ADD(m_zero_crossing_active_timer, by17_state, timer_z_pulse)		// Active pulse length from Zero Crossing detector

	MCFG_DEVICE_ADD("pia_u11", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, by17_state, u11_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, by17_state, u11_a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by17_state, u11_b_w))
	MCFG_PIA_READCA1_HANDLER(READLINE(*this, by17_state, u11_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(*this, by17_state, u11_cb1_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(*this, by17_state, u11_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by17_state, u11_cb2_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_d_freq", by17_state, u11_timer, PERIOD_OF_555_ASTABLE(36000, 4700, 0.0000001))	// 555 Timer 317-320Hz
	MCFG_TIMER_DRIVER_ADD(m_display_refresh_timer, by17_state, timer_d_pulse)												// 555 Active pulse length
MACHINE_CONFIG_END




/*------------------------------------------------------------------
/ Bow and Arrow #1033 (prototype only, slightly different hardware)
/ not sure yet if it belongs in this driver
/-------------------------------------------------------------------*/
ROM_START(bowarrow)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("b14.bin", 0x1400, 0x0200, CRC(d4d0f92a) SHA1(b996cbe9762fafd64115dc78e24626cf08f8abf7))
	ROM_LOAD("b16.bin", 0x1600, 0x0200, CRC(ad2102e7) SHA1(86887beea5e03e80f60c947d6d71431e5eab3d1b))
	ROM_LOAD("b18.bin", 0x1800, 0x0200, CRC(5d84656b) SHA1(d17350f5a0cc0cd00b60df4903034489dce7ade5))
	ROM_LOAD("b1a.bin", 0x1a00, 0x0200, CRC(6f083ce6) SHA1(624b00e72e223c6b9fbf38b831200c9a7aa0d8f7))
	ROM_LOAD("b1c.bin", 0x1c00, 0x0200, CRC(6ed4d39e) SHA1(1f6c57c7274c76246dd2f0b70ec459857a5cf1eb))
	ROM_LOAD("b1e.bin", 0x1e00, 0x0200, CRC(ff2f97de) SHA1(28a8fdeccb1382d3a1153c97466426459c9fa075))
ROM_END

ROM_START(bowarrowa)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("u42704", 0x1400, 0x0200, CRC(b2ccd455) SHA1(07ba19ce2bcd0d2d0d27cab5aafd510423d2e8fe))
	ROM_LOAD("u32704", 0x1600, 0x0200, CRC(ec673d77) SHA1(f350df32182da4958b4607db6952ffce89cda18f))
	ROM_LOAD("u22704", 0x1800, 0x0200, CRC(4b4512da) SHA1(b427c504667769bd3b5c78ad3866c750cb7b1ed4))
	ROM_LOAD("u12704", 0x1a00, 0x0200, CRC(a35e7473) SHA1(d5cadd968cad931dfe92d6ba4f013844f5b5d244))
	ROM_LOAD("u62704", 0x1c00, 0x0200, CRC(8c421ae4) SHA1(93fcf26667308467215d35685c1acb04b0555d6b))
	ROM_LOAD("u52704", 0x1e00, 0x0200, CRC(a6644eee) SHA1(8afa941d1dd94308272bbc1874603d3ed41d3b89))
ROM_END

/*--------------------------------
/ Freedom #1066
/-------------------------------*/
ROM_START(freedom)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "720-08_1.474", 0x1400, 0x0200, CRC(b78bceeb) SHA1(acf6f1a497ada344211f12dbf4be619bee559950))
	ROM_LOAD( "720-10_2.474", 0x1600, 0x0200, CRC(ca90c8a7) SHA1(d9b5e95247e846e50a2a43c85ad5eb1fc761ab67))
	ROM_LOAD( "720-07_6.716", 0x1800, 0x0800, CRC(0f4e8b83) SHA1(faa05dde24eb60be0cdc4456ae2e660a15ed85ac))
ROM_END

/*--------------------------------
/ Night Rider #1074
/-------------------------------*/
ROM_START(nightrdr)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "721-21_1.716", 0x1000, 0x0800, CRC(237c4060) SHA1(4ce3dba9189fe7666fc76a2c8ee7fff9b12d4c00))
	ROM_LOAD( "720-21_6.716", 0x1800, 0x0800, CRC(f394e357) SHA1(73444f848825a398515153d18de027792b57bcc7))   // PROM 9316A-2481
ROM_END

ROM_START(nightr20)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "721-21_1.716", 0x1000, 0x0800, CRC(237c4060) SHA1(4ce3dba9189fe7666fc76a2c8ee7fff9b12d4c00))
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279)) // sldh
ROM_END

/*--------------------------------
/ Black Jack #1092
/-------------------------------*/
ROM_START(blackjck)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "728-32_2.716", 0x1000, 0x0800, CRC(1333c9d1) SHA1(1fbb60d84db47ffaf7f291575b2705783a110678))   // PROM 9316A-2483?
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))   // PROM 9316A-2454
ROM_END

/*--------------------------------
/ Evel Knievel #1094
/-------------------------------*/
ROM_START(evelknie)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "722-17_2.716", 0x1000, 0x0800, CRC(b6d9a3fa) SHA1(1939e13f73a324e3d2fd269a54446f48cf530f50))   // PROM 9316A-2459
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))   // PROM 9316A-2454
ROM_END

#if 0	// Not dumped yet
ROM_START(evelknio)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "722-11_2.716", 0x1000, 0x0800, CRC(00000000))   // PROM 9316A-2456
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))   // PROM 9316A-2454
ROM_END
#endif

/*--------------------------------
/ Mata Hari #1104
/-------------------------------*/
ROM_START(matahari)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "725-21_2.716", 0x1000, 0x0800, CRC(63acd9b0) SHA1(2347342f1281c097ea39c79236d85b00a1dfc7b2))   // PROM 9316A-2432
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))   // PROM 9316A-2454
ROM_END

/*--------------------------------
/ Eight Ball #1118
/-------------------------------*/
ROM_START(eightbll)   // Bally fixed exploit in previous 17.u2 version - next ball will not be served until the Tilt switch is released
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "723-20_2.716", 0x1000, 0x0800, CRC(33559e7b) SHA1(49008db95c8f012e7e3b613e6eee811512207fa9))
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))   // PROM 9316A-2454
ROM_END

ROM_START(eightblo)   // Has an exploit if you tilt the ball in play and hold tilt active, after the ball enters the outhole the next ball will be served with tilt state flagged off and the tilt switch is ignored until it's released
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "e723-17.u2", 0x1000, 0x0800, CRC(7e7554ae) SHA1(e03c47c4a7a7352293f246ae5bff970fb53fcd88))
	ROM_LOAD( "e720-20.u6", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------
/ Power Play #1120
/-------------------------------*/
ROM_START(pwerplay)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "724-25_2.716", 0x1000, 0x0800, CRC(43012f35) SHA1(f90d582e3394d949a637a09882ffdad7664c44c0))   // PROM 9316A-2480
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))   // PROM 9316A-2454
ROM_END

/*--------------------------------
/ Strikes and Spares #1135
/-------------------------------*/
ROM_START(stk_sprs)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "740-16_2.716", 0x1000, 0x0800, CRC(2be27024) SHA1(266dee3a5c4c115acc20543df2eb172f1e85dacb))
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------------------------------------
/ Stellar Airship / Geiger-Automatenbau GMBH, of Germany (1981)
/---------------------------------------------------------------*/
// Outer space re-theme kit for Eight Ball. Probably uses Eight Ball ROMs





//  (  YEAR  NAME          PARENT      MACHINE    INPUT      STATE            INIT        MONITOR COMPANY  FULLNAME                                         FLAGS )
GAME(  1976, bowarrow,     0,          by17,      by17,      by17_state,      init_by17,    ROT0, "Bally", "Bow & Arrow (Prototype)",                       MACHINE_IS_SKELETON_MECHANICAL)
GAME(  1976, bowarrowa,    bowarrow,   by17,      by17,      by17_state,      init_by17,    ROT0, "Bally", "Bow & Arrow (Prototype, rev. 22)",              MACHINE_IS_SKELETON_MECHANICAL)
GAMEL( 1977, freedom,      0,          by17,      freedom,   freedom_state,   init_by17,    ROT0, "Bally", "Freedom",                                       MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by17_freedom)
GAMEL( 1977, nightrdr,     0,          by17,      nightrdr,  nightrdr_state,  init_by17,    ROT0, "Bally", "Night Rider (rev. 21)",                         MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by17_nightrdr)
GAMEL( 1977, nightr20,     nightrdr,   by17,      nightrdr,  nightrdr_state,  init_by17,    ROT0, "Bally", "Night Rider (rev. 20)",                         MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by17_nightrdr)
GAMEL( 1978, blackjck,     0,          by17,      blackjck,  blackjck_state,  init_by17,    ROT0, "Bally", "Black Jack (Pinball)",                          MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by17_blackjck)
layout_by17_blackjck)
GAMEL( 1977, evelknie,     0,          by17,      evelknie,  evelknie_state,  init_by17,    ROT0, "Bally", "Evel Knievel",                                  MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by17_evelknie)
GAMEL( 1978, matahari,     0,          by17,      matahari,  matahari_state,  init_by17,    ROT0, "Bally", "Mata Hari",                                     MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by17_matahari)
GAMEL( 1977, eightbll,     0,          by17,      eightbll,  eightbll_state,  init_by17,    ROT0, "Bally", "Eight Ball (rev. 20)",                          MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by17_eightbll)
GAMEL( 1977, eightblo,     eightbll,   by17,      eightbll,  eightbll_state,  init_by17,    ROT0, "Bally", "Eight Ball (rev. 17)",                          MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by17_eightbll)
GAMEL( 1978, pwerplay,     0,          by17,      pwerplay,  pwerplay_state,  init_by17,    ROT0, "Bally", "Power Play (Pinball)",                          MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by17_pwerplay)
GAMEL( 1978, stk_sprs,     0,          by17,      stk_sprs,  stk_sprs_state,  init_by17,    ROT0, "Bally", "Strikes and Spares",                            MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by17_stk_sprs)

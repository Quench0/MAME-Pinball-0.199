// license:BSD-3-Clause
// copyright-holders:Robbbert, Quench
/********************************************************************************************

    PINBALL
    Stern MPU-100   (almost identical to Bally MPU-17)


ToDo:
- Sound board: Effects currently don't sound right. Mechanical chime simulation at address 0xC0 not yet implemented.


Notes:
- Machine Configuration options:
  1) MPU board Zero Crossing frequency can be set according to your countries Mains Power frequency
  2) Spinner simulation can be disabled or the rotation speed can be selected
  3) Ability to have (or not) a ball in the Outhole at Power-On

Schematic/hardware build errors:
- "Cosmic Princess", the orange and yellow spinner lamps are reversed on real machines.
  Videos of attract and game play show these lights scoring is advanced on the opposite spinner.
- "Wild Fyre", Manual has wrong playfield layout for switch locations, and mistakenly shows the Lectronamo playfield.
- "Nugent", Pop Bumpers shown in the manual don't align the switches to the solenoids correctly.
  Schematics currently not available so Pop Bumpers have been configured as per sister machine Lectronamo. 
- "Stingray", Schematics for the switch matrix shows the left and right outlane switches incorrectly reversed.
  Left and right flipper return lane switches are also incorrectly reversed on the switch matrix schematic.
- "Pinball", Schematics for the Solenoid Driver Board list the Upper Left and Right Drop Target Reset solenoids reversed.
- "Stars", After first showing of HSTD in attract mode at power-up, the player scores become blank instead of "00".

*********************************************************************************************/


#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "sound/discrete.h"
#include "render.h"
#include "speaker.h"

//#define VERBOSE 1
#include "logmacro.h"

#include "st_mp100.lh"
#include "st_mp100_pinball.lh"
#include "st_mp100_stingray.lh"
#include "st_mp100_stingrayF.lh"
#include "st_mp100_stars.lh"
#include "st_mp100_memlane.lh"
#include "st_mp100_lectrono.lh"
#include "st_mp100_nugent.lh"
#include "st_mp100_wildfyre.lh"
#include "st_mp100_dracula.lh"
#include "st_mp100_trident.lh"
#include "st_mp100_princess.lh"
#include "st_mp100_magic.lh"
#include "st_mp100_hothand.lh"
#include "st_mp200_sam.lh"
#include "st_mp200.lh"


class st_mp100_state : public genpin_class
{
public:
	st_mp100_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp100_state(mconfig, type, tag, s_solenoid_features_default)
	{ }

	void init_st_mp100()		{ m_outhole = 0x0080; }
	void init_st_mp200()		{ m_outhole = 0x0401; }

	DECLARE_INPUT_CHANGED_MEMBER(activity_button);
	DECLARE_INPUT_CHANGED_MEMBER(self_test);
	DECLARE_INPUT_CHANGED_MEMBER(reset_sam);
	DECLARE_CUSTOM_INPUT_MEMBER(outhole);
	DECLARE_CUSTOM_INPUT_MEMBER(spinner);
	DECLARE_CUSTOM_INPUT_MEMBER(kicker);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x0);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x1);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x2);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x3);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x4);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_nugent_dt);

	void st_mp100(machine_config &config);
	void st_mp100F(machine_config &config);
	void st_sb100(machine_config &config);
	void st_sb100F(machine_config &config);
	void st_sb100c(machine_config &config);
	void st_sb100cF(machine_config &config);


protected:
	typedef uint8_t solenoid_feature_data[20][4];

	st_mp100_state(machine_config const &mconfig, device_type type, char const *tag, solenoid_feature_data const &solenoid_features)
		: genpin_class(mconfig, type, tag)
		, m_solenoid_features(solenoid_features)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_pia_u10(*this, "pia_u10")
		, m_pia_u11(*this, "pia_u11")
		, m_zero_crossing_freq_timer(*this, "timer_z_freq")
		, m_zero_crossing_active_timer(*this, "timer_z_pulse")
		, m_display_refresh_timer(*this, "timer_d_pulse")
		, m_discrete(*this, "discrete")
		, m_io_system(*this, "SYSTEM")
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
	DECLARE_WRITE8_MEMBER(sb100_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	TIMER_DEVICE_CALLBACK_MEMBER(timer_z_freq);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_z_pulse);
	TIMER_DEVICE_CALLBACK_MEMBER(u11_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_d_pulse);

	void st_mp100_map(address_map &map);
	void st_mp100_map_full(address_map &map);

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
	bool m_u11_cb1;
	bool m_u11_cb2;
	bool m_7d;       // 7-digit display yes/no
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
	optional_device<discrete_device> m_discrete;
	required_ioport m_io_system;
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


class pinball_state : public st_mp100_state
{
public:
	pinball_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp100_state(mconfig, type, tag, s_solenoid_features_pinball)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_pinball;
};

class stingray_state : public st_mp100_state
{
public:
	stingray_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp100_state(mconfig, type, tag, s_solenoid_features_stingray)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_stingray;
};

class stars_state : public st_mp100_state
{
public:
	stars_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp100_state(mconfig, type, tag, s_solenoid_features_stars)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_stars;
};

class memlane_state : public st_mp100_state
{
public:
	memlane_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp100_state(mconfig, type, tag, s_solenoid_features_memlane)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_memlane;
};

class lectrono_state : public st_mp100_state
{
public:
	lectrono_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp100_state(mconfig, type, tag, s_solenoid_features_lectrono)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_lectrono;
};

class wildfyre_state : public st_mp100_state
{
public:
	wildfyre_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp100_state(mconfig, type, tag, s_solenoid_features_wildfyre)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_wildfyre;
};

class nugent_state : public st_mp100_state
{
public:
	nugent_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp100_state(mconfig, type, tag, s_solenoid_features_nugent)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_nugent;
};

class dracula_state : public st_mp100_state
{
public:
	dracula_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp100_state(mconfig, type, tag, s_solenoid_features_dracula)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_dracula;
};

class trident_state : public st_mp100_state
{
public:
	trident_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp100_state(mconfig, type, tag, s_solenoid_features_trident)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_trident;
};

class hothand_state : public st_mp100_state
{
public:
	hothand_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp100_state(mconfig, type, tag, s_solenoid_features_hothand)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_hothand;
};

class princess_state : public st_mp100_state
{
public:
	princess_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp100_state(mconfig, type, tag, s_solenoid_features_princess)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_princess;
};

class magic_state : public st_mp100_state
{
public:
	magic_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp100_state(mconfig, type, tag, s_solenoid_features_magic)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_magic;
};



void st_mp100_state::st_mp100_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x1fff);     // A15, A14 and A13 are not connected
	map(0x0000, 0x007f).mirror(0x0100).ram();
	map(0x0088, 0x008b).mirror(0x0d74).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).mirror(0x0d6c).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x00a0, 0x00a7).w(FUNC(st_mp100_state::sb100_w));         // To SB-100 sound board, computer sounds
	map(0x00c0, 0x00c7).nopw();                                   // To SB-100 sound board, electronically simulated chimes
	map(0x0200, 0x02ff).mirror(0x0d00).ram().rw(FUNC(st_mp100_state::nibble_nvram_r), FUNC(st_mp100_state::nibble_nvram_w)).share("nvram");
	map(0x1000, 0x1fff).mirror(0x0000).rom();
}

void st_mp100_state::st_mp100_map_full(address_map &map)
{
	map(0x0000, 0x007f).ram();
	map(0x0088, 0x008b).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x00a0, 0x00a7).w(FUNC(st_mp100_state::sb100_w));         // To SB-100 sound board, computer sounds
	map(0x00c0, 0x00c7).nopw();                                   // To SB-100 sound board, electronically simulated chimes
	map(0x0200, 0x0fff).ram().rw(FUNC(st_mp100_state::nibble_nvram_r), FUNC(st_mp100_state::nibble_nvram_w)).share("nvram");
	map(0x1000, 0xffff).rom();
}



static INPUT_PORTS_START( mp100 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Self Test") PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp100_state, self_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Activity")  PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp100_state, activity_button, 0)

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
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 2 (Coins/Credits)")    PORT_DIPLOCATION("S01-S08:!1,!2,!3,!4,!5")  // Same as Coin Slot 3
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
	PORT_DIPNAME( 0x20, 0x20, "Score Level Award")              PORT_DIPLOCATION("S01-S08:!6")
	PORT_DIPSETTING(    0x00, "Extra Ball")
	PORT_DIPSETTING(    0x20, "Replay")
	PORT_DIPNAME( 0x40, 0x00, "Balls per Game")                 PORT_DIPLOCATION("S01-S08:!7")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x40, "5")
	PORT_DIPNAME( 0x80, 0x80, "Melody Playback")                PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, "Partial Melodies")
	PORT_DIPSETTING(    0x80, "Full Melodies")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 3 (Coins/Credits)")    PORT_DIPLOCATION("S09-S16:!1,!2,!3,!4,!5")  // Same as Coin Slot 2
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
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 14")                       PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "Beating Highest Score Awards")   PORT_DIPLOCATION("S09-S16:!7")
	PORT_DIPSETTING(    0x00, "Novelty")
	PORT_DIPSETTING(    0x40, "3 Credits")
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 16")                       PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x01, "Maximum Credits")            PORT_DIPLOCATION("S17-S24:!1,!2,!3")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "20")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x05, "30")
	PORT_DIPSETTING(    0x06, "35")
	PORT_DIPSETTING(    0x07, "40")
	PORT_DIPNAME( 0x08, 0x08, "Credits Displayed")          PORT_DIPLOCATION("S17-S24:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x10, "Match Feature")              PORT_DIPLOCATION("S17-S24:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 22")                   PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 23")                   PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 24")                   PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 25")                   PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 26")                   PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 27")                   PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 28")                   PORT_DIPLOCATION("S25-S32:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 29")                   PORT_DIPLOCATION("S25-S32:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 30")                   PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xc0, 0x80, "Specials Award")             PORT_DIPLOCATION("S25-S32:!7,!8")
	PORT_DIPSETTING(    0x00, "100000 points")
	PORT_DIPSETTING(    0x40, "Extra Ball")
	PORT_DIPSETTING(    0x80, "Credit")
	PORT_DIPSETTING(    0xc0, "Extra Ball and Credit")

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	// standard
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, outhole, (void *)0x0080)  // PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT2 ) PORT_NAME("Slam Tilt") PORT_CODE(KEYCODE_EQUALS)

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

static INPUT_PORTS_START( pinball )
	PORT_INCLUDE( mp100 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x80, 0x80, "Outlane, Pop Bumper and Bonus Rollover Lamps")  PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x80, "Alternate")
	PORT_DIPSETTING(    0x00, "Always On")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x80, 0x80, "Drop Target Specials Award")     PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Novelty")
	PORT_DIPSETTING(    0x80, "Credit")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Maximum Number Of Players")      PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x00, "2")
	PORT_DIPSETTING(    0x01, "4")
	PORT_DIPNAME( 0xc0, 0x80, "Outlane Specials Award")         PORT_DIPLOCATION("S25-S32:!7,!8")
	PORT_DIPSETTING(    0x00, "100000 points")
	PORT_DIPSETTING(    0x40, "Extra Ball")
	PORT_DIPSETTING(    0x80, "Credit")
	PORT_DIPSETTING(    0xc0, "Extra Ball and Credit")

	PORT_MODIFY("X1")   /* Saucer switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x1, (void *)0x0108)  // PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x1, (void *)0x0110)  // PORT_CODE(KEYCODE_QUOTE)

	PORT_MODIFY("X3")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0301)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0302)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)

	PORT_MODIFY("X3")   /* Spinner switch */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, spinner, (void *)0x1320)  // PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

static INPUT_PORTS_START( pinballt )
	PORT_INCLUDE( pinball )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x02, 0x02, "Extra Ball Award")                   PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
INPUT_PORTS_END


static INPUT_PORTS_START( stingray )
	PORT_INCLUDE( mp100 )

	PORT_MODIFY("SPINNER")                          // Allow user to select the simulated rotation speed of Spinners
	PORT_CONFNAME( 0x07, 0x01, "Spinner Simulation")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ))
	PORT_CONFSETTING(    0x01, "Slow Speed")
	PORT_CONFSETTING(    0x02, "Medium Speed")
	PORT_CONFSETTING(    0x03, "High Speed")
	PORT_CONFSETTING(    0x07, "Turbo Speed")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x80, 0x80, "Outlane, Pop Bumper and Bonus Rollover Lamps")  PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x80, "Alternate")
	PORT_DIPSETTING(    0x00, "Always On")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x80, 0x80, "Drop Target Specials Award")         PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Novelty")
	PORT_DIPSETTING(    0x80, "Credit")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Maximum Number Of Players")          PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x00, "2")
	PORT_DIPSETTING(    0x01, "4")
	PORT_DIPNAME( 0xc0, 0x80, "Outlane Specials Award")             PORT_DIPLOCATION("S25-S32:!7,!8")
	PORT_DIPSETTING(    0x00, "100000 points")
	PORT_DIPSETTING(    0x40, "Extra Ball")
	PORT_DIPSETTING(    0x80, "Credit")
	PORT_DIPSETTING(    0xc0, "Extra Ball and Credit")

	PORT_MODIFY("X1")   /* Saucer switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x1, (void *)0x0108)  // PORT_CODE(KEYCODE_ENTER)

	PORT_MODIFY("X3")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0301)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0302)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)
	/* Spinner switch - unfortunately it's shared with other playfield switches so we default it above to slow spin simulation */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, spinner, (void *)0x1320)  // PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

static INPUT_PORTS_START( stingrayt )
	PORT_INCLUDE( stingray )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xc0, 0xc0, "Drop Target Specials Award")         PORT_DIPLOCATION("S17-S24:!7,!8")
	PORT_DIPSETTING(    0x00, "Novelty: 50,000 Points")
	PORT_DIPSETTING(    0x40, "Novelty: 100,000 Points")
	PORT_DIPSETTING(    0x80, "Extra Ball")
	PORT_DIPSETTING(    0xc0, "Credit")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x02, 0x00, "Saucer Position Reset on New Ball")  PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x00, "Saucer Award Potential")             PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x04, "Stepped by Saucer Entry")
	PORT_DIPSETTING(    0x00, "Stepped by 10 Point Scores")
	PORT_DIPNAME( 0x18, 0x18, "Extra Ball Award")                   PORT_DIPLOCATION("S25-S32:!4,!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, "Novelty: 50,000 Points")
	PORT_DIPSETTING(    0x10, "Extra Ball")
	PORT_DIPSETTING(    0x18, "Extra Ball, then 25,000 Points")
	PORT_DIPNAME( 0x20, 0x00, "Outlane Specials")                   PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x20, "1 Per Drop Target Bank Reset")
	PORT_DIPSETTING(    0x00, "Open Ended")
	PORT_DIPNAME( 0xc0, 0xc0, "Outlane Specials Award")             PORT_DIPLOCATION("S25-S32:!7,!8")
	PORT_DIPSETTING(    0x00, "Novelty: 50000 points")
	PORT_DIPSETTING(    0x40, "Novelty: 100000 points")
	PORT_DIPSETTING(    0x80, "Extra Ball")
	PORT_DIPSETTING(    0xc0, "Credit")
INPUT_PORTS_END


static INPUT_PORTS_START( stingrayf )
	PORT_INCLUDE( stingray )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Maximum Credits")                        PORT_DIPLOCATION("S17-S24:!1,!2")	/// DSW:17,18
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "20")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
	PORT_DIPNAME( 0x04, 0x00, "Player Displays")                        PORT_DIPLOCATION("S17-S24:!3")		/// DSW:19
	PORT_DIPSETTING(    0x00, "6 Digits")
	PORT_DIPSETTING(    0x04, "7 Digits")
	PORT_DIPNAME( 0x20, 0x00, "Standup Target Uses")                    PORT_DIPLOCATION("S17-S24:!6")		/// DSW:22
	PORT_DIPSETTING(    0x00, "Drop Target Specials Award")
	PORT_DIPSETTING(    0x20, "Outlane Specials Award")
	PORT_DIPNAME( 0xc0, 0xc0, "Drop Target Specials Award")             PORT_DIPLOCATION("S17-S24:!7,!8")
	PORT_DIPSETTING(    0x00, "Novelty: 50,000 Points")
	PORT_DIPSETTING(    0x40, "Novelty: 100,000 Points")
	PORT_DIPSETTING(    0x80, "Extra Ball")
	PORT_DIPSETTING(    0xc0, "Credit")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x02, 0x00, "Saucer Position Reset on New Ball")  PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x00, "Saucer Award Potential")             PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x04, "Stepped by Saucer Entry")
	PORT_DIPSETTING(    0x00, "Stepped by 10 Point Scores")
	PORT_DIPNAME( 0x18, 0x18, "Extra Ball Award")                   PORT_DIPLOCATION("S25-S32:!4,!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, "Novelty: 50,000 Points")
	PORT_DIPSETTING(    0x10, "Extra Ball")
	PORT_DIPSETTING(    0x18, "Extra Ball, then 25,000 Points")
	PORT_DIPNAME( 0x20, 0x20, "Standup Target Specials")                   PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, "1 Per Ball")
	PORT_DIPSETTING(    0x20, "1 Per Making the Two Standup Targets")
	PORT_DIPNAME( 0xc0, 0xc0, "Outlane Specials Award")             PORT_DIPLOCATION("S25-S32:!7,!8")
	PORT_DIPSETTING(    0x00, "Novelty: 50000 points")
	PORT_DIPSETTING(    0x40, "Novelty: 100000 points")
	PORT_DIPSETTING(    0x80, "Extra Ball")
	PORT_DIPSETTING(    0xc0, "Credit")
INPUT_PORTS_END


static INPUT_PORTS_START( stars )
	PORT_INCLUDE( mp100 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "Bonus Countdown")                        PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, "Multiple Steps")
	PORT_DIPSETTING(    0x20, "1000 Steps")
	PORT_DIPNAME( 0x80, 0x80, "Drop Target Tripple Bonus Award")        PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x80, "Both Sets of 3 Drop Targets")
	PORT_DIPSETTING(    0x00, "Either Set of 3 Drop Targets")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Maximum Number Of Players")              PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x00, "2")
	PORT_DIPSETTING(    0x01, "4")
	PORT_DIPNAME( 0x02, 0x02, "WOW! Award")                             PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x02, "Extra Ball")
	PORT_DIPNAME( 0x20, 0x20, "Star Specials")                          PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x20, "1 Per Ball")
	PORT_DIPSETTING(    0x00, "Open Ended")

	PORT_MODIFY("X0")   /* Spinner switch */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, spinner, (void *)0x1001)  // PORT_CODE(KEYCODE_BACKSLASH)

	PORT_MODIFY("X2")   /* Spinner switch */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, spinner, (void *)0x2201)  // PORT_CODE(KEYCODE_COMMA)

	PORT_MODIFY("X3")   /* Drop Target switches */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0340)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END

static INPUT_PORTS_START( memlane )
	PORT_INCLUDE( mp100 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x20, "3X Lamp On Top Saucer")              PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x20, "Alternates On 10 Points")
	PORT_DIPSETTING(    0x00, "No Alternating")
	PORT_DIPNAME( 0xc0, 0xc0, "Beating Highest Score Awards")       PORT_DIPLOCATION("S09-S16:!7,!8")
	PORT_DIPSETTING(    0x00, "Novelty")
	PORT_DIPSETTING(    0x40, "1 Credit")
	PORT_DIPSETTING(    0x80, "2 Credits")
	PORT_DIPSETTING(    0xc0, "3 Credits")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x60, 0x20, "Number Of Strikes To Light Specials")    PORT_DIPLOCATION("S17-S24:!6,!7")
	PORT_DIPSETTING(    0x60, "6 Strikes")
	PORT_DIPSETTING(    0x40, "5 Strikes")
	PORT_DIPSETTING(    0x20, "4 Strikes")
	PORT_DIPSETTING(    0x00, "3 Strikes")
	PORT_DIPNAME( 0x80, 0x80, "Drop Target Strike Lamp")            PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x80, "Every Multiple Of 40")
	PORT_DIPSETTING(    0x00, "Every Multiple Of 20")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Extra Ball Outlanes")                PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x01, "Alternate")
	PORT_DIPSETTING(    0x00, "Both")
	PORT_DIPNAME( 0x02, 0x02, "Extra Ball Award")                   PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x04, "Strike Specials")                    PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x04, "1 Per Strike")
	PORT_DIPSETTING(    0x00, "Open Ended")
	PORT_DIPNAME( 0x38, 0x00, "Coin Slot 1")                        PORT_DIPLOCATION("S25-S32:!4,!5,!6")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_5C ))

	PORT_MODIFY("X0")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)

	PORT_MODIFY("X1")   /* Spinner switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, spinner, (void *)0x1120)  // PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, spinner, (void *)0x2140)  // PORT_CODE(KEYCODE_L)

	PORT_MODIFY("X4")   /* Saucer switches */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x4, (void *)0x0440)  // PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x4, (void *)0x0480)  // PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END

static INPUT_PORTS_START( lectrono )
	PORT_INCLUDE( mp100 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Top Saucer And Left Upper Lane Lamps")   PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, "Off After Entering Saucer")
	PORT_DIPSETTING(    0x20, "On For Entire Ball")
	PORT_DIPNAME( 0xc0, 0xc0, "Beating Highest Score Awards")       PORT_DIPLOCATION("S09-S16:!7,!8")
	PORT_DIPSETTING(    0x00, "Novelty")
	PORT_DIPSETTING(    0x40, "1 Credit")
	PORT_DIPSETTING(    0x80, "2 Credits")
	PORT_DIPSETTING(    0xc0, "3 Credits")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "5 Bank Drop Target Specials")        PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x20, "1 Per Ball")
	PORT_DIPSETTING(    0x00, "Open Ended")
	PORT_DIPNAME( 0x40, 0x40, "Electronic Sounds")                  PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "Electronic Chimes")
	PORT_DIPSETTING(    0x40, "Computer Type Sounds")
	PORT_DIPNAME( 0x80, 0x80, "Bonus Countdown")                    PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Multiple Steps")
	PORT_DIPSETTING(    0x80, "1000 Steps")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x02, 0x02, "Extra Ball Award")                   PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x38, 0x00, "Coin Slot 1")                        PORT_DIPLOCATION("S25-S32:!4,!5,!6")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_5C ))

	PORT_MODIFY("X0")   /* Drop Target switch */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X1")   /* Drop Target and Spinner switches */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x1, (void *)0x0110)  // PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x1, (void *)0x0120)  // PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, spinner, (void *)0x1140)  // PORT_CODE(KEYCODE_L)

	PORT_MODIFY("X2")   /* Drop Target and Kickback switches */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")   /* Drop Target switches */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)

	PORT_MODIFY("X4")   /* Drop Target and Saucer switches */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x4, (void *)0x0410)  // PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x4, (void *)0x0480)  // PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END

static INPUT_PORTS_START( nugent )
	PORT_INCLUDE( lectrono )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Flipper Return Lane Lamps")          PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, "Off After Being Made")
	PORT_DIPSETTING(    0x20, "On For Entire Ball")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "Left Bank Drop Target Specials")     PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x20, "1 Per Ball")
	PORT_DIPSETTING(    0x00, "Open Ended")

	PORT_MODIFY("X0")   /* Drop Target switch */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_nugent_dt, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X1")   /* Spinner */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_nugent_dt, (void *)0x0110)  // PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, spinner, (void *)0x1140)  // PORT_CODE(KEYCODE_L)

	PORT_MODIFY("X2")   /* Drop Target switch */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)

	PORT_MODIFY("X3")   /* Drop Target switch */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_nugent_dt, (void *)0x0310)  // PORT_CODE(KEYCODE_F)

	PORT_MODIFY("X4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_nugent_dt, (void *)0x0410)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END

static INPUT_PORTS_START( dracula )
	PORT_INCLUDE( mp100 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x20, "Extra Ball Award")                   PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xc0, 0xc0, "Beating Highest Score Awards")       PORT_DIPLOCATION("S09-S16:!7,!8")
	PORT_DIPSETTING(    0x00, "Novelty")
	PORT_DIPSETTING(    0x40, "1 Credit")
	PORT_DIPSETTING(    0x80, "2 Credits")
	PORT_DIPSETTING(    0xc0, "3 Credits")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x20, "Y Target Specials")                  PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x20, "1 Per Ball")
	PORT_DIPSETTING(    0x00, "Open Ended")
	PORT_DIPNAME( 0x40, 0x40, "Electronic Sounds")                  PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "Electronic Chimes")
	PORT_DIPSETTING(    0x40, "Computer Type Sounds")
	PORT_DIPNAME( 0x80, 0x00, "Left Bottom Lane Specials")          PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x80, "1 Per Ball")
	PORT_DIPSETTING(    0x00, "Open Ended")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Extra Ball Lane Lights")             PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x01, "After Spotting 4 Cats and 4 Bats")
	PORT_DIPSETTING(    0x00, "After Spotting 4 Cats")
	PORT_DIPNAME( 0x02, 0x02, "Bottom Lane Advance Bonus")          PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x00, "2 Advances (4000)")
	PORT_DIPSETTING(    0x02, "3 Advances (6000)")
	PORT_DIPNAME( 0x04, 0x04, "Extra Ball Lane")                    PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x04, "Alternates On and Off")
	PORT_DIPSETTING(    0x00, "Stays On")
	PORT_DIPNAME( 0x38, 0x00, "Coin Slot 1")                        PORT_DIPLOCATION("S25-S32:!4,!5,!6")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_5C ))

	PORT_MODIFY("X1")   /* Drop Target and Spinner switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x1, (void *)0x0108)  // PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, spinner, (void *)0x1140)  // PORT_CODE(KEYCODE_L)

	PORT_MODIFY("X2")   /* Drop Target switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x2, (void *)0x0208)  // PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)

	PORT_MODIFY("X3")   /* Drop Target switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)

	PORT_MODIFY("X4")   /* Drop Target and Saucer switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x4, (void *)0x0408)  // PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x4, (void *)0x0410)  // PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x4, (void *)0x0480)  // PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END

static INPUT_PORTS_START( draculat )
	PORT_INCLUDE( dracula )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Maximum Credits")                        PORT_DIPLOCATION("S17-S24:!1,!2")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
	PORT_DIPNAME( 0x04, 0x00, "Freeplay Mode")                          PORT_DIPLOCATION("S17-S24:!3")
	PORT_DIPSETTING(    0x00, "Reset Credits to 01 on Power-Up, Subtract Credits with Start Button")
	PORT_DIPSETTING(    0x04, "Remember Credits on Power-Up, Reset Credits to 01 with Start Button")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x18, 0x00, "Coin Slot 1")                            PORT_DIPLOCATION("S25-S32:!4,!5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_3C ))
	PORT_DIPNAME( 0x20, 0x00, "Tilt Cancels Extra Ball")                PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, "Yes")
	PORT_DIPSETTING(    0x20, "No")
INPUT_PORTS_END

static INPUT_PORTS_START( draculac )
	PORT_INCLUDE( draculat )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x01, "Maximum Credits")                        PORT_DIPLOCATION("S17-S24:!1,!2,!3")	/// DSW:17,18,19
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "20")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x05, "30")
	PORT_DIPSETTING(    0x06, "99 (Freeplay), Subtract Credits with Start Button")
	PORT_DIPSETTING(    0x07, "99 (Freeplay), Reset Credits to none with Start Button")
INPUT_PORTS_END


static INPUT_PORTS_START( wildfyre )
	PORT_INCLUDE( dracula )

	PORT_MODIFY("X2")   /* Saucer switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x20, "Top Saucer Specials")                PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x20, "1 Per Ball")
	PORT_DIPSETTING(    0x00, "Open Ended")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Extra Ball Lane Lights")             PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x01, "After Spotting Wild Fyre")
	PORT_DIPSETTING(    0x00, "After Spotting Wild")
INPUT_PORTS_END

static INPUT_PORTS_START( wildfyret )
	PORT_INCLUDE( dracula )

	PORT_MODIFY("X2")   /* Saucer switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Maximum Credits")                        PORT_DIPLOCATION("S17-S24:!1,!2")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
	PORT_DIPNAME( 0x04, 0x00, "Freeplay Mode")                          PORT_DIPLOCATION("S17-S24:!3")
	PORT_DIPSETTING(    0x00, "Reset Credits to 01 on Power-Up, Subtract Credits with Start Button")
	PORT_DIPSETTING(    0x04, "Remember Credits on Power-Up, Reset Credits to 01 with Start Button")
	PORT_DIPNAME( 0x20, 0x20, "Top Saucer Specials")                    PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x20, "1 Per Ball")
	PORT_DIPSETTING(    0x00, "Open Ended")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Extra Ball Lane Lights")                 PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x01, "After Spotting Wild Fyre")
	PORT_DIPSETTING(    0x00, "After Spotting Wild")
	PORT_DIPNAME( 0x18, 0x00, "Coin Slot 1")                            PORT_DIPLOCATION("S25-S32:!4,!5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_3C ))
	PORT_DIPNAME( 0x20, 0x00, "Tilt Cancels Extra Ball")                PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, "Yes")
	PORT_DIPSETTING(    0x20, "No")
INPUT_PORTS_END

static INPUT_PORTS_START( wildfyrec )
	PORT_INCLUDE( wildfyret )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x01, "Maximum Credits")                        PORT_DIPLOCATION("S17-S24:!1,!2,!3")	/// DSW:17,18,19
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "20")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x05, "30")
	PORT_DIPSETTING(    0x06, "99 (Freeplay), Subtract Credits with Start Button")
	PORT_DIPSETTING(    0x07, "99 (Freeplay), Reset Credits to none with Start Button")
INPUT_PORTS_END


static INPUT_PORTS_START( st_game )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM )   PORT_NAME("SW48 Self Test") PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp100_state, self_test, 0) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Activity")       PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp100_state, activity_button, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("SW41 Reset")     PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp100_state, reset_sam, 0)


	PORT_START("ZX")                                // The Zero Crossing timer frequency on the MPU board is derived from mains power frequency * 2
	PORT_CONFNAME( 0x01, 0x01, "Mains AC Power")    // It affects audio playback rate and other game timers, so allow users to configure it
	PORT_CONFSETTING(    0x00, "50Hz Frequency")
	PORT_CONFSETTING(    0x01, "60Hz Frequency")

	PORT_START("SPINNER")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("OUTHOLE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )


	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 01")            PORT_DIPLOCATION("S01-S08:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 02")            PORT_DIPLOCATION("S01-S08:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 03")            PORT_DIPLOCATION("S01-S08:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 04")            PORT_DIPLOCATION("S01-S08:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 05")            PORT_DIPLOCATION("S01-S08:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 06")            PORT_DIPLOCATION("S01-S08:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 07")            PORT_DIPLOCATION("S01-S08:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 08")            PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 09")            PORT_DIPLOCATION("S09-S16:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 10")            PORT_DIPLOCATION("S09-S16:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 11")            PORT_DIPLOCATION("S09-S16:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 12")            PORT_DIPLOCATION("S09-S16:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 13")            PORT_DIPLOCATION("S09-S16:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 14")            PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 15")            PORT_DIPLOCATION("S09-S16:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 16")            PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 17")            PORT_DIPLOCATION("S17-S24:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 18")            PORT_DIPLOCATION("S17-S24:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 19")            PORT_DIPLOCATION("S17-S24:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 20")            PORT_DIPLOCATION("S17-S24:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 21")            PORT_DIPLOCATION("S17-S24:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 22")            PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 23")            PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 24")            PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 25")            PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 26")            PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 27")            PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 28")            PORT_DIPLOCATION("S25-S32:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 29")            PORT_DIPLOCATION("S25-S32:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 30")            PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 31")            PORT_DIPLOCATION("S25-S32:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 32")            PORT_DIPLOCATION("S25-S32:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))


	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_8)

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_X)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
INPUT_PORTS_END



static INPUT_PORTS_START( trident )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Self Test") PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp100_state, self_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Activity")  PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp100_state, activity_button, 0)

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
	PORT_CONFSETTING(    0x04, "Turbo Speed")

	PORT_START("OUTHOLE")                           // Start machine with or without a ball in the Outhole
	PORT_CONFNAME( 0x01, 0x01, "Outhole State on Power-Up")
	PORT_CONFSETTING(    0x00, "Ball Not in Outhole")
	PORT_CONFSETTING(    0x01, "Ball in Outhole")

	PORT_START("DSW0")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 2 (Coins/Credits)")    PORT_DIPLOCATION("S01-S08:!1,!2,!3,!4,!5")  // Same as Coin Slot 3
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
	PORT_DIPNAME( 0x20, 0x20, "Score Level Award")              PORT_DIPLOCATION("S01-S08:!6")
	PORT_DIPSETTING(    0x00, "Extra Ball")
	PORT_DIPSETTING(    0x20, "Replay")
	PORT_DIPNAME( 0x40, 0x00, "Balls per Game")                 PORT_DIPLOCATION("S01-S08:!7")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x40, "5")
	PORT_DIPNAME( 0x80, 0x80, "Melody Option")                  PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, "Some Tunes Are 2 Tones")
	PORT_DIPSETTING(    0x80, "All Tunes Are Full Melodies")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 3 (Coins/Credits)")    PORT_DIPLOCATION("S09-S16:!1,!2,!3,!4,!5")  // Same as Coin Slot 2
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
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 14")                           PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xc0, 0xc0, "Beating Highest Score Awards")       PORT_DIPLOCATION("S09-S16:!7,!8")
	PORT_DIPSETTING(    0x00, "Novelty")
	PORT_DIPSETTING(    0x40, "1 Credit")
	PORT_DIPSETTING(    0x80, "2 Credits")
	PORT_DIPSETTING(    0xc0, "3 Credits")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x01, "Maximum Credits")            PORT_DIPLOCATION("S17-S24:!1,!2,!3")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "20")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x05, "30")
	PORT_DIPSETTING(    0x06, "35")
	PORT_DIPSETTING(    0x07, "40")
	PORT_DIPNAME( 0x08, 0x08, "Credits Displayed")          PORT_DIPLOCATION("S17-S24:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x10, "Match Feature")              PORT_DIPLOCATION("S17-S24:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 22")                   PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "Extra Ball Lane Award")      PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Drop Target & Outlane Specials")           PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x80, "Lights After 3rd Drop Target Reset")
	PORT_DIPSETTING(    0x00, "Lights After 2nd Drop Target Reset")

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Drop Target Specials")               PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x01, "1 Per Ball")
	PORT_DIPSETTING(    0x00, "Open Ended")
	PORT_DIPNAME( 0x02, 0x02, "Outlane Specials")                   PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x02, "1 Per Ball")
	PORT_DIPSETTING(    0x00, "Open Ended")
	PORT_DIPNAME( 0x04, 0x04, "Extra Ball and Outlane Specials")    PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x04, "Alternate When Lit")
	PORT_DIPSETTING(    0x00, "Don't Alternate When Lit")
	PORT_DIPNAME( 0x38, 0x00, "Coin Slot 1")                        PORT_DIPLOCATION("S25-S32:!4,!5,!6")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_5C ))
	PORT_DIPNAME( 0xc0, 0x80, "Award for Special")                  PORT_DIPLOCATION("S25-S32:!7,!8")
	PORT_DIPSETTING(    0x00, "100000 points")
	PORT_DIPSETTING(    0x40, "Extra Ball")
	PORT_DIPSETTING(    0x80, "Free Game")
	PORT_DIPSETTING(    0xc0, "Extra Ball and Free Game")

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, spinner, (void *)0x1008)  // PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, spinner, (void *)0x2010)  // PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT2 ) PORT_NAME("Slam Tilt") PORT_CODE(KEYCODE_EQUALS)

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)

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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0302)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0340)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, outhole, (void *)0x0401)  // PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END

static INPUT_PORTS_START( tridentf )
	PORT_INCLUDE( trident )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Extend Drop Target Specials - See DSW25")            PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Maximum Credits")                        PORT_DIPLOCATION("S17-S24:!1,!2")	/// DSW:17,18
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "20")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
	PORT_DIPNAME( 0x04, 0x00, "Player Displays")                        PORT_DIPLOCATION("S17-S24:!3")		/// DSW:19
	PORT_DIPSETTING(    0x00, "6 Digits")
	PORT_DIPSETTING(    0x04, "7 Digits")
	PORT_DIPNAME( 0xa0, 0x20, "Drop Target & Outlane Specials")         PORT_DIPLOCATION("S17-S24:!6,!8")	/// DSW:22,24  (22 previously unused)
	PORT_DIPSETTING(    0xa0, "Lights After 4th Drop Target Reset")
	PORT_DIPSETTING(    0x80, "Lights After 3rd Drop Target Reset")
	PORT_DIPSETTING(    0x20, "Lights After 2nd Drop Target Reset")
	PORT_DIPSETTING(    0x00, "Lights After 1st Drop Target Reset")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Drop Target Specials")               PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x01, "1 Per Ball")                         PORT_CONDITION("DSW1", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "Open Ended")                         PORT_CONDITION("DSW1", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, "1 Per Ball")                         PORT_CONDITION("DSW1", 0x20, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "Every Second Bank Reset")            PORT_CONDITION("DSW1", 0x20, NOTEQUALS, 0x00)
	PORT_DIPNAME( 0x18, 0x00, "Coin Slot 1")                        PORT_DIPLOCATION("S25-S32:!4,!5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x18, "2 Coins/3 Credits (C1/1C, C2/2C)")
	PORT_DIPNAME( 0x20, 0x00, "Electronic Sounds")                  PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, "Computer Type Sounds")
	PORT_DIPSETTING(    0x20, "Electronic Chimes")
INPUT_PORTS_END


static INPUT_PORTS_START( magic )
	PORT_INCLUDE( trident )

	PORT_MODIFY("X3")   /* These Drop Targets have two switches on them */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0318)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x0318)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x03c0)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x3, (void *)0x03c0)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END

static INPUT_PORTS_START( magict )
	PORT_INCLUDE( magic )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x0f, 0x02, "Coin Slot 2 (Coins/Credits)")    PORT_DIPLOCATION("S01-S08:!1,!2,!3,!4")  // Same as Coin Slot 3
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
	PORT_DIPNAME( 0x10, 0x00, "Tilt Cancels Extra Ball")                PORT_DIPLOCATION("S01-S08:!5")	/// DSW:05  (05 previously used for coins/credit)
	PORT_DIPSETTING(    0x10, "Yes")
	PORT_DIPSETTING(    0x00, "No")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Flipper Return Lane Bonus")              PORT_DIPLOCATION("S09-S16:!6")	/// DSW:14  (14 previously unused)
	PORT_DIPSETTING(    0x20, "Reset Value to 2000 on Entry")
	PORT_DIPSETTING(    0x00, "Value Remains Lit")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Maximum Credits")                        PORT_DIPLOCATION("S17-S24:!1,!2")	/// DSW:17,18
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
	PORT_DIPNAME( 0x04, 0x00, "Divide by 10 Scoring")                   PORT_DIPLOCATION("S17-S24:!3")	/// DSW:19
	PORT_DIPSETTING(    0x00, "Disabled")
	PORT_DIPSETTING(    0x04, "Enabled")
	PORT_DIPNAME( 0xa0, 0x20, "Drop Target & Inlane Specials")          PORT_DIPLOCATION("S17-S24:!6,!8")	/// DSW:22,24  (22 previously unused)
	PORT_DIPSETTING(    0xa0, "Lights After 4th Drop Target Reset")
	PORT_DIPSETTING(    0x80, "Lights After 3rd Drop Target Reset")
	PORT_DIPSETTING(    0x20, "Lights After 2nd Drop Target Reset")
	PORT_DIPSETTING(    0x00, "Lights After 1st Drop Target Reset")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x02, 0x00, "M-A-G-I-C Lane/Target Special Lites")    PORT_DIPLOCATION("S25-S32:!2")	/// DSW:26
	PORT_DIPSETTING(    0x02, "After Completing M-A-G-I-C Twice")
	PORT_DIPSETTING(    0x00, "After Completing M-A-G-I-C Once")
	PORT_DIPNAME( 0x18, 0x00, "Coin Slot 1")                            PORT_DIPLOCATION("S25-S32:!4,!5")	/// DSW:28,29
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_3C ))
	PORT_DIPNAME( 0x20, 0x00, "M-A-G-I-C Lane/Target Special Award")    PORT_DIPLOCATION("S25-S32:!6")	/// DSW:30
	PORT_DIPSETTING(    0x20, "1 Per Ball")
	PORT_DIPSETTING(    0x00, "Open Ended")
INPUT_PORTS_END

static INPUT_PORTS_START( magicf )
	PORT_INCLUDE( magic )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Extend Drop Target Specials - See DSW25")    PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Maximum Credits")                        PORT_DIPLOCATION("S17-S24:!1,!2")	/// DSW:17,18
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "20")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
	PORT_DIPNAME( 0x04, 0x00, "Player Displays")                        PORT_DIPLOCATION("S17-S24:!3")		/// DSW:19
	PORT_DIPSETTING(    0x00, "6 Digits")
	PORT_DIPSETTING(    0x04, "7 Digits")
	PORT_DIPNAME( 0xa0, 0x20, "Drop Target & Outlane Specials")         PORT_DIPLOCATION("S17-S24:!6,!8")	/// DSW:22,24  (22 previously unused)
	PORT_DIPSETTING(    0xa0, "Lights After 4th Drop Target Reset")
	PORT_DIPSETTING(    0x80, "Lights After 3rd Drop Target Reset")
	PORT_DIPSETTING(    0x20, "Lights After 2nd Drop Target Reset")
	PORT_DIPSETTING(    0x00, "Lights After 1st Drop Target Reset")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Drop Target Specials")               PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x01, "1 Per Ball")                         PORT_CONDITION("DSW1", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "Open Ended")                         PORT_CONDITION("DSW1", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, "1 Per Ball")                         PORT_CONDITION("DSW1", 0x20, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "Every Second Bank Reset")            PORT_CONDITION("DSW1", 0x20, NOTEQUALS, 0x00)
	PORT_DIPNAME( 0x18, 0x00, "Coin Slot 1")                        PORT_DIPLOCATION("S25-S32:!4,!5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x18, "2 Coins/3 Credits (C1/1C, C2/2C)")
	PORT_DIPNAME( 0x20, 0x00, "Electronic Sounds")                  PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, "Computer Type Sounds")
	PORT_DIPSETTING(    0x20, "Electronic Chimes")
INPUT_PORTS_END


static INPUT_PORTS_START( princess )
	PORT_INCLUDE( trident )

	PORT_MODIFY("ZX")                               // The Zero Crossing timer frequency on the MPU board is derived from mains power frequency * 2
	PORT_CONFNAME( 0x01, 0x00, "Mains AC Power")    // Cosmic Princess only shipped in Australia where the mains AC frequency is 50Hz
	PORT_CONFSETTING(    0x00, "50Hz Frequency")
	PORT_CONFSETTING(    0x01, "60Hz Frequency")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x80, 0x00, "Drop Target & Inlane Specials")      PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x80, "Lights After 3rd Drop Target Reset")
	PORT_DIPSETTING(    0x00, "Lights After 2nd Drop Target Reset")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x02, 0x02, "Inlane Specials")                    PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x02, "1 Per Ball")
	PORT_DIPSETTING(    0x00, "Open Ended")
	PORT_DIPNAME( 0x04, 0x04, "Extra Ball and Special Inlanes")     PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x04, "Alternate When Lit")
	PORT_DIPSETTING(    0x00, "Don't Alternate When Lit")
INPUT_PORTS_END

static INPUT_PORTS_START( princesst )
	PORT_INCLUDE( princess )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x0f, 0x02, "Coin Slot 2 (Coins/Credits)")    PORT_DIPLOCATION("S01-S08:!1,!2,!3,!4")  // Same as Coin Slot 3
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
	PORT_DIPNAME( 0x10, 0x00, "Tilt Cancels Extra Ball")                PORT_DIPLOCATION("S01-S08:!5")	/// DSW:05  (05 previously used for coins/credit)
	PORT_DIPSETTING(    0x10, "Yes")
	PORT_DIPSETTING(    0x00, "No")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Flipper Return Lane Bonus")              PORT_DIPLOCATION("S09-S16:!6")	/// DSW:14  (14 previously unused)
	PORT_DIPSETTING(    0x20, "Reset Value to 2000 on Entry")
	PORT_DIPSETTING(    0x00, "Value Remains Lit")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Maximum Credits")                        PORT_DIPLOCATION("S17-S24:!1,!2")	/// DSW:17,18
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
	PORT_DIPNAME( 0x04, 0x00, "Divide by 10 Scoring")                   PORT_DIPLOCATION("S17-S24:!3")	/// DSW:19
	PORT_DIPSETTING(    0x00, "Disabled")
	PORT_DIPSETTING(    0x04, "Enabled")
	PORT_DIPNAME( 0xa0, 0x20, "Drop Target & Inlane Specials")          PORT_DIPLOCATION("S17-S24:!6,!8")	/// DSW:22,24  (22 previously unused)
	PORT_DIPSETTING(    0xa0, "Lights After 4th Drop Target Reset")
	PORT_DIPSETTING(    0x80, "Lights After 3rd Drop Target Reset")
	PORT_DIPSETTING(    0x20, "Lights After 2nd Drop Target Reset")
	PORT_DIPSETTING(    0x00, "Lights After 1st Drop Target Reset")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x02, 0x02, "Inlane Specials")                    PORT_DIPLOCATION("S25-S32:!2")	//  DSW:26
	PORT_DIPSETTING(    0x02, "1 Per Ball")
	PORT_DIPSETTING(    0x00, "Open Ended")
	PORT_DIPNAME( 0x08, 0x00, "Coin Slot 1")                        PORT_DIPLOCATION("S25-S32:!4")	/// DSW:28
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPNAME( 0x10, 0x00, "Colored 1-5 Lanes Special Lites")    PORT_DIPLOCATION("S25-S32:!5")	/// DSW:29
	PORT_DIPSETTING(    0x10, "After Completing Lanes Twice")
	PORT_DIPSETTING(    0x00, "After Completing Lanes Once")
	PORT_DIPNAME( 0x20, 0x00, "Colored 1-5 Lanes Special Award")    PORT_DIPLOCATION("S25-S32:!6")	/// DSW:30
	PORT_DIPSETTING(    0x20, "1 Per Ball")
	PORT_DIPSETTING(    0x00, "Open Ended")
INPUT_PORTS_END

static INPUT_PORTS_START( princessf )
	PORT_INCLUDE( princess )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Extend Drop Target Specials - See DSW25")    PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Maximum Credits")                        PORT_DIPLOCATION("S17-S24:!1,!2")	/// DSW:17,18
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "20")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
	PORT_DIPNAME( 0x04, 0x00, "Player Displays")                        PORT_DIPLOCATION("S17-S24:!3")		/// DSW:19
	PORT_DIPSETTING(    0x00, "6 Digits")
	PORT_DIPSETTING(    0x04, "7 Digits")
	PORT_DIPNAME( 0xa0, 0x20, "Drop Target & Inlane Specials")          PORT_DIPLOCATION("S17-S24:!6,!8")	/// DSW:22,24  (22 previously unused)
	PORT_DIPSETTING(    0xa0, "Lights After 4th Drop Target Reset")
	PORT_DIPSETTING(    0x80, "Lights After 3rd Drop Target Reset")
	PORT_DIPSETTING(    0x20, "Lights After 2nd Drop Target Reset")
	PORT_DIPSETTING(    0x00, "Lights After 1st Drop Target Reset")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Drop Target Specials")               PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x01, "1 Per Ball")                         PORT_CONDITION("DSW1", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "Open Ended")                         PORT_CONDITION("DSW1", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, "1 Per Ball")                         PORT_CONDITION("DSW1", 0x20, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "Every Second Bank Reset")            PORT_CONDITION("DSW1", 0x20, NOTEQUALS, 0x00)
	PORT_DIPNAME( 0x18, 0x00, "Coin Slot 1")                        PORT_DIPLOCATION("S25-S32:!4,!5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x18, "2 Coins/3 Credits (C1/1C, C2/2C)")
	PORT_DIPNAME( 0x20, 0x00, "Electronic Sounds")                  PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, "Computer Type Sounds")
	PORT_DIPSETTING(    0x20, "Electronic Chimes")
INPUT_PORTS_END


static INPUT_PORTS_START( hothand )
	PORT_INCLUDE( trident )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x20, "Extra Ball Award")                   PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "Extra Ball Lights On")               PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x40, "Spade Flush and Spade Suit Lit")
	PORT_DIPSETTING(    0x00, "Spade Flush")
	PORT_DIPNAME( 0x80, 0x00, "Outlane Specials")                   PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x80, "Alternate")
	PORT_DIPSETTING(    0x00, "Both")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Specials Adjustment")                PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x01, "3 Royal Flushes")
	PORT_DIPSETTING(    0x00, "2 Royal Flushes")
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 22")                           PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "Outlane Specials")                   PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x04, "1 Per Game")
	PORT_DIPSETTING(    0x00, "Open Ended")

	PORT_MODIFY("X0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X1")   /* Spinner */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, spinner, (void *)0x1104)  // PORT_CODE(KEYCODE_COLON)

	PORT_MODIFY("X3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)

	PORT_MODIFY("X4")   /* Kickback switch */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp100_state, switch_hold_x4, (void *)0x0402)  // PORT_CODE(KEYCODE_I)
INPUT_PORTS_END


CUSTOM_INPUT_MEMBER( st_mp100_state::spinner)
{
	uint16_t data = (uintptr_t)param & 0xffff;
	uint8_t bit_mask = data & 0xff;
	uint8_t port = (data >> 8) & 0x07;
	uint8_t spinner_num = ((data >> 12) - 1) & 0x03;
	bool state = false;


	switch (data & 0x7ff)
	{
		case 0x001:  if (machine().input().code_pressed(KEYCODE_BACKSLASH)) state = true; break;  // X0:I0  Stars:L
		case 0x008:  if (machine().input().code_pressed(KEYCODE_SLASH))     state = true; break;  // X0:I3  Trident:R, Magic:R, CosmicPrincess:R
		case 0x010:  if (machine().input().code_pressed(KEYCODE_STOP))      state = true; break;  // X0:I4  Trident:L, Magic:L, CosmicPrincess:L
		case 0x104:  if (machine().input().code_pressed(KEYCODE_COLON))     state = true; break;  // X1:I2  HotHand
		case 0x120:  if (machine().input().code_pressed(KEYCODE_COLON))     state = true; break;  // X1:I5  MemLane:L
		case 0x140:  if (machine().input().code_pressed(KEYCODE_L))         state = true; break;  // X1:I6  MemLane:R, Lectronamo, Nugent, Dracula
		case 0x201:  if (machine().input().code_pressed(KEYCODE_COMMA))     state = true; break;  // X2:I0  Stars:R
		case 0x320:  if (machine().input().code_pressed(KEYCODE_D))         state = true; break;  // X3:I5  Pinball, Stingray
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

CUSTOM_INPUT_MEMBER( st_mp100_state::outhole )
{
	uint8_t bit_mask = ((uintptr_t)param & 0xff);
	uint8_t port = (((uintptr_t)param >> 8) & 0x07);

	/* Here we simulate the ball sitting in the Outhole so the Outhole Solenoid can release it */

	if (machine().input().code_pressed_once(KEYCODE_BACKSPACE))  m_io_hold_x[port] |= bit_mask;

	return ((m_io_hold_x[port] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( st_mp100_state::switch_nugent_dt )
{
	/* Nugent has multiple switches on single drop targets that are spread across different Switch Strobes, so handle this unique situation */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);
	uint8_t port = ((uintptr_t)param >> 8) & 0x07;

	if (machine().input().code_pressed_once(KEYCODE_STOP) || machine().input().code_pressed_once(KEYCODE_QUOTE))
	{
		m_io_hold_x[0] |= bit_mask;
		m_io_hold_x[1] |= bit_mask;
	}
	if (machine().input().code_pressed_once(KEYCODE_F) || machine().input().code_pressed_once(KEYCODE_R))
	{
		m_io_hold_x[3] |= bit_mask;
		m_io_hold_x[4] |= bit_mask;
	}

	return ((m_io_hold_x[port] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( st_mp100_state::switch_hold_x0 )
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

CUSTOM_INPUT_MEMBER( st_mp100_state::switch_hold_x1 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_ENTER))  m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_QUOTE))  m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x20) && machine().input().code_pressed_once(KEYCODE_COLON))  m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x40) && machine().input().code_pressed_once(KEYCODE_L))      m_io_hold_x[1] |= bit_mask;

	return ((m_io_hold_x[1] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( st_mp100_state::switch_hold_x2 )
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

CUSTOM_INPUT_MEMBER( st_mp100_state::switch_hold_x3 )
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

CUSTOM_INPUT_MEMBER( st_mp100_state::switch_hold_x4 )
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


READ8_MEMBER( st_mp100_state::nibble_nvram_r )
{
	return (m_nvram[offset] | 0x0f);
}

WRITE8_MEMBER( st_mp100_state::nibble_nvram_w )
{
	m_nvram[offset] = (data | 0x0f);
}

INPUT_CHANGED_MEMBER( st_mp100_state::activity_button )
{
	if (newval != oldval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, (newval ? ASSERT_LINE : CLEAR_LINE));
}

INPUT_CHANGED_MEMBER( st_mp100_state::self_test )
{
	m_pia_u10->ca1_w(newval);
}

INPUT_CHANGED_MEMBER( st_mp100_state::reset_sam )
{
	if (newval != oldval)
		m_maincpu->set_input_line(INPUT_LINE_RESET, (newval ? CLEAR_LINE : ASSERT_LINE));
}

READ_LINE_MEMBER( st_mp100_state::u10_ca1_r )
{
	return (m_io_system->read() & 0x01);
}
READ_LINE_MEMBER( st_mp100_state::u10_cb1_r )
{
	return m_u10_cb1;
}

WRITE_LINE_MEMBER( st_mp100_state::u10_ca2_w )
{
#if 0                   // Display Blanking - Out of sync with video redraw rate and causes flicker so it's disabled
	if (m_u10_ca2 && (state == false))
	{
		for (digit = 1; digit <= 8; digit++)
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

WRITE_LINE_MEMBER( st_mp100_state::u10_cb2_w )
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

WRITE_LINE_MEMBER( st_mp100_state::u11_ca2_w )
{
	LOG("New U11 CA2 state %01x, was %01x.   U10-PIA-PortA=%02x\n", state, m_u11_ca2, m_u10a);

	output().set_value("led0", state);

	m_u11_ca2 = state;
}

READ_LINE_MEMBER( st_mp100_state::u11_ca1_r )
{
	return m_u11_ca1;
}

READ_LINE_MEMBER( st_mp100_state::u11_cb1_r )
{
	/* Pin 32 on MPU J5 AID connector tied low */
	return m_u11_cb1;
}

WRITE_LINE_MEMBER( st_mp100_state::u11_cb2_w )
{
	m_u11_cb2 = state;
}

READ8_MEMBER( st_mp100_state::u10_a_r )
{
	return m_u10a;
}

WRITE8_MEMBER( st_mp100_state::u10_a_w )
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

READ8_MEMBER( st_mp100_state::u10_b_r )
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

WRITE8_MEMBER( st_mp100_state::u10_b_w )
{
	m_u10b = data;
}

READ8_MEMBER( st_mp100_state::u11_a_r )
{
	return m_u11a;
}

WRITE8_MEMBER( st_mp100_state::u11_a_w )
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

WRITE8_MEMBER( st_mp100_state::u11_b_w )
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


void st_mp100_state::update_lamps(u8 lamp_board_base, u8 lamp_decoder_base, u8 lamp_decoder_addr, u8 lamp_decoder_data)
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


TIMER_DEVICE_CALLBACK_MEMBER( st_mp100_state::timer_z_freq )
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
TIMER_DEVICE_CALLBACK_MEMBER( st_mp100_state::timer_z_pulse )
{
	/*** Line Power to DC Zero Crossing has ended ***/

	m_u10_cb1 = false;
	m_pia_u10->cb1_w(m_u10_cb1);
}

TIMER_DEVICE_CALLBACK_MEMBER( st_mp100_state::u11_timer )
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

TIMER_DEVICE_CALLBACK_MEMBER( st_mp100_state::timer_d_pulse )
{
	m_u11_ca1 = false;
	m_pia_u11->ca1_w(m_u11_ca1);
}



st_mp100_state::solenoid_feature_data const st_mp100_state::s_solenoid_features_default =
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


st_mp100_state::solenoid_feature_data const pinball_state::s_solenoid_features_pinball =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x05, 0x03,  0x00, 0x00 },     // Chime 10
	/*01*/  { 0x05, 0x02,  0x00, 0x00 },     // Chime 100
	/*02*/  { 0x05, 0x01,  0x00, 0x00 },     // Chime 1000
	/*03*/  { 0x05, 0x04,  0x00, 0x00 },     // Chime 10000
	/*04*/  { 0x03, 0x0b,  0x08, 0x06 },     // 2 Drop Targets Reset Upper Left
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x02, 0x10 },     // Saucer Top
	/*08*/  { 0x02, 0x05,  0x02, 0x08 },     // Saucer Right
	/*09*/  { 0x03, 0x0b,  0x08, 0x19 },     // 2 Drop Targets Reset Upper Right and Lower Left
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//      {  SC,   SE,   SR,    N/U }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};


st_mp100_state::solenoid_feature_data const stingray_state::s_solenoid_features_stingray =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x05, 0x03,  0x00, 0x00 },     // Chime 10
	/*01*/  { 0x05, 0x02,  0x00, 0x00 },     // Chime 100
	/*02*/  { 0x05, 0x01,  0x00, 0x00 },     // Chime 1000
	/*03*/  { 0x05, 0x04,  0x00, 0x00 },     // Chime 10000
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0xff, 0x00,  0x00, 0x00 },
	/*08*/  { 0x02, 0x05,  0x02, 0x08 },     // 2x Saucers
	/*09*/  { 0x03, 0x0b,  0x08, 0x1f },     // Drop Target Bank Reset
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

st_mp100_state::solenoid_feature_data const stars_state::s_solenoid_features_stars =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x05, 0x03,  0x00, 0x00 },     // Chime 10
	/*01*/  { 0x05, 0x02,  0x00, 0x00 },     // Chime 100
	/*02*/  { 0x05, 0x01,  0x00, 0x00 },     // Chime 1000
	/*03*/  { 0x05, 0x04,  0x00, 0x00 },     // Chime 10000
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0xff, 0x00,  0x00, 0x00 },
	/*08*/  { 0x03, 0x0b,  0x08, 0xc8 },     // 3 Drop Targets Reset left
	/*09*/  { 0x03, 0x0b,  0x08, 0x34 },     // 3 Drop Targets Reset Right
	/*10*/  { 0xff, 0x00,  0x00, 0x00 },
	/*11*/  { 0xff, 0x00,  0x00, 0x00 },
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

st_mp100_state::solenoid_feature_data const memlane_state::s_solenoid_features_memlane =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x05, 0x03,  0x00, 0x00 },     // Chime 10
	/*01*/  { 0x05, 0x02,  0x00, 0x00 },     // Chime 100
	/*02*/  { 0x05, 0x01,  0x00, 0x00 },     // Chime 1000
	/*03*/  { 0x05, 0x04,  0x00, 0x00 },     // Chime 10000
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x10, 0x80 },     // Saucer Top
	/*08*/  { 0x02, 0x05,  0x10, 0x40 },     // Saucer Left
	/*09*/  { 0x03, 0x0b,  0x01, 0x07 },     // Drop Target Bank Reset
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*11*/  { 0xff, 0x00,  0x00, 0x00 },
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

st_mp100_state::solenoid_feature_data const lectrono_state::s_solenoid_features_lectrono =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x05, 0x03,  0x00, 0x00 },     // * Mod * Chime 10
	/*01*/  { 0x05, 0x02,  0x00, 0x00 },     // * Mod * Chime 100
	/*02*/  { 0x05, 0x01,  0x00, 0x00 },     // * Mod * Chime 1000
	/*03*/  { 0x05, 0x04,  0x00, 0x00 },     // * Mod * Chime 10000
	/*04*/  { 0x03, 0x0b,  0x0e, 0x20 },     // 3 Drop Target Bank Reset
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x10, 0x80 },     // Saucer Middle
	/*08*/  { 0x02, 0x0a,  0x04, 0x80 },     // Kickback Lane
	/*09*/  { 0x03, 0x0b,  0x1f, 0x10 },     // 5 Drop Target Bank Reset
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Middle
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

st_mp100_state::solenoid_feature_data const nugent_state::s_solenoid_features_nugent =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x05, 0x03,  0x00, 0x00 },     // * Mod * Chime 10
	/*01*/  { 0x05, 0x02,  0x00, 0x00 },     // * Mod * Chime 100
	/*02*/  { 0x05, 0x01,  0x00, 0x00 },     // * Mod * Chime 1000
	/*03*/  { 0x05, 0x04,  0x00, 0x00 },     // * Mod * Chime 10000
	/*04*/  { 0x03, 0x0b,  0x0e, 0x20 },     // Drop Target Bank Reset Rightside
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0xff, 0x00,  0x00, 0x00 },
	/*08*/  { 0x02, 0x0a,  0x04, 0x80 },     // Saucer
	/*09*/  { 0x03, 0x0b,  0x1f, 0x10 },     // Drop Target Bank Reset Leftside (5 switches)
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Middle
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

st_mp100_state::solenoid_feature_data const wildfyre_state::s_solenoid_features_wildfyre =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x05, 0x03,  0x00, 0x00 },     // * Mod * Chime 10
	/*01*/  { 0x05, 0x02,  0x00, 0x00 },     // * Mod * Chime 100
	/*02*/  { 0x05, 0x01,  0x00, 0x00 },     // * Mod * Chime 1000
	/*03*/  { 0x05, 0x04,  0x00, 0x00 },     // * Mod * Chime 10000
	/*04*/  { 0x03, 0x0b,  0x1c, 0x10 },     // 3 Drop Target Bank Reset
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x04, 0xe0 },     // Saucer Top x3
	/*08*/  { 0x02, 0x05,  0x10, 0x80 },     // Saucer Collect Bonus
	/*09*/  { 0x03, 0x0b,  0x1e, 0x08 },     // 4 Drop Target Bank Reset
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Middle
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

st_mp100_state::solenoid_feature_data const dracula_state::s_solenoid_features_dracula =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x05, 0x03,  0x00, 0x00 },     // * Mod * Chime 10
	/*01*/  { 0x05, 0x02,  0x00, 0x00 },     // * Mod * Chime 100
	/*02*/  { 0x05, 0x01,  0x00, 0x00 },     // * Mod * Chime 1000
	/*03*/  { 0x05, 0x04,  0x00, 0x00 },     // * Mod * Chime 10000
	/*04*/  { 0x03, 0x0b,  0x1c, 0x10 },     // 3 Drop Target Bank Reset
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0xff, 0x00,  0x00, 0x00 },
	/*08*/  { 0x02, 0x05,  0x10, 0x80 },     // Saucer Collect Bonus
	/*09*/  { 0x03, 0x0b,  0x1e, 0x08 },     // 4 Drop Target Bank Reset
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Middle
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

st_mp100_state::solenoid_feature_data const trident_state::s_solenoid_features_trident =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right Upper
	/*01*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Lower
	/*02*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left Upper
	/*03*/  { 0x05, 0x0c,  0x88, 0x80 },     // Drop Target #1
	/*04*/  { 0x05, 0x0c,  0x88, 0x40 },     // Drop Target #2
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left Bottom
	/*07*/  { 0x05, 0x0c,  0x88, 0x20 },     // Drop Target #3
	/*08*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right Upper
	/*09*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right Lower
	/*10*/  { 0x05, 0x0c,  0x88, 0x10 },     // Drop Target #4
	/*11*/  { 0x05, 0x0c,  0x88, 0x08 },     // Drop Target #5
	/*12*/  { 0x02, 0x05,  0x08, 0x02 },     // Saucer Top
	/*13*/  { 0x03, 0x0b,  0x08, 0xf8 },     // Drop Target Bank Reset
	/*14*/  { 0x01, 0x09,  0x10, 0x01 },     // Outhole
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

st_mp100_state::solenoid_feature_data const princess_state::s_solenoid_features_princess =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*01*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*02*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top
	/*03*/  { 0x05, 0x0c,  0x88, 0x80 },     // Drop Target #1
	/*04*/  { 0x05, 0x0c,  0x88, 0x40 },     // Drop Target #2
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*07*/  { 0x05, 0x0c,  0x88, 0x20 },     // Drop Target #3
	/*08*/  { 0xff, 0x00,  0x00, 0x00 },
	/*09*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*10*/  { 0x05, 0x0c,  0x88, 0x10 },     // Drop Target #4
	/*11*/  { 0x05, 0x0c,  0x88, 0x08 },     // Drop Target #5
	/*12*/  { 0x02, 0x05,  0x08, 0x02 },     // Saucer Top
	/*13*/  { 0x03, 0x0b,  0x08, 0xf8 },     // Drop Target Bank Reset
	/*14*/  { 0x01, 0x09,  0x10, 0x01 },     // Outhole
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

st_mp100_state::solenoid_feature_data const magic_state::s_solenoid_features_magic =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*01*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*02*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*07*/  { 0xff, 0x00,  0x00, 0x00 },
	/*08*/  { 0xff, 0x00,  0x00, 0x00 },
	/*09*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*10*/  { 0xff, 0x00,  0x00, 0x00 },
	/*11*/  { 0xff, 0x00,  0x00, 0x00 },
	/*12*/  { 0x02, 0x05,  0x08, 0x02 },     // Saucer Top
	/*13*/  { 0x03, 0x0b,  0x08, 0xf8 },     // Drop Target Bank Reset
	/*14*/  { 0x01, 0x09,  0x10, 0x01 },     // Outhole
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

st_mp100_state::solenoid_feature_data const hothand_state::s_solenoid_features_hothand =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0xff, 0x00,  0x00, 0x00 },
	/*07*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Middle
	/*08*/  { 0x03, 0x0b,  0x08, 0xf8 },     // Drop Target Bank Reset
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*10*/  { 0x02, 0x0a,  0x10, 0x02 },     // Kickback Lane
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*12*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*14*/  { 0x01, 0x09,  0x10, 0x01 },     // Outhole
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0x00, 0x0e, 0x0f,  0x00 }      // Motor for Large Upper Rotating Flipper relay
};


void st_mp100_state::machine_start()
{
	genpin_class::machine_start();

	m_lamps.resolve();
	m_display.resolve();
	m_solenoids.resolve();
	m_spinners.resolve();
}

void st_mp100_state::machine_reset()
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


WRITE8_MEMBER( st_mp100_state::sb100_w )
{
	// address_space &space = m_maincpu->space(AS_PROGRAM);

	m_discrete->write(space, NODE_01, ((data & 0x01)>>0) * 1);
	m_discrete->write(space, NODE_02, ((data & 0x02)>>1) * 1);
	m_discrete->write(space, NODE_03, ((data & 0x04)>>2) * 1);
	m_discrete->write(space, NODE_04, ((data & 0x08)>>3) * 1);
	m_discrete->write(space, NODE_05, ((data & 0x10)>>4) * 5);
	m_discrete->write(space, NODE_06, ((data & 0x20)>>5) * 4);
}



static const discrete_mixer_desc sb100_digital_mixer_info =
{
		DISC_MIXER_IS_RESISTOR,                       /* type */
		{RES_M(2.2), RES_M(2.2), RES_M(2.2), RES_M(2.2), RES_M(2.2), RES_M(2.2)},    /* r{ } */
		{0, 0, 0, 0, 0, 0},                           /* r_node */
		{0, 0, 0, 0, 0, 0},                           /* c{} */
		0,                                            /* rI  */
//      RES_VOLTAGE_DIVIDER(RES_K(10), RES_R(360)),   /* rF  */
		RES_K(10),                                    /* rF  */   // not really
		0,                                            /* cF  */
		0,                                            /* cAmp */
		0,                                            /* vRef */
		0.00002                                       /* gain */
};

static const discrete_555_desc sb100_discrete_556_A =
{
    DISC_555_OUT_ENERGY | DISC_555_OUT_DC,  /* options */
    5,                     /* v_pos */
    DEFAULT_555_CHARGE,    /* v_charge */
    DEFAULT_555_HIGH       /* v_out_high */
};
static const discrete_555_desc sb100_discrete_556_B =
{
    DISC_555_OUT_ENERGY | DISC_555_OUT_COUNT_F,  /* options */
    5,                     /* v_pos */
    DEFAULT_555_CHARGE,    /* v_charge */
    DEFAULT_555_HIGH       /* v_out_high */
};

static const discrete_op_amp_osc_info sb100_discrete_op_amp_osc =
{
	// type, r1, r2, r3, r4, r5, r6, r7, r8, c, vP
	DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_K(100), RES_K(47), RES_K(100), RES_K(47), RES_K(47), RES_K(47),0,0,CAP_N(3), 5
};

static DISCRETE_SOUND_START(sb100)
	DISCRETE_INPUT_LOGIC(NODE_01)       // Bit xx000001      10 point sound
	DISCRETE_INPUT_LOGIC(NODE_02)       // Bit xx000010     100 point sound
	DISCRETE_INPUT_LOGIC(NODE_03)       // Bit xx000100   1,000 point sound
	DISCRETE_INPUT_LOGIC(NODE_04)       // Bit xx001000  10,000 point sound
	DISCRETE_INPUT_DATA(NODE_05)        // Bit xx010000  Add Bonus sound
	DISCRETE_INPUT_DATA(NODE_06)        // Bit xx100000  Pop Bumper sound

	
	DISCRETE_555_ASTABLE(NODE_52, NODE_05, RES_K(100), RES_K(680), CAP_U(0.1), &sb100_discrete_556_A)
	DISCRETE_RCFILTER(NODE_53, NODE_52, RES_K(2.2), CAP_U(22))
	DISCRETE_555_ASTABLE_CV(NODE_54,NODE_05,RES_K(15),RES_K(1),CAP_U(0.1),NODE_53, &sb100_discrete_556_B)
	DISCRETE_LOGIC_JKFLIPFLOP(NODE_51,NODE_05,5,NODE_54,5,5)

	DISCRETE_GAIN(NODE_59, NODE_51, 50) // Output amplifier LM380 fixed inbuilt gain of 50



	//DISCRETE_CRFILTER_VREF(NODE_63,NODE_06,RES_K(10),CAP_N(100), NODE_64)

	DISCRETE_CRFILTER(NODE_65, NODE_06, RES_M(1), CAP_U(0.33))

//	DISCRETE_SWITCH(NODE_62,1, NODE_06, 1.0, NODE_65)
	DISCRETE_RAMP(NODE_62,NODE_06, 0, 4.0, 4, 0, 0)


	DISCRETE_OP_AMP_VCO1(NODE_67, NODE_62, NODE_65, &sb100_discrete_op_amp_osc)
//	DISCRETE_CRFILTER(NODE_68,NODE_67,RES_M(100),CAP_P(100))
	DISCRETE_GAIN(NODE_69, NODE_67, 100)  // Output amplifier LM380 fixed inbuilt gain of 50




// Nugent:          523, 698, 880, 1046  C5,   F5,   A5,   C6     SB100 Rev A
// Cosmic Princess: 104, 138, 172, 208   G#2,  C#3,  F3,   G#3    SB100 Rev C-1
// Hot Hand:        110, 139, 165, 220   A10,  C#14, E17,  A22    SB100 Rev B
// Dracula:         116, 156, 196, 233   A#11, D#16, G20,  A#23   SB100 Rev B (Game shipped with Rev B, but supports Rev A boards with chimes)
	DISCRETE_SQUAREWAVE(NODE_11,NODE_01,208,100,50,0,90) // Frequency set by Pot R2
	DISCRETE_SQUAREWAVE(NODE_21,NODE_02,172,100,50,0,90) // Frequency set by Pot R13
	DISCRETE_SQUAREWAVE(NODE_31,NODE_03,138,100,50,0,90) // Frequency set by Pot R6
	DISCRETE_SQUAREWAVE(NODE_41,NODE_04,104,100,50,0,90) // This frequency is divided through a flip flop off Node_11


	DISCRETE_MIXER6(NODE_70, 1, NODE_11, NODE_21, NODE_31, NODE_41, NODE_59, NODE_69, &sb100_digital_mixer_info)   // Mix and filter T1 and T4 together

	DISCRETE_CRFILTER(NODE_71,NODE_70,RES_M(10),CAP_N(100))

	DISCRETE_GAIN(NODE_99, NODE_71, 100) // Output amplifier LM380 fixed inbuilt gain of 50

	DISCRETE_OUTPUT(NODE_99, 10000000)   // 17000000

DISCRETE_SOUND_END



MACHINE_CONFIG_START( st_mp100_state::st_mp100 )
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M6800, 530000) // No xtal, just 2 chips forming a multivibrator oscillator around 530kHz
	MCFG_DEVICE_PROGRAM_MAP(st_mp100_map)

	MCFG_NVRAM_ADD_0FILL("nvram")   // 'F' filled causes Credit Display to be blank on first startup

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_st_mp100)

	/* Sound */
	genpin_audio(config);

	/* Devices */
	MCFG_DEVICE_ADD("pia_u10", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, st_mp100_state, u10_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, st_mp100_state, u10_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(*this, st_mp100_state, u10_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, st_mp100_state, u10_b_w))
	MCFG_PIA_READCA1_HANDLER(READLINE(*this, st_mp100_state, u10_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(*this, st_mp100_state, u10_cb1_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(*this, st_mp100_state, u10_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, st_mp100_state, u10_cb2_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_TIMER_DRIVER_ADD("timer_z_freq", st_mp100_state, timer_z_freq)						// Mains Line Frequency * 2 (100Hz or 120Hz depending on country)
	MCFG_TIMER_DRIVER_ADD(m_zero_crossing_active_timer, st_mp100_state, timer_z_pulse)		// Active pulse length from Zero Crossing detector

	MCFG_DEVICE_ADD("pia_u11", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, st_mp100_state, u11_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, st_mp100_state, u11_a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, st_mp100_state, u11_b_w))
	MCFG_PIA_READCA1_HANDLER(READLINE(*this, st_mp100_state, u11_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(*this, st_mp100_state, u11_cb1_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(*this, st_mp100_state, u11_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, st_mp100_state, u11_cb2_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_d_freq", st_mp100_state, u11_timer, PERIOD_OF_555_ASTABLE(36000, 4700, 0.0000001))	// 555 Timer 317-320Hz
	MCFG_TIMER_DRIVER_ADD(m_display_refresh_timer, st_mp100_state, timer_d_pulse)												// 555 Active pulse length
MACHINE_CONFIG_END

MACHINE_CONFIG_START(st_mp100_state::st_mp100F)
	st_mp100(config);

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(st_mp100_map_full)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(st_mp100_state::st_sb100)
	st_mp100(config);

	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("discrete", DISCRETE, sb100)        // This hardwares sound board (SB100 Rev A) also includes electronic chime simulation (not emulated yet). Tones are also pitched higher
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(st_mp100_state::st_sb100F)
	st_mp100(config);

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(st_mp100_map_full)

	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("discrete", DISCRETE, sb100)        // This hardwares sound board (SB100 Rev A) also includes electronic chime simulation (not emulated yet). Tones are also pitched higher
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(st_mp100_state::st_sb100c)
	st_mp100(config);

	MCFG_DEFAULT_LAYOUT(layout_st_mp200)                // System switch defaults are different to earlier games

	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("discrete", DISCRETE, sb100)        // This hardwares sound boards (SB100 Rev B and Rev C-1) has no electronic chime simulation
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(st_mp100_state::st_sb100cF)
	st_mp100(config);

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(st_mp100_map_full)

	MCFG_DEFAULT_LAYOUT(layout_st_mp200)                // System switch defaults are different to earlier games

	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("discrete", DISCRETE, sb100)        // This hardwares sound boards (SB100 Rev B and Rev C-1) have no electronic chime simulation
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END



/*--------------------------------
/ Pinball #101
/-------------------------------*/
ROM_START(pinball)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(1db32a33) SHA1(2f0a3ca36968b81f29373e4f2cf7ee28a4071882))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(432e9b9e) SHA1(292e509f50bc841f6e469c198fc82c2a9095f008))
ROM_END

ROM_START(pinballt)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "pinballt.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_LOAD( "pinballt.u6", 0x1800, 0x0800, CRC(00000000))
ROM_END

/*------------------------------------
/ Stingray #102 - same roms as Pinball
/-------------------------------------*/
ROM_START(stingray)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(1db32a33) SHA1(2f0a3ca36968b81f29373e4f2cf7ee28a4071882))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(432e9b9e) SHA1(292e509f50bc841f6e469c198fc82c2a9095f008))
ROM_END

ROM_START(stingrayt)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "stingrayt.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_LOAD( "stingrayt.u6", 0x1800, 0x0800, CRC(00000000))
ROM_END

ROM_START(stingrayf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.732", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "cpu_u6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5FF8, 0xFFF8,0x08)
ROM_END

/*--------------------------------
/ Stars #103
/-------------------------------*/
ROM_START(stars)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "25a-rom-p3a_u2.cpu", 0x1000, 0x0800, CRC(da00b6d1) SHA1(5961eae2390571b7ac3be537478a90fdfce00bce))
	ROM_LOAD( "25a-rom-p4a_u6.cpu", 0x1800, 0x0800, CRC(0d13ebaa) SHA1(e75c88b67ed3fcb613e58dd678febfce927284e0))
ROM_END

ROM_START(starso)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "25a-rom-p3_u2.cpu", 0x1000, 0x0800, CRC(630d05df) SHA1(2baa16265d524297332fa951d9eab3e0e8d26078))
	ROM_LOAD( "25a-rom-p4_u6.cpu", 0x1800, 0x0800, CRC(57e63d42) SHA1(619ef955553654893c3071d8b70855fee8a5e6a7))
ROM_END

/*--------------------------------
/ Memory Lane #104
/-------------------------------*/
ROM_START(memlane)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(aff1859d) SHA1(5a9801d139bf2477b6d351a2654ae07516be144a))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(3e236e3c) SHA1(7f631a5fac8a1b1af3b5332ba38d52553f13531a))
ROM_END

/*--------------------------------
/ Lectronamo #105
/-------------------------------*/
ROM_START(lectrono)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(79e918ff) SHA1(a728eb26d941a9c7484be593a216905237d32551))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(7c6e5fb5) SHA1(3aa4e0c1f377ba024e6b34bd431a188ff02d4eaa))
ROM_END

/*--------------------------------
/ Wildfyre #106
/-------------------------------*/
ROM_START(wildfyre)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(063f8b5e) SHA1(80434de549102bff829b474603d6736b839b8999))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(00336fbc) SHA1(d2c360b8a80b209ecf4ec02ee19a5234c0364504))
ROM_END

ROM_START(wildfyrec)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "draculac_2.716", 0x1000, 0x0800, CRC(00000000))
	ROM_LOAD( "draculac_6.716", 0x1800, 0x0800, CRC(00000000))
ROM_END

ROM_START(wildfyret)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "draculat_u2.716", 0x1000, 0x0800, CRC(00000000))
	ROM_LOAD( "draculat_u6.716", 0x1800, 0x0800, CRC(00000000))
ROM_END

ROM_START(wildfyref)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "draculaf_u2.732", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "draculaf_u6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5FF8, 0xFFF8,0x08)
ROM_END


/*-----------------------------------
/ Nugent #108 - same ROMs as lectrono
/------------------------------------*/
ROM_START(nugent)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(79e918ff) SHA1(a728eb26d941a9c7484be593a216905237d32551))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(7c6e5fb5) SHA1(3aa4e0c1f377ba024e6b34bd431a188ff02d4eaa))
ROM_END

/*------------------------------------
/ Dracula #109 - same ROMs as wildfyre
/-------------------------------------*/
ROM_START(dracula)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "25arom-p9.u2",  0x1000, 0x0800, CRC(063f8b5e) SHA1(80434de549102bff829b474603d6736b839b8999))
	ROM_LOAD( "25arom-p10.u6", 0x1800, 0x0800, CRC(00336fbc) SHA1(d2c360b8a80b209ecf4ec02ee19a5234c0364504))   // PROM 9316A-2917
ROM_END

ROM_START(draculac)	// Coded for Electro-Mechanical Chimes
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "draculac_2.716", 0x1000, 0x0800, CRC(00000000))
	ROM_LOAD( "draculac_6.716", 0x1800, 0x0800, CRC(00000000))
ROM_END

ROM_START(draculat)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "draculat_u2.716", 0x1000, 0x0800, CRC(00000000))
	ROM_LOAD( "draculat_u6.716", 0x1800, 0x0800, CRC(00000000))
ROM_END

ROM_START(draculaf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "draculaf_u2.732", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "draculaf_u6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5FF8, 0xFFF8,0x08)
ROM_END


/*--------------------------------
/ Trident #110
/-------------------------------*/
ROM_START(trident)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "25arom_p11a.u2",  0x1000, 0x0800, CRC(6dcd6ad3) SHA1(f748acc8628c5013b630a5c7b25a1bf72e36b16d))
	ROM_LOAD( "25arom_p12au.u6", 0x1800, 0x0800, CRC(fb955a6f) SHA1(387080d5af318463475797fecff026d6db776a0c))
ROM_END

ROM_START(tridento)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "25arom_p11.u2",  0x1000, 0x0800, CRC(934e49dd) SHA1(cbf6ca2759166f522f651825da0c75cf7248d3da))
	ROM_LOAD( "25arom_p12u.u6", 0x1800, 0x0800, CRC(540bce56) SHA1(0b21385501b83e448403e0216371487ed54026b7))
ROM_END

ROM_START(tridentf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.732", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "cpu_u6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5FF8, 0xFFF8,0x08)
ROM_END


/*-------------------------------------
/ Cosmic Princess #111 - same ROMs as Magic
/-------------------------------------*/
ROM_START(princess)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(8838091f) SHA1(d2702b5e15076793b4560c77b78eed6c1da571b6))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(fb955a6f) SHA1(387080d5af318463475797fecff026d6db776a0c))
ROM_END

ROM_START(princesst)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(00000000))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(00000000))
ROM_END

ROM_START(princessf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.732", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "cpu_u6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5FF8, 0xFFF8,0x08)
ROM_END

ROM_START(princessp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2_2timer.732", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "cpu_u6_2timer.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5FF8, 0xFFF8,0x08)
ROM_END


/*--------------------------------
/ Hot Hand #112
/-------------------------------*/
ROM_START(hothand)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "25arom_p13.u2",   0x1000, 0x0800, CRC(5e79ea2e) SHA1(9b45c59b2076fcb3a35de1dd3ba2444ea852f149))
	ROM_LOAD( "25arom_p12au.u6", 0x1800, 0x0800, CRC(fb955a6f) SHA1(387080d5af318463475797fecff026d6db776a0c))
ROM_END

ROM_START(hothandt)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "hothandt_u2.716",   0x1000, 0x0800, CRC(00000000))
	ROM_LOAD( "hothandt_u6.716", 0x1800, 0x0800, CRC(00000000))
ROM_END

ROM_START(hothandf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "hothandf_u2.732", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "hothandf_u6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5FF8, 0xFFF8,0x08)
ROM_END

ROM_START(hothanda)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "hothand512_u2.716",   0x8000, 0x0800, CRC(00000000))
	ROM_LOAD( "hothand512_u2.716", 0x8800, 0x0800, CRC(00000000))
	ROM_COPY("maincpu", 0x8FF8, 0xFFF8,0x08)
ROM_END


/*------------------------------------
/ Magic #115 - 2nd ROM same as HotHand
/-------------------------------------*/
ROM_START(magic)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(8838091f) SHA1(d2702b5e15076793b4560c77b78eed6c1da571b6))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(fb955a6f) SHA1(387080d5af318463475797fecff026d6db776a0c))
ROM_END

ROM_START(magict)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(00000000))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(00000000))
ROM_END

ROM_START(magicf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.732", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "cpu_u6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5FF8, 0xFFF8,0x08)
ROM_END

/*----------------------------------
/ Black Sheep Squadron (Astro game)
/---------------------------------*/
ROM_START(blkshpsq)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(23d6cd54) SHA1(301ba10f3f333109630dd8abd13a6b4063f805a9))
	ROM_RELOAD( 0x5000, 0x0800)
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(ea68b9f7) SHA1(ebb69f4faadf457454939e47d8ae6e79eb0e1a11))
	ROM_RELOAD( 0x5800, 0x0800)
//	ROM_RELOAD( 0x7800, 0x0800)
ROM_END


/*----------------------------------
/ Stern System Test Fixture
/---------------------------------*/
ROM_START(st_game2)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, NO_DUMP)	// Loading the "st_game4" ROMs will also work here on the MPU-100 board
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, NO_DUMP)
ROM_END


//  (  YEAR   NAME        PARENT      MACHINE     INPUT       STATE           INIT             MONITOR COMPANY  FULLNAME                   FLAGS )
// Electro-mechanical chimes
GAMEL( 1977,  pinball,    0,          st_mp100,   pinball,    pinball_state,  init_st_mp100,    ROT0,  "Stern", "Pinball",                 MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp100_pinball )
GAMEL( 1977,  pinballt,   pinball,    st_mp100,   pinballt,   pinball_state,  init_st_mp100,    ROT0,  "Stern", "Pinball TEST",            MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp100_pinball )
GAMEL( 1977,  stingray,   0,          st_mp100,   stingray,   stingray_state, init_st_mp100,    ROT0,  "Stern", "Stingray",                MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp100_stingray )
GAMEL( 1977,  stingrayt,  stingray,   st_mp100,   stingrayt,  stingray_state, init_st_mp100,    ROT0,  "Stern", "Stingray TEST",           MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp100_stingray )
GAMEL( 1977,  stingrayf,  stingray,   st_mp100F,  stingrayf,  stingray_state, init_st_mp100,    ROT0,  "Stern", "Stingray TEST -35 FullAddressing",        MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp100_stingrayF )
GAMEL( 1978,  stars,      0,          st_mp100,   stars,      stars_state,    init_st_mp100,    ROT0,  "Stern", "Stars",                   MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp100_stars )
GAMEL( 1978,  starso,     stars,      st_mp100,   stars,      stars_state,    init_st_mp100,    ROT0,  "Stern", "Stars (Older set)",       MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp100_stars )
GAMEL( 1978,  memlane,    0,          st_mp100,   memlane,    memlane_state,  init_st_mp100,    ROT0,  "Stern", "Memory Lane",             MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp100_memlane )
GAME ( 1978,  blkshpsq,   0,          st_mp100,   mp100,      st_mp100_state, init_st_mp100,    ROT0,  "Astro", "Black Sheep Squadron",    MACHINE_MECHANICAL | MACHINE_NOT_WORKING)

// Electronic sound unit SB-100 Rev A (B-521)
GAMEL( 1978,  lectrono,   0,          st_sb100,   lectrono,   lectrono_state, init_st_mp100,    ROT0,  "Stern", "Lectronamo",              MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_lectrono )
GAMEL( 1978,  wildfyre,   0,          st_sb100,   wildfyre,   wildfyre_state, init_st_mp100,    ROT0,  "Stern", "Wildfyre",                MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_wildfyre )
GAMEL( 1978,  wildfyret,  wildfyre,   st_sb100,   wildfyret,  wildfyre_state, init_st_mp100,    ROT0,  "Stern", "Wildfyre TEST",           MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_wildfyre )
GAMEL( 1978,  wildfyrec,  wildfyre,   st_sb100,   wildfyrec,  wildfyre_state, init_st_mp100,    ROT0,  "Stern", "Wildfyre TEST Chimes",    MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp100_wildfyre )
GAMEL( 1979,  wildfyref,  wildfyre,   st_sb100F,  wildfyrec,  wildfyre_state, init_st_mp100,    ROT0,  "Stern", "Wildfyre TEST -35 FullAddressing",        MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_dracula )
GAMEL( 1978,  nugent,     0,          st_sb100,   nugent,     nugent_state,   init_st_mp100,    ROT0,  "Stern", "Nugent",                  MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_nugent )
GAMEL( 1979,  dracula,    0,          st_sb100,   dracula,    dracula_state,  init_st_mp100,    ROT0,  "Stern", "Dracula (Pinball)",       MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_dracula )
GAMEL( 1979,  draculat,   dracula,    st_sb100,   draculat,   dracula_state,  init_st_mp100,    ROT0,  "Stern", "Dracula TEST",            MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_dracula )
GAMEL( 1979,  draculac,   dracula,    st_sb100,   draculac,   dracula_state,  init_st_mp100,    ROT0,  "Stern", "Dracula TEST Chimes",     MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp100_dracula )
GAMEL( 1979,  draculaf,   dracula,    st_sb100F,  draculat,   dracula_state,  init_st_mp100,    ROT0,  "Stern", "Dracula TEST -35 FullAddressing",         MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_dracula )
GAMEL( 197?,  st_game2,   0,          st_sb100,   st_game,    st_mp100_state, init_st_mp200,    ROT0,  "Stern", "SAM II (Service Assistance Module 2)",    MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp200_sam)

// Electronic sound unit SB-100 Rev B and Rev C-1, different inputs
GAMEL( 1979,  trident,    0,          st_sb100c,  trident,    trident_state,  init_st_mp200,    ROT0,  "Stern", "Trident",                                                 MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_trident )
GAMEL( 1979,  tridento,   trident,    st_sb100c,  trident,    trident_state,  init_st_mp200,    ROT0,  "Stern", "Trident (Older set)",                                     MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_trident )
GAMEL( 1979,  tridentf,   trident,    st_sb100cF, tridentf,   trident_state,  init_st_mp200,    ROT0,  "Stern", "Trident TEST -35 FullAddressing",                         MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_trident )
GAMEL( 1979,  hothand,    0,          st_sb100c,  hothand,    hothand_state,  init_st_mp200,    ROT0,  "Stern", "Hot Hand",                                                MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_hothand )
GAMEL( 1979,  hothandt,   hothand,    st_sb100c,  hothand,    hothand_state,  init_st_mp200,    ROT0,  "Stern", "Hot Hand TEST",                                           MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_hothand )
GAMEL( 1979,  hothandf,   hothand,    st_sb100cF, hothand,    hothand_state,  init_st_mp200,    ROT0,  "Stern", "Hot Hand TEST -35 FullAddressing",                        MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_hothand )
GAMEL( 1979,  hothanda,   hothand,    st_sb100cF, hothand,    hothand_state,  init_st_mp200,    ROT0,  "Stern", "Hot Hand TEST Barakandl FullAddressing",                  MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_hothand )
GAMEL( 1979,  princess,   0,          st_sb100c,  princess,   princess_state, init_st_mp200,    ROT0,  "Stern", "Cosmic Princess",                                         MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_princess )
GAMEL( 1979,  princesst,  princess,   st_sb100c,  princesst,  princess_state, init_st_mp200,    ROT0,  "Stern", "Cosmic Princess TEST",                                    MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_princess )
GAMEL( 1979,  princessf,  princess,   st_sb100cF, princessf,  princess_state, init_st_mp200,    ROT0,  "Stern", "Cosmic Princess TEST -35 FullAddressing",                 MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_princess )
GAMEL( 1979,  princessp,  princess,   st_sb100cF, princessf,  princess_state, init_st_mp200,    ROT0,  "Stern", "Cosmic Princess TEST -35 FullAddressing Prev twin timer", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_princess )
GAMEL( 1979,  magic,      0,          st_sb100c,  magic,      magic_state,    init_st_mp200,    ROT0,  "Stern", "Magic",                                                   MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_magic )
GAMEL( 1979,  magict,     magic,      st_sb100c,  magict,     magic_state,    init_st_mp200,    ROT0,  "Stern", "Magic TEST",                                              MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_magic )
GAMEL( 1979,  magicf,     magic,      st_sb100cF, magicf,     magic_state,    init_st_mp200,    ROT0,  "Stern", "Magic TEST -35 FullAddressing",                           MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_st_mp100_magic )

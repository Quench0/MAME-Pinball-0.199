// license:BSD-3-Clause
// copyright-holders:Robbbert, Quench
/********************************************************************************************

    PINBALL
    Stern MP-200 MPU
    (similar to Bally MPU-35)


ToDo:
- Sound: All machines have a B605/C605 sound card containing a 6840 and many other chips
- Sound: Games 126,128-151,165 have a A720 voice synthesizer with a 'CRC' CPU and many other chips
- Sound: https://pinside.com/pinball/forum/topic/controlling-the-sb-300
- Dips, Inputs, Solenoids vary per game
- Mechanical
*********************************************************************************************/


#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "machine/6840ptm.h"
#include "sound/s14001a.h"
#include "render.h"
#include "speaker.h"

#define VERBOSE 1
#include "logmacro.h"

#include "st_mp200.lh"
#include "st_mp200_meteor.lh"
#include "st_mp200_ali.lh"


#define S14001_CLOCK                (20e3)

class st_mp200_state : public genpin_class
{
public:
	st_mp200_state(machine_config const &mconfig, device_type type, char const *tag)
		: st_mp200_state(mconfig, type, tag, s_solenoid_features_default)
	{ }

	void init_st_mp200()	{ m_7d = 0; m_outhole = 0x0401; m_sw_ext = 0x0000; m_gi_flasher_sol = 255; }
	void init_galaxy()		{ m_7d = 0; m_outhole = 0x0401; m_sw_ext = 0x0008; m_gi_flasher_sol = 16;  }
	void init_ali()		    { m_7d = 0; m_outhole = 0x0401; m_sw_ext = 0x0008; m_gi_flasher_sol = 13;  }
	void init_st_mp207()	{ m_7d = 1; m_outhole = 0x0401; m_sw_ext = 0x0000; m_gi_flasher_sol = 255; }
	void init_st_mp237()	{ m_7d = 1; m_outhole = 0x0407; m_sw_ext = 0x0000; m_gi_flasher_sol = 255; }		// 3 Ball Multiball
	void init_flight2k()	{ m_7d = 1; m_outhole = 0x0407; m_sw_ext = 0x0000; m_gi_flasher_sol = 255; m_disp_key[8] = 32; }		// 3 Ball Multiball
	void init_nineball()	{ m_7d = 1; m_outhole = 0x0419; m_sw_ext = 0x0000; m_gi_flasher_sol = 255; }							// 3 Ball Multiball
	void init_catacomb()	{ m_7d = 1; m_outhole = 0x0407; m_sw_ext = 0x0000; m_gi_flasher_sol = 255; m_disp_key[8] = 32; }		// 3 Ball Multiball
	void init_lightnin()	{ m_7d = 1; m_outhole = 0x0407; m_sw_ext = 0x0000; m_gi_flasher_sol = 255; m_disp_key[8] = 32; }		// 3 Ball Multiball

	DECLARE_INPUT_CHANGED_MEMBER(activity_button);
	DECLARE_INPUT_CHANGED_MEMBER(self_test);
	DECLARE_INPUT_CHANGED_MEMBER(reset_switch);
	DECLARE_CUSTOM_INPUT_MEMBER(outhole);
	DECLARE_CUSTOM_INPUT_MEMBER(spinner);
	DECLARE_CUSTOM_INPUT_MEMBER(kicker);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x0);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x1);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x2);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x3);
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x4);

	void st_mp200(machine_config &config);
	void st_mp200_ext(machine_config &config);
	void st_mp200_full(machine_config &config);
	void st_mp200_spch(machine_config &config);


protected:
	typedef uint8_t solenoid_feature_data[20][4];

	st_mp200_state(machine_config const &mconfig, device_type type, char const *tag, solenoid_feature_data const &solenoid_features)
		: genpin_class(mconfig, type, tag)
		, m_solenoid_features(solenoid_features)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_pia_u10(*this, "pia_u10")
		, m_pia_u11(*this, "pia_u11")
		, m_zero_crossing_freq_timer(*this, "timer_z_freq")
		, m_zero_crossing_active_timer(*this, "timer_z_pulse")
		, m_display_refresh_timer(*this, "timer_d_pulse")
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
		, m_scrn_switches(*this, "switch%u", 0U)
///		, m_sb300_ptm(*this, "sb300_ptm")
		, m_s14001a(*this, "speech")
	{ }


	DECLARE_READ8_MEMBER(u10_a_r);
	DECLARE_WRITE8_MEMBER(u10_a_w);
	DECLARE_READ8_MEMBER(u10_b_r);
	DECLARE_WRITE8_MEMBER(u10_b_w);
	DECLARE_READ8_MEMBER(u11_a_r);
	DECLARE_WRITE8_MEMBER(u11_a_w);
	DECLARE_WRITE8_MEMBER(u11_b_w);
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
///	DECLARE_WRITE_LINE_MEMBER(ptm_o1_callback);
///	DECLARE_WRITE_LINE_MEMBER(ptm_o2_callback);
///	DECLARE_WRITE_LINE_MEMBER(ptm_o3_callback);

	void st_mp200_map(address_map &map);
	void st_mp200_map_ext(address_map &map);
	void st_mp200_map_full(address_map &map);

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
	uint16_t m_sw_ext;            // Drop Target switches can be in a separate part of the switch matrix. So allow them to be part of a bank reset
	uint16_t m_outhole;           // Optionally put a ball in the outhole at power on
	uint16_t m_spinner[4];        // Helps animate a simulated Spinner on the playfield layout
	uint8_t m_gi_flasher_sol;     // Solenoid number that activates the G.I. flasher relay
	uint8_t m_disp_key[41];       // Helps re-assign keys on the display layouts switch matrix. Use ASCII values
	required_device<m6800_cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_nvram;
	required_device<pia6821_device> m_pia_u10;
	required_device<pia6821_device> m_pia_u11;
	required_device<timer_device> m_zero_crossing_freq_timer;
	required_device<timer_device> m_zero_crossing_active_timer;
	required_device<timer_device> m_display_refresh_timer;
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
	output_finder<4> m_spinners;
	output_finder<(5 * 8) + 1> m_scrn_switches;
	void update_lamps(u8 lamp_board_base, u8 lamp_decoder_base, u8 lamp_decoder_addr, u8 lamp_decoder_data);
///	required_device<ptm6840_device> m_sb300_ptm;
	optional_device<s14001a_device> m_s14001a;
};


class meteor_state : public st_mp200_state {
public:
	meteor_state(machine_config const &mconfig, device_type type, char const *tag) : st_mp200_state(mconfig, type, tag, s_solenoid_features_meteor) { }
protected:
	static solenoid_feature_data const s_solenoid_features_meteor;
};

class galaxy_state : public st_mp200_state {
public:
	galaxy_state(machine_config const &mconfig, device_type type, char const *tag) : st_mp200_state(mconfig, type, tag, s_solenoid_features_galaxy) { }
protected:
	static solenoid_feature_data const s_solenoid_features_galaxy;
};

class ali_state : public st_mp200_state {
public:
	ali_state(machine_config const &mconfig, device_type type, char const *tag) : st_mp200_state(mconfig, type, tag, s_solenoid_features_ali) { }
protected:
	static solenoid_feature_data const s_solenoid_features_ali;
};

class flight2k_state : public st_mp200_state {
public:
	flight2k_state(machine_config const &mconfig, device_type type, char const *tag) : st_mp200_state(mconfig, type, tag, s_solenoid_features_flight2k) { }
protected:
	static solenoid_feature_data const s_solenoid_features_flight2k;
};

class seawitch_state : public st_mp200_state {
public:
	seawitch_state(machine_config const &mconfig, device_type type, char const *tag) : st_mp200_state(mconfig, type, tag, s_solenoid_features_seawitch) { }
protected:
	static solenoid_feature_data const s_solenoid_features_seawitch;
};

class nineball_state : public st_mp200_state {
public:
	nineball_state(machine_config const &mconfig, device_type type, char const *tag) : st_mp200_state(mconfig, type, tag, s_solenoid_features_nineball) { }
protected:
	static solenoid_feature_data const s_solenoid_features_nineball;
};
class lightnin_state : public st_mp200_state {
public:
	lightnin_state(machine_config const &mconfig, device_type type, char const *tag) : st_mp200_state(mconfig, type, tag, s_solenoid_features_lightnin) { }
protected:
	static solenoid_feature_data const s_solenoid_features_lightnin;
};

class catacomb_state : public st_mp200_state {
public:
	catacomb_state(machine_config const &mconfig, device_type type, char const *tag) : st_mp200_state(mconfig, type, tag, s_solenoid_features_catacomb) { }
protected:
	static solenoid_feature_data const s_solenoid_features_catacomb;
};



void st_mp200_state::st_mp200_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7fff);     // A15 is not connected
	map(0x0000, 0x007f).mirror(0x6100).ram();
	map(0x0088, 0x008b).mirror(0x6d74).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).mirror(0x6d6c).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
///	map(0x00a0, 0x00a7).rw("sb300_ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));	// SB300 PTM6840 chip
	map(0x00a0, 0x00a7).nopw();
///	map(0x00c0, 0x00c7).w("sb300_effects", FUNC(sb300_device::write));								// SB300 Effects Control
	map(0x00c0, 0x00c7).nopw();
	map(0x0200, 0x02ff).mirror(0x6d00).ram().share("nvram");
	map(0x1000, 0x1fff).mirror(0x2000).rom();
	map(0x5000, 0x5fff).mirror(0x2000).rom();
}


void st_mp200_state::st_mp200_map_ext(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7fff);     // A15 is not connected
	map(0x0000, 0x007f).ram();
	map(0x0088, 0x008b).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
///	map(0x00a0, 0x00a7).rw("sb300_ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));	// SB300 PTM6840 chip
	map(0x00a0, 0x00a7).nopw();
///	map(0x00c0, 0x00c7).w("sb300_effects", FUNC(sb300_device::write));								// SB300 Effects Control
	map(0x00c0, 0x00c7).nopw();
	map(0x0200, 0x0bff).ram().share("nvram");
	map(0x1000, 0x1fff).rom();
	map(0x3000, 0x3fff).rom();
	map(0x5000, 0x5fff).rom();
	map(0x7000, 0x7fff).rom();
}

void st_mp200_state::st_mp200_map_full(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).ram();
	map(0x0088, 0x008b).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
///	map(0x00a0, 0x00a7).rw("sb300_ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));	// SB300 PTM6840 chip
	map(0x00a0, 0x00a7).nopw();
///	map(0x00c0, 0x00c7).w("sb300_effects", FUNC(sb300_device::write));								// SB300 Effects Control
	map(0x00c0, 0x00c7).nopw();
	map(0x0200, 0x0bff).ram().share("nvram");
	map(0x1000, 0xffff).rom();
}



static INPUT_PORTS_START( mp200 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Self Test") PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp200_state, self_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Activity")  PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp200_state, activity_button, 0)

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
//	PORT_DIPSETTING(    0x01, "2 Coins/3 Credits (C1/1C, C2/2C)")
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
//	PORT_DIPSETTING(    0x01, "2 Coins/3 Credits (C1/1C, C2/2C)")
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
	PORT_DIPNAME( 0xc0, 0xc0, "Beating Highest Score Awards")   PORT_DIPLOCATION("S09-S16:!7,!8")
	PORT_DIPSETTING(    0x00, "Novelty")
	PORT_DIPSETTING(    0x40, "1 Credit")
	PORT_DIPSETTING(    0x80, "2 Credits")
	PORT_DIPSETTING(    0xc0, "3 Credits")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x01, "Maximum Credits")        PORT_DIPLOCATION("S17-S24:!1,!2,!3")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "20")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x05, "30")
	PORT_DIPSETTING(    0x06, "35")
	PORT_DIPSETTING(    0x07, "40")
	PORT_DIPNAME( 0x08, 0x08, "Credits Displayed")      PORT_DIPLOCATION("S17-S24:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x10, "Match Feature")          PORT_DIPLOCATION("S17-S24:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 22")               PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 23")               PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 24")               PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 25")               PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 26")               PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x3c, 0x00, "Coin Slot 1")            PORT_DIPLOCATION("S25-S32:!3,!4,!5,!6")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x14, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x24, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x2c, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x30, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0x34, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0x38, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x3c, "1 Coin/14 Credits")
	PORT_DIPNAME( 0xc0, 0xc0, "Award for Special")      PORT_DIPLOCATION("S25-S32:!7,!8")
	PORT_DIPSETTING(    0x00, "None")
	PORT_DIPSETTING(    0x40, "100000 points")
	PORT_DIPSETTING(    0x80, "Extra Ball")
	PORT_DIPSETTING(    0xc0, "Free Game")


	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, outhole, (void *)0x0401)  // PORT_CODE(KEYCODE_BACKSPACE)
//	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END

static INPUT_PORTS_START( mp237 )
	PORT_INCLUDE( mp200 )

	PORT_MODIFY("X4")   /* Trough switches */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0402)  // PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0404)  // PORT_CODE(KEYCODE_U)
INPUT_PORTS_END

static INPUT_PORTS_START( meteor )
	PORT_INCLUDE( mp200 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x80, 0x80, "Missile WOW Lights When")    PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, "Missiles All Match 1000 Thru 6000")
	PORT_DIPSETTING(    0x80, "Missiles All Match 1000 Thru 7000")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x20, "Award for Special / WOW (see below)")    PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, "Off")
	PORT_DIPSETTING(    0x20, "On")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x20, "Special Replay Limit")               PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, "1 Replay per Ball")
	PORT_DIPSETTING(    0x20, "3 Replays per Ball")
	PORT_DIPNAME( 0xc0, 0x40, "METEOR Bank WOW Lights")             PORT_DIPLOCATION("S17-S24:!7,!8")
	PORT_DIPSETTING(    0x00, "Undefined Off, Off")
	PORT_DIPSETTING(    0xc0, "Undefined On, On")
	PORT_DIPSETTING(    0x80, "When Multiplier Equals 7X")
	PORT_DIPSETTING(    0x40, "When Multiplier Equals 6X")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "Slingshot Feature")                  PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x00, "Moves Spot Light Features")
	PORT_DIPSETTING(    0x01, "Doesn't Move Spot Light Features")
	PORT_DIPNAME( 0x02, 0x00, "Spot Meteor")                        PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x00, "Hard")
	PORT_DIPSETTING(    0x02, "Easy")
	PORT_DIPNAME( 0x04, 0x00, "Spot Meteor Lane Feature")           PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x00, "Alternates")
	PORT_DIPSETTING(    0x04, "Stays On")
	PORT_DIPNAME( 0x38, 0x00, "Coin Slot 1")                        PORT_DIPLOCATION("S25-S32:!4,!5,!6")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_5C ))
	PORT_DIPNAME( 0xc0, 0xc0, "Award for Special / WOW")            PORT_DIPLOCATION("S25-S32:!7,!8")
	PORT_DIPSETTING(    0x00, "No Awards")                          PORT_CONDITION("DSW1",0x20,NOTEQUALS,0x20)
	PORT_DIPSETTING(    0x40, "130,000 / 70,000")                   PORT_CONDITION("DSW1",0x20,NOTEQUALS,0x20)
	PORT_DIPSETTING(    0x80, "Extra Ball / 40,000")                PORT_CONDITION("DSW1",0x20,NOTEQUALS,0x20)
	PORT_DIPSETTING(    0xc0, "Free Game / 70,000")                 PORT_CONDITION("DSW1",0x20,NOTEQUALS,0x20)
	PORT_DIPSETTING(    0x00, "90,000 / Extra Ball")                PORT_CONDITION("DSW1",0x20,EQUALS,0x20)
	PORT_DIPSETTING(    0x40, "130,000 / Extra Ball")               PORT_CONDITION("DSW1",0x20,EQUALS,0x20)
	PORT_DIPSETTING(    0x80, "Extra Ball / 70,000")                PORT_CONDITION("DSW1",0x20,EQUALS,0x20)
	PORT_DIPSETTING(    0xc0, "Free Game / Extra Ball")             PORT_CONDITION("DSW1",0x20,EQUALS,0x20)


	PORT_MODIFY("X0")   /* Spinner switch */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, spinner, (void *)0x1010)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X1")   /* Drop Target "3" switches Right Lower */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0101)  // PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0102)  // PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0104)  // PORT_CODE(KEYCODE_COLON)

	PORT_MODIFY("X2")   /* Drop Target "2" switches Left Upper */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0204)  // PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0208)  // PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	/* Drop Target "1" switches Left Lower */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")   /* Drop Target switches */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0340)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)

	PORT_MODIFY("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, outhole, (void *)0x0401)  // PORT_CODE(KEYCODE_BACKSPACE)
INPUT_PORTS_END


static INPUT_PORTS_START( galaxy )
	PORT_INCLUDE( mp200 )

	PORT_MODIFY("X0")   /* Drop Target Red */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	/* Spinner switch */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, spinner, (void *)0x1010)         // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X1")   /* Drop Target 4 bank (4th target on strobe X0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0101)  // PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0102)  // PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0104)  // PORT_CODE(KEYCODE_COLON)

	PORT_MODIFY("X4")
	/* Drop Target Bank Left switches */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0404)  // PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0480)  // PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END


static INPUT_PORTS_START( ali )
	PORT_INCLUDE( mp200 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x80, 0x00, "Eject Pocket")               PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x20, "Background Sound")           PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Attract Mode Sound")         PORT_DIPLOCATION("S17-S24:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x06, 0x00, "Maximum Credits")            PORT_DIPLOCATION("S17-S24:!2,!3")
	PORT_DIPSETTING(    0x00, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x06, "40")
	PORT_DIPNAME( 0x20, 0x00, "Special Replay Limit")               PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, "1 Replay Per Ball")
	PORT_DIPSETTING(    0x20, "Once On, Stays On For Rest Of Game")
	PORT_DIPNAME( 0x40, 0x00, "Special Lights")                     PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "One At A Time (Rotating)")
	PORT_DIPSETTING(    0x40, "All Three On At Once")
	PORT_DIPNAME( 0x80, 0x80, "Special Lights When Completing")     PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "ALI Once, GREATEST Twice")                                                       // 3-Ball and 5-Ball
	PORT_DIPSETTING(    0x80, "ALI Once, GREATEST Once")            PORT_CONDITION("DSW0",0x40,NOTEQUALS,0x40)  // 3-Ball
	PORT_DIPSETTING(    0x80, "ALI Once, GREATEST Twice")           PORT_CONDITION("DSW0",0x40,EQUALS,0x40)     // 5-Ball

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Spotting Letter 'T'")                PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x00, "One 'T' Per Switch")
	PORT_DIPSETTING(    0x01, "Both 'T' In GREATEST")
	PORT_DIPNAME( 0x02, 0x02, "Extra Balls")                PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x00, "Disabled")
	PORT_DIPSETTING(    0x02, "Enabled")


	PORT_MODIFY("X1")   /* Drop Target 4 bank (4th target on strobe X0 */
	/* Spinner switch */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, spinner, (void *)0x1101)         // PORT_CODE(KEYCODE_ENTER)

	PORT_MODIFY("X2")	/* Drop Target Bank Top */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0204)  // PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0208)  // PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	/* Drop Target Bank Middle */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")
	/* Saucers Top 3 switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0340)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)

	PORT_MODIFY("X4")
	/* Saucer Middle Right */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0420)  // PORT_CODE(KEYCODE_E)
INPUT_PORTS_END

static INPUT_PORTS_START( alit )
	PORT_INCLUDE( ali )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x80, 0x00, "Freeplay Mode")              PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, "Disabled")
	PORT_DIPSETTING(    0x80, "Enabled")
INPUT_PORTS_END


static INPUT_PORTS_START( flight2k )
	PORT_INCLUDE( mp200 )

	PORT_MODIFY("X0")   /* Spinner switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, spinner, (void *)0x1008)         // PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, spinner, (void *)0x2010)         // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X1")
	/* Kickers Top switches */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0140)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0180)  // PORT_CODE(KEYCODE_O)

	PORT_MODIFY("X3")
	/* Drop Target Bank Right switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0301)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0302)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
	/* Drop Target Bank 1-2-3-4-5 switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0340)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)

	PORT_MODIFY("X4")   /* Trough switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, outhole, (void *)0x0401)         // PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0402)  // PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0404)  // PORT_CODE(KEYCODE_U)
	/* Ball Launcher switch */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0440)  // PORT_CODE(KEYCODE_W)
INPUT_PORTS_END


static INPUT_PORTS_START( seawitch )
	PORT_INCLUDE( mp200 )

	PORT_MODIFY("X0")   /* Spinner switch */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, spinner, (void *)0x1010)         // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X2")	/* Drop Target Bank Middle switches */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")
	/* Drop Target Bank Top switches */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0340)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)

	PORT_MODIFY("X4")
	/* Drop Target Bank Left switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0420)  // PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0440)  // PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0480)  // PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END


static INPUT_PORTS_START( nineball )
	PORT_INCLUDE( mp200 )

	PORT_MODIFY("X0")   /* Spinner switch */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, spinner, (void *)0x1010)         // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X1")
	/* Drop Target 8-Bank Left switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0101)  // PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0102)  // PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0104)  // PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0108)  // PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0110)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0120)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0140)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0180)  // PORT_CODE(KEYCODE_O)

	PORT_MODIFY("X2")
	/* Drop Target 3-Bank Rear switches */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0202)  // PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0204)  // PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0208)  // PORT_CODE(KEYCODE_B)
	/* Drop Target Loop switch */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	/* Drop Target 3-Bank Right switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X4")   /* Trough switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, outhole, (void *)0x0401)         // PORT_CODE(KEYCODE_BACKSPACE)
	/* Saucer Left switch */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0402)  // PORT_CODE(KEYCODE_I)
   /* Trough switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0408)  // PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0410)  // PORT_CODE(KEYCODE_R)
	/* Left Trough switches above Saucer */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0420)  // PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0440)  // PORT_CODE(KEYCODE_W)
INPUT_PORTS_END


static INPUT_PORTS_START( catacomb )
	PORT_INCLUDE( mp200 )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Slam Tilt")  PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp200_state, reset_switch, 0)

	PORT_MODIFY("X0")   /* Spinner switch */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, spinner, (void *)0x1008)         // PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_CODE(KEYCODE_SPACE)

	PORT_MODIFY("X1")
	/* Drop Target Bank "A" Middle Left switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0120)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0140)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x1, (void *)0x0180)  // PORT_CODE(KEYCODE_O)

	PORT_MODIFY("X2")   /* Outhole Reflex switch */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0201)  // PORT_CODE(KEYCODE_COMMA)
	/* Drop Target Bank "B" Rear Middle switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")
	/* Drop Target Bank "C" Rear Right switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0340)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)

	PORT_MODIFY("X4")   /* Trough switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, outhole, (void *)0x0401)         // PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0402)  // PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0404)  // PORT_CODE(KEYCODE_U)
	/* Saucer Left and Right switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0408)  // PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0410)  // PORT_CODE(KEYCODE_R)
	/* Drop Target Bank "D" Lower Right switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0420)  // PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0440)  // PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0480)  // PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END

static INPUT_PORTS_START( lightnin )
	PORT_INCLUDE( mp200 )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Slam Tilt")  PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp200_state, reset_switch, 0)

	PORT_MODIFY("X0")   /* Outhole Reflex switch */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	/* Spinner switch */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, spinner, (void *)0x1010)         // PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_CODE(KEYCODE_SPACE)

	PORT_MODIFY("X2")   /* Drop Target Bank Rear switches */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0204)  // PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0208)  // PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	/* Drop Target Bank Middle switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X4")   /* Trough switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, outhole, (void *)0x0401)         // PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0402)  // PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0404)  // PORT_CODE(KEYCODE_U)
	/* Saucer Right and Rear switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0408)  // PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0410)  // PORT_CODE(KEYCODE_R)
	/* Drop Target Bank Left switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0420)  // PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0440)  // PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, st_mp200_state, switch_hold_x4, (void *)0x0480)  // PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END


CUSTOM_INPUT_MEMBER( st_mp200_state::spinner)
{
	uint16_t data = (uintptr_t)param & 0xffff;
	uint8_t bit_mask = data & 0xff;
	uint8_t port = (data >> 8) & 0x07;
	uint8_t spinner_num = ((data >> 12) - 1) & 0x03;
	bool state = false;


	switch (data & 0x7ff)
	{
		case 0x008:  if (machine().input().code_pressed(KEYCODE_SLASH))     state = true; break;  // X0:I3  Spinner
		case 0x010:  if (machine().input().code_pressed(KEYCODE_STOP))      state = true; break;  // X0:I4  Spinner
		case 0x101:  if (machine().input().code_pressed(KEYCODE_ENTER))     state = true; break;  // X1:I0  Spinner
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

CUSTOM_INPUT_MEMBER( st_mp200_state::outhole )
{
	uint8_t bit_mask = ((uintptr_t)param & 0xff);
	uint8_t port = (((uintptr_t)param >> 8) & 0x07);

	/* Here we simulate the ball sitting in the Outhole so the Outhole Solenoid can release it */

	if (machine().input().code_pressed_once(KEYCODE_BACKSPACE))  m_io_hold_x[port] |= bit_mask;

	return ((m_io_hold_x[port] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( st_mp200_state::switch_hold_x0 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_SLASH))       m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_STOP))        m_io_hold_x[0] |= bit_mask;

	return ((m_io_hold_x[0] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( st_mp200_state::switch_hold_x1 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x01) && machine().input().code_pressed_once(KEYCODE_ENTER))       m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_QUOTE))       m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_COLON))       m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_L))           m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_BACKSLASH))   m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x20) && machine().input().code_pressed_once(KEYCODE_CLOSEBRACE))  m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x40) && machine().input().code_pressed_once(KEYCODE_OPENBRACE))   m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x80) && machine().input().code_pressed_once(KEYCODE_O))           m_io_hold_x[1] |= bit_mask;

	return ((m_io_hold_x[1] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( st_mp200_state::switch_hold_x2 )
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

CUSTOM_INPUT_MEMBER( st_mp200_state::switch_hold_x3 )
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

CUSTOM_INPUT_MEMBER( st_mp200_state::switch_hold_x4 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_I))  m_io_hold_x[4] |= bit_mask;
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_U))  m_io_hold_x[4] |= bit_mask;
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_Y))  m_io_hold_x[4] |= bit_mask;
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_R))  m_io_hold_x[4] |= bit_mask;
	if ((bit_mask & 0x20) && machine().input().code_pressed_once(KEYCODE_E))  m_io_hold_x[4] |= bit_mask;
	if ((bit_mask & 0x40) && machine().input().code_pressed_once(KEYCODE_W))  m_io_hold_x[4] |= bit_mask;
	if ((bit_mask & 0x80) && machine().input().code_pressed_once(KEYCODE_Q))  m_io_hold_x[4] |= bit_mask;

	return ((m_io_hold_x[4] & bit_mask) ? true : false);
}


INPUT_CHANGED_MEMBER( st_mp200_state::activity_button )
{
	if (newval != oldval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, (newval ? ASSERT_LINE : CLEAR_LINE));
}

INPUT_CHANGED_MEMBER( st_mp200_state::self_test )
{
	m_pia_u10->ca1_w(newval);
}

INPUT_CHANGED_MEMBER( st_mp200_state::reset_switch )
{
	if (newval != oldval)
		m_maincpu->set_input_line(INPUT_LINE_RESET, (newval ? CLEAR_LINE : ASSERT_LINE));
}

READ_LINE_MEMBER( st_mp200_state::u10_ca1_r )
{
	return (m_io_system->read() & 0x01);
}
READ_LINE_MEMBER( st_mp200_state::u10_cb1_r )
{
	return m_u10_cb1;
}

WRITE_LINE_MEMBER( st_mp200_state::u10_ca2_w )
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

		// LOG("Display Player 0 = %02x: %02x written to digit %01x of Player 0 display. Segment is %02x\n", m_display[0][m_digit], patterns[m_segment[0]], m_digit, m_segment[0]);
	}

	m_u10_ca2 = state;
}

WRITE_LINE_MEMBER( st_mp200_state::u10_cb2_w )
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


#if 0
	if (m_s14001a)
	{
		if (m_s14001a->busy_r())
			m_pia_u11->cb1_w(false);
		else
			m_pia_u11->cb1_w(state);
	}

#else

	if (m_s14001a)
	{
		if (state)
		{
//			if (m_u11_cb1 != !m_s14001a->busy_r())
			{
				logerror("S14001A, Speech processor /BUSY line is %01x. U11-CB1 Interrupt was %01x, will now be %01x - Lamp Strobe #1 is %01x\n", !m_s14001a->busy_r(), m_u11_cb1, !m_s14001a->busy_r(), state);  ///
				m_u11_cb1 = !m_s14001a->busy_r();
				m_pia_u11->cb1_w(m_u11_cb1);
			}
		}
		else
		{
			if (m_u11_cb1 != false)
			{
				logerror("S14001A, Speech processor /BUSY line is %01x. U11-CB1 Interrupt was %01x, will now be %01x - Lamp Strobe #1 is %01x\n", !m_s14001a->busy_r(), m_u11_cb1, false, state);  ///
				m_u11_cb1 = false;
				m_pia_u11->cb1_w(m_u11_cb1);
			}
		}

	}
#endif

	m_u10_cb2 = state;
}

WRITE_LINE_MEMBER( st_mp200_state::u11_ca2_w )
{
	// LOG("New U11 CA2 state %01x, was %01x.   U10-PIA-PortA=%02x\n", state, m_u11_ca2, m_u10a);

	output().set_value("led0", state);


#if 0
	if (m_s14001a && state)
	{
		if (BIT(m_u10a, 7))
		{
			logerror("S14001A, Initiating Start command. Sound command from U10 port A is %02x\n", (m_u10a & 0x3f));  ///
			m_s14001a->data_w(generic_space(), 0, m_u10a & 0x3f);
			m_s14001a->start_w(true);
			m_s14001a->start_w(false);
		}
		else if (BIT(m_u10a, 6))
		{
			logerror("S14001A, Setting external clock divisor bits to %01x. Clock is now %08d kHz.  Setting output gain bits to %01x\n", (m_u10a & 0x07), (S14001_CLOCK / (16 - (m_u10a & 0x07)) / 16), ((m_u10a >> 3) & 0x7) );  ///
			m_s14001a->force_update();
			m_s14001a->set_output_gain(0, ((m_u10a >> 3 & 0xF) + 1) / 16.0);   /// changed mask from 0xF to 0x7

			uint8_t clock_divisor = 16 - (m_u10a & 0x07);

			m_s14001a->set_clock(S14001_CLOCK / clock_divisor / 8);
		}
	}

#else

	if (m_s14001a)
	{
		if (state)
		{
			if (BIT(m_u10a, 7) && (m_u11_ca2 != state))
			{
				uint8_t clock_divisor = (m_u10a & 0x07);

				logerror("Lamp Strobe #2 went High. Lamp Bit 6 is High. S14001A: Setting external clock divisor bits to %01x. Clock is now %08d kHz.  Setting output gain bits to %01x.\n", (m_u10a & 0x07), (S14001_CLOCK + (clock_divisor * 1500)), ((m_u10a >> 3) & 0x7) );  ///
				m_s14001a->force_update();
				m_s14001a->set_output_gain(0, ((m_u10a >> 3 & 0x7) + 1));   /// changed mask from 0xF to 0x7 and got rid of divisor

				m_s14001a->set_clock((S14001_CLOCK + (clock_divisor * 1500)));
			}
			if (BIT(m_u10a, 6))
			{
				if (m_u11_ca2 != state)
				{
					logerror("Lamp Strobe #2 went High. S14001A: Initiating Start command. Sound command from U10 port A is %02x\n", (m_u10a & 0x3f));  ///
					m_s14001a->data_w(generic_space(), 0, m_u10a & 0x3f);
					m_s14001a->start_w(true);
				}
				else
					logerror("Lamp Strobe #2 stayed High, Lamp Bit 7 is High\n");  ///
			}
			else
			{
				logerror("Lamp Strobe #2 stayed High, Lamp Bit 7 is Low\n");  ///
		//		m_s14001a->start_w(false);
			}
		}
		else
		{
			if (m_u11_ca2 != state)
			{
				logerror("Lamp Strobe #2 went Low, releasing Start signal\n");  ///
				m_s14001a->start_w(false);
			}
			else
				logerror("Lamp Strobe #2 is Low\n");  ///

	//		m_s14001a->start_w(false);
		}
	}
#endif


	m_u11_ca2 = state;
}

READ_LINE_MEMBER( st_mp200_state::u11_ca1_r )
{
	return m_u11_ca1;
}

READ_LINE_MEMBER( st_mp200_state::u11_cb1_r )
{
	/* Pin 32 on MPU J5 AID connector tied low */
	return m_u11_cb1;
}

WRITE_LINE_MEMBER( st_mp200_state::u11_cb2_w )
{
	m_u11_cb2 = state;
}

READ8_MEMBER( st_mp200_state::u10_a_r )
{
	return m_u10a;
}

WRITE8_MEMBER( st_mp200_state::u10_a_w )
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



	if (m_s14001a)
	{
		if (m_u11_ca2)
		{
			if (BIT(data, 6))
			{
				if (!BIT(m_u10a, 7))
				{
					uint8_t clock_divisor = (m_u10a & 0x07);

					logerror("U10 Port A write %02x, Lamp Strobe #2 is High. Lamp Bit 6 went High. S14001A: Setting external clock divisor bits to %01x. Clock is now %08d kHz.  Setting output gain bits to %01x.\n", data, (m_u10a & 0x07), (S14001_CLOCK + (clock_divisor * 1500)), ((m_u10a >> 3) & 0x7) );  ///
					m_s14001a->force_update();
					m_s14001a->set_output_gain(0, ((m_u10a >> 3 & 0x7) + 1));   /// changed mask from 0xF to 0x7 and got rid of divisor

					m_s14001a->set_clock((S14001_CLOCK + (clock_divisor * 1500)));
				}
			}
			if (BIT(data, 7))
			{
				if (!BIT(m_u10a, 6))
				{
					logerror("U10 Port A write %02x, Lamp Strobe #2 is High. Lamp Bit 7 went High. S14001A: Initiating Start command. Sound command from U10 port A is %02x\n", (m_u10a & 0x3f));  ///
					m_s14001a->data_w(generic_space(), 0, m_u10a & 0x3f);
					m_s14001a->start_w(true);
				}
			}
			else
			{
				logerror("Lamp Bit 7 is Low, Lamp Strobe #2 stayed High\n");  ///
	//			m_s14001a->start_w(false);
			}
		}
		else
		{
			logerror("Lamp Bit 6 and 7 are ???, Lamp Strobe #2 is Low\n");  ///
	//		m_s14001a->start_w(false);
		}
	}

	m_u10a = data;
}

READ8_MEMBER( st_mp200_state::u10_b_r )
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

WRITE8_MEMBER( st_mp200_state::u10_b_w )
{
	m_u10b = data;
}

READ8_MEMBER( st_mp200_state::u11_a_r )
{
	return m_u11a;
}

WRITE8_MEMBER( st_mp200_state::u11_a_w )
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

WRITE8_MEMBER( st_mp200_state::u11_b_w )
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
							if (m_solenoid_features[(data & 0x0f)][2] & 0x40)  // Second upper bit means we want to activate switch(s) on another stobe (troughs)
							{
								if (m_sw_ext)
									m_io_hold_x[(m_sw_ext >> 8) & 0x07] |= (m_sw_ext & 0xff);
							}
						}
						else
						{
							m_io_hold_x[sw_strobe_mask] &= ~(m_solenoid_features[(data & 0x0f)][3]);  // Release a switch
							if (m_solenoid_features[(data & 0x0f)][2] & 0x40)  // Second upper bit means we want to release switch(s) on another stobe (troughs)
							{
								if (m_sw_ext)
									m_io_hold_x[(m_sw_ext >> 8) & 0x07] &= ~(m_sw_ext & 0xff);
							}
						}
					}
				}
			}

			// G.I. Flasher
			if (m_gi_flasher_sol != 255)
			{
				if ((data & 0x0f) == m_gi_flasher_sol)			// Check if the game specific Solenoid driven control of the G.I. Flasher Relay is active
				{
					output().set_value("Relay0", 1);			// Activate the G.I. power Relay
					output().set_value("GI_Lamps", 0);			// Relay On (N.C) switches the G.I. lamps Off
				}
			}
		}
		else                        // Rest output - all momentary solenoids are off
		{
			if (m_gi_flasher_sol < 15)
			{
				output().set_value("Relay0", 0);				// Release the G.I. power Relay
				output().set_value("GI_Lamps", 1);				// Relay Off (N.C) switches the G.I. lamps On
			}
			std::fill_n(std::begin(m_solenoids) + 00, 15, false);
		}
	}


	if ((m_u11b & 0x10) && ((data & 0x10) == 0))
	{
		m_solenoids[16] = 1;
		if (m_solenoid_features[16][0] != 0xff)
			m_samples->start(m_solenoid_features[16][0], m_solenoid_features[16][1]);
		if (m_solenoid_features[16][3])  // Release/Activate relevant switch after firing Solenoid
		{
			for (int sw_strobe_mask = 0; sw_strobe_mask <= 5; sw_strobe_mask++)
			{
				if (m_solenoid_features[16][2] & (1 << sw_strobe_mask) )
				{
					if (m_solenoid_features[16][2] & 0x80)  // Upper bit means we want to activate a switch (drop a target)
					{
						m_io_hold_x[sw_strobe_mask] |= (m_solenoid_features[16][3]);   // Activate a switch
						if (m_solenoid_features[(data & 0x0f)][2] & 0x40)  // Second upper bit means we want to activate switch(s) on another stobe (troughs)
						{
							if (m_sw_ext)
								m_io_hold_x[(m_sw_ext >> 8) & 0x07] |= (m_sw_ext & 0xff);
						}
					}
					else
					{
						m_io_hold_x[sw_strobe_mask] &= ~(m_solenoid_features[16][3]);  // Release a switch
						if (m_solenoid_features[(data & 0x0f)][2] & 0x40)  // Second upper bit means we want to release switch(s) on another stobe (troughs)
						{
							if (m_sw_ext)
								m_io_hold_x[(m_sw_ext >> 8) & 0x07] &= ~(m_sw_ext & 0xff);
						}
					}
				}
			}
		}
		if (m_gi_flasher_sol == 16)						// Check if the game specific Solenoid driven control of the G.I. Flasher Relay is active
		{
			output().set_value("Relay0", 1);			// Activate the G.I. power Relay
			output().set_value("GI_Lamps", 0);			// Relay On (N.C) switches the G.I. lamps Off
		}
	}
	else if ((data & 0x10) && ((m_u11b & 0x10) == 0))
	{
		m_solenoids[16] = 0;
		if ((m_solenoid_features[16][0] != 0xff) && (m_solenoid_features[16][3] == 0))
			m_samples->start(m_solenoid_features[16][0], m_solenoid_features[16][2]);
		if (m_gi_flasher_sol == 16)						// Check if the game specific Solenoid driven control of the G.I. Flasher Relay is active
		{
			output().set_value("Relay0", 0);			// Release the G.I. power Relay
			output().set_value("GI_Lamps", 1);			// Relay Off (N.C) switches the G.I. lamps On
		}
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
		if (m_solenoid_features[19][3])  // Release/Activate relevant switch after firing Solenoid
		{
			for (int sw_strobe_mask = 0; sw_strobe_mask <= 5; sw_strobe_mask++)
			{
				if (m_solenoid_features[19][2] & (1 << sw_strobe_mask) )
				{
					if (m_solenoid_features[19][2] & 0x80)  // Upper bit means we want to activate a switch (drop a target)
					{
						m_io_hold_x[sw_strobe_mask] |= (m_solenoid_features[19][3]);   // Activate a switch
						if (m_solenoid_features[(data & 0x0f)][2] & 0x40)  // Second upper bit means we want to activate switch(s) on another stobe (troughs)
						{
							if (m_sw_ext)
								m_io_hold_x[(m_sw_ext >> 8) & 0x07] |= (m_sw_ext & 0xff);
						}
					}
					else
					{
						m_io_hold_x[sw_strobe_mask] &= ~(m_solenoid_features[19][3]);  // Release a switch
						if (m_solenoid_features[(data & 0x0f)][2] & 0x40)  // Second upper bit means we want to release switch(s) on another stobe (troughs)
						{
							if (m_sw_ext)
								m_io_hold_x[(m_sw_ext >> 8) & 0x07] &= ~(m_sw_ext & 0xff);
						}
					}
				}
			}
		}
	}
	else if ((data & 0x80) && ((m_u11b & 0x80) == 0))
	{
		m_solenoids[19] = 0;
		if ((m_solenoid_features[19][0] != 0xff) && (m_solenoid_features[19][3] == 0))
			m_samples->start(m_solenoid_features[19][0], m_solenoid_features[19][2]);
	}

	m_u11b = data;
}


void st_mp200_state::update_lamps(u8 lamp_board_base, u8 lamp_decoder_base, u8 lamp_decoder_addr, u8 lamp_decoder_data)
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


TIMER_DEVICE_CALLBACK_MEMBER( st_mp200_state::timer_z_freq )
{
/*  Zero Crossing Detector - this timing is based on 50Hz AC line power input rectified to unregulated DC

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
TIMER_DEVICE_CALLBACK_MEMBER( st_mp200_state::timer_z_pulse )
{
	/*** Line Power to DC Zero Crossing has ended ***/

	m_u10_cb1 = false;
	m_pia_u10->cb1_w(m_u10_cb1);
}

TIMER_DEVICE_CALLBACK_MEMBER( st_mp200_state::u11_timer )
{
/*   555 timer for display interrupt generator

     +--------------------------+   +-----
     |                          |   |
     |<-------- 2.25ms -------->|<->|250us
     |                          |   |
    -+                          +---+
*/

	m_display_refresh_timer->adjust(attotime::from_usec(2250));

	m_u11_ca1 = true;
	m_pia_u11->ca1_w(m_u11_ca1);
}

TIMER_DEVICE_CALLBACK_MEMBER( st_mp200_state::timer_d_pulse )
{
	m_u11_ca1 = false;
	m_pia_u11->ca1_w(m_u11_ca1);
}

#if 0 ///
WRITE_LINE_MEMBER(st_mp200_state::ptm_o1_callback)
{
	// m_speaker->level_w(state);
}
WRITE_LINE_MEMBER(st_mp200_state::ptm_o2_callback)
{
	// m_speaker->level_w(state);
}
WRITE_LINE_MEMBER(st_mp200_state::ptm_o3_callback)
{
	// m_speaker->level_w(state);
}
#endif



st_mp200_state::solenoid_feature_data const st_mp200_state::s_solenoid_features_default =
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
	// 16-19 { SC, AS, SSB, SRB } Constant Coils with some manual Momentary Coils requiring switch manipulation
	//                      SRB = Switch Return Bits used to release/assert a switch depending on game specific usage. Bit is active Hi
	//                 SSB      = Switch Strobe Bits that the Return Bits lives within the switch matrix
	//                 SSB        Bit 0x80 Hi indicates we want to activate a switch, Lo indicates we want to release a switch
	//             AS           = Audio Sample (mechanical and/or chime) to playback
	//         SC               = MAME Sound Channel to play the audio sample on (make sure samples that might play at the same time are on different channels)
	//         SC                 Value 0xff indicates not to play any audio sample
	//
	//      {  SC,   AS,    SSB,  SRB }
	/*00*/  { 0x02, 0x00,  0x00, 0x00 },
	/*01*/  { 0x02, 0x00,  0x00, 0x00 },
	/*02*/  { 0x02, 0x00,  0x00, 0x00 },
	/*03*/  { 0x02, 0x00,  0x00, 0x00 },
	/*04*/  { 0x02, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x02, 0x00,  0x10, 0x07 },     // Outhole		Split Second Reflex releases switch 04
	/*07*/  { 0x02, 0x00,  0x10, 0x07 },     // Outhole Split Second
	/*08*/  { 0x02, 0x00,  0x04, 0x01 },     // Outhole Reflex Catacomb;  Frefall Outhole
	/*09*/  { 0x02, 0x00,  0x10, 0x07 },     // Outhole Catacomb
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },
	/*12*/  { 0x02, 0x00,  0x01, 0x80 },     // Outhole Reflex Freefall
	/*13*/  { 0x02, 0x00,  0x00, 0x00 },
	/*14*/  { 0x02, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//      {  SC,   SE,   SR,    N/U }
	/*16*/  { 0xff, 0x00, 0x00, 0x00  },
	/*17*/  { 0x00, 0x0c, 0x0d, 0x00  },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f, 0x00  },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00, 0x00  }
};


st_mp200_state::solenoid_feature_data const meteor_state::s_solenoid_features_meteor =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first solenoid table above
	/*00*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*01*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Middle
	/*02*/  { 0x03, 0x0b,  0x04, 0x1c },     // Drop Target Bank Reset "2" Upper Left
	/*03*/  { 0x05, 0x0c,  0x88, 0x80 },     // Drop Target M
	/*04*/  { 0x05, 0x0c,  0x88, 0x40 },     // Drop Target E
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x03, 0x0b,  0x04, 0xe0 },     // Drop Target Bank Reset "1" Lower Left
	/*07*/  { 0x05, 0x0c,  0x88, 0x20 },     // Drop Target T
	/*08*/  { 0x03, 0x0b,  0x02, 0x07 },     // Drop Target Bank Reset "3" Lower Right
	/*09*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*10*/  { 0x05, 0x0c,  0x88, 0x10 },     // Drop Target E
	/*11*/  { 0x05, 0x0c,  0x88, 0x08 },     // Drop Target O
	/*12*/  { 0x03, 0x0b,  0x08, 0xfc },     // Drop Target Bank Reset "METEOR" Upper
	/*13*/  { 0x05, 0x0c,  0x88, 0x04 },     // Drop Target R
	/*14*/  { 0x01, 0x09,  0x10, 0x01 },     // Outhole
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00, 0x00  },
	/*17*/  { 0x00, 0x0c, 0x0d, 0x00  },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f, 0x00  },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00, 0x00  }
};

st_mp200_state::solenoid_feature_data const galaxy_state::s_solenoid_features_galaxy =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first solenoid table above
	/*00*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*01*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*02*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top
	/*03*/  { 0x03, 0x0b,  0x42, 0x07 },     // Drop Target Bank Reset
	/*04*/  { 0x02, 0x0a,  0x10, 0x04 },     // Kickback
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Lower
	/*07*/  { 0x02, 0x05,  0x10, 0x80 },     // Saucer
	/*08*/  { 0x05, 0x0c,  0x81, 0x08 },     // Drop Target Red
	/*09*/  { 0x05, 0x0c,  0x82, 0x04 },     // Drop Target Blue
	/*10*/  { 0x01, 0x09,  0x10, 0x01 },     // Outhole
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Lower Left
	/*12*/  { 0x05, 0x0c,  0x82, 0x02 },     // Drop Target Yellow
	/*13*/  { 0x05, 0x0c,  0x82, 0x01 },     // Drop Target Black
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0c, 0x0d, 0x00  },     // G.I. relay
	/*17*/  { 0x00, 0x0c, 0x0d, 0x00  },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f, 0x00  },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00, 0x00  }
};

st_mp200_state::solenoid_feature_data const ali_state::s_solenoid_features_ali =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first solenoid table above
	/*00*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*01*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*02*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*03*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Middle
	/*04*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x02, 0x05,  0x08, 0xe0 },     // Saucer Top x 3
	/*07*/  { 0x03, 0x0b,  0x04, 0x1c },     // Drop Target Bank Reset Top
	/*08*/  { 0x03, 0x0b,  0x04, 0xe0 },     // Drop Target Bank Reset Left
	/*09*/  { 0x02, 0x05,  0x10, 0x20 },     // Saucer Middle Right
	/*10*/  { 0x01, 0x09,  0x10, 0x01 },     // Outhole
	/*11*/  { 0xff, 0x00,  0x00, 0x00 },
	/*12*/  { 0xff, 0x00,  0x00, 0x00 },
	/*13*/  { 0x00, 0x0c,  0x00, 0x00 },     // G.I. relay
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00, 0x00  },
	/*17*/  { 0x00, 0x0c, 0x0d, 0x00  },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f, 0x00  },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00, 0x00  }
};

st_mp200_state::solenoid_feature_data const flight2k_state::s_solenoid_features_flight2k =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first solenoid table above
	/*00*/  { 0x02, 0x00,  0x00, 0x00 },     // Slingshot Top Left
	/*01*/  { 0x02, 0x00,  0x00, 0x00 },     // Slingshot Top Right
	/*02*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Bottom Left
	/*03*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Bottom Right
	/*04*/  { 0x02, 0x07,  0x00, 0x00 },     // Pop Bumper Top
	/*05*/  { 0x02, 0x07,  0x00, 0x00 },     // Pop Bumper Bottom
	/*06*/  { 0x03, 0x0b,  0x08, 0xf8 },     // Drop Target Bank Reset "1-2-3-4-5"
	/*07*/  { 0x05, 0x0c,  0x88, 0x80 },     // Drop Target "5"
	/*08*/  { 0x02, 0x0a,  0x10, 0x40 },     // Ball Launcher
	/*09*/  { 0x01, 0x09,  0x10, 0x07 },     // Outhole
	/*10*/  { 0x05, 0x0c,  0x88, 0x40 },     // Drop Target "4"
	/*11*/  { 0x05, 0x0c,  0x88, 0x20 },     // Drop Target "3"
	/*12*/  { 0x05, 0x0c,  0x88, 0x08 },     // Drop Target "1"
	/*13*/  { 0x05, 0x0c,  0x88, 0x10 },     // Drop Target "2"
	/*14*/  { 0x03, 0x0b,  0x08, 0x07 },     // Drop Target Bank Reset Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x02, 0x05,  0x02, 0x80 },     // Kicker Top Right
	/*17*/  { 0x00, 0x0c, 0x0d, 0x00  },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f, 0x00  },     // Flipper Enable relay
	/*19*/  { 0x02, 0x05,  0x02, 0x40 },     // Kicker Top Left
};

st_mp200_state::solenoid_feature_data const seawitch_state::s_solenoid_features_seawitch =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first solenoid table above
	/*00*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top
	/*01*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*02*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Top
	/*03*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*04*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x03, 0x0b,  0x04, 0xf0 },     // Drop Target Bank Reset Middle
	/*07*/  { 0x03, 0x0b,  0x10, 0xe0 },     // Drop Target Bank Reset Left
	/*08*/  { 0x03, 0x0b,  0x08, 0xf0 },     // Drop Target Bank Reset Top
	/*09*/  { 0x01, 0x09,  0x10, 0x01 },     // Outhole
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*11*/  { 0xff, 0x00,  0x00, 0x00 },
	/*12*/  { 0xff, 0x00,  0x00, 0x00 },
	/*13*/  { 0xff, 0x00,  0x00, 0x00 },
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 },
};

st_mp200_state::solenoid_feature_data const nineball_state::s_solenoid_features_nineball =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first solenoid table above
	/*00*/  { 0x02, 0x00,  0x00, 0x00 },     // Slingshot Top Right
	/*01*/  { 0x02, 0x00,  0x00, 0x00 },     // Slingshot Top Left
	/*02*/  { 0x03, 0x0b,  0x04, 0xe0 },     // Drop Target 3-Bank Reset Right
	/*03*/  { 0x03, 0x0b,  0x04, 0x0e },     // Drop Target 3-Bank Reset Rear
	/*04*/  { 0x02, 0x07,  0x00, 0x00 },     // Pop Bumper Top
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x10, 0x19 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x10, 0x62 },     // Saucer Left
	/*08*/  { 0x05, 0x0c,  0x82, 0x40 },     // Drop Target "7"
	/*09*/  { 0x05, 0x0c,  0x82, 0x20 },     // Drop Target "6"
	/*10*/  { 0x05, 0x0c,  0x82, 0x10 },     // Drop Target "5"
	/*11*/  { 0x05, 0x0c,  0x82, 0x08 },     // Drop Target "4"
	/*12*/  { 0x05, 0x0c,  0x82, 0x04 },     // Drop Target "3"
	/*13*/  { 0x05, 0x0c,  0x82, 0x02 },     // Drop Target "2"
	/*14*/  { 0x05, 0x0c,  0x82, 0x01 },     // Drop Target "1"
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x02, 0x05,  0x02, 0xff },     // Drop Target 8-Bank Reset Left
	/*17*/  { 0x00, 0x0c, 0x0d, 0x00  },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f, 0x00  },     // Flipper Enable relay
	/*19*/  { 0x02, 0x05,  0x04, 0x10 },     // Drop Target Reset Loop
};

st_mp200_state::solenoid_feature_data const catacomb_state::s_solenoid_features_catacomb =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first solenoid table above
	/*00*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top
	/*01*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*02*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Top
	/*03*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*04*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x02, 0x05,  0x10, 0x08 },     // Saucer Left
	/*07*/  { 0x02, 0x05,  0x10, 0x10 },     // Saucer Right
	/*08*/  { 0x01, 0x09,  0x04, 0x01 },     // Outhole Reflex
	/*09*/  { 0x02, 0x11,  0x10, 0x07 },     // Outhole Trough Ball Release
	/*10*/  { 0x03, 0x0b,  0x02, 0xe0 },     // Drop Target Bank Reset "A"
	/*11*/  { 0x03, 0x0b,  0x04, 0xe0 },     // Drop Target Bank Reset "B"
	/*12*/  { 0x03, 0x0b,  0x08, 0xe0 },     // Drop Target Bank Reset "C"
	/*13*/  { 0x03, 0x0b,  0x10, 0xe0 },     // Drop Target Bank Reset "D"
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper Enable relay
	/*19*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Backbox Playfield Relay
};

st_mp200_state::solenoid_feature_data const lightnin_state::s_solenoid_features_lightnin =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first solenoid table above
	/*00*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Top Right
	/*01*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Bottom Right
	/*02*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Top Left
	/*03*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Bottom Left
	/*04*/  { 0x03, 0x0b,  0x04, 0xe0 },     // Drop Target Bank Reset Rear
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x08 },     // Outhole Reflex
	/*07*/  { 0x02, 0x11,  0x10, 0x07 },     // Outhole Trough Ball Release
	/*08*/  { 0x03, 0x0b,  0x04, 0x1c },     // Drop Target Bank Reset Middle
	/*09*/  { 0x03, 0x0b,  0x10, 0xe0 },     // Drop Target Bank Reset Left
	/*10*/  { 0x02, 0x05,  0x10, 0x08 },     // Saucer Left
	/*11*/  { 0x02, 0x05,  0x10, 0x10 },     // Saucer Right
	/*12*/  { 0xff, 0x00,  0x00, 0x00 },
	/*13*/  { 0xff, 0x00,  0x00, 0x00 },
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00, 0x00  },
	/*17*/  { 0x00, 0x0c, 0x0d, 0x00  },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f, 0x00  },     // Flipper Enable relay
	/*19*/  { 0xff, 0x00, 0x00, 0x00  },     // Bonus Display...
};



void st_mp200_state::machine_start()
{
	genpin_class::machine_start();

	m_lamps.resolve();
	m_display.resolve();
	m_solenoids.resolve();
	m_spinners.resolve();
	m_scrn_switches.resolve();
}

void st_mp200_state::machine_reset()
{
	genpin_class::machine_reset();

	render_target *target = machine().render().first_target();

	target->set_view(0);

	m_u10a = 0xff;
	m_u10b = 0;
	m_u11a = 0;
	m_u11b = 0;
//	m_u10_cb2 = true;
//	m_u11_ca2 = true;
//	m_u11_cb1 = false;
	m_lamp_decode = 0x0f;
	m_io_hold_x[0] = m_io_hold_x[1] = m_io_hold_x[2] = m_io_hold_x[3] = m_io_hold_x[4] = 0;   // Clear the temp switch holds

	if (m_io_outhole->read())           // Put a ball in the Outhole at power-on if selected in the machine configuration
		m_io_hold_x[(m_outhole >> 8)& 7] = m_outhole & 0xff;

	if (m_io_zx->read())                // Machine configuration sets the Zero Crossing frequency based countries DC rectified mains line frequency
		m_zero_crossing_freq_timer->adjust(attotime::from_hz(120), 0, attotime::from_hz(120));  // Country whose mains line frequency is 60Hz
	else
		m_zero_crossing_freq_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));  // Country whose mains line frequency is 50Hz


	for (int i = 1; i<=40; i++)
	{
		if ((m_disp_key[i] > 0) && (m_disp_key[i] <128))	// Change the switch key characters in the default layout where games need require customisations
			m_scrn_switches[i] = m_disp_key[i];
	}
///	output().set_value("switch12", 8);
///	m_scrn_switches[13] = 65;
}



MACHINE_CONFIG_START( st_mp200_state::st_mp200 )
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M6800, 1000000)  // No xtal, just 2 chips forming a multivibrator oscillator
	MCFG_DEVICE_PROGRAM_MAP(st_mp200_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_st_mp200)

	/* Sound */
	genpin_audio(config);

	/* Devices */
	MCFG_DEVICE_ADD("pia_u10", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, st_mp200_state, u10_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, st_mp200_state, u10_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(*this, st_mp200_state, u10_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, st_mp200_state, u10_b_w))
	MCFG_PIA_READCA1_HANDLER(READLINE(*this, st_mp200_state, u10_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(*this, st_mp200_state, u10_cb1_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(*this, st_mp200_state, u10_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, st_mp200_state, u10_cb2_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_TIMER_DRIVER_ADD("timer_z_freq", st_mp200_state, timer_z_freq)						// Mains Line Frequency * 2 (100Hz or 120Hz depending on country)
	MCFG_TIMER_DRIVER_ADD(m_zero_crossing_active_timer, st_mp200_state, timer_z_pulse)		// Active pulse length from Zero Crossing detector

	MCFG_DEVICE_ADD("pia_u11", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, st_mp200_state, u11_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, st_mp200_state, u11_a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, st_mp200_state, u11_b_w))
	MCFG_PIA_READCA1_HANDLER(READLINE(*this, st_mp200_state, u11_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(*this, st_mp200_state, u11_cb1_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(*this, st_mp200_state, u11_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, st_mp200_state, u11_cb2_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_d_freq", st_mp200_state, u11_timer, PERIOD_OF_555_ASTABLE(27000, 4700, 0.0000001))	// 555 Timer 400Hz
	MCFG_TIMER_DRIVER_ADD(m_display_refresh_timer, st_mp200_state, timer_d_pulse)												// 555 Active pulse length

///	MCFG_DEVICE_ADD("sb300_ptm", PTM6840, 1000000)
///	MCFG_PTM6840_EXTERNAL_CLOCKS(700000, 700000, 700000)
///	MCFG_PTM6840_OUT0_CB(WRITELINE(st_mp200_state, ptm_o1_callback))
///	MCFG_PTM6840_OUT1_CB(WRITELINE(st_mp200_state, ptm_o2_callback))
///	MCFG_PTM6840_OUT2_CB(WRITELINE(st_mp200_state, ptm_o3_callback))
//	MCFG_PTM6840_IRQ_CB(INPUTLINE("maincpu", M6800_IRQ_LINE))
MACHINE_CONFIG_END


MACHINE_CONFIG_START( st_mp200_state::st_mp200_ext )
	st_mp200(config);

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(st_mp200_map_ext)
MACHINE_CONFIG_END


MACHINE_CONFIG_START( st_mp200_state::st_mp200_full )
	st_mp200(config);

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(st_mp200_map_full)
MACHINE_CONFIG_END


MACHINE_CONFIG_START( st_mp200_state::st_mp200_spch )
	st_mp200(config);

	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("speech", S14001A, S14001_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


/*--------------------------------
/ Meteor #113
/-------------------------------*/
ROM_START(meteor)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "25AROM_P21A.U1", 0x1000, 0x0800, CRC(9ee33909) SHA1(5f58e4e72af47047c8f060f98706ed9607720705))
	ROM_LOAD( "25AROM_P23.U5",  0x1800, 0x0800, CRC(43a46997) SHA1(2c74ca10cf9091db10542960f499f39f3da277ee))
	ROM_LOAD( "25AROM_P22.U2",  0x5000, 0x0800, CRC(fd396792) SHA1(b5d051a7ce7e7c2f9c4a0d900cef4f9ef2089476))
	ROM_LOAD( "25AROM_P24.U6",  0x5800, 0x0800, CRC(03fa346c) SHA1(51c04123cb433e90920c241e2d1f89db4643427b))
//	ROM_RELOAD( 0x7800, 0x0800)
ROM_END

ROM_START(meteoro)   // Original release has the Bonus countdown bug
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "25AROM_P21.U1",  0x1000, 0x0800, CRC(e0fd8452) SHA1(a13215378a678e26a565742d81fdadd2e161ba7a))   // PROM 9316A-2923
	ROM_LOAD( "25AROM_P23.U5",  0x1800, 0x0800, CRC(43a46997) SHA1(2c74ca10cf9091db10542960f499f39f3da277ee))   // PROM 9316A-2924
	ROM_LOAD( "25AROM_P22.U2",  0x5000, 0x0800, CRC(fd396792) SHA1(b5d051a7ce7e7c2f9c4a0d900cef4f9ef2089476))   // PROM 9316A-2925
	ROM_LOAD( "25AROM_P24.U6",  0x5800, 0x0800, CRC(03fa346c) SHA1(51c04123cb433e90920c241e2d1f89db4643427b))   // PROM 9316A-2926
ROM_END

ROM_START(meteors)   // Original release has the Bonus countdown bug
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "meteor65-u1.716",  0x1000, 0x0800, CRC(9ccd236f) SHA1(4261ab6188aa3534459f2818100de19a8c64657f))
	ROM_LOAD( "meteor65-u5.716",  0x1800, 0x0800, CRC(d79fa4ee) SHA1(968ced25ae1cb823dedb004211281e6a0e8f178e))
	ROM_LOAD( "meteor65-u2.716",  0x5000, 0x0800, CRC(2efca800) SHA1(bb1810d37904fff1226028e70eac45ad9e967dc7))
	ROM_LOAD( "meteor65-u6.716",  0x5800, 0x0800, CRC(ccca52d5) SHA1(fbf74dba227517531dac9a3deb2bc44a295697e9))
ROM_END

ROM_START(meteorl)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "meteor128k.bin", 0x1000, 0x0800, CRC(12b95808) SHA1(02564a37c111b00f336aeb34a19ed9ead0c1b2a2))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_CONTINUE( 0x1800, 0x0800)
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_CONTINUE( 0x3000, 0x0800)
	ROM_CONTINUE( 0x7000, 0x0800)
	ROM_CONTINUE( 0x3800, 0x0800)
	ROM_CONTINUE( 0x7800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0xfff8,0x0008)
ROM_END

ROM_START(disptest)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "disptest.bin", 0x5800, 0x0800, CRC(00000000))
ROM_END

/*--------------------------------
/ Galaxy #114
/-------------------------------*/
ROM_START(galaxy)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "25AROM_P25.U1", 0x1000, 0x0800, CRC(35656b67) SHA1(e1ad9456c561d19220f8607576cb505588512179))   // PROM 9316A-2927
	ROM_LOAD( "25AROM_P27.U5", 0x1800, 0x0800, CRC(12be0601) SHA1(d651b834348c071dda660f37b4e359bf01cbd8d3))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(08bdb285) SHA1(7984835ac151e5dac05628f3d5146d20e3623c38))      // Either PROM 9316A-2930 or PROM 9316A-2??? , 25AROM_P26 or 25AROM_P26B respectively
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(ad846a42) SHA1(303c9cb933ca60d35e12793a4ac0cf7ef11bc92e))      // Either PROM 9316A-???? or PROM 9316A-2A20 , 25AROM_P28 or 25AROM_P28B respectively
ROM_END

ROM_START(galaxyb)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1b.716", 0x1000, 0x0800, CRC(53f7c0c9) SHA1(c3ee8bbdd1eca7a044c7abf4e0ba6059f523c323))
	ROM_LOAD( "cpu_u5b.716", 0x1800, 0x0800, CRC(1b1cd31b) SHA1(65a6a58d2c509419fce3142a9ae88d8ea7d25f1c))
	ROM_LOAD( "cpu_u2b.716", 0x5000, 0x0800, CRC(f0b4e60b) SHA1(e1628ec94585fbf4935e824721472cc9c91bbf89))
	ROM_LOAD( "cpu_u6b.716", 0x5800, 0x0800, CRC(be4eacc1) SHA1(3d95e8e859312ef0a7ed52356dabe35ed0bebdef))
ROM_END

ROM_START(galaxyfp)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "25AROM_P25.U1", 0x1000, 0x0800, CRC(35656b67) SHA1(e1ad9456c561d19220f8607576cb505588512179))   // PROM 9316A-2927
	ROM_LOAD( "25AROM_P27.U5", 0x1800, 0x0800, CRC(12be0601) SHA1(d651b834348c071dda660f37b4e359bf01cbd8d3))
	ROM_LOAD( "fpgal_u2.716", 0x5000, 0x0800, CRC(c41ce9e8) SHA1(15e344cb72ce80fa5baa0136f2ca461b1d7d39ac))
	ROM_LOAD( "fpgal_u6.716", 0x5800, 0x0800, CRC(a5ab7492) SHA1(b7c2635ab721d50f87e87cbf779cab7c787be222))
ROM_END

ROM_START(galaxyt)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "galaxy_test_u2.732", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "galaxy_test_u6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
ROM_END


/*--------------------------------
/ Cheetah #116
/-------------------------------*/
ROM_START(cheetah)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(6a845d94) SHA1(c272d5895edf2270f5f06fc33345bb4911abbee4))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(e7bdbe6c) SHA1(8b213c2271dbd5157e0d34a33672130b935d76be))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(a827a1a1) SHA1(723ebf193b5ce7b19df70e83caa9bb80f2e3fa66))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(ed33c227) SHA1(a96ba2814cef7663728bb5fdea2dc6ecfa219038))
ROM_END

ROM_START(cheetahb)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cheetah.u1", 0x1000, 0x0800, CRC(2f736A0A) SHA1(e0dc14215d90145881ac1b407fbe057770696122))
	ROM_LOAD( "cheetah.u5", 0x1800, 0x0800, CRC(168f0650) SHA1(5b3294bf64f06cc9d193bb14891b2acfbb5c06d4))
	ROM_LOAD( "cheetah.u2", 0x5000, 0x0800, CRC(f6bd41bc) SHA1(ac94f4ba17c31dfe10ab7efab63d98aa3401e4ae))
	ROM_LOAD( "cheetah.u6", 0x5800, 0x0800, CRC(c7eba210) SHA1(ced377e53f30b371e74c26527e5f8bebcc10ee59))
ROM_END

/*--------------------------------
/ Quicksilver #117
/-------------------------------*/
ROM_START(quicksil)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(fc1bd20a) SHA1(e3c547f996dfc5d1567223d234443cf31d648ef6))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(0bcaceb4) SHA1(461d2fe5772a5ac84d31a4a186b9f639c683ca8a))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(8cb01165) SHA1(b42e2ccce2c20ad570cdcdb63c9d12e414f9b255))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(8c0e336a) SHA1(8d3a5b7c07d03c7e2945ea60c72f9181d3ee2a14))
ROM_END

/*--------------------------------
/ Ali #119
/-------------------------------*/
ROM_START(ali)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(92e75b40) SHA1(bace68db0ea12d50a546157d11084f3b00949136))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(119a4300) SHA1(e913d9bd399b90502efe110c8bf7f23ae07df276))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(9c91d08f) SHA1(a3e8c8e8c2c8b03d86b36eea8c84e5c0a27b8444))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(7629db56) SHA1(f922d31ec4dd1755da0a24bec4e3fa3a7a9b22fc))
ROM_END

ROM_START(alit)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "ali_test_u2.732", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "ali_test_u6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
ROM_END

/*--------------------------------
/ Big Game #121
/-------------------------------*/
ROM_START(biggame)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(f59c7514) SHA1(49ab034a21e70956f63327aec4cbae115cd66a66))   // BIGGAME__V_A30.U1
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(57df1dc5) SHA1(283f45879b76d56ba0db0fb3d9d9771f91a70d02))   // BIGGAME__V_A30.U5
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(0251039b) SHA1(0a0e662788cf012dfb773d200c542a2a363748a8))   // BIGGAME__V_A30.U2
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(801e9a66) SHA1(8634d6bd4af3e5ec3b736679393462961b76ede1))   // BIGGAME__V_A30.U6
ROM_END

/*--------------------------------
/ Seawitch #123
/-------------------------------*/
ROM_START(seawitch)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(c214140b) SHA1(4d68ddd3b0f051c5f601ea5b9d5d5195d6017304))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(ab2eab3a) SHA1(80a8c1ccd554be279720a26466bd6c59e1e56df0))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(b8844174) SHA1(6e01321196fd6fce7b5526efc402044c87fe96a6))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(6c296d8f) SHA1(8cdb77f382ef1214ef45579213cf8f19141366ad))
ROM_END

ROM_START(seawitchs)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "seawitch_v08-u2.732", 0x1000, 0x0800, CRC(db20c725) SHA1(69d94ef366335b9587f41c52d24c2fe839d4bcca))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "seawitch_v08-u6.732", 0x1800, 0x0800, CRC(ee759465) SHA1(96e4fdc7b037da0a919254920f6f73dbf740ae77))
	ROM_CONTINUE( 0x5800, 0x0800)
ROM_END


/*--------------------------------
/ Nine Ball #125
/-------------------------------*/
ROM_START(nineball)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(fcb58f97) SHA1(6510a6d0b466bd27ade50992260cea716d79fda2))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(c7c62161) SHA1(624eab2fdf7bafbf4af012df521bd09f9b2da8d8))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(bdd7f258) SHA1(2a38de09827100cbbd4e79be50aad03a3f2b63b4))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(7e831499) SHA1(8d3c148b91c21938b1b5fca85ecd8f6d7f1e76b0))
ROM_END

ROM_START(ninebalb)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "nineball.256", 0x0000, 0x8000, CRC(06cb8a63) SHA1(c901bba0b41b45c5cfa6d04181f1e035beab5a08))
	ROM_COPY("maincpu", 0x7ff8, 0xfff8,0x0008)
ROM_END

ROM_START(ninebaln)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "9ball.512", 0x0000, 0x10000, CRC(00000000))
ROM_END

ROM_START(ninebalf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "9ball_full.512", 0x0000, 0x10000, CRC(00000000))
ROM_END

/*--------------------------------
/ Lightning #126
/-------------------------------*/
ROM_START(lightnin)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d3469d0a) SHA1(18565f5c85694da8eaf850146d3d9a90a17b7816))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(cd52262d) SHA1(099aeda2183822046cce907b265b42319007ac32))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(e0933419) SHA1(1f7cad915496f34473dffde7e320d51838acd0fd))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(df221c6b) SHA1(5935020d3a24d829fbeaa8cf764daff48a151a81))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(00ffa77c) SHA1(242efd800731a7f84369c6ce54298d0a227dd8ba))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(80fe9158) SHA1(20fcdb4c09b25e494f02bbfb20c07ff2870d5798))
ROM_END

/*--------------------------------
/ Stargazer #127
/-------------------------------*/
ROM_START(stargzr)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(83606fd4) SHA1(7f6448bc0dabe50de40fd47a7242c1be4a93e84d))   // StrGazr__R_A6.U1
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(c54ae389) SHA1(062e64e8ced723adb7f4040539ba6400fc4a9c9a))   // StrGazr__R_A6.U5
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(1a4c7dcb) SHA1(54888a8867b8d60f215b7e683ae4966f14ddca15))   // StrGazr__R_A6.U2
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(4e1f4dc6) SHA1(1f63a0b71af84fb6e1168ff77cbcbabcaa1323f3))   // StrGazr__R_A6.U6
ROM_END

/*--------------------------------
/ Flight 2000 #128
/-------------------------------*/
ROM_START(flight2k)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(df9efed9) SHA1(47727664e745e77ca1c221a32bd56d936f5b31bc))   // Flt2000__R_A13.U1
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(38c13649) SHA1(bcdbd17b48edd41ec7d38261595ac06eb8fc6a4d))   // Flt2000__R_A13.U5
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(425fae6a) SHA1(fde8d23e6ebb176ba72f763d66c2e17e51237fa1))   // Flt2000__R_A13.U2
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(dc243186) SHA1(046ce51b8a8218214088c4264548c753bd880e19))   // Flt2000__R_A13.U6

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(d816573c) SHA1(75134a017c34abbb149159ca001d35464a3f5128))
ROM_END

/*--------------------------------
/ Freefall #134
/-------------------------------*/
ROM_START(freefall)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "Freefall_RB23_U1.bin", 0x1000, 0x0800, CRC(da1473ba) SHA1(f27cbf1b16ad2a5f8c621ba3a069d4fca83d6ce1))
	ROM_LOAD( "Freefall_RB23_U5.bin", 0x1800, 0x0800, CRC(bdd833ee) SHA1(630d33be6765c83b3969a196f815c063f2ffb5f0))
	ROM_LOAD( "Freefall_RB23_U2.bin", 0x5000, 0x0800, CRC(b79a6ca4) SHA1(4a89babfacc7842320a3ac5a3e082ff93a46b93c))
	ROM_LOAD( "Freefall_RB23_U6.bin", 0x5800, 0x0800, CRC(a15414c0) SHA1(0d19e9ecd5e8c8d29b9b445c9984171b6679a8c7))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("freefall_rv0_snd_u9.716", 0x0000, 0x0800, CRC(ea8cf062) SHA1(55c840a9bea363fd436c00a115cb61d15a9f8c47))
	ROM_LOAD("freefall_rv0_snd_u10.716",0x0800, 0x0800, CRC(dd681a79) SHA1(d954cae375fb0145e10536e43d1cb03902de2ea3))
ROM_END

ROM_START(freefalo)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d13891ad) SHA1(afb40c51f2d5695c74ce9979c0a818845f95edd4))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(77bc7759) SHA1(3f739757180b3dcce5426935a51e4b615f157199))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(82bda054) SHA1(32772e878d2a4bba8f67e419a68a81fec2a5f6d7))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(68168b97) SHA1(defa4bba465182db22debddb4070c40c048c95e2))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("freefall_rv0_snd_u9.716", 0x0000, 0x0800, CRC(ea8cf062) SHA1(55c840a9bea363fd436c00a115cb61d15a9f8c47))
	ROM_LOAD("freefall_rv0_snd_u10.716",0x0800, 0x0800, CRC(dd681a79) SHA1(d954cae375fb0145e10536e43d1cb03902de2ea3))
ROM_END

ROM_START(freefalf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "new_freefall_mod16_64k", 0x0000, 0x10000, CRC(00000000))
ROM_END



/*--------------------------------
/ Split Second #144
/-------------------------------*/
ROM_START(spltsecp)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(c6ff9aa9) SHA1(39f80faca16c869ac14df7c5fc3dfa80b47dad95))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(fda74efc) SHA1(31becc243ada23e2f4d17927985772c9fcf8a3c3))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(81b9f784) SHA1(43cf71b51eda70a3c126340ea658c03c438e4f18))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(ecbedb0a) SHA1(8cc7281dd2bd300ab95a08761c12733d98599ebd))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(e6ed5f48) SHA1(ea2bbc607acb2b816667cd54f3d07605110c252e))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(36e6ee70) SHA1(61bd89d69627bea89b7f31af63ff90ace6db3c85))
ROM_END

/*--------------------------------
/ Catacomb #147
/-------------------------------*/
ROM_START(catacomb)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "catacomb_ra23_u1.bin", 0x1000, 0x0800, CRC(bf674561) SHA1(e66fdf9f8a0d5f51cb45550f46f936f4377e34da))
	ROM_LOAD( "catacomb_ra23_u5.bin", 0x1800, 0x0800, CRC(2e6c5e8a) SHA1(0c97cae873e615569d0eddaf8273c46e3c796a36))
	ROM_LOAD( "catacomb_ra23_u2.bin", 0x5000, 0x0800, CRC(a7e326c5) SHA1(3a8afa7c9fd5ad66d3250b456c9f3ed959d8a5e9))
	ROM_LOAD( "catacomb_ra23_u6.bin", 0x5800, 0x0800, CRC(9f2ca810) SHA1(4de16d9ed1a5bef74d9450a54f8441343b0cb66f))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(a13cb591) SHA1(b64a2dc3429803095dc05cdd1718db2404b13eb8))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(2b31f8be) SHA1(05b394bd8b6c04e34fe2bab19cbd0f06d9e4b90d))
ROM_END

ROM_START(catacomo)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "catacomb_ra22_u1.716", 0x1000, 0x0800, CRC(d445dd40) SHA1(9ff5896977d7e2a0cf788c77dcfd7c010e17d2fb))
	ROM_LOAD( "catacomb_ra22_u5.716", 0x1800, 0x0800, CRC(d717a545) SHA1(a183f3b1f766c3a82ae52defc38d84328fb7b31a))
	ROM_LOAD( "catacomb_ra22_u2.716", 0x5000, 0x0800, CRC(bc504409) SHA1(cd3e948d34a8db71fc841261e683988c9df31ef8))
	ROM_LOAD( "catacomb_ra22_u6.716", 0x5800, 0x0800, CRC(da61b5a2) SHA1(ec4a914cd57b37921578699bc427f12a3670c7eb))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(a13cb591) SHA1(b64a2dc3429803095dc05cdd1718db2404b13eb8))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(2b31f8be) SHA1(05b394bd8b6c04e34fe2bab19cbd0f06d9e4b90d))
ROM_END

ROM_START(catacomf)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "catacomb_ra23_u1.bin",          0x1000, 0x0800, CRC(bf674561) SHA1(e66fdf9f8a0d5f51cb45550f46f936f4377e34da))
	ROM_LOAD( "catacomb_ra22_u5_freeplay.bin", 0x1800, 0x0800, CRC(a98e639e) SHA1(fd9bb50eeef313189b332954938ae5cf1ce3b4ce))
	ROM_LOAD( "catacomb_ra23_u2.bin",          0x5000, 0x0800, CRC(a7e326c5) SHA1(3a8afa7c9fd5ad66d3250b456c9f3ed959d8a5e9))
	ROM_LOAD( "catacomb_ra23_u6.bin",          0x5800, 0x0800, CRC(9f2ca810) SHA1(4de16d9ed1a5bef74d9450a54f8441343b0cb66f))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(a13cb591) SHA1(b64a2dc3429803095dc05cdd1718db2404b13eb8))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(2b31f8be) SHA1(05b394bd8b6c04e34fe2bab19cbd0f06d9e4b90d))
ROM_END

ROM_START(catacofp)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "catacomb_ra22_u1.716",          0x1000, 0x0800, CRC(d445dd40) SHA1(9ff5896977d7e2a0cf788c77dcfd7c010e17d2fb))
	ROM_LOAD( "catacomb_ra22_u5_freeplay.716", 0x1800, 0x0800, CRC(3114512f) SHA1(660286e223ad87ddeefdcc7887cdb77bd7a42cee))
	ROM_LOAD( "catacomb_ra22_u2.716",          0x5000, 0x0800, CRC(bc504409) SHA1(cd3e948d34a8db71fc841261e683988c9df31ef8))
	ROM_LOAD( "catacomb_ra22_u6.716",          0x5800, 0x0800, CRC(da61b5a2) SHA1(ec4a914cd57b37921578699bc427f12a3670c7eb))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(a13cb591) SHA1(b64a2dc3429803095dc05cdd1718db2404b13eb8))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(2b31f8be) SHA1(05b394bd8b6c04e34fe2bab19cbd0f06d9e4b90d))
ROM_END


/*--------------------------------
/ Viper #148
/-------------------------------*/
ROM_START(viperp)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d0ea0aeb) SHA1(28f4df9f45807abd1528aa6e5a80933156e6d692))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(d26c7273) SHA1(303c18861941463932fdf47e9606159936b28dc1))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(d03f1612) SHA1(d390ec1e953148ac26bf218701117855c941fc65))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(96ff5f60) SHA1(a9df887ca338db208a684540f6c9fc07722c3aa5))
ROM_END

/*--------------------------------
/ Iron Maiden #151
/-------------------------------*/
ROM_START(ironmaid)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(e15371a4) SHA1(fe441ed8abd325190d8eee6d907e17c7fc02be64))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(84a29c01) SHA1(0e0ff8821c7028ce690328cd08a77bb51c0993c9))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(981ac0dd) SHA1(c585907b74695812f333867cf359a01a5ea6ed81))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(4e6f9c25) SHA1(9053e1d335a29f7acade7752adffe69f42032959))
ROM_END

/*--------------------------------
/ Dragonfist #153
/-------------------------------*/
ROM_START(dragfist)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(4cbd1a38) SHA1(73b7291f38cd0a3300107605db26d474ecfc3101))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(1783269a) SHA1(75151b79844d26d9e8ecf00dec96643ee2fedc5b))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(9ac8292b) SHA1(99ad3ad6e1d1b19695ce1b5b76f6bd85c9c6530d))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(a374c8f9) SHA1(481116025a52353f298f3d93dfe33b3ad9f86d18))
ROM_END

/*--------------------------------
/ Orbitor 1 #165
/-------------------------------*/
ROM_START(orbitor1)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(575520e3) SHA1(9d52b065a14d4f95cebd48f60f628f2c246385fa))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(d31f27a8) SHA1(0442260db42192a95f6292e6b57000c127871d28))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(4421d827) SHA1(9b617215f2d92ef2c69104eb4e63a924704665aa))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(8861155a) SHA1(81a1b3434d4f80dee5704454f8359200faea173d))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(2ba24569) SHA1(da2f4a4eeed9ae7ff8a342f4d630e12dcb2decf5))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(8e5b4a38) SHA1(de3f59363553f5f0d6098401734436930e64fbbd))
ROM_END

/*--------------------------------
/ Cue (Proto - Never released)
/-------------------------------*/
ROM_START(cue)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "xa14_u1.bin", 0x1000, 0x0800, CRC(0e1b4136) SHA1(ce69436a8cd30e2056df2ef86339f2e98e749774))
	ROM_LOAD( "xa14_u5.bin", 0x1800, 0x0800, CRC(65e15866) SHA1(a5f0d156b7429e2565da762d53decf8bc1589a5e))
	ROM_LOAD( "xa14_u2.bin", 0x5000, 0x0800, CRC(7a30ea8e) SHA1(5c8b1ad0add887c5986559c640d620971739e9a1))
	ROM_LOAD( "xa14_u6.bin", 0x5800, 0x0800, CRC(412d4592) SHA1(2bcc8832875bd6be49e17328069c19c955f35f8d))
ROM_END


/*--------------------------------
/ Hypnox
/-------------------------------*/

/*----------------------------------------
/ Lazer Lord (Proto - Never released)
/---------------------------------------*/
ROM_START(lazrlord)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(32a6f341) SHA1(75922c6831463d240fe057a0f72280d417899fa4))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(17583ba4) SHA1(4807e3ab18c2e40a292b499fe038975bb4b9fc17))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(669f3a8e) SHA1(4beb0e4c75f4e3c1788808b57081612d4774d130))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(395327a3) SHA1(e2a3a8ea696bcc4b5e11b08b6c7a6d9a991aa4af))
ROM_END

/*--------------------------------
/ Gamatron (Pinstar game, 1985)
/-------------------------------*/
ROM_START(gamatron)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "gamatron.764", 0x1000, 0x0800, CRC(fa9f7676) SHA1(8c56868eb6af7bb8ad73523ab6583100fcadc3c1))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_CONTINUE( 0x1800, 0x0800)
	ROM_CONTINUE( 0x5800, 0x0800)
ROM_END

/*----------------------------------
/ Stern System Test Fixture
/---------------------------------*/

ROM_START(st_game4)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(b9ac5204) SHA1(1ac4e336eb62c091e61e9b6b21a858e70ac9ab38))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(e16fbde1) SHA1(f7fe2f2ef9251792af1227f82dcc95239dd8baa1))
ROM_END



/*--------------------------------
/ 64kbit ROM/RAM qualification test
/-------------------------------*/
ROM_START(64ktests)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "64ktests_full.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "64ktests_full.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
ROM_END

ROM_START(64ktests1)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "64kTests_Full_2716.u1", 0x1000, 0x0800, CRC(00000000))
	ROM_LOAD( "64kTests_Full_2716.u5", 0x1800, 0x0800, CRC(00000000))
	ROM_LOAD( "64kTests_Full_2716.u2", 0x5000, 0x0800, CRC(00000000))
	ROM_LOAD( "64kTests_Full_2716.u6", 0x5800, 0x0800, CRC(00000000))
ROM_END




//   (YEAR   NAME        PARENT      MACHINE        INPUT      STATE             INIT             MONITOR COMPANY      FULLNAME                  FLAGS )
// 6-digit
GAMEL(1979,  meteor,     0,          st_mp200,      meteor,    meteor_state,     init_st_mp200,     ROT0, "Stern",     "Meteor (Pinball)",                          MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp200_meteor)
GAMEL(1979,  meteoro,    meteor,     st_mp200,      meteor,    meteor_state,     init_st_mp200,     ROT0, "Stern",     "Meteor (Pinball) Old",                      MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp200_meteor)
GAMEL(1979,  meteors,    meteor,     st_mp200,      meteor,    meteor_state,     init_st_mp200,     ROT0, "Stern",     "Meteor (Pinball) Seymour Shabow v65",       MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp200_meteor)
GAMEL(1979,  meteorl,    meteor,     st_mp200_ext,  meteor,    meteor_state,     init_st_mp200,     ROT0, "Stern",     "Meteor (Pinball) TEST 128kbit 16kByte ROM", MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp200_meteor)
GAME( 1980,  galaxy,     0,          st_mp200,      galaxy,    galaxy_state,     init_galaxy,       ROT0, "Stern",     "Galaxy",                                    MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  galaxyb,    galaxy,     st_mp200,      galaxy,    galaxy_state,     init_galaxy,       ROT0, "Stern",     "Galaxy 7-Digit (Oliver set)",               MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  galaxyfp,   galaxy,     st_mp200,      galaxy,    galaxy_state,     init_galaxy,       ROT0, "Stern",     "Galaxy (Freeplay)",                         MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  galaxyt,    galaxy,     st_mp200,      galaxy,    galaxy_state,     init_galaxy,       ROT0, "Stern",     "Galaxy TEST",                               MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAMEL(1980,  ali,        0,          st_mp200,      ali,       ali_state,        init_ali,          ROT0, "Stern",     "Ali",                                       MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_st_mp200_ali)
GAME( 1980,  alit,       ali,        st_mp200,      alit,      ali_state,        init_ali,          ROT0, "Stern",     "Ali TEST",                                  MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 2017,  disptest,   0,          st_mp200,      mp200,     st_mp200_state,   init_st_mp200,     ROT0, "Stern",     "Display Test",                              MACHINE_MECHANICAL | MACHINE_NOT_WORKING)


// 7-digit
GAME( 1980,  biggame,    0,          st_mp200,      mp200,     st_mp200_state,   init_st_mp207,     ROT0, "Stern",     "Big Game",                          MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  cheetah,    0,          st_mp200,      mp200,     st_mp200_state,   init_st_mp207,     ROT0, "Stern",     "Cheetah",                           MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  cheetahb,   cheetah,    st_mp200,      mp200,     st_mp200_state,   init_st_mp207,     ROT0, "Stern",     "Cheetah (Blue Cabinet)",            MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  quicksil,   0,          st_mp200,      mp200,     st_mp200_state,   init_st_mp207,     ROT0, "Stern",     "Quicksilver",                       MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  seawitch,   0,          st_mp200,      seawitch,  seawitch_state,   init_st_mp207,     ROT0, "Stern",     "Seawitch",                          MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  seawitchs,  seawitch,   st_mp200,      seawitch,  seawitch_state,   init_st_mp207,     ROT0, "Stern",     "Seawitch (Scotts Update)",          MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  nineball,   0,          st_mp200,      nineball,  nineball_state,   init_nineball,     ROT0, "Stern",     "Nine Ball",                         MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  ninebalb,   nineball,   st_mp200_ext,  nineball,  nineball_state,   init_nineball,     ROT0, "Stern",     "Nine Ball (Oliver Rule Set)",       MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  ninebaln,   nineball,   st_mp200_ext,  nineball,  nineball_state,   init_nineball,     ROT0, "Stern",     "Nine Ball (Barakandl board)",       MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  ninebalf,   nineball,   st_mp200_full, nineball,  nineball_state,   init_nineball,     ROT0, "Stern",     "Nine Ball (Barakandl board Full)",  MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981,  lightnin,   0,          st_mp200_spch, lightnin,  lightnin_state,   init_lightnin,     ROT0, "Stern",     "Lightning",                         MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  stargzr,    0,          st_mp200,      mp200,     st_mp200_state,   init_st_mp207,     ROT0, "Stern",     "Stargazer",                         MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981,  spltsecp,   0,          st_mp200_spch, mp237,     st_mp200_state,   init_st_mp237,     ROT0, "Stern",     "Split Second (Pinball)",            MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981,  catacomb,   0,          st_mp200_spch, catacomb,  catacomb_state,   init_catacomb,     ROT0, "Stern",     "Catacomb ver RA23 (Pinball)",                MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981,  catacomf,   catacomb,   st_mp200_spch, catacomb,  catacomb_state,   init_catacomb,     ROT0, "Stern",     "Catacomb ver RA23 (Pinball) (Freeplay)",     MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981,  catacomo,   catacomb,   st_mp200_spch, catacomb,  catacomb_state,   init_catacomb,     ROT0, "Stern",     "Catacomb ver RA22 (Pinball) (Older set)",    MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981,  catacofp,   catacomb,   st_mp200_spch, catacomb,  catacomb_state,   init_catacomb,     ROT0, "Stern",     "Catacomb ver RA22 (Pinball) (Older set Freeplay)",   MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1982,  dragfist,   0,          st_mp200,      mp200,     st_mp200_state,   init_st_mp207,     ROT0, "Stern",     "Dragonfist",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1984,  lazrlord,   0,          st_mp200,      mp200,     st_mp200_state,   init_st_mp207,     ROT0, "Stern",     "Lazer Lord",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING)

// hang after boot
GAME( 1980,  flight2k,   0,          st_mp200_spch, flight2k,  flight2k_state,   init_st_mp237,     ROT0, "Stern",     "Flight 2000",                       MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981,  freefall,   0,          st_mp200_spch, mp237,     st_mp200_state,   init_st_mp237,     ROT0, "Stern",     "Freefall",                          MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981,  freefalo,   freefall,   st_mp200_spch, mp237,     st_mp200_state,   init_st_mp237,     ROT0, "Stern",     "Freefall (Older set)",              MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980,  freefalf,   freefall,   st_mp200_full, mp200,     st_mp200_state,   init_st_mp207,     ROT0, "Stern",     "Freefall (Barakandl board Full)",   MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981,  viperp,     0,          st_mp200,      mp237,     st_mp200_state,   init_st_mp237,     ROT0, "Stern",     "Viper (Pinball)",                   MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981,  ironmaid,   0,          st_mp200,      mp237,     st_mp200_state,   init_st_mp237,     ROT0, "Stern",     "Iron Maiden",                       MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1982,  orbitor1,   0,          st_mp200_spch, mp237,     st_mp200_state,   init_st_mp237,     ROT0, "Stern",     "Orbitor 1",                         MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1982,  cue,        0,          st_mp200,      mp200,     st_mp200_state,   init_st_mp207,     ROT0, "Stern",     "Cue (Prototype)",                   MACHINE_MECHANICAL | MACHINE_NOT_WORKING)

// other manufacturer
GAME( 1985,  gamatron,   flight2k,   st_mp200,      mp200,     st_mp200_state,   init_st_mp207,     ROT0, "Pinstar",   "Gamatron",                          MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 198?,  st_game4,   0,          st_mp200,      mp200,     st_mp200_state,   init_st_mp200,     ROT0, "Stern",     "SAM IV (Service Assistance Module 4)",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING)

GAME( 2023,  64ktests,   0,          st_mp200,      mp200,     st_mp200_state,   init_st_mp200,     ROT0, "Quench",    "TEST 64kbit 8kb 2732 ROM MPU200",   MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 2023,  64ktests1,  64ktests,   st_mp200,      mp200,     st_mp200_state,   init_st_mp200,     ROT0, "Quench",    "TEST 64kbit 8kb 2716 ROM MPU200",   MACHINE_MECHANICAL | MACHINE_NOT_WORKING)

// license:BSD-3-Clause
// copyright-holders:Robbbert, Quench
/********************************************************************************************

    PINBALL
    Bally MPU AS-2518-35


Various sound boards used:

* AS-2518-32 and AS-2518-50
  Discrete circuitry

* AS-2518-51 Sounds Module
  6802/6808/6810 + 6821 + AY-3-8910 + 1x4kROM

* AS-2518-56 Sounds Plus Board (similar to Sounds Module but with extra connector to interface to Vocalizer Speech board)
  6802/6808/6810 + 6821 + AY-3-8910 + 1x4kROM

* AS-2518-57 Vocalizer Speech Board adds speech to Sounds Plus board. Only used on Xenon and very early Flash Gordons
  MC3417 + 7x4kROMs for speech data

* AS-2518-61 Squawk & Talk
  6802/6808/6810 + 2x6821 + AY-3-8912 + AD558 + TMS5200 + TMS6100 + 4x1kROM

* AS-2518-81 Say it Again board adds Reverb to "Squawk & Talk" boards. Only used on Centaur and Centaur II
  SAD4096

* AS-2518-45 / A084-91495-A360 Cheap Squeak
  6803 + ZN429 + 2x2kROM




Sound Board                      Game Part            Machines
                                 Number
---------------------------------------------------------------------------------

AS-2518-32                       AS-2888-1            1106, 1116, 1119, 1138, 1147, 1148
AS-2518-50                       AS-2888-3            1152
AS-2518-50                       AS-2888-4            1161, 1162, 1167
AS-2518-51                       AS-3022-1            1173
AS-2518-51                       AS-3022-2            1154
AS-2518-51                       AS-3022-3            1157
AS-2518-51                       AS-3022-5            1187
AS-2518-51                       AS-3022-6            1178
AS-2518-51                       AS-3022-7            1192
AS-2518-51                       AS-3022-8            1198
AS-2518-51                       AS-3022-9            1199
AS-2518-51                       AS-3022-11           1210
AS-2518-51                       AS-3022-12           1217
AS-2518-51                       AS-3022-14           1273
AS-2518-51                       AS-3022-15           1276
AS-2518-51                       AS-3022-18           1311
AS-2518-56 + AS-2518-57          AS-3060 + AS-3059    1196, 1215 (very early games)
AS-2518-61                       AS-3107-1            1215
AS-2518-61                       AS-3107-2            1220
AS-2518-61                       AS-3107-3            1219
AS-2518-61                       AS-3107-4            1222
AS-2518-61                       AS-3107-5            1233
AS-2518-61                       AS-3107-6            1245
AS-2518-61 + AS-2518-81          AS-3107-7            1239, 1370
AS-2518-61                       AS-3107-8            1248
AS-2518-61                       AS-3107-9            1247
AS-2518-61                       AS-3107-10           1262
AS-2518-61                       AS-3107-11           1282
AS-2518-61                       AS-3107-12           1283
AS-2518-45 / A084-91495-A360     A084-91485-A371      1371
AS-2518-45 / A080-91603-B000     A080-91603-B391      1390, 1391
AS-2518-45 / A080-91603-C000     A084-91603-AA17      0A17
AS-2518-45 / A080-91603-C000     A084-91603-AA40      0A40
AS-2518-45 / A080-91603-C000     A084-91603-AA44      0A44
AS-2518-45 / A080-91603-C000     A084-91603-AB42      0B42


- The Nuova Bell Games from Dark Shadow onwards use inhouse designed circuit boards. The MPU board contains enhancements for full
  CPU address space, larger ROMs, 6802 CPU, Toshiba TC5517 CMOS RAM (2kb) for battery backup that can be jumpered in nibble or byte mode, etc.

- Space Invaders has a software bug with the "-37" system ROM at U6. Dip Switch 1 and 9 are masked out when processing the number of coins per credit.
  Owners can replace their U6 ROM with version "-35" for the standard coin/credit settings.
  The "-37" system ROM also has the multiple 1,000's score queue removed to make room for the alternate coin/credit settings functionality.

ToDo:
- The Nuova Bell games don't boot.
- The Bell games have major problems
- Add sound for non Bally games
- Add game specific DIP Switches, Inputs and Solenoids which vary per game

*********************************************************************************************/


#include "emu.h"
#include "machine/genpin.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "audio/bally.h"
#include "render.h"
#include "speaker.h"

#define VERBOSE 1
#include "logmacro.h"

#include "by35.lh"
#include "by35_53.lh"
#include "by35_53_nuova.lh"
#include "by35_lostwrlp.lh"
#include "by35_playboy.lh"
#include "by35_slbmania.lh"
#include "by35_smman.lh"
#include "by35_sst.lh"
#include "by35_startrep.lh"


class by35_state : public genpin_class
{
public:
	by35_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_default)
	{ }

	void init_by35_6()   { m_7d = 0;  m_outhole[0] = 0x0080;  m_outhole[1] = 0x0000;  m_6mdman = 0;  m_io_x5_mask = 0;     m_sol_exp_lamp = 255; m_gi_flasher_lamp = 255; }
	void init_by35_7()   { m_7d = 1;  m_outhole[0] = 0x0080;  m_outhole[1] = 0x0000;  m_6mdman = 0;  m_io_x5_mask = 0;     m_sol_exp_lamp = 255; m_gi_flasher_lamp = 255; }
	void init_6mdman()   { m_7d = 0;  m_outhole[0] = 0x0080;  m_outhole[1] = 0x0000;  m_6mdman = 1;  m_io_x5_mask = 0;     m_sol_exp_lamp = 255; m_gi_flasher_lamp = 255; }
	void init_embryon()  { m_7d = 1;  m_outhole[0] = 0x0090;  m_outhole[1] = 0x0000;  m_6mdman = 0;  m_io_x5_mask = 0;     m_sol_exp_lamp = 255; m_gi_flasher_lamp = 52; }
	void init_xenon()    { m_7d = 1;  m_outhole[0] = 0x0082;  m_outhole[1] = 0x0308;  m_6mdman = 0;  m_io_x5_mask = 0;     m_sol_exp_lamp = 255; m_gi_flasher_lamp = 255; }
	void init_flashgdn() { m_7d = 1;  m_outhole[0] = 0x0080;  m_outhole[1] = 0x0000;  m_6mdman = 0;  m_io_x5_mask = 0;     m_sol_exp_lamp = 255; m_gi_flasher_lamp = 84;  }
	void init_fball_ii() { m_7d = 1;  m_outhole[0] = 0x0080;  m_outhole[1] = 0x0202;  m_6mdman = 0;  m_io_x5_mask = 0;     m_sol_exp_lamp = 28;  m_gi_flasher_lamp = 86;  }	// Doodle Bug lamp flash on 28
	void init_eballdlx() { m_7d = 1;  m_outhole[0] = 0x0080;  m_outhole[1] = 0x0000;  m_6mdman = 0;  m_io_x5_mask = 0;     m_sol_exp_lamp = 48;  m_gi_flasher_lamp = 255; }
	void init_fathom()   { m_7d = 1;  m_outhole[0] = 0x0007;  m_outhole[1] = 0x0000;  m_6mdman = 0;  m_io_x5_mask = 0x80;  m_sol_exp_lamp = 44;  m_gi_flasher_lamp = 255; }
	void init_centaur()  { m_7d = 1;  m_outhole[0] = 0x0081;  m_outhole[1] = 0x8201;  m_6mdman = 0;  m_io_x5_mask = 0x10;  m_sol_exp_lamp = 255; m_gi_flasher_lamp = 00;  }
	void init_medusa()   { m_7d = 1;  m_outhole[0] = 0x0080;  m_outhole[1] = 0x0000;  m_6mdman = 0;  m_io_x5_mask = 0x80;  m_sol_exp_lamp = 31;  m_gi_flasher_lamp = 255; }
	void init_vector()   { m_7d = 1;  m_outhole[0] = 0x0007;  m_outhole[1] = 0x0000;  m_6mdman = 0;  m_io_x5_mask = 0x10;  m_sol_exp_lamp = 59;  m_gi_flasher_lamp = 45;  }	// Other flashers for the back box on lamps 00 and 30
	void init_m_mpac()   { m_7d = 1;  m_outhole[0] = 0x0010;  m_outhole[1] = 0x0000;  m_6mdman = 0;  m_io_x5_mask = 0;     m_sol_exp_lamp = 73;  m_gi_flasher_lamp = 66;  }	// Other flasher for the back box on lamp 73
	void init_goldball() { m_7d = 0;  m_outhole[0] = 0x0080;  m_outhole[1] = 0x8320;  m_6mdman = 0;  m_io_x5_mask = 0;     m_sol_exp_lamp = 255; m_gi_flasher_lamp = 33;  } // J1-21 U3-02
	void init_blakpyra() { m_7d = 1;  m_outhole[0] = 0x0080;  m_outhole[1] = 0x0000;  m_6mdman = 0;  m_io_x5_mask = 0;     m_sol_exp_lamp = 255; m_gi_flasher_lamp = 00; }
	void init_fbclass()  { m_7d = 1;  m_outhole[0] = 0x0338;  m_outhole[1] = 0x0000;  m_6mdman = 0;  m_io_x5_mask = 0;     m_sol_exp_lamp = 50;  m_gi_flasher_lamp = 255; }
	void init_bullseye() { m_7d = 0;  m_outhole[0] = 0x0401;  m_outhole[1] = 0x0000;  m_6mdman = 0;  m_io_x5_mask = 0;     m_sol_exp_lamp = 255; m_gi_flasher_lamp = 255; }

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
	DECLARE_CUSTOM_INPUT_MEMBER(switch_hold_x5);

	void by35(machine_config &config);
	void by35_53(machine_config &config);
	void by35_ext_rom(machine_config &config);
	void by35_barakandl(machine_config &config);
	void nuova(machine_config &config);
	void grand(machine_config &config);
	void as2888(machine_config &config);
	void as2888_ext(machine_config &config);
	void as3022(machine_config &config);
	void sounds_plus(machine_config &config);
	void cheap_squeak(machine_config &config);
	void squawk_n_talk(machine_config &config);
	void squawk_n_talk_ay(machine_config &config);
	void squawk_n_talk_ay_53(machine_config &config);
	void bell_cheap_squeak(machine_config &config);

protected:
	typedef uint8_t solenoid_feature_data[20][8];

	by35_state(machine_config const &mconfig, device_type type, char const *tag, solenoid_feature_data const &solenoid_features)
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
		, m_io_zx(*this, "ZX")
		, m_io_outhole(*this, "OUTHOLE")
		, m_io_spinner(*this, "SPINNER")
		, m_lamps(*this, "lamp%u", 0U)
		, m_display(*this, "digit%u%u", 0U, 0U)
		, m_solenoids(*this, "solenoid%u", 0U)
		, m_spinners(*this, "spinner%u", 0U)
		, m_relay(*this, "relay%u", 0U)
		, m_scrn_switches(*this, "switch%u", 0U)
		, m_io_x5(*this, "X5")
		, m_as2888(*this, "as2888")
		, m_as3022(*this, "as3022")
		, m_sounds_plus(*this, "sounds_plus")
		, m_cheap_squeak(*this, "cheap_squeak")
		, m_squawk_n_talk(*this, "squawk_n_talk")
		, m_squawk_n_talk_ay(*this, "squawk_n_talk_ay")
	{ }


	DECLARE_READ8_MEMBER(u10_a_r);
	DECLARE_WRITE8_MEMBER(u10_a_w);
	DECLARE_READ8_MEMBER(u10_b_r);
	DECLARE_WRITE8_MEMBER(u10_b_w);
	DECLARE_READ8_MEMBER(u11_a_r);
	DECLARE_WRITE8_MEMBER(u11_a_w);
	DECLARE_WRITE8_MEMBER(u11_a_as2888_w);
	DECLARE_WRITE8_MEMBER(u11_b_w);
	DECLARE_WRITE8_MEMBER(u11_b_as2888_w);
	DECLARE_WRITE8_MEMBER(u11_b_as3022_w);
	DECLARE_WRITE8_MEMBER(u11_b_as3060_w);
	DECLARE_WRITE8_MEMBER(u11_b_a91603_w);
	DECLARE_WRITE8_MEMBER(u11_b_as3107_w);
	DECLARE_WRITE8_MEMBER(u11_b_as3107a_w);
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
	DECLARE_WRITE_LINE_MEMBER(u11_cb2_as2888_w);
	DECLARE_WRITE_LINE_MEMBER(u11_cb2_as3022_w);
	DECLARE_WRITE_LINE_MEMBER(u11_cb2_as3060_w);
	DECLARE_WRITE_LINE_MEMBER(u11_cb2_a91603_w);
	DECLARE_WRITE_LINE_MEMBER(u11_cb2_as3107_w);
	DECLARE_WRITE_LINE_MEMBER(u11_cb2_as3107a_w);
	DECLARE_WRITE_LINE_MEMBER(sound_ack_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	TIMER_DEVICE_CALLBACK_MEMBER(timer_z_freq);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_z_pulse);
	TIMER_DEVICE_CALLBACK_MEMBER(u11_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_d_pulse);

	void by35_map(address_map &map);
	void by35_ext_map(address_map &map);
	void nuova_map(address_map &map);
	void grand_map(address_map &map);
	void by35_barakandl_map(address_map &map);

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
	bool m_7d;
	bool m_6mdman;
	uint8_t m_digit;
	uint8_t m_segment[7];
	uint8_t m_lamp_decode;
	uint8_t m_lamp_decode_aux;
	solenoid_feature_data const &m_solenoid_features;
	uint8_t m_io_hold_x[6];       // Holds switches closed (drop targets, balls in outholes/saucers/kickbacks etc). Solenoid activity releases the switch.
	uint16_t m_outhole[2];        // Optionally put a ball in the outhole(s) at power on. Trough switches can be is a separate part of the switch matrix.
	uint16_t m_spinner[4];        // Helps animate a simulated Spinner on the playfield layout
	uint8_t m_io_x5_mask;         // Games with extra strobe line for switch matrix are wired to unused continuous solenoid bits and varies per game
	uint8_t m_sol_exp_lamp;       // Lamp number that activates the Solenoid Expander Board relay for banked solenoids
	uint8_t m_sol_exp_relay;      // State of the Solenoid Expander Board relay for banked solenoids
	bool m_lamp_prepare_relay;    // Prepare Lamp driven Relays to switch only when required
	uint8_t m_gi_flasher_lamp;    // Lamp number that activates the G.I. flasher Triac
	uint8_t m_disp_key[49];       // Helps re-assign keys on the display layouts switch matrix. Use ASCII values
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
	required_ioport m_io_zx;
	required_ioport m_io_outhole;
	required_ioport m_io_spinner;
	output_finder<22 * 4> m_lamps;
	output_finder<7, 8> m_display;
	output_finder<35> m_solenoids;
	output_finder<3> m_spinners;
	output_finder<2> m_relay;
	output_finder<(6 * 8) + 1> m_scrn_switches;
	optional_ioport m_io_x5;
	optional_device<bally_as2888_device> m_as2888;
	optional_device<bally_as3022_device> m_as3022;
	optional_device<bally_sounds_plus_device> m_sounds_plus;
	optional_device<bally_cheap_squeak_device> m_cheap_squeak;
	optional_device<bally_squawk_n_talk_device> m_squawk_n_talk;
	optional_device<bally_squawk_n_talk_ay_device> m_squawk_n_talk_ay;
	void update_lamps(u8 lamp_board_base, u8 lamp_decoder_base, u8 lamp_decoder_addr, u8 lamp_decoder_data);
};

class lostwrlp_state : public by35_state
{
public:
	lostwrlp_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_lostwrlp)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_lostwrlp;
};

class sst_state : public by35_state
{
public:
	sst_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_sst)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_sst;
};

class playboy_state : public by35_state
{
public:
	playboy_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_playboy)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_playboy;
};

class smman_state : public by35_state
{
public:
	smman_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_smman)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_smman;
};

class startrep_state : public by35_state
{
public:
	startrep_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_startrep)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_startrep;
};

class kiss_state : public by35_state
{
public:
	kiss_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_kiss)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_kiss;
};

class slbmania_state : public by35_state
{
public:
	slbmania_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_slbmania)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_slbmania;
};

class hglbtrtr_state : public by35_state
{
public:
	hglbtrtr_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_hglbtrtr)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_hglbtrtr;
};

class ngndshkr_state : public by35_state
{
public:
	ngndshkr_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_ngndshkr)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_ngndshkr;
};

class futurspa_state : public by35_state
{
public:
	futurspa_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_futurspa)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_futurspa;
};

class spaceinv_state : public by35_state
{
public:
	spaceinv_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_spaceinv)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_spaceinv;
};

class viking_state : public by35_state
{
public:
	viking_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_viking)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_viking;
};

class frontier_state : public by35_state
{
public:
	frontier_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_frontier)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_frontier;
};

class skatebll_state : public by35_state
{
public:
	skatebll_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_skatebll)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_skatebll;
};

class xenon_state : public by35_state
{
public:
	xenon_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_xenon)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_xenon;
};

class embryon_state : public by35_state
{
public:
	embryon_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_embryon)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_embryon;
};

class flashgdn_state : public by35_state
{
public:
	flashgdn_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_flashgdn)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_flashgdn;
};

class fball_ii_state : public by35_state
{
public:
	fball_ii_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_fball_ii)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_fball_ii;
};

class eballdlx_state : public by35_state
{
public:
	eballdlx_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_eballdlx)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_eballdlx;
};

class fathom_state : public by35_state
{
public:
	fathom_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_fathom)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_fathom;
};

class centaur_state : public by35_state
{
public:
	centaur_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_centaur)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_centaur;
};

class medusa_state : public by35_state
{
public:
	medusa_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_medusa)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_medusa;
};

class vector_state : public by35_state
{
public:
	vector_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_vector)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_vector;
};

class m_mpac_state : public by35_state
{
public:
	m_mpac_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_m_mpac)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_m_mpac;
};

class goldball_state : public by35_state
{
public:
	goldball_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_goldball)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_goldball;
};

class kosteel_state : public by35_state
{
public:
	kosteel_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_kosteel)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_kosteel;
};

class xsandos_state : public by35_state
{
public:
	xsandos_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_xsandos)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_xsandos;
};

class blakpyra_state : public by35_state
{
public:
	blakpyra_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_blakpyra)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_blakpyra;
};

class spyhuntr_state : public by35_state
{
public:
	spyhuntr_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_spyhuntr)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_spyhuntr;
};

class fbclass_state : public by35_state
{
public:
	fbclass_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_fbclass)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_fbclass;
};

class cybrnaut_state : public by35_state
{
public:
	cybrnaut_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_cybrnaut)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_cybrnaut;
};

class bullseye_state : public by35_state
{
public:
	bullseye_state(machine_config const &mconfig, device_type type, char const *tag)
		: by35_state(mconfig, type, tag, s_solenoid_features_bullseye)
	{ }

protected:
	static solenoid_feature_data const s_solenoid_features_bullseye;
};


void by35_state::by35_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7fff);     // A15 is not connected
	map(0x0000, 0x007f).mirror(0x6100).ram();
	map(0x0088, 0x008b).mirror(0x6d74).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).mirror(0x6d6c).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0200, 0x02ff).mirror(0x6d00).ram().rw(FUNC(by35_state::nibble_nvram_r), FUNC(by35_state::nibble_nvram_w)).share("nvram");
	map(0x1000, 0x1fff).mirror(0x2000).rom();
	map(0x5000, 0x5fff).mirror(0x2000).rom();
}

void by35_state::by35_ext_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7fff);     // A15 is not connected
	map(0x0000, 0x007f).mirror(0x6100).ram();
	map(0x0088, 0x008b).mirror(0x6d74).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).mirror(0x6d6c).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0200, 0x02ff).mirror(0x6d00).ram().rw(FUNC(by35_state::nibble_nvram_r), FUNC(by35_state::nibble_nvram_w)).share("nvram");
	map(0x1000, 0x1fff).rom();
	map(0x3000, 0x3fff).rom();
	map(0x5000, 0x5fff).rom();
	map(0x7000, 0x7fff).rom();
}

void by35_state::by35_barakandl_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).ram();
	map(0x0088, 0x008b).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0100, 0x0fff).ram().rw(FUNC(by35_state::nibble_nvram_r), FUNC(by35_state::nibble_nvram_w)).share("nvram");
	map(0x1000, 0xffff).rom();
}

void by35_state::nuova_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
//  AM_RANGE(0x0000, 0x007f) AM_RAM     // Schematics infer that the M6802 internal RAM is disabled.
	map(0x0088, 0x008b).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0xffff).rom();
}

void by35_state::grand_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x3fff);     // A15 and A14 are not connected
	map(0x0000, 0x007f).mirror(0x2100).ram();
	map(0x0088, 0x008b).mirror(0x2d74).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).mirror(0x2d6c).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0200, 0x02ff).mirror(0x2d00).ram().rw(FUNC(by35_state::nibble_nvram_r), FUNC(by35_state::nibble_nvram_w)).share("nvram");
	map(0x2000, 0x2fff).rom();
	map(0x3000, 0x3fff).rom();
}


static INPUT_PORTS_START( by35_machine )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Self Test") PORT_CHANGED_MEMBER(DEVICE_SELF, by35_state, self_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Activity")  PORT_CHANGED_MEMBER(DEVICE_SELF, by35_state, activity_button, 0)

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
INPUT_PORTS_END

static INPUT_PORTS_START( by35_switch_matrix_x0_x4 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, outhole, (void *)0x0080)  // PORT_CODE(KEYCODE_BACKSPACE)

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

static INPUT_PORTS_START( by35_switch_matrix_x5 )
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


static INPUT_PORTS_START( by35 )
	PORT_INCLUDE( by35_machine )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 1 (Coins/Credits)")    PORT_DIPLOCATION("S01-S08:!1,!2,!3,!4,!5")  // Same as Coin Slot 3
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
	PORT_DIPNAME( 0x60, 0x60, "Beating Highest Score Awards")   PORT_DIPLOCATION("S01-S08:!6,!7")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x20, "1 Credit")
	PORT_DIPSETTING(    0x40, "2 Credits")
	PORT_DIPSETTING(    0x60, "3 Credits")
	PORT_DIPNAME( 0x80, 0x80, "Melody Option 1")                PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 3 (Coins/Credits)")    PORT_DIPLOCATION("S09-S16:!1,!2,!3,!4,!5")  // Same as Coin Slot 1
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
	PORT_DIPNAME( 0x60, 0x60, "Score Level Awards")     PORT_DIPLOCATION("S09-S16:!6,!7")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x40, "Extra Ball")
	PORT_DIPSETTING(    0x60, "Replay")
	PORT_DIPNAME( 0x80, 0x80, "Balls Per Game")         PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x80, "5")

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
	PORT_DIPNAME( 0x08, 0x08, "Credits Displayed")          PORT_DIPLOCATION("S17-S24:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x10, "Match Feature")              PORT_DIPLOCATION("S17-S24:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 22 (Game Specific)")   PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 23 (Game Specific)")   PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 24 (Game Specific)")   PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2")                PORT_DIPLOCATION("S25-S32:!1,!2,!3,!4")
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
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 29 (Game Specific)")   PORT_DIPLOCATION("S25-S32:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 30 (Game Specific)")   PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 31 (Game Specific)")   PORT_DIPLOCATION("S25-S32:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, "Melody Option 2")            PORT_DIPLOCATION("S25-S32:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_INCLUDE( by35_switch_matrix_x0_x4 )
INPUT_PORTS_END


/*
    Dips for os28/30 - Lost World, SuperSonic, Playboy, Six Million Dollar Man, Star Trek, Kiss
*/
static INPUT_PORTS_START( by35_30 )    // System ROM v30
	PORT_INCLUDE( by35 )
INPUT_PORTS_END


static INPUT_PORTS_START( lostwrlp )
	PORT_INCLUDE( by35 )

	PORT_MODIFY("SPINNER")                           // Allow user to select the simulated rotation speed of Spinners
	PORT_CONFNAME( 0x07, 0x03, "Spinner Simulation")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ))
	PORT_CONFSETTING(    0x01, "Slow Speed")
	PORT_CONFSETTING(    0x02, "Medium Speed")
	PORT_CONFSETTING(    0x03, "High Speed")
	PORT_CONFSETTING(    0x07, "Turbo Speed")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "Saucer Specials")                PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, "1 Per Ball")
	PORT_DIPSETTING(    0x20, "Open Ended")
	PORT_DIPNAME( 0x40, 0x00, "Saucer Starting Value")          PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "500 Points")
	PORT_DIPSETTING(    0x40, "1000 Points")
	PORT_DIPNAME( 0x80, 0x00, "Outlane Specials")               PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x80, "Both On")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x10, 0x00, "Flipper Lanes Advance Saucer Value")    PORT_DIPLOCATION("S25-S32:!5")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x10, "Both")

	PORT_MODIFY("X2")   /* Spinner and Saucers */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1201)  // PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)
INPUT_PORTS_END

static INPUT_PORTS_START( sst )
	PORT_INCLUDE( by35_30 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "4 and 5 Lanes")                  PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, "Separate")
	PORT_DIPSETTING(    0x20, "Tied Together")
	PORT_DIPNAME( 0x40, 0x00, "1 and 3 Lanes")                  PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "Separate")
	PORT_DIPSETTING(    0x40, "Tied Together")
	PORT_DIPNAME( 0x80, 0x00, "Drop Targets Twice Down Awards") PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Extra Ball")
	PORT_DIPSETTING(    0x80, "Extra Ball and Specials")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x40, 0x00, "Outlane Specials")               PORT_DIPLOCATION("S25-S32:!7")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x40, "Both On")

	PORT_MODIFY("X0")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X2")   /* Spinner and Saucer */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1201)  // PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)
INPUT_PORTS_END

static INPUT_PORTS_START( sstf )
	PORT_INCLUDE( sst )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Maximum Credits")			PORT_DIPLOCATION("S17-S24:!1,!2")
	PORT_DIPSETTING(    0x00, "05")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
	PORT_DIPNAME( 0x04, 0x04, "Display Digits")				PORT_DIPLOCATION("S17-S24:!3")
	PORT_DIPSETTING(    0x00, "6 Digits")
	PORT_DIPSETTING(    0x04, "7 Digits")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x60, 0x60, "Awards")     PORT_DIPLOCATION("S09-S16:!6,!7")
	PORT_DIPSETTING(    0x00, "Novelty")
	PORT_DIPSETTING(    0x20, "Novelty / Extra Ball")
	PORT_DIPSETTING(    0x40, "Extra Ball")
	PORT_DIPSETTING(    0x60, "Replay")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2 (Coins/Credits)")    PORT_DIPLOCATION("S25-S32:!1,!2,!3,!4")
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x0f, "5 Coins/2 Credits (C3/1C, C5/1C)")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, "Same as Coin Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
INPUT_PORTS_END

static INPUT_PORTS_START( playboy )
	PORT_INCLUDE( by35_30 )

	PORT_MODIFY("SPINNER")                           // Allow user to select the simulated rotation speed of Spinners
	PORT_CONFNAME( 0x07, 0x00, "Spinner Simulation")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ))

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "Drop Target Special")        PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, "Lit Until Next Ball")
	PORT_DIPSETTING(    0x20, "Lit Until Collected")
	PORT_DIPNAME( 0x40, 0x00, "Numbered Keys")              PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "Reset At Next Ball")
	PORT_DIPSETTING(    0x40, "Remembered Next Ball")
	PORT_DIPNAME( 0x80, 0x00, "25000 Outlanes")             PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x80, "Both On")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x10, 0x00, "2 and 3 Key Lanes")          PORT_DIPLOCATION("S25-S32:!5")
	PORT_DIPSETTING(    0x00, "Separate")
	PORT_DIPSETTING(    0x10, "Tied Together")
	PORT_DIPNAME( 0x20, 0x00, "1 and 4 Key Lanes")          PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, "Separate")
	PORT_DIPSETTING(    0x20, "Tied Together")
	PORT_DIPNAME( 0x40, 0x00, "Rollover Button")            PORT_DIPLOCATION("S25-S32:!7")
	PORT_DIPSETTING(    0x00, "Extra Ball and/or Special Reset At Next Ball")
	PORT_DIPSETTING(    0x40, "Extra Ball or Special Held Until Collected")

	PORT_MODIFY("X0")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X3")   /* Grotto Kickback */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END

static INPUT_PORTS_START( playboyt )
	PORT_INCLUDE( playboy )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x80, 0x80, "Melody Option")				PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Maximum Credits")			PORT_DIPLOCATION("S17-S24:!1,!2")
	PORT_DIPSETTING(    0x00, "10")
	PORT_DIPSETTING(    0x01, "15")
	PORT_DIPSETTING(    0x02, "25")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
	PORT_DIPNAME( 0x04, 0x04, "Display Digits")				PORT_DIPLOCATION("S17-S24:!3")
	PORT_DIPSETTING(    0x00, "6 Digits")
	PORT_DIPSETTING(    0x04, "7 Digits")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2 (Coins/Credits)")    PORT_DIPLOCATION("S25-S32:!1,!2,!3,!4")
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x0f, "5 Coins/2 Credits (C3/1C, C5/1C)")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, "Same as Coin Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
	PORT_DIPNAME( 0x80, 0x00, "Playmates")					PORT_DIPLOCATION("S25-S32:!8")
	PORT_DIPSETTING(    0x00, "Reset At Next Ball")
	PORT_DIPSETTING(    0x80, "Remembered Next Ball")
INPUT_PORTS_END

static INPUT_PORTS_START( smman )
	PORT_INCLUDE( by35_30 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "Outlane Specials")           PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x20, "Both On")
	PORT_DIPNAME( 0x40, 0x00, "Saucer Score")               PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "Starts At 3000")
	PORT_DIPSETTING(    0x40, "Starts At 5000")
	PORT_DIPNAME( 0x80, 0x00, "Upper 0 and Lower 0")        PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Separate")
	PORT_DIPSETTING(    0x80, "Tied Together")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x10, 0x00, "50000 Special")                          PORT_DIPLOCATION("S25-S32:!5")
	PORT_DIPSETTING(    0x00, "Lights Making 50000 3 Times On A Ball")
	PORT_DIPSETTING(    0x10, "Lights Making 50000 2 Times On A Ball")
	PORT_DIPNAME( 0x60, 0x00, "Book Keeping For Games Played")          PORT_DIPLOCATION("S25-S32:!6,!7")
	PORT_DIPSETTING(    0x00, "Normal Game Mode")
	PORT_DIPSETTING(    0x40, "Show Player Counters On Power-Up")
	PORT_DIPSETTING(    0x60, "Reset Player Counters On Power-Up")

	PORT_MODIFY("X0")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X2")   /* Spinners */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1208)  // PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x2210)  // PORT_CODE(KEYCODE_V)

	PORT_MODIFY("X3")   /* Top Saucer */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END

static INPUT_PORTS_START( startrep )
	PORT_INCLUDE( by35_30 )

	PORT_MODIFY("SPINNER")                           // Allow user to select the simulated rotation speed of Spinners
	PORT_CONFNAME( 0x07, 0x00, "Spinner Simulation")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ))

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "Hyper Space Start Value")        PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, "2,000 Points")
	PORT_DIPSETTING(    0x20, "4,000 Points")
	PORT_DIPNAME( 0x40, 0x00, "B-A-L-L-Y Specials")             PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "Awards Once, Then 50,000")
	PORT_DIPSETTING(    0x40, "Remains Lit")
	PORT_DIPNAME( 0x80, 0x00, "Centre Target Light")            PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Alternates On and Off")
	PORT_DIPSETTING(    0x80, "Stays On")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x10, 0x00, "B-A-L-L-Y Starting Position")    PORT_DIPLOCATION("S25-S32:!5")
	PORT_DIPSETTING(    0x00, "10,000")
	PORT_DIPSETTING(    0x10, "25,000")
	PORT_DIPNAME( 0x20, 0x00, "Outlane Specials")               PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x20, "Both On")
	PORT_DIPNAME( 0x40, 0x00, "Flipper Return Lane Lights")     PORT_DIPLOCATION("S25-S32:!7")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x40, "Both On")


	PORT_MODIFY("X0")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)

	PORT_MODIFY("X3")   /* Top Saucer */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END

static INPUT_PORTS_START( startrekt )
	PORT_INCLUDE( startrep )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x0f, 0x02, "Coin Slot 1 (Coins/Credits)")    PORT_DIPLOCATION("S01-S08:!1,!2,!3,!4")  // Same as Coin Slot 3
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
	PORT_DIPNAME( 0x10, 0x00, "Tilt Through Protection")    PORT_DIPLOCATION("S01-S08:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, "Sound Options")              PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, "Chimes")
	PORT_DIPSETTING(    0x80, "Tunes and Noises")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x60, 0x60, "Awards")     PORT_DIPLOCATION("S09-S16:!6,!7")
	PORT_DIPSETTING(    0x00, "SL:None, Sp:50k, EB:25k")
	PORT_DIPSETTING(    0x20, "SL:Extra Ball, Sp:Extra Ball, EB:25k")
	PORT_DIPSETTING(    0x40, "SL:Extra Ball, Sp:Extra Ball, EB:Extra Ball")
	PORT_DIPSETTING(    0x60, "SL:Replay, Sp:Replay, EB:Extra Ball")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "Coin Slot 2")                PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x00, "Same as Coin Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPNAME( 0x02, 0x02, "B-A-L-L-Y Letters")          PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x00, "Reset At Next Ball")
	PORT_DIPSETTING(    0x02, "Remembered Next Ball")
	PORT_DIPNAME( 0x04, 0x04, "Hyper Space Upper Star Rollover Switch") PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x00, "Awards Only")
	PORT_DIPSETTING(    0x04, "Awards and Increments")
	PORT_DIPNAME( 0x08, 0x00, "Time Warp Bonus Collection")             PORT_DIPLOCATION("S25-S32:!4")
	PORT_DIPSETTING(    0x00, "Set Bonus Back to Original Value")
	PORT_DIPSETTING(    0x08, "Reset Bonus Back to 1000 points")
	PORT_DIPNAME( 0x80, 0x00, "Bonus Collect Speed and Sound")          PORT_DIPLOCATION("S25-S32:!8")
	PORT_DIPSETTING(    0x00, "Slow Factory 1000 Steps")
	PORT_DIPSETTING(    0x80, "Fast Bonus Multiplied Score Stepping")
INPUT_PORTS_END


static INPUT_PORTS_START( kiss )
	PORT_INCLUDE( by35_30 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "After Completing KISS Cards 3 Times")    PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, "Forget Extra KISS Achieved Next Ball")
	PORT_DIPSETTING(    0x20, "Remember Extra KISS Achieved Next Ball")
	PORT_DIPNAME( 0x40, 0x00, "Lite-A-Line Frequency")                  PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "Alternates Until Specials Is Lit")
	PORT_DIPSETTING(    0x40, "Lit Until Specials Is Lit")
	PORT_DIPNAME( 0x80, 0x00, "Kiss Specials")                          PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Lit After Colossal Bonus")
	PORT_DIPSETTING(    0x80, "Lit With Colossal Bonus")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x10, 0x00, "Open Gate When Lit Adjustment")  PORT_DIPLOCATION("S25-S32:!5")
	PORT_DIPSETTING(    0x00, "Lit Randomly (1 in 3)")
	PORT_DIPSETTING(    0x10, "Lit At Start Of New Ball")
	PORT_DIPNAME( 0x20, 0x00, "Lite-A-Line")                    PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, "Lit Randomly")
	PORT_DIPSETTING(    0x20, "Lit At Start Of New Ball")
	PORT_DIPNAME( 0x40, 0x00, "A-B-C-D Target Switches")        PORT_DIPLOCATION("S25-S32:!7")
	PORT_DIPSETTING(    0x00, "Reset Next Ball")
	PORT_DIPSETTING(    0x40, "Remembered Next Ball")

	PORT_MODIFY("X0")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)

	PORT_MODIFY("X3")   /* Spinners */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x2340)  // PORT_CODE(KEYCODE_S)
INPUT_PORTS_END


/*
    Dips for os35 - Harlem Globetrotters, Dolly, Future Spa, Nitro Ground Shaker, Silverball Mania, Space Invaders, Rolling Stones, Mystic, Hotdoggin', Viking
*/
static INPUT_PORTS_START( by35_35 )    // System ROM v35
	PORT_INCLUDE( by35 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x60, 0x60, "Score Level Awards")         PORT_DIPLOCATION("S01-S08:!6,!7")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x20, "Nothing")
	PORT_DIPSETTING(    0x40, "Extra Ball")
	PORT_DIPSETTING(    0x60, "Replay")
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 08 (Game Specific)")   PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 14 (Game Specific)")   PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 15 (Game Specific)")   PORT_DIPLOCATION("S09-S16:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 16 (Game Specific)")   PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2")                PORT_DIPLOCATION("S17-S24:!1,!2,!3,!4")
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
	PORT_DIPNAME( 0x30, 0x30, "Beating Highest Score Awards")   PORT_DIPLOCATION("S17-S24:!5,!6")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x10, "1 Credit")
	PORT_DIPSETTING(    0x20, "2 Credits")
	PORT_DIPSETTING(    0x30, "3 Credits")
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 23 (Game Specific)")       PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 24 (Game Specific)")       PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "Maximum Credits")            PORT_DIPLOCATION("S25-S32:!1,!2")
	PORT_DIPSETTING(    0x00, "10")
	PORT_DIPSETTING(    0x01, "15")
	PORT_DIPSETTING(    0x02, "25")
	PORT_DIPSETTING(    0x03, "40")
	PORT_DIPNAME( 0x04, 0x04, "Credits Displayed")          PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x08, "Match Feature")              PORT_DIPLOCATION("S25-S32:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x30, 0x30, "Sound Option")               PORT_DIPLOCATION("S25-S32:!5,!6")
	PORT_DIPSETTING(    0x00, "Most Scoring Has Chime Effects")
	PORT_DIPSETTING(    0x10, "Scoring Has Noise Effects, Background Off")
	PORT_DIPSETTING(    0x20, "Most Scoring Has Noise Effects")
	PORT_DIPSETTING(    0x30, "Scoring Has Noise Effects, Background On")
	PORT_DIPNAME( 0x40, 0x40, "Balls Per Game")             PORT_DIPLOCATION("S25-S32:!7")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x40, "5")
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 32 (Game Specific)")   PORT_DIPLOCATION("S25-S32:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

INPUT_PORTS_END

static INPUT_PORTS_START( slbmania )
	PORT_INCLUDE( by35_35 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x80, 0x00, "Silverball Carryover Awards")    PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, "Nothing")   PORT_CONDITION("DSW2",0x40,NOTEQUALS,0x40)
	PORT_DIPSETTING(    0x80, "Nothing")   PORT_CONDITION("DSW2",0x40,NOTEQUALS,0x40)
	PORT_DIPSETTING(    0x00, "1 Credit")  PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x80, "3 Credits") PORT_CONDITION("DSW2",0x40,EQUALS,0x40)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x40, 0x00, "Silverball Carryover Advances")  PORT_DIPLOCATION("S09-S16:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))                   PORT_CONDITION("DSW2",0x40,NOTEQUALS,0x40)
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))                   PORT_CONDITION("DSW2",0x40,NOTEQUALS,0x40)
	PORT_DIPSETTING(    0x00, "On Kicker Special")              PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x40, "On Silverball Mania Special")    PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPNAME( 0x80, 0x00, "Kicker Specials")                PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, "Lights After Silverball Mania Specials")
	PORT_DIPSETTING(    0x80, "Lights With Silverball Mania Specials")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x40, 0x00, "Silverball Carryover")               PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Middle Hoop Silverball Mania")       PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Spot 1 Letter")
	PORT_DIPSETTING(    0x80, "Spot 2 Letters")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x80, 0x00, "Middle Target Extra Ball Lights")    PORT_DIPLOCATION("S25-S32:!8")
	PORT_DIPSETTING(    0x00, "After 5X Potential Bonus")
	PORT_DIPSETTING(    0x80, "With 5X Potential Bonus")

	PORT_MODIFY("X0")   /* Kicker switch */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, kicker, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)

	PORT_MODIFY("X3")   /* Spinner Right */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1301)  // PORT_CODE(KEYCODE_K)

	PORT_MODIFY("X4")   /* Spinner Left */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x2401)  // PORT_CODE(KEYCODE_O)

	PORT_START("X5")    /* Fake switch but we use this as a helper for the layout system to show the state of the Kicker */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0501)
INPUT_PORTS_END

static INPUT_PORTS_START( hglbtrtr )
	PORT_INCLUDE( by35_35 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x80, 0x00, "Inline Drop Targets Reset")                   PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, "When Ball Enters Inline Drop Target Lane Saucer")
	PORT_DIPSETTING(    0x80, "At Next Ball in Play")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x20, "G-L-O-B-E Saucer Lamps")        PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x20, "Scan")
	PORT_DIPSETTING(    0x00, "Step One at a Time")
	PORT_DIPNAME( 0x40, 0x00, "Super Bonus memory")            PORT_DIPLOCATION("S09-S16:!7")
	PORT_DIPSETTING(    0x00, "Reset at the Start of New Ball")
	PORT_DIPSETTING(    0x40, "Remembered every Ball")
	PORT_DIPNAME( 0x80, 0x00, "Centre Spinners")               PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, "Spinners Alternate on Multiple 300 or 3000 Scores")
	PORT_DIPSETTING(    0x80, "Both Spinners On (On Advaance Bonus)")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x40, 0x00, "Right Side Slam Dunk Target")                 PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "5 Left Targets Must Be Made Again to Relite Special")
	PORT_DIPSETTING(    0x40, "Special Stays On for the Remainder of the Ball")
	PORT_DIPNAME( 0x80, 0x80, "Left Standup Targets")                 PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Targets Reset Next Ball")
	PORT_DIPSETTING(    0x80, "Any Targets Made Remembered Next Ball")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x80, 0x00, "Spotting G-L-O-B-E 2nd Time")             PORT_DIPLOCATION("S25-S32:!8")
	PORT_DIPSETTING(    0x00, "Lite Special")
	PORT_DIPSETTING(    0x80, "Score Special")

	PORT_MODIFY("X0")   /* Drop Target Inline switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)

	PORT_MODIFY("X2")   /* Spinner Right */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x3201)  // PORT_CODE(KEYCODE_COMMA)
	/* Saucer Top */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")   /* Spinner Right Middle */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1301)  // PORT_CODE(KEYCODE_K)
	/* Saucer Right */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)

	PORT_MODIFY("X4")   /* Spinner Right Middle */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x2401)  // PORT_CODE(KEYCODE_K)
INPUT_PORTS_END

static INPUT_PORTS_START( ngndshkr )
	PORT_INCLUDE( by35_35 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x80, 0x00, "Outlane Specials")                   PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x80, "Both On")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x60, 0x60, "Outlane Special Lights When")        PORT_DIPLOCATION("S09-S16:!6,!7")
	PORT_DIPSETTING(    0x00, "Making Left and Right Bonus at 10k")
	PORT_DIPSETTING(    0x20, "Making Left and Right Bonus at 15k")
	PORT_DIPSETTING(    0x40, "Making Left and Right Bonus at ??k")
	PORT_DIPSETTING(    0x60, "Making Left and Right Bonus at 20k")
	PORT_DIPNAME( 0x80, 0x00, "Top Saucer Double Bonus")            PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, "Lights Every 3rd Time")
	PORT_DIPSETTING(    0x80, "Lights Every 2nd Time")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x40, 0x00, "20k, 30k Bonus 2X Adjustment")       PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "Making 20k or 30k bonus scores single when 2X is lit")
	PORT_DIPSETTING(    0x40, "Making 20k or 30k bonus scores double when 2X is lit")
	PORT_DIPNAME( 0x80, 0x80, "Attract Mode Sound")                 PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Disabled")
	PORT_DIPSETTING(    0x80, "Enabled")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x80, 0x00, "E and F rollover lanes")             PORT_DIPLOCATION("S25-S32:!8")
	PORT_DIPSETTING(    0x00, "Separate")
	PORT_DIPSETTING(    0x80, "Tied Together")

	PORT_MODIFY("X1")   /* Drop Target switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x1, (void *)0x0108)  // PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x1, (void *)0x0110)  // PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x1, (void *)0x0120)  // PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x1, (void *)0x0140)  // PORT_CODE(KEYCODE_L)

	PORT_MODIFY("X2")   /* Saucer Top */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")   /* Spinner Right */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1301)  // PORT_CODE(KEYCODE_K)
	/* Saucer Left */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END

static INPUT_PORTS_START( ngndshko )
	PORT_INCLUDE( ngndshkr )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 24 (Game Specific)")       PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))
INPUT_PORTS_END

static INPUT_PORTS_START( futurspa )
	PORT_INCLUDE( by35_35 )

	PORT_MODIFY("X0")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)

	PORT_MODIFY("X1")   /* Spinners */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1120)  // PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x2140)  // PORT_CODE(KEYCODE_L)

	PORT_MODIFY("X2")   /* Saucer */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")   /* Kickback Left */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
INPUT_PORTS_END

static INPUT_PORTS_START( spaceinv )
	PORT_INCLUDE( by35_35 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x80, 0x80, "Flipper Blast Noise")                PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Pop Bumpers Score")                  PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, "100 points")
	PORT_DIPSETTING(    0x20, "1000 points")
	PORT_DIPNAME( 0x40, 0x40, "Background Thumping Sound")          PORT_DIPLOCATION("S09-S16:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Small Flipper Return Lane Lights")   PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, "Separate")
	PORT_DIPSETTING(    0x80, "Tied Together")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x40, 0x00, "Red Invader Bonus")                  PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "Resets Between Balls")
	PORT_DIPSETTING(    0x40, "Remembered Between Balls")
	PORT_DIPNAME( 0x80, 0x00, "Top Centre Rollover When Lit")       PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Scores Clone Chamber Value")
	PORT_DIPSETTING(    0x80, "Scores and Increases Clone Chamber Value")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x80, 0x00, "Left and Right Extra Ball Lane Lamps")       PORT_DIPLOCATION("S25-S32:!8")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x80, "Both On")

	PORT_MODIFY("X0")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	/* Spinner */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1008)         // PORT_CODE(KEYCODE_SLASH)

	PORT_MODIFY("X4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0402)  // PORT_CODE(KEYCODE_I)
INPUT_PORTS_END

static INPUT_PORTS_START( viking )
	PORT_INCLUDE( by35_35 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x80, 0x80, "Inline Red Rear Target Awards")      PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, "No Bonus Advance")
	PORT_DIPSETTING(    0x80, "5 Bonus Advances")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Flipper Return Lane Special and Left Saucer 3000")  PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, "Resets Between Balls")
	PORT_DIPSETTING(    0x20, "Remembered Between Balls")
	PORT_DIPNAME( 0x40, 0x00, "25000 Outlanes")                                    PORT_DIPLOCATION("S09-S16:!7")
	PORT_DIPSETTING(    0x00, "Alternate")
	PORT_DIPSETTING(    0x40, "Both On")
	PORT_DIPNAME( 0x80, 0x00, "3-Bank Drop Target Value")                          PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, "Resets Between Balls")
	PORT_DIPSETTING(    0x80, "Remembered Between Balls")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x40, 0x00, "InLine Drop Target Value")           PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "Resets Between Balls")
	PORT_DIPSETTING(    0x40, "Remembered Between Balls")
	PORT_DIPNAME( 0x80, 0x00, "Pop Bumpers Score")                  PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "100 points")
	PORT_DIPSETTING(    0x80, "1000 points")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x80, 0x00, "Rollover Button")                    PORT_DIPLOCATION("S25-S32:!8")
	PORT_DIPSETTING(    0x00, "Extra Ball and/or Special Reset At Next Ball")
	PORT_DIPSETTING(    0x80, "Extra Ball or Special Held Until Collected")

	PORT_MODIFY("X0")   /* Drop Target Inline switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)

	PORT_MODIFY("X1")   /* Spinner */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1120)         // PORT_CODE(KEYCODE_COLON)

	PORT_MODIFY("X2")   /* Drop Target 3 bank switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0201)  // PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0202)  // PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0204)  // PORT_CODE(KEYCODE_N)
	/* Saucer Top Middle */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")   /* Saucer Top Left */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END


/*
    Dips for os40 - Skateball, Frontier, Xenon
*/
static INPUT_PORTS_START( by35_40 )    // System ROM v40
	PORT_INCLUDE( by35_machine )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x1f, 0x00, "Coin Slot 1 (Coins/Credits)")    PORT_DIPLOCATION("S01-S08:!1,!2,!3,!4,!5")  // Same as Coin Slot 3
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x18, "4 Coins/3 Credits (C1/0C, C2/1C, C3/0C, C4/2C)")
	PORT_DIPSETTING(    0x17, "4 Coins/3 Credits (C1/0C, C2/1C, C3/1C, C4/1C)")
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x19, "4 Coins/5 Credits (C1/1C, C2/1C, C3/1C, C4/2C)")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x1d, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x1e, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x1f, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x16, "2 Coins/3 Credits (C1/1C, C2/2C)")
	PORT_DIPSETTING(    0x1a, "4 Coins/7 Credits (C1/1C, C2/2C, C3/1C, C4/3C)")
	PORT_DIPSETTING(    0x1b, "4 Coins/7 Credits (C1/1C, C2/2C, C3/2C, C4/2C)")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x13, "2 Coins/9 Credits")
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x14, "2 Coins/12 Credits")
	PORT_DIPSETTING(    0x09, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x15, "2 Coins/14 Credits")
	PORT_DIPSETTING(    0x0a, "1 Coin/14 Credits")
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 06 (Game Specific)")       PORT_DIPLOCATION("S01-S08:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 07 (Game Specific)")       PORT_DIPLOCATION("S01-S08:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 08 (Game Specific)")       PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x1f, 0x00, "Coin Slot 3 (Coins/Credits)")    PORT_DIPLOCATION("S09-S16:!1,!2,!3,!4,!5")  // Same as Coin Slot 1
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x18, "4 Coins/3 Credits (C1/0C, C2/1C, C3/0C, C4/2C)")
	PORT_DIPSETTING(    0x17, "4 Coins/3 Credits (C1/0C, C2/1C, C3/1C, C4/1C)")
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x19, "4 Coins/5 Credits (C1/1C, C2/1C, C3/1C, C4/2C)")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x1d, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x1e, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x1f, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x16, "2 Coins/3 Credits (C1/1C, C2/2C)")
	PORT_DIPSETTING(    0x1a, "4 Coins/7 Credits (C1/1C, C2/2C, C3/1C, C4/3C)")
	PORT_DIPSETTING(    0x1b, "4 Coins/7 Credits (C1/1C, C2/2C, C3/2C, C4/2C)")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x13, "2 Coins/9 Credits")
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x14, "2 Coins/12 Credits")
	PORT_DIPSETTING(    0x09, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x15, "2 Coins/14 Credits")
	PORT_DIPSETTING(    0x0a, "1 Coin/14 Credits")
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 14 (Game Specific)")       PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 15 (Game Specific)")       PORT_DIPLOCATION("S09-S16:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 16 (Game Specific)")       PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3,!4")
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
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 21 (Game Specific)")       PORT_DIPLOCATION("S17-S24:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 22 (Game Specific)")       PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 23 (Game Specific)")       PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 24 (Game Specific)")       PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "Maximum Credits")                PORT_DIPLOCATION("S25-S32:!1,!2")
	PORT_DIPSETTING(    0x00, "10")
	PORT_DIPSETTING(    0x01, "15")
	PORT_DIPSETTING(    0x02, "25")
	PORT_DIPSETTING(    0x03, "40")
	PORT_DIPNAME( 0x04, 0x04, "Credits Displayed")              PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x08, "Match Feature")                  PORT_DIPLOCATION("S25-S32:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 29 (Game Specific)")       PORT_DIPLOCATION("S25-S32:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 30 (Game Specific)")       PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xc0, 0x40, "Balls Per Game")                 PORT_DIPLOCATION("S25-S32:!7,!8")
	PORT_DIPSETTING(    0xc0, "2")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x80, "4")
	PORT_DIPSETTING(    0x40, "5")

	PORT_INCLUDE( by35_switch_matrix_x0_x4 )
INPUT_PORTS_END

static INPUT_PORTS_START( frontier )
	PORT_INCLUDE( by35_40 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x20, 0x00, "A and C Rollover Lane Lite")                                 PORT_DIPLOCATION("S01-S08:!6")
	PORT_DIPSETTING(    0x00, "Making A or C Lane Lites Only Puts That Lane Out")
	PORT_DIPSETTING(    0x20, "Making A or C Lane Puts Both Lites Out")
	PORT_DIPNAME( 0x40, 0x00, "A-B-C Extra Ball Lite")                                      PORT_DIPLOCATION("S01-S08:!7")
	PORT_DIPSETTING(    0x00, "A-B-C Extra Ball Lite Comes On After Making A-B-C Lanes")
	PORT_DIPSETTING(    0x40, "A-B-C Extra Ball Lite Is On At Start Of Game")
	PORT_DIPNAME( 0x80, 0x00, "Special Outlane Lite")                                       PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, "1 Lite Comes On Then Alternates")
	PORT_DIPSETTING(    0x80, "Both Lites Come On")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Special Outlane Lites And Side Targets Adjustment")          PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, "Special Lites Come On When Side Drop Target Special Is On")
	PORT_DIPSETTING(    0x20, "Special Lites Come On When Side Drop Target 4X Is On")
	PORT_DIPNAME( 0x40, 0x00, "A-B-C Lane Lites Recall")                                    PORT_DIPLOCATION("S09-S16:!7")
	PORT_DIPSETTING(    0x00, "Any A-B-C Lite Out Will Come Back For Next Ball")
	PORT_DIPSETTING(    0x40, "Any A-B-C Lite Out Will Not Come Back On For Next Ball")
	PORT_DIPNAME( 0x80, 0x00, "Frontier Bonus Score")                                       PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, "Going In Outhole Frontier Bonus Will Not Be Collected")
	PORT_DIPSETTING(    0x80, "Going In Outhole Frontier Bonus Will Be Collected")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 21 (unused)")                                          PORT_DIPLOCATION("S17-S24:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 22 (unused)")                                          PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "Frontier Bonus Lite Adjustment")                             PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "Frontier Bonus Will Stop At 60,000")
	PORT_DIPSETTING(    0x40, "Frontier Bonus Will Stop At 110,000")
	PORT_DIPNAME( 0x80, 0x00, "Gate Recall")                                                PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Gate Will Stay Down For Next Ball")
	PORT_DIPSETTING(    0x80, "Gate Will Keep Up For Next Ball")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 29 (unused)")                                          PORT_DIPLOCATION("S25-S32:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "Flipper Feed Lane 15,000")                                   PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, "1 Lite Comes On Then Alternates")
	PORT_DIPSETTING(    0x20, "Both Lites Come On")

	PORT_MODIFY("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("30 Point Rebound") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Outlane Right") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Flipper Feed Lane Right") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Flipper Feed Lane Left") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Outlane Left") PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )   PORT_NAME("Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Outhole") PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, outhole, (void *)0x0080)  // PORT_CODE(KEYCODE_BACKSPACE)

	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 )  PORT_NAME("Coin3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )  PORT_NAME("Coin1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )  PORT_NAME("Coin2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Saucer") PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x1, (void *)0x0108)  // PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A Rollover Lane") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("B Rollover Lane") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C Rollover Lane") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT2 )  PORT_NAME("Slam Tilt") PORT_CODE(KEYCODE_EQUALS)

	PORT_MODIFY("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("InLine Drop Target 3rd") PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("InLine Drop Target 2nd") PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("InLine Drop Target 1st") PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Grizzly Target") PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Saucer Rollover Button") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Spinner") PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1304)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Standup Target Left") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Standup Target Middle") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Standup Target Right") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_CODE(KEYCODE_A)

	PORT_MODIFY("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Drop Target Bottom Side") PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0401)  // PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Drop Target Middle Side") PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0402)  // PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Drop Target Top Side")    PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0404)  // PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Slingshot Right") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Slingshot Left") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Thumper Bumper Bottom") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Thumper Bumper Right") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Thumper Bumper Left") PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END

static INPUT_PORTS_START( skatebll )
	PORT_INCLUDE( by35_40 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x40, 0x00, "Each Drop Target (At Both 3-Banks) Award")     PORT_DIPLOCATION("S01-S08:!7")
	PORT_DIPSETTING(    0x00, "No Bonus Advance")
	PORT_DIPSETTING(    0x40, "1 Bonus Advance")
	PORT_DIPNAME( 0x80, 0x00, "Drop Targets Top Spot X")                      PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, "Resets Between Balls")
	PORT_DIPSETTING(    0x80, "Remembered Between Balls")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Drop Targets Centre Arrow")                    PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, "Resets Between Balls")
	PORT_DIPSETTING(    0x20, "Remembered Between Balls")
	PORT_DIPNAME( 0x40, 0x00, "Drop Targets Top Arrow")                       PORT_DIPLOCATION("S09-S16:!7")
	PORT_DIPSETTING(    0x00, "Resets Between Balls")
	PORT_DIPSETTING(    0x40, "Remembered Between Balls")
	PORT_DIPNAME( 0x80, 0x00, "50000 and Saucer Extra Ball")                  PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, "50000 Lights First then Extra Ball")
	PORT_DIPSETTING(    0x80, "Light Together")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x40, 0x00, "Bonus Multiplier")                             PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "Resets Between Balls")
	PORT_DIPSETTING(    0x40, "Remembered Between Balls")
	PORT_DIPNAME( 0x80, 0x00, "Saucer Extra Ball And Right Outlane Special")  PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Extra Ball Lights First then Special")
	PORT_DIPSETTING(    0x80, "Light Together")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x20, 0x20, "Saucer Extra Ball")                            PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, "Disabled")
	PORT_DIPSETTING(    0x20, "Enabled")

	PORT_MODIFY("X0")   /* Drop Target Middle 3 Bank switches */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	/* Saucer */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X2")   /* Drop Targets Left Middle 5 Bank switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0208)  // PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")   /* Drop Target Left Top 3 Bank switches */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0302)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)

	PORT_MODIFY("X4")   /* Spinner */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1401)		  // PORT_CODE(KEYCODE_O)
INPUT_PORTS_END

static INPUT_PORTS_START( xenon )
	PORT_INCLUDE( by35_40 )

	PORT_MODIFY("X0")   /* Trough switch #1 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	/* Spinner */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1010)		  // PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, outhole, (void *)0x0080)		  // PORT_CODE(KEYCODE_BACKSPACE)

	PORT_MODIFY("X2")   /* Drop Targets */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")   /* Trough switch #2 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)

	PORT_MODIFY("X4")   /* Saucers */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0401)  // PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0402)  // PORT_CODE(KEYCODE_I)
INPUT_PORTS_END


/*
    Dips for os52 - Flash Gordon, Eight Blall Deluxe, etc
*/
static INPUT_PORTS_START( by35_52 )    // System ROM v52
	PORT_INCLUDE( by35_machine )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x1f, 0x00, "Coin Slot 1 (Coins/Credits)")    PORT_DIPLOCATION("S01-S08:!1,!2,!3,!4,!5")  // Same as Coin Slot 3
	PORT_DIPSETTING(    0x1e, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(    0x1d, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x1c, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x1f, "5 Coins/2 Credits (C1/0C, C2/0C, C3/1C, C4/0C, C5/1C)")
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x18, "4 Coins/3 Credits (C1/0C, C2/1C, C3/0C, C4/2C)")
	PORT_DIPSETTING(    0x17, "4 Coins/3 Credits (C1/0C, C2/1C, C3/1C, C4/1C)")
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x19, "4 Coins/5 Credits (C1/1C, C2/1C, C3/1C, C4/2C)")
	PORT_DIPSETTING(    0x16, "2 Coins/3 Credits (C1/1C, C2/2C)")
	PORT_DIPSETTING(    0x1a, "4 Coins/7 Credits (C1/1C, C2/2C, C3/1C, C4/3C)")
	PORT_DIPSETTING(    0x1b, "4 Coins/7 Credits (C1/1C, C2/2C, C3/2C, C4/2C)")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x13, "2 Coins/9 Credits")
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x14, "2 Coins/12 Credits")
	PORT_DIPSETTING(    0x09, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x15, "2 Coins/14 Credits")
	PORT_DIPSETTING(    0x0a, "1 Coin/14 Credits")
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 06 (Game Specific)")       PORT_DIPLOCATION("S01-S08:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 07 (Game Specific)")       PORT_DIPLOCATION("S01-S08:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 08 (Game Specific)")       PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x1f, 0x00, "Coin Slot 3 (Coins/Credits)")    PORT_DIPLOCATION("S09-S16:!1,!2,!3,!4,!5")  // Same as Coin Slot 1
	PORT_DIPSETTING(    0x1e, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(    0x1d, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x1c, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x1f, "5 Coins/2 Credits (C1/0C, C2/0C, C3/1C, C4/0C, C5/1C)")
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x18, "4 Coins/3 Credits (C1/0C, C2/1C, C3/0C, C4/2C)")
	PORT_DIPSETTING(    0x17, "4 Coins/3 Credits (C1/0C, C2/1C, C3/1C, C4/1C)")
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x19, "4 Coins/5 Credits (C1/1C, C2/1C, C3/1C, C4/2C)")
	PORT_DIPSETTING(    0x16, "2 Coins/3 Credits (C1/1C, C2/2C)")
	PORT_DIPSETTING(    0x1a, "4 Coins/7 Credits (C1/1C, C2/2C, C3/1C, C4/3C)")
	PORT_DIPSETTING(    0x1b, "4 Coins/7 Credits (C1/1C, C2/2C, C3/2C, C4/2C)")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x13, "2 Coins/9 Credits")
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x14, "2 Coins/12 Credits")
	PORT_DIPSETTING(    0x09, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x15, "2 Coins/14 Credits")
	PORT_DIPSETTING(    0x0a, "1 Coin/14 Credits")
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 14 (Game Specific)")       PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 15 (Game Specific)")       PORT_DIPLOCATION("S09-S16:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 16 (Game Specific)")       PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3,!4")
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
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 21 (Game Specific)")       PORT_DIPLOCATION("S17-S24:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 22 (Game Specific)")       PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 23 (Game Specific)")       PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 24 (Game Specific)")       PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "Maximum Credits")                PORT_DIPLOCATION("S25-S32:!1,!2")
	PORT_DIPSETTING(    0x00, "10")
	PORT_DIPSETTING(    0x01, "15")
	PORT_DIPSETTING(    0x02, "25")
	PORT_DIPSETTING(    0x03, "40")
	PORT_DIPNAME( 0x04, 0x04, "Credits Displayed")              PORT_DIPLOCATION("S25-S32:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x08, "Match Feature")                  PORT_DIPLOCATION("S25-S32:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x10, "Number of Replays per Game")     PORT_DIPLOCATION("S25-S32:!5")
	PORT_DIPSETTING(    0x00, "1 Replay Per Player Per Game")
	PORT_DIPSETTING(    0x10, "All Replays Are Collected")
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 30 (Game Specific)")       PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xc0, 0x40, "Balls Per Game")                 PORT_DIPLOCATION("S25-S32:!7,!8")
	PORT_DIPSETTING(    0xc0, "2")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x80, "4")
	PORT_DIPSETTING(    0x40, "5")

	PORT_INCLUDE( by35_switch_matrix_x0_x4 )
INPUT_PORTS_END

static INPUT_PORTS_START( flashgdn )
	PORT_INCLUDE( by35_52 )

	PORT_MODIFY("X0")   /* Drop Target Single Upper */
	/* Drop Target Single */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)

	PORT_MODIFY("X2")   /* Drop Targets Right 4 Bank switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0201)  // PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0202)  // PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0204)  // PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0208)  // PORT_CODE(KEYCODE_B)
	/* Drop Targets Right 3 Bank switches */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)

	PORT_MODIFY("X3")   /* Drop Target Inline 3 Bank */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0301)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0302)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
 	/* Saucer */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)

	PORT_MODIFY("X4")   /* Spinners */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1401) 		  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x2402) 		  // PORT_CODE(KEYCODE_J)
INPUT_PORTS_END

static INPUT_PORTS_START( fball_ii )
	PORT_INCLUDE( by35_52 )

	PORT_MODIFY("X0")   /* Trough and Outhole switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, outhole, (void *)0x0004)		  // PORT_CODE(KEYCODE_BACKSPACE)
	/* Saucer Top Right Wotans */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	/* Saucer Top Left Odin */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)

	PORT_MODIFY("X2")   /* Trough #1 switch */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0202)  // PORT_CODE(KEYCODE_M)
	/* Drop Targets Right 4 Bank switches */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")   /* Drop Target Left 4 Bank switches */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0340)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)

	PORT_MODIFY("X4")   /* Drop Target Middle switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0401)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0402)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0404)  // PORT_CODE(KEYCODE_H)
INPUT_PORTS_END

static INPUT_PORTS_START( eballdlx )
	PORT_INCLUDE( by35_52 )

	PORT_MODIFY("X0")   /* Drop Targets Left Inline switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)

	PORT_MODIFY("X2")   /* Drop Targets Right 7 Bank switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0201)  // PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0202)  // PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0204)  // PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0208)  // PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)

	PORT_MODIFY("X4")   /* Drop Target Single and Saucer */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0401)  // PORT_CODE(KEYCODE_O)
	/* Saucer */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0402)  // PORT_CODE(KEYCODE_I)
INPUT_PORTS_END

static INPUT_PORTS_START( worlddeft )
	PORT_INCLUDE( by35_52 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3,!4")     PORT_CONDITION("DSW3", 0x03, NOTEQUALS, 0x03)
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
	PORT_DIPNAME( 0x07, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3")        PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "Same as Coin Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
	PORT_DIPNAME( 0x08, 0x00, "Freeplay Mode")                  PORT_DIPLOCATION("S17-S24:!4")              PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "Credit Reset Mode")
	PORT_DIPSETTING(    0x08, "Credit Count Mode")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "Maximum Credits")                PORT_DIPLOCATION("S25-S32:!1,!2")
	PORT_DIPSETTING(    0x00, "05")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "20")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
INPUT_PORTS_END


static INPUT_PORTS_START( embryon )
	PORT_INCLUDE( by35_52 )

	PORT_MODIFY("X0")	/* Saucer */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	/* Outhole and Trough switches */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, outhole, (void *)0x0010)         // PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0080)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X2")   /* Drop Target Single */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0201)  // PORT_CODE(KEYCODE_COMMA)
	/* Spinner */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1202) 		  // PORT_CODE(KEYCODE_M)
	/* Drop Targets Top 3 Bank switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0208)  // PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)

	PORT_MODIFY("X3")   /* Drop Target Left 3 Bank switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0301)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0302)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
INPUT_PORTS_END


/*
    Dips for os53 - Fathom, Centaur, Medusa, Etc
*/
static INPUT_PORTS_START( by35_53 )    // System ROM v53 (same system DipSW's as v52)
	PORT_INCLUDE( by35_52 )

	PORT_MODIFY("X0")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)

	PORT_MODIFY("X1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )

	PORT_INCLUDE( by35_switch_matrix_x5 )
INPUT_PORTS_END

static INPUT_PORTS_START( fathom )
	PORT_INCLUDE( by35_53 )

	PORT_MODIFY("X0")   /* Trough and Outhole switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, outhole, (void *)0x0004)		  // PORT_CODE(KEYCODE_BACKSPACE)
	/* Saucer Top */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	/* Saucer Right */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)

	PORT_MODIFY("X2")   /* Spinner */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1202) 		  // PORT_CODE(KEYCODE_M)

	PORT_MODIFY("X3")   /* Drop Target Left switches */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0340)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)

	PORT_MODIFY("X4")   /* Drop Target Middle switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0401)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0402)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0404)  // PORT_CODE(KEYCODE_H)

	PORT_MODIFY("X5")
	/* Drop Targets Blue Inline Top switches */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0502)  // PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0504)  // PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0508)  // PORT_CODE(KEYCODE_4_PAD)
	/* Drop Targets Green Inline Left switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0520)  // PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0540)  // PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0580)  // PORT_CODE(KEYCODE_8_PAD)
INPUT_PORTS_END

static INPUT_PORTS_START( centaur )
	PORT_INCLUDE( by35_53 )

	PORT_MODIFY("X0")   /* Trough #4, Trough #5 and Outhole switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, outhole, (void *)0x0080)		  // PORT_CODE(KEYCODE_BACKSPACE)

	PORT_MODIFY("X2")   /* Trough #1 switch */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0201)  // PORT_CODE(KEYCODE_COMMA)

	PORT_MODIFY("X3")   /* Drop Targets, Right Side 1234 and Front Centre ORBS switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0301)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0302)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0340)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)

	PORT_MODIFY("X5")   /* Drop Targets Left Side Inline switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0501)  // PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0502)  // PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0504)  // PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0508)  // PORT_CODE(KEYCODE_4_PAD)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x20, 0x00, "Bonus Multiplier")   PORT_DIPLOCATION("S01-S08:!6")
	PORT_DIPSETTING(    0x00, "Resets Between Balls")
	PORT_DIPSETTING(    0x20, "Remembered Between Balls")
	PORT_DIPNAME( 0x40, 0x00, "Bonus Control")      PORT_DIPLOCATION("S01-S08:!7")
	PORT_DIPSETTING(    0x00, "Only 20k, 40k And 60k Are Remembered Between Balls")
	PORT_DIPSETTING(    0x40, "All Remembered Between Balls")
	PORT_DIPNAME( 0x80, 0x00, "Sequence Feature Drop Targets")   PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, "Any Targets Down Will Reset For Next Ball")
	PORT_DIPSETTING(    0x80, "Only Drop Targets Down In Sequence 1-3 Will Drop Next Ball")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Chamber Feature")    PORT_DIPLOCATION("S09-S16:!6")
	PORT_DIPSETTING(    0x00, "Any Feature Light ON Will Come ON For Next Ball")
	PORT_DIPSETTING(    0x20, "Any Feature Light ON Will Reset To 10 For Next Ball")
	PORT_DIPNAME( 0x40, 0x00, "Captive Orbs")       PORT_DIPLOCATION("S09-S16:!7")
	PORT_DIPSETTING(    0x00, "Lit Orbs Reset At End Of Ball")
	PORT_DIPSETTING(    0x40, "Lit Orbs Remain Lit Until Their Release")
	PORT_DIPNAME( 0x80, 0x00, "Guardian Feature")   PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, "Guardians Can Only Release One Ball Per Ball")
	PORT_DIPSETTING(    0x80, "Guardians Release A Ball Every Time")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x10, 0x00, "Release On Last Ball")       PORT_DIPLOCATION("S17-S24:!5")
	PORT_DIPSETTING(    0x00, "Release Light Comes ON And Go On And Off")
	PORT_DIPSETTING(    0x10, "Release Light Comes ON And Stays ON")
	PORT_DIPNAME( 0x20, 0x00, "Captive Orb Initialisation") PORT_DIPLOCATION("S17-S24:!6")
	PORT_DIPSETTING(    0x00, "Player Must Earn All Lit Orbs")
	PORT_DIPSETTING(    0x20, "Each Ball Begins With At Least One Orb Lit")
	PORT_DIPNAME( 0x40, 0x40, "Tilt Feature")               PORT_DIPLOCATION("S17-S24:!7")
	PORT_DIPSETTING(    0x00, "First Tilt Warns, Second Tilt Causes Game Over For Player")
	PORT_DIPSETTING(    0x40, "Ball In Play Tilts")
	PORT_DIPNAME( 0x80, 0x00, "Right Release Target")       PORT_DIPLOCATION("S17-S24:!8")
	PORT_DIPSETTING(    0x00, "Must Be Qualified On Each Ball")
	PORT_DIPSETTING(    0x80, "Lit Release Target Remains Lit Between Balls")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x10, 0x10, "Replays Per Game")           PORT_DIPLOCATION("S25-S32:!5")
	PORT_DIPSETTING(    0x00, "Only 1 Replay Per Player Per Game")
	PORT_DIPSETTING(    0x10, "All Replays Earned Are Awarded")
	PORT_DIPNAME( 0x20, 0x20, "Game Over Animation")        PORT_DIPLOCATION("S25-S32:!6")
	PORT_DIPSETTING(    0x00, "No Kickout Balls")
	PORT_DIPSETTING(    0x20, "5 Balls Will Kickout To Playfield Every 15 Minutes")
INPUT_PORTS_END


static INPUT_PORTS_START( medusa )
	PORT_INCLUDE( by35_53 )

	PORT_MODIFY("X0")   /* Drop Targets Right switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)

	PORT_MODIFY("X2")   /* Saucer */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0204)  // PORT_CODE(KEYCODE_B)

	PORT_MODIFY("X4")   /* Spinner */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1401)  // PORT_CODE(KEYCODE_O)

	PORT_MODIFY("X5")   /* Zipper Flippers and Drop Targets Upper switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0501)  // PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0502)  // PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0504)  // PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0508)  // PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0510)  // PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0520)  // PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0540)  // PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0580)  // PORT_CODE(KEYCODE_8_PAD)
INPUT_PORTS_END

static INPUT_PORTS_START( vector )
	PORT_INCLUDE( by35_53 )

	PORT_MODIFY("X0")   /* Trough and Outhole switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, outhole, (void *)0x0004)		  // PORT_CODE(KEYCODE_BACKSPACE)
	/* Saucer Bottom Right */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	/* Saucer Bottom left */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)

	PORT_MODIFY("X2")   /* Saucers Captive switches; Bottom, Middle and Top */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0201)  // PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0202)  // PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0204)  // PORT_CODE(KEYCODE_N)

	PORT_MODIFY("X3")   /* Drop Target Lower switches */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0302)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
	/* Drop Target Upper switches */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0340)  // PORT_CODE(KEYCODE_S)

	PORT_MODIFY("X5")
	/* Drop Targets X-Y-Z switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0520)  // PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0540)  // PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x5, (void *)0x0580)  // PORT_CODE(KEYCODE_8_PAD)
INPUT_PORTS_END


static INPUT_PORTS_START( m_mpac )
	PORT_INCLUDE( by35_53 )

	PORT_MODIFY("X0")   /* Drop Targets Left, Saucers and Outhole switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, outhole, (void *)0x0010)  // PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0040)  // PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0080)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X2")   /* Drop Target Top switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("X3")   /* Drop Target Right switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0301)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0302)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0304)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)
	/* Spinner */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1320)		  // PORT_CODE(KEYCODE_D)
INPUT_PORTS_END


static INPUT_PORTS_START( goldball )
	PORT_INCLUDE( by35_53 )

	PORT_MODIFY("X3")   /* Trough switch Goldball */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
INPUT_PORTS_END


static INPUT_PORTS_START( kosteel )
	PORT_INCLUDE( by35_53 )

	PORT_MODIFY("X0")   /* Saucer switch */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)

	PORT_MODIFY("X4")   /* Drop Target switches */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0404)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0408)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0410)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0420)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0440)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x4, (void *)0x0480)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END

static INPUT_PORTS_START( tigerragt )
	PORT_INCLUDE( kosteel )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3,!4")     PORT_CONDITION("DSW3", 0x03, NOTEQUALS, 0x03)
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
	PORT_DIPNAME( 0x07, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3")        PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "Same as Coin Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
	PORT_DIPNAME( 0x08, 0x00, "Freeplay Mode")                  PORT_DIPLOCATION("S17-S24:!4")              PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "Credit Reset Mode")
	PORT_DIPSETTING(    0x08, "Credit Count Mode")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "Maximum Credits")                PORT_DIPLOCATION("S25-S32:!1,!2")
	PORT_DIPSETTING(    0x00, "05")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "20")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
INPUT_PORTS_END


static INPUT_PORTS_START( xsandos )
	PORT_INCLUDE( by35_53 )

	PORT_MODIFY("X0")   /* Drop Targets Left, Saucers and Outhole switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
   /* Saucers */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0008)  // PORT_CODE(KEYCODE_SLASH)
INPUT_PORTS_END

static INPUT_PORTS_START( suprbowlt )
	PORT_INCLUDE( xsandos )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3,!4")     PORT_CONDITION("DSW3", 0x03, NOTEQUALS, 0x03)
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
	PORT_DIPNAME( 0x07, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3")        PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "Same as Coin Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
	PORT_DIPNAME( 0x08, 0x00, "Freeplay Mode")                  PORT_DIPLOCATION("S17-S24:!4")              PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "Credit Reset Mode")
	PORT_DIPSETTING(    0x08, "Credit Count Mode")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "Maximum Credits")                PORT_DIPLOCATION("S25-S32:!1,!2")
	PORT_DIPSETTING(    0x00, "05")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "20")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
INPUT_PORTS_END


static INPUT_PORTS_START( blakpyra )
	PORT_INCLUDE( by35_53 )

	PORT_MODIFY("X0")   /* Saucer switch */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0040)  // PORT_CODE(KEYCODE_L)

	PORT_MODIFY("X3")   /* Drop Target switches */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0340)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0380)  // PORT_CODE(KEYCODE_A)
INPUT_PORTS_END

static INPUT_PORTS_START( newwavet )
	PORT_INCLUDE( blakpyra )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3,!4")     PORT_CONDITION("DSW3", 0x03, NOTEQUALS, 0x03)
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
	PORT_DIPNAME( 0x07, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3")        PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "Same as Coin Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
	PORT_DIPNAME( 0x08, 0x00, "Freeplay Mode")                  PORT_DIPLOCATION("S17-S24:!4")              PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "Credit Reset Mode")
	PORT_DIPSETTING(    0x08, "Credit Count Mode")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "Maximum Credits")                PORT_DIPLOCATION("S25-S32:!1,!2")
	PORT_DIPSETTING(    0x00, "05")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "20")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
INPUT_PORTS_END


static INPUT_PORTS_START( spyhuntr )
	PORT_INCLUDE( by35_53 )

	PORT_MODIFY("X0")   /* Saucers */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1040)		  // PORT_CODE(KEYCODE_L)

	PORT_MODIFY("X2")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0201)  // PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0202)  // PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0204)  // PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0208)  // PORT_CODE(KEYCODE_B)
INPUT_PORTS_END

static INPUT_PORTS_START( saturn2t )
	PORT_INCLUDE( spyhuntr )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3,!4")     PORT_CONDITION("DSW3", 0x03, NOTEQUALS, 0x03)
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
	PORT_DIPNAME( 0x07, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3")        PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "Same as Coin Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
	PORT_DIPNAME( 0x08, 0x00, "Freeplay Mode")                  PORT_DIPLOCATION("S17-S24:!4")              PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "Credit Reset Mode")
	PORT_DIPSETTING(    0x08, "Credit Count Mode")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "Maximum Credits")                PORT_DIPLOCATION("S25-S32:!1,!2")
	PORT_DIPSETTING(    0x00, "05")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "20")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
INPUT_PORTS_END

static INPUT_PORTS_START( fbclass )
	PORT_INCLUDE( by35_53 )

	PORT_MODIFY("X2")   /* Saucers */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)  Saucer Right
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)  Saucer Left

	PORT_MODIFY("X3")   /* Trough switches */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0308)  // PORT_CODE(KEYCODE_G)  Trough #3 (Left)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0310)  // PORT_CODE(KEYCODE_F)  Trough #2 (Middle)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x3, (void *)0x0320)  // PORT_CODE(KEYCODE_D)  Trough #1 (Right)
INPUT_PORTS_END


static INPUT_PORTS_START( cybrnaut )
	PORT_INCLUDE( by35_53 )

	PORT_MODIFY("X0")   /* Saucer */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0010)  // PORT_CODE(KEYCODE_STOP)

	PORT_MODIFY("X2")   /* Drop Target switches */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0210)  // PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0220)  // PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0240)  // PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x2, (void *)0x0280)  // PORT_CODE(KEYCODE_Z)
INPUT_PORTS_END


static INPUT_PORTS_START( spacehawt )
	PORT_INCLUDE( spyhuntr )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3,!4")     PORT_CONDITION("DSW3", 0x03, NOTEQUALS, 0x03)
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
	PORT_DIPNAME( 0x07, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!1,!2,!3")        PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "Same as Coin Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
	PORT_DIPNAME( 0x08, 0x00, "Freeplay Mode")                  PORT_DIPLOCATION("S17-S24:!4")              PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "Credit Reset Mode")
	PORT_DIPSETTING(    0x08, "Credit Count Mode")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "Maximum Credits")                PORT_DIPLOCATION("S25-S32:!1,!2")
	PORT_DIPSETTING(    0x00, "05")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "20")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
INPUT_PORTS_END

static INPUT_PORTS_START( bullseye )
	PORT_INCLUDE( by35_30 )

	PORT_MODIFY("X0")   /* Saucers */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0001)  // PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0002)  // PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, switch_hold_x0, (void *)0x0004)  // PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_MODIFY("X4")   /* Outhole and Spinner */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, outhole, (void *)0x0401)  // PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by35_state, spinner, (void *)0x1480)  // PORT_CODE(KEYCODE_Q)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x20, 0x00, "Scoring Direction")          PORT_DIPLOCATION("S01-S08:!6")
	PORT_DIPSETTING(    0x00, "Decrease from 301 to 0")
	PORT_DIPSETTING(    0x20, "Increase from 0 to 301")
	PORT_DIPNAME( 0x40, 0x00, "Inlane/Outlane lights")   PORT_DIPLOCATION("S01-S08:!7")
	PORT_DIPSETTING(    0x00, "One Extra Ball and One Special lamp lit")
	PORT_DIPSETTING(    0x40, "Both Extra Ball and Special lamps lit")
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 08 (Game Specific)")   PORT_DIPLOCATION("S01-S08:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x60, 0x60, "Beating Highest Score Awards")   PORT_DIPLOCATION("S09-S16:!6,!7")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x20, "1 Credit")
	PORT_DIPSETTING(    0x40, "2 Credits")
	PORT_DIPSETTING(    0x60, "3 Credits")
	PORT_DIPNAME( 0x80, 0x80, "Balls Per Game")             PORT_DIPLOCATION("S09-S16:!8")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x80, "5")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "Maximum Credits")            PORT_DIPLOCATION("S17-S24:!1,!2")
	PORT_DIPSETTING(    0x00, "10")
	PORT_DIPSETTING(    0x01, "20")
	PORT_DIPSETTING(    0x02, "30")
	PORT_DIPSETTING(    0x03, "40")
	PORT_DIPNAME( 0x0c, 0x04, "Score Level Awards")         PORT_DIPLOCATION("S17-S24:!3,!4")
	PORT_DIPSETTING(    0x00, "Extra Ball")
	PORT_DIPSETTING(    0x04, "Replay")
	PORT_DIPSETTING(    0x08, "Nothing")
	PORT_DIPSETTING(    0x0c, "Novelty")
	PORT_DIPNAME( 0xf0, 0x00, "Coin Slot 2")                PORT_DIPLOCATION("S17-S24:!5,!6,!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x90, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0xa0, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0xb0, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0xc0, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0xd0, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0xe0, "1 Coin/15 Credits")
	PORT_DIPSETTING(    0xf0, "1 Coin/16 Credits")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Credits Displayed")          PORT_DIPLOCATION("S25-S32:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x02, "Match Feature")              PORT_DIPLOCATION("S25-S32:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x1c, 0x00, "301 Game Specials")          PORT_DIPLOCATION("S25-S32:!3,!4,!5")
	PORT_DIPSETTING(    0x00, "05")
	PORT_DIPSETTING(    0x04, "06")
	PORT_DIPSETTING(    0x08, "07")
	PORT_DIPSETTING(    0x0c, "08")
	PORT_DIPSETTING(    0x10, "09")
	PORT_DIPSETTING(    0x14, "10")
	PORT_DIPSETTING(    0x18, "11")
	PORT_DIPSETTING(    0x1c, "12")
	PORT_DIPNAME( 0xe0, 0x00, "301 Game Extra Ball")        PORT_DIPLOCATION("S25-S32:!6,!7,!8")
	PORT_DIPSETTING(    0x00, "02")
	PORT_DIPSETTING(    0x20, "03")
	PORT_DIPSETTING(    0x40, "04")
	PORT_DIPSETTING(    0x60, "05")
	PORT_DIPSETTING(    0x80, "06")
	PORT_DIPSETTING(    0xa0, "07")
	PORT_DIPSETTING(    0xc0, "08")
	PORT_DIPSETTING(    0xe0, DEF_STR( Unknown ))
INPUT_PORTS_END

static INPUT_PORTS_START( bullseyet )
	PORT_INCLUDE( bullseye )
	
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "Maximum Credits")                PORT_DIPLOCATION("S17-S24:!1,!2")
	PORT_DIPSETTING(    0x00, "05")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "20")
	PORT_DIPSETTING(    0x03, "99 (Freeplay)")
	PORT_DIPNAME( 0xf0, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!5,!6,!7,!8")     PORT_CONDITION("DSW2", 0x03, NOTEQUALS, 0x03)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x90, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0xa0, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0xb0, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0xc0, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0xd0, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0xe0, "1 Coin/15 Credits")
	PORT_DIPSETTING(    0xf0, "1 Coin/16 Credits")
	PORT_DIPNAME( 0x70, 0x00, "Coin Slot 2")                    PORT_DIPLOCATION("S17-S24:!5,!6,!7")        PORT_CONDITION("DSW2", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ))
	PORT_DIPNAME( 0x80, 0x00, "Freeplay Mode")                  PORT_DIPLOCATION("S17-S24:!8")              PORT_CONDITION("DSW2", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "Credit Reset Mode")
	PORT_DIPSETTING(    0x80, "Credit Count Mode")
INPUT_PORTS_END


CUSTOM_INPUT_MEMBER( by35_state::spinner)
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

CUSTOM_INPUT_MEMBER( by35_state::outhole )
{
	uint8_t bit_mask = ((uintptr_t)param & 0xff);
	uint8_t port = (((uintptr_t)param >> 8) & 0x07);

	/* Here we simulate the ball sitting in the Outhole so the Outhole Solenoid can release it */

	if (machine().input().code_pressed_once(KEYCODE_BACKSPACE))  m_io_hold_x[port] |= bit_mask;

	return ((m_io_hold_x[port] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( by35_state::kicker )
{
	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((m_io_hold_x[5] & bit_mask) == 0x00) m_io_hold_x[0] &= ~(bit_mask);
	if ((bit_mask & 0x01) && machine().input().code_pressed(KEYCODE_BACKSLASH)) m_io_hold_x[0] |= bit_mask;

	return ((m_io_hold_x[0] & bit_mask) ? true : false);
}


CUSTOM_INPUT_MEMBER( by35_state::switch_hold_x0 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x01) && machine().input().code_pressed_once(KEYCODE_BACKSLASH))   m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x02) && machine().input().code_pressed_once(KEYCODE_CLOSEBRACE))  m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x04) && machine().input().code_pressed_once(KEYCODE_OPENBRACE))   m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_SLASH))       m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_STOP))        m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x40) && machine().input().code_pressed_once(KEYCODE_L))           m_io_hold_x[0] |= bit_mask;
	if ((bit_mask & 0x80) && machine().input().code_pressed_once(KEYCODE_STOP))        m_io_hold_x[0] |= bit_mask;

	return ((m_io_hold_x[0] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( by35_state::switch_hold_x1 )
{
	/* Here we simulate switches that are held closed like fallen Drop Targets, ball in Saucers, etc so that Solenoid activation can release the respective switches */

	uint8_t bit_mask = ((uintptr_t)param & 0xff);

	if ((bit_mask & 0x08) && machine().input().code_pressed_once(KEYCODE_ENTER))  m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x10) && machine().input().code_pressed_once(KEYCODE_QUOTE))  m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x20) && machine().input().code_pressed_once(KEYCODE_COLON))  m_io_hold_x[1] |= bit_mask;
	if ((bit_mask & 0x40) && machine().input().code_pressed_once(KEYCODE_L))      m_io_hold_x[1] |= bit_mask;

	return ((m_io_hold_x[1] & bit_mask) ? true : false);
}

CUSTOM_INPUT_MEMBER( by35_state::switch_hold_x2 )
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

CUSTOM_INPUT_MEMBER( by35_state::switch_hold_x3 )
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

CUSTOM_INPUT_MEMBER( by35_state::switch_hold_x4 )
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

CUSTOM_INPUT_MEMBER( by35_state::switch_hold_x5 )
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



READ8_MEMBER(by35_state::nibble_nvram_r)
{
	return (m_nvram[offset] | 0x0f);
}

WRITE8_MEMBER(by35_state::nibble_nvram_w)
{
	m_nvram[offset] = (data | 0x0f);
}

INPUT_CHANGED_MEMBER( by35_state::activity_button )
{
	if (newval != oldval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, (newval ? ASSERT_LINE : CLEAR_LINE));
}

INPUT_CHANGED_MEMBER( by35_state::self_test )
{
	m_pia_u10->ca1_w(newval);
}

READ_LINE_MEMBER( by35_state::u10_ca1_r )
{
	return (m_io_test->read() & 0x01);
}
READ_LINE_MEMBER( by35_state::u10_cb1_r )
{
	return m_u10_cb1;
}

WRITE_LINE_MEMBER( by35_state::u10_ca2_w )
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
			m_display[5][digit] = 0;
			m_display[6][digit] = 0;
		}
	}
#endif

	if ((m_u10_ca2 == false) && state)
	{
		static constexpr uint8_t patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 };  // MC14543 - BCD to 7 Segment Display Decoder

		m_display[0][0] = patterns[0];                   // Nuova-Bell Games first GI lit digit
		m_display[0][m_digit] = patterns[m_segment[0]];  // Credits/Match/BallInPlay
		m_display[1][m_digit] = patterns[m_segment[1]];  // Player 1
		m_display[2][m_digit] = patterns[m_segment[2]];  // Player 2
		m_display[3][m_digit] = patterns[m_segment[3]];  // Player 3
		m_display[4][m_digit] = patterns[m_segment[4]];  // Player 4
		m_display[5][m_digit] = patterns[m_segment[5]];  // Player 5
		m_display[6][m_digit] = patterns[m_segment[6]];  // Player 6

		LOG("Display Player 0 = %02x: %02x written to digit %01x of Player 0 display. Segment is %02x\n", m_display[0][m_digit], patterns[m_segment[0]], m_digit, m_segment[0]);
	}

	m_u10_ca2 = state;
}

WRITE_LINE_MEMBER( by35_state::u10_cb2_w )
{
	LOG("New U10 CB2 state %01x, was %01x.   PIA=%02x\n", state, m_u10_cb2, m_u10a);

	if ((state) || ((m_u10_cb2==true) && (state==false)))
	{
		if (m_lamp_decode != (m_u10a & 0x0f))
		{
			if ((m_u10a & 0x0f) < 0x0f)
			{
				update_lamps(0, 15, (m_u10a & 0x0f), m_u10a);
				m_lamp_prepare_relay = true;
				LOG("ON:  Board 0, Lamp_decode now=%02x, Lamp_decode was %02x,  U10-PortA=%02x,   m_sol_exp_lamp=%02x\n", (m_u10a & 0x0f), m_lamp_decode, m_u10a, m_sol_exp_lamp);
			}
		}
		m_lamp_decode = (m_u10a & 0x0f);
	}

	m_u10_cb2 = state;
}

WRITE_LINE_MEMBER( by35_state::u11_ca2_w )
{
	LOG("New U11 CA2 state %01x, was %01x.   U10-PIA-PortA=%02x\n", state, m_u11_ca2, m_u10a);

	if ((m_u11_ca2==false) && (state==true))
	{
		if (m_lamp_decode_aux != (m_u10a & 0x07))
		{
			if ((m_u10a & 0x07) < 0x07)
			{
				update_lamps(60, 7, (m_u10a & 0x07), m_u10a);
				m_lamp_prepare_relay = true;
				LOG("ON:  Board 1, Lamp_decode now=%02x, Lamp_decode was %02x,   U10-PortA=%02x,   m_sol_exp_lamp=%02x\n", (m_u10a & 0x07), m_lamp_decode_aux, m_u10a, m_sol_exp_lamp);
			}
		}
		m_lamp_decode_aux = (m_u10a & 0x07);
	}


	output().set_value("led0", state);

	if (m_6mdman)            // Player Display 5
		m_segment[5] = (m_u10a >> 4);

	m_u11_ca2 = state;
}

READ_LINE_MEMBER( by35_state::u11_ca1_r )
{
	return m_u11_ca1;
}

READ_LINE_MEMBER( by35_state::u11_cb1_r )
{
	/* Pin 32 on MPU J5 AID connector tied low */
	return false;
}

WRITE_LINE_MEMBER( by35_state::u11_cb2_w )
{
	m_u11_cb2 = state;
}

READ8_MEMBER( by35_state::u10_a_r )
{
	return m_u10a;
}

WRITE8_MEMBER( by35_state::u10_a_w )
{
	LOG("Writing %02x to U10 PIA, CB2 state is %01x,  CA2 state is %01x, Lamp_Dec is %02x\n",data, m_u10_cb2, m_u10_ca2, (m_lamp_decode & 0x0f));

	if (!m_u10_ca2)
	{
		if (BIT(m_u11a, 0) == 0)                // Credit/Ball in Play Display
			m_segment[0] = (data >> 4);
		if (BIT(data, 0) == 0)                  // Player Display 1
			m_segment[1] = (data >> 4);
		if (BIT(data, 1) == 0)                  // Player Display 2
			m_segment[2] = (data >> 4);
		if (BIT(data, 2) == 0)                  // Player Display 3
			m_segment[3] = (data >> 4);
		if (BIT(data, 3) == 0)                  // Player Display 4
			m_segment[4] = (data >> 4);
		if (m_6mdman) {
			if (m_u11_ca2)                      // Player Display 5
				m_segment[5] = (data >> 4);
			if (m_u11b & 0x80)                  // Player Display 6
				m_segment[6] = (data >> 4);
		}
	}


	if (m_u10_cb2)
		m_lamp_decode = (data & 0x0f);

	if (m_lamp_decode < 0x0f)
		update_lamps(0, 15, m_lamp_decode, data);

	if (m_lamp_decode_aux < 0x07)
		update_lamps(60, 7, m_lamp_decode_aux, data);

	m_lamp_prepare_relay = false;

	m_u10a = data;
}

READ8_MEMBER( by35_state::u10_b_r )
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

	if (m_u11b & m_io_x5_mask)
		data |= m_io_x5->read();

	return data;
}

WRITE8_MEMBER( by35_state::u10_b_w )
{
	m_u10b = data;
}

READ8_MEMBER( by35_state::u11_a_r )
{
	return m_u11a;
}

WRITE8_MEMBER( by35_state::u11_a_w )
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
	else if ((BIT(data, 1)) && m_7d)
		m_digit = 7;

	m_u11a = data;
}

WRITE8_MEMBER( by35_state::u11_b_w )
{
	if (!m_u11_cb2)
	{
		if ((data & 0x0f) < 0x0f)   // Momentary Solenoids
		{
			m_sol_exp_relay = ((m_relay[0]) ? 4 : 0);

			if (m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 0] != 0xff)     // Play solenoid audio sample
			{
				if (m_solenoids[(5 * m_sol_exp_relay) + (data & 0x0f)] == 0)
				{
					m_samples->start(m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 0], m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 1]);
				}
			}

			m_solenoids[(5 * m_sol_exp_relay) + (data & 0x0f)] = 1;

			if (m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 3])   // Release/Activate relevant switch after firing Solenoid
			{
				for (int sw_strobe_mask = 0; sw_strobe_mask <= 5; sw_strobe_mask++)
				{
					if (m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 2] & (1 << sw_strobe_mask) )
					{
						if (m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 2] & 0x80)   // Upper bit means we want to activate a switch (drop a target)
						{
							m_io_hold_x[sw_strobe_mask] |= (m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 3]);    // Activate a switch
							if (m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 2] & 0x40)  // Second upper bit means we want to activate switch(s) on another stobe (troughs)
							{
								if (m_outhole[1])
									m_io_hold_x[(m_outhole[1] >> 8) & 0x07] |= (m_outhole[1] & 0xff);
							}
						}
						else
						{
							m_io_hold_x[sw_strobe_mask] &= ~(m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 3]);   // Release a switch
							if (m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 2] & 0x40)  // Second upper bit means we want to release switch(s) on another stobe (troughs)
							{
								if (m_outhole[1])
									m_io_hold_x[(m_outhole[1] >> 8) & 0x07] &= ~(m_outhole[1] & 0xff);
							}
						}
					}
				}
			}
		}
		else                        // Rest output - all momentary solenoids are off
		{
			std::fill_n(std::begin(m_solenoids) + 00, 15, false);
			std::fill_n(std::begin(m_solenoids) + 20, 15, false);
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
						if (m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 2] & 0x40)  // Second upper bit means we want to activate switch(s) on another stobe (troughs)
						{
							if (m_outhole[1])
								m_io_hold_x[(m_outhole[1] >> 8) & 0x07] |= (m_outhole[1] & 0xff);
						}
					}
					else
					{
						m_io_hold_x[sw_strobe_mask] &= ~(m_solenoid_features[16][3]);  // Release a switch
						if (m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 2] & 0x40)  // Second upper bit means we want to release switch(s) on another stobe (troughs)
						{
							if (m_outhole[1])
								m_io_hold_x[(m_outhole[1] >> 8) & 0x07] &= ~(m_outhole[1] & 0xff);
						}
					}
				}
			}
		}
	}
	else if ((data & 0x10) && ((m_u11b & 0x10) == 0))
	{
		m_solenoids[16] = 0;
		if ((m_solenoid_features[16][0] != 0xff) && (m_solenoid_features[16][3] == 0))
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
		if (m_solenoid_features[19][3])  // Release/Activate relevant switch after firing Solenoid
		{
			for (int sw_strobe_mask = 0; sw_strobe_mask <= 5; sw_strobe_mask++)
			{
				if (m_solenoid_features[19][2] & (1 << sw_strobe_mask) )
				{
					if (m_solenoid_features[19][2] & 0x80)  // Upper bit means we want to activate a switch (drop a target)
					{
						m_io_hold_x[sw_strobe_mask] |= (m_solenoid_features[19][3]);   // Activate a switch
						if (m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 2] & 0x40)  // Second upper bit means we want to activate switch(s) on another stobe (troughs)
						{
							if (m_outhole[1])
								m_io_hold_x[(m_outhole[1] >> 8) & 0x07] |= (m_outhole[1] & 0xff);
						}
					}
					else
					{
						m_io_hold_x[sw_strobe_mask] &= ~(m_solenoid_features[19][3]);  // Release a switch
						if (m_solenoid_features[(data & 0x0f)][m_sol_exp_relay + 2] & 0x40)  // Second upper bit means we want to release switch(s) on another stobe (troughs)
						{
							if (m_outhole[1])
								m_io_hold_x[(m_outhole[1] >> 8) & 0x07] &= ~(m_outhole[1] & 0xff);
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
		if (m_6mdman)            // Player Display 6
			m_segment[6] = (m_u10a >> 4);
	}

	m_u11b = data;
}


void by35_state::update_lamps(u8 lamp_board_base, u8 lamp_decoder_base, u8 lamp_decoder_addr, u8 lamp_decoder_data)
{
	if (((lamp_board_base + lamp_decoder_addr) != 15) && ((lamp_board_base + lamp_decoder_addr) != 67)) 
	{
	// Update the Lamps on the Lamp Driver Boards only if their respective SCRs aren't already latched
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

	// Solenoid Expander Board (SEB)
	if (m_sol_exp_lamp != 255)
	{
		if ( (((m_sol_exp_lamp < 60) && (lamp_board_base==0)) && (lamp_decoder_addr == ((m_sol_exp_lamp - lamp_board_base) % lamp_decoder_base))) || (((m_sol_exp_lamp >= 60) && (m_sol_exp_lamp < 120) && (lamp_board_base==60)) && (lamp_decoder_addr == ((m_sol_exp_lamp - lamp_board_base) % lamp_decoder_base))) )
		{
			if (m_lamps[m_sol_exp_lamp])	// Check if the game specific Lamp driven control of the Solenoid Expander Board is active
			{
				LOG("PC:$%04x ON:  get_indexed_lamp%d=%d,   get_indexed_relay0=%d,   m_sol_exp_lamp=%d,   lamp_board_base=%d,   lamp_decoder_base=%d,   lamp_decoder_addr=%02x,   lamp_decoder_data=%02x,   m_lamp_prepare_relay=%01x\n", m_maincpu->pcbase(), m_sol_exp_lamp, m_lamps[m_sol_exp_lamp], m_relay[0], m_sol_exp_lamp, lamp_board_base, lamp_decoder_base, lamp_decoder_addr, lamp_decoder_data, m_lamp_prepare_relay);
				if ((m_relay[0] == 0) && (m_lamp_prepare_relay))
				{
				//	LOG("Playing the relay angage sound\n");
					m_samples->start(0, 0x0e);							// Play a clicking relay engaging sound effect
					m_lamp_prepare_relay = false;
					m_relay[0] = 1;										// Activate the Solenoid Expander relay
				}
			}
			else
			{	
				LOG("PC:$%04x OFF: get_indexed_lamp%d=%d,   get_indexed_relay0=%d,   m_sol_exp_lamp=%d,   lamp_board_base=%d,   lamp_decoder_base=%d,   lamp_decoder_addr=%02x,   lamp_decoder_data=%02x,   m_lamp_prepare_relay=%01x\n", m_maincpu->pcbase(), m_sol_exp_lamp, m_lamps[m_sol_exp_lamp], m_relay[0], m_sol_exp_lamp, lamp_board_base, lamp_decoder_base, lamp_decoder_addr, lamp_decoder_data, m_lamp_prepare_relay);
				if ((m_relay[0]) && (m_lamp_prepare_relay))
				{
				//	LOG("Playing the relay release sound\n");
					m_samples->start(0, 0x0f);							// Play a clicking relay releasing sound effect
					m_lamp_prepare_relay = false;
					m_relay[0] = 0;										// Release the Solenoid Expander relay
				}
	  		}
		}
	}

	// G.I. Flasher
	if (m_gi_flasher_lamp != 255)
	{
		if (m_lamps[m_gi_flasher_lamp])					// Check if the game specific Lamp driven control of the G.I. Flasher Board is active
		{
			output().set_value("Triac", 1);				// Activate the Triac on the G.I. power rail
			output().set_value("GI_Lamps", 1);			// Triac switches on the G.I. lamps
			//	LOG("m_gi_flasher_lamp=%d,  Lamp State=%02x,   Triac=%02x,  GI_Lamps=%02x\n", m_gi_flasher_lamp, m_lamps[m_gi_flasher_lamp], output().get_value("Triac"), output().get_value("GI_Lamps"));
		}
		else
		{
			output().set_value("Triac", 0);				// Release the Triac on the G.I. power rail
			output().set_value("GI_Lamps", 0);			// Triac switches off the G.I. lamps
			//	LOG("m_gi_flasher_lamp=%d,  Lamp State=%02x,   Triac=%02x,  GI_Lamps=%02x\n", m_gi_flasher_lamp, m_lamps[m_gi_flasher_lamp], output().get_value("Triac"), output().get_value("GI_Lamps"));
		}
	}
}


WRITE8_MEMBER( by35_state::u11_a_as2888_w )
{
	m_as2888->sound_select(space, offset, (m_u11b & 0x0f) | ((data & 0x02) << 3));
	u11_a_w( space, offset, data );
}
WRITE8_MEMBER( by35_state::u11_b_as2888_w )
{
	m_as2888->sound_select(space, offset, (data & 0x0f) | ((m_u11a & 0x02) << 3));
	u11_b_w( space, offset, data );
}
WRITE_LINE_MEMBER( by35_state::u11_cb2_as2888_w )
{
	m_as2888->sound_int(state);
	u11_cb2_w(state);
}
WRITE8_MEMBER( by35_state::u11_b_as3022_w )
{
	if (m_7d) m_as3022->sound_select(space, offset, (data & 0x0f));
	else      m_as3022->sound_select(space, offset, (data & 0x0f) | ((m_u11a & 0x02) << 3));
	u11_b_w( space, offset, data );
}
WRITE_LINE_MEMBER( by35_state::u11_cb2_as3022_w )
{
	m_as3022->sound_int(state);
	u11_cb2_w(state);
}
WRITE8_MEMBER( by35_state::u11_b_as3060_w )
{
	m_sounds_plus->sound_select(space, offset, (data & 0x0f));
	u11_b_w( space, offset, data );
}
WRITE_LINE_MEMBER( by35_state::u11_cb2_as3060_w )
{
	m_sounds_plus->sound_int(state);
	u11_cb2_w(state);
}
WRITE8_MEMBER( by35_state::u11_b_a91603_w )
{
	m_cheap_squeak->sound_select(space, offset, (data & 0x0f));
	u11_b_w( space, offset, data );
}
WRITE_LINE_MEMBER( by35_state::u11_cb2_a91603_w )
{
	m_cheap_squeak->sound_int(state);
	u11_cb2_w(state);
}
WRITE8_MEMBER( by35_state::u11_b_as3107_w )
{
	m_squawk_n_talk->sound_select(space, offset, (data & 0x0f));
	u11_b_w( space, offset, data );
}
WRITE_LINE_MEMBER( by35_state::u11_cb2_as3107_w )
{
	m_squawk_n_talk->sound_int(state);
	u11_cb2_w(state);
}
WRITE8_MEMBER( by35_state::u11_b_as3107a_w )
{
	m_squawk_n_talk_ay->sound_select(space, offset, (data & 0x0f));
	u11_b_w( space, offset, data );
}
WRITE_LINE_MEMBER( by35_state::u11_cb2_as3107a_w )
{
	m_squawk_n_talk_ay->sound_int(state);
	u11_cb2_w(state);
}


WRITE_LINE_MEMBER( by35_state::sound_ack_w )
{
	m_pia_u11->cb2_w(state);
}


TIMER_DEVICE_CALLBACK_MEMBER( by35_state::timer_z_freq )
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
TIMER_DEVICE_CALLBACK_MEMBER( by35_state::timer_z_pulse )
{
	/*** Line Power to DC Zero Crossing has ended ***/

	m_u10_cb1 = false;
	m_pia_u10->cb1_w(m_u10_cb1);
}

TIMER_DEVICE_CALLBACK_MEMBER( by35_state::u11_timer )
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

TIMER_DEVICE_CALLBACK_MEMBER( by35_state::timer_d_pulse )
{
	m_u11_ca1 = false;
	m_pia_u11->ca1_w(m_u11_ca1);
}


by35_state::solenoid_feature_data const by35_state::s_solenoid_features_default =
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



by35_state::solenoid_feature_data const lostwrlp_state::s_solenoid_features_lostwrlp =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x04, 0x40 },     // Saucer Left
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x02, 0x05,  0x04, 0x80 },     // Saucer Right
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const sst_state::s_solenoid_features_sst =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x04, 0x80 },     // Saucer
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*13*/  { 0x03, 0x0b,  0x01, 0x1f },     // Drop Target Reset
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Gate Right
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const playboy_state::s_solenoid_features_playboy =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x0a,  0x08, 0x80 },     // Kickback Grotto
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x03, 0x0b,  0x01, 0x1f },     // Drop Target Reset
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const smman_state::s_solenoid_features_smman =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x05, 0x10,  0x00, 0x00 },     // Post Down
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x08, 0x80 },     // Saucer Top
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x05, 0x11,  0x00, 0x00 },     // Post Up
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0x03, 0x0b,  0x01, 0x1f },     // Drop Target Reset
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Gate Right
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const startrep_state::s_solenoid_features_startrep =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x08, 0x80 },     // Saucer Top
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*13*/  { 0x03, 0x0b,  0x01, 0x0f },     // Drop Target Reset
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const kiss_state::s_solenoid_features_kiss =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Upper
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Lower
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*13*/  { 0x03, 0x0b,  0x01, 0x0f },     // Drop Target Reset
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Gate Right
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const slbmania_state::s_solenoid_features_slbmania =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Lower
	/*10*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*12*/  { 0x05, 0x11,  0xa0, 0x01 },     // Kicker Up (fake switch used here to flag Kicker is up)
	/*13*/  { 0x05, 0x10,  0x21, 0x01 },     // Kickback, then Kicker Down (Strobe 0x20 is fake flag for the state of the kicker)
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const hglbtrtr_state::s_solenoid_features_hglbtrtr =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Slingshot Left
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Rightt
	/*12*/  { 0x02, 0x05,  0x04, 0x80 },     // Saucer Top (G-L-O-B-E)
	/*13*/  { 0x02, 0x05,  0x08, 0x80 },     // Saucer Right
	/*14*/  { 0x03, 0x0b,  0x01, 0x0f },     // Drop Target Reset
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Gate Right
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const ngndshkr_state::s_solenoid_features_ngndshkr =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x08, 0x80 },     // Saucer Left
	/*08*/  { 0x02, 0x05,  0x04, 0x80 },     // Saucer Top
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*13*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Lower
	/*14*/  { 0x03, 0x0b,  0x02, 0x78 },     // Drop Target Reset
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const futurspa_state::s_solenoid_features_futurspa =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0x03, 0x0b,  0x01, 0x0f },     // Drop Target Reset
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x04, 0x80 },     // Saucer
	/*08*/  { 0x02, 0x0a,  0x08, 0x08 },     // Kickback Left Outlane
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top Left
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top Right
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top Middle
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom Left
	/*13*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom Right
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Gate Left
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const spaceinv_state::s_solenoid_features_spaceinv =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x03, 0x0b,  0x01, 0x07 },     // Drop Target Reset 3 Bank
	/*08*/  { 0x03, 0x0b,  0x10, 0x02 },     // Drop Target Reset Single
	/*09*/  { 0xff, 0x00,  0x00, 0x00 },
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Middle
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const viking_state::s_solenoid_features_viking =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x03, 0x0b,  0x01, 0x0f },     // Drop Target Reset 4 Bank Inline
	/*01*/  { 0x03, 0x0b,  0x04, 0x07 },     // Drop Target Reset 3 Bank
	/*02*/  { 0x05, 0x10,  0x84, 0x04 },     // Drop Target #1
	/*03*/  { 0x05, 0x10,  0x84, 0x04 },     // Drop Target #2
	/*04*/  { 0x05, 0x10,  0x84, 0x04 },     // Drop Target #3
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x08, 0x80 },     // Saucer Kick Up
	/*08*/  { 0x02, 0x05,  0x08, 0x80 },     // Saucer Kick Down
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top Left
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top Right
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x02, 0x05, 0x04,  0x80 },     // Saucer Top
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const embryon_state::s_solenoid_features_embryon =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x03, 0x0b,  0x04, 0x01 },     // Drop Target Reset Single
	/*01*/  { 0x03, 0x0b,  0x08, 0x07 },     // Drop Target Reset 3 Bank Left
	/*02*/  { 0x03, 0x0b,  0x04, 0x38 },     // Drop Target Reset 3 Bank Top
	/*03*/  { 0x02, 0x05,  0x01, 0x04 },     // Saucer Top
	/*04*/  { 0x01, 0x09,  0x01, 0x90 },     // Outhole
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0xff, 0x00,  0x00, 0x00 },
	/*07*/  { 0xff, 0x00,  0x00, 0x00 },
	/*08*/  { 0xff, 0x00,  0x00, 0x00 },
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left Top
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left Bottom
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right Top
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right Bottom
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const frontier_state::s_solenoid_features_frontier =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x03, 0x0b,  0x04, 0x70 },     // Drop Target Reset Inline
	/*08*/  { 0x03, 0x0b,  0x10, 0x07 },     // Drop Target Reset 3 Bank
	/*09*/  { 0x02, 0x05,  0x02, 0x08 },     // Saucer
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Gate Upper Right
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const skatebll_state::s_solenoid_features_skatebll =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0x02, 0x05,  0x01, 0x10 },     // Saucer
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x03, 0x0b,  0x04, 0xf8 },     // Drop Target Reset 5 Bank Left
	/*08*/  { 0x03, 0x0b,  0x08, 0x0e },     // Drop Target Reset 3 Bank Top
	/*09*/  { 0x03, 0x0b,  0x01, 0x0e },     // Drop Target Reset 3 Bank Middle
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const xenon_state::s_solenoid_features_xenon =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x03, 0x0b,  0x04, 0xf0 },     // Drop Target Reset 4 Bank
	/*01*/  { 0x05, 0x10,  0x84, 0x80 },     // Drop Target #1
	/*02*/  { 0x05, 0x10,  0x84, 0x40 },     // Drop Target #2
	/*03*/  { 0x05, 0x10,  0x84, 0x20 },     // Drop Target #3
	/*04*/  { 0x05, 0x10,  0x84, 0x10 },     // Drop Target #4
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x11,  0x41, 0x02 },     // Ball Release
	/*08*/  { 0x02, 0x05,  0x10, 0x01 },     // Saucer Top
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Middle
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Lower
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }
	/*16*/  { 0x02, 0x05,  0x10, 0x02 },     // Saucer Top
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const flashgdn_state::s_solenoid_features_flashgdn =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x03, 0x0b,  0x04, 0x0f },     // Drop Target Reset 4 Bank
	/*01*/  { 0x03, 0x0b,  0x04, 0x70 },     // Drop Target Reset 3 Bank
	/*02*/  { 0x03, 0x0b,  0x08, 0x07 },     // Drop Target Reset Inline
	/*03*/  { 0x02, 0x05,  0x08, 0x20 },     // Saucer Kick Down
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x08, 0x20 },     // Saucer Kick Up
	/*08*/  { 0x03, 0x0b,  0x01, 0x04 },     // Drop Target Reset Single
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*11*/  { 0x05, 0x10,  0x81, 0x04 },     // Drop Target Single Down
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const fball_ii_state::s_solenoid_features_fball_ii =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x03, 0x0b,  0x04, 0xf0 },     // Drop Target Reset 4 Bank Right
	/*01*/  { 0x03, 0x0b,  0x08, 0xf0 },     // Drop Target Reset 4 Bank Left
	/*02*/  { 0x03, 0x0b,  0x10, 0x07 },     // Drop Target Reset 3 Bank Middle
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x41, 0x07 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x01, 0x10 },     // Saucer Top Left Odin
	/*08*/  { 0x02, 0x05,  0x01, 0x08 },     // Saucer Top Right Wotan
	/*09*/  { 0x05, 0x0a,  0x00, 0x00 },     // Little Demon Post Kicker
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Fireball Relay (Doodle Bug)
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const eballdlx_state::s_solenoid_features_eballdlx =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*01*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*02*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top Left
	/*03*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*04*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x03, 0x0b,  0x10, 0x01 },     // Drop Target Reset Single
	/*07*/  { 0x03, 0x0b,  0x04, 0x7f, 0x02, 0x05,  0x10, 0x02 },     // Drop Target Reset 7 Bank  OR Saucer
	/*08*/  { 0x05, 0x10,  0x84, 0x01, 0x01, 0x09,  0x01, 0x80 },     // Drop Target #1/9          OR Outhole
	/*09*/  { 0x05, 0x10,  0x84, 0x02, 0x03, 0x0b,  0x01, 0x0f },     // Drop Target #2/10         OR Drop Target Reset 4 Bank
	/*10*/  { 0x05, 0x10,  0x84, 0x04 },     // Drop Target #3/11
	/*11*/  { 0x05, 0x10,  0x84, 0x08 },     // Drop Target #4/12
	/*12*/  { 0x05, 0x10,  0x84, 0x10 },     // Drop Target #5/13
	/*13*/  { 0x05, 0x10,  0x84, 0x20 },     // Drop Target #6/14
	/*14*/  { 0x05, 0x10,  0x84, 0x40 },     // Drop Target #7/15
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const fathom_state::s_solenoid_features_fathom =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x03, 0x0b,  0x20, 0x0e, 0x05, 0x10,  0xa0, 0x80 },     // Drop Target Reset Upper Inline  OR 1st Inline Green Drop
	/*01*/  { 0x03, 0x0b,  0x08, 0xfc, 0x05, 0x10,  0xa0, 0x40 },     // Drop Target Reset Left          OR 2nd Inline Green Drop
	/*02*/  { 0x03, 0x0b,  0x10, 0x07, 0x05, 0x10,  0xa0, 0x20 },     // Drop Target Reset Middle        OR 3rd Inline Green Drop
	/*03*/  { 0x03, 0x0b,  0x20, 0xe0 },     // Drop Target Reset Inline Right
	/*04*/  { 0x00, 0x00,  0x00, 0x00 },     // 
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x07 },     // Outhole
	/*07*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*10*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*12*/  { 0x02, 0x05,  0x01, 0x08, 0x05, 0x10,  0xa0, 0x08 },     // Saucer Top                      OR 1st Inline Blue Drop
	/*13*/  { 0x02, 0x05,  0x01, 0x10, 0x05, 0x10,  0xa0, 0x04 },     // Saucer Right                    OR 2nd Inline Blue Drop
	/*14*/  { 0xff, 0x00,  0x00, 0x00, 0x05, 0x10,  0xa0, 0x02 },     // None                            OR 3rd Inline Blue Drop
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }      // This bit is used for Strobe #5 in the Switch matrix
};

by35_state::solenoid_feature_data const centaur_state::s_solenoid_features_centaur =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x03, 0x0b,  0x08, 0xf0 },     // Drop Target Reset Middle (ORBS)
	/*01*/  { 0x05, 0x10,  0x88, 0x08 },     // Drop Target Right #1 Top
	/*02*/  { 0x05, 0x10,  0x88, 0x04 },     // Drop Target Right #2
	/*03*/  { 0x05, 0x10,  0x88, 0x02 },     // Drop Target Right #3
	/*04*/  { 0x05, 0x10,  0x88, 0x01 },     // Drop Target Right #4 Bottom
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x03, 0x0b,  0x20, 0x0f },     // Drop Target Reset Inline left
	/*08*/  { 0x03, 0x0b,  0x08, 0x0f },     // Drop Target Reset Right
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*13*/  { 0x03, 0x0a,  0x00, 0x00 },     // Ball Kickback to Playfield { ,,0x90, 0x01 }
	/*14*/  { 0x02, 0x11,  0x41, 0x03 },     // Ball Release
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },     // This bit is wired to Strobe #5 in the Switch matrix
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }      // Magnet
};

by35_state::solenoid_feature_data const medusa_state::s_solenoid_features_medusa =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x04, 0x06,  0x00, 0x00, 0x02, 0x11,  0xa0, 0x01 },     // Knocker                   OR Zipper Flippers Open
	/*01*/  { 0x03, 0x0b,  0x01, 0x0f, 0x02, 0x10,  0x20, 0x01 },     // Drop Target Reset Right   OR Zipper Flippers Close
	/*02*/  { 0x03, 0x0b,  0x20, 0xfe, 0x05, 0x10,  0xa0, 0x80 },     // Drop Target Reset Upper   OR Drop Target #1
	/*03*/  { 0x05, 0x10,  0xa0, 0x40, 0x05, 0x10,  0xa0, 0x20 },     // Drop Target #2            OR Drop Target #3
	/*04*/  { 0x05, 0x10,  0xa0, 0x10, 0x05, 0x10,  0xa0, 0x08 },     // Drop Target #4            OR Drop Target #5
	/*05*/  { 0x05, 0x10,  0xa0, 0x04, 0x05, 0x10,  0xa0, 0x02 },     // Drop Target #6            OR Drop Target #7
	/*06*/  { 0x05, 0x0a,  0x00, 0x00 },     // Shields of GODs Post Kicker
	/*07*/  { 0x01, 0x09,  0x01, 0x80, 0x02, 0x05,  0x04, 0x04 },     // Outhole                   OR Saucer
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom Left
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom Right
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top Left
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top Right
	/*12*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right Upper
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }      // This bit is used for Strobe #5 in the Switch matrix
};

by35_state::solenoid_feature_data const vector_state::s_solenoid_features_vector =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x03, 0x0b,  0x20, 0xe0 },     // Drop Target Reset X-Y-Z 
	/*01*/  { 0x03, 0x0b,  0x08, 0x0e },     // Drop Target Reset Lower
	/*02*/  { 0x02, 0x05,  0x01, 0x10 },     // Saucer Bottom Left
	/*03*/  { 0x03, 0x0b,  0x08, 0x70 },     // Drop Target Reset Upper
	/*04*/  { 0x02, 0x05,  0x01, 0x08 },     // Saucer Bottom Right
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x07, 0x05, 0x10,  0x88, 0x40 },     // Outhole                   OR Drop Target Upper Left
	/*07*/  { 0x05, 0x10,  0x88, 0x20 },     // Drop Target Upper Middle
	/*08*/  { 0x02, 0x05,  0x04, 0x04, 0x05, 0x10,  0x88, 0x10 },     // Captive Ball #1 Kickup    OR Drop Target Upper Right
	/*09*/  { 0x02, 0x05,  0x04, 0x04, 0x05, 0x10,  0x88, 0x08 },     // Captive Ball #1 Kickdown  OR Drop Target Lower Left
	/*10*/  { 0x02, 0x05,  0x04, 0x02, 0x05, 0x10,  0x88, 0x04 },     // Captive Ball #2           OR Drop Target Lower Middle
	/*11*/  { 0x02, 0x05,  0x04, 0x01, 0x05, 0x10,  0x88, 0x02 },     // Captive Ball #3           OR Drop Target Lower Right
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },     // This bit is used for Strobe #5 in the Switch matrix
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const m_mpac_state::s_solenoid_features_m_mpac =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x02, 0x05,  0x01, 0x40 },     // Saucer Top Left Kick Left
	/*01*/  { 0x02, 0x05,  0x01, 0x40 },     // Saucer Top Left Kick Right
	/*02*/  { 0x02, 0x05,  0x01, 0x80 },     // Saucer Right
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x10 },     // Outhole
	/*07*/  { 0x05, 0x10,  0x81, 0x01 },     // Drop Target 4 Left #1 (Bottom)
	/*08*/  { 0x02, 0x00,  0x00, 0x00, 0x05, 0x10,  0x81, 0x02 },     // Pop Bumper Left           OR Drop Target 4 Left #2
	/*09*/  { 0x02, 0x00,  0x00, 0x00, 0x05, 0x10,  0x81, 0x04 },     // Pop Bumper Bottom         OR Drop Target 4 Left #3
	/*10*/  { 0x02, 0x07,  0x00, 0x00, 0x05, 0x10,  0x81, 0x08 },     // Slingshot Left            OR Drop Target 4 Left #4 (Upper)
	/*11*/  { 0x02, 0x07,  0x00, 0x00, 0x05, 0x10,  0x88, 0x01 },     // Slingshot Right           OR Drop Target 4 Right #1 (Upper)
	/*12*/  { 0x03, 0x0b,  0x01, 0x0f, 0x05, 0x10,  0x88, 0x02 },     // Drop Target Reset 4 Left  OR Drop Target 4 Right #2
	/*13*/  { 0x03, 0x0b,  0x08, 0x0f, 0x05, 0x10,  0x88, 0x04 },     // Drop Target Reset 4 Right OR Drop Target 4 Right #3
	/*14*/  { 0x03, 0x0b,  0x04, 0xe0, 0x05, 0x10,  0x88, 0x08 },     // Drop Target Reset 3 Upper OR Drop Target 4 Right #4 (Bottom)
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const goldball_state::s_solenoid_features_goldball =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*01*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*02*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*03*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*04*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*05*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*06*/  { 0x02, 0x05,  0x03, 0x20 },     // Goldball Kicker
	/*07*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	/*08*/  { 0xff, 0x00,  0x00, 0x00 },
	/*09*/  { 0xff, 0x00,  0x00, 0x00 },
	/*10*/  { 0xff, 0x00,  0x00, 0x00 },
	/*11*/  { 0xff, 0x00,  0x00, 0x00 },
	/*12*/  { 0xff, 0x00,  0x00, 0x00 },
	/*13*/  { 0xff, 0x00,  0x00, 0x00 },
	/*14*/  { 0xff, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Gate Open
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0x00, 0x0e, 0x0f,  0x00 }      // Gate Close
};

by35_state::solenoid_feature_data const kosteel_state::s_solenoid_features_kosteel =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*01*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*02*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*07*/  { 0x05, 0x10,  0x81, 0x80 },     // Drop Target "Q" (top)
	/*08*/  { 0x05, 0x10,  0x81, 0x40 },     // Drop Target "J" (2nd from top)
	/*09*/  { 0x05, 0x10,  0x81, 0x20 },     // Drop Target "Q" (3rd from top)
	/*10*/  { 0x05, 0x10,  0x81, 0x10 },     // Drop Target "10" (3rd from bottom)
	/*11*/  { 0x05, 0x10,  0x81, 0x08 },     // Drop Target "J" (2nd from bottom)
	/*12*/  { 0x05, 0x10,  0x81, 0x04 },     // Drop Target "Q" (bottom)
	/*13*/  { 0x03, 0x0b,  0x10, 0xfc },     // Drop Target Reset Upper
	/*14*/  { 0x02, 0x05,  0x01, 0x01 },     // Saucer
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Gate
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const xsandos_state::s_solenoid_features_xsandos =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0xff, 0x00,  0x00, 0x00 },
	/*06*/  { 0xff, 0x00,  0x00, 0x00 },
	/*07*/  { 0xff, 0x00,  0x00, 0x00 },
	/*08*/  { 0x03, 0x0b,  0x01, 0x07 },     // Drop Target Reset Upper
	/*09*/  { 0x02, 0x05,  0x01, 0x08 },     // Saucer
	/*10*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top
	/*12*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*13*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*14*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const blakpyra_state::s_solenoid_features_blakpyra =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0xff, 0x00,  0x00, 0x00 },
	/*06*/  { 0xff, 0x00,  0x00, 0x00 },
	/*07*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*08*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*11*/  { 0x02, 0x05,  0x01, 0x40 },     // Saucer
	/*12*/  { 0x03, 0x0b,  0x08, 0xe0 },     // Drop Target Reset
	/*13*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*14*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Gate
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const spyhuntr_state::s_solenoid_features_spyhuntr =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0xff, 0x00,  0x00, 0x00 },
	/*06*/  { 0xff, 0x00,  0x00, 0x00 },
	/*07*/  { 0xff, 0x00,  0x00, 0x00 },
	/*08*/  { 0xff, 0x00,  0x00, 0x00 },
	/*09*/  { 0x02, 0x05,  0x01, 0x10 },     // Saucer
	/*10*/  { 0x03, 0x0b,  0x04, 0x0f },     // Drop Target Reset Upper
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper
	/*12*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*13*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*14*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Gate Left
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const fbclass_state::s_solenoid_features_fbclass =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x02, 0x05,  0x04, 0x80 },     // Saucer Left
	/*01*/  { 0x02, 0x05,  0x04, 0x40 },     // Saucer Right
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0xff, 0x00,  0x00, 0x00 },
	/*06*/  { 0x02, 0x0a,  0x04, 0x20 },     // Kickback Left Outlane
	/*07*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Top
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Middle
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Bottom
	/*10*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*12*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*13*/  { 0x05, 0x11,  0x08, 0x38 },     // Ball Release (Lets release all trough switches)
	/*14*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Gate Right
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};

by35_state::solenoid_feature_data const cybrnaut_state::s_solenoid_features_cybrnaut =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0xff, 0x00,  0x00, 0x00 },
	/*01*/  { 0xff, 0x00,  0x00, 0x00 },
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x05, 0x0e,  0x00, 0x00 },     // Price Meter (Space Hawks only)
	/*06*/  { 0xff, 0x00,  0x00, 0x00 },
	/*07*/  { 0xff, 0x00,  0x00, 0x00 },
	/*08*/  { 0x02, 0x05,  0x01, 0x10 },     // Saucer Right
	/*09*/  { 0x03, 0x0b,  0x04, 0xf0 },     // Drop Target Reset
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*12*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*13*/  { 0x01, 0x09,  0x01, 0x80 },     // Outhole
	/*14*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Gate Left
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0x00, 0x0e, 0x0f,  0x00 }      // Gate Right
};

by35_state::solenoid_feature_data const bullseye_state::s_solenoid_features_bullseye =
{
	//  { Sound Channel, Sound Sample, Switch Strobe Bits, Switch Return Bits }  See definition in first Driver Init above
	/*00*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Left
	/*01*/  { 0x02, 0x00,  0x00, 0x00 },     // Pop Bumper Right
	/*02*/  { 0xff, 0x00,  0x00, 0x00 },
	/*03*/  { 0xff, 0x00,  0x00, 0x00 },
	/*04*/  { 0xff, 0x00,  0x00, 0x00 },
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },     // Knocker
	/*06*/  { 0x01, 0x09,  0x10, 0x01 },     // Outhole
	/*07*/  { 0x02, 0x05,  0x01, 0x04 },     // Saucer Left
	/*08*/  { 0x02, 0x05,  0x01, 0x01 },     // Saucer Right
	/*09*/  { 0x02, 0x05,  0x01, 0x02 },     // Saucer Top
	/*10*/  { 0xff, 0x00,  0x00, 0x00 },
	/*11*/  { 0xff, 0x00,  0x00, 0x00 },
	/*12*/  { 0xff, 0x00,  0x00, 0x00 },
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Left
	/*14*/  { 0x02, 0x07,  0x00, 0x00 },     // Slingshot Right
	/*15*/  { 0xff, 0x00,  0x00, 0x00 },     // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0x00, 0x00,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },     // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },     // Flipper enable relay
	/*19*/  { 0xff, 0x00, 0x00,  0x00 }
};


void by35_state::machine_start()
{
	genpin_class::machine_start();

	m_lamps.resolve();
	m_display.resolve();
	m_solenoids.resolve();
	m_spinners.resolve();
	m_relay.resolve();
	m_scrn_switches.resolve();

	save_item(NAME(m_u10a));
	save_item(NAME(m_u10b));
	save_item(NAME(m_u11a));
	save_item(NAME(m_u11b));
	save_item(NAME(m_io_hold_x[0]));
	save_item(NAME(m_io_hold_x[1]));
	save_item(NAME(m_io_hold_x[2]));
	save_item(NAME(m_io_hold_x[3]));
	save_item(NAME(m_io_hold_x[4]));
	save_item(NAME(m_io_hold_x[5]));
	save_item(NAME(m_u10_ca2));
	save_item(NAME(m_u10_cb1));
	save_item(NAME(m_u10_cb2));
	save_item(NAME(m_u11_ca1));
	save_item(NAME(m_u11_ca2));
	save_item(NAME(m_u11_cb2));
}

void by35_state::machine_reset()
{
	genpin_class::machine_reset();

	render_target *target = machine().render().first_target();

	target->set_view(0);

	m_u10a = 0xff;
	m_u10b = 0;
	m_u11a = 0;
	m_u11b = 0;
	m_lamp_decode = 0x0f;
	m_lamp_decode_aux = 0x07;
	m_lamp_prepare_relay = false;
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

	output().set_value("digit00", 0x3f);				// Nuova-Bell Games first digit of each players score is a fake '0' decal insert lit by a GI lamp

///	output().set_value("switch24", 47);
///	m_scrn_switches[13] = 65;
}

MACHINE_CONFIG_START(by35_state::by35)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M6800, 530000) // No xtal, just 2 chips forming a multivibrator oscillator around 530kHz
	MCFG_DEVICE_PROGRAM_MAP(by35_map)

	MCFG_NVRAM_ADD_0FILL("nvram")   // 'F' filled causes Credit Display to be blank on first startup

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_by35)

	/* Sound */
	genpin_audio(config);

	/* Devices */
	MCFG_DEVICE_ADD("pia_u10", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, by35_state, u10_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, by35_state, u10_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(*this, by35_state, u10_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by35_state, u10_b_w))
	MCFG_PIA_READCA1_HANDLER(READLINE(*this, by35_state, u10_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(*this, by35_state, u10_cb1_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(*this, by35_state, u10_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by35_state, u10_cb2_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_TIMER_DRIVER_ADD("timer_z_freq", by35_state, timer_z_freq)						// Mains Line Frequency * 2 (100Hz or 120Hz depending on country)
	MCFG_TIMER_DRIVER_ADD(m_zero_crossing_active_timer, by35_state, timer_z_pulse)		// Active pulse length from Zero Crossing detector

	MCFG_DEVICE_ADD("pia_u11", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, by35_state, u11_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, by35_state, u11_a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by35_state, u11_b_w))
	MCFG_PIA_READCA1_HANDLER(READLINE(*this, by35_state, u11_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(*this, by35_state, u11_cb1_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(*this, by35_state, u11_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by35_state, u11_cb2_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_d_freq", by35_state, u11_timer, PERIOD_OF_555_ASTABLE(36000, 4700, 0.0000001))	// 555 Timer 317-320Hz
	MCFG_TIMER_DRIVER_ADD(m_display_refresh_timer, by35_state, timer_d_pulse)												// 555 Active pulse length
MACHINE_CONFIG_END


MACHINE_CONFIG_START(by35_state::by35_53)
	by35(config);

	MCFG_DEFAULT_LAYOUT(layout_by35_53)    // This hardware has an extra switch strobe line. Layout shows the extra switches
MACHINE_CONFIG_END


MACHINE_CONFIG_START(by35_state::by35_ext_rom)
	by35(config);

	MCFG_DEVICE_MODIFY("maincpu")
MACHINE_CONFIG_END


MACHINE_CONFIG_START(by35_state::nuova)
	by35(config);

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_REPLACE("maincpu", M6802, 2000000) // ? MHz ?  Large crystal next to CPU, schematics don't indicate speed.
	MCFG_DEVICE_PROGRAM_MAP(nuova_map)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(by35_state::grand)
	by35(config);

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(grand_map)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(by35_state::by35_barakandl)
	by35(config);

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(by35_barakandl_map)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(by35_state::as2888)
	by35(config);
	
	MCFG_DEVICE_MODIFY("pia_u11")
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, by35_state, u11_a_as2888_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by35_state, u11_b_as2888_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by35_state, u11_cb2_as2888_w))
	
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD(m_as2888, BALLY_AS2888)		///	BALLY_AS2888(config, m_as2888).add_route(ALL_OUTPUTS, "mono", 1.0);
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(by35_state::as2888_ext)
	by35(config);
	
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(by35_ext_map)

	MCFG_DEVICE_MODIFY("pia_u11")
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, by35_state, u11_a_as2888_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by35_state, u11_b_as2888_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by35_state, u11_cb2_as2888_w))
	
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD(m_as2888, BALLY_AS2888)		///	BALLY_AS2888(config, m_as2888).add_route(ALL_OUTPUTS, "mono", 1.0);
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(by35_state::as3022)
	by35(config);

	MCFG_DEVICE_MODIFY("pia_u11")
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by35_state, u11_b_as3022_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by35_state, u11_cb2_as3022_w))

	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD(m_as3022, BALLY_AS3022)		///	BALLY_AS3022(config, "soundbrd").add_route(ALL_OUTPUTS, "mono", 1.0);
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(by35_state::sounds_plus)
	by35(config);

	MCFG_DEVICE_MODIFY("pia_u11")
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by35_state, u11_b_as3060_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by35_state, u11_cb2_as3060_w))

	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD(m_sounds_plus, BALLY_SOUNDS_PLUS)		///	BALLY_SOUNDS_PLUS(config, "soundbrd").add_route(ALL_OUTPUTS, "mono", 1.0);
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(by35_state::cheap_squeak)
	by35(config);

	MCFG_DEVICE_MODIFY("pia_u11")
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by35_state, u11_b_a91603_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by35_state, u11_cb2_a91603_w))

	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD(m_cheap_squeak, BALLY_CHEAP_SQUEAK)		///	BALLY_CHEAP_SQUEAK(config, m_cheap_squeak).add_route(ALL_OUTPUTS, "mono", 1.0);
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_BALLY_CHEAP_SQUEAK_SOUND_ACK_W(WRITELINE(*this, by35_state, sound_ack_w))
MACHINE_CONFIG_END


MACHINE_CONFIG_START(by35_state::squawk_n_talk)
	by35(config);

	MCFG_DEVICE_MODIFY("pia_u11")
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by35_state, u11_b_as3107_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by35_state, u11_cb2_as3107_w))

	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD(m_squawk_n_talk, BALLY_SQUAWK_N_TALK)		///	BALLY_SQUAWK_N_TALK(config, m_squawk_n_talk).add_route(ALL_OUTPUTS, "mono", 1.0);
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(by35_state::squawk_n_talk_ay)
	by35(config);

	MCFG_DEVICE_MODIFY("pia_u11")
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by35_state, u11_b_as3107a_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by35_state, u11_cb2_as3107a_w))

	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD(m_squawk_n_talk_ay, BALLY_SQUAWK_N_TALK_AY)		///	BALLY_SQUAWK_N_TALK_AY(config, "soundbrd").add_route(ALL_OUTPUTS, "mono", 1.0);
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(by35_state::squawk_n_talk_ay_53)
	by35(config);

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(by35_ext_map)

	MCFG_DEVICE_MODIFY("pia_u11")
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by35_state, u11_b_as3107a_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by35_state, u11_cb2_as3107a_w))

	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD(m_squawk_n_talk_ay, BALLY_SQUAWK_N_TALK_AY)		///	BALLY_SQUAWK_N_TALK_AY(config, "soundbrd").add_route(ALL_OUTPUTS, "mono", 1.0);
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(by35_state::bell_cheap_squeak)
	by35(config);

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_REPLACE("maincpu", M6802, 2000000) // ? MHz ?  Large crystal next to CPU, schematics don't indicate speed.
	MCFG_DEVICE_PROGRAM_MAP(by35_map)
///	MCFG_DEVICE_PROGRAM_MAP(nuova_map)

	MCFG_DEVICE_MODIFY("pia_u11")
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, by35_state, u11_b_a91603_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, by35_state, u11_cb2_a91603_w))

	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD(m_cheap_squeak, BALLY_CHEAP_SQUEAK)		///	BALLY_CHEAP_SQUEAK(config, m_cheap_squeak).add_route(ALL_OUTPUTS, "mono", 1.0);
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_BALLY_CHEAP_SQUEAK_SOUND_ACK_W(WRITELINE(*this, by35_state, sound_ack_w))
MACHINE_CONFIG_END

/*--------------------------------
/ Supersonic #1106
/-------------------------------*/
ROM_START(sst)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "741-10_1.716", 0x1000, 0x0800, CRC(5e4cd81a) SHA1(d2a4a3599ad7271cd0ddc376c31c9b2e8defa379))   // PROM 9316A-2424
	ROM_LOAD( "741-08_2.716", 0x5000, 0x0800, CRC(2789cbe6) SHA1(8230657cb5ee793354a5d4a80a9348639ec9af8f))   // PROM 9316A-2497
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))   // PROM 9316A-2491
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

ROM_START(sstb)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "surp2732.u2",  0x1000, 0x0800, CRC(4987f46e) SHA1(a32984f29ba41c8c03883cb80797c55b20d1ce42))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "3032d7.bin", 0x1800, 0x0800, CRC(c0fc5342) SHA1(0511162ac54e1c630c7460cec7311bc928baf656))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END
ROM_START(sstc)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "surp2732.u2",  0x1000, 0x0800, CRC(4987f46e) SHA1(a32984f29ba41c8c03883cb80797c55b20d1ce42))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "strekc.u6", 0x1800, 0x0800, CRC(0ee0d6ac) SHA1(1c71d2ade69d433780a2becc5480b2b3e7ddb0e0))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END
ROM_START(sstd)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "surp2732.u2",  0x1000, 0x0800, CRC(4987f46e) SHA1(a32984f29ba41c8c03883cb80797c55b20d1ce42))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "strekd.u6", 0x1800, 0x0800, CRC(03344105) SHA1(78fd822854a0843ef0e73e261716b15da07ae3b8))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

ROM_START(sstt)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "SuperSonicT.u2",  0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "SuperSonicT.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

ROM_START(sstf)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "SuperSonicF_u2.732",  0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "SuperSonicF_u6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

/*--------------------------------
/ Playboy #1116
/-------------------------------*/
ROM_START(playboy)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "743-14_1.716", 0x1000, 0x0800, CRC(5c40984a) SHA1(dea104242fcb6d604faa0f01f087bc58bd43cd9d))   // PROM 9316A-2500
	ROM_LOAD( "743-12_2.716", 0x5000, 0x0800, CRC(6fa66664) SHA1(4943220942ce74d4620eb5fbbab8f8a763f65a2e))   // PROM 9316A-2496
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))   // PROM 9316A-2491
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

ROM_START(playboyt)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "playboyt.u2",    0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "playboyt.u6",    0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

ROM_START(playboy2)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "playboy2.u2",    0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "playboy2.u6",    0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

#if 1
ROM_START(playboyl)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "playboy128k.bin",    0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_CONTINUE( 0x1800, 0x0800)
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_CONTINUE( 0x3000, 0x0800)
	ROM_CONTINUE( 0x7000, 0x0800)
	ROM_CONTINUE( 0x3800, 0x0800)
	ROM_CONTINUE( 0x7800, 0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END
#else
ROM_START(playboyl)			// Peter Inkochnitos Bell Games EX3 J5 board
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "playboy128k.bin",    0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x1000, 0x0800)
	ROM_CONTINUE( 0x1000, 0x0800)
	ROM_CONTINUE( 0x1000, 0x0800)
	ROM_CONTINUE( 0x1000, 0x0800)
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_CONTINUE( 0x1800, 0x0800)
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_CONTINUE( 0x3000, 0x0800)
	ROM_CONTINUE( 0x3000, 0x0800)
	ROM_CONTINUE( 0x3000, 0x0800)
	ROM_CONTINUE( 0x3000, 0x0800)
	ROM_CONTINUE( 0x3000, 0x0800)
	ROM_CONTINUE( 0x7000, 0x0800)
	ROM_CONTINUE( 0x3800, 0x0800)
	ROM_CONTINUE( 0x7800, 0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END
#endif

ROM_START(playboys)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "playboy2716.u2",    0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "playboy2716.u6",    0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

/*--------------------------------
/ Lost World #1119
/-------------------------------*/
ROM_START(lostwrlp)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "729-33_1.716", 0x1000, 0x0800, CRC(4ca40b95) SHA1(4b4a3fbffb0aa99dab6330e24f93605eee35ac54))   // PROM 9316A-2485
	ROM_LOAD( "729-48_2.716", 0x5000, 0x0800, CRC(963bffd8) SHA1(5144092d019132946b396fd7134866a878b3ca62))   // PROM 9316A-2490
	ROM_LOAD( "720-28_6.716", 0x5800, 0x0800, CRC(f24cce3e) SHA1(0dfeaeb5b1cf4c950ff530ee56966ac0f2257111))   // PROM 9316A-2487
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

/*--------------------------------
/ Six Million Dollar Man #1138
/-------------------------------*/
ROM_START(smman)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "742-20_1.716", 0x1000, 0x0800, CRC(33e55a75) SHA1(98fbec07c9d03557654e5b67e29738c66156ec62))   // PROM 9316A-2499
	ROM_LOAD( "742-18_2.716", 0x5000, 0x0800, CRC(5365d36c) SHA1(1db651d31e28cf3fda00bef5289bb14d3b37b3c1))   // PROM 9316A-2498
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))   // PROM 9316A-2491
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

ROM_START(smmanb)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "6mil2732.u2",  0x1000, 0x0800, CRC(52a6bcd4) SHA1(d7be836776ff5be8d694726b785023ceba312f5b))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "3032d7.bin", 0x1800, 0x0800, CRC(c0fc5342) SHA1(0511162ac54e1c630c7460cec7311bc928baf656))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

ROM_START(smmanc)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "6mil2732.u2",  0x1000, 0x0800, CRC(52a6bcd4) SHA1(d7be836776ff5be8d694726b785023ceba312f5b))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "strekc.u6", 0x1800, 0x0800, CRC(0ee0d6ac) SHA1(1c71d2ade69d433780a2becc5480b2b3e7ddb0e0))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

ROM_START(smmand)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "6mil2732.u2",  0x1000, 0x0800, CRC(52a6bcd4) SHA1(d7be836776ff5be8d694726b785023ceba312f5b))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "strekd.u6", 0x1800, 0x0800, CRC(03344105) SHA1(78fd822854a0843ef0e73e261716b15da07ae3b8))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

/*----------------------------------
/ Voltan Escapes Cosmic Doom #1147
/-----------------------------------*/
ROM_START(voltan)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "744-03_1.716", 0x1000, 0x0800, CRC(ad2467ae) SHA1(58c4de1ea696372bce9146a4c48a296ebcb2c431))
	ROM_LOAD( "744-04_2.716", 0x5000, 0x0800, CRC(dbf58b83) SHA1(2d5e1c42fb8987eec81d89a4fe758ff0b88a1889))
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

/*--------------------------------
/ Star Trek #1148
/-------------------------------*/
ROM_START(startrep)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "745-11_1.716", 0x1000, 0x0800, CRC(a077efca) SHA1(6f78d9a43db0b99c3818a73a04d15aa300194a6d))   // PROM 9316A-2427
	ROM_LOAD( "745-12_2.716", 0x5000, 0x0800, CRC(f683210a) SHA1(6120909d97269d9abfcc34eef2c79b56a9cf53bc))   // PROM 9316A-2428
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))   // PROM 9316A-2491
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

ROM_START(startrekt)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "startrekt.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "startrekt.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

/*--------------------------------
/ Kiss #1152
/-------------------------------*/
ROM_START(kiss)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "746-11_1.716", 0x1000, 0x0800, CRC(78ec7fad) SHA1(b7e47ed14be08571b620de71cd5006faaddc88d5))   // PROM 9316A-2435
	ROM_LOAD( "746-14_2.716", 0x5000, 0x0800, CRC(0fc8922d) SHA1(dc6bd4d2d744df69b33ec69896cf71ac10c14a35))
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))   // PROM 9316A-2491
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

ROM_START(kissb)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "kiss2732.u2",  0x1000, 0x0800, CRC(716adcfd) SHA1(048e3142cfa307ea4552e6af3812b0d7301b62ad))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "3032d7.bin", 0x1800, 0x0800, CRC(c0fc5342) SHA1(0511162ac54e1c630c7460cec7311bc928baf656))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

ROM_START(kissc)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "kiss2732.u2",  0x1000, 0x0800, CRC(716adcfd) SHA1(048e3142cfa307ea4552e6af3812b0d7301b62ad))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "strekc.u6", 0x1800, 0x0800, CRC(0ee0d6ac) SHA1(1c71d2ade69d433780a2becc5480b2b3e7ddb0e0))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

ROM_START(kissd)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "kiss2732.u2",  0x1000, 0x0800, CRC(716adcfd) SHA1(048e3142cfa307ea4552e6af3812b0d7301b62ad))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "strekd.u6", 0x1800, 0x0800, CRC(03344105) SHA1(78fd822854a0843ef0e73e261716b15da07ae3b8))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END


#ifdef MISSING_GAME
ROM_START(kisso)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "746-11_1.716", 0x1000, 0x0800, CRC(78ec7fad) SHA1(b7e47ed14be08571b620de71cd5006faaddc88d5))   // PROM 9316A-2435
	ROM_LOAD( "746-12_2.716", 0x5000, 0x0800, NO_DUMP)                                                        // PROM 9316A-2436
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))   // PROM 9316A-2491
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END
#endif

/*--------------------------------
/ Nitro Ground Shaker #1154
/-------------------------------*/
ROM_START(ngndshkr)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "776-17_1.716", 0x1000, 0x0800, CRC(f2d44235) SHA1(282106767b5ec5180fa8e7eb2eb5b4766849c920))
	ROM_LOAD( "776-11_2.716", 0x5000, 0x0800, CRC(b0396b55) SHA1(2d10c4af7ecfa23b64ffb640111b582f44256fd5))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))   // PROM 9316A-2443
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("776-15_4.716", 0x1000, 0x0800, CRC(63c80c52) SHA1(3350919fce237b308b8f960948f70d01d312e9c0))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

ROM_START(ngndshko)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "776-10_1.716", 0x1000, 0x0800, CRC(4b700ca0) SHA1(1994a2024e64bb8517f678d230681f02c0db2246))
	ROM_LOAD( "776-11_2.716", 0x5000, 0x0800, CRC(b0396b55) SHA1(2d10c4af7ecfa23b64ffb640111b582f44256fd5))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))   // PROM 9316A-2443
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("776-15_4.716", 0x1000, 0x0800, CRC(63c80c52) SHA1(3350919fce237b308b8f960948f70d01d312e9c0))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

ROM_START(ngndshkb)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "nitr2732.u2", 0x1000, 0x0800, CRC(3b8d62ef) SHA1(da9652de8930bdd092928775a886252798ce6bf8))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-3532.u6b", 0x1800, 0x0800, CRC(b5e6a3d5) SHA1(fa1593eeed449dbac87965e613b501108a015eb2))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("776-15_4.716", 0x1000, 0x0800, CRC(63c80c52) SHA1(3350919fce237b308b8f960948f70d01d312e9c0))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Silverball Mania #1157
/-------------------------------*/
ROM_START(slbmania)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "786-16_1.716", 0x1000, 0x0800, CRC(c054733f) SHA1(2699cf940ce40012e2d7554b0b130adcb2bec6d1))
	ROM_LOAD( "786-17_2.716", 0x5000, 0x0800, CRC(94af0298) SHA1(579eb0290283194d92b172f787d8a9ff54f16a07))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))   // PROM 9316A-2443
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("786-11_4.716", 0x1000, 0x0800, CRC(2a3641e6) SHA1(64693d424277e2aaf5fd4af33b2d348a8a455448))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

/*-----------------------------------
/ Harlem Globetrotters On Tour #1161
/------------------------------------*/
ROM_START(hglbtrtr)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "750-07_1.716", 0x1000, 0x0800, CRC(da594719) SHA1(0aaa50e7d62da64f88d82b00cf0747945be88818))   // PROM 9316A-2441
	ROM_LOAD( "750-08_2.716", 0x5000, 0x0800, CRC(3c783931) SHA1(ee260511063aff1b72e18b3bc5a5be81aecf10c9))   // PROM 9316A-2442
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))   // PROM 9316A-2443
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END

ROM_START(hglbtrtb)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "harl2732.u2", 0x1000, 0x0800, CRC(f70a2981) SHA1(dd3e6448efa0dff49ed84c1f586d3b817598fa31))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-3532.u6b", 0x1800, 0x0800, CRC(b5e6a3d5) SHA1(fa1593eeed449dbac87965e613b501108a015eb2))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END

ROM_START(hglbtrtrt)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "hglbtrtrt.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "hglbtrtrt.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END

/*--------------------------------
/ Dolly Parton #1162
/-------------------------------*/
ROM_START(dollyptn)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "777-10_1.716", 0x1000, 0x0800, CRC(ca88cb9a) SHA1(0deac1c02b2121635af4bd76a6695d8abc09d694))   // PROM 9316A-2447
	ROM_LOAD( "777-13_2.716", 0x5000, 0x0800, CRC(7fc93ea3) SHA1(534ac5ed34397fe622dcf7cc90eaf38a311fa871))   // PROM 9316A-2453
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))   // PROM 9316A-2443
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END

ROM_START(dollyptb)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "doll2732.u2", 0x1000, 0x0800, CRC(cd649da3) SHA1(10fbffa0dc620d8bc35b8236b1d55fbf3338b6b7))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-3532.u6b", 0x1800, 0x0800, CRC(b5e6a3d5) SHA1(fa1593eeed449dbac87965e613b501108a015eb2))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END

ROM_START(dollyptnt)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "dollyptnt.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "dollyptnt.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END

#ifdef MISSING_GAME
ROM_START(dollyptno)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "777-10_1.716", 0x1000, 0x0800, CRC(ca88cb9a) SHA1(0deac1c02b2121635af4bd76a6695d8abc09d694))   // PROM 9316A-2447
	ROM_LOAD( "777-11_2.716", 0x5000, 0x0800, NO_DUMP)                                                        // PROM 9316A-2448
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))   // PROM 9316A-2443
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END
#endif

/*--------------------------------
/ Paragon #1167
/-------------------------------*/
ROM_START(paragon)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "748-17_1.716", 0x1000, 0x0800, CRC(08dbdf32) SHA1(43d1380d809683e74d67b6cf57c6eb0ad248a813))   // PROM 9316A-2437
	ROM_LOAD( "748-15_2.716", 0x5000, 0x0800, CRC(26cc05c1) SHA1(6e11a0f2327dbf15f6c149ddd873d9af96597d9d))   // PROM 9316A-2434
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))   // PROM 9316A-2491
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END

ROM_START(paragonb)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "para2732.u2",  0x1000, 0x0800, CRC(b3c990a1) SHA1(cb90c5fa52fefc29574a86d0f39fd29b2a70b8f2))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "3032d7.bin", 0x1800, 0x0800, CRC(c0fc5342) SHA1(0511162ac54e1c630c7460cec7311bc928baf656))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END

ROM_START(paragonc)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "para2732.u2",  0x1000, 0x0800, CRC(b3c990a1) SHA1(cb90c5fa52fefc29574a86d0f39fd29b2a70b8f2))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "strekc.u6", 0x1800, 0x0800, CRC(0ee0d6ac) SHA1(1c71d2ade69d433780a2becc5480b2b3e7ddb0e0))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END

ROM_START(paragond)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "para2732.u2",  0x1000, 0x0800, CRC(b3c990a1) SHA1(cb90c5fa52fefc29574a86d0f39fd29b2a70b8f2))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "strekd.u6", 0x1800, 0x0800, CRC(03344105) SHA1(78fd822854a0843ef0e73e261716b15da07ae3b8))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x0020, "as2888:sound", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END

/*--------------------------------
/ Future Spa #1173
/-------------------------------*/
ROM_START(futurspa)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "781-07_1.716", 0x1000, 0x0800, CRC(4c716a6a) SHA1(a19ff17079b7ef0b9e6933ffc718dee0236bae10))
	ROM_LOAD( "781-09_2.716", 0x5000, 0x0800, CRC(316617ed) SHA1(749d63cefe9541885b51db89302ad8a23e8f5b0a))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))   // PROM 9316A-2443
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("781-02_4.716", 0x1000, 0x0800, CRC(364f7c9a) SHA1(e6a3d425317eaeba4109712c6949f11c50b82892))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

ROM_START(futurspat)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "futurspat.u2",    0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "futurspat.u6",    0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("781-02_4.716", 0x1000, 0x0800, CRC(364f7c9a) SHA1(e6a3d425317eaeba4109712c6949f11c50b82892))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Space Invaders #1178
/-------------------------------*/
ROM_START(spaceinv)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "792-10_1.716", 0x1000, 0x0800, CRC(075eba5a) SHA1(7147c2dfb6af1c39bbfb9e98f409baae10d09628))   // PROM 9316A-2812
	ROM_LOAD( "792-13_2.716", 0x5000, 0x0800, CRC(b87b9e6b) SHA1(eab787ea81409ba88e30a342564944e1fade8124))   // PROM 9316A-2814
	ROM_LOAD( "720-37_6.716", 0x5800, 0x0800, CRC(ceff6993) SHA1(bc91e7afdfc441ff47a37031f2d6caeb9ab64143))   // PROM 9316A-2B15
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("792-07_4.716", 0x1000, 0x0800, CRC(787ffd5e) SHA1(4dadad7095de27622c2120311a84555dacdc3364))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

ROM_START(spaceinvt)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "spaceinvt.u2",    0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "spaceinvt.u6",    0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5800, 0x7800,0x0800)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("792-07_4.716", 0x1000, 0x0800, CRC(787ffd5e) SHA1(4dadad7095de27622c2120311a84555dacdc3364))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Rolling Stones #1187
/-------------------------------*/
ROM_START(rollston)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "796-17_1.716", 0x1000, 0x0800, CRC(51a826d7) SHA1(6811149c8948066b85b4018802afd409dbe8c2e1))   // PROM 9316A-2816
	ROM_LOAD( "796-18_2.716", 0x5000, 0x0800, CRC(08c75b1a) SHA1(792a535514fe4d9476914f7f61c696a7a1bdb549))   // PROM 9316A-2817
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))   // PROM 9316A-2443
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("796-19_4.716", 0x1000, 0x0800, CRC(b740d047) SHA1(710edb6bbba0a03e4f516b501f019493a3a4033e))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

ROM_START(rollston2)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "796-21_1.716", 0x1000, 0x0800, CRC(35141D40) SHA1(22283ff6f15e91838edbe6260faa0c1059499917))   // PROM 9316A-2819
	ROM_LOAD( "796-18_2.716", 0x5000, 0x0800, CRC(08c75b1a) SHA1(792a535514fe4d9476914f7f61c696a7a1bdb549))   // PROM 9316A-2817
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))   // PROM 9316A-2443
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("796-19_4.716", 0x1000, 0x0800, CRC(b740d047) SHA1(710edb6bbba0a03e4f516b501f019493a3a4033e))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Mystic #1192
/-------------------------------*/
ROM_START(mystic)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "798-03_1.716", 0x1000, 0x0800, CRC(f9c91e3b) SHA1(a3e6600b7b809cdd51a2d61b679f4f45ecf16e99))
	ROM_LOAD( "798-04_2.716", 0x5000, 0x0800, CRC(f54e5785) SHA1(425304512b70ef0f17ca9854af96cbb63c5ee33e))   // PROM 9316A-2B22
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("798-05_4.716", 0x1000, 0x0800, CRC(e759e093) SHA1(e635dac4aa925804ec658e856f7830290bfbc7b8))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Xenon #1196
/-------------------------------*/
ROM_START(xenon)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "811-40_1.716", 0x1000, 0x0800, CRC(0fba871b) SHA1(52bc0ef65507f0f7422c319d0dc2059e12deab6d))
	ROM_LOAD( "811-41_2.716", 0x5000, 0x0800, CRC(1ea0d891) SHA1(98cd8cfed5c0f437d2b9423b31205f1e8b7436f9))
	ROM_LOAD( "720-40_6.732", 0x1800, 0x0800, CRC(d7aaaa03) SHA1(4e0b901645e509bcb59bf81a6ffc1612b4fb16ee))   // PROM 9332A-2590
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_RELOAD( 0x7000, 0x1000 )
	ROM_REGION(0x10000, "sounds_plus:cpu", 0)
	ROM_LOAD("811-35_4.532", 0xf000, 0x1000, CRC(e9caccbb) SHA1(e2e09ac738c48342212bf38687299876b40cecbb))   // PROM  9332B-2315
	ROM_LOAD("811-22_1.532", 0x8000, 0x1000, CRC(c49a968e) SHA1(86680e8cbb82e69c232313e5fdd7a0058b7eef13))   // PROM  9332B-2317
	ROM_LOAD("811-23_2.532", 0x9000, 0x1000, CRC(41043996) SHA1(78fa3782ee9f32d14cf41a96a60f708087e97bb9))   // PROM  9332B-2318
	ROM_LOAD("811-24_3.532", 0xa000, 0x1000, CRC(53d65542) SHA1(edb63b6d36524ae17ec40cfc02d5cf9985f0477f))   // PROM  9332B-2319
	ROM_LOAD("811-25_4.532", 0xb000, 0x1000, CRC(2c678631) SHA1(a1f9a732fdb498a71caf61ec8cf3d105cf7e114e))   // PROM  9332B-2320
	ROM_LOAD("811-26_5.532", 0xc000, 0x1000, CRC(b8e7febc) SHA1(e557b1bbbc68a6884edebe779df4529116031e00))   // PROM  9332B-2321
	ROM_LOAD("811-27_6.532", 0xd000, 0x1000, CRC(1e2a2afa) SHA1(3f4d4a562e46c162b80660eec8d9af6efe165dd6))   // PROM  9332B-2322
	ROM_LOAD("811-28_7.532", 0xe000, 0x1000, CRC(cebb4cd8) SHA1(2678ffb5e8e2fcff07f029f14a9e0bf1fb95f7bc))   // PROM  9332B-2323
	ROM_REGION(0x0020, "prom", 0)    // PROM on Vocalizer board. Used as an address decoder for the Speech ROMs.
	ROM_LOAD("720-44_9.6331", 0x0000, 0x0020, CRC(7f18f0e4) SHA1(7ed341f41374340664143efc503592a3002feabd))  // PROM  MMI 6331-1J
ROM_END

ROM_START(xenonf)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "811-40_1.716", 0x1000, 0x0800, CRC(0fba871b) SHA1(52bc0ef65507f0f7422c319d0dc2059e12deab6d))
	ROM_LOAD( "811-41_2.716", 0x5000, 0x0800, CRC(1ea0d891) SHA1(98cd8cfed5c0f437d2b9423b31205f1e8b7436f9))
	ROM_LOAD( "720-40_6.732", 0x1800, 0x0800, CRC(d7aaaa03) SHA1(4e0b901645e509bcb59bf81a6ffc1612b4fb16ee))
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_RELOAD( 0x7000, 0x1000 )
	ROM_REGION(0x10000, "sounds_plus:cpu", 0)
	ROM_LOAD("811-36_4.532", 0xf000, 0x1000, CRC(73156c6e) SHA1(b0b3ecb44428c01849189adf6c86be3e95a99012))   // PROM  9332B-????
	ROM_LOAD("811-22_1.532", 0x8000, 0x1000, CRC(c49a968e) SHA1(86680e8cbb82e69c232313e5fdd7a0058b7eef13))   // PROM  9332B-2317
	ROM_LOAD("811-23_2.532", 0x9000, 0x1000, CRC(41043996) SHA1(78fa3782ee9f32d14cf41a96a60f708087e97bb9))   // PROM  9332B-2318
	ROM_LOAD("811-24_3.532", 0xa000, 0x1000, CRC(53d65542) SHA1(edb63b6d36524ae17ec40cfc02d5cf9985f0477f))   // PROM  9332B-2319
	ROM_LOAD("811-29_4.532", 0xb000, 0x1000, CRC(e586ec31) SHA1(080d43e9a5895e95533ae73cffa4948f747ce510))   // PROM  9332B-????
	ROM_LOAD("811-30_5.532", 0xc000, 0x1000, CRC(e48d98e3) SHA1(bb32ab96501dcd21525540a61bd5e478a35b1fef))   // PROM  9332B-????
	ROM_LOAD("811-31_6.532", 0xd000, 0x1000, CRC(0a2336e5) SHA1(28eeb00b03b8d9eb0e6966be00dfbf3a1e13e04c))   // PROM  9332B-????
	ROM_LOAD("811-32_7.532", 0xe000, 0x1000, CRC(987e6118) SHA1(4cded4ff715494f762d043dbcb0298111f327311))   // PROM  9332B-????
	ROM_REGION(0x0020, "prom", 0)    // PROM on Vocalizer board. Used as an address decoder for the Speech ROMs.
	ROM_LOAD("720-44_9.6331", 0x0000, 0x0020, CRC(7f18f0e4) SHA1(7ed341f41374340664143efc503592a3002feabd))  // PROM  MMI 6331-1J
ROM_END

ROM_START(xenont)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "xenont.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "xenont.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_RELOAD( 0x7000, 0x1000 )
	ROM_REGION(0x10000, "sounds_plus:cpu", 0)
	ROM_LOAD("811-35_4.532", 0xf000, 0x1000, CRC(e9caccbb) SHA1(e2e09ac738c48342212bf38687299876b40cecbb))   // PROM  9332B-2315
	ROM_LOAD("811-22_1.532", 0x8000, 0x1000, CRC(c49a968e) SHA1(86680e8cbb82e69c232313e5fdd7a0058b7eef13))   // PROM  9332B-2317
	ROM_LOAD("811-23_2.532", 0x9000, 0x1000, CRC(41043996) SHA1(78fa3782ee9f32d14cf41a96a60f708087e97bb9))   // PROM  9332B-2318
	ROM_LOAD("811-24_3.532", 0xa000, 0x1000, CRC(53d65542) SHA1(edb63b6d36524ae17ec40cfc02d5cf9985f0477f))   // PROM  9332B-2319
	ROM_LOAD("811-25_4.532", 0xb000, 0x1000, CRC(2c678631) SHA1(a1f9a732fdb498a71caf61ec8cf3d105cf7e114e))   // PROM  9332B-2320
	ROM_LOAD("811-26_5.532", 0xc000, 0x1000, CRC(b8e7febc) SHA1(e557b1bbbc68a6884edebe779df4529116031e00))   // PROM  9332B-2321
	ROM_LOAD("811-27_6.532", 0xd000, 0x1000, CRC(1e2a2afa) SHA1(3f4d4a562e46c162b80660eec8d9af6efe165dd6))   // PROM  9332B-2322
	ROM_LOAD("811-28_7.532", 0xe000, 0x1000, CRC(cebb4cd8) SHA1(2678ffb5e8e2fcff07f029f14a9e0bf1fb95f7bc))   // PROM  9332B-2323
	ROM_REGION(0x0020, "prom", 0)    // PROM on Vocalizer board. Used as an address decoder for the Speech ROMs.
	ROM_LOAD("720-44_9.6331", 0x0000, 0x0020, CRC(7f18f0e4) SHA1(7ed341f41374340664143efc503592a3002feabd))  // PROM  MMI 6331-1J
ROM_END

ROM_START(xenonl)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "811-43_2.732_mod.bin", 0x1000, 0x0800, CRC(3785b929))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-40_6.732_mod.bin", 0x1800, 0x0800, CRC(65cb1ffd))
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_RELOAD( 0x7000, 0x1000 )
	ROM_REGION(0x10000, "sounds_plus:cpu", 0)
	ROM_LOAD("811-35_4.532", 0xf000, 0x1000, CRC(e9caccbb) SHA1(e2e09ac738c48342212bf38687299876b40cecbb))   // PROM  9332B-2315
	ROM_LOAD("811-22_1.532", 0x8000, 0x1000, CRC(c49a968e) SHA1(86680e8cbb82e69c232313e5fdd7a0058b7eef13))   // PROM  9332B-2317
	ROM_LOAD("811-23_2.532", 0x9000, 0x1000, CRC(41043996) SHA1(78fa3782ee9f32d14cf41a96a60f708087e97bb9))   // PROM  9332B-2318
	ROM_LOAD("811-24_3.532", 0xa000, 0x1000, CRC(53d65542) SHA1(edb63b6d36524ae17ec40cfc02d5cf9985f0477f))   // PROM  9332B-2319
	ROM_LOAD("811-25_4.532", 0xb000, 0x1000, CRC(2c678631) SHA1(a1f9a732fdb498a71caf61ec8cf3d105cf7e114e))   // PROM  9332B-2320
	ROM_LOAD("811-26_5.532", 0xc000, 0x1000, CRC(b8e7febc) SHA1(e557b1bbbc68a6884edebe779df4529116031e00))   // PROM  9332B-2321
	ROM_LOAD("811-27_6.532", 0xd000, 0x1000, CRC(1e2a2afa) SHA1(3f4d4a562e46c162b80660eec8d9af6efe165dd6))   // PROM  9332B-2322
	ROM_LOAD("811-28_7.532", 0xe000, 0x1000, CRC(cebb4cd8) SHA1(2678ffb5e8e2fcff07f029f14a9e0bf1fb95f7bc))   // PROM  9332B-2323
	ROM_REGION(0x0020, "prom", 0)    // PROM on Vocalizer board. Used as an address decoder for the Speech ROMs.
	ROM_LOAD("720-44_9.6331", 0x0000, 0x0020, CRC(7f18f0e4) SHA1(7ed341f41374340664143efc503592a3002feabd))  // PROM  MMI 6331-1J
ROM_END


/*--------------------------------
/ Viking #1198
/-------------------------------*/
ROM_START(viking)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "802-05_1.716", 0x1000, 0x0800, CRC(a5db0574) SHA1(d9836679ed797b649f2c1e22bc24e8a9fe1c3000))
	ROM_LOAD( "802-06_2.716", 0x5000, 0x0800, CRC(40410760) SHA1(b0b87d8600a03de7090e42f6ebdeeb5feccf87f6))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("802-07-4.716", 0x1000, 0x0800, CRC(62bc5030) SHA1(5a696f784a415d5b16ee23cd72a905264a2bbeac))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

ROM_START(vikingt)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "vikingt.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "vikingt.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_COPY("maincpu", 0x5FF8, 0x7FF8,0x08)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("802-07-4.716", 0x1000, 0x0800, CRC(62bc5030) SHA1(5a696f784a415d5b16ee23cd72a905264a2bbeac))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Hot Doggin' #1199
/-------------------------------*/
ROM_START(hotdoggn)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "809-05_1.716", 0x1000, 0x0800, CRC(2744abcb) SHA1(b45bd58c365785d12f9bec381574058e29f33fd2))
	ROM_LOAD( "809-06_2.716", 0x5000, 0x0800, CRC(03db3d4d) SHA1(b8eed2d22474d2b0a1667eef2fdd4ecfa5fd35f3))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))
	ROM_RELOAD( 0x7800, 0x0800)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("809-07_4.716", 0x1000, 0x0800, CRC(43f28d7f) SHA1(01fca0ee0137a0715421eaa3582ff8d324340ecf))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

#ifdef MISSING_GAME
ROM_START(hotdoggb) // check to see if this is the same as above but with a different split
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "hotd2732.u2", 0x1000, 0x0800, CRC(709305ee) SHA1(37d5e681a1a2b8b2782dae3007db3e5036003e00))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-3532.u6b", 0x1800, 0x0800, CRC(b5e6a3d5) SHA1(fa1593eeed449dbac87965e613b501108a015eb2))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("809-07_4.716", 0x1000, 0x0800, CRC(43f28d7f) SHA1(01fca0ee0137a0715421eaa3582ff8d324340ecf))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END
#endif

/*--------------------------------
/ Skateball #1210
/-------------------------------*/
ROM_START(skatebll)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "823-24_1.716", 0x1000, 0x0800, CRC(46e797d1) SHA1(7ddbf6047b8d95af8727c32b056bee1c4aa228e4))
	ROM_LOAD( "823-25_2.716", 0x5000, 0x0800, CRC(960cb8c3) SHA1(3a4499cab85d3563961b0a01c78fa1f3ba2188fe))
	ROM_LOAD( "720-40_6.732", 0x1800, 0x0800, CRC(d7aaaa03) SHA1(4e0b901645e509bcb59bf81a6ffc1612b4fb16ee))
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_RELOAD( 0x7000, 0x1000 )
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("823-02_4.716", 0x1000, 0x0800, CRC(d1037b20) SHA1(8784728540573be5e8ebb940ec0046b778f9413b))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

ROM_START(skateballt)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "skateballt.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "skateballt.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_COPY("maincpu", 0x5FF8, 0x7FF8,0x08)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("823-02_4.716", 0x1000, 0x0800, CRC(d1037b20) SHA1(8784728540573be5e8ebb940ec0046b778f9413b))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Flash Gordon #1215
/-------------------------------*/
ROM_START(flashgdn)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "834-23_2.732", 0x1000, 0x0800, CRC(0c7a0d91) SHA1(1f79be15817975acbc35cb08591e2289e2eca938))   // PROM 9332B-2333
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7))   // PROM 9332B-2696
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("834-20_2.532", 0x8000, 0x1000, CRC(2f8ced3e) SHA1(ecdeb07c31c22ec313b55774f4358a9923c5e9e7))
	ROM_LOAD("834-18_5.532", 0xb000, 0x1000, CRC(8799e80e) SHA1(f255b4e7964967c82cfc2de20ebe4b8d501e3cb0))
ROM_END

ROM_START(flashgdnt)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "834-23t_2.732", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52t_6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("834-20_2.532", 0x8000, 0x1000, CRC(2f8ced3e) SHA1(ecdeb07c31c22ec313b55774f4358a9923c5e9e7))
	ROM_LOAD("834-18_5.532", 0xb000, 0x1000, CRC(8799e80e) SHA1(f255b4e7964967c82cfc2de20ebe4b8d501e3cb0))
ROM_END

ROM_START(flashgdnf)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "834-23_2.732", 0x1000, 0x0800, CRC(0c7a0d91) SHA1(1f79be15817975acbc35cb08591e2289e2eca938))   // PROM 9332B-2333
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7))   // PROM 9332B-2696
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("834-35_2.532", 0x8000, 0x1000, CRC(dff3f711) SHA1(254a5670775ecb6c347f33af8ba7c350e4cfa550))
	ROM_LOAD("834-36_5.532", 0xb000, 0x1000, CRC(18691897) SHA1(3b445e0756c07d80f14c01af5a7f87744474ae15))
ROM_END

ROM_START(flashgdnv)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "834-23_2.732", 0x1000, 0x0800, CRC(0c7a0d91) SHA1(1f79be15817975acbc35cb08591e2289e2eca938))   // PROM 9332B-2333
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7))   // PROM 9332B-2696
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "sounds_plus:cpu", 0)
	ROM_LOAD("834-02_4.532", 0xf000, 0x1000, CRC(f1eb0a12) SHA1(a58567665547aacf9a1b2c39295d963527ef8696))
	ROM_LOAD("834-03_1.532", 0x8000, 0x1000, CRC(88bef6f4) SHA1(561e0bde04661b700552e4fbb6141c39f2789c99))
	ROM_LOAD("834-04_2.532", 0x9000, 0x1000, CRC(bce91475) SHA1(482b424977d73b36e2014617e3bd3deb51091c28))
	ROM_LOAD("834-05_3.532", 0xa000, 0x1000, CRC(1a4dbd99) SHA1(fa9ae0bde118a40ba9a0e9a085b30298cac0ea93))
	ROM_LOAD("834-06_4.532", 0xb000, 0x1000, CRC(983c9e9d) SHA1(aae323a39b0ec987e6b9b98e5d9b2c58b1eea1a4))
	ROM_LOAD("834-07_5.532", 0xc000, 0x1000, CRC(697f5333) SHA1(39bbff8790e394a20ef5ba3239fb1d9359be0fe5))
	ROM_LOAD("834-08_6.532", 0xd000, 0x1000, CRC(75dd195f) SHA1(fdb6f7a15cd42e1326bf6baf8fa69f6266653cef))
	ROM_LOAD("834-09_7.532", 0xe000, 0x1000, CRC(19ceabd1) SHA1(37e7780f2ba3e06462e775547278dcba1b6d2ac8))
	ROM_REGION(0x0020, "prom", 0)    // PROM on Vocalizer board. Used as an address decoder for the Speech ROMs.
	ROM_LOAD("720-44_9.6331", 0x0000, 0x0020, CRC(7f18f0e4) SHA1(7ed341f41374340664143efc503592a3002feabd))  // PROM  MMI 6331-1J
ROM_END

/*--------------------------------
/ Frontier #1217
/-------------------------------*/
ROM_START(frontier)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "819-08_1.716", 0x1000, 0x0800, CRC(e2f8ce9d) SHA1(03b38486e12f1677dcabcd0f14d194c59b3bd214))
	ROM_LOAD( "819-07_2.716", 0x5000, 0x0800, CRC(af023a85) SHA1(95df232ba654293066beccbad158146259a764b7))
	ROM_LOAD( "720-40_6.732", 0x1800, 0x0800, CRC(d7aaaa03) SHA1(4e0b901645e509bcb59bf81a6ffc1612b4fb16ee))
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_RELOAD( 0x7000, 0x1000 )
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("819-09_4.716", 0x1000, 0x0800, CRC(a62059ca) SHA1(75e139ea2573a8c3b666c9a1024d9308da9875c7))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

ROM_START(frontiea)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "frnt2732.u2", 0x1000, 0x0800, CRC(d866b2da) SHA1(d16cd346886149d7a19d3be8f0a626c34827242a))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "7406fn.u6", 0x1800, 0x0800, CRC(43a5f459) SHA1(3c8326df8a363d9bd64863a467b6988c3608e9f8))
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_RELOAD( 0x7000, 0x1000 )
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("819-09_4.716", 0x1000, 0x0800, CRC(a62059ca) SHA1(75e139ea2573a8c3b666c9a1024d9308da9875c7))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

ROM_START(frontiert)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "frontiert_u2.732", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "frontiert_u6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_RELOAD( 0x7000, 0x1000 )
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("819-09_4.716", 0x1000, 0x0800, CRC(a62059ca) SHA1(75e139ea2573a8c3b666c9a1024d9308da9875c7))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Fireball II #1219
/-------------------------------*/
ROM_START(fball_ii)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "839-12_2.732", 0x1000, 0x0800, CRC(45e768ad) SHA1(b706cb5f3dcfa2db54d8d15de180fcbf36b3768f))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("839-01_2.532", 0x8000, 0x1000, CRC(4aa473bd) SHA1(eaa12ded76f9999d33ce0fe6198df1708e007e12))
	ROM_LOAD("839-02_5.532", 0xb000, 0x1000, CRC(8bf904ff) SHA1(de78d08bddd546abac65c2f95f1d52797e716362))
ROM_END

/*--------------------------------
/ Eight Ball Deluxe #1220
/-------------------------------*/
ROM_START(eballdlx)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "838-15_2.732", 0x1000, 0x0800, CRC(68d92acc) SHA1(f37b16d2953677cd779073bc3eac4b586d62fad8))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7))    // PROM 9332B-2696
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("838-08_3.532", 0x9000, 0x1000, CRC(c39478d7) SHA1(8148aca7c4113921ab882da32d6d88e66abb22cc))     // PROM 9332B-2334
	ROM_LOAD("838-09_4.716", 0xa000, 0x0800, CRC(518ea89e) SHA1(a387274ef530bb57f31819733b35615a39260126))
	ROM_RELOAD(0xa800, 0x0800)
	ROM_LOAD("838-10_5.532", 0xb000, 0x1000, CRC(9c63925d) SHA1(abd1fa6308d3569e16ee10bfabce269a124d8f26))
ROM_END

ROM_START(eballd14)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "838-14_2.732", 0x1000, 0x0800, CRC(27eeabde) SHA1(a8f81dbb70202bdad1c9734d629e8a5c27f2a835))    // PROM 9332B-2337
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7))    // PROM 9332B-2696
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)

	ROM_LOAD("838-08_3.532", 0x9000, 0x1000, CRC(c39478d7) SHA1(8148aca7c4113921ab882da32d6d88e66abb22cc))     // PROM 9332B-2334
	ROM_LOAD("838-09_4.716", 0xa000, 0x0800, CRC(518ea89e) SHA1(a387274ef530bb57f31819733b35615a39260126))     // PROM 9316B-4B39
	ROM_RELOAD(0xa800, 0x0800)
	ROM_LOAD("838-10_5.532", 0xb000, 0x1000, CRC(9c63925d) SHA1(abd1fa6308d3569e16ee10bfabce269a124d8f26))

//	These are from Eight Ball Champ, temporarily tested to extract the speech commands
//	ROM_LOAD("u3_snd.532", 0x9000, 0x1000, CRC(4836d70d) SHA1(a4acc64609d91a84ba4c8101186d07397b496600))
//	ROM_LOAD("u4_snd.532", 0xa000, 0x1000, CRC(4b49d94d) SHA1(52d5f4b7604601cd86f0e80ed7c4fe09d14f5976))
//	ROM_LOAD("u5_snd.532", 0xb000, 0x1000, CRC(655441df) SHA1(9da5578856ded3dcdafed67679eb4c4134dc9f81))
ROM_END

ROM_START(eballdlxt)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "838-fix2.732", 0x1000, 0x0800, CRC(1d9350f4) SHA1(c4ff2e896846420798e89024aa1d5ed4cc28a228))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52t_6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("838-08_3.532", 0x9000, 0x1000, CRC(c39478d7) SHA1(8148aca7c4113921ab882da32d6d88e66abb22cc))
	ROM_LOAD("838-09_4.716", 0xa000, 0x0800, CRC(518ea89e) SHA1(a387274ef530bb57f31819733b35615a39260126))
	ROM_RELOAD(0xa800, 0x0800)
	ROM_LOAD("838-10_5.532", 0xb000, 0x1000, CRC(9c63925d) SHA1(abd1fa6308d3569e16ee10bfabce269a124d8f26))
ROM_END

/*--------------------------------
/ Embryon #1222
/-------------------------------*/
ROM_START(embryon)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "841-06_2.732", 0x1000, 0x0800, CRC(80ab18e7) SHA1(52e5b1709e6f21919fc9efed67f51934d883dbb7))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk:cpu", 0)
	ROM_LOAD("841-01_4.716", 0xa000, 0x0800, CRC(e8b234e3) SHA1(584e553748b1c6571491150e346d815005948b68))
	ROM_RELOAD(0xa800, 0x0800)
	ROM_LOAD("841-02_5.532", 0xb000, 0x1000, CRC(9cd8c04e) SHA1(7d74d8f33a98c9832fda1054187eb7300dbf5f5e))
ROM_END

ROM_START(embryona)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "841-06_2.732", 0x1000, 0x0800, CRC(80ab18e7) SHA1(52e5b1709e6f21919fc9efed67f51934d883dbb7))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "7526fn.u6", 0x1800, 0x0800, CRC(00258a56) SHA1(40143d65ee92d20a565d21b6c1c3507d659a38e0))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk:cpu", 0)
	ROM_LOAD("841-01_4.716", 0xa000, 0x0800, CRC(e8b234e3) SHA1(584e553748b1c6571491150e346d815005948b68))
	ROM_RELOAD(0xa800, 0x0800)
	ROM_LOAD("841-02_5.532", 0xb000, 0x1000, CRC(9cd8c04e) SHA1(7d74d8f33a98c9832fda1054187eb7300dbf5f5e))
ROM_END

ROM_START(embryond)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "embd78u2.bin", 0x1000, 0x0800, CRC(7b399975) SHA1(8e8c30884af61ca2003d9d38871d5c4d48bcf177))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "embd79u6.bin", 0x1800, 0x0800, CRC(5742794c) SHA1(f56b85a2370876f38144c8aceb8d513801fd40e1))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk:cpu", 0)
	ROM_LOAD("841-01_4.716", 0xa000, 0x0800, CRC(e8b234e3) SHA1(584e553748b1c6571491150e346d815005948b68))
	ROM_RELOAD(0xa800, 0x0800)
	ROM_LOAD("841-02_5.532", 0xb000, 0x1000, CRC(9cd8c04e) SHA1(7d74d8f33a98c9832fda1054187eb7300dbf5f5e))
ROM_END

ROM_START(embryone)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "emb2xsp.u2", 0x1000, 0x0800, CRC(9e721fd7) SHA1(83e1d081a48e68bb4a61833156c539bcd04c9505))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "emb2xsp.u6", 0x1800, 0x0800, CRC(a421784b) SHA1(9997805abeb8877b96eb0587a56f2f89fbf7fa55))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk:cpu", 0)
	ROM_LOAD("841-01_4.716", 0xa000, 0x0800, CRC(e8b234e3) SHA1(584e553748b1c6571491150e346d815005948b68))
	ROM_RELOAD(0xa800, 0x0800)
	ROM_LOAD("841-02_5.532", 0xb000, 0x1000, CRC(9cd8c04e) SHA1(7d74d8f33a98c9832fda1054187eb7300dbf5f5e))
ROM_END

ROM_START(embryont)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "841-06t_2.732", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52t_6.732", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk:cpu", 0)
	ROM_LOAD("841-01_4.716", 0xa000, 0x0800, CRC(e8b234e3) SHA1(584e553748b1c6571491150e346d815005948b68))
	ROM_RELOAD(0xa800, 0x0800)
	ROM_LOAD("841-02_5.532", 0xb000, 0x1000, CRC(9cd8c04e) SHA1(7d74d8f33a98c9832fda1054187eb7300dbf5f5e))
ROM_END

/*--------------------------------
/ Fathom #1233
/-------------------------------*/
ROM_START(fathom)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "842-08_2.732", 0x1000, 0x0800, CRC(1180f284) SHA1(78be1fa54faba5c5b14f580e41546be685846391))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk:cpu", 0)
	ROM_LOAD("842-01_4.532", 0xa000, 0x1000, CRC(2ac02093) SHA1(a89c1d24f4f3e1f58ca4e476f408835efb368a90))
	ROM_LOAD("842-02_5.532", 0xb000, 0x1000, CRC(736800bc) SHA1(2679d4d76e7258ad18ffe05cf333f21c35adfe0e))
ROM_END

/*--------------------------------
/ Centaur #1239
/-------------------------------*/
ROM_START(centaur)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "848-08_2.732", 0x1000, 0x0800, CRC(8bdcd32b) SHA1(39f64393d3a39a8172b3d80d196253aac1342f40))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk:cpu", 0)
	ROM_LOAD("848-01_3.532", 0x9000, 0x1000, CRC(88322c8a) SHA1(424fd2b107f5fbc3ab8b58e3fa8c285170b1f09a))
	ROM_LOAD("848-02_4.532", 0xa000, 0x1000, CRC(d6dbd0e4) SHA1(62e4c8c1a747c5f6a3a4bf4d0bc80b06a1f70d13))
	ROM_LOAD("848-05_5.716", 0xb000, 0x0800, CRC(cbd765ba) SHA1(bdfae28af46c805f253f02d449dd81575aa9305b))
	ROM_RELOAD(0xb800, 0x0800)
ROM_END

/*--------------------------------
/ Medusa #1245
/-------------------------------*/
ROM_START(medusa)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "845-16_2.732", 0x1000, 0x0800, CRC(b0fbd1ac) SHA1(e876eced0c02a2b4b3c308494e8c453074d0e561))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk:cpu", 0)
	ROM_LOAD("845-01_3.532", 0x9000, 0x1000, CRC(32200e02) SHA1(e75356a20f81a68e6b27d2fa04b8cc9b17f3976a))
	ROM_LOAD("845-02_4.532", 0xa000, 0x1000, CRC(ab95885a) SHA1(fa91cef2a244d25d408585d1e14e1ed8fdc8c845))
	ROM_LOAD("845-05_5.716", 0xb000, 0x0800, CRC(3792a812) SHA1(5c7cc43e57d8e8ded1cc109aa65c4f08052899b9))
	ROM_RELOAD(0xb800, 0x0800)
ROM_END

ROM_START(medusat)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "845-16_2.732", 0x1000, 0x0800, CRC(00000000) SHA1(0000000000000000000000000000000000000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(00000000) SHA1(0000000000000000000000000000000000000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk:cpu", 0)
	ROM_LOAD("845-01_3.532", 0x9000, 0x1000, CRC(32200e02) SHA1(e75356a20f81a68e6b27d2fa04b8cc9b17f3976a))
	ROM_LOAD("845-02_4.532", 0xa000, 0x1000, CRC(ab95885a) SHA1(fa91cef2a244d25d408585d1e14e1ed8fdc8c845))
	ROM_LOAD("845-05_5.716", 0xb000, 0x0800, CRC(3792a812) SHA1(5c7cc43e57d8e8ded1cc109aa65c4f08052899b9))
	ROM_RELOAD(0xb800, 0x0800)
ROM_END

/*--------------------------------
/ Vector #1247
/-------------------------------*/
ROM_START(vector)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "858-11_2.732", 0x1000, 0x0800, CRC(323e286b) SHA1(998387900363fd46d392a931c1f092c886a23c69))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("858-01_2.532", 0x8000, 0x1000, CRC(bd2edef9) SHA1(8f129016440bad5e78d4b073268e76e542b61684))
	ROM_LOAD("858-02_3.532", 0x9000, 0x1000, CRC(c592fb35) SHA1(5201824f129812c907e7d8a4600de23d95fd1eb0))
	ROM_LOAD("858-03_4.532", 0xa000, 0x1000, CRC(8661d312) SHA1(36d04d875382ff5387991d660d031c662b414698))
	ROM_LOAD("858-06_5.532", 0xb000, 0x1000, CRC(3050edf6) SHA1(e028192d9a8c17123b07566c6d73302cec07b440))
ROM_END

ROM_START(vectora)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "858-11_2.732", 0x1000, 0x0800, CRC(323e286b) SHA1(998387900363fd46d392a931c1f092c886a23c69))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "7536fn.u6",    0x1800, 0x0800, CRC(5c892af6) SHA1(a5dedf78de450747627769d03778d5b509930ba6))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("858-01_2.532", 0x8000, 0x1000, CRC(bd2edef9) SHA1(8f129016440bad5e78d4b073268e76e542b61684))
	ROM_LOAD("858-02_3.532", 0x9000, 0x1000, CRC(c592fb35) SHA1(5201824f129812c907e7d8a4600de23d95fd1eb0))
	ROM_LOAD("858-03_4.532", 0xa000, 0x1000, CRC(8661d312) SHA1(36d04d875382ff5387991d660d031c662b414698))
	ROM_LOAD("858-06_5.532", 0xb000, 0x1000, CRC(3050edf6) SHA1(e028192d9a8c17123b07566c6d73302cec07b440))
ROM_END

ROM_START(vector4)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "vectoru2.732", 0x1000, 0x0800, CRC(1b1740d5) SHA1(0e4e932bcdf37ac033fa1f9fe5ead4073a6280f0))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "vectoru6.732", 0x1800, 0x0800, CRC(42d7f176) SHA1(93460c46b3cf033ff23d063e8160cc2c18d00458))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("858-01_2.532", 0x8000, 0x1000, CRC(bd2edef9) SHA1(8f129016440bad5e78d4b073268e76e542b61684))
	ROM_LOAD("858-02_3.532", 0x9000, 0x1000, CRC(c592fb35) SHA1(5201824f129812c907e7d8a4600de23d95fd1eb0))
	ROM_LOAD("858-03_4.532", 0xa000, 0x1000, CRC(8661d312) SHA1(36d04d875382ff5387991d660d031c662b414698))
	ROM_LOAD("858-06_5.532", 0xb000, 0x1000, CRC(3050edf6) SHA1(e028192d9a8c17123b07566c6d73302cec07b440))
ROM_END

ROM_START(vectorb)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "vec2uN.U2", 0x1000, 0x0800, CRC(6b2cbd42) SHA1(f778d19f8ff7a6228ccd3b49af9d60bc9eeffe7b))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "vec2uN.U6", 0x1800, 0x0800, CRC(fe504d05) SHA1(27c72358ea53fd051e64c0179019116356f543d5))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("858-01_2.532", 0x8000, 0x1000, CRC(bd2edef9) SHA1(8f129016440bad5e78d4b073268e76e542b61684))
	ROM_LOAD("858-02_3.532", 0x9000, 0x1000, CRC(c592fb35) SHA1(5201824f129812c907e7d8a4600de23d95fd1eb0))
	ROM_LOAD("858-03_4.532", 0xa000, 0x1000, CRC(8661d312) SHA1(36d04d875382ff5387991d660d031c662b414698))
	ROM_LOAD("858-06_5.532", 0xb000, 0x1000, CRC(3050edf6) SHA1(e028192d9a8c17123b07566c6d73302cec07b440))
ROM_END

ROM_START(vectorc)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "vectorc_OliverV10.128", 0x1000, 0x0800, CRC(5a0fc2d2) SHA1(c1f41d98d53fd19f96d7c8f08d8d83797f874196))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_CONTINUE( 0x1800, 0x0800)
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_CONTINUE( 0x3000, 0x0800)
	ROM_CONTINUE( 0x7000, 0x0800)
	ROM_CONTINUE( 0x3800, 0x0800)
	ROM_CONTINUE( 0x7800, 0x0800)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("858-01_2.532", 0x8000, 0x1000, CRC(bd2edef9) SHA1(8f129016440bad5e78d4b073268e76e542b61684))
	ROM_LOAD("858-02_3.532", 0x9000, 0x1000, CRC(c592fb35) SHA1(5201824f129812c907e7d8a4600de23d95fd1eb0))
	ROM_LOAD("858-03_4.532", 0xa000, 0x1000, CRC(8661d312) SHA1(36d04d875382ff5387991d660d031c662b414698))
	ROM_LOAD("858-06_5.532", 0xb000, 0x1000, CRC(3050edf6) SHA1(e028192d9a8c17123b07566c6d73302cec07b440))
ROM_END

/*--------------------------------
/ Elektra #1248
/-------------------------------*/
ROM_START(elektra)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "857-04_2.732", 0x1000, 0x0800, CRC(d2476720) SHA1(372c210c4f19302ffe25722bba6bcaaa85c4b90d))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("857-01_3.532", 0x9000, 0x1000, CRC(031548cc) SHA1(1f0204afd32dc07a301f404b4b064e34a83bd783))
	ROM_LOAD("857-02_4.532", 0xa000, 0x1000, CRC(efc870d9) SHA1(45132c123b3191d616e2e9372948ab66ff221228))
	ROM_LOAD("857-03_5.716", 0xb000, 0x0800, CRC(eae2c6a6) SHA1(ee3a9b01fa07e2df4eb6d2ab26da5f7f0e12475b))
	ROM_RELOAD(0xb800, 0x0800)
ROM_END

/*--------------------------------
/ Spectrum #1262
/-------------------------------*/
ROM_START(spectrm)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "868-00_2.732", 0x1000, 0x0800, NO_DUMP)
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk:cpu", 0)
	ROM_LOAD("868-01_3.532", 0x9000, 0x1000, CRC(c3a16c66) SHA1(8c0a8b50fac0e218515b471621e80000ae475296))
	ROM_LOAD("868-02_4.532", 0xa000, 0x1000, CRC(6b441399) SHA1(aae9e805f76cd6bc264bf69dd2d57629ee58bfc2))
	ROM_LOAD("868-03_5.716", 0xb000, 0x0800, CRC(4a5ac3b8) SHA1(288feba40efd65f4eec5c0b2fcf013904e3dc24e))
	ROM_RELOAD(0xb800, 0x0800)
ROM_END

ROM_START(spectrm4)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "868-04_2.732", 0x1000, 0x0800, CRC(b377f5f1) SHA1(adc40204da90ef1a4470a478520b949c6ded07b5))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk:cpu", 0)
	ROM_LOAD("868-01_3.532", 0x9000, 0x1000, CRC(c3a16c66) SHA1(8c0a8b50fac0e218515b471621e80000ae475296))
	ROM_LOAD("868-02_4.532", 0xa000, 0x1000, CRC(6b441399) SHA1(aae9e805f76cd6bc264bf69dd2d57629ee58bfc2))
	ROM_LOAD("868-03_5.716", 0xb000, 0x0800, CRC(4a5ac3b8) SHA1(288feba40efd65f4eec5c0b2fcf013904e3dc24e))
	ROM_RELOAD(0xb800, 0x0800)
ROM_END

/*--------------------------------------------------
/ Speakeasy #1273
/--------------------------------------------------*/
ROM_START(speakesy)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "877-03_2.732", 0x1000, 0x0800, CRC(34b28bbc) SHA1(c649a04664e694cfbd6b4d496bf76f5e802d492a))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("877-01_4.716", 0x1000, 0x0800, CRC(6534e826) SHA1(580653636f8d33e758e6631c9ce495f42fe3747a))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

ROM_START(speakesy4p)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "877-04_2.732", 0x1000, 0x0800, CRC(8926f2bb) SHA1(617c032ce949007d6bcb52268f17bec5a02f8651))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("877-01_4.716", 0x1000, 0x0800, CRC(6534e826) SHA1(580653636f8d33e758e6631c9ce495f42fe3747a))
	ROM_RELOAD( 0x1800, 0x0800)
ROM_END

/*---------------------------------------------------
/ BMX #1276
/----------------------------------------------------*/
ROM_START(bmx)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "888-03_2.732", 0x1000, 0x0800, CRC(038cf1be) SHA1(b000a3d84623db6a7644551e5e2f0d7b533acb13))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("888-02_4.532", 0x1000, 0x1000, CRC(5692c679) SHA1(7eef074d16cde589cde7500c4dc76c9a902c7fe3))
ROM_END

/*--------------------------------
/ Rapid Fire #1282
/-------------------------------*/
ROM_START(rapidfip)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "869-04_2.732", 0x1000, 0x0800, CRC(26fdf048) SHA1(15787345e7162a530334bff98d877e525d4a1295))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "869-03_6.732", 0x1800, 0x0800, CRC(f6af5e8d) SHA1(3cf782d4a0ca38e3953a20d23d0eb01af87ba445))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk:cpu", 0)
	ROM_LOAD("869-02_5.532", 0xb000, 0x1000, CRC(5a74cb86) SHA1(4fd09b0bc4257cb7b48cd8087b8b15fe768f7ddf))
ROM_END

/*--------------------------------------
/ Mr. and Mrs. Pacman #1283
/--------------------------------------*/
ROM_START(m_mpac)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "872-04_2.732", 0x1000, 0x0800, CRC(5e542882) SHA1(bec5f56cd5192e0a12ea1226a49a2b7d8eaaa5cf))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("872-01_4.532", 0xa000, 0x1000, CRC(d21ce16d) SHA1(3ee6e2629530e7e6e4d7eac713d34c48297a1047))
	ROM_LOAD("872-03_5.532", 0xb000, 0x1000, CRC(8fcdf853) SHA1(7c6bffcd974d2684e7f2c69d926f6cabb53e2f90))
ROM_END

/*-----------------------------------------------------------
/ Grand Slam #1311
/-----------------------------------------------------------*/
ROM_START(granslam)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "grndslam.u2", 0x1000, 0x0800, CRC(66aea9dc) SHA1(76c017dc83a63b7f1e6035e228370219eb9c0678))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "grndslam.u6", 0x1800, 0x0800, CRC(9e6ccea1) SHA1(5e158e021e0f3eed063577ae22cf5f1bc9655065))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("grndslam.u4", 0x1000, 0x1000, CRC(ac34bc38) SHA1(376ceb53cb51d250b5bc222001291b0c85e42e8a))
ROM_END

ROM_START(granslam4)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "gr_slam.u2b", 0x1000, 0x0800, CRC(552d9423) SHA1(16b86d5b7539fd803f458f1633ecc249ef15243d))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "grndslam.u6", 0x1800, 0x0800, CRC(9e6ccea1) SHA1(5e158e021e0f3eed063577ae22cf5f1bc9655065))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("grndslam.u4", 0x1000, 0x1000, CRC(ac34bc38) SHA1(376ceb53cb51d250b5bc222001291b0c85e42e8a))
ROM_END


/*--------------------------------
/ Centaur II #1370
/-------------------------------*/

/*----------------------------------------------------------
/ Gold Ball #1371
/----------------------------------------------------------*/
ROM_START(goldball)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "896-04_2.732", 0x1000, 0x0800, CRC(3169493c) SHA1(1335fcdfb2d6970d78c636748ff419baf85ef78b))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "896-01_6.732", 0x1800, 0x0800, CRC(9b6e79d0) SHA1(4fcda91bbe930e6131d94964a08459e395f841af))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("gb_u4.532", 0x1000, 0x1000, CRC(2dcb0315) SHA1(8cb9c9f627f0c8420d3b3d9f0d10d77a82c8be56))
ROM_END

ROM_START(goldballn)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "u2.532", 0x1000, 0x0800, CRC(aa6eb9d6) SHA1(a73cc832450e718d9b8484e409a1f8093d91d786))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "896-01_6.732", 0x1800, 0x0800, CRC(9b6e79d0) SHA1(4fcda91bbe930e6131d94964a08459e395f841af))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("gb_u4.532", 0x1000, 0x1000, CRC(2dcb0315) SHA1(8cb9c9f627f0c8420d3b3d9f0d10d77a82c8be56))
ROM_END

/*--------------------------------
/ Kings of Steel #1390
/-------------------------------*/
ROM_START(kosteel)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "kngs2732.u2", 0x1000, 0x0800, CRC(f876d8f2) SHA1(581f4b98e0a69f4ae879caeafdbf2eb979514ad1))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-5332.u6", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("kngsu4.snd", 0x8000, 0x1000, CRC(f3e4d2f6) SHA1(93f4e9e1348b1225bc02db38c994e3338afb175c))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_LOAD("kngsu3.snd", 0xc000, 0x1000, CRC(11b02dca) SHA1(464eee1aa1fd9b6e26d4ba635777fffad0222106))
	ROM_RELOAD(0xd000, 0x1000)
ROM_END

/*--------------------------------
/ X's & O's #1391
/-------------------------------*/
ROM_START(xsandos)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "x+os2732.u2", 0x1000, 0x0800, CRC(068dfe5a) SHA1(028baf79852b14cac51a7cdc8e751a8173beeccb))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-5332.u6", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("720_u3.snd", 0xc000, 0x2000, CRC(5d8e2adb) SHA1(901a26f5e598386295a1298ee3a634941bd58b3e))
ROM_END

/*--------------------------------
/ Spy Hunter #0A17
/-------------------------------*/
ROM_START(spyhuntr)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "spy-2732.u2", 0x1000, 0x0800, CRC(9e930f2d) SHA1(fb48ce0d8d8f8a695827c0eea57510b53daa7c39))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-5332.u6", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("spy_u4.532", 0x8000, 0x1000, CRC(a43887d0) SHA1(6bbc55943fa9f0cd97f946767f21652e19d85265))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_LOAD("spy_u3.532", 0xc000, 0x1000, CRC(95ffc1b8) SHA1(28f058f74abbbee120dca06f7321bcb588bef3c6))
	ROM_RELOAD(0xd000, 0x1000)
ROM_END

/*-------------------------------------
/ Fireball Classic #0A40
/------------------------------------*/
ROM_START(fbclass)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "fb-class.u2", 0x1000, 0x0800, CRC(32faac6c) SHA1(589020d09f26326dab266bc7c74ca0e10de565e6))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-5332.u6", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("fbcu4.snd", 0x8000, 0x1000, CRC(697ab16f) SHA1(7beed02e6cb042f90d2048778408b1f744ffe242))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_LOAD("fbcu3.snd", 0xc000, 0x1000, CRC(1ad71775) SHA1(ddb885730deaf315fe7f3c1803628c06eedc8350))
	ROM_RELOAD(0xd000, 0x1000)
ROM_END

/*--------------------------------
/ Black Pyramid #0A44
/-------------------------------*/
#if 1
ROM_START(blakpyra)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "blkp2732.u2", 0x1000, 0x0800, CRC(600535b0) SHA1(33d080f4430ad9c33ee9de1bfbb5cfde50f0776e))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-5332.u6", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("bp_u4.532", 0x8000, 0x1000, CRC(57978b4a) SHA1(4995837790d81b02325d39b548fb882a591769c5))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_LOAD("bp_u3.532", 0xc000, 0x1000, CRC(a5005067) SHA1(bd460a20a6e8f33746880d72241d6776b85126cf))
	ROM_RELOAD(0xd000, 0x1000)
ROM_END
#else
ROM_START(blakpyra)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "blkp27512", 0x8000, 0x1000, CRC(00000000) SHA1(0000000000000000000000000000000000000000))
	ROM_CONTINUE( 0x1000, 0xf000)
	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("bp_u4.532", 0x8000, 0x1000, CRC(57978b4a) SHA1(4995837790d81b02325d39b548fb882a591769c5))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_LOAD("bp_u3.532", 0xc000, 0x1000, CRC(a5005067) SHA1(bd460a20a6e8f33746880d72241d6776b85126cf))
	ROM_RELOAD(0xd000, 0x1000)
ROM_END
#endif

/*--------------------------------
/ Cybernaut #0B42
/-------------------------------*/
ROM_START(cybrnaut)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "cybe2732.u2", 0x1000, 0x0800, CRC(0610b0e0) SHA1(92f5e8a83240ad03ecc16ece4824b047b77816f7))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-5332.u6", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("cybu3.snd", 0xc000, 0x2000, CRC(a3c1f6e7) SHA1(35a5e828a6f2dd9009e165328a005fa079bad6cb))
ROM_END

/*--------------------------------
/ Cosmic Flash (Flash Gordon Clone)
/-------------------------------*/
ROM_START(cosflash)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "cf2d.532", 0x1000, 0x0800, CRC(939e941d) SHA1(889862043f351762e8c866aefb36a9ea75cbf828))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "cf6d.532", 0x1800, 0x0800, CRC(7af93d2f) SHA1(2d939b14f7fe79f836e12926f44b70037630cd3f))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("cf-sound.532", 0x1000, 0x1000, CRC(ac34bc38) SHA1(9993ba890e91613014bad0950511bcff522b8dbd))
ROM_END

ROM_START(cosflasht)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "cosmicflasht.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "cosmicflasht.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("cf-sound.532", 0x1000, 0x1000, CRC(ac34bc38) SHA1(9993ba890e91613014bad0950511bcff522b8dbd))
ROM_END

/*--------------------------------
/ Dark Rider
/-------------------------------*/

/*--------------------------------
/ Fantasy
/-------------------------------*/

/*--------------------------------
/ Fly High
/-------------------------------*/

/*--------------------------------
/ Genesis
/-------------------------------*/

/*--------------------------------
/ Miss World
/-------------------------------*/

/*--------------------------------
/ Mystic Star
/-------------------------------*/
ROM_START(myststar)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "rom1.bin", 0x1000, 0x0800, CRC(9a12dc91) SHA1(8961c22b2aeabac04d36d124f283409e11faee8a))
	ROM_LOAD( "rom2.bin", 0x5000, 0x0800, CRC(888ee5ae) SHA1(d99746c7c9a9a0a83b4bc15473fe9ebd3b02ffe4))
	ROM_LOAD( "rom3.bin", 0x5800, 0x0800, CRC(9e0a4619) SHA1(82065b74d39ba932704514e83d432262d360f1e1))
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x0020, "sound1", 0)
	ROM_LOAD( "snd.123", 0x0000, 0x0020, NO_DUMP)
ROM_END

/*--------------------------------
/ New Wave (Black Pyramid Clone)
/-------------------------------*/
ROM_START(newwave)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "blkp2732.u2", 0x1000, 0x0800, CRC(600535b0) SHA1(33d080f4430ad9c33ee9de1bfbb5cfde50f0776e))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "newwu6.532", 0x1800, 0x0800, CRC(ca72a96b) SHA1(efcd8b41bf0c19ebd7db492632e046b348619460))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("newwu4.532", 0x8000, 0x1000, CRC(6f4f2a95) SHA1(a7a375827c0429b8b3d2ee9e471f557152492993))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_LOAD("bp_u3.532", 0xc000, 0x1000, CRC(a5005067) SHA1(bd460a20a6e8f33746880d72241d6776b85126cf))
	ROM_RELOAD(0xd000, 0x1000)
ROM_END

ROM_START(newwavet)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "newwavet.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "newwavet.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("newwu4.532", 0x8000, 0x1000, CRC(6f4f2a95) SHA1(a7a375827c0429b8b3d2ee9e471f557152492993))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_LOAD("bp_u3.532", 0xc000, 0x1000, CRC(a5005067) SHA1(bd460a20a6e8f33746880d72241d6776b85126cf))
	ROM_RELOAD(0xd000, 0x1000)
ROM_END

/*--------------------------------
/ Pin Ball Pool
/-------------------------------*/

/*--------------------------------
/ Pinball
/-------------------------------*/

/*--------------------------------
/ Saturn 2 (Spy Hunter Clone)
/-------------------------------*/
ROM_START(saturn2)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "spy-2732.u2", 0x1000, 0x0800, CRC(9e930f2d) SHA1(fb48ce0d8d8f8a695827c0eea57510b53daa7c39))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "saturn2.u6", 0x1800, 0x0800, CRC(ca72a96b) SHA1(efcd8b41bf0c19ebd7db492632e046b348619460))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("sat2_snd.764", 0xc000, 0x2000, CRC(6bf15541) SHA1(dcdd4e8f662818584de9b1ed7ae69d57362ebadb))
ROM_END

ROM_START(saturn2t)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "saturn2t.u2", 0x1000, 0x0800, CRC(9e930f2d) SHA1(fb48ce0d8d8f8a695827c0eea57510b53daa7c39))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "saturn2t.u6", 0x1800, 0x0800, CRC(ca72a96b) SHA1(efcd8b41bf0c19ebd7db492632e046b348619460))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("sat2_snd.764", 0xc000, 0x2000, CRC(6bf15541) SHA1(dcdd4e8f662818584de9b1ed7ae69d57362ebadb))
ROM_END

/*--------------------------------
/ Sexy Girl
/-------------------------------*/

/*--------------------------------
/ Space Hawks (Cybernaut Clone)
/-------------------------------*/
ROM_START(spacehaw)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "cybe2732.u2g", 0x1000, 0x0800, CRC(d4a5e2f6) SHA1(841e940632993919a68c905546f533ff38a0ce31))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "spacehaw.u6", 0x1800, 0x0800, CRC(b154a3a3) SHA1(d632c5eddd0582ba2ca778ab03e11ca3f6f4e1ed))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("spacehaw.snd", 0x8000, 0x1000, CRC(2b548d24) SHA1(83ac9b75ae9c1960ad73abcf40adc2bc46827568))
	ROM_CONTINUE( 0xc000, 0x1000)
	ROM_RELOAD( 0x9000, 0x1000)
	ROM_CONTINUE( 0xd000, 0x1000)
ROM_END

ROM_START(spacehawt)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "spacehawt.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "spacehawt.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("spacehaw.snd", 0x9000, 0x1000, CRC(2b548d24) SHA1(83ac9b75ae9c1960ad73abcf40adc2bc46827568))
	ROM_CONTINUE( 0xd000, 0x1000)
ROM_END

/*--------------------------------
/ Space Rider
/-------------------------------*/

/*--------------------------------
/ Super Bowl
/-------------------------------*/

/*--------------------------------
/ Tiger Rag (Kings Of Steel Clone)
/-------------------------------*/
ROM_START(tigerrag)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("tigerrag.mpu", 0x1000, 0x0800, CRC(3eb389ba) SHA1(bdfdcf00f4a2200d39d7e469fe633e0b7b8f1676))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_CONTINUE( 0x1800, 0x0800)
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("kngsu4.snd", 0x8000, 0x1000, CRC(f3e4d2f6) SHA1(93f4e9e1348b1225bc02db38c994e3338afb175c))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_LOAD("kngsu3.snd", 0xc000, 0x1000, CRC(11b02dca) SHA1(464eee1aa1fd9b6e26d4ba635777fffad0222106))
	ROM_RELOAD(0xd000, 0x1000)
ROM_END

ROM_START(tigerragt)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "tigerragt.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "tigerragt.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("kngsu4.snd", 0x8000, 0x1000, CRC(f3e4d2f6) SHA1(93f4e9e1348b1225bc02db38c994e3338afb175c))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_LOAD("kngsu3.snd", 0xc000, 0x1000, CRC(11b02dca) SHA1(464eee1aa1fd9b6e26d4ba635777fffad0222106))
	ROM_RELOAD(0xd000, 0x1000)
ROM_END

/*--------------------------------
/ Unofficial
/-------------------------------*/
/*--------------------------------
/ 301/Bulls Eye
/-------------------------------*/
ROM_START(bullseye)                     // Has darts 301 based scoring
	ROM_REGION(0x4000, "maincpu", 0)    // Has an address mask of 0x3fff
	ROM_LOAD("bull.u2", 0x2000, 0x1000, CRC(a2951aa2) SHA1(f9c0826c5d1d6d904286678ed90de3850a13b5f4))
	ROM_LOAD("bull.u6", 0x3000, 0x1000, CRC(64d4b9c4) SHA1(bf4d0671372fd3a445c4c7330b9849171ca8048c))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("bull.snd", 0x8000, 0x0800, CRC(c0482a2f) SHA1(a6aa698ad517cdc078129d702ee936af576260ed))
	ROM_RELOAD(0x8800, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

ROM_START(bullseyn)                     // Later version dumbed down with traditional Pinball scoring
	ROM_REGION(0x4000, "maincpu", 0)    // Has an address mask of 0x3fff
	ROM_LOAD("301new_normalscoring.u2", 0x2000, 0x1000, CRC(febebc63) SHA1(9221b02bc5952203f5b2527e4c40d17d5986abdf))
	ROM_LOAD("301new_normalscoring.u6", 0x3000, 0x1000, CRC(1357cd6a) SHA1(4e02c96b141dab6cdea1a15539214976eb052838))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("bull.snd", 0x8000, 0x0800, CRC(c0482a2f) SHA1(a6aa698ad517cdc078129d702ee936af576260ed))
	ROM_RELOAD(0x8800, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

ROM_START(bullseyet)                    // Has darts 301 based scoring
	ROM_REGION(0x8000, "maincpu", 0)    // Recompiled to run on traditional Bally-35 MPU addressing
	ROM_LOAD( "bullseyet.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "bullseyet.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("bull.snd", 0x8000, 0x0800, CRC(c0482a2f) SHA1(a6aa698ad517cdc078129d702ee936af576260ed))
	ROM_RELOAD(0x8800, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

ROM_START(bullseynt)                    // Later version dumbed down with traditional Pinball scoring
	ROM_REGION(0x8000, "maincpu", 0)    // Recompiled to run on traditional Bally-35 MPU addressing
	ROM_LOAD( "bullseynt.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "bullseynt.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("bull.snd", 0x8000, 0x0800, CRC(c0482a2f) SHA1(a6aa698ad517cdc078129d702ee936af576260ed))
	ROM_RELOAD(0x8800, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

/*--------------------------------
/ World Defender (Software based on Ballys Eight Ball Deluxe)
/-------------------------------*/
ROM_START(worlddef)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("worlddef.764", 0x1000, 0x0800, CRC(ad1a7ba3) SHA1(d799b6d1cd252cd6d9fb72586099c43de7c22a00))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_CONTINUE( 0x1800, 0x0800)
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("wodefsnd.764", 0xc000, 0x2000, CRC(b8d4dc20) SHA1(5aecac4a2deb7ea8e0ff0600ea459ef272dcd5f0))
ROM_END

ROM_START(worlddeft)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "worlddefendert.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "worlddefendert.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("wodefsnd.764", 0xc000, 0x2000, CRC(b8d4dc20) SHA1(5aecac4a2deb7ea8e0ff0600ea459ef272dcd5f0))
ROM_END


/*--------------------------------
/ Dark Shadow
/-------------------------------*/
ROM_START(darkshad)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u7.bin", 0x1000, 0x0800, CRC(8d04c546) SHA1(951e75d9867b85a0bf9f04fe9aa647a53b6830bc))
	ROM_CONTINUE( 0x1800, 0x0800)
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0xfff8,0x08)

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("darkshad.snd", 0x8000, 0x1000, CRC(9fd6ee82) SHA1(6486fa56c663152e565e160b8f517be824338a9a))
	ROM_CONTINUE( 0xc000, 0x1000)
	ROM_RELOAD( 0x9000, 0x1000)
	ROM_CONTINUE( 0xd000, 0x1000)
ROM_END

ROM_START(darkshadt)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "darkshadowt.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "darkshadowt.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0xfff8,0x08)

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD("darkshad.snd", 0x9000, 0x1000, CRC(9fd6ee82) SHA1(6486fa56c663152e565e160b8f517be824338a9a))
	ROM_CONTINUE( 0xd000, 0x1000)
ROM_END

/*--------------------------------
/ Based of Nuova hardware
/-------------------------------*/

/*--------------------------------
/ Skill Flight
/-------------------------------*/
ROM_START(skflight)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("game_u7.64", 0xe000, 0x2000, CRC(fe5001eb) SHA1(f7d56d484141ba8ec82664b6aebbf3a683547d20))
	ROM_LOAD("game_u8.64", 0xc000, 0x2000, CRC(58f259fe) SHA1(505f3996f66dbb4027bd47f6b7ba9e4baaeb6e51))
	ROM_COPY("maincpu", 0xc000, 0x9000,0x1000)
	ROM_COPY("maincpu", 0xe000, 0x1000,0x1000)
	ROM_COPY("maincpu", 0xf000, 0x5000,0x1000)
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("snd_u3.256", 0x0000, 0x8000, CRC(43424fb1) SHA1(428d2f7444cd71b6c49c04749b42263e3c185856))
	ROM_RELOAD(0x10000, 0x8000)
	ROM_RELOAD(0x20000, 0x8000)
	ROM_RELOAD(0x30000, 0x8000)
	ROM_LOAD("snd_u4.256", 0x8000, 0x8000, CRC(10378feb) SHA1(5da2b9c530167c80b9d411da159e4b6e95b76647))
	ROM_RELOAD(0x18000, 0x8000)
	ROM_RELOAD(0x28000, 0x8000)
	ROM_RELOAD(0x38000, 0x8000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_COPY("sound1", 0x0000, 0x8000,0x8000)
ROM_END

/*--------------------------------
/ Cobra
/-------------------------------*/
ROM_START(cobrap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u7.256", 0xc000, 0x4000, CRC(c0f89577) SHA1(16d351f2bf642bf886e808b58173b3e699a44fd6))
	ROM_COPY("maincpu", 0xc000, 0x1000,0x1000)
	ROM_COPY("maincpu", 0xd000, 0x5000,0x1000)
	ROM_COPY("maincpu", 0xe000, 0x9000,0x1000)
	ROM_COPY("maincpu", 0xf000, 0xd000,0x1000)
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("snd_u8.256", 0x00000,0x8000, CRC(cdf2a28d) SHA1(d4969370109b4c7f31f48a3ebd8925268caf9c44))
	ROM_RELOAD(0x20000, 0x8000)
	ROM_LOAD("snd_u9.256", 0x08000,0x8000, CRC(08bd0db9) SHA1(af851b8c993649b61645a414459000c206516bec))
	ROM_RELOAD(0x28000, 0x8000)
	ROM_LOAD("snd_u10.256",0x10000,0x8000, CRC(634bc64c) SHA1(8389fda08ee7bf0e5002153cec22e219bf786993))
	ROM_RELOAD(0x30000, 0x8000)
	ROM_LOAD("snd_u11.256",0x18000,0x8000, CRC(d4da383c) SHA1(032a4a425936d5c822fba6e46483f03a87c1a6ec))
	ROM_RELOAD(0x38000, 0x8000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_COPY("sound1", 0x0000, 0x8000,0x8000)
ROM_END

/*--------------------------------
/ Future Queen
/-------------------------------*/
ROM_START(futrquen)
	ROM_REGION(0x10000, "maincpu", 0)
#if 0
	ROM_LOAD("mpu_u2.bin", 0xc000, 0x4000, CRC(bc66b636) SHA1(65f3e6461a1eca8542bbbc5b8c7cd1fca1b3011f))
	ROM_COPY("maincpu", 0xc000, 0x1000,0x0800)
	ROM_COPY("maincpu", 0xd000, 0x1800,0x0800)
	ROM_COPY("maincpu", 0xc800, 0x5000,0x0800)
	ROM_COPY("maincpu", 0xd800, 0x5800,0x0800)
	ROM_COPY("maincpu", 0xe000, 0x9000,0x0800)
	ROM_COPY("maincpu", 0xf000, 0x9800,0x0800)
	ROM_COPY("maincpu", 0xe800, 0xd000,0x0800)
	ROM_COPY("maincpu", 0xf800, 0xd800,0x0800)
#else
	ROM_LOAD("mpu_u2.bin", 0x1000, 0x0800, CRC(bc66b636) SHA1(65f3e6461a1eca8542bbbc5b8c7cd1fca1b3011f))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_CONTINUE( 0x1800, 0x0800)
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_CONTINUE( 0x9000, 0x0800)
	ROM_CONTINUE( 0xd000, 0x0800)
	ROM_CONTINUE( 0x9800, 0x0800)
	ROM_CONTINUE( 0xd800, 0x0800)
	ROM_COPY("maincpu", 0xd800, 0xf800,0x0800)
#endif

	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("snd_u8.bin", 0x00000,0x8000, CRC(3d254d89) SHA1(2b4aa3387179e2c0fbf18684128761d3f778dcb2))
	ROM_RELOAD(0x20000, 0x8000)
	ROM_LOAD("snd_u9.bin", 0x08000,0x8000, CRC(9560f2c3) SHA1(3de6d074e2a3d3c8377fa330d4562b2d266bbfff))
	ROM_RELOAD(0x28000, 0x8000)
	ROM_LOAD("snd_u10.bin",0x10000,0x8000, CRC(70f440bc) SHA1(9fa4d33cc6174ce8f43f030487171bfbacf65537))
	ROM_RELOAD(0x30000, 0x8000)
	ROM_LOAD("snd_u11.bin",0x18000,0x8000, CRC(71d98d17) SHA1(9575b80a91a67b1644e909f70d364e0a75f73b02))
	ROM_RELOAD(0x38000, 0x8000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_COPY("sound1", 0x0000, 0x8000,0x8000)
ROM_END
/*--------------------------------
/ F1 Grand Prix
/-------------------------------*/
ROM_START(f1gpp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u7", 0x8000, 0x8000, CRC(2287dea1) SHA1(5438752bf63aadaa6b6d71bbf56a72d8b67b545a))
	ROM_COPY("maincpu", 0x8000, 0x1000,0x1000)
	ROM_COPY("maincpu", 0x9000, 0x5000,0x1000)
	ROM_COPY("maincpu", 0xd000, 0x7000,0x1000)
	ROM_COPY("maincpu", 0xa000, 0x9000,0x1000)
	ROM_COPY("maincpu", 0xb000, 0xd000,0x1000)
	ROM_COPY("maincpu", 0xe000, 0xb000,0x1000)
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("snd_u8a", 0x20000,0x8000, CRC(3a2af90b) SHA1(f6eeae74b3bfb1cfd9235c5214f7c029e0ad14d6))
	ROM_LOAD("snd_u8b", 0x00000,0x8000, CRC(14cddb29) SHA1(667b54174ad5dd8aa45037574916ecb4ee996a94))
	ROM_LOAD("snd_u9a", 0x28000,0x8000, CRC(681ee99c) SHA1(955cd782073a1ce0be7a427c236d47fcb9cccd20))
	ROM_LOAD("snd_u9b", 0x08000,0x8000, CRC(726920b5) SHA1(002e7a072a173836c89746cceca7e5d2ac26356d))
	ROM_LOAD("snd_u10a",0x30000,0x8000, CRC(4d3fc9bb) SHA1(d43cd134f399e128a678b86e57b1917fad70df76))
	ROM_LOAD("snd_u10b",0x10000,0x8000, CRC(9de359fb) SHA1(ce75a78dc4ed747421a386d172fa0f8a1369e860))
	ROM_LOAD("snd_u11a",0x38000,0x8000, CRC(884dc754) SHA1(b121476ea621eae7a7ba0b9a1b5e87051e1e9e3d))
	ROM_LOAD("snd_u11b",0x18000,0x8000, CRC(2394b498) SHA1(bf0884a6556a27791e7e801051be5975dd6b95c4))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_COPY("sound1", 0x0000, 0x8000,0x8000)
ROM_END

/*--------------------------------
/ Top Pin
/-------------------------------*/
ROM_START(toppin)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_256.bin", 0xc000, 0x4000, CRC(3aa32c96) SHA1(989fdc642efe6fa41319d7ccae6681ab4d76feb4))
	ROM_COPY("maincpu", 0xc000, 0x1000,0x0800)
	ROM_COPY("maincpu", 0xd000, 0x1800,0x0800)
	ROM_COPY("maincpu", 0xc800, 0x5000,0x0800)
	ROM_COPY("maincpu", 0xd800, 0x5800,0x0800)
	ROM_COPY("maincpu", 0xe000, 0x9000,0x0800)
	ROM_COPY("maincpu", 0xf000, 0x9800,0x0800)
	ROM_COPY("maincpu", 0xe800, 0xd000,0x0800)
	ROM_COPY("maincpu", 0xf800, 0xd800,0x0800)
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("snd_u8.bin", 0x00000,0x8000, CRC(2cb9c931) SHA1(2537976c890ceff857b9aaf204c48ab014aad94e))
	ROM_RELOAD(0x20000, 0x8000)
	ROM_LOAD("snd_u9.bin", 0x08000,0x8000, CRC(72690344) SHA1(c2a13aa59f0c605eb616256cd288b79cceca003b))
	ROM_RELOAD(0x28000, 0x8000)
	ROM_LOAD("snd_u10.bin",0x10000,0x8000, CRC(bca9a805) SHA1(0deb3172b5c8fc91c4b02b21b1e3794ed7adef13))
	ROM_RELOAD(0x30000, 0x8000)
	ROM_LOAD("snd_u11.bin",0x18000,0x8000, CRC(513d06a9) SHA1(3785398649fde5579b5a0461b52360ef83d71323))
	ROM_RELOAD(0x38000, 0x8000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_COPY("sound1", 0x0000, 0x8000,0x8000)
ROM_END

/*--------------------------------
/ U-boat 65
/-------------------------------*/
ROM_START(uboat65)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u7.256", 0x8000, 0x8000, CRC(f0fa1cbc) SHA1(4373bb37927dde01f5a4da5ef6094424909e9bc6))
	ROM_COPY("maincpu", 0x8000, 0x1000,0x1000)
	ROM_COPY("maincpu", 0x9000, 0x5000,0x1000)
	ROM_COPY("maincpu", 0xd000, 0x7000,0x1000)
	ROM_COPY("maincpu", 0xa000, 0x9000,0x1000)
	ROM_COPY("maincpu", 0xb000, 0xd000,0x1000)
	ROM_COPY("maincpu", 0xe000, 0xb000,0x1000)
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("snd_ic3.256", 0x0000, 0x8000, CRC(c7811983) SHA1(7924248dcc08b05c34d3ddf2e488b778215bc7ea))
	ROM_RELOAD(0x10000, 0x8000)
	ROM_RELOAD(0x20000, 0x8000)
	ROM_RELOAD(0x30000, 0x8000)
	ROM_LOAD("snd_ic5.256", 0x8000, 0x8000, CRC(bc35e5cf) SHA1(a809b0056c576416aa76ead0437e036c2cdbd1ef))
	ROM_RELOAD(0x18000, 0x8000)
	ROM_RELOAD(0x28000, 0x8000)
	ROM_RELOAD(0x38000, 0x8000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd_u8.bin", 0x8000, 0x8000, CRC(d00fd4fd) SHA1(23f6b7c5d60821eb7fa2fdcfc85caeb536eef99a))
ROM_END
/*--------------------------------
/ Big Ball Bowling (Bowler)
/-------------------------------*/
ROM_START(bbbowlin)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(179e0c69) SHA1(7921839d2014a00b99ce7c44b325ea4403df9eea))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(7b48e45b) SHA1(ac32292ef593bf8350e8bbc41113b6c1cb78a79e))
	ROM_RELOAD( 0x7800, 0x0800)
ROM_END

/*----------------------------
/ Stars & Strikes (Bowler)
/----------------------------*/
ROM_START(monrobwl)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(42592cc9) SHA1(22452072199c4b82a413065f8dfe235a39fe3825))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(78e2dcd2) SHA1(7fbe9f7adc69af5afa489d9fd953640f3466de3f))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(73534680) SHA1(d5233a9d4600fa28b767ee1a251ed1a1ffbaf9c4))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(ad77d719) SHA1(f8f8d0d183d639d19fea552d35a7be3aa7f07c17))
	ROM_RELOAD( 0x7800, 0x0800)
ROM_END

/*-----------------------------------------------------------------------------------------------
/ Big Bat (Bat game)
/------------------------------------------------------------------------------------------------*/
ROM_START(bigbat)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "u2.bin", 0x1000, 0x0800, CRC(2beda24d) SHA1(80fb9ed548e4886741c709979aa4f865f47d2257))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "u6.bin", 0x1800, 0x0800, CRC(8f13469d) SHA1(00c626f7eb166f627f6498d75906b3c56bccdd62))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("u3.bin", 0x9000, 0x1000, CRC(b87a9335) SHA1(8a21bcbcbe91da1bab0af06b71604bb8f247d0d4))
	ROM_LOAD("u4.bin", 0xa000, 0x1000, CRC(4ab75b31) SHA1(46acd1c9250a635b51bffccd77ea4e67a0c5edf5))
	ROM_LOAD("u5.bin", 0xb000, 0x1000, CRC(0aec8204) SHA1(f44216cccc3652399549345d8c74bcae54662aa3))
ROM_END

/*-----------------------------------------------------------------------------------------------
/ Midnight Marauders (Gun game) different hardware, not a pinball, to be moved to its own driver
/------------------------------------------------------------------------------------------------*/
ROM_START(mdntmrdr)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "mdru2.532", 0x1000, 0x0800, CRC(f72668bc) SHA1(25b984e1828905190c73c359ee6c9858ed1b2224))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "mdru6.732", 0x1800, 0x0800, CRC(ff55fb57) SHA1(4a44fc8732c8cbce38c9605c7958b02a6bc95da1))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "squawk_n_talk_ay:cpu", 0)
	ROM_LOAD("u3.bin", 0x9000, 0x1000, CRC(3ba474e4) SHA1(4ee5c3ad2c9dca49e9394521506e97a95e3d9a17))
	ROM_LOAD("u5.bin", 0xb000, 0x1000, CRC(3ab40e35) SHA1(63b2ee074e5993a2616e67d3383bc3d3ac51b400))
ROM_END

/*----------------------------
/ Black Beauty (Shuffle)
/----------------------------*/
ROM_START(blbeauty)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(e2550957) SHA1(e445548b650fec5d593ca7da587300799ef94991))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(70fcd9f7) SHA1(ca5c2ea09f45f5ba50526880c158aaac61f007d5))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(3f55d17f) SHA1(e6333e53570fb05a841a7f141872c8bd14143f9c))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(842cd307) SHA1(8429d84e8bc4343b437801d0236150e04de79b75))
	ROM_RELOAD( 0x7800, 0x0800)
ROM_END

/*--------------------------------
/ Super Bowl (X's & O's Clone)
/-------------------------------*/
ROM_START(suprbowl)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "sbowlu2.732", 0x1000, 0x0800, CRC(bc497a13) SHA1(f428373bde72f0302c45c326aebbe56e8b09c2d6))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "sbowlu6.732", 0x1800, 0x0800, CRC(a9c92719) SHA1(972da0cf87863b637b88575c329f1d8162098d6f))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("suprbowl.snd", 0x1000, 0x1000, CRC(97fc0f7a) SHA1(595aa080a6d2c1ab7e718974c4d01e846e142cc1))

//	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
//	ROM_LOAD("suprbowl.snd", 0x8000, 0x1000, CRC(97fc0f7a) SHA1(595aa080a6d2c1ab7e718974c4d01e846e142cc1))
//	ROM_CONTINUE( 0xc000, 0x1000)
//	ROM_RELOAD( 0x9000, 0x1000)
//	ROM_CONTINUE( 0xd000, 0x1000)
ROM_END

ROM_START(suprbowlt)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "suprbowlt.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "suprbowlt.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_COPY("maincpu", 0x5ff8, 0x7ff8,0x08)

	ROM_REGION(0x10000, "as3022:cpu", 0)
	ROM_LOAD("suprbowl.snd", 0x1000, 0x1000, CRC(97fc0f7a) SHA1(595aa080a6d2c1ab7e718974c4d01e846e142cc1))
ROM_END


/*--------------------------------
/ 64kbit ROM/RAM qualification test
/-------------------------------*/
ROM_START(64ktest)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "64ktest_full.u2", 0x1000, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "64ktest_full.u6", 0x1800, 0x0800, CRC(00000000))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0x7800, 0x0800)
ROM_END



//  ( YEAR  NAME        PARENT    MACHINE           INPUT      STATE           INIT         MONITOR COMPANY  FULLNAME                             FLAGS )
// AS-2888 sound
GAMEL(1979, sst,        0,        as2888,           sst,       sst_state,      init_by35_6,   ROT0, "Bally", "Supersonic",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_sst)
GAMEL(1979, sstb,       sst,      as2888,           sst,       sst_state,      init_by35_6,   ROT0, "Bally", "Supersonic (7-digit conversion rev. 20)",              MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_sst)
GAMEL(1979, sstc,       sst,      as2888,           sst,       sst_state,      init_by35_6,   ROT0, "Bally", "Supersonic (Free Play rev. 3)",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_sst)
GAMEL(1979, sstd,       sst,      as2888,           sst,       sst_state,      init_by35_6,   ROT0, "Bally", "Supersonic (/10 Scoring Free Play)",                   MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_sst)
GAMEL(1979, sstt,       sst,      as2888,           sst,       sst_state,      init_by35_6,   ROT0, "Bally", "Supersonic TEST",                   MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_sst)
GAMEL(1979, sstf,       sst,      as2888,           sstf,      sst_state,      init_by35_6,   ROT0, "Bally", "Supersonic TEST NewOS",             MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_sst)

GAMEL(1978, playboy,    0,        as2888,           playboy,   playboy_state,  init_by35_6,   ROT0, "Bally", "Playboy",                           MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_playboy)
GAMEL(1978, playboyt,   playboy,  as2888,           playboyt,  playboy_state,  init_by35_6,   ROT0, "Bally", "Playboy TEST",                      MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_playboy)
GAMEL(1978, playboy2,   playboy,  as2888,           playboyt,  playboy_state,  init_by35_6,   ROT0, "Bally", "Playboy TEST Gen 2",                MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_playboy)
GAMEL(1978, playboyl,   playboy,  as2888_ext,       playboyt,  playboy_state,  init_by35_6,   ROT0, "Bally", "Playboy TEST 128kbit 16kByte ROM",  MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_playboy)
GAMEL(1978, playboys,   playboy,  as2888,           playboy,   playboy_state,  init_by35_6,   ROT0, "Bally", "Playboy TEST Bally-17",             MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_playboy)
GAMEL(1978, lostwrlp,   0,        as2888,           lostwrlp,  lostwrlp_state, init_by35_6,   ROT0, "Bally", "Lost World",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_lostwrlp)
GAMEL(1978, smman,      0,        as2888,           smman,     smman_state,    init_6mdman,   ROT0, "Bally", "Six Million Dollar Man",            MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_smman)
GAMEL(1978, smmanb,     smman,    as2888,           smman,     smman_state,    init_6mdman,   ROT0, "Bally", "Six Million Dollar Man (7-digit conversion rev. 20)",  MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_smman)
GAMEL(1978, smmanc,     smman,    as2888,           smman,     smman_state,    init_6mdman,   ROT0, "Bally", "Six Million Dollar Man (Free Play rev. 3)",            MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_smman)
GAMEL(1978, smmand,     smman,    as2888,           smman,     smman_state,    init_6mdman,   ROT0, "Bally", "Six Million Dollar Man (/10 Scoring Free Play)",       MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_smman)
GAME( 1978, voltan,     0,        as2888,           by35_30,   by35_state,     init_by35_6,   ROT0, "Bally", "Voltan Escapes Cosmic Doom",        MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAMEL(1979, startrep,   0,        as2888,           startrep,  startrep_state, init_by35_6,   ROT0, "Bally", "Star Trek (Pinball)",               MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_startrep)
GAMEL(1979, startrekt,  startrep, as2888,           startrekt, startrep_state, init_by35_6,   ROT0, "Bally", "Star Trek TEST (Pinball)",          MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_startrep)
GAME( 1979, kiss,       0,        as2888,           kiss,      kiss_state,     init_by35_6,   ROT0, "Bally", "Kiss",                              MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, kissb,      kiss,     as2888,           kiss,      kiss_state,     init_by35_6,   ROT0, "Bally", "Kiss (7-digit conversion rev. 20)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, kissc,      kiss,     as2888,           kiss,      kiss_state,     init_by35_6,   ROT0, "Bally", "Kiss (7-digit Free Play rev. 3)",   MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, kissd,      kiss,     as2888,           kiss,      kiss_state,     init_by35_6,   ROT0, "Bally", "Kiss (/10 Scoring Free Play)",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, hglbtrtr,   0,        as2888,           hglbtrtr,  hglbtrtr_state, init_by35_6,   ROT0, "Bally", "Harlem Globetrotters On Tour",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, hglbtrtb,   hglbtrtr, as2888,           hglbtrtr,  hglbtrtr_state, init_by35_6,   ROT0, "Bally", "Harlem Globetrotters On Tour (7-digit conversion rev. 90)",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, hglbtrtrt,  hglbtrtr, as2888,           hglbtrtr,  hglbtrtr_state, init_by35_6,   ROT0, "Bally", "Harlem Globetrotters On Tour TEST", MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, dollyptn,   0,        as2888,           by35_35,   by35_state,     init_by35_6,   ROT0, "Bally", "Dolly Parton",                      MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, dollyptb,   dollyptn, as2888,           by35_35,   by35_state,     init_by35_6,   ROT0, "Bally", "Dolly Parton (7-digit conversion rev. 90)",                      MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, dollyptnt,  dollyptn, as2888,           by35_35,   by35_state,     init_by35_6,   ROT0, "Bally", "Dolly Parton TEST",                 MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, paragon,    0,        as2888,           by35_30,   by35_state,     init_by35_6,   ROT0, "Bally", "Paragon",                           MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, paragonb,   paragon,  as2888,           by35_30,   by35_state,     init_by35_6,   ROT0, "Bally", "Paragon (7-digit conversion rev. 20)",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, paragonc,   paragon,  as2888,           by35_30,   by35_state,     init_by35_6,   ROT0, "Bally", "Paragon (7-digit Free Play rev. 3)",        MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, paragond,   paragon,  as2888,           by35_30,   by35_state,     init_by35_6,   ROT0, "Bally", "Paragon (/10 Scoring Free Play)",           MACHINE_MECHANICAL | MACHINE_NOT_WORKING)

// AS-3022 sound
GAME( 1980, ngndshkr,   0,        as3022,           ngndshkr,  ngndshkr_state, init_by35_6,   ROT0, "Bally", "Nitro Ground Shaker (Attract Mode sound optional)",             MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, ngndshko,   ngndshkr, as3022,           ngndshko,  ngndshkr_state, init_by35_6,   ROT0, "Bally", "Nitro Ground Shaker (Attract Mode sound always)",               MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, ngndshkb,   ngndshkr, as3022,           ngndshkr,  ngndshkr_state, init_by35_7,   ROT0, "Bally", "Nitro Ground Shaker (7 Digit plus Attract Mode sound always)",  MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAMEL(1980, slbmania,   0,        as3022,           slbmania,  slbmania_state, init_by35_6,   ROT0, "Bally", "Silverball Mania",                  MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_slbmania)
GAME( 1979, futurspa,   0,        as3022,           futurspa,  futurspa_state, init_by35_6,   ROT0, "Bally", "Future Spa",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, futurspat,  futurspa, as3022,           futurspa,  futurspa_state, init_by35_6,   ROT0, "Bally", "Future Spa TEST",                   MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, spaceinv,   0,        as3022,           spaceinv,  spaceinv_state, init_by35_6,   ROT0, "Bally", "Space Invaders",                    MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, spaceinvt,  spaceinv, as3022,           spaceinv,  spaceinv_state, init_by35_6,   ROT0, "Bally", "Space Invaders TEST",               MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, rollston,   0,        as3022,           by35_35,   by35_state,     init_by35_6,   ROT0, "Bally", "Rolling Stones rev 18",             MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, rollston2,  rollston, as3022,           by35_35,   by35_state,     init_by35_6,   ROT0, "Bally", "Rolling Stones rev 21",             MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, mystic,     0,        as3022,           by35_35,   by35_state,     init_by35_6,   ROT0, "Bally", "Mystic",                            MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, xenon,      0,        sounds_plus,      xenon,     xenon_state,    init_xenon,    ROT0, "Bally", "Xenon",                             MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, xenont,     xenon,    sounds_plus,      xenon,     xenon_state,    init_xenon,    ROT0, "Bally", "Xenon TEST",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, xenonl,     xenon,    sounds_plus,      xenon,     xenon_state,    init_xenon,    ROT0, "Bally", "Xenon LAMP INVERSION",              MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, xenonf,     xenon,    sounds_plus,      xenon,     xenon_state,    init_xenon,    ROT0, "Bally", "Xenon (French)",                    MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, viking,     0,        as3022,           viking,    viking_state,   init_by35_6,   ROT0, "Bally", "Viking",                            MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, vikingt,    viking,   as3022,           viking,    viking_state,   init_by35_6,   ROT0, "Bally", "Viking TEST",                       MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, hotdoggn,   0,        as3022,           by35_35,   by35_state,     init_by35_6,   ROT0, "Bally", "Hotdoggin'",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, skatebll,   0,        as3022,           skatebll,  skatebll_state, init_by35_7,   ROT0, "Bally", "Skateball",                         MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, skateballt, skatebll, as3022,           skatebll,  skatebll_state, init_by35_7,   ROT0, "Bally", "Skateball TEST",                    MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, frontier,   0,        as3022,           frontier,  frontier_state, init_by35_7,   ROT0, "Bally", "Frontier",                          MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, frontiea,   frontier, as3022,           frontier,  frontier_state, init_by35_7,   ROT0, "Bally", "Frontier (Free Play rev. 1",        MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, frontiert,  frontier, as3022,           frontier,  frontier_state, init_by35_7,   ROT0, "Bally", "Frontier TEST",                     MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1982, speakesy,   0,        as3022,           by35_53,   by35_state,     init_by35_7,   ROT0, "Bally", "Speakeasy",                         MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1982, speakesy4p, speakesy, as3022,           by35_53,   by35_state,     init_by35_7,   ROT0, "Bally", "Speakeasy 4 Player",                MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1983, bmx,        0,        as3022,           by35_53,   by35_state,     init_by35_7,   ROT0, "Bally", "BMX",                               MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1983, granslam,   0,        as3022,           by35_53,   by35_state,     init_by35_7,   ROT0, "Bally", "Grand Slam",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1983, granslam4,  granslam, as3022,           by35_53,   by35_state,     init_by35_7,   ROT0, "Bally", "Grand Slam (4 Players)",            MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1983, goldball,   0,        as3022,           goldball,  goldball_state, init_goldball, ROT0, "Bally", "Gold Ball (set 1)",                 MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1983, goldballn,  goldball, as3022,           goldball,  goldball_state, init_goldball, ROT0, "Bally", "Gold Ball (Field Service Upgrade)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING)

// Squawk & Talk sound
GAME( 1981, flashgdn,   0,        squawk_n_talk_ay, flashgdn,  flashgdn_state, init_flashgdn, ROT0, "Bally", "Flash Gordon",                      MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981, flashgdnt,  flashgdn, squawk_n_talk_ay, flashgdn,  flashgdn_state, init_flashgdn, ROT0, "Bally", "Flash Gordon TEST",                 MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981, flashgdnf,  flashgdn, squawk_n_talk_ay, flashgdn,  flashgdn_state, init_flashgdn, ROT0, "Bally", "Flash Gordon (French)",             MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981, flashgdnv,  flashgdn, sounds_plus,      flashgdn,  flashgdn_state, init_flashgdn, ROT0, "Bally", "Flash Gordon (Vocalizer sound)",    MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981, fball_ii,   0,        squawk_n_talk_ay, fball_ii,  fball_ii_state, init_fball_ii, ROT0, "Bally", "Fireball II",                       MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981, eballdlx,   0,        squawk_n_talk_ay, eballdlx,  eballdlx_state, init_eballdlx, ROT0, "Bally", "Eight Ball Deluxe (rev. 15)",       MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981, eballd14,   eballdlx, squawk_n_talk_ay, eballdlx,  eballdlx_state, init_eballdlx, ROT0, "Bally", "Eight Ball Deluxe (rev. 14)",       MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981, eballdlxt,  eballdlx, squawk_n_talk_ay, eballdlx,  eballdlx_state, init_eballdlx, ROT0, "Bally", "Eight Ball Deluxe (rev. 16)",       MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981, embryon,    0,        squawk_n_talk,    embryon,   embryon_state,  init_embryon,  ROT0, "Bally", "Embryon",                           MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981, embryona,   embryon,  squawk_n_talk,    embryon,   embryon_state,  init_embryon,  ROT0, "Bally", "Embryon (Oliver Freeplay)",         MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981, embryond,   embryon,  squawk_n_talk,    embryon,   embryon_state,  init_embryon,  ROT0, "Bally", "Embryon (Oliver 7-Digit v09)",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981, embryone,   embryon,  squawk_n_talk,    embryon,   embryon_state,  init_embryon,  ROT0, "Bally", "Embryon (Oliver 7-Digit v92)",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1981, embryont,   embryon,  squawk_n_talk,    embryon,   embryon_state,  init_embryon,  ROT0, "Bally", "Embryon TEST",                      MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAMEL(1981, fathom,     0,        squawk_n_talk,    fathom,    fathom_state,   init_fathom,   ROT0, "Bally", "Fathom",                            MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53)
GAMEL(1981, centaur,    0,        squawk_n_talk,    centaur,   centaur_state,  init_centaur,  ROT0, "Bally", "Centaur",                           MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53)
GAMEL(1981, medusa,     0,        squawk_n_talk,    medusa,    medusa_state,   init_medusa,   ROT0, "Bally", "Medusa",                            MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53)
GAMEL(1981, medusat,    medusa,   squawk_n_talk,    medusa,    medusa_state,   init_medusa,   ROT0, "Bally", "Medusa TEST",                       MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53)
GAMEL(1982, vector,     0,        squawk_n_talk_ay, vector,    vector_state,   init_vector,   ROT0, "Bally", "Vector",                            MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53)
GAMEL(1982, vectora,    vector,   squawk_n_talk_ay, vector,    vector_state,   init_vector,   ROT0, "Bally", "Vector (Oliver Freeplay)",          MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53)
GAMEL(1982, vector4,    vector,   squawk_n_talk_ay, vector,    vector_state,   init_vector,   ROT0, "Bally", "Vector (Modified rules rev. 4)",    MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53)
GAMEL(1982, vectorb,    vector,   squawk_n_talk_ay, vector,    vector_state,   init_vector,   ROT0, "Bally", "Vector (Modified rules rev. 5)",    MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53)
GAMEL(1982, vectorc,    vector,   squawk_n_talk_ay_53, vector,    vector_state,   init_vector,   ROT0, "Bally", "Vector (Modified rules rev. 10)",   MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53)
GAME( 1981, elektra,    0,        squawk_n_talk_ay, by35,      by35_state,     init_by35_7,   ROT0, "Bally", "Elektra",                           MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAMEL(1982, spectrm,    0,        squawk_n_talk,    by35_53,   by35_state,     init_by35_7,   ROT0, "Bally", "Spectrum",                          MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53)
GAMEL(1982, spectrm4,   spectrm,  squawk_n_talk,    by35_53,   by35_state,     init_by35_7,   ROT0, "Bally", "Spectrum (ver 4)",                  MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53)
GAME( 1982, rapidfip,   0,        squawk_n_talk,    by35,      by35_state,     init_by35_7,   ROT0, "Bally", "Rapid Fire",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAMEL(1982, m_mpac,     0,        squawk_n_talk_ay, m_mpac,    m_mpac_state,   init_m_mpac,   ROT0, "Bally", "Mr. and Mrs. PacMan",               MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53)

// Cheap Squeak sound
GAME( 1984, kosteel,    0,        cheap_squeak,     kosteel,   kosteel_state,  init_by35_7,   ROT0, "Bally", "Kings of Steel",                    MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1983, xsandos,    0,        cheap_squeak,     xsandos,   xsandos_state,  init_by35_7,   ROT0, "Bally", "X's & O's",                         MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1984, spyhuntr,   0,        cheap_squeak,     spyhuntr,  spyhuntr_state, init_by35_7,   ROT0, "Bally", "Spy Hunter (Pinball)",              MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1984, fbclass,    0,        cheap_squeak,     fbclass,   fbclass_state,  init_fbclass,  ROT0, "Bally", "Fireball Classic",                  MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1984, blakpyra,   0,        cheap_squeak,     blakpyra,  blakpyra_state, init_blakpyra, ROT0, "Bally", "Black Pyramid",                     MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
//GAME( 1984, blakpyra,   blakpyra,  by35_barakandl,     by35_53,  by35_state,     init_by35_7,   ROT0, "Bally", "Black Pyramid",                     MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1985, cybrnaut,   0,        cheap_squeak,     cybrnaut,  cybrnaut_state, init_by35_7,   ROT0, "Bally", "Cybernaut",                         MACHINE_MECHANICAL | MACHINE_NOT_WORKING)

// Other manufacturers
GAMEL(1984, suprbowl,   xsandos,  as3022,           xsandos,   xsandos_state,  init_by35_7,   ROT0, "Bell Games",         "Super Bowl",                         MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAMEL(1984, suprbowlt,  xsandos,  as3022,           suprbowlt, xsandos_state,  init_by35_7,   ROT0, "Bell Games",         "Super Bowl TEST",                    MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAMEL(1984, tigerrag,   kosteel,  cheap_squeak,     kosteel,   kosteel_state,  init_by35_7,   ROT0, "Bell Games",         "Tiger Rag",                          MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAMEL(1984, tigerragt,  kosteel,  cheap_squeak,     tigerragt, kosteel_state,  init_by35_7,   ROT0, "Bell Games",         "Tiger Rag TEST",                     MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAMEL(1985, cosflash,   flashgdn, as3022,           flashgdn,  flashgdn_state, init_flashgdn, ROT0, "Bell Games",         "Cosmic Flash",                       MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAMEL(1985, cosflasht,  flashgdn, as3022,           flashgdn,  flashgdn_state, init_flashgdn, ROT0, "Bell Games",         "Cosmic Flash TEST",                  MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAMEL(1985, newwave,    blakpyra, cheap_squeak,     blakpyra,  blakpyra_state, init_blakpyra, ROT0, "Bell Games",         "New Wave",                           MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAMEL(1985, newwavet,   blakpyra, cheap_squeak,     newwavet,  blakpyra_state, init_blakpyra, ROT0, "Bell Games",         "New Wave TEST",                      MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAMEL(1985, saturn2,    spyhuntr, cheap_squeak,     spyhuntr,  spyhuntr_state, init_by35_7,   ROT0, "Bell Games",         "Saturn 2",                           MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAMEL(1985, saturn2t,   spyhuntr, cheap_squeak,     saturn2t,  spyhuntr_state, init_by35_7,   ROT0, "Bell Games",         "Saturn 2 Freeplay",                  MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAMEL(1985, worlddef,   0,        cheap_squeak,     by35_52,   by35_state,     init_by35_7,   ROT0, "Bell Games",         "World Defender",                     MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAMEL(1985, worlddeft,  worlddef, cheap_squeak,     worlddeft, by35_state,     init_by35_7,   ROT0, "Bell Games",         "World Defender TEST",                MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAMEL(1986, spacehaw,   cybrnaut, cheap_squeak,     cybrnaut,  cybrnaut_state, init_by35_7,   ROT0, "Nuova Bell Games",   "Space Hawks",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAMEL(1986, spacehawt,  cybrnaut, cheap_squeak,     spacehawt, cybrnaut_state, init_by35_7,   ROT0, "Nuova Bell Games",   "Space Hawks TEST",                   MACHINE_MECHANICAL | MACHINE_NOT_WORKING, layout_by35_53_nuova)
GAME( 1986, darkshad,   0,        bell_cheap_squeak,spyhuntr,  spyhuntr_state, init_by35_7,   ROT0, "Nuova Bell Games",   "Dark Shadow",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1986, darkshadt,  darkshad, bell_cheap_squeak,saturn2t,  spyhuntr_state, init_by35_7,   ROT0, "Nuova Bell Games",   "Dark Shadow TEST",                   MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1986, skflight,   0,        nuova,            by35,      by35_state,     init_by35_7,   ROT0, "Nuova Bell Games",   "Skill Flight",                       MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1987, cobrap,     0,        nuova,            by35,      by35_state,     init_by35_7,   ROT0, "Nuova Bell Games",   "Cobra",                              MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1987, futrquen,   0,        nuova,            by35,      by35_state,     init_by35_7,   ROT0, "Nuova Bell Games",   "Future Queen",                       MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1987, f1gpp,      0,        nuova,            by35,      by35_state,     init_by35_7,   ROT0, "Nuova Bell Games",   "F1 Grand Prix (Nuova Bell Games)",   MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1988, toppin,     0,        nuova,            by35,      by35_state,     init_by35_7,   ROT0, "Nuova Bell Games",   "Top Pin",                            MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1988, uboat65,    0,        nuova,            by35,      by35_state,     init_by35_7,   ROT0, "Nuova Bell Games",   "U-boat 65",                          MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1986, bullseye,   0,        grand,            bullseye,  bullseye_state, init_bullseye, ROT0, "Grand Products",     "301/Bullseye (301 Darts Scoring)",   MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1986, bullseyn,   0,        grand,            bullseye,  bullseye_state, init_bullseye, ROT0, "Grand Products",     "301/Bullseye (Traditional Scoring)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1986, bullseyet,  bullseye, by35,             bullseyet, bullseye_state, init_bullseye, ROT0, "Grand Products",     "301/Bullseye (301 Darts Scoring) TEST",   MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1986, bullseynt,  bullseyn, by35,             bullseyet, bullseye_state, init_bullseye, ROT0, "Grand Products",     "301/Bullseye (Traditional Scoring) TEST", MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1988, bbbowlin,   0,        by35,             by35,      by35_state,     init_by35_7,   ROT0, "United",             "Big Ball Bowling (Bowler)",          MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1988, monrobwl,   0,        by35,             by35,      by35_state,     init_by35_7,   ROT0, "Monroe Bowling Co.", "Stars & Strikes (Bowler)",           MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1984, bigbat,     0,        squawk_n_talk_ay, by35,      by35_state,     init_by35_7,   ROT0, "Bally Midway",       "Big Bat (Bat game)",                 MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1984, mdntmrdr,   0,        squawk_n_talk_ay, by35,      by35_state,     init_by35_6,   ROT0, "Bally Midway",       "Midnight Marauders (Gun game)",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1988, blbeauty,   0,        by35,             by35,      by35_state,     init_by35_7,   ROT0, "Stern",              "Black Beauty (Shuffle)",             MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1984, myststar,   0,        by35,             by35_30,   by35_state,     init_by35_6,   ROT0, "Zaccaria",           "Mystic Star",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING)

GAME( 2023, 64ktest,    0,        by35,             by35,      by35_state,     init_by35_6,   ROT0, "Quench",             "TEST 64kbit 8kByte ROM",             MACHINE_MECHANICAL | MACHINE_NOT_WORKING)

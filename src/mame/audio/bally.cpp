// license:BSD-3-Clause
// copyright-holders:Mike Harris, Quench
/***************************************************************************

    bally.cpp

    Functions to emulate the various Bally pinball sound boards.

***************************************************************************/

#include "emu.h"
#include "audio/bally.h"
#include "sound/volt_reg.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BALLY_AS2888,           bally_as2888_device,           "as2888",           "Bally AS2888 Sound Board")
DEFINE_DEVICE_TYPE(BALLY_AS3022,           bally_as3022_device,           "as3022",           "Bally AS3022 Sound Board")
DEFINE_DEVICE_TYPE(BALLY_SOUNDS_PLUS,      bally_sounds_plus_device,      "sounds_plus",      "Bally Sounds Plus w/ Vocalizer Board")
DEFINE_DEVICE_TYPE(BALLY_CHEAP_SQUEAK,     bally_cheap_squeak_device,     "cheap_squeak",     "Bally Cheap Squeak Board")
DEFINE_DEVICE_TYPE(BALLY_SQUAWK_N_TALK,    bally_squawk_n_talk_device,    "squawk_n_talk",    "Bally Squawk & Talk Board")
DEFINE_DEVICE_TYPE(BALLY_SQUAWK_N_TALK_AY, bally_squawk_n_talk_ay_device, "squawk_n_talk_ay", "Bally Squawk & Talk w/ AY8910 Board")


//**************************************************************************
//  AS2888
//**************************************************************************
static const discrete_mixer_desc as2888_digital_mixer_info =
{
		DISC_MIXER_IS_RESISTOR,                       /* type */
		{RES_K(33), RES_K(3.9)},                      /* r{} */
		{0, 0, 0, 0},                                 /* r_node */
		{0, 0},                                       /* c{} */
		0,                                            /* rI  */
//      RES_VOLTAGE_DIVIDER(RES_K(10), RES_R(360)),   /* rF  */
		RES_K(10),                                    /* rF  */   // not really
		CAP_U(0.01),                                  /* cF  */
		0,                                            /* cAmp */
		0,                                            /* vRef */
		0.00002                                       /* gain */
};

static const discrete_op_amp_filt_info as2888_preamp_info = {
		RES_K(10), 0, RES_R(470), 0,      /* r1 .. r4 */
		RES_K(10),                        /* rF */
		CAP_U(1),                         /* C1 */
		0,                                /* C2 */
		0,                                /* C3 */
		0.0,                              /* vRef */
		12.0,                             /* vP */
		-12.0,                            /* vN */
};

static DISCRETE_SOUND_START(as2888_discrete)

	DISCRETE_INPUT_DATA(NODE_08)        // Start Sustain Attenuation from 555 circuit
	DISCRETE_INPUT_LOGIC(NODE_01)       // Binary Counter B output (divide by 1) T2
	DISCRETE_INPUT_LOGIC(NODE_04)       // Binary Counter D output (divide by 4) T3

	DISCRETE_DIVIDE(NODE_11, 1, NODE_01, 1) // 2
	DISCRETE_DIVIDE(NODE_14, 1, NODE_04, 1)


	DISCRETE_RCFILTER(NODE_06, NODE_14, RES_K(15), CAP_U(0.1))      // T4 filter
#if 0
	DISCRETE_RCFILTER(NODE_05, NODE_11, RES_K(33), CAP_U(0.01))     // T1 filter
	DISCRETE_ADDER2(NODE_07, 1, NODE_05, NODE_06)
#else

	DISCRETE_MIXER2(NODE_07, 1, NODE_11, NODE_06, &as2888_digital_mixer_info)   // Mix and filter T1 and T4 together
#endif
	DISCRETE_RCDISC5(NODE_87, 1, NODE_08, RES_K(150), CAP_U(1.0))

	DISCRETE_RCFILTER_VREF(NODE_88,NODE_87,RES_M(1),CAP_U(0.01),2)
	DISCRETE_MULTIPLY(NODE_09, NODE_07, NODE_88)    // Apply sustain

	DISCRETE_OP_AMP_FILTER(NODE_20, 1, NODE_09, 0, DISC_OP_AMP_FILTER_IS_HIGH_PASS_1, &as2888_preamp_info)

	DISCRETE_CRFILTER(NODE_25, NODE_20, RES_M(100), CAP_U(0.05))    // Resistor is fake. Capacitor in series between pre-amp and output amp.

	DISCRETE_GAIN(NODE_30, NODE_25, 50) // Output amplifier LM380 fixed inbuilt gain of 50

	DISCRETE_OUTPUT(NODE_30, 10000000)  //  17000000
DISCRETE_SOUND_END

//-------------------------------------------------
//  sound_select - handle an external write to the board
//-------------------------------------------------
WRITE8_MEMBER(bally_as2888_device::sound_select)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_as2888_device::sound_select_sync), this), data);
}


TIMER_CALLBACK_MEMBER(bally_as2888_device::sound_select_sync)
{
	m_sound_select = param ^ 0x10;
}

//-------------------------------------------------
//  sound_int - handle an external sound interrupt to the board
//-------------------------------------------------
WRITE_LINE_MEMBER(bally_as2888_device::sound_int)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_as2888_device::sound_int_sync), this), state);
}

TIMER_CALLBACK_MEMBER(bally_as2888_device::sound_int_sync)
{
	if (param)
	{
		m_snd_sustain_timer->adjust(attotime::from_msec(5));
		m_discrete->write(machine().dummy_space(), NODE_08, 11);  // 11 volt pulse
	}
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------
MACHINE_CONFIG_START(bally_as2888_device::device_add_mconfig)
	MCFG_DEVICE_ADD("discrete", DISCRETE, as2888_discrete)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, *this, 1.00)

	MCFG_TIMER_DRIVER_ADD_PERIODIC(m_timer_s_freq, bally_as2888_device, timer_s, attotime::from_hz(353000))     // Inverter clock on AS-2888 sound board
	MCFG_TIMER_DRIVER_ADD(m_snd_sustain_timer, bally_as2888_device, timer_as2888)
MACHINE_CONFIG_END

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void bally_as2888_device::device_start()
{
	save_item(NAME(m_sound_select));
	save_item(NAME(m_snd_sel));
	save_item(NAME(m_snd_tone_gen));
	save_item(NAME(m_snd_div));
}

TIMER_DEVICE_CALLBACK_MEMBER(bally_as2888_device::timer_s)
{
	m_snd_tone_gen--;

	if ((m_snd_tone_gen == 0) && (m_snd_sel != 0x01))
	{
		m_snd_tone_gen = m_snd_sel;
		m_snd_div++;

		m_discrete->write(machine().dummy_space(), NODE_04, BIT(m_snd_div, 2) * 1);
		m_discrete->write(machine().dummy_space(), NODE_01, BIT(m_snd_div, 0) * 1);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(bally_as2888_device::timer_as2888)
{
	m_snd_sel = m_snd_prom[m_sound_select];
	m_snd_sel = bitswap<8>(m_snd_sel,0,1,2,3,4,5,6,7);
	m_snd_tone_gen = m_snd_sel;

	m_discrete->write(machine().dummy_space(), NODE_08, 0);
	m_snd_sustain_timer->adjust(attotime::never);
}


//**************************************************************************
//  AS3022
//**************************************************************************

//--------------------------------------------------------------------------
//  IO ports
//--------------------------------------------------------------------------
static INPUT_PORTS_START(as3022)
		PORT_START("SW1")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Sound Test") PORT_CHANGED_MEMBER(DEVICE_SELF, bally_as3022_device, sw1, 0)
INPUT_PORTS_END

ioport_constructor bally_as3022_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(as3022);
}

INPUT_CHANGED_MEMBER(bally_as3022_device::sw1)
{
	if (newval != oldval)
		m_cpu->set_input_line(INPUT_LINE_NMI, (newval ? ASSERT_LINE : CLEAR_LINE));
}

//-------------------------------------------------
//  sound_select - handle an external write to the board
//-------------------------------------------------
WRITE8_MEMBER(bally_as3022_device::sound_select)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_as3022_device::sound_select_sync), this), data);
}


TIMER_CALLBACK_MEMBER(bally_as3022_device::sound_select_sync)
{
	logerror("Writing %02x to m_sound_select,  was %02x\n", param, m_sound_select);
	m_sound_select = param;
}

//-------------------------------------------------
//  sound_int - handle an external sound interrupt to the board
//-------------------------------------------------
WRITE_LINE_MEMBER(bally_as3022_device::sound_int)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_as3022_device::sound_int_sync), this), state);
}

TIMER_CALLBACK_MEMBER(bally_as3022_device::sound_int_sync)
{
	logerror("Writing %02x to sound_int,  sound_select is %02x\n", param, m_sound_select);
	m_pia->ca1_w(param);
}

//-------------------------------------------------
//  CPU map, from schematics
//-------------------------------------------------
void bally_as3022_device::as3022_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x1fff);  // A13-15 are unconnected
	map(0x0000, 0x007f).mirror(0x0f00).bankrw("m6810");
	map(0x0080, 0x0083).mirror(0x0f7c).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x1fff).rom();  // 4k RAM space, but could be jumpered for 2k
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------
MACHINE_CONFIG_START(bally_as3022_device::device_add_mconfig)
	MCFG_DEVICE_ADD("cpu", M6808, DERIVED_CLOCK(1, 1))		//	XTAL(3'579'545))
	MCFG_DEVICE_PROGRAM_MAP(as3022_map)

	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, bally_as3022_device, pia_porta_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, bally_as3022_device, pia_porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, bally_as3022_device, pia_portb_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, bally_as3022_device, pia_cb2_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("cpu", M6802_IRQ_LINE))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("cpu", M6802_IRQ_LINE))

	for (required_device<filter_rc_device> &filter : m_ay_filters)
		// TODO: Calculate exact filter values. An AC filter is good enough for now
		// and required as the chip likes to output a DC offset at idle.
		FILTER_RC(config, filter).set_rc(filter_rc_device::AC, 10000, 0, 0, CAP_U(1));		/// FILTER_RC(config, filter).set_ac().add_route(ALL_OUTPUTS, *this, 1.0);
	MCFG_DEVICE_ADD("ay", AY8910, DERIVED_CLOCK(1, 4))
	MCFG_AY8910_PORT_A_READ_CB(READ8(*this, bally_as3022_device, ay_io_r))
///	MCFG_SOUND_ROUTE(0, "ay_filter0", 0.33)  MCFG_SOUND_ROUTE(1, "ay_filter1", 0.33)  MCFG_SOUND_ROUTE(2, "ay_filter2", 0.33)
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, *this, 0.33, 0)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128")
	MCFG_RAM_DEFAULT_VALUE(0xff)
MACHINE_CONFIG_END

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void bally_as3022_device::device_start()
{
	// Set volumes to a sane default.
	m_ay->set_volume(0, 0);
	m_ay->set_volume(1, 0);
	m_ay->set_volume(2, 0);

	save_item(NAME(m_bc1));
	save_item(NAME(m_bdir));
	save_item(NAME(m_sound_select));
	save_item(NAME(m_ay_data));
}

void bally_as3022_device::device_reset()
{
	membank("m6810")->set_base(m_ram->pointer());
}

//-------------------------------------------------
//  pia_porta_r - PIA port A reads
//-------------------------------------------------
READ8_MEMBER(bally_as3022_device::pia_porta_r)
{
	if (m_bc1 && !m_bdir)
	{
		m_ay_data = m_ay->read_data();
		return m_ay_data;
	}
	else
	{
		// Nothing is active on the bus, so return open bus.
		return 0xff;
	}
}

//-------------------------------------------------
//  pia_porta_w - PIA port A writes
//-------------------------------------------------
WRITE8_MEMBER(bally_as3022_device::pia_porta_w)
{
	if (m_bc1 && !m_bdir)
	{
		logerror("PIA port A bus contention!\n");
	}
	m_ay_data = data;
	update_ay_bus();
}

//-------------------------------------------------
//  pia_portb_w - PIA port B writes
//-------------------------------------------------
WRITE8_MEMBER(bally_as3022_device::pia_portb_w)
{
	m_bc1 = BIT(data, 0);
	m_bdir = BIT(data, 1);

	if (m_bc1 && !m_bdir)
	{
		m_ay_data = m_ay->read_data();
	}
	update_ay_bus();
}

//-------------------------------------------------
//  pia_cb2_w - PIA CB2 writes
//-------------------------------------------------
WRITE_LINE_MEMBER(bally_as3022_device::pia_cb2_w)
{
	// This pin is hooked up to the amp, and disables sounds when hi. TODO: It doesn't disable the sound, it adds a filter cap
	logerror("PIA CB2 write %1\n", state);

	if (state)
	{
		m_ay->set_volume(0, 128);
		m_ay->set_volume(1, 128);
		m_ay->set_volume(2, 128);
	}
	else
	{
		m_ay->set_volume(0, 0xff);
		m_ay->set_volume(1, 0xff);
		m_ay->set_volume(2, 0xff);
	}
}

//-------------------------------------------------
//  ay_io_r - AY8912 IO A reads (B is unconnected)
//-------------------------------------------------
READ8_MEMBER(bally_as3022_device::ay_io_r)
{
	// The two high bits are unconnected, the others are inverted.
	return (~m_sound_select) & 0x1f;
}

void bally_as3022_device::update_ay_bus()
{
	if (m_bc1 && m_bdir)
	{
		m_ay->write_address(m_ay_data);
	}
	else if (!m_bc1 && m_bdir)
	{
		m_ay->write_data(m_ay_data);
	}
}


//**************************************************************************
//  SOUNDS PLUS WITH VOCALIZER
//**************************************************************************

//-------------------------------------------------
//  CPU map, from schematics
//-------------------------------------------------
void bally_sounds_plus_device::sounds_plus_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).mirror(0x0f00).bankrw("m6810");
	map(0x0080, 0x0083).mirror(0x7f7c).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0xffff).rom();
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------
MACHINE_CONFIG_START(bally_sounds_plus_device::device_add_mconfig)
	bally_as3022_device::device_add_mconfig(config);

	MCFG_DEVICE_MODIFY("cpu")
	MCFG_DEVICE_PROGRAM_MAP(sounds_plus_map)

	MCFG_DEVICE_MODIFY("pia")
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, bally_sounds_plus_device, vocalizer_pia_portb_w))

	// TODO: Calculate exact filter values. An AC filter is good enough for now
	// and required as the chip likes to output a DC offset at idle.
	FILTER_RC(config, m_mc3417_filter).set_rc(filter_rc_device::AC, 10000, 0, 0, CAP_U(1));		 /// FILTER_RC(config, m_mc3417_filter).set_ac().add_route(ALL_OUTPUTS, *this, 1.0);;
	MCFG_DEVICE_ADD("mc3417", MC3417, 0)
///	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mc3417_filter", 2.2)		// A gain of 2.2 is a guess. It sounds about loud enough and doesn't clip.
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, *this, 4.4, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  pia_portb_w - PIA port B writes
//-------------------------------------------------
WRITE8_MEMBER(bally_sounds_plus_device::vocalizer_pia_portb_w)
{
	bool speech_clock = BIT(data, 6);
	bool speech_data = BIT(data, 7);
	m_mc3417->clock_w(speech_clock ? 1 : 0);
	m_mc3417->digit_w(speech_data ? 1 : 0);
	pia_portb_w(space, offset, data);
}


//**************************************************************************
//  Cheap Squeak
//**************************************************************************

//--------------------------------------------------------------------------
//  IO ports
//--------------------------------------------------------------------------
static INPUT_PORTS_START(cheap_squeak)
		PORT_START("SW1")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Sound Test") PORT_CHANGED_MEMBER(DEVICE_SELF, bally_cheap_squeak_device, sw1, 0)
INPUT_PORTS_END

ioport_constructor bally_cheap_squeak_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cheap_squeak);
}

INPUT_CHANGED_MEMBER(bally_cheap_squeak_device::sw1)
{
	if (newval != oldval)
		m_cpu->set_input_line(INPUT_LINE_NMI, (newval ? ASSERT_LINE : CLEAR_LINE));
}

//-------------------------------------------------
//  sound_select - handle an external write to the board
//-------------------------------------------------
WRITE8_MEMBER(bally_cheap_squeak_device::sound_select)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_cheap_squeak_device::sound_select_sync), this), data);
}

TIMER_CALLBACK_MEMBER(bally_cheap_squeak_device::sound_select_sync)
{
	m_sound_select = param;
}

//-------------------------------------------------
//  sound_int - handle an external sound interrupt to the board
//-------------------------------------------------
WRITE_LINE_MEMBER(bally_cheap_squeak_device::sound_int)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_cheap_squeak_device::sound_int_sync), this), state);
}

TIMER_CALLBACK_MEMBER(bally_cheap_squeak_device::sound_int_sync)
{
	m_sound_int = param;

	m_cpu->set_input_line(M6801_TIN_LINE, (m_sound_int ? ASSERT_LINE : CLEAR_LINE));
	update_led();
}

//-------------------------------------------------
//  CPU map, from schematics
//-------------------------------------------------
void bally_cheap_squeak_device::cheap_squeak_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0080, 0x00ff).ram();
	map(0x8000, 0x9fff).mirror(0x2000).rom();
	map(0xc000, 0xdfff).mirror(0x2000).rom();
}
void bally_cheap_squeak_device::cheap_squeak_io_map(address_map &map)
{
	map(M6801_PORT1, M6801_PORT1).w(FUNC(bally_cheap_squeak_device::out_p1_cb)); // P10-P17
	map(M6801_PORT2, M6801_PORT2).rw(FUNC(bally_cheap_squeak_device::in_p2_cb), FUNC(bally_cheap_squeak_device::out_p2_cb)); // P20-P24
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------
MACHINE_CONFIG_START(bally_cheap_squeak_device::device_add_mconfig)
	MCFG_DEVICE_ADD("cpu", M6803, DERIVED_CLOCK(1, 1))		//	XTAL(3'579'545))
	MCFG_DEVICE_PROGRAM_MAP(cheap_squeak_map)
	MCFG_DEVICE_IO_MAP(cheap_squeak_io_map)

	MCFG_DEVICE_ADD("dac", ZN429E, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, *this, 0.45)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void bally_cheap_squeak_device::device_start()
{
	m_sound_ack_w_cb.resolve_safe();
	save_item(NAME(m_sound_select));
	save_item(NAME(m_sound_int));
}

//-------------------------------------------------
//  out_p1_cb - IO port 1 write
//-------------------------------------------------
WRITE8_MEMBER(bally_cheap_squeak_device::out_p1_cb)
{
	m_dac->write(data);
}

//-------------------------------------------------
//  in_p2_cb - IO port 2 read
//-------------------------------------------------
READ8_MEMBER(bally_cheap_squeak_device::in_p2_cb)
{
  int sound_int_bit = m_sound_int ? 1 : 0;
  return 0x40 | (m_sound_select & 0x0f) << 1 | sound_int_bit;
}

//-------------------------------------------------
//  out_p2_cb - IO port 2 write
//-------------------------------------------------
WRITE8_MEMBER(bally_cheap_squeak_device::out_p2_cb)
{
	m_sound_ack = BIT(data, 0);
	if (!m_sound_ack_w_cb.isnull())
	{
		m_sound_ack_w_cb(m_sound_ack);
	}
	update_led();
}

void bally_cheap_squeak_device::update_led()
{
	// Either input or output can pull the led line high
	bool led_state = m_sound_int || m_sound_ack;
	machine().output().set_value("sound_led0", led_state);
}


//**************************************************************************
//  Squawk & Talk
//**************************************************************************

//--------------------------------------------------------------------------
//  IO ports
//--------------------------------------------------------------------------
static INPUT_PORTS_START(squawk_n_talk)
	PORT_START("SW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Sound Test") PORT_CHANGED_MEMBER(DEVICE_SELF, bally_squawk_n_talk_device, sw1, 0)
INPUT_PORTS_END

ioport_constructor bally_squawk_n_talk_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(squawk_n_talk);
}

INPUT_CHANGED_MEMBER(bally_squawk_n_talk_device::sw1)
{
	if (newval != oldval)
		m_cpu->set_input_line(INPUT_LINE_NMI, (newval ? ASSERT_LINE : CLEAR_LINE));
}

//-------------------------------------------------
//  sound_select - handle an external write to the board
//-------------------------------------------------
WRITE8_MEMBER(bally_squawk_n_talk_device::sound_select)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_squawk_n_talk_device::sound_select_sync), this), data);
}

TIMER_CALLBACK_MEMBER(bally_squawk_n_talk_device::sound_select_sync)
{
	m_sound_select = param;
}

//-------------------------------------------------
//  sound_int - handle an external sound interrupt to the board
//-------------------------------------------------
WRITE_LINE_MEMBER(bally_squawk_n_talk_device::sound_int)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_squawk_n_talk_device::sound_int_sync), this), state);
}

TIMER_CALLBACK_MEMBER(bally_squawk_n_talk_device::sound_int_sync)
{
	// the line runs though in inverter
	m_pia2->cb1_w(!param);
}

//-------------------------------------------------
//  CPU map, from schematics
//-------------------------------------------------
void bally_squawk_n_talk_device::squawk_n_talk_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).ram();  // internal RAM, could also be jumpered to use a 6808
	map(0x0080, 0x0083).mirror(0x4f6c).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).mirror(0x4f6c).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x1000).mirror(0x40ff).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x8000, 0x8fff).mirror(0x4000).rom();  // U2
	map(0x9000, 0x9fff).mirror(0x4000).rom();  // U3
	map(0xa000, 0xafff).mirror(0x4000).rom();  // U4
	map(0xb000, 0xbfff).mirror(0x4000).rom();  // U5
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------
MACHINE_CONFIG_START(bally_squawk_n_talk_device::device_add_mconfig)
	MCFG_DEVICE_ADD("cpu", M6808, DERIVED_CLOCK(1, 1))		//	XTAL(3'579'545))
	MCFG_DEVICE_PROGRAM_MAP(squawk_n_talk_map)

	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8("tms5200", tms5220_device, status_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8("tms5200", tms5220_device, data_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, bally_squawk_n_talk_device, pia1_portb_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("cpu", M6802_IRQ_LINE))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("cpu", M6802_IRQ_LINE))

	MCFG_DEVICE_ADD("pia2", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, bally_squawk_n_talk_device, pia2_porta_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(*this, bally_squawk_n_talk_device, pia2_ca2_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("cpu", M6802_IRQ_LINE))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("cpu", M6802_IRQ_LINE))

	FILTER_RC(config, m_dac_filter).add_route(ALL_OUTPUTS, *this, 1.0);
	m_dac_filter->set_rc(filter_rc_device::HIGHPASS, 2000, 0, 0, CAP_U(2));
	MCFG_DEVICE_ADD("dac", AD558, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "dac_filter", 0.25)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE(0, "dac", -1.0, DAC_VREF_NEG_INPUT)


	// TODO: Calculate exact filter values. An AC filter is good enough for now
	// and required as the chip likes to output a DC offset at idle.
	FILTER_RC(config, m_speech_filter).set_rc(filter_rc_device::AC, 10000, 0, 0, CAP_U(1));		 /// FILTER_RC(config, m_speech_filter).set_ac().add_route(ALL_OUTPUTS, *this, 1.0);
	MCFG_DEVICE_ADD("tms5200", TMS5200, 640000)
	MCFG_TMS52XX_READYQ_HANDLER(WRITELINE("pia1", pia6821_device, ca2_w))
	MCFG_TMS52XX_IRQ_HANDLER(WRITELINE("pia1", pia6821_device, cb1_w))
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, *this, 1.0, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void bally_squawk_n_talk_device::device_start()
{
	save_item(NAME(m_sound_select));
}

//-------------------------------------------------
//  pia1_portb_w - PIA 1 port B write
//-------------------------------------------------
WRITE8_MEMBER(bally_squawk_n_talk_device::pia1_portb_w)
{

	m_tms5200->rsq_w(BIT(data, 0));
	m_tms5200->wsq_w(BIT(data, 1));
}

//-------------------------------------------------
//  pia2_porta_r - PIA 2 port A reads
//-------------------------------------------------
READ8_MEMBER(bally_squawk_n_talk_device::pia2_porta_r)
{
	// 4 lines and they go through inverters
	return (~m_sound_select) & 0x0f;
}

//-------------------------------------------------
//  pia2_ca2_w - PIA 2 CA2 writes
//-------------------------------------------------
WRITE_LINE_MEMBER(bally_squawk_n_talk_device::pia2_ca2_w)
{
	machine().output().set_value("sound_led0", state);
}

//-------------------------------------------------
//  sound_select with AY8912 - handle an external write to the board
//-------------------------------------------------
WRITE8_MEMBER(bally_squawk_n_talk_ay_device::sound_select)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_squawk_n_talk_ay_device::sound_select_sync), this), data);
}

TIMER_CALLBACK_MEMBER(bally_squawk_n_talk_ay_device::sound_select_sync)
{
	m_sound_select = param;
}

//-------------------------------------------------
//  sound_int with AY8912 - handle an external sound interrupt to the board
//-------------------------------------------------
WRITE_LINE_MEMBER(bally_squawk_n_talk_ay_device::sound_int)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_squawk_n_talk_ay_device::sound_int_sync), this), state);
}

TIMER_CALLBACK_MEMBER(bally_squawk_n_talk_ay_device::sound_int_sync)
{
	// the line runs though in inverter
	m_pia2->cb1_w(!param);
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------
MACHINE_CONFIG_START(bally_squawk_n_talk_ay_device::device_add_mconfig)
	bally_squawk_n_talk_device::device_add_mconfig(config);

	MCFG_DEVICE_MODIFY("pia2")
	MCFG_PIA_READPA_HANDLER(READ8(*this, bally_squawk_n_talk_ay_device, pia2_porta_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, bally_squawk_n_talk_ay_device, pia2_porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, bally_squawk_n_talk_ay_device, pia2_portb_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(*this, bally_squawk_n_talk_ay_device, pia2_cb2_w))

	for (optional_device<filter_rc_device> &filter : m_ay_filters)
		// TODO: Calculate exact filter values. An AC filter is good enough for now
		// and required as the chip likes to output a DC offset at idle.
		FILTER_RC(config, filter).set_rc(filter_rc_device::AC, 10000, 0, 0, CAP_U(1));		/// FILTER_RC(config, filter).set_ac().add_route(ALL_OUTPUTS, *this, 1.0);
	MCFG_DEVICE_ADD("ay", AY8912, DERIVED_CLOCK(1, 4))
	MCFG_AY8910_PORT_A_READ_CB(READ8(*this, bally_squawk_n_talk_ay_device, ay_io_r))
///	MCFG_SOUND_ROUTE(0, "ay_filter0", 0.33)  MCFG_SOUND_ROUTE(1, "ay_filter1", 0.33)  MCFG_SOUND_ROUTE(2, "ay_filter2", 0.33)
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, *this, 0.33, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void bally_squawk_n_talk_ay_device::device_start()
{
	// Set volumes to a sane default.
	m_ay->set_volume(0, 0);
	m_ay->set_volume(1, 0);
	m_ay->set_volume(2, 0);

	save_item(NAME(m_bc1));
	save_item(NAME(m_bdir));
	save_item(NAME(m_ay_data));
}

//-------------------------------------------------
//  pia2_porta_r - PIA 2 port A reads
//-------------------------------------------------
READ8_MEMBER(bally_squawk_n_talk_ay_device::pia2_porta_r)
{
	if (m_bc1 && !m_bdir)
	{
		m_ay_data = m_ay->read_data();
	}
	// This should return the open bus, but this method is called even if the PIA
	// is in output mode. Self test expects to see the same value.
	return m_ay_data;
}

//-------------------------------------------------
//  pia2_porta_w - PIA 2 port A writes
//-------------------------------------------------
WRITE8_MEMBER(bally_squawk_n_talk_ay_device::pia2_porta_w)
{
	if (m_bc1 && !m_bdir)
	{
		logerror("PIA2 port A bus contention!\n");
	}
	m_ay_data = data;
	update_ay_bus();
}

//-------------------------------------------------
//  pia2_portb_w - PIA 2 port B writes
//-------------------------------------------------
WRITE8_MEMBER(bally_squawk_n_talk_ay_device::pia2_portb_w)
{
	m_bc1 = BIT(data, 0);
	m_bdir = BIT(data, 1);
	if (m_bc1 && !m_bdir)
	{
		m_ay_data = m_ay->read_data();
	}
	update_ay_bus();
}

//-------------------------------------------------
//  pia2_cb2_w - PIA 2 CB2 writes
//-------------------------------------------------
WRITE_LINE_MEMBER(bally_squawk_n_talk_ay_device::pia2_cb2_w)
{
	// This pin is hooked up to the amp, and disables sounds when hi
	if (state)
	{
		m_ay->set_volume(0, 0);
		m_ay->set_volume(1, 0);
		m_ay->set_volume(2, 0);
	}
	else
	{
		m_ay->set_volume(0, 0xff);
		m_ay->set_volume(1, 0xff);
		m_ay->set_volume(2, 0xff);
	}
}

//-------------------------------------------------
//  ay_io_r - AY8910 read
//-------------------------------------------------
READ8_MEMBER(bally_squawk_n_talk_ay_device::ay_io_r)
{
	// 4 lines and they go through inverters
	return (~m_sound_select) & 0x0f;
}

void bally_squawk_n_talk_ay_device::update_ay_bus()
{
	if (m_bc1 && m_bdir)
	{
		m_ay->write_address(m_ay_data);
	}
	else if (!m_bc1 && m_bdir)
	{
		m_ay->write_data(m_ay_data);
	}
}

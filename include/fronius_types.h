#ifndef FRONIUS_TYPES_H_
#define FRONIUS_TYPES_H_

#include <cstdint>
#include <optional>
#include <string>

/**
 * @brief Container for project enums and their string conversion helpers.
 *
 * This struct holds all related enums and provides static
 * constexpr `toString()` functions to convert enum values to
 * human-readable strings, useful for logging, error messages,
 * and debugging.
 */
struct FroniusTypes {
  /**
   * @brief Represents AC or DC phases.
   */
  enum class Phase {
    TOTAL,   /**< Total value */
    AVERAGE, /**< Average value */
    PHV,     /**< Average phase to neutral */
    PPV,     /**< Average phase to phase */
    A,       /**< Phase A to neutral */
    B,       /**< Phase B to neutral */
    C,       /**< Phase C to neutral */
    AB,      /**< Phase AB */
    BC,      /**< Phase BC */
    CA       /**< Phase CA */
  };

  /**
   * @brief Get a string representation of a Phase enum.
   *
   * @param phase The Phase value to convert.
   * @return A C-style string representing the Phase value.
   */
  static constexpr const char *toString(Phase phase) {
    switch (phase) {
    case Phase::TOTAL:
      return "TOTAL";
    case Phase::AVERAGE:
      return "AVERAGE";
    case Phase::PHV:
      return "PHV";
    case Phase::PPV:
      return "PPV";
    case Phase::A:
      return "A";
    case Phase::B:
      return "B";
    case Phase::C:
      return "C";
    case Phase::AB:
      return "AB";
    case Phase::BC:
      return "BC";
    case Phase::CA:
      return "CA";
    default:
      return "UNKNOWN";
    }
  }

  /**
   * @brief Represents inverter input channels.
   */
  enum class Input {
    TOTAL, /**< Total input */
    A,     /**< Input A */
    B      /**< Input B */
  };

  /**
   * @brief Get a string representation of an Input enum.
   *
   * @param input The Input value to convert.
   * @return A C-style string representing the Input value.
   */
  static constexpr const char *toString(Input input) {
    switch (input) {
    case Input::TOTAL:
      return "TOTAL";
    case Input::A:
      return "A";
    case Input::B:
      return "B";
    default:
      return "UNKNOWN";
    }
  }

  /**
   * @brief Types of power output reported by the inverter.
   */
  enum class Output {
    ACTIVE,      ///< Active (real) power in watts (W)
    APPARENT,    ///< Apparent power in volt-amperes (VA)
    REACTIVE,    ///< Reactive power in vars (VAr)
    Q1_REACTIVE, ///< Reactive power (Q1, inductive) in vars (VAr)
    Q4_REACTIVE, ///< Reactive power (Q4, capacitive) in vars (VAr)
    FACTOR       ///< Power factor in (%)
  };

  /**
   * @brief Converts an Output enum value to a string representation.
   *
   * @param output The Output enum value.
   * @return const char* A string literal representing the enum value.
   */
  static constexpr const char *toString(Output output) {
    switch (output) {
    case Output::ACTIVE:
      return "ACTIVE";
    case Output::APPARENT:
      return "APPARENT";
    case Output::Q1_REACTIVE:
      return "Q1_REACTIVE";
    case Output::Q4_REACTIVE:
      return "Q4_REACTIVE";
    case Output::FACTOR:
      return "FACTOR";
    }
    return "UNKNOWN";
  }

  /**
   * @brief Operating state codes of the inverter.
   *
   * This enumeration defines the possible runtime states of a Fronius inverter.
   * The values correspond to Modbus status codes reported by the device and
   * represent various operational, standby, and fault conditions.
   */
  enum class State : uint16_t {
    /** Inverter is turned off and not producing power. */
    POWER_OFF = 1,

    /** Inverter is in standby or low-power mode (sleeping). */
    SLEEPING,

    /** Inverter is starting up and initializing operation. */
    STARTING,

    /** Inverter is actively tracking the maximum power point (MPPT). */
    MPPT,

    /** Inverter output is throttled due to a limiting condition. */
    THROTTLED,

    /** Inverter is shutting down. */
    SHUTTING_DOWN,

    /** Inverter has encountered a fault condition. */
    FAULT,

    /** Inverter is in standby mode but ready to start operation. */
    STANDBY,

    /** DC bus initialization has not yet been completed. */
    NO_BUSINIT,

    /** Communication with the inverter has failed. */
    NO_COMM_INV,

    /** Serial number overcurrent or related hardware fault. */
    SN_OVERCURRENT,

    /** Inverter is in bootloader mode (firmware update or startup). */
    BOOTLOAD,

    /** Arc Fault Circuit Interruption (AFCI) has been triggered. */
    AFCI,
  };

  /**
   * @brief Convert an inverter status code to its string representation.
   *
   * This helper function returns a human-readable string for a given
   * `InverterStatus` value, allowing convenient logging or debugging
   * of inverter operating states.
   *
   * @param state The inverter status code.
   * @return A constant string corresponding to the provided status
   */
  static constexpr std::optional<const char *> toString(State state) {
    switch (state) {
    case State::POWER_OFF:
      return "Off";
    case State::SLEEPING:
      return "Sleeping (auto-shutdown)";
    case State::STARTING:
      return "Starting up";
    case State::MPPT:
      return "Tracking power point";
    case State::THROTTLED:
      return "Forced power reduction";
    case State::SHUTTING_DOWN:
      return "Shutting down";
    case State::FAULT:
      return "One or more faults exist";
    case State::STANDBY:
      return "Standby";
    case State::NO_BUSINIT:
      return "No SolarNet communication";
    case State::NO_COMM_INV:
      return "No communication with inverter";
    case State::SN_OVERCURRENT:
      return "Overcurrent on SolarNet plug detected";
    case State::BOOTLOAD:
      return "Inverter is being updated";
    case State::AFCI:
      return "AFCI Event";
    default:
      return "Invalid inverter operating state";
    }
  }

  /**
   * @brief Vendor-specific inverter fault and event flags.
   *
   * This enumeration represents 32-bit bitmask flags for various fault
   * conditions and events reported by a specific inverter vendor. Each
   * enumerator corresponds to a specific hardware or operational fault
   * condition. Multiple flags can be active simultaneously, allowing them to be
   * combined using bitwise operations.
   */
  enum class Event_1 : uint32_t {
    /** Insulation fault detected on the DC side. */
    INSULATION_FAULT = 0x00000001,

    /** Grid error detected. */
    GRID_ERROR = 0x00000002,

    /** AC overcurrent condition detected. */
    AC_OVERCURRENT = 0x00000004,

    /** DC overcurrent condition detected. */
    DC_OVERCURRENT = 0x00000008,

    /** Over-temperature condition detected. */
    OVER_TEMP = 0x00000010,

    /** Low power detected. */
    POWER_LOW = 0x00000020,

    /** Low DC voltage detected. */
    DC_LOW = 0x00000040,

    /** Intermediate fault (vendor-specific). */
    INTERMEDIATE_FAULT = 0x00000080,

    /** AC frequency above allowed limit. */
    FREQUENCY_HIGH = 0x00000100,

    /** AC frequency below allowed limit. */
    FREQUENCY_LOW = 0x00000200,

    /** AC voltage above allowed limit. */
    AC_VOLTAGE_HIGH = 0x00000400,

    /** AC voltage below allowed limit. */
    AC_VOLTAGE_LOW = 0x00000800,

    /** Direct current detected (vendor-specific). */
    DIRECT_CURRENT = 0x00001000,

    /** Relay fault detected. */
    RELAY_FAULT = 0x00002000,

    /** Power stage fault detected. */
    POWER_STAGE_FAULT = 0x00004000,

    /** Control fault detected. */
    CONTROL_FAULT = 0x00008000,

    /** Grid code voltage error detected. */
    GC_GRID_VOLT_ERR = 0x00010000,

    /** Grid code frequency error detected. */
    GC_GRID_FREQU_ERR = 0x00020000,

    /** Energy transfer fault detected. */
    ENERGY_TRANSFER_FAULT = 0x00040000,

    /** Reference power source is AC. */
    REF_POWER_SOURCE_AC = 0x00080000,

    /** Anti-islanding fault detected. */
    ANTI_ISLANDING_FAULT = 0x00100000,

    /** Fixed voltage fault detected. */
    FIXED_VOLTAGE_FAULT = 0x00200000,

    /** Memory fault detected. */
    MEMORY_FAULT = 0x00400000,

    /** Display fault detected. */
    DISPLAY_FAULT = 0x00800000,

    /** Communication fault detected. */
    COMMUNICATION_FAULT = 0x01000000,

    /** Temperature sensors fault detected. */
    TEMP_SENSORS_FAULT = 0x02000000,

    /** DC/AC board fault detected. */
    DC_AC_BOARD_FAULT = 0x04000000,

    /** ENS (Safety Relay) fault detected. */
    ENS_FAULT = 0x08000000,

    /** Fan fault detected. */
    FAN_FAULT = 0x10000000,

    /** Defective fuse detected. */
    DEFECTIVE_FUSE = 0x20000000,

    /** Output choke fault detected. */
    OUTPUT_CHOKE_FAULT = 0x40000000,

    /** Converter relay fault detected. */
    CONVERTER_RELAY_FAULT = 0x80000000,
  };

  /**
   * @brief Convert a Event_1 event flag to a human-readable string.
   *
   * @param event1 The Event_1 flag to convert.
   * @return A constant string describing the event, suitable for logging or
   * debugging.
   *
   * @note Only a single flag should be passed at a time. Multiple flags should
   *       be tested individually using bitwise operations.
   */
  static constexpr std::optional<const char *> toString(Event_1 event1) {
    switch (event1) {
    case Event_1::INSULATION_FAULT:
      return "DC Insulation fault";
    case Event_1::GRID_ERROR:
      return "Grid error";
    case Event_1::AC_OVERCURRENT:
      return "Overcurrent AC";
    case Event_1::DC_OVERCURRENT:
      return "Overcurrent DC";
    case Event_1::OVER_TEMP:
      return "Over-temperature";
    case Event_1::POWER_LOW:
      return "Power low";
    case Event_1::DC_LOW:
      return "DC low";
    case Event_1::INTERMEDIATE_FAULT:
      return "Intermediate circuit error";
    case Event_1::FREQUENCY_HIGH:
      return "AC frequency too high";
    case Event_1::FREQUENCY_LOW:
      return "AC frequency too low";
    case Event_1::AC_VOLTAGE_HIGH:
      return "AC voltage too high";
    case Event_1::AC_VOLTAGE_LOW:
      return "AC voltage too low";
    case Event_1::DIRECT_CURRENT:
      return "Direct current feed in";
    case Event_1::RELAY_FAULT:
      return "Relay problem";
    case Event_1::POWER_STAGE_FAULT:
      return "Internal power stage error";
    case Event_1::CONTROL_FAULT:
      return "Control problems";
    case Event_1::GC_GRID_VOLT_ERR:
      return "Guard Controller - AC voltage error";
    case Event_1::GC_GRID_FREQU_ERR:
      return "Guard Controller - AC Frequency Error";
    case Event_1::ENERGY_TRANSFER_FAULT:
      return "Energy transfer not possible";
    case Event_1::REF_POWER_SOURCE_AC:
      return "Reference power source AC outside tolerances";
    case Event_1::ANTI_ISLANDING_FAULT:
      return "Error during anti islanding test";
    case Event_1::FIXED_VOLTAGE_FAULT:
      return "Fixed voltage lower than current MPP voltage";
    case Event_1::MEMORY_FAULT:
      return "Memory fault";
    case Event_1::DISPLAY_FAULT:
      return "Display";
    case Event_1::COMMUNICATION_FAULT:
      return "Internal communication error";
    case Event_1::TEMP_SENSORS_FAULT:
      return "Temperature sensors defective";
    case Event_1::DC_AC_BOARD_FAULT:
      return "DC or AC board fault";
    case Event_1::ENS_FAULT:
      return "ENS error";
    case Event_1::FAN_FAULT:
      return "Fan error";
    case Event_1::DEFECTIVE_FUSE:
      return "Defective fuse";
    case Event_1::OUTPUT_CHOKE_FAULT:
      return "Output choke connected to wrong poles";
    case Event_1::CONVERTER_RELAY_FAULT:
      return "The buck converter relay does not open at high DC voltage";
    default:
      return std::nullopt;
    }
  }

  /**
   * @brief Vendor-specific inverter warning and diagnostic flags (Set 2).
   *
   * This enumeration defines a 32-bit bitmask of vendor-specific warning and
   * diagnostic conditions. Each bit corresponds to a specific fault or issue
   * that may be reported by the inverter firmware.
   *
   * Multiple flags can be combined using bitwise operations to represent
   * concurrent issues. Use helper functions such as `hasFlag()` or bitwise
   * tests to check individual flags.
   */
  enum class Event_2 : uint32_t {
    /** No communication with SolarNet or datalogger. */
    NO_SOLARNET_COMM = 0x00000001,

    /** Inverter address configuration fault. */
    INV_ADDRESS_FAULT = 0x00000002,

    /** No grid feed-in detected within the last 24 hours. */
    NO_FEED_IN_24H = 0x00000004,

    /** Plug or connector fault detected. */
    PLUG_FAULT = 0x00000008,

    /** Incorrect or missing phase allocation detected. */
    PHASE_ALLOC_FAULT = 0x00000010,

    /** Open conductor on grid connection detected. */
    GRID_CONDUCTOR_OPEN = 0x00000020,

    /** General software issue or firmware malfunction. */
    SOFTWARE_ISSUE = 0x00000040,

    /** Inverter power is being derated (limited). */
    POWER_DERATING = 0x00000080,

    /** Incorrect jumper configuration detected. */
    JUMPER_INCORRECT = 0x00000100,

    /** Feature incompatibility detected between modules or firmware. */
    INCOMPATIBLE_FEATURE = 0x00000200,

    /** Air vents or cooling pathways are blocked. */
    VENTS_BLOCKED = 0x00000400,

    /** Power reduction feature error. */
    POWER_REDUCTION_ERROR = 0x00000800,

    /** Arc detected in the DC circuit (AFCI triggered). */
    ARC_DETECTED = 0x00001000,

    /** AFCI self-test failed. */
    AFCI_SELF_TEST_FAILED = 0x00002000,

    /** Fault in current sensor or measurement circuit. */
    CURRENT_SENSOR_ERROR = 0x00004000,

    /** Fault in DC switch or disconnector. */
    DC_SWITCH_FAULT = 0x00008000,

    /** AFCI device defective. */
    AFCI_DEFECTIVE = 0x00010000,

    /** Manual AFCI test completed successfully. */
    AFCI_MANUAL_TEST_OK = 0x00020000,

    /** Power supply issue in power stage or control board. */
    PS_PWR_SUPPLY_ISSUE = 0x00040000,

    /** No communication with AFCI device. */
    AFCI_NO_COMM = 0x00080000,

    /** Manual AFCI test failed. */
    AFCI_MANUAL_TEST_FAILED = 0x00100000,

    /** AC polarity reversed on grid connection. */
    AC_POLARITY_REVERSED = 0x00200000,

    /** Faulty or defective AC-side component detected. */
    FAULTY_AC_DEVICE = 0x00400000,

    /** Flash memory fault or corruption detected. */
    FLASH_FAULT = 0x00800000,

    /** General inverter error (unspecified fault). */
    GENERAL_ERROR = 0x01000000,

    /** Grounding or protective earth issue detected. */
    GROUNDING_ISSUE = 0x02000000,

    /** Limitation fault — output limited due to external control or error. */
    LIMITATION_FAULT = 0x04000000,

    /** Open contact detected in protective circuit. */
    OPEN_CONTACT = 0x08000000,

    /** Overvoltage protection triggered. */
    OVERVOLTAGE_PROTECTION = 0x10000000,

    /** Program or firmware status message (non-critical). */
    PROGRAM_STATUS = 0x20000000,

    /** General SolarNet issue detected. */
    SOLARNET_ISSUE = 0x40000000,

    /** Supply voltage out of range or missing. */
    SUPPLY_VOLTAGE_FAULT = 0x80000000,
  };

  /**
   * @brief Convert a Event_2 event flag to a human-readable string.
   *
   * @param event2 The Event_2 event flag to convert.
   * @return A string representing the given event flag.
   *
   * @note This function is intended for single flag values only. If multiple
   *       flags are combined, test each one individually using bitwise logic
   *       before calling this function.
   */
  static constexpr std::optional<const char *> toString(Event_2 event2) {
    switch (event2) {
    case Event_2::NO_SOLARNET_COMM:
      return "No SolarNet communication";
    case Event_2::INV_ADDRESS_FAULT:
      return "Inverter address incorrect";
    case Event_2::NO_FEED_IN_24H:
      return "24h no feed in";
    case Event_2::PLUG_FAULT:
      return "Faulty plug connections";
    case Event_2::PHASE_ALLOC_FAULT:
      return "Incorrect phase allocation";
    case Event_2::GRID_CONDUCTOR_OPEN:
      return "Grid conductor open or supply phase has failed";
    case Event_2::SOFTWARE_ISSUE:
      return "Incompatible or old software";
    case Event_2::POWER_DERATING:
      return "Power Derating Due To Overtemperature";
    case Event_2::JUMPER_INCORRECT:
      return "Jumper set incorrectly";
    case Event_2::INCOMPATIBLE_FEATURE:
      return "Incompatible feature";
    case Event_2::VENTS_BLOCKED:
      return "Defective ventilator/air vents blocked";
    case Event_2::POWER_REDUCTION_ERROR:
      return "Power reduction on error";
    case Event_2::ARC_DETECTED:
      return "Arc Detected";
    case Event_2::AFCI_SELF_TEST_FAILED:
      return "AFCI Self Test Failed";
    case Event_2::CURRENT_SENSOR_ERROR:
      return "Current Sensor Error";
    case Event_2::DC_SWITCH_FAULT:
      return "DC switch fault";
    case Event_2::AFCI_DEFECTIVE:
      return "AFCI Defective";
    case Event_2::AFCI_MANUAL_TEST_OK:
      return "AFCI Manual Test Successful";
    case Event_2::PS_PWR_SUPPLY_ISSUE:
      return "Power Stack Supply Missing";
    case Event_2::AFCI_NO_COMM:
      return "AFCI Communication Stopped";
    case Event_2::AFCI_MANUAL_TEST_FAILED:
      return "AFCI Manual Test Failed";
    case Event_2::AC_POLARITY_REVERSED:
      return "AC polarity reversed";
    case Event_2::FAULTY_AC_DEVICE:
      return "AC measurement device fault";
    case Event_2::FLASH_FAULT:
      return "Flash fault";
    case Event_2::GENERAL_ERROR:
      return "General error";
    case Event_2::GROUNDING_ISSUE:
      return "Grounding fault";
    case Event_2::LIMITATION_FAULT:
      return "Power limitation fault";
    case Event_2::OPEN_CONTACT:
      return "External NO contact open";
    case Event_2::OVERVOLTAGE_PROTECTION:
      return "External overvoltage protection has tripped";
    case Event_2::PROGRAM_STATUS:
      return "Internal processor program status";
    case Event_2::SOLARNET_ISSUE:
      return "SolarNet issue";
    case Event_2::SUPPLY_VOLTAGE_FAULT:
      return "Supply voltage fault";
    default:
      return std::nullopt;
    }
  }

  /**
   * @brief Vendor-specific inverter diagnostic flags (Set 3).
   *
   * This enumeration defines an additional set of vendor-specific fault and
   * diagnostic flags reported by the inverter. Unlike `Event_1` and
   * `Event_2`, this set typically represents less common or auxiliary faults.
   *
   * Each bit corresponds to an independent fault condition and may be combined
   * as a bitfield. Use bitwise operations to check for specific flags.
   */
  enum class Event_3 {
    /** Real-time clock or system time synchronization fault. */
    TIME_FAULT = 1,

    /** Fault in USB interface or communication. */
    USB_FAULT = 2,

    /** DC voltage too high — overvoltage condition detected. */
    DC_HIGH = 4,

    /** Initialization or startup error. */
    INIT_ERROR = 8,
  };

  /**
   * @brief Convert a Event_3 event flag to a human-readable string.
   *
   * @param event3 The Event_3 event flag to convert.
   * @return A descriptive string corresponding to the given event flag.
   *
   * @note This function is intended for single flag values only. If multiple
   *       flags are combined, test each individually before calling this
   * function.
   */
  static constexpr std::optional<const char *> toString(Event_3 event3) {
    switch (event3) {
    case Event_3::TIME_FAULT:
      return "Time error";
    case Event_3::USB_FAULT:
      return "USB error";
    case Event_3::DC_HIGH:
      return "DC high";
    case Event_3::INIT_ERROR:
      return "Init error";
    default:
      return std::nullopt;
    }
  }

  /** @brief Information about a remote Modbus TCP endpoint. */
  struct RemoteEndpoint {
    std::string ip; ///< IP address (IPv4 or IPv6)
    int port{0};    ///< TCP port number
  };
};

#endif /* FRONIUS_TYPES_H_ */

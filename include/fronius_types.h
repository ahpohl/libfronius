#ifndef FRONIUS_TYPES_H_
#define FRONIUS_TYPES_H_

#include <cstdint>
#include <optional>

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
    TOTAL,   /**< Total phase */
    AVERAGE, /**< Average phase */
    A,       /**< Phase A */
    B,       /**< Phase B */
    C        /**< Phase C */
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
    case Phase::A:
      return "A";
    case Phase::B:
      return "B";
    case Phase::C:
      return "C";
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
   * @param status The inverter status code.
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
   * @brief Inverter event and fault flags.
   *
   * This enumeration defines bitmask flags that represent various inverter
   * events, warnings, and fault conditions as reported by the device.
   * Each flag corresponds to a specific hardware or operational state,
   * and multiple flags may be set simultaneously in a single status word.
   *
   * The values are typically read from a Modbus event register and can
   * be used to interpret the inverter's fault or warning conditions.
   */
  enum class Event : uint32_t {
    /** Ground fault detected on the DC side. */
    GROUND_FAULT = 0x0001,

    /** DC over-voltage condition detected. */
    DC_OVER_VOLT = 0x0002,

    /** AC disconnect switch is open. */
    AC_DISCONNECT = 0x0004,

    /** DC disconnect switch is open. */
    DC_DISCONNECT = 0x0008,

    /** Grid connection has been lost or intentionally disconnected. */
    GRID_DISCONNECT = 0x0010,

    /** Inverter cabinet door is open. */
    CABINET_OPEN = 0x0020,

    /** Manual shutdown initiated. */
    MANUAL_SHUTDOWN = 0x0040,

    /** Inverter over-temperature condition. */
    OVER_TEMP = 0x0080,

    /** AC frequency above permitted limit. */
    OVER_FREQUENCY = 0x0100,

    /** AC frequency below permitted limit. */
    UNDER_FREQUENCY = 0x0200,

    /** AC voltage above permitted limit. */
    AC_OVER_VOLT = 0x0400,

    /** AC voltage below permitted limit. */
    AC_UNDER_VOLT = 0x0800,

    /** Blown fuse detected in one of the DC strings. */
    BLOWN_STRING_FUSE = 0x1000,

    /** Inverter under-temperature condition. */
    UNDER_TEMP = 0x2000,

    /** Internal memory or communication error. */
    MEMORY_LOSS = 0x4000,

    /** Hardware self-test failure detected. */
    HW_TEST_FAILURE = 0x8000
  };

  /**
   * @brief Convert an inverter event flag to a human-readable string.
   *
   * This helper function translates a given `Event` flag into a descriptive
   * string, suitable for logging, debugging, or user display.
   *
   * @param event The inverter event flag to convert.
   * @return A constant character string describing the event, or
   *         `"Invalid inverter event flag"` if the flag is not recognized.
   *
   * @note Multiple event flags can be active simultaneously; this function
   *       converts only a single flag value. When handling bitfields,
   *       each bit should be tested individually before calling this function.
   */
  static constexpr std::optional<const char *> toString(Event event) {
    switch (event) {
    case Event::GROUND_FAULT:
      return "Ground fault";
    case Event::DC_OVER_VOLT:
      return "DC over voltage";
    case Event::AC_DISCONNECT:
      return "AC disconnect open";
    case Event::DC_DISCONNECT:
      return "DC disconnect open";
    case Event::GRID_DISCONNECT:
      return "Grid shutdown";
    case Event::CABINET_OPEN:
      return "Cabinet open";
    case Event::MANUAL_SHUTDOWN:
      return "Manual shutdown";
    case Event::OVER_TEMP:
      return "Over temperature";
    case Event::OVER_FREQUENCY:
      return "Frequency above limit";
    case Event::UNDER_FREQUENCY:
      return "Frequency under limit";
    case Event::AC_OVER_VOLT:
      return "AC voltage above limit";
    case Event::AC_UNDER_VOLT:
      return "AC voltage under limit";
    case Event::BLOWN_STRING_FUSE:
      return "Blown string fuse";
    case Event::UNDER_TEMP:
      return "Under temperature";
    case Event::MEMORY_LOSS:
      return "Generic Memory or Communication error (internal)";
    case Event::HW_TEST_FAILURE:
      return "Hardware test failure";
    default:
      return std::nullopt;
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
  enum class Vendor_1 : uint32_t {
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
   * @brief Convert a Vendor_1 event flag to a human-readable string.
   *
   * @param event The Vendor_1 flag to convert.
   * @return A constant string describing the event, suitable for logging or
   * debugging.
   *
   * @note Only a single flag should be passed at a time. Multiple flags should
   *       be tested individually using bitwise operations.
   */
  static constexpr std::optional<const char *> toString(Vendor_1 vendor1) {
    switch (vendor1) {
    case Vendor_1::INSULATION_FAULT:
      return "DC Insulation fault";
    case Vendor_1::GRID_ERROR:
      return "Grid error";
    case Vendor_1::AC_OVERCURRENT:
      return "Overcurrent AC";
    case Vendor_1::DC_OVERCURRENT:
      return "Overcurrent DC";
    case Vendor_1::OVER_TEMP:
      return "Over-temperature";
    case Vendor_1::POWER_LOW:
      return "Power low";
    case Vendor_1::DC_LOW:
      return "DC low";
    case Vendor_1::INTERMEDIATE_FAULT:
      return "Intermediate circuit error";
    case Vendor_1::FREQUENCY_HIGH:
      return "AC frequency too high";
    case Vendor_1::FREQUENCY_LOW:
      return "AC frequency too low";
    case Vendor_1::AC_VOLTAGE_HIGH:
      return "AC voltage too high";
    case Vendor_1::AC_VOLTAGE_LOW:
      return "AC voltage too low";
    case Vendor_1::DIRECT_CURRENT:
      return "Direct current feed in";
    case Vendor_1::RELAY_FAULT:
      return "Relay problem";
    case Vendor_1::POWER_STAGE_FAULT:
      return "Internal power stage error";
    case Vendor_1::CONTROL_FAULT:
      return "Control problems";
    case Vendor_1::GC_GRID_VOLT_ERR:
      return "Guard Controller - AC voltage error";
    case Vendor_1::GC_GRID_FREQU_ERR:
      return "Guard Controller - AC Frequency Error";
    case Vendor_1::ENERGY_TRANSFER_FAULT:
      return "Energy transfer not possible";
    case Vendor_1::REF_POWER_SOURCE_AC:
      return "Reference power source AC outside tolerances";
    case Vendor_1::ANTI_ISLANDING_FAULT:
      return "Error during anti islanding test";
    case Vendor_1::FIXED_VOLTAGE_FAULT:
      return "Fixed voltage lower than current MPP voltage";
    case Vendor_1::MEMORY_FAULT:
      return "Memory fault";
    case Vendor_1::DISPLAY_FAULT:
      return "Display";
    case Vendor_1::COMMUNICATION_FAULT:
      return "Internal communication error";
    case Vendor_1::TEMP_SENSORS_FAULT:
      return "Temperature sensors defective";
    case Vendor_1::DC_AC_BOARD_FAULT:
      return "DC or AC board fault";
    case Vendor_1::ENS_FAULT:
      return "ENS error";
    case Vendor_1::FAN_FAULT:
      return "Fan error";
    case Vendor_1::DEFECTIVE_FUSE:
      return "Defective fuse";
    case Vendor_1::OUTPUT_CHOKE_FAULT:
      return "Output choke connected to wrong poles";
    case Vendor_1::CONVERTER_RELAY_FAULT:
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
  enum class Vendor_2 : uint32_t {
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
   * @brief Convert a Vendor_2 event flag to a human-readable string.
   *
   * @param event The Vendor_2 event flag to convert.
   * @return A string representing the given event flag.
   *
   * @note This function is intended for single flag values only. If multiple
   *       flags are combined, test each one individually using bitwise logic
   *       before calling this function.
   */
  static constexpr std::optional<const char *> toString(Vendor_2 vendor2) {
    switch (vendor2) {
    case Vendor_2::NO_SOLARNET_COMM:
      return "No SolarNet communication";
    case Vendor_2::INV_ADDRESS_FAULT:
      return "Inverter address incorrect";
    case Vendor_2::NO_FEED_IN_24H:
      return "24h no feed in";
    case Vendor_2::PLUG_FAULT:
      return "Faulty plug connections";
    case Vendor_2::PHASE_ALLOC_FAULT:
      return "Incorrect phase allocation";
    case Vendor_2::GRID_CONDUCTOR_OPEN:
      return "Grid conductor open or supply phase has failed";
    case Vendor_2::SOFTWARE_ISSUE:
      return "Incompatible or old software";
    case Vendor_2::POWER_DERATING:
      return "Power Derating Due To Overtemperature";
    case Vendor_2::JUMPER_INCORRECT:
      return "Jumper set incorrectly";
    case Vendor_2::INCOMPATIBLE_FEATURE:
      return "Incompatible feature";
    case Vendor_2::VENTS_BLOCKED:
      return "Defective ventilator/air vents blocked";
    case Vendor_2::POWER_REDUCTION_ERROR:
      return "Power reduction on error";
    case Vendor_2::ARC_DETECTED:
      return "Arc Detected";
    case Vendor_2::AFCI_SELF_TEST_FAILED:
      return "AFCI Self Test Failed";
    case Vendor_2::CURRENT_SENSOR_ERROR:
      return "Current Sensor Error";
    case Vendor_2::DC_SWITCH_FAULT:
      return "DC switch fault";
    case Vendor_2::AFCI_DEFECTIVE:
      return "AFCI Defective";
    case Vendor_2::AFCI_MANUAL_TEST_OK:
      return "AFCI Manual Test Successful";
    case Vendor_2::PS_PWR_SUPPLY_ISSUE:
      return "Power Stack Supply Missing";
    case Vendor_2::AFCI_NO_COMM:
      return "AFCI Communication Stopped";
    case Vendor_2::AFCI_MANUAL_TEST_FAILED:
      return "AFCI Manual Test Failed";
    case Vendor_2::AC_POLARITY_REVERSED:
      return "AC polarity reversed";
    case Vendor_2::FAULTY_AC_DEVICE:
      return "AC measurement device fault";
    case Vendor_2::FLASH_FAULT:
      return "Flash fault";
    case Vendor_2::GENERAL_ERROR:
      return "General error";
    case Vendor_2::GROUNDING_ISSUE:
      return "Grounding fault";
    case Vendor_2::LIMITATION_FAULT:
      return "Power limitation fault";
    case Vendor_2::OPEN_CONTACT:
      return "External NO contact open";
    case Vendor_2::OVERVOLTAGE_PROTECTION:
      return "External overvoltage protection has tripped";
    case Vendor_2::PROGRAM_STATUS:
      return "Internal processor program status";
    case Vendor_2::SOLARNET_ISSUE:
      return "SolarNet issue";
    case Vendor_2::SUPPLY_VOLTAGE_FAULT:
      return "Supply voltage fault";
    default:
      return std::nullopt;
    }
  }

  /**
   * @brief Vendor-specific inverter diagnostic flags (Set 3).
   *
   * This enumeration defines an additional set of vendor-specific fault and
   * diagnostic flags reported by the inverter. Unlike `Vendor_1` and
   * `Vendor_2`, this set typically represents less common or auxiliary faults.
   *
   * Each bit corresponds to an independent fault condition and may be combined
   * as a bitfield. Use bitwise operations to check for specific flags.
   */
  enum class Vendor_3 {
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
   * @brief Convert a Vendor_3 event flag to a human-readable string.
   *
   * @param event The Vendor_3 event flag to convert.
   * @return A descriptive string corresponding to the given event flag.
   *
   * @note This function is intended for single flag values only. If multiple
   *       flags are combined, test each individually before calling this
   * function.
   */
  static constexpr std::optional<const char *> toString(Vendor_3 vendor3) {
    switch (vendor3) {
    case Vendor_3::TIME_FAULT:
      return "Time error";
    case Vendor_3::USB_FAULT:
      return "USB error";
    case Vendor_3::DC_HIGH:
      return "DC high";
    case Vendor_3::INIT_ERROR:
      return "Init error";
    default:
      return std::nullopt;
    }
  }
};

#endif /* FRONIUS_TYPES_H_ */
